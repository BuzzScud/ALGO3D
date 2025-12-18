/**
 * @file message_passing.c
 * @brief Implementation of Generic Lock-Free Message Passing System
 */

#include "message_passing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdatomic.h>

// ============================================================================
// ATOMIC OPERATIONS
// ============================================================================

// Compare-and-swap for pointers
static inline bool cas_ptr(void** ptr, void* expected, void* new_val) {
    return atomic_compare_exchange_strong((_Atomic(void*)*)ptr, &expected, new_val);
}

// Atomic increment
static inline uint64_t atomic_inc(uint64_t* ptr) {
    return atomic_fetch_add((_Atomic uint64_t*)ptr, 1);
}

// Atomic load
static inline uint64_t atomic_load_u64(uint64_t* ptr) {
    return atomic_load((_Atomic uint64_t*)ptr);
}

// ============================================================================
// TIMESTAMP UTILITIES
// ============================================================================

uint64_t message_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

bool message_is_expired(Message* msg) {
    if (msg->deadline == 0) return false;
    return message_get_timestamp() > msg->deadline;
}

// ============================================================================
// MESSAGE POOL IMPLEMENTATION
// ============================================================================

static MessagePool* message_pool_create(uint32_t pool_size) {
    MessagePool* pool = (MessagePool*)calloc(1, sizeof(MessagePool));
    if (!pool) return NULL;
    
    pool->pool_size = pool_size;
    pool->free_list = NULL;
    
    // Pre-allocate messages
    for (uint32_t i = 0; i < pool_size; i++) {
        Message* msg = (Message*)calloc(1, sizeof(Message));
        if (msg) {
            msg->next = pool->free_list;
            pool->free_list = msg;
        }
    }
    
    return pool;
}

static void message_pool_destroy(MessagePool* pool) {
    if (!pool) return;
    
    Message* current = pool->free_list;
    while (current) {
        Message* next = current->next;
        free(current);
        current = next;
    }
    
    free(pool);
}

Message* message_alloc(MessageSystem* system) {
    if (!system || !system->pool) return NULL;
    
    MessagePool* pool = system->pool;
    Message* msg = pool->free_list;
    
    if (msg) {
        pool->free_list = msg->next;
        atomic_inc(&pool->allocated);
        
        uint64_t current = atomic_load_u64(&pool->allocated) - atomic_load_u64(&pool->freed);
        if (current > pool->peak_usage) {
            pool->peak_usage = current;
        }
    } else {
        // Pool exhausted, allocate new message
        msg = (Message*)calloc(1, sizeof(Message));
    }
    
    if (msg) {
        memset(msg, 0, sizeof(Message));
        // Increment first, then assign (so ID starts at 1, not 0)
        uint64_t new_id = atomic_fetch_add((_Atomic uint64_t*)&system->total_messages, 1) + 1;
        msg->id = new_id;
        msg->timestamp = message_get_timestamp();
    }
    
    return msg;
}

void message_free(MessageSystem* system, Message* msg) {
    if (!system || !msg) return;
    
    // Free custom data if needed
    if (msg->data && msg->free_fn) {
        msg->free_fn(msg->data);
    } else if (msg->data && !(msg->flags & MSG_FLAG_NO_COPY)) {
        free(msg->data);
    }
    
    // Return to pool
    MessagePool* pool = system->pool;
    msg->next = pool->free_list;
    pool->free_list = msg;
    atomic_inc(&pool->freed);
}

// ============================================================================
// MESSAGE QUEUE IMPLEMENTATION
// ============================================================================

void message_queue_init(MessageQueue* queue) {
    if (!queue) return;
    
    // Create sentinel node
    MessageQueueNode* sentinel = (MessageQueueNode*)calloc(1, sizeof(MessageQueueNode));
    sentinel->message = NULL;
    sentinel->next = NULL;
    sentinel->version = 0;
    
    queue->head = sentinel;
    queue->tail = sentinel;
    queue->version_counter = 1;
    queue->enqueue_count = 0;
    queue->dequeue_count = 0;
    queue->current_size = 0;
    queue->peak_size = 0;
}

