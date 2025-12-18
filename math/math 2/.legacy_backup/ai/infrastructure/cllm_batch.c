/**
 * @file infrastructure/cllm_batch.c
 * @brief Low-Level Batch Infrastructure and Tensor Operations
 * 
 * This is the LOW-LEVEL INFRASTRUCTURE for batch processing.
 * Provides tensor operations, batch splitting, and thread-safe management.
 * 
 * RELATIONSHIP TO OTHER FILES:
 * - This file: Low-level infrastructure (tensors, splitting, queues)
 * - src/ai/cllm_batch.c: High-level simple API for training loops
 * 
 * USE THIS WHEN:
 * - Need multi-dimensional tensor operations (reshape, slice, etc.)
 * - Splitting batches for parallel processing across threads
 * - Thread-safe batch queue management
 * - Reference-counted batch sharing between threads
 * - Complex batch transformations
 * 
 * USE src/ai/cllm_batch.c WHEN:
 * - Simple batch generation from tokens
 * - Basic training loop batch iteration
 * - Don't need advanced tensor operations
 * 
 * ANALOGY: This is like open() - powerful, flexible, but more complex.
 *          src/ai/cllm_batch.c is like fopen() - simple and easy to use.
 */

#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "ai/cllm_batch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static size_t compute_tensor_size(const uint32_t* shape, uint32_t ndim) {
    size_t size = 1;
    for (uint32_t i = 0; i < ndim; i++) {
        size *= shape[i];
    }
    return size;
}

static size_t compute_flat_index(const uint32_t* indices, const uint32_t* shape, uint32_t ndim) {
    size_t index = 0;
    size_t stride = 1;
    for (int i = ndim - 1; i >= 0; i--) {
        index += indices[i] * stride;
        stride *= shape[i];
    }
    return index;
}

// ============================================================================
// TENSOR FUNCTIONS
// ============================================================================

Tensor* tensor_create(const uint32_t* shape, uint32_t ndim) {
    if (!shape || ndim == 0) {
        fprintf(stderr, "Error: Invalid tensor shape\n");
        return NULL;
    }
    
    Tensor* tensor = (Tensor*)calloc(1, sizeof(Tensor));
    if (!tensor) {
        fprintf(stderr, "Error: Failed to allocate tensor\n");
        return NULL;
    }
    
    // Allocate shape array
    tensor->shape = (uint32_t*)malloc(ndim * sizeof(uint32_t));
    if (!tensor->shape) {
        free(tensor);
        fprintf(stderr, "Error: Failed to allocate tensor shape\n");
        return NULL;
    }
    memcpy(tensor->shape, shape, ndim * sizeof(uint32_t));
    tensor->ndim = ndim;
    
    // Compute total size and allocate data
    tensor->total_size = compute_tensor_size(shape, ndim);
    tensor->data = (float*)calloc(tensor->total_size, sizeof(float));
    if (!tensor->data) {
        free(tensor->shape);
        free(tensor);
        fprintf(stderr, "Error: Failed to allocate tensor data\n");
        return NULL;
    }
    
    tensor->owns_data = true;
    
    return tensor;
}

void tensor_free(Tensor* tensor) {
    if (!tensor) return;
    
    if (tensor->owns_data && tensor->data) {
        free(tensor->data);
    }
    if (tensor->shape) {
        free(tensor->shape);
    }
    free(tensor);
}

Tensor* tensor_copy(const Tensor* src) {
    if (!src) return NULL;
    
    Tensor* dst = tensor_create(src->shape, src->ndim);
    if (!dst) return NULL;
    
    memcpy(dst->data, src->data, src->total_size * sizeof(float));
    
    return dst;
}

