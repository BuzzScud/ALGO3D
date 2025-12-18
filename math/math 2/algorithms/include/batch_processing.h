#ifndef BATCH_PROCESSING_H
#define BATCH_PROCESSING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

/**
 * Generic Batch Processing
 * 
 * Provides generic batch queue management and memory pooling extracted from
 * the CLLM batch processing implementation.
 * 
 * This module provides broadly applicable algorithms for:
 * - Batch queue management (thread-safe FIFO queue)
 * - Memory pooling (pre-allocated batch objects)
 * - Reference counting (shared batch management)
 * - Batch splitting and merging
 * 
 * All tensor-specific operations remain in libcllm.so.
 */

// ============================================================================
// GENERIC BATCH STRUCTURE
// ============================================================================

/**
 * Generic Batch
 * 
 * Represents a generic batch of data with user-defined payload.
 */
typedef struct GenericBatch {
    // Batch identification
    uint64_t batch_id;                  // Unique batch ID
    uint32_t epoch_id;                  // Epoch this batch belongs to
    
    // User data
    void* data;                         // User-defined batch data
    size_t data_size;                   // Size of user data
    void (*free_data)(void*);           // Function to free user data
    
    // Batch metadata
    uint32_t batch_size;                // Number of items in batch
    size_t total_memory;                // Total memory used
    bool is_pooled;                     // Whether from pool
    
    // Processing state
    bool is_processed;                  // Whether processed
    double processing_time;             // Processing time (seconds)
    
    // Reference counting
    atomic_int ref_count;               // Reference count
    pthread_mutex_t mutex;              // Mutex for thread safety
    
} GenericBatch;

// ============================================================================
// BATCH QUEUE STRUCTURE
// ============================================================================

/**
 * Batch Queue Node
 */
typedef struct BatchQueueNode {
    GenericBatch* batch;                // Batch data
    struct BatchQueueNode* next;        // Next node
} BatchQueueNode;

/**
 * Batch Queue
 * 
 * Thread-safe FIFO queue for managing batches.
 */
typedef struct {
    BatchQueueNode* head;               // Queue head
    BatchQueueNode* tail;               // Queue tail
    uint32_t size;                      // Current size
    uint32_t capacity;                  // Maximum size
    
    pthread_mutex_t mutex;              // Mutex for thread safety
    pthread_cond_t not_empty;           // Condition: not empty
    pthread_cond_t not_full;            // Condition: not full
    
    bool closed;                        // Whether queue is closed
    
} BatchQueue;

// ============================================================================
// BATCH POOL STRUCTURE
// ============================================================================

/**
 * Batch Pool
 * 
 * Pool of pre-allocated batches for efficient memory management.
 */
typedef struct {
    GenericBatch** batches;             // Array of batch pointers
    bool* available;                    // Availability flags
    uint32_t pool_size;                 // Number of batches
    
    uint32_t batch_size;                // Size of each batch
    size_t data_size;                   // Size of data per batch
    
    pthread_mutex_t mutex;              // Mutex for thread safety
    pthread_cond_t available_cond;      // Condition: batch available
    
    // Statistics
    atomic_uint_fast64_t allocations;   // Total allocations
    atomic_uint_fast64_t releases;      // Total releases
    atomic_uint_fast64_t hits;          // Pool hits
    atomic_uint_fast64_t misses;        // Pool misses
    
} BatchPool;

// ============================================================================
// BATCH MANAGEMENT FUNCTIONS
// ============================================================================

/**
 * Create a generic batch
 * 
 * @param batch_id Unique batch ID
 * @param epoch_id Epoch ID
 * @param batch_size Number of items in batch
 * @param data User data
 * @param data_size Size of user data
 * @param free_data Function to free user data (can be NULL)
 * @return New batch, or NULL on error
 */
GenericBatch* batch_create(
    uint64_t batch_id,
    uint32_t epoch_id,
    uint32_t batch_size,
    void* data,
    size_t data_size,
    void (*free_data)(void*)
);

/**
 * Free a batch
 * 
 * @param batch Batch to free
 */
void batch_free(GenericBatch* batch);

/**
 * Copy a batch
 * 
 * @param src Source batch
 * @return New batch copy, or NULL on error
 */
GenericBatch* batch_copy(const GenericBatch* src);

/**
 * Retain a batch (increment reference count)
 * 
 * @param batch Batch to retain
 */
void batch_retain(GenericBatch* batch);

/**
 * Release a batch (decrement reference count, free if zero)
 * 
 * @param batch Batch to release
 */
void batch_release(GenericBatch* batch);

/**
 * Get batch reference count
 * 
 * @param batch Batch
 * @return Current reference count
 */
int batch_get_ref_count(const GenericBatch* batch);

/**
 * Mark batch as processed
 * 
 * @param batch Batch
 * @param processing_time Processing time in seconds
 */
void batch_mark_processed(GenericBatch* batch, double processing_time);

/**
 * Check if batch is processed
 * 
 * @param batch Batch
 * @return true if processed, false otherwise
 */
bool batch_is_processed(const GenericBatch* batch);

// ============================================================================
// BATCH QUEUE FUNCTIONS
// ============================================================================

/**
 * Create a batch queue
 * 
 * @param capacity Maximum queue capacity (0 = unlimited)
 * @return New queue, or NULL on error
 */
