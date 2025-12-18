#include "ai/cllm_message_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// INTERNAL HELPER FUNCTIONS
// ============================================================================

/**
 * Allocate a new queue node
 */
static QueueNode* allocate_node(LockFreeMessageQueue* queue, SphereMessage* message) {
    QueueNode* node = NULL;
    
    // Try to get node from free list first
    if (queue->max_free_nodes > 0) {
        QueueNode* free_head = atomic_load(&queue->free_list);
        while (free_head != NULL) {
            QueueNode* next = atomic_load(&free_head->next);
            if (atomic_compare_exchange_weak(&queue->free_list, &free_head, next)) {
                // Successfully got node from free list
                node = free_head;
                atomic_fetch_sub(&queue->free_count, 1);
                break;
            }
            // CAS failed, retry with updated free_head
        }
    }
    
    // If no node from free list, allocate new one
    if (node == NULL) {
        node = aligned_alloc(64, sizeof(QueueNode));
        if (!node) {
            return NULL;
        }
    }
    
    // Initialize node
    node->message = message;
    atomic_init(&node->next, NULL);
    node->sequence = atomic_fetch_add(&queue->sequence_counter, 1);
    
    return node;
}

/**
 * Free a queue node (return to free list or deallocate)
 */
static void free_node(LockFreeMessageQueue* queue, QueueNode* node) {
    if (!node) return;
    
    // Try to return to free list if not full
    if (queue->max_free_nodes > 0 && 
        atomic_load(&queue->free_count) < queue->max_free_nodes) {
        
        QueueNode* free_head = atomic_load(&queue->free_list);
        do {
            atomic_store(&node->next, free_head);
        } while (!atomic_compare_exchange_weak(&queue->free_list, &free_head, node));
        
        atomic_fetch_add(&queue->free_count, 1);
    } else {
        // Free list full or disabled, deallocate
        free(node);
    }
}

/**
 * Get current time in nanoseconds
 */
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// QUEUE OPERATIONS
// ============================================================================

LockFreeMessageQueue* message_queue_create(uint64_t max_queue_size,
                                           bool drop_on_full) {
    LockFreeMessageQueue* queue = aligned_alloc(64, sizeof(LockFreeMessageQueue));
    if (!queue) {
        return NULL;
    }
    
    // Zero out structure
    memset(queue, 0, sizeof(LockFreeMessageQueue));
    
    // Initialize priority queues
    for (int i = 0; i < 4; i++) {
        // Create dummy head node for each priority queue
        QueueNode* dummy = aligned_alloc(64, sizeof(QueueNode));
        if (!dummy) {
            // Cleanup and return NULL
            for (int j = 0; j < i; j++) {
                QueueNode* d = atomic_load(&queue->queues[j].head);
                free(d);
            }
            free(queue);
            return NULL;
        }
        
        dummy->message = NULL;
        atomic_init(&dummy->next, NULL);
        dummy->sequence = 0;
        
        atomic_init(&queue->queues[i].head, dummy);
        atomic_init(&queue->queues[i].tail, dummy);
        atomic_init(&queue->queues[i].count, 0);
    }
    
    // Initialize statistics
    atomic_init(&queue->total_enqueued, 0);
    atomic_init(&queue->total_dequeued, 0);
    atomic_init(&queue->total_dropped, 0);
    atomic_init(&queue->enqueue_failures, 0);
    atomic_init(&queue->dequeue_failures, 0);
    
    // Set configuration
    queue->max_queue_size = max_queue_size;
    queue->drop_on_full = drop_on_full;
    
    // Initialize sequence counter
    atomic_init(&queue->sequence_counter, 1);
    
    // Initialize free list
    atomic_init(&queue->free_list, NULL);
    atomic_init(&queue->free_count, 0);
    queue->max_free_nodes = 1000; // Cache up to 1000 nodes
    
    return queue;
}

void message_queue_free(LockFreeMessageQueue* queue) {
    if (!queue) return;
    
    // Clear all messages
    message_queue_clear(queue, true);
    
    // Free dummy head nodes
    for (int i = 0; i < 4; i++) {
        QueueNode* head = atomic_load(&queue->queues[i].head);
        free(head);
    }
    
    // Free all nodes in free list
    QueueNode* free_node = atomic_load(&queue->free_list);
    while (free_node != NULL) {
        QueueNode* next = atomic_load(&free_node->next);
        free(free_node);
        free_node = next;
    }
    
    free(queue);
}