bool tensor_reshape(Tensor* tensor, const uint32_t* new_shape, uint32_t new_ndim) {
    if (!tensor || !new_shape) return false;
    
    // Check that total size matches
    size_t new_size = compute_tensor_size(new_shape, new_ndim);
    if (new_size != tensor->total_size) {
        fprintf(stderr, "Error: Cannot reshape tensor - size mismatch\n");
        return false;
    }
    
    // Update shape
    uint32_t* shape = (uint32_t*)realloc(tensor->shape, new_ndim * sizeof(uint32_t));
    if (!shape) {
        fprintf(stderr, "Error: Failed to reallocate tensor shape\n");
        return false;
    }
    
    tensor->shape = shape;
    memcpy(tensor->shape, new_shape, new_ndim * sizeof(uint32_t));
    tensor->ndim = new_ndim;
    
    return true;
}

float tensor_get(const Tensor* tensor, const uint32_t* indices) {
    if (!tensor || !indices) return 0.0f;
    
    size_t index = compute_flat_index(indices, tensor->shape, tensor->ndim);
    if (index >= tensor->total_size) return 0.0f;
    
    return tensor->data[index];
}

void tensor_set(Tensor* tensor, const uint32_t* indices, float value) {
    if (!tensor || !indices) return;
    
    size_t index = compute_flat_index(indices, tensor->shape, tensor->ndim);
    if (index >= tensor->total_size) return;
    
    tensor->data[index] = value;
}

void tensor_fill(Tensor* tensor, float value) {
    if (!tensor) return;
    
    for (size_t i = 0; i < tensor->total_size; i++) {
        tensor->data[i] = value;
    }
}

void tensor_print(const Tensor* tensor) {
    if (!tensor) return;
    
    printf("Tensor(shape=[");
    for (uint32_t i = 0; i < tensor->ndim; i++) {
        printf("%u", tensor->shape[i]);
        if (i < tensor->ndim - 1) printf(", ");
    }
    printf("], size=%zu, owns_data=%d)\n", tensor->total_size, tensor->owns_data);
}

// ============================================================================
// BATCH FUNCTIONS
// ============================================================================

Batch* batch_create(uint32_t batch_size, uint32_t sequence_length, uint32_t vocab_size) {
    if (batch_size == 0 || sequence_length == 0 || vocab_size == 0) {
        fprintf(stderr, "Error: Invalid batch parameters\n");
        return NULL;
    }
    
    Batch* batch = (Batch*)calloc(1, sizeof(Batch));
    if (!batch) {
        fprintf(stderr, "Error: Failed to allocate batch\n");
        return NULL;
    }
    
    // Set metadata
    batch->batch_size = batch_size;
    batch->sequence_length = sequence_length;
    batch->vocab_size = vocab_size;
    
    // Create input tensor [batch_size, sequence_length]
    uint32_t input_shape[2] = {batch_size, sequence_length};
    Tensor* input = tensor_create(input_shape, 2);
    if (!input) {
        free(batch);
        return NULL;
    }
    memcpy(&batch->input, input, sizeof(Tensor));
    free(input); // Free the wrapper, not the data
    
    // Create target tensor [batch_size, sequence_length]
    uint32_t target_shape[2] = {batch_size, sequence_length};
    Tensor* target = tensor_create(target_shape, 2);
    if (!target) {
        if (batch->input.data) free(batch->input.data);
        if (batch->input.shape) free(batch->input.shape);
        free(batch);
        return NULL;
    }
    memcpy(&batch->target, target, sizeof(Tensor));
    free(target);
    
    // Create mask tensor [batch_size, sequence_length]
    uint32_t mask_shape[2] = {batch_size, sequence_length};
    Tensor* mask = tensor_create(mask_shape, 2);
    if (!mask) {
        if (batch->input.data) free(batch->input.data);
        if (batch->input.shape) free(batch->input.shape);
        if (batch->target.data) free(batch->target.data);
        if (batch->target.shape) free(batch->target.shape);
        free(batch);
        return NULL;
    }
    memcpy(&batch->mask, mask, sizeof(Tensor));
    free(mask);
    
    // Initialize mask to all ones
    tensor_fill(&batch->mask, 1.0f);
    
    // Calculate total memory
    batch->total_memory = (batch->input.total_size + 
                          batch->target.total_size + 
                          batch->mask.total_size) * sizeof(float);
    
    // Initialize reference count
    atomic_init(&batch->ref_count, 1);
    pthread_mutex_init(&batch->mutex, NULL);
    
    return batch;
}