BatchQueue* batch_queue_create(uint32_t capacity);

/**
 * Free a batch queue
 * 
 * @param queue Queue to free
 */
void batch_queue_free(BatchQueue* queue);

/**
 * Enqueue a batch (blocking if full)
 * 
 * @param queue Batch queue
 * @param batch Batch to enqueue
 * @return true on success, false on error
 */
bool batch_queue_enqueue(BatchQueue* queue, GenericBatch* batch);

/**
 * Try to enqueue a batch (non-blocking)
 * 
 * @param queue Batch queue
 * @param batch Batch to enqueue
 * @return true on success, false if full or error
 */
bool batch_queue_try_enqueue(BatchQueue* queue, GenericBatch* batch);

/**
 * Dequeue a batch (blocking if empty)
 * 
 * @param queue Batch queue
 * @return Batch, or NULL if queue is closed
 */
GenericBatch* batch_queue_dequeue(BatchQueue* queue);

/**
 * Try to dequeue a batch (non-blocking)
 * 
 * @param queue Batch queue
 * @return Batch, or NULL if empty or closed
 */
GenericBatch* batch_queue_try_dequeue(BatchQueue* queue);

/**
 * Peek at next batch without removing
 * 
 * @param queue Batch queue
 * @return Batch, or NULL if empty
 */
GenericBatch* batch_queue_peek(BatchQueue* queue);

/**
 * Get queue size
 * 
 * @param queue Batch queue
 * @return Current number of batches in queue
 */
uint32_t batch_queue_size(const BatchQueue* queue);

/**
 * Check if queue is empty
 * 
 * @param queue Batch queue
 * @return true if empty, false otherwise
 */
bool batch_queue_is_empty(const BatchQueue* queue);

/**
 * Check if queue is full
 * 
 * @param queue Batch queue
 * @return true if full, false otherwise
 */
bool batch_queue_is_full(const BatchQueue* queue);

/**
 * Close queue (no more enqueues allowed)
 * 
 * @param queue Batch queue
 */
void batch_queue_close(BatchQueue* queue);

/**
 * Check if queue is closed
 * 
 * @param queue Batch queue
 * @return true if closed, false otherwise
 */
bool batch_queue_is_closed(const BatchQueue* queue);

/**
 * Clear all batches from queue
 * 
 * @param queue Batch queue
 */
void batch_queue_clear(BatchQueue* queue);

// ============================================================================
// BATCH POOL FUNCTIONS
// ============================================================================

/**
 * Create a batch pool
 * 
 * @param pool_size Number of batches in pool
 * @param batch_size Size of each batch
 * @param data_size Size of data per batch
 * @return New pool, or NULL on error
 */
BatchPool* batch_pool_create(
    uint32_t pool_size,
    uint32_t batch_size,
    size_t data_size
);

/**
 * Free a batch pool
 * 
 * @param pool Pool to free
 */
void batch_pool_free(BatchPool* pool);

/**
 * Allocate a batch from pool (blocking if none available)
 * 
 * @param pool Batch pool
 * @return Batch from pool, or NULL on error
 */
GenericBatch* batch_pool_allocate(BatchPool* pool);

/**
 * Try to allocate a batch from pool (non-blocking)
 * 
 * @param pool Batch pool
 * @return Batch from pool, or NULL if none available
 */
GenericBatch* batch_pool_try_allocate(BatchPool* pool);

/**
 * Release a batch back to pool
 * 
 * @param pool Batch pool
 * @param batch Batch to release
 */
void batch_pool_release(BatchPool* pool, GenericBatch* batch);

/**
 * Resize batch pool
 * 
 * @param pool Batch pool
 * @param new_size New pool size
 * @return true on success, false on error
 */
bool batch_pool_resize(BatchPool* pool, uint32_t new_size);

/**
 * Get pool statistics
 * 
 * @param pool Batch pool
 * @param allocations Output: total allocations
 * @param releases Output: total releases
 * @param hits Output: pool hits
 * @param misses Output: pool misses
 */
void batch_pool_get_stats(
    const BatchPool* pool,
    uint64_t* allocations,
    uint64_t* releases,
    uint64_t* hits,
    uint64_t* misses
);

/**
 * Get pool efficiency (hit rate)
 * 
 * @param pool Batch pool
 * @return Hit rate (0.0 to 1.0)
 */
double batch_pool_get_efficiency(const BatchPool* pool);

/**
 * Print pool statistics
 * 
 * @param pool Batch pool
 */
void batch_pool_print_stats(const BatchPool* pool);

// ============================================================================
// BATCH SPLITTING AND MERGING
// ============================================================================

/**
 * Split a batch into smaller batches
 * 
 * @param batch Source batch
 * @param num_splits Number of splits
 * @param splits Output array of split batches
 * @return true on success, false on error
 */
bool batch_split(
    const GenericBatch* batch,
    uint32_t num_splits,
    GenericBatch** splits
);

/**
 * Merge multiple batches into one
 * 
 * @param batches Array of batches to merge
 * @param num_batches Number of batches
 * @return Merged batch, or NULL on error
 */
GenericBatch* batch_merge(GenericBatch** batches, uint32_t num_batches);

#endif // BATCH_PROCESSING_H