bool message_queue_enqueue(LockFreeMessageQueue* queue,
                          SphereMessage* message) {
    if (!queue || !message) {
        if (queue) atomic_fetch_add(&queue->enqueue_failures, 1);
        return false;
    }
    
    // Check if queue is full
    if (queue->max_queue_size > 0) {
        uint64_t current_size = message_queue_size(queue);
        if (current_size >= queue->max_queue_size) {
            if (queue->drop_on_full) {
                atomic_fetch_add(&queue->total_dropped, 1);
                sphere_message_free(message);
                return false;
            } else {
                atomic_fetch_add(&queue->enqueue_failures, 1);
                return false;
            }
        }
    }
    
    // Get priority queue
    MessagePriority priority = message->priority;
    if (priority < MSG_PRIORITY_LOW || priority > MSG_PRIORITY_CRITICAL) {
        priority = MSG_PRIORITY_NORMAL;
    }
    
    PriorityQueueHead* pq = &queue->queues[priority];
    
    // Allocate node
    QueueNode* node = allocate_node(queue, message);
    if (!node) {
        atomic_fetch_add(&queue->enqueue_failures, 1);
        return false;
    }
    
    // Enqueue using lock-free algorithm
    while (true) {
        QueueNode* tail = atomic_load(&pq->tail);
        QueueNode* next = atomic_load(&tail->next);
        
        // Check if tail is still the last node
        if (tail == atomic_load(&pq->tail)) {
            if (next == NULL) {
                // Try to link new node at the end
                if (atomic_compare_exchange_weak(&tail->next, &next, node)) {
                    // Successfully enqueued, try to swing tail
                    atomic_compare_exchange_weak(&pq->tail, &tail, node);
                    atomic_fetch_add(&pq->count, 1);
                    atomic_fetch_add(&queue->total_enqueued, 1);
                    return true;
                }
            } else {
                // Tail is lagging, try to advance it
                atomic_compare_exchange_weak(&pq->tail, &tail, next);
            }
        }
    }
}

SphereMessage* message_queue_dequeue(LockFreeMessageQueue* queue) {
    if (!queue) return NULL;
    
    // Try to dequeue from highest priority first
    for (int priority = MSG_PRIORITY_CRITICAL; priority >= MSG_PRIORITY_LOW; priority--) {
        PriorityQueueHead* pq = &queue->queues[priority];
        
        // Skip if queue is empty
        if (atomic_load(&pq->count) == 0) {
            continue;
        }
        
        // Dequeue using lock-free algorithm
        while (true) {
            QueueNode* head = atomic_load(&pq->head);
            QueueNode* tail = atomic_load(&pq->tail);
            QueueNode* next = atomic_load(&head->next);
            
            // Check if head is still the first node
            if (head == atomic_load(&pq->head)) {
                if (head == tail) {
                    // Queue is empty or tail is lagging
                    if (next == NULL) {
                        // Queue is empty
                        break;
                    }
                    // Tail is lagging, try to advance it
                    atomic_compare_exchange_weak(&pq->tail, &tail, next);
                } else {
                    // Try to dequeue
                    SphereMessage* message = next->message;
                    if (atomic_compare_exchange_weak(&pq->head, &head, next)) {
                        // Successfully dequeued
                        atomic_fetch_sub(&pq->count, 1);
                        atomic_fetch_add(&queue->total_dequeued, 1);
                        
                        // Free old head node
                        free_node(queue, head);
                        
                        return message;
                    }
                }
            }
        }
    }
    
    // No messages in any queue
    atomic_fetch_add(&queue->dequeue_failures, 1);
    return NULL;
}

SphereMessage* message_queue_peek(const LockFreeMessageQueue* queue) {
    if (!queue) return NULL;
    
    // Try to peek from highest priority first
    for (int priority = MSG_PRIORITY_CRITICAL; priority >= MSG_PRIORITY_LOW; priority--) {
        const PriorityQueueHead* pq = &queue->queues[priority];
        
        // Skip if queue is empty
        if (atomic_load(&pq->count) == 0) {
            continue;
        }
        
        QueueNode* head = atomic_load(&pq->head);
        QueueNode* next = atomic_load(&head->next);
        
        if (next != NULL) {
            return next->message;
        }
    }
    
    return NULL;
}