void batch_free(Batch* batch) {
    if (!batch) return;
    
    // Free tensor data and shapes
    if (batch->input.owns_data && batch->input.data) {
        free(batch->input.data);
    }
    if (batch->input.shape) {
        free(batch->input.shape);
    }
    
    if (batch->target.owns_data && batch->target.data) {
        free(batch->target.data);
    }
    if (batch->target.shape) {
        free(batch->target.shape);
    }
    
    if (batch->mask.owns_data && batch->mask.data) {
        free(batch->mask.data);
    }
    if (batch->mask.shape) {
        free(batch->mask.shape);
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&batch->mutex);
    
    free(batch);
}

Batch* batch_copy(const Batch* src) {
    if (!src) return NULL;
    
    Batch* dst = batch_create(src->batch_size, src->sequence_length, src->vocab_size);
    if (!dst) return NULL;
    
    // Copy tensor data
    memcpy(dst->input.data, src->input.data, src->input.total_size * sizeof(float));
    memcpy(dst->target.data, src->target.data, src->target.total_size * sizeof(float));
    memcpy(dst->mask.data, src->mask.data, src->mask.total_size * sizeof(float));
    
    // Copy metadata
    dst->batch_id = src->batch_id;
    dst->epoch_id = src->epoch_id;
    dst->is_processed = src->is_processed;
    dst->processing_time = src->processing_time;
    
    return dst;
}

bool batch_split(const Batch* batch, uint32_t num_splits, Batch** splits) {
    if (!batch || num_splits == 0 || !splits) return false;
    
    if (batch->batch_size % num_splits != 0) {
        fprintf(stderr, "Error: Batch size must be divisible by num_splits\n");
        return false;
    }
    
    uint32_t split_size = batch->batch_size / num_splits;
    
    for (uint32_t i = 0; i < num_splits; i++) {
        splits[i] = batch_create(split_size, batch->sequence_length, batch->vocab_size);
        if (!splits[i]) {
            // Clean up previously created splits
            for (uint32_t j = 0; j < i; j++) {
                batch_free(splits[j]);
            }
            return false;
        }
        
        // Copy data for this split
        size_t offset = i * split_size * batch->sequence_length;
        size_t size = split_size * batch->sequence_length;
        
        memcpy(splits[i]->input.data, batch->input.data + offset, size * sizeof(float));
        memcpy(splits[i]->target.data, batch->target.data + offset, size * sizeof(float));
        memcpy(splits[i]->mask.data, batch->mask.data + offset, size * sizeof(float));
        
        splits[i]->batch_id = batch->batch_id;
        splits[i]->epoch_id = batch->epoch_id;
    }
    
    return true;
}

Batch* batch_merge(Batch** batches, uint32_t num_batches) {
    if (!batches || num_batches == 0) return NULL;
    
    // Check that all batches have same sequence length and vocab size
    uint32_t sequence_length = batches[0]->sequence_length;
    uint32_t vocab_size = batches[0]->vocab_size;
    uint32_t total_batch_size = 0;
    
    for (uint32_t i = 0; i < num_batches; i++) {
        if (batches[i]->sequence_length != sequence_length ||
            batches[i]->vocab_size != vocab_size) {
            fprintf(stderr, "Error: All batches must have same sequence length and vocab size\n");
            return NULL;
        }
        total_batch_size += batches[i]->batch_size;
    }
    
    // Create merged batch
    Batch* merged = batch_create(total_batch_size, sequence_length, vocab_size);
    if (!merged) return NULL;
    
    // Copy data from all batches
    size_t offset = 0;
    for (uint32_t i = 0; i < num_batches; i++) {
        size_t size = batches[i]->batch_size * sequence_length;
        
        memcpy(merged->input.data + offset, batches[i]->input.data, size * sizeof(float));
        memcpy(merged->target.data + offset, batches[i]->target.data, size * sizeof(float));
        memcpy(merged->mask.data + offset, batches[i]->mask.data, size * sizeof(float));
        
        offset += size;
    }
    
    return merged;
}