bool message_queue_enqueue(MessageQueue* queue, Message* msg) {
    if (!queue || !msg) return false;
    
    // Create new node
    MessageQueueNode* node = (MessageQueueNode*)calloc(1, sizeof(MessageQueueNode));
    if (!node) return false;
    
    node->message = msg;
    node->next = NULL;
    node->version = atomic_inc(&queue->version_counter);
    
    // Lock-free enqueue
    while (true) {
        MessageQueueNode* tail = queue->tail;
        MessageQueueNode* next = tail->next;
        
        if (tail == queue->tail) {
            if (next == NULL) {
                // Try to link node at the end
                if (cas_ptr((void**)&tail->next, NULL, node)) {
                    // Success, swing tail
                    cas_ptr((void**)&queue->tail, tail, node);
                    atomic_inc(&queue->enqueue_count);
                    atomic_inc(&queue->current_size);
                    
                    uint64_t size = atomic_load_u64(&queue->current_size);
                    if (size > queue->peak_size) {
                        queue->peak_size = size;
                    }
                    return true;
                }
            } else {
                // Tail was not pointing to last node, help advance it
                cas_ptr((void**)&queue->tail, tail, next);
            }
        }
    }
}

Message* message_queue_dequeue(MessageQueue* queue) {
    if (!queue) return NULL;
    
    while (true) {
        MessageQueueNode* head = queue->head;
        MessageQueueNode* tail = queue->tail;
        MessageQueueNode* next = head->next;
        
        if (head == queue->head) {
            if (head == tail) {
                if (next == NULL) {
                    // Queue is empty
                    return NULL;
                }
                // Tail is falling behind, help advance it
                cas_ptr((void**)&queue->tail, tail, next);
            } else {
                // Read message before CAS
                Message* msg = next->message;
                
                // Try to swing head to next node
                if (cas_ptr((void**)&queue->head, head, next)) {
                    atomic_inc(&queue->dequeue_count);
                    
                    uint64_t size = atomic_load_u64(&queue->current_size);
                    if (size > 0) {
                        atomic_fetch_sub((_Atomic uint64_t*)&queue->current_size, 1);
                    }
                    
                    free(head);  // Free old sentinel
                    return msg;
                }
            }
        }
    }
}

bool message_queue_is_empty(MessageQueue* queue) {
    if (!queue) return true;
    return queue->head->next == NULL;
}

uint64_t message_queue_size(MessageQueue* queue) {
    if (!queue) return 0;
    return atomic_load_u64(&queue->current_size);
}

// ============================================================================
// PRIORITY QUEUE IMPLEMENTATION
// ============================================================================

void priority_queue_init(PriorityMessageQueue* pqueue) {
    if (!pqueue) return;
    
    for (int i = 0; i < 4; i++) {
        message_queue_init(&pqueue->queues[i]);
    }
    pqueue->total_messages = 0;
}

bool priority_queue_enqueue(PriorityMessageQueue* pqueue, Message* msg) {
    if (!pqueue || !msg) return false;
    
    MessagePriority priority = msg->priority;
    if (priority < 0 || priority > 3) {
        priority = MSG_PRIORITY_NORMAL;
    }
    
    bool result = message_queue_enqueue(&pqueue->queues[priority], msg);
    if (result) {
        atomic_inc(&pqueue->total_messages);
    }
    return result;
}

Message* priority_queue_dequeue(PriorityMessageQueue* pqueue) {
    if (!pqueue) return NULL;
    
    // Try each priority level from highest to lowest
    for (int i = 0; i < 4; i++) {
        Message* msg = message_queue_dequeue(&pqueue->queues[i]);
        if (msg) {
            uint64_t total = atomic_load_u64(&pqueue->total_messages);
            if (total > 0) {
                atomic_fetch_sub((_Atomic uint64_t*)&pqueue->total_messages, 1);
            }
            return msg;
        }
    }
    
    return NULL;
}

// ============================================================================
// MESSAGE OPERATIONS
// ============================================================================

Message* message_create(
    MessageSystem* system,
    MessageType type,
    MessagePriority priority,
    uint32_t sender_id,
    uint32_t receiver_id,
    void* data,
    size_t data_size
) {
    Message* msg = message_alloc(system);
    if (!msg) return NULL;
    
    msg->type = type;
    msg->priority = priority;
    msg->sender_id = sender_id;
    msg->receiver_id = receiver_id;
    msg->data = data;
    msg->data_size = data_size;
    
    return msg;
}