SphereMessage* message_queue_dequeue_timeout(LockFreeMessageQueue* queue,
                                             uint64_t timeout_ns) {
    if (!queue) return NULL;
    
    uint64_t start_time = get_time_ns();
    uint64_t elapsed = 0;
    
    while (elapsed < timeout_ns) {
        SphereMessage* message = message_queue_dequeue(queue);
        if (message != NULL) {
            return message;
        }
        
        // Small delay to avoid busy-waiting
        struct timespec ts = {0, 1000}; // 1 microsecond
        nanosleep(&ts, NULL);
        
        elapsed = get_time_ns() - start_time;
    }
    
    return NULL;
}

SphereMessage* message_queue_dequeue_type(LockFreeMessageQueue* queue,
                                         MessageType type) {
    if (!queue) return NULL;
    
    // This is a simplified O(n) implementation
    // For production, consider using a hash table for type-based lookup
    
    // Try each priority level
    for (int priority = MSG_PRIORITY_CRITICAL; priority >= MSG_PRIORITY_LOW; priority--) {
        PriorityQueueHead* pq = &queue->queues[priority];
        
        QueueNode* prev = atomic_load(&pq->head);
        QueueNode* curr = atomic_load(&prev->next);
        
        while (curr != NULL) {
            if (curr->message && curr->message->type == type) {
                // Found matching message
                // Try to remove it (simplified, not fully lock-free)
                QueueNode* next = atomic_load(&curr->next);
                if (atomic_compare_exchange_strong(&prev->next, &curr, next)) {
                    atomic_fetch_sub(&pq->count, 1);
                    atomic_fetch_add(&queue->total_dequeued, 1);
                    
                    SphereMessage* message = curr->message;
                    free_node(queue, curr);
                    return message;
                }
            }
            
            prev = curr;
            curr = atomic_load(&curr->next);
        }
    }
    
    return NULL;
}

SphereMessage* message_queue_dequeue_for_receiver(LockFreeMessageQueue* queue,
                                                  int receiver_id) {
    if (!queue) return NULL;
    
    // Similar to dequeue_type, O(n) implementation
    for (int priority = MSG_PRIORITY_CRITICAL; priority >= MSG_PRIORITY_LOW; priority--) {
        PriorityQueueHead* pq = &queue->queues[priority];
        
        QueueNode* prev = atomic_load(&pq->head);
        QueueNode* curr = atomic_load(&prev->next);
        
        while (curr != NULL) {
            if (curr->message && 
                (curr->message->receiver_id == receiver_id || 
                 curr->message->receiver_id == -1)) {
                // Found matching message
                QueueNode* next = atomic_load(&curr->next);
                if (atomic_compare_exchange_strong(&prev->next, &curr, next)) {
                    atomic_fetch_sub(&pq->count, 1);
                    atomic_fetch_add(&queue->total_dequeued, 1);
                    
                    SphereMessage* message = curr->message;
                    free_node(queue, curr);
                    return message;
                }
            }
            
            prev = curr;
            curr = atomic_load(&curr->next);
        }
    }
    
    return NULL;
}

// ============================================================================
// QUEUE QUERIES
// ============================================================================

bool message_queue_is_empty(const LockFreeMessageQueue* queue) {
    if (!queue) return true;
    
    for (int i = 0; i < 4; i++) {
        if (atomic_load(&queue->queues[i].count) > 0) {
            return false;
        }
    }
    
    return true;
}

uint64_t message_queue_size(const LockFreeMessageQueue* queue) {
    if (!queue) return 0;
    
    uint64_t total = 0;
    for (int i = 0; i < 4; i++) {
        total += atomic_load(&queue->queues[i].count);
    }
    
    return total;
}

uint64_t message_queue_size_priority(const LockFreeMessageQueue* queue,
                                     MessagePriority priority) {
    if (!queue || priority < MSG_PRIORITY_LOW || priority > MSG_PRIORITY_CRITICAL) {
        return 0;
    }
    
    return atomic_load(&queue->queues[priority].count);
}