void batch_retain(Batch* batch) {
    if (!batch) return;
    atomic_fetch_add(&batch->ref_count, 1);
}

void batch_release(Batch* batch) {
    if (!batch) return;
    
    int old_count = atomic_fetch_sub(&batch->ref_count, 1);
    if (old_count == 1) {
        // Last reference, free the batch
        batch_free(batch);
    }
}

void batch_print(const Batch* batch) {
    if (!batch) return;
    
    printf("Batch(id=%lu, epoch=%u, batch_size=%u, seq_len=%u, vocab_size=%u, memory=%zu bytes)\n",
           batch->batch_id, batch->epoch_id, batch->batch_size, 
           batch->sequence_length, batch->vocab_size, batch->total_memory);
}

bool batch_validate(const Batch* batch) {
    if (!batch) return false;
    
    // Check tensors
    if (!batch->input.data || !batch->target.data || !batch->mask.data) {
        return false;
    }
    
    // Check for NaN or Inf
    for (size_t i = 0; i < batch->input.total_size; i++) {
        if (math_is_nan((double)batch->input.data[i]) || math_is_inf((double)batch->input.data[i])) {
            return false;
        }
    }
    
    for (size_t i = 0; i < batch->target.total_size; i++) {
        if (math_is_nan((double)batch->target.data[i]) || math_is_inf((double)batch->target.data[i])) {
            return false;
        }
    }
    
    return true;
}

// ============================================================================
// BATCH QUEUE FUNCTIONS
// ============================================================================

