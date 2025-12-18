#ifndef ALGORITHMS_LOCK_FREE_QUEUE_H
#define ALGORITHMS_LOCK_FREE_QUEUE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generic Lock-Free Queue
 * 
 * High-performance, thread-safe queue using lock-free algorithms.
 * Uses Compare-And-Swap (CAS) operations for concurrent access.
 * 
 * Features:
 * - Lock-free enqueue and dequeue operations
 * - Cache-line aligned to prevent false sharing
 * - ABA problem prevention using sequence numbers
 * - Optional memory pooling for reduced allocation overhead
 * - Thread-safe statistics tracking
 * 
 * Based on Michael-Scott lock-free queue algorithm.
 */

// Cache line size for alignment
#define CACHE_LINE_SIZE 64

// ============================================================================
// QUEUE NODE STRUCTURE
// ============================================================================

/**
 * Queue Node
 * 
 * Internal node structure for the lock-free queue.
 * Cache-line aligned to prevent false sharing.
 */
typedef struct LockFreeQueueNode {
    void* data;                                    // Pointer to user data
    _Atomic(struct LockFreeQueueNode*) next;      // Next node (atomic)
    uint64_t sequence;                            // Sequence number for ABA prevention
    char padding[CACHE_LINE_SIZE - sizeof(void*) - sizeof(_Atomic(void*)) - sizeof(uint64_t)];
} __attribute__((aligned(CACHE_LINE_SIZE))) LockFreeQueueNode;

// ============================================================================
// QUEUE STRUCTURE
// ============================================================================

/**
 * Lock-Free Queue
 * 
 * Main queue structure.
 */
typedef struct {
    // Queue head and tail (cache-line aligned)
    _Atomic(LockFreeQueueNode*) head;             // Head of queue (atomic)
    char padding1[CACHE_LINE_SIZE - sizeof(_Atomic(void*))];
    
    _Atomic(LockFreeQueueNode*) tail;             // Tail of queue (atomic)
    char padding2[CACHE_LINE_SIZE - sizeof(_Atomic(void*))];
    
    // Statistics (atomic)
    atomic_uint_fast64_t total_enqueued;          // Total items enqueued
    atomic_uint_fast64_t total_dequeued;          // Total items dequeued
    atomic_uint_fast64_t enqueue_failures;        // Failed enqueue attempts
    atomic_uint_fast64_t dequeue_failures;        // Failed dequeue attempts
    atomic_uint_fast64_t current_size;            // Current queue size (approximate)
    
    // Configuration
    uint64_t max_queue_size;                      // Maximum queue size (0 = unlimited)
    bool drop_on_full;                            // Drop items when full
    
    // Sequence counter for ABA prevention
    atomic_uint_fast64_t sequence_counter;
    
    // Memory pool for nodes (optional optimization)
    _Atomic(LockFreeQueueNode*) free_list;        // Free node list
    atomic_uint_fast64_t free_count;              // Number of free nodes
    uint64_t max_free_nodes;                      // Maximum free nodes to cache
    
} __attribute__((aligned(CACHE_LINE_SIZE))) LockFreeQueue;

// ============================================================================
// QUEUE OPERATIONS
// ============================================================================

/**
 * Create a new lock-free queue
 * 
 * @param max_queue_size Maximum queue size (0 = unlimited)
 * @param drop_on_full Drop items when queue is full
 * @return Pointer to new queue, or NULL on failure
 */
LockFreeQueue* lock_free_queue_create(uint64_t max_queue_size, bool drop_on_full);

/**
 * Free a lock-free queue
 * 
 * WARNING: This function is NOT thread-safe. Ensure no other threads
 * are accessing the queue before calling this function.
 * 
 * @param queue Queue to free
 * @param free_data_fn Optional function to free user data (can be NULL)
 */
void lock_free_queue_free(LockFreeQueue* queue, void (*free_data_fn)(void*));

/**
 * Enqueue an item (lock-free)
 * 
 * Thread-safe. Can be called concurrently from multiple threads.
 * 
 * @param queue Queue to enqueue to
 * @param data Pointer to user data (ownership transferred to queue)
 * @return true on success, false on failure
 */
bool lock_free_queue_enqueue(LockFreeQueue* queue, void* data);

/**
 * Dequeue an item (lock-free)
 * 
 * Thread-safe. Can be called concurrently from multiple threads.
 * 
 * @param queue Queue to dequeue from
 * @return Pointer to user data, or NULL if queue is empty
 */
void* lock_free_queue_dequeue(LockFreeQueue* queue);

/**
 * Peek at next item without removing it
 * 
 * Thread-safe, but item may be dequeued by another thread
 * before you can process it.
 * 
 * @param queue Queue to peek at
 * @return Pointer to user data, or NULL if queue is empty
 */
