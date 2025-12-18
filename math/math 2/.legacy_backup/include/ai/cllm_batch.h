#ifndef CLLM_BATCH_H
#define CLLM_BATCH_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// BATCH DATA STRUCTURES
// ============================================================================

/**
 * Tensor Structure
 * 
 * Represents a multi-dimensional tensor for batch data.
 */
typedef struct {
    float* data;                   // Tensor data (flattened)
    uint32_t* shape;               // Tensor shape (dimensions)
    uint32_t ndim;                 // Number of dimensions
    size_t total_size;             // Total number of elements
    bool owns_data;                // Whether this tensor owns the data
} Tensor;

/**
 * Batch Structure
 * 
 * Represents a batch of training data.
 */
typedef struct Batch {
    // Batch identification
    uint64_t batch_id;             // Unique batch ID
    uint32_t epoch_id;             // Epoch this batch belongs to
    
    // Data tensors
    Tensor input;                  // Input data tensor
    Tensor target;                 // Target data tensor
    Tensor mask;                   // Attention mask (optional)
    
    // Batch metadata
    uint32_t batch_size;           // Number of samples in batch
    uint32_t sequence_length;      // Sequence length (for NLP)
    uint32_t vocab_size;           // Vocabulary size
    
    // Memory management
    size_t total_memory;           // Total memory used by batch
    bool is_pooled;                // Whether batch is from pool
    
    // Processing state
    bool is_processed;             // Whether batch has been processed
    double processing_time;        // Time taken to process (seconds)
    
    // Reference counting for shared batches
    atomic_int ref_count;          // Reference count
    pthread_mutex_t mutex;         // Mutex for thread safety
    
} Batch;

/**
 * Batch Queue Node
 */
typedef struct BatchQueueNode {
    Batch* batch;                  // Batch data
    struct BatchQueueNode* next;   // Next node
} BatchQueueNode;

/**
 * Batch Queue
 * 
 * Thread-safe queue for managing batches.
 */
typedef struct {
    BatchQueueNode* head;          // Queue head
    BatchQueueNode* tail;          // Queue tail
    uint32_t size;                 // Current queue size
    uint32_t capacity;             // Maximum queue size
    
    pthread_mutex_t mutex;         // Mutex for thread safety
    pthread_cond_t not_empty;      // Condition: queue not empty
    pthread_cond_t not_full;       // Condition: queue not full
    
    bool closed;                   // Whether queue is closed
    
} BatchQueue;

/**
 * Batch Pool
 * 
 * Pool of pre-allocated batches for efficient memory management.
 */
typedef struct {
    Batch** batches;               // Array of batch pointers
    bool* available;               // Availability flags
    uint32_t pool_size;            // Number of batches in pool
    
    uint32_t batch_size;           // Batch size for pooled batches
    uint32_t sequence_length;      // Sequence length for pooled batches
    uint32_t vocab_size;           // Vocabulary size
    
    pthread_mutex_t mutex;         // Mutex for thread safety
    pthread_cond_t available_cond; // Condition: batch available
    
    // Statistics
    uint64_t allocations;          // Total allocations
    uint64_t releases;             // Total releases
    uint64_t cache_hits;           // Cache hits
    uint64_t cache_misses;         // Cache misses
    
} BatchPool;

// ============================================================================
// TENSOR FUNCTIONS
// ============================================================================

/**
 * Create a tensor with specified shape
 * 
 * @param shape Array of dimension sizes
 * @param ndim Number of dimensions
 * @return Pointer to created tensor, or NULL on failure
 */
Tensor* tensor_create(const uint32_t* shape, uint32_t ndim);

/**
 * Free a tensor
 * 
 * @param tensor Tensor to free
 */
void tensor_free(Tensor* tensor);

/**
 * Copy a tensor
 * 
 * @param src Source tensor
 * @return Pointer to copied tensor, or NULL on failure
 */
Tensor* tensor_copy(const Tensor* src);

/**
 * Reshape a tensor (does not copy data)
 * 
 * @param tensor Tensor to reshape
 * @param new_shape New shape
 * @param new_ndim New number of dimensions
 * @return true on success, false on failure
 */
bool tensor_reshape(Tensor* tensor, const uint32_t* new_shape, uint32_t new_ndim);

/**
 * Get element at specified indices
 * 
 * @param tensor Tensor
 * @param indices Array of indices
 * @return Element value
 */
float tensor_get(const Tensor* tensor, const uint32_t* indices);

/**
 * Set element at specified indices
 * 
 * @param tensor Tensor
 * @param indices Array of indices
 * @param value Value to set
 */
void tensor_set(Tensor* tensor, const uint32_t* indices, float value);

/**
 * Fill tensor with value
 * 
 * @param tensor Tensor to fill
 * @param value Value to fill with
 */
void tensor_fill(Tensor* tensor, float value);

/**
 * Print tensor information
 * 
 * @param tensor Tensor to print
 */
void tensor_print(const Tensor* tensor);

// ============================================================================
// BATCH FUNCTIONS
// ============================================================================

/**
 * Create a batch
 * 
 * @param batch_size Number of samples in batch
 * @param sequence_length Sequence length
 * @param vocab_size Vocabulary size
 * @return Pointer to created batch, or NULL on failure
 */
Batch* batch_create(uint32_t batch_size, uint32_t sequence_length, uint32_t vocab_size);

/**
 * Free a batch
 * 
 * @param batch Batch to free
 */
void batch_free(Batch* batch);