bool message_queue_is_full(const LockFreeMessageQueue* queue) {
    if (!queue || queue->max_queue_size == 0) {
        return false;
    }
    
    return message_queue_size(queue) >= queue->max_queue_size;
}

double message_queue_utilization(const LockFreeMessageQueue* queue) {
    if (!queue) return 0.0;
    
    if (queue->max_queue_size == 0) {
        return -1.0; // Unlimited
    }
    
    uint64_t size = message_queue_size(queue);
    return (double)size / (double)queue->max_queue_size;
}

// ============================================================================
// QUEUE STATISTICS
// ============================================================================

void message_queue_get_statistics(const LockFreeMessageQueue* queue,
                                  MessageQueueStatistics* stats) {
    if (!queue || !stats) return;
    
    stats->total_enqueued = atomic_load(&queue->total_enqueued);
    stats->total_dequeued = atomic_load(&queue->total_dequeued);
    stats->total_dropped = atomic_load(&queue->total_dropped);
    stats->enqueue_failures = atomic_load(&queue->enqueue_failures);
    stats->dequeue_failures = atomic_load(&queue->dequeue_failures);
    stats->current_size = message_queue_size(queue);
    
    for (int i = 0; i < 4; i++) {
        stats->size_by_priority[i] = atomic_load(&queue->queues[i].count);
    }
    
    stats->utilization = message_queue_utilization(queue);
}

void message_queue_print_statistics(const LockFreeMessageQueue* queue) {
    if (!queue) return;
    
    MessageQueueStatistics stats;
    message_queue_get_statistics(queue, &stats);
    
    printf("\n=== Message Queue Statistics ===\n");
    printf("Total Enqueued: %lu\n", (unsigned long)stats.total_enqueued);
    printf("Total Dequeued: %lu\n", (unsigned long)stats.total_dequeued);
    printf("Total Dropped: %lu\n", (unsigned long)stats.total_dropped);
    printf("Enqueue Failures: %lu\n", (unsigned long)stats.enqueue_failures);
    printf("Dequeue Failures: %lu\n", (unsigned long)stats.dequeue_failures);
    printf("Current Size: %lu\n", (unsigned long)stats.current_size);
    printf("\nSize by Priority:\n");
    printf("  CRITICAL: %lu\n", (unsigned long)stats.size_by_priority[MSG_PRIORITY_CRITICAL]);
    printf("  HIGH:     %lu\n", (unsigned long)stats.size_by_priority[MSG_PRIORITY_HIGH]);
    printf("  NORMAL:   %lu\n", (unsigned long)stats.size_by_priority[MSG_PRIORITY_NORMAL]);
    printf("  LOW:      %lu\n", (unsigned long)stats.size_by_priority[MSG_PRIORITY_LOW]);
    
    if (stats.utilization >= 0.0) {
        printf("Utilization: %.2f%%\n", stats.utilization * 100.0);
    } else {
        printf("Utilization: Unlimited\n");
    }
    printf("================================\n\n");
}

void message_queue_reset_statistics(LockFreeMessageQueue* queue) {
    if (!queue) return;
    
    atomic_store(&queue->total_enqueued, 0);
    atomic_store(&queue->total_dequeued, 0);
    atomic_store(&queue->total_dropped, 0);
    atomic_store(&queue->enqueue_failures, 0);
    atomic_store(&queue->dequeue_failures, 0);
}

// ============================================================================
// QUEUE MAINTENANCE
// ============================================================================

void message_queue_clear(LockFreeMessageQueue* queue, bool free_messages) {
    if (!queue) return;
    
    for (int priority = 0; priority < 4; priority++) {
        PriorityQueueHead* pq = &queue->queues[priority];
        
        QueueNode* head = atomic_load(&pq->head);
        QueueNode* curr = atomic_load(&head->next);
        
        while (curr != NULL) {
            QueueNode* next = atomic_load(&curr->next);
            
            if (free_messages && curr->message) {
                sphere_message_free(curr->message);
            }
            
            free(curr);
            curr = next;
        }
        
        // Reset to dummy head
        atomic_store(&head->next, NULL);
        atomic_store(&pq->tail, head);
        atomic_store(&pq->count, 0);
    }
}