void* lock_free_queue_peek(const LockFreeQueue* queue);

/**
 * Try to dequeue an item with timeout
 * 
 * Spins for specified duration trying to dequeue an item.
 * 
 * @param queue Queue to dequeue from
 * @param timeout_ns Timeout in nanoseconds
 * @return Pointer to user data, or NULL on timeout
 */
void* lock_free_queue_dequeue_timeout(LockFreeQueue* queue, uint64_t timeout_ns);

// ============================================================================
// QUEUE QUERIES
// ============================================================================

/**
 * Check if queue is empty
 * 
 * @param queue Queue to check
 * @return true if empty, false otherwise
 */
bool lock_free_queue_is_empty(const LockFreeQueue* queue);

/**
 * Get approximate number of items in queue
 * 
 * Note: This is an approximation due to concurrent access.
 * 
 * @param queue Queue to query
 * @return Approximate number of items
 */
uint64_t lock_free_queue_size(const LockFreeQueue* queue);

/**
 * Check if queue is full
 * 
 * @param queue Queue to check
 * @return true if full, false otherwise
 */
bool lock_free_queue_is_full(const LockFreeQueue* queue);

/**
 * Get queue utilization
 * 
 * @param queue Queue to query
 * @return Utilization (0.0 to 1.0), or -1.0 if unlimited
 */
double lock_free_queue_utilization(const LockFreeQueue* queue);

// ============================================================================
// QUEUE STATISTICS
// ============================================================================

/**
 * Queue Statistics
 */
typedef struct {
    uint64_t total_enqueued;        // Total items enqueued
    uint64_t total_dequeued;        // Total items dequeued
    uint64_t enqueue_failures;      // Failed enqueue attempts
    uint64_t dequeue_failures;      // Failed dequeue attempts
    uint64_t current_size;          // Current queue size (approximate)
    double utilization;             // Queue utilization (0.0-1.0)
} LockFreeQueueStatistics;

/**
 * Get queue statistics
 * 
 * @param queue Queue to query
 * @param stats Pointer to statistics structure to fill
 */
void lock_free_queue_get_statistics(const LockFreeQueue* queue,
                                   LockFreeQueueStatistics* stats);

/**
 * Print queue statistics
 * 
 * @param queue Queue to print statistics for
 * @param name Optional name for the queue
 */
void lock_free_queue_print_statistics(const LockFreeQueue* queue, const char* name);

/**
 * Reset queue statistics
 * 
 * @param queue Queue to reset statistics for
 */
void lock_free_queue_reset_statistics(LockFreeQueue* queue);

// ============================================================================
// QUEUE MAINTENANCE
// ============================================================================

/**
 * Clear all items from queue
 * 
 * WARNING: This function is NOT thread-safe. Ensure no other threads
 * are accessing the queue before calling this function.
 * 
 * @param queue Queue to clear
 * @param free_data_fn Optional function to free user data (can be NULL)
 */
void lock_free_queue_clear(LockFreeQueue* queue, void (*free_data_fn)(void*));

/**
 * Validate queue integrity
 * 
 * Checks queue for corruption or inconsistencies.
 * 
 * @param queue Queue to validate
 * @return true if valid, false if corrupted
 */
bool lock_free_queue_validate(const LockFreeQueue* queue);

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
uint64_t lock_free_queue_preallocate_nodes(LockFreeQueue* queue, uint64_t count);

/**
 * Trim free node pool
 * 
 * Frees excess nodes from the free list.
 * 
 * @param queue Queue to trim
 * @param target_count Target number of free nodes to keep
 * @return Number of nodes freed
 */
uint64_t lock_free_queue_trim_free_nodes(LockFreeQueue* queue, uint64_t target_count);

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

/**
 * Enqueue multiple items at once
 * 
 * More efficient than calling enqueue multiple times.
 * 
 * @param queue Queue to enqueue to
 * @param items Array of items to enqueue
 * @param count Number of items
 * @return Number of items successfully enqueued
 */
uint64_t lock_free_queue_enqueue_batch(LockFreeQueue* queue,
                                      void** items,
                                      uint64_t count);

/**
 * Dequeue multiple items at once
 * 
 * More efficient than calling dequeue multiple times.
 * 
 * @param queue Queue to dequeue from
 * @param items Array to store dequeued items
 * @param max_count Maximum number of items to dequeue
 * @return Number of items actually dequeued
 */
uint64_t lock_free_queue_dequeue_batch(LockFreeQueue* queue,
                                      void** items,
                                      uint64_t max_count);

#ifdef __cplusplus
}
#endif

#endif // ALGORITHMS_LOCK_FREE_QUEUE_H