// ============================================================================
// CHANNEL OPERATIONS
// ============================================================================

MessageChannel* message_channel_create(MessageSystem* system, uint32_t thread_a, uint32_t thread_b) {
    if (!system) return NULL;
    
    MessageChannel* channel = (MessageChannel*)calloc(1, sizeof(MessageChannel));
    if (!channel) return NULL;
    
    channel->id = system->num_channels++;
    channel->thread_a = thread_a;
    channel->thread_b = thread_b;
    
    priority_queue_init(&channel->a_to_b);
    priority_queue_init(&channel->b_to_a);
    
    return channel;
}

void message_channel_destroy(MessageChannel* channel) {
    if (!channel) return;
    
    // TODO: Drain queues and free messages
    
    free(channel);
}

bool message_send(MessageChannel* channel, Message* msg) {
    if (!channel || !msg) return false;
    
    // Determine which queue to use
    PriorityMessageQueue* queue;
    if (msg->sender_id == channel->thread_a) {
        queue = &channel->a_to_b;
    } else if (msg->sender_id == channel->thread_b) {
        queue = &channel->b_to_a;
    } else {
        return false;  // Invalid sender
    }
    
    bool result = priority_queue_enqueue(queue, msg);
    if (result) {
        atomic_inc(&channel->messages_sent);
        atomic_fetch_add((_Atomic uint64_t*)&channel->bytes_transferred, msg->data_size);
    }
    
    return result;
}

Message* message_receive(MessageChannel* channel, uint32_t thread_id) {
    if (!channel) return NULL;
    
    // Determine which queue to read from
    PriorityMessageQueue* queue;
    if (thread_id == channel->thread_a) {
        queue = &channel->b_to_a;
    } else if (thread_id == channel->thread_b) {
        queue = &channel->a_to_b;
    } else {
        return NULL;  // Invalid receiver
    }
    
    Message* msg = priority_queue_dequeue(queue);
    if (msg) {
        atomic_inc(&channel->messages_received);
    }
    
    return msg;
}

Message* message_receive_timeout(MessageChannel* channel, uint32_t thread_id, uint64_t timeout_ns) {
    if (!channel) return NULL;
    
    uint64_t start = message_get_timestamp();
    uint64_t deadline = start + timeout_ns;
    
    while (message_get_timestamp() < deadline) {
        Message* msg = message_receive(channel, thread_id);
        if (msg) return msg;
        
        // Small sleep to avoid busy-waiting
        struct timespec ts = {0, 1000};  // 1 microsecond
        nanosleep(&ts, NULL);
    }
    
    return NULL;  // Timeout
}

bool message_send_batch(MessageChannel* channel, Message** messages, uint32_t count) {
    if (!channel || !messages) return false;
    
    for (uint32_t i = 0; i < count; i++) {
        if (!message_send(channel, messages[i])) {
            return false;
        }
    }
    
    return true;
}

uint32_t message_receive_batch(MessageChannel* channel, uint32_t thread_id, Message** out_messages, uint32_t max_count) {
    if (!channel || !out_messages) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < max_count; i++) {
        Message* msg = message_receive(channel, thread_id);
        if (!msg) break;
        
        out_messages[count++] = msg;
    }
    
    return count;
}

// ============================================================================
// SYSTEM OPERATIONS
// ============================================================================

MessageSystem* message_system_create(uint32_t max_channels, uint32_t pool_size) {
    MessageSystem* system = (MessageSystem*)calloc(1, sizeof(MessageSystem));
    if (!system) return NULL;
    
    system->pool = message_pool_create(pool_size);
    if (!system->pool) {
        free(system);
        return NULL;
    }
    
    system->channels = (MessageChannel**)calloc(max_channels, sizeof(MessageChannel*));
    if (!system->channels) {
        message_pool_destroy(system->pool);
        free(system);
        return NULL;
    }
    
    system->max_channels = max_channels;
    system->num_channels = 0;
    
    return system;
}