uint64_t message_queue_compact(LockFreeMessageQueue* queue) {
    if (!queue) return 0;
    
    uint64_t removed = 0;
    
    for (int priority = 0; priority < 4; priority++) {
        PriorityQueueHead* pq = &queue->queues[priority];
        
        QueueNode* prev = atomic_load(&pq->head);
        QueueNode* curr = atomic_load(&prev->next);
        
        while (curr != NULL) {
            QueueNode* next = atomic_load(&curr->next);
            
            if (curr->message && sphere_message_is_processed(curr->message)) {
                // Remove processed message
                atomic_store(&prev->next, next);
                sphere_message_free(curr->message);
                free_node(queue, curr);
                atomic_fetch_sub(&pq->count, 1);
                removed++;
            } else {
                prev = curr;
            }
            
            curr = next;
        }
    }
    
    return removed;
}

bool message_queue_validate(const LockFreeMessageQueue* queue) {
    if (!queue) return false;
    
    // Check each priority queue
    for (int priority = 0; priority < 4; priority++) {
        const PriorityQueueHead* pq = &queue->queues[priority];
        
        QueueNode* head = atomic_load(&pq->head);
        if (!head) return false;
        
        // Count nodes
        uint64_t count = 0;
        QueueNode* curr = atomic_load(&head->next);
        
        while (curr != NULL) {
            count++;
            
            // Check for cycles (simple check)
            if (count > atomic_load(&pq->count) + 100) {
                return false; // Likely a cycle
            }
            
            curr = atomic_load(&curr->next);
        }
        
        // Verify count matches
        if (count != atomic_load(&pq->count)) {
            return false;
        }
    }
    
    return true;
}

// ============================================================================
// MEMORY POOL OPERATIONS
// ============================================================================

uint64_t message_queue_preallocate_nodes(LockFreeMessageQueue* queue,
                                        uint64_t count) {
    if (!queue) return 0;
    
    uint64_t allocated = 0;
    
    for (uint64_t i = 0; i < count; i++) {
        QueueNode* node = aligned_alloc(64, sizeof(QueueNode));
        if (!node) break;
        
        // Add to free list
        QueueNode* free_head = atomic_load(&queue->free_list);
        do {
            atomic_store(&node->next, free_head);
        } while (!atomic_compare_exchange_weak(&queue->free_list, &free_head, node));
        
        atomic_fetch_add(&queue->free_count, 1);
        allocated++;
    }
    
    return allocated;
}

uint64_t message_queue_trim_free_nodes(LockFreeMessageQueue* queue,
                                       uint64_t target_count) {
    if (!queue) return 0;
    
    uint64_t freed = 0;
    uint64_t current_count = atomic_load(&queue->free_count);
    
    while (current_count > target_count) {
        QueueNode* free_head = atomic_load(&queue->free_list);
        if (!free_head) break;
        
        QueueNode* next = atomic_load(&free_head->next);
        if (atomic_compare_exchange_weak(&queue->free_list, &free_head, next)) {
            free(free_head);
            atomic_fetch_sub(&queue->free_count, 1);
            freed++;
            current_count--;
        }
    }
    
    return freed;
}

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

uint64_t message_queue_enqueue_batch(LockFreeMessageQueue* queue,
                                     SphereMessage** messages,
                                     uint64_t count) {
    if (!queue || !messages) return 0;
    
    uint64_t enqueued = 0;
    
    for (uint64_t i = 0; i < count; i++) {
        if (message_queue_enqueue(queue, messages[i])) {
            enqueued++;
        }
    }
    
    return enqueued;
}

uint64_t message_queue_dequeue_batch(LockFreeMessageQueue* queue,
                                     SphereMessage** messages,
                                     uint64_t max_count) {
    if (!queue || !messages) return 0;
    
    uint64_t dequeued = 0;
    
    for (uint64_t i = 0; i < max_count; i++) {
        SphereMessage* message = message_queue_dequeue(queue);
        if (!message) break;
        
        messages[i] = message;
        dequeued++;
    }
    
    return dequeued;
}