BatchQueue* batch_queue_create(uint32_t capacity) {
    BatchQueue* queue = (BatchQueue*)calloc(1, sizeof(BatchQueue));
    if (!queue) {
        fprintf(stderr, "Error: Failed to allocate batch queue\n");
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
    queue->closed = false;
    
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    
    return queue;
}

void batch_queue_free(BatchQueue* queue) {
    if (!queue) return;
    
    // Clear all batches
    batch_queue_clear(queue);
    
    // Destroy synchronization primitives
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    
    free(queue);
}

bool batch_queue_enqueue(BatchQueue* queue, Batch* batch) {
    if (!queue || !batch) return false;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Wait if queue is full
    while (queue->capacity > 0 && queue->size >= queue->capacity && !queue->closed) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    if (queue->closed) {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    
    // Create node
    BatchQueueNode* node = (BatchQueueNode*)malloc(sizeof(BatchQueueNode));
    if (!node) {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    
    node->batch = batch;
    node->next = NULL;
    
    // Add to queue
    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
    queue->size++;
    
    // Signal that queue is not empty
    pthread_cond_signal(&queue->not_empty);
    
    pthread_mutex_unlock(&queue->mutex);
    
    return true;
}

bool batch_queue_try_enqueue(BatchQueue* queue, Batch* batch) {
    if (!queue || !batch) return false;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Check if queue is full or closed
    if (queue->closed || (queue->capacity > 0 && queue->size >= queue->capacity)) {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    
    // Create node
    BatchQueueNode* node = (BatchQueueNode*)malloc(sizeof(BatchQueueNode));
    if (!node) {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    
    node->batch = batch;
    node->next = NULL;
    
    // Add to queue
    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
    queue->size++;
    
    // Signal that queue is not empty
    pthread_cond_signal(&queue->not_empty);
    
    pthread_mutex_unlock(&queue->mutex);
    
    return true;
}

Batch* batch_queue_dequeue(BatchQueue* queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Wait if queue is empty
    while (queue->size == 0 && !queue->closed) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    // Remove from queue
    BatchQueueNode* node = queue->head;
    Batch* batch = node->batch;
    
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->size--;
    
    free(node);
    
    // Signal that queue is not full
    pthread_cond_signal(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
    
    return batch;
}

Batch* batch_queue_try_dequeue(BatchQueue* queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    // Remove from queue
    BatchQueueNode* node = queue->head;
    Batch* batch = node->batch;
    
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->size--;
    
    free(node);
    
    // Signal that queue is not full
    pthread_cond_signal(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
    
    return batch;
}

Batch* batch_queue_peek(BatchQueue* queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    Batch* batch = NULL;
    if (queue->head) {
        batch = queue->head->batch;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    
    return batch;
}

uint32_t batch_queue_size(const BatchQueue* queue) {
    if (!queue) return 0;
    return queue->size;
}

bool batch_queue_is_empty(const BatchQueue* queue) {
    if (!queue) return true;
    return queue->size == 0;
}

bool batch_queue_is_full(const BatchQueue* queue) {
    if (!queue) return false;
    if (queue->capacity == 0) return false;
    return queue->size >= queue->capacity;
}

void batch_queue_close(BatchQueue* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    queue->closed = true;
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
}

void batch_queue_clear(BatchQueue* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->head) {
        BatchQueueNode* node = queue->head;
        queue->head = node->next;
        batch_release(node->batch);
        free(node);
    }
    
    queue->tail = NULL;
    queue->size = 0;
    
    pthread_mutex_unlock(&queue->mutex);
}

// ============================================================================
// BATCH POOL FUNCTIONS
// ============================================================================

BatchPool* batch_pool_create(uint32_t pool_size, uint32_t batch_size,
                             uint32_t sequence_length, uint32_t vocab_size) {
    if (pool_size == 0) {
        fprintf(stderr, "Error: Pool size must be > 0\n");
        return NULL;
    }
    
    BatchPool* pool = (BatchPool*)calloc(1, sizeof(BatchPool));
    if (!pool) {
        fprintf(stderr, "Error: Failed to allocate batch pool\n");
        return NULL;
    }
    
    pool->pool_size = pool_size;
    pool->batch_size = batch_size;
    pool->sequence_length = sequence_length;
    pool->vocab_size = vocab_size;
    
    // Allocate arrays
    pool->batches = (Batch**)calloc(pool_size, sizeof(Batch*));
    pool->available = (bool*)calloc(pool_size, sizeof(bool));
    
    if (!pool->batches || !pool->available) {
        if (pool->batches) free(pool->batches);
        if (pool->available) free(pool->available);
        free(pool);
        fprintf(stderr, "Error: Failed to allocate pool arrays\n");
        return NULL;
    }
    
    // Create batches
    for (uint32_t i = 0; i < pool_size; i++) {
        pool->batches[i] = batch_create(batch_size, sequence_length, vocab_size);
        if (!pool->batches[i]) {
            // Clean up
            for (uint32_t j = 0; j < i; j++) {
                batch_free(pool->batches[j]);
            }
            free(pool->batches);
            free(pool->available);
            free(pool);
            fprintf(stderr, "Error: Failed to create batch %u\n", i);
            return NULL;
        }
        pool->batches[i]->is_pooled = true;
        pool->available[i] = true;
    }
    
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->available_cond, NULL);
    
    return pool;
}

void batch_pool_free(BatchPool* pool) {
    if (!pool) return;
    
    // Free all batches
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->batches[i]) {
            batch_free(pool->batches[i]);
        }
    }
    
    free(pool->batches);
    free(pool->available);
    
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->available_cond);
    
    free(pool);
}

Batch* batch_pool_allocate(BatchPool* pool) {
    if (!pool) return NULL;
    
    pthread_mutex_lock(&pool->mutex);
    
    // Wait for available batch
    while (true) {
        for (uint32_t i = 0; i < pool->pool_size; i++) {
            if (pool->available[i]) {
                pool->available[i] = false;
                pool->allocations++;
                pool->cache_hits++;
                
                Batch* batch = pool->batches[i];
                pthread_mutex_unlock(&pool->mutex);
                return batch;
            }
        }
        
        // No batch available, wait
        pthread_cond_wait(&pool->available_cond, &pool->mutex);
    }
    
    pthread_mutex_unlock(&pool->mutex);
    return NULL;
}

Batch* batch_pool_try_allocate(BatchPool* pool) {
    if (!pool) return NULL;
    
    pthread_mutex_lock(&pool->mutex);
    
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->available[i]) {
            pool->available[i] = false;
            pool->allocations++;
            pool->cache_hits++;
            
            Batch* batch = pool->batches[i];
            pthread_mutex_unlock(&pool->mutex);
            return batch;
        }
    }
    
    pool->cache_misses++;
    pthread_mutex_unlock(&pool->mutex);
    return NULL;
}

void batch_pool_release(BatchPool* pool, Batch* batch) {
    if (!pool || !batch) return;
    
    pthread_mutex_lock(&pool->mutex);
    
    // Find batch in pool
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->batches[i] == batch) {
            pool->available[i] = true;
            pool->releases++;
            
            // Signal that a batch is available
            pthread_cond_signal(&pool->available_cond);
            
            pthread_mutex_unlock(&pool->mutex);
            return;
        }
    }
    
    pthread_mutex_unlock(&pool->mutex);
}