void message_system_destroy(MessageSystem* system) {
    if (!system) return;
    
    // Destroy all channels
    for (uint32_t i = 0; i < system->num_channels; i++) {
        message_channel_destroy(system->channels[i]);
    }
    
    free(system->channels);
    message_pool_destroy(system->pool);
    free(system);
}

bool message_broadcast(MessageSystem* system, Message* msg) {
    if (!system || !msg) return false;
    
    // TODO: Implement broadcast to all channels
    
    return true;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* message_type_name(MessageType type) {
    switch (type) {
        case MSG_TYPE_INIT: return "INIT";
        case MSG_TYPE_SHUTDOWN: return "SHUTDOWN";
        case MSG_TYPE_PAUSE: return "PAUSE";
        case MSG_TYPE_RESUME: return "RESUME";
        case MSG_TYPE_RESET: return "RESET";
        case MSG_TYPE_WORK_REQUEST: return "WORK_REQUEST";
        case MSG_TYPE_WORK_ASSIGN: return "WORK_ASSIGN";
        case MSG_TYPE_WORK_COMPLETE: return "WORK_COMPLETE";
        case MSG_TYPE_WORK_STEAL: return "WORK_STEAL";
        case MSG_TYPE_WORK_REJECT: return "WORK_REJECT";
        case MSG_TYPE_BARRIER_WAIT: return "BARRIER_WAIT";
        case MSG_TYPE_BARRIER_RELEASE: return "BARRIER_RELEASE";
        case MSG_TYPE_DATA_SEND: return "DATA_SEND";
        case MSG_TYPE_DATA_RECEIVE: return "DATA_RECEIVE";
        case MSG_TYPE_STATE_CHANGE: return "STATE_CHANGE";
        case MSG_TYPE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char* message_priority_name(MessagePriority priority) {
    switch (priority) {
        case MSG_PRIORITY_CRITICAL: return "CRITICAL";
        case MSG_PRIORITY_HIGH: return "HIGH";
        case MSG_PRIORITY_NORMAL: return "NORMAL";
        case MSG_PRIORITY_LOW: return "LOW";
        default: return "UNKNOWN";
    }
}

void message_print(Message* msg) {
    if (!msg) return;
    
    printf("Message ID: %lu\n", msg->id);
    printf("  Type: %s (%d)\n", message_type_name(msg->type), msg->type);
    printf("  Priority: %s\n", message_priority_name(msg->priority));
    printf("  Sender: %u -> Receiver: %u\n", msg->sender_id, msg->receiver_id);
    printf("  Data Size: %zu bytes\n", msg->data_size);
    printf("  Timestamp: %lu\n", msg->timestamp);
    if (msg->deadline > 0) {
        printf("  Deadline: %lu\n", msg->deadline);
    }
}

void message_channel_print_stats(MessageChannel* channel) {
    if (!channel) return;
    
    printf("\n=== Message Channel Statistics ===\n");
    printf("Channel ID: %u\n", channel->id);
    printf("Thread A: %u <-> Thread B: %u\n", channel->thread_a, channel->thread_b);
    printf("Messages Sent: %lu\n", channel->messages_sent);
    printf("Messages Received: %lu\n", channel->messages_received);
    printf("Bytes Transferred: %lu\n", channel->bytes_transferred);
    printf("A->B Queue: %lu messages\n", channel->a_to_b.total_messages);
    printf("B->A Queue: %lu messages\n", channel->b_to_a.total_messages);
    printf("===================================\n\n");
}

void message_system_print_stats(MessageSystem* system) {
    if (!system) return;
    
    printf("\n=== Message System Statistics ===\n");
    printf("Channels: %u / %u\n", system->num_channels, system->max_channels);
    printf("Total Messages: %lu\n", system->total_messages);
    printf("Total Bytes: %lu\n", system->total_bytes);
    printf("Dropped Messages: %lu\n", system->dropped_messages);
    
    if (system->pool) {
        printf("\nMessage Pool:\n");
        printf("  Pool Size: %lu\n", system->pool->pool_size);
        printf("  Allocated: %lu\n", system->pool->allocated);
        printf("  Freed: %lu\n", system->pool->freed);
        printf("  Peak Usage: %lu\n", system->pool->peak_usage);
    }
    printf("==================================\n\n");
}