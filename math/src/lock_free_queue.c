/**
 * Generic Lock-Free Queue - Implementation
 * 
 * Based on Michael-Scott lock-free queue algorithm.
 * Adapted from CLLM message queue for generic use.
 */

#include "lock_free_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// INTERNAL HELPER FUNCTIONS
// ============================================================================

/**
 * Get current time in nanoseconds
 */
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * Allocate a new queue node
 */
static LockFreeQueueNode* allocate_node(LockFreeQueue* queue, void* data) {
    LockFreeQueueNode* node = NULL;
    
    // Try to get node from free list first
    if (queue->max_free_nodes > 0) {
        LockFreeQueueNode* free_head = atomic_load(&queue->free_list);
        while (free_head != NULL) {
            LockFreeQueueNode* next = atomic_load(&free_head->next);
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
        node = aligned_alloc(CACHE_LINE_SIZE, sizeof(LockFreeQueueNode));
        if (!node) {
            return NULL;
        }
    }
    
    // Initialize node
    node->data = data;
    atomic_init(&node->next, NULL);
    node->sequence = atomic_fetch_add(&queue->sequence_counter, 1);
    
    return node;
}

/**
 * Free a queue node (return to free list or deallocate)
 */
static void free_node(LockFreeQueue* queue, LockFreeQueueNode* node) {
    if (!node) return;
    
    // Try to return to free list if not full
    if (queue->max_free_nodes > 0 && 
        atomic_load(&queue->free_count) < queue->max_free_nodes) {
        
        LockFreeQueueNode* free_head = atomic_load(&queue->free_list);
        do {
            atomic_store(&node->next, free_head);
        } while (!atomic_compare_exchange_weak(&queue->free_list, &free_head, node));
        
        atomic_fetch_add(&queue->free_count, 1);
    } else {
        // Free list full or disabled, deallocate
        free(node);
    }
}

// ============================================================================
// QUEUE OPERATIONS
// ============================================================================

LockFreeQueue* lock_free_queue_create(uint64_t max_queue_size, bool drop_on_full) {
    LockFreeQueue* queue = aligned_alloc(CACHE_LINE_SIZE, sizeof(LockFreeQueue));
    if (!queue) {
        return NULL;
    }
    
    // Zero out structure
    memset(queue, 0, sizeof(LockFreeQueue));
    
    // Create dummy head node
    LockFreeQueueNode* dummy = aligned_alloc(CACHE_LINE_SIZE, sizeof(LockFreeQueueNode));
    if (!dummy) {
        free(queue);
        return NULL;
    }
    
    dummy->data = NULL;
    atomic_init(&dummy->next, NULL);
    dummy->sequence = 0;
    
    // Initialize head and tail to dummy node
    atomic_init(&queue->head, dummy);
    atomic_init(&queue->tail, dummy);
    
    // Initialize statistics
    atomic_init(&queue->total_enqueued, 0);
    atomic_init(&queue->total_dequeued, 0);
    atomic_init(&queue->enqueue_failures, 0);
    atomic_init(&queue->dequeue_failures, 0);
    atomic_init(&queue->current_size, 0);
    
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

void lock_free_queue_free(LockFreeQueue* queue, void (*free_data_fn)(void*)) {
    if (!queue) return;
    
    // Clear all items
    lock_free_queue_clear(queue, free_data_fn);
    
    // Free dummy head node
    LockFreeQueueNode* head = atomic_load(&queue->head);
    free(head);
    
    // Free all nodes in free list
    LockFreeQueueNode* free_node = atomic_load(&queue->free_list);
    while (free_node != NULL) {
        LockFreeQueueNode* next = atomic_load(&free_node->next);
        free(free_node);
        free_node = next;
    }
    
    free(queue);
}

bool lock_free_queue_enqueue(LockFreeQueue* queue, void* data) {
    if (!queue || !data) {
        if (queue) atomic_fetch_add(&queue->enqueue_failures, 1);
        return false;
    }
    
    // Check if queue is full
    if (queue->max_queue_size > 0) {
        uint64_t current_size = atomic_load(&queue->current_size);
        if (current_size >= queue->max_queue_size) {
            if (queue->drop_on_full) {
                // Drop the item (caller is responsible for freeing if needed)
                return false;
            } else {
                atomic_fetch_add(&queue->enqueue_failures, 1);
                return false;
            }
        }
    }
    
    // Allocate node
    LockFreeQueueNode* node = allocate_node(queue, data);
    if (!node) {
        atomic_fetch_add(&queue->enqueue_failures, 1);
        return false;
    }
    
    // Enqueue using Michael-Scott lock-free algorithm
    while (true) {
        LockFreeQueueNode* tail = atomic_load(&queue->tail);
        LockFreeQueueNode* next = atomic_load(&tail->next);
        
        // Check if tail is still the last node
        if (tail == atomic_load(&queue->tail)) {
            if (next == NULL) {
                // Try to link new node at the end
                if (atomic_compare_exchange_weak(&tail->next, &next, node)) {
                    // Successfully enqueued, try to swing tail
                    atomic_compare_exchange_weak(&queue->tail, &tail, node);
                    atomic_fetch_add(&queue->current_size, 1);
                    atomic_fetch_add(&queue->total_enqueued, 1);
                    return true;
                }
            } else {
                // Tail is lagging, try to advance it
                atomic_compare_exchange_weak(&queue->tail, &tail, next);
            }
        }
    }
}

void* lock_free_queue_dequeue(LockFreeQueue* queue) {
    if (!queue) return NULL;
    
    // Dequeue using Michael-Scott lock-free algorithm
    while (true) {
        LockFreeQueueNode* head = atomic_load(&queue->head);
        LockFreeQueueNode* tail = atomic_load(&queue->tail);
        LockFreeQueueNode* next = atomic_load(&head->next);
        
        // Check if head is still the first node
        if (head == atomic_load(&queue->head)) {
            if (head == tail) {
                // Queue is empty or tail is lagging
                if (next == NULL) {
                    // Queue is empty
                    atomic_fetch_add(&queue->dequeue_failures, 1);
                    return NULL;
                }
                // Tail is lagging, try to advance it
                atomic_compare_exchange_weak(&queue->tail, &tail, next);
            } else {
                // Try to dequeue
                void* data = next->data;
                if (atomic_compare_exchange_weak(&queue->head, &head, next)) {
                    // Successfully dequeued
                    atomic_fetch_sub(&queue->current_size, 1);
                    atomic_fetch_add(&queue->total_dequeued, 1);
                    
                    // Free old head node
                    free_node(queue, head);
                    
                    return data;
                }
            }
        }
    }
}

void* lock_free_queue_peek(const LockFreeQueue* queue) {
    if (!queue) return NULL;
    
    LockFreeQueueNode* head = atomic_load(&queue->head);
    LockFreeQueueNode* next = atomic_load(&head->next);
    
    if (next != NULL) {
        return next->data;
    }
    
    return NULL;
}

void* lock_free_queue_dequeue_timeout(LockFreeQueue* queue, uint64_t timeout_ns) {
    if (!queue) return NULL;
    
    uint64_t start_time = get_time_ns();
    
    while (true) {
        void* data = lock_free_queue_dequeue(queue);
        if (data != NULL) {
            return data;
        }
        
        // Check timeout
        uint64_t elapsed = get_time_ns() - start_time;
        if (elapsed >= timeout_ns) {
            return NULL;
        }
        
        // Small delay to reduce CPU usage
        // (In production, you might want to use a more sophisticated backoff strategy)
        for (volatile int i = 0; i < 100; i++);
    }
}

// ============================================================================
// QUEUE QUERIES
// ============================================================================

bool lock_free_queue_is_empty(const LockFreeQueue* queue) {
    if (!queue) return true;
    
    LockFreeQueueNode* head = atomic_load(&queue->head);
    LockFreeQueueNode* next = atomic_load(&head->next);
    
    return (next == NULL);
}

uint64_t lock_free_queue_size(const LockFreeQueue* queue) {
    if (!queue) return 0;
    return atomic_load(&queue->current_size);
}

bool lock_free_queue_is_full(const LockFreeQueue* queue) {
    if (!queue) return false;
    if (queue->max_queue_size == 0) return false; // Unlimited
    
    return atomic_load(&queue->current_size) >= queue->max_queue_size;
}

double lock_free_queue_utilization(const LockFreeQueue* queue) {
    if (!queue) return 0.0;
    if (queue->max_queue_size == 0) return -1.0; // Unlimited
    
    uint64_t size = atomic_load(&queue->current_size);
    return (double)size / (double)queue->max_queue_size;
}

// ============================================================================
// QUEUE STATISTICS
// ============================================================================

void lock_free_queue_get_statistics(const LockFreeQueue* queue,
                                   LockFreeQueueStatistics* stats) {
    if (!queue || !stats) return;
    
    stats->total_enqueued = atomic_load(&queue->total_enqueued);
    stats->total_dequeued = atomic_load(&queue->total_dequeued);
    stats->enqueue_failures = atomic_load(&queue->enqueue_failures);
    stats->dequeue_failures = atomic_load(&queue->dequeue_failures);
    stats->current_size = atomic_load(&queue->current_size);
    stats->utilization = lock_free_queue_utilization(queue);
}

void lock_free_queue_print_statistics(const LockFreeQueue* queue, const char* name) {
    if (!queue) return;
    
    LockFreeQueueStatistics stats;
    lock_free_queue_get_statistics(queue, &stats);
    
    printf("\n=== Lock-Free Queue Statistics");
    if (name) printf(": %s", name);
    printf(" ===\n");
    
    printf("Total Enqueued:     %lu\n", stats.total_enqueued);
    printf("Total Dequeued:     %lu\n", stats.total_dequeued);
    printf("Enqueue Failures:   %lu\n", stats.enqueue_failures);
    printf("Dequeue Failures:   %lu\n", stats.dequeue_failures);
    printf("Current Size:       %lu\n", stats.current_size);
    
    if (stats.utilization >= 0.0) {
        printf("Utilization:        %.2f%%\n", stats.utilization * 100.0);
    } else {
        printf("Utilization:        N/A (unlimited)\n");
    }
    
    printf("\n");
}

void lock_free_queue_reset_statistics(LockFreeQueue* queue) {
    if (!queue) return;
    
    atomic_store(&queue->total_enqueued, 0);
    atomic_store(&queue->total_dequeued, 0);
    atomic_store(&queue->enqueue_failures, 0);
    atomic_store(&queue->dequeue_failures, 0);
}

// ============================================================================
// QUEUE MAINTENANCE
// ============================================================================

void lock_free_queue_clear(LockFreeQueue* queue, void (*free_data_fn)(void*)) {
    if (!queue) return;
    
    // Dequeue all items
    while (true) {
        void* data = lock_free_queue_dequeue(queue);
        if (data == NULL) break;
        
        if (free_data_fn) {
            free_data_fn(data);
        }
    }
}

bool lock_free_queue_validate(const LockFreeQueue* queue) {
    if (!queue) return false;
    
    // Basic validation checks
    LockFreeQueueNode* head = atomic_load(&queue->head);
    LockFreeQueueNode* tail = atomic_load(&queue->tail);
    
    if (!head || !tail) return false;
    
    // Head should never be NULL
    if (head == NULL) return false;
    
    // Tail should be reachable from head
    LockFreeQueueNode* current = head;
    int max_iterations = 1000000; // Prevent infinite loop
    while (current != tail && max_iterations-- > 0) {
        LockFreeQueueNode* next = atomic_load(&current->next);
        if (next == NULL) break;
        current = next;
    }
    
    if (max_iterations <= 0) return false; // Possible corruption
    
    return true;
}

// ============================================================================
// MEMORY POOL OPERATIONS
// ============================================================================

uint64_t lock_free_queue_preallocate_nodes(LockFreeQueue* queue, uint64_t count) {
    if (!queue) return 0;
    
    uint64_t allocated = 0;
    
    for (uint64_t i = 0; i < count; i++) {
        LockFreeQueueNode* node = aligned_alloc(CACHE_LINE_SIZE, sizeof(LockFreeQueueNode));
        if (!node) break;
        
        // Add to free list
        LockFreeQueueNode* free_head = atomic_load(&queue->free_list);
        do {
            atomic_store(&node->next, free_head);
        } while (!atomic_compare_exchange_weak(&queue->free_list, &free_head, node));
        
        atomic_fetch_add(&queue->free_count, 1);
        allocated++;
    }
    
    return allocated;
}

uint64_t lock_free_queue_trim_free_nodes(LockFreeQueue* queue, uint64_t target_count) {
    if (!queue) return 0;
    
    uint64_t freed = 0;
    uint64_t current_count = atomic_load(&queue->free_count);
    
    while (current_count > target_count) {
        LockFreeQueueNode* free_head = atomic_load(&queue->free_list);
        if (free_head == NULL) break;
        
        LockFreeQueueNode* next = atomic_load(&free_head->next);
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

uint64_t lock_free_queue_enqueue_batch(LockFreeQueue* queue,
                                      void** items,
                                      uint64_t count) {
    if (!queue || !items) return 0;
    
    uint64_t enqueued = 0;
    
    for (uint64_t i = 0; i < count; i++) {
        if (lock_free_queue_enqueue(queue, items[i])) {
            enqueued++;
        } else {
            break; // Stop on first failure
        }
    }
    
    return enqueued;
}

uint64_t lock_free_queue_dequeue_batch(LockFreeQueue* queue,
                                      void** items,
                                      uint64_t max_count) {
    if (!queue || !items) return 0;
    
    uint64_t dequeued = 0;
    
    for (uint64_t i = 0; i < max_count; i++) {
        void* data = lock_free_queue_dequeue(queue);
        if (data == NULL) break;
        
        items[i] = data;
        dequeued++;
    }
    
    return dequeued;
}