bool batch_pool_resize(BatchPool* pool, uint32_t new_size) {
    if (!pool || new_size == 0) return false;
    
    pthread_mutex_lock(&pool->mutex);
    
    if (new_size == pool->pool_size) {
        pthread_mutex_unlock(&pool->mutex);
        return true;
    }
    
    // TODO: Implement pool resizing
    // This is complex and requires careful handling of in-use batches
    
    pthread_mutex_unlock(&pool->mutex);
    return false;
}

void batch_pool_get_stats(const BatchPool* pool, uint64_t* allocations,
                          uint64_t* releases, uint64_t* cache_hits,
                          uint64_t* cache_misses) {
    if (!pool) return;
    
    if (allocations) *allocations = pool->allocations;
    if (releases) *releases = pool->releases;
    if (cache_hits) *cache_hits = pool->cache_hits;
    if (cache_misses) *cache_misses = pool->cache_misses;
}

void batch_pool_print_stats(const BatchPool* pool) {
    if (!pool) return;
    
    printf("Batch Pool Statistics:\n");
    printf("  Pool Size: %u\n", pool->pool_size);
    printf("  Allocations: %lu\n", pool->allocations);
    printf("  Releases: %lu\n", pool->releases);
    printf("  Cache Hits: %lu\n", pool->cache_hits);
    printf("  Cache Misses: %lu\n", pool->cache_misses);
    
    if (pool->allocations > 0) {
        double hit_rate = 100.0 * pool->cache_hits / pool->allocations;
        printf("  Hit Rate: %.2f%%\n", hit_rate);
    }
}

// ============================================================================
// BATCH DISTRIBUTION FUNCTIONS
// ============================================================================

bool batch_distribute_to_spheres(const Batch* batch, uint32_t num_spheres,
                                 Batch** sphere_batches) {
    if (!batch || num_spheres == 0 || !sphere_batches) return false;
    
    // Split batch evenly across spheres
    return batch_split(batch, num_spheres, sphere_batches);
}

bool batch_assign_to_group(Batch* batch, uint32_t symmetry_group) {
    if (!batch || symmetry_group >= 12) return false;
    
    // Store symmetry group in batch_id (upper bits)
    batch->batch_id = (batch->batch_id & 0x0FFFFFFFFFFFFFFF) | 
                     ((uint64_t)symmetry_group << 60);
    
    return true;
}

bool batch_balance_distribution(Batch** batches, uint32_t num_batches,
                                uint32_t num_spheres, uint32_t* assignments) {
    if (!batches || num_batches == 0 || num_spheres == 0 || !assignments) {
        return false;
    }
    
    // Simple round-robin assignment
    for (uint32_t i = 0; i < num_batches; i++) {
        assignments[i] = i % num_spheres;
    }
    
    return true;
}