/**
 * Copy a batch
 * 
 * @param src Source batch
 * @return Pointer to copied batch, or NULL on failure
 */
Batch* batch_copy(const Batch* src);

/**
 * Split a batch into multiple smaller batches
 * 
 * @param batch Batch to split
 * @param num_splits Number of splits
 * @param splits Output array of split batches
 * @return true on success, false on failure
 */
bool batch_split(const Batch* batch, uint32_t num_splits, Batch** splits);

/**
 * Merge multiple batches into one
 * 
 * @param batches Array of batches to merge
 * @param num_batches Number of batches
 * @return Pointer to merged batch, or NULL on failure
 */
Batch* batch_merge(Batch** batches, uint32_t num_batches);

/**
 * Increment batch reference count
 * 
 * @param batch Batch
 */
void batch_retain(Batch* batch);

/**
 * Decrement batch reference count and free if zero
 * 
 * @param batch Batch
 */
void batch_release(Batch* batch);

/**
 * Print batch information
 * 
 * @param batch Batch to print
 */
void batch_print(const Batch* batch);

/**
 * Validate batch data
 * 
 * @param batch Batch to validate
 * @return true if valid, false if invalid
 */
bool batch_validate(const Batch* batch);

// ============================================================================
// BATCH QUEUE FUNCTIONS
// ============================================================================

/**
 * Create a batch queue
 * 
 * @param capacity Maximum queue capacity (0 = unlimited)
 * @return Pointer to created queue, or NULL on failure
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
 * @return true on success, false on failure
 */
bool batch_queue_enqueue(BatchQueue* queue, Batch* batch);

/**
 * Enqueue a batch (non-blocking)
 * 
 * @param queue Batch queue
 * @param batch Batch to enqueue
 * @return true on success, false if full
 */
bool batch_queue_try_enqueue(BatchQueue* queue, Batch* batch);

/**
 * Dequeue a batch (blocking if empty)
 * 
 * @param queue Batch queue
 * @return Pointer to dequeued batch, or NULL if closed
 */
Batch* batch_queue_dequeue(BatchQueue* queue);

/**
 * Dequeue a batch (non-blocking)
 * 
 * @param queue Batch queue
 * @return Pointer to dequeued batch, or NULL if empty
 */
Batch* batch_queue_try_dequeue(BatchQueue* queue);

/**
 * Peek at front batch without removing
 * 
 * @param queue Batch queue
 * @return Pointer to front batch, or NULL if empty
 */
Batch* batch_queue_peek(BatchQueue* queue);

/**
 * Get current queue size
 * 
 * @param queue Batch queue
 * @return Current size
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
 * Close the queue (no more enqueues allowed)
 * 
 * @param queue Batch queue
 */
void batch_queue_close(BatchQueue* queue);

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
 * @param batch_size Batch size for pooled batches
 * @param sequence_length Sequence length
 * @param vocab_size Vocabulary size
 * @return Pointer to created pool, or NULL on failure
 */
BatchPool* batch_pool_create(uint32_t pool_size, uint32_t batch_size,
                             uint32_t sequence_length, uint32_t vocab_size);

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
 * @return Pointer to allocated batch, or NULL on failure
 */
Batch* batch_pool_allocate(BatchPool* pool);

/**
 * Allocate a batch from pool (non-blocking)
 * 
 * @param pool Batch pool
 * @return Pointer to allocated batch, or NULL if none available
 */
Batch* batch_pool_try_allocate(BatchPool* pool);

/**
 * Release a batch back to pool
 * 
 * @param pool Batch pool
 * @param batch Batch to release
 */
void batch_pool_release(BatchPool* pool, Batch* batch);

/**
 * Resize the batch pool
 * 
 * @param pool Batch pool
 * @param new_size New pool size
 * @return true on success, false on failure
 */
bool batch_pool_resize(BatchPool* pool, uint32_t new_size);

/**
 * Get pool statistics
 * 
 * @param pool Batch pool
 * @param allocations Output: total allocations
 * @param releases Output: total releases
 * @param cache_hits Output: cache hits
 * @param cache_misses Output: cache misses
 */
void batch_pool_get_stats(const BatchPool* pool, uint64_t* allocations,
                          uint64_t* releases, uint64_t* cache_hits,
                          uint64_t* cache_misses);

/**
 * Print pool statistics
 * 
 * @param pool Batch pool
 */
void batch_pool_print_stats(const BatchPool* pool);

// ============================================================================
// BATCH DISTRIBUTION FUNCTIONS
// ============================================================================

/**
 * Distribute batch across spheres
 * 
 * @param batch Batch to distribute
 * @param num_spheres Number of spheres
 * @param sphere_batches Output array of batches for each sphere
 * @return true on success, false on failure
 */
bool batch_distribute_to_spheres(const Batch* batch, uint32_t num_spheres,
                                 Batch** sphere_batches);

/**
 * Assign batch to symmetry group
 * 
 * @param batch Batch
 * @param symmetry_group Symmetry group (0-11)
 * @return true on success, false on failure
 */
bool batch_assign_to_group(Batch* batch, uint32_t symmetry_group);

/**
 * Balance batch distribution across spheres
 * 
 * @param batches Array of batches
 * @param num_batches Number of batches
 * @param num_spheres Number of spheres
 * @param assignments Output array of sphere assignments
 * @return true on success, false on failure
 */
bool batch_balance_distribution(Batch** batches, uint32_t num_batches,
                                uint32_t num_spheres, uint32_t* assignments);

#ifdef __cplusplus
}
#endif

#endif // CLLM_BATCH_H