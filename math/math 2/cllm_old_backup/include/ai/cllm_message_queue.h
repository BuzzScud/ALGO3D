#ifndef CLLM_MESSAGE_QUEUE_H
#define CLLM_MESSAGE_QUEUE_H

#include "ai/cllm_sphere_message.h"
#include <stdatomic.h>
#include <stdbool.h>

/**
 * Lock-Free Message Queue
 * 
 * High-performance, thread-safe message queue using lock-free algorithms.
 * Uses Compare-And-Swap (CAS) operations for concurrent access.
 * 
 * Design:
 * - Lock-free enqueue and dequeue operations
 * - Cache-line aligned to prevent false sharing
 * - Priority queue support (4 priority levels)
 * - FIFO ordering within each priority level
 * - ABA problem prevention using tagged pointers
 */

// ============================================================================
// QUEUE NODE STRUCTURE
// ============================================================================

/**
 * Queue Node
 * 
 * Internal node structure for the lock-free queue.
 * Cache-line aligned to prevent false sharing.
 */
typedef struct QueueNode {
    SphereMessage* message;         // Pointer to message
    _Atomic(struct QueueNode*) next; // Next node (atomic)
    uint64_t sequence;              // Sequence number for ABA prevention
    char padding[64 - sizeof(void*) - sizeof(_Atomic(void*)) - sizeof(uint64_t)];
} __attribute__((aligned(64))) QueueNode;

/**
 * Priority Queue Head
 * 
 * Separate head/tail for each priority level.
 */
typedef struct {
    _Atomic(QueueNode*) head;       // Head of queue (atomic)
    _Atomic(QueueNode*) tail;       // Tail of queue (atomic)
    atomic_uint_fast64_t count;     // Number of messages in queue
    char padding[64 - sizeof(_Atomic(void*)) * 2 - sizeof(atomic_uint_fast64_t)];
} __attribute__((aligned(64))) PriorityQueueHead;

// ============================================================================
// MESSAGE QUEUE STRUCTURE
// ============================================================================

/**
 * Lock-Free Message Queue
 * 
 * Main queue structure with priority support.
 */
typedef struct {
    // Priority queues (one per priority level)
    PriorityQueueHead queues[4];    // LOW, NORMAL, HIGH, CRITICAL
    
    // Statistics (atomic)
    atomic_uint_fast64_t total_enqueued;    // Total messages enqueued
    atomic_uint_fast64_t total_dequeued;    // Total messages dequeued
    atomic_uint_fast64_t total_dropped;     // Total messages dropped
    atomic_uint_fast64_t enqueue_failures;  // Failed enqueue attempts
    atomic_uint_fast64_t dequeue_failures;  // Failed dequeue attempts
    
    // Configuration
    uint64_t max_queue_size;        // Maximum queue size (0 = unlimited)
    bool drop_on_full;              // Drop messages when full
    
    // Sequence counter for ABA prevention
    atomic_uint_fast64_t sequence_counter;
    
    // Memory pool for nodes (optional optimization)
    _Atomic(QueueNode*) free_list;  // Free node list
    atomic_uint_fast64_t free_count; // Number of free nodes
    uint64_t max_free_nodes;        // Maximum free nodes to cache
    
} __attribute__((aligned(64))) LockFreeMessageQueue;

// ============================================================================
// QUEUE OPERATIONS
// ============================================================================

/**
 * Create a new lock-free message queue
 * 
 * @param max_queue_size Maximum queue size (0 = unlimited)
 * @param drop_on_full Drop messages when queue is full
 * @return Pointer to new queue, or NULL on failure
 */
LockFreeMessageQueue* message_queue_create(uint64_t max_queue_size,
                                           bool drop_on_full);

/**
 * Free a message queue
 * 
 * WARNING: This function is NOT thread-safe. Ensure no other threads
 * are accessing the queue before calling this function.
 * 
 * @param queue Queue to free
 */
void message_queue_free(LockFreeMessageQueue* queue);

/**
 * Enqueue a message (lock-free)
 * 
 * Thread-safe. Can be called concurrently from multiple threads.
 * 
 * @param queue Queue to enqueue to
 * @param message Message to enqueue (ownership transferred to queue)
 * @return true on success, false on failure
 */
bool message_queue_enqueue(LockFreeMessageQueue* queue,
                          SphereMessage* message);

/**
 * Dequeue a message (lock-free)
 * 
 * Thread-safe. Can be called concurrently from multiple threads.
 * Dequeues highest priority message first.
 * 
 * @param queue Queue to dequeue from
 * @return Pointer to message, or NULL if queue is empty
 */
SphereMessage* message_queue_dequeue(LockFreeMessageQueue* queue);

/**
 * Peek at next message without removing it
 * 
 * Thread-safe, but message may be dequeued by another thread
 * before you can process it.
 * 
 * @param queue Queue to peek at
 * @return Pointer to message, or NULL if queue is empty
 */
SphereMessage* message_queue_peek(const LockFreeMessageQueue* queue);

/**
 * Try to dequeue a message with timeout
 * 
 * Spins for specified duration trying to dequeue a message.
 * 
 * @param queue Queue to dequeue from
 * @param timeout_ns Timeout in nanoseconds
 * @return Pointer to message, or NULL on timeout
 */
SphereMessage* message_queue_dequeue_timeout(LockFreeMessageQueue* queue,
                                             uint64_t timeout_ns);

/**
 * Dequeue message of specific type
 * 
 * Searches queue for message of specified type and removes it.
 * WARNING: This is O(n) and may be slow for large queues.
 * 
 * @param queue Queue to search
 * @param type Message type to find
 * @return Pointer to message, or NULL if not found
 */
SphereMessage* message_queue_dequeue_type(LockFreeMessageQueue* queue,
                                         MessageType type);

/**
 * Dequeue message for specific receiver
 * 
 * Searches queue for message addressed to specified receiver.
 * WARNING: This is O(n) and may be slow for large queues.
 * 
 * @param queue Queue to search
 * @param receiver_id Receiver ID to match
 * @return Pointer to message, or NULL if not found
 */
SphereMessage* message_queue_dequeue_for_receiver(LockFreeMessageQueue* queue,
                                                  int receiver_id);

// ============================================================================
// QUEUE QUERIES
// ============================================================================

/**
 * Check if queue is empty
 * 
 * @param queue Queue to check
 * @return true if empty, false otherwise
 */
bool message_queue_is_empty(const LockFreeMessageQueue* queue);

/**
 * Get total number of messages in queue (all priorities)
 * 
 * @param queue Queue to query
 * @return Number of messages
 */
uint64_t message_queue_size(const LockFreeMessageQueue* queue);

/**
 * Get number of messages at specific priority
 * 
 * @param queue Queue to query
 * @param priority Priority level
 * @return Number of messages at that priority
 */
uint64_t message_queue_size_priority(const LockFreeMessageQueue* queue,
                                     MessagePriority priority);

/**
 * Check if queue is full
 * 
 * @param queue Queue to check
 * @return true if full, false otherwise
 */
bool message_queue_is_full(const LockFreeMessageQueue* queue);

/**
 * Get queue utilization
 * 
 * @param queue Queue to query
 * @return Utilization (0.0 to 1.0), or -1.0 if unlimited
 */
double message_queue_utilization(const LockFreeMessageQueue* queue);

// ============================================================================
// QUEUE STATISTICS
// ============================================================================

/**
 * Queue Statistics
 */
typedef struct {
    uint64_t total_enqueued;        // Total messages enqueued
    uint64_t total_dequeued;        // Total messages dequeued
    uint64_t total_dropped;         // Total messages dropped
    uint64_t enqueue_failures;      // Failed enqueue attempts
    uint64_t dequeue_failures;      // Failed dequeue attempts
    uint64_t current_size;          // Current queue size
    uint64_t size_by_priority[4];   // Size by priority level
    double utilization;             // Queue utilization (0.0-1.0)
} MessageQueueStatistics;

/**
 * Get queue statistics
 * 
 * @param queue Queue to query
 * @param stats Pointer to statistics structure to fill
 */
void message_queue_get_statistics(const LockFreeMessageQueue* queue,
                                  MessageQueueStatistics* stats);

/**
 * Print queue statistics
 * 
 * @param queue Queue to print statistics for
 */
void message_queue_print_statistics(const LockFreeMessageQueue* queue);

/**
 * Reset queue statistics
 * 
 * @param queue Queue to reset statistics for
 */
void message_queue_reset_statistics(LockFreeMessageQueue* queue);

// ============================================================================
// QUEUE MAINTENANCE
// ============================================================================

/**
 * Clear all messages from queue
 * 
 * WARNING: This function is NOT thread-safe. Ensure no other threads
 * are accessing the queue before calling this function.
 * 
 * @param queue Queue to clear
 * @param free_messages If true, free all messages
 */
void message_queue_clear(LockFreeMessageQueue* queue, bool free_messages);

/**
 * Compact queue by removing processed messages
 * 
 * Removes messages that have been marked as processed.
 * 
 * @param queue Queue to compact
 * @return Number of messages removed
 */
uint64_t message_queue_compact(LockFreeMessageQueue* queue);

/**
 * Validate queue integrity
 * 
 * Checks queue for corruption or inconsistencies.
 * 
 * @param queue Queue to validate
 * @return true if valid, false if corrupted
 */
bool message_queue_validate(const LockFreeMessageQueue* queue);

// ============================================================================
// MEMORY POOL OPERATIONS
// ============================================================================

/**
 * Pre-allocate nodes for queue
 * 
 * Pre-allocates nodes to reduce allocation overhead during runtime.
 * 
 * @param queue Queue to pre-allocate for
 * @param count Number of nodes to pre-allocate
 * @return Number of nodes successfully allocated
 */
uint64_t message_queue_preallocate_nodes(LockFreeMessageQueue* queue,
                                        uint64_t count);

/**
 * Trim free node pool
 * 
 * Frees excess nodes from the free list.
 * 
 * @param queue Queue to trim
 * @param target_count Target number of free nodes to keep
 * @return Number of nodes freed
 */
uint64_t message_queue_trim_free_nodes(LockFreeMessageQueue* queue,
                                       uint64_t target_count);

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

/**
 * Enqueue multiple messages at once
 * 
 * More efficient than calling enqueue multiple times.
 * 
 * @param queue Queue to enqueue to
 * @param messages Array of messages to enqueue
 * @param count Number of messages
 * @return Number of messages successfully enqueued
 */
uint64_t message_queue_enqueue_batch(LockFreeMessageQueue* queue,
                                     SphereMessage** messages,
                                     uint64_t count);

/**
 * Dequeue multiple messages at once
 * 
 * More efficient than calling dequeue multiple times.
 * 
 * @param queue Queue to dequeue from
 * @param messages Array to store dequeued messages
 * @param max_count Maximum number of messages to dequeue
 * @return Number of messages actually dequeued
 */
uint64_t message_queue_dequeue_batch(LockFreeMessageQueue* queue,
                                     SphereMessage** messages,
                                     uint64_t max_count);

#endif // CLLM_MESSAGE_QUEUE_H
