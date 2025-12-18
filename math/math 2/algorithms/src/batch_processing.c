#include "batch_processing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// BATCH MANAGEMENT
// ============================================================================

GenericBatch* batch_create(
    uint64_t batch_id,
    uint32_t epoch_id,
    uint32_t batch_size,
    void* data,
    size_t data_size,
    void (*free_data)(void*)
) {
    GenericBatch* batch = calloc(1, sizeof(GenericBatch));
    if (!batch) {
        fprintf(stderr, "ERROR: Failed to allocate batch\n");
        return NULL;
    }
    
    batch->batch_id = batch_id;
    batch->epoch_id = epoch_id;
    batch->batch_size = batch_size;
    batch->data = data;
    batch->data_size = data_size;
    batch->free_data = free_data;
    batch->total_memory = sizeof(GenericBatch) + data_size;
    batch->is_pooled = false;
    batch->is_processed = false;
    batch->processing_time = 0.0;
    
    atomic_init(&batch->ref_count, 1);
    pthread_mutex_init(&batch->mutex, NULL);
    
    return batch;
}

void batch_free(GenericBatch* batch) {
    if (!batch) return;
    
    // Free user data if provided
    if (batch->data && batch->free_data) {
        batch->free_data(batch->data);
    }
    
    pthread_mutex_destroy(&batch->mutex);
    free(batch);
}

GenericBatch* batch_copy(const GenericBatch* src) {
    if (!src) return NULL;
    
    // Note: This creates a shallow copy of the data pointer
    // For deep copy, user should provide custom copy function
    GenericBatch* dst = batch_create(
        src->batch_id,
        src->epoch_id,
        src->batch_size,
        src->data,  // Shallow copy
        src->data_size,
        NULL  // Don't free data on copy
    );
    
    if (dst) {
        dst->is_pooled = src->is_pooled;
        dst->is_processed = src->is_processed;
        dst->processing_time = src->processing_time;
    }
    
    return dst;
}

void batch_retain(GenericBatch* batch) {
    if (!batch) return;
    atomic_fetch_add(&batch->ref_count, 1);
}

void batch_release(GenericBatch* batch) {
    if (!batch) return;
    
    int old_count = atomic_fetch_sub(&batch->ref_count, 1);
    if (old_count == 1) {
        // Last reference, free the batch
        batch_free(batch);
    }
}

int batch_get_ref_count(const GenericBatch* batch) {
    if (!batch) return 0;
    return atomic_load(&batch->ref_count);
}

void batch_mark_processed(GenericBatch* batch, double processing_time) {
    if (!batch) return;
    
    pthread_mutex_lock(&batch->mutex);
    batch->is_processed = true;
    batch->processing_time = processing_time;
    pthread_mutex_unlock(&batch->mutex);
}

bool batch_is_processed(const GenericBatch* batch) {
    if (!batch) return false;
    return batch->is_processed;
}

// ============================================================================
// BATCH QUEUE
// ============================================================================

BatchQueue* batch_queue_create(uint32_t capacity) {
    BatchQueue* queue = calloc(1, sizeof(BatchQueue));
    if (!queue) {
        fprintf(stderr, "ERROR: Failed to allocate batch queue\n");
        return NULL;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->capacity = capacity;
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
    
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    
    free(queue);
}

bool batch_queue_enqueue(BatchQueue* queue, GenericBatch* batch) {
    if (!queue || !batch) return false;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Wait while queue is full
    while (queue->capacity > 0 && queue->size >= queue->capacity && !queue->closed) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    if (queue->closed) {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    
    // Create new node
    BatchQueueNode* node = malloc(sizeof(BatchQueueNode));
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

bool batch_queue_try_enqueue(BatchQueue* queue, GenericBatch* batch) {
    if (!queue || !batch) return false;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Check if queue is full or closed
    if (queue->closed || (queue->capacity > 0 && queue->size >= queue->capacity)) {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    
    // Create new node
    BatchQueueNode* node = malloc(sizeof(BatchQueueNode));
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

GenericBatch* batch_queue_dequeue(BatchQueue* queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Wait while queue is empty and not closed
    while (queue->size == 0 && !queue->closed) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    // If closed and empty, return NULL
    if (queue->size == 0 && queue->closed) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    // Remove from queue
    BatchQueueNode* node = queue->head;
    GenericBatch* batch = node->batch;
    
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

GenericBatch* batch_queue_try_dequeue(BatchQueue* queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Check if queue is empty
    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    // Remove from queue
    BatchQueueNode* node = queue->head;
    GenericBatch* batch = node->batch;
    
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

GenericBatch* batch_queue_peek(BatchQueue* queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    GenericBatch* batch = NULL;
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
    if (queue->capacity == 0) return false;  // Unlimited capacity
    return queue->size >= queue->capacity;
}

void batch_queue_close(BatchQueue* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    queue->closed = true;
    
    // Wake up all waiting threads
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
}

bool batch_queue_is_closed(const BatchQueue* queue) {
    if (!queue) return true;
    return queue->closed;
}

void batch_queue_clear(BatchQueue* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Free all nodes and batches
    BatchQueueNode* node = queue->head;
    while (node) {
        BatchQueueNode* next = node->next;
        batch_release(node->batch);
        free(node);
        node = next;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    
    pthread_mutex_unlock(&queue->mutex);
}

// ============================================================================
// BATCH POOL
// ============================================================================

BatchPool* batch_pool_create(
    uint32_t pool_size,
    uint32_t batch_size,
    size_t data_size
) {
    if (pool_size == 0) {
        fprintf(stderr, "ERROR: Invalid pool size\n");
        return NULL;
    }
    
    BatchPool* pool = calloc(1, sizeof(BatchPool));
    if (!pool) {
        fprintf(stderr, "ERROR: Failed to allocate batch pool\n");
        return NULL;
    }
    
    pool->pool_size = pool_size;
    pool->batch_size = batch_size;
    pool->data_size = data_size;
    
    // Allocate arrays
    pool->batches = calloc(pool_size, sizeof(GenericBatch*));
    pool->available = calloc(pool_size, sizeof(bool));
    
    if (!pool->batches || !pool->available) {
        fprintf(stderr, "ERROR: Failed to allocate pool arrays\n");
        free(pool->batches);
        free(pool->available);
        free(pool);
        return NULL;
    }
    
    // Pre-allocate batches
    for (uint32_t i = 0; i < pool_size; i++) {
        void* data = calloc(1, data_size);
        if (!data) {
            fprintf(stderr, "ERROR: Failed to allocate batch data\n");
            // Clean up
            for (uint32_t j = 0; j < i; j++) {
                batch_free(pool->batches[j]);
            }
            free(pool->batches);
            free(pool->available);
            free(pool);
            return NULL;
        }
        
        pool->batches[i] = batch_create(i, 0, batch_size, data, data_size, free);
        if (!pool->batches[i]) {
            free(data);
            // Clean up
            for (uint32_t j = 0; j < i; j++) {
                batch_free(pool->batches[j]);
            }
            free(pool->batches);
            free(pool->available);
            free(pool);
            return NULL;
        }
        
        pool->batches[i]->is_pooled = true;
        pool->available[i] = true;
    }
    
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->available_cond, NULL);
    
    atomic_init(&pool->allocations, 0);
    atomic_init(&pool->releases, 0);
    atomic_init(&pool->hits, 0);
    atomic_init(&pool->misses, 0);
    
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

GenericBatch* batch_pool_allocate(BatchPool* pool) {
    if (!pool) return NULL;
    
    pthread_mutex_lock(&pool->mutex);
    
    atomic_fetch_add(&pool->allocations, 1);
    
    // Wait for available batch
    while (1) {
        for (uint32_t i = 0; i < pool->pool_size; i++) {
            if (pool->available[i]) {
                pool->available[i] = false;
                GenericBatch* batch = pool->batches[i];
                
                // Reset batch state
                batch->is_processed = false;
                batch->processing_time = 0.0;
                atomic_store(&batch->ref_count, 1);
                
                atomic_fetch_add(&pool->hits, 1);
                
                pthread_mutex_unlock(&pool->mutex);
                return batch;
            }
        }
        
        // No batch available, wait
        pthread_cond_wait(&pool->available_cond, &pool->mutex);
    }
}

GenericBatch* batch_pool_try_allocate(BatchPool* pool) {
    if (!pool) return NULL;
    
    pthread_mutex_lock(&pool->mutex);
    
    atomic_fetch_add(&pool->allocations, 1);
    
    // Try to find available batch
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->available[i]) {
            pool->available[i] = false;
            GenericBatch* batch = pool->batches[i];
            
            // Reset batch state
            batch->is_processed = false;
            batch->processing_time = 0.0;
            atomic_store(&batch->ref_count, 1);
            
            atomic_fetch_add(&pool->hits, 1);
            
            pthread_mutex_unlock(&pool->mutex);
            return batch;
        }
    }
    
    // No batch available
    atomic_fetch_add(&pool->misses, 1);
    
    pthread_mutex_unlock(&pool->mutex);
    return NULL;
}

void batch_pool_release(BatchPool* pool, GenericBatch* batch) {
    if (!pool || !batch) return;
    
    pthread_mutex_lock(&pool->mutex);
    
    atomic_fetch_add(&pool->releases, 1);
    
    // Find batch in pool
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->batches[i] == batch) {
            pool->available[i] = true;
            
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
    
    // For simplicity, only support growing the pool
    if (new_size < pool->pool_size) {
        fprintf(stderr, "ERROR: Shrinking pool not supported\n");
        pthread_mutex_unlock(&pool->mutex);
        return false;
    }
    
    // Reallocate arrays
    GenericBatch** new_batches = realloc(pool->batches, new_size * sizeof(GenericBatch*));
    bool* new_available = realloc(pool->available, new_size * sizeof(bool));
    
    if (!new_batches || !new_available) {
        pthread_mutex_unlock(&pool->mutex);
        return false;
    }
    
    pool->batches = new_batches;
    pool->available = new_available;
    
    // Allocate new batches
    for (uint32_t i = pool->pool_size; i < new_size; i++) {
        void* data = calloc(1, pool->data_size);
        if (!data) {
            pthread_mutex_unlock(&pool->mutex);
            return false;
        }
        
        pool->batches[i] = batch_create(i, 0, pool->batch_size, data, pool->data_size, free);
        if (!pool->batches[i]) {
            free(data);
            pthread_mutex_unlock(&pool->mutex);
            return false;
        }
        
        pool->batches[i]->is_pooled = true;
        pool->available[i] = true;
    }
    
    pool->pool_size = new_size;
    
    pthread_mutex_unlock(&pool->mutex);
    return true;
}

void batch_pool_get_stats(
    const BatchPool* pool,
    uint64_t* allocations,
    uint64_t* releases,
    uint64_t* hits,
    uint64_t* misses
) {
    if (!pool) return;
    
    if (allocations) *allocations = atomic_load(&pool->allocations);
    if (releases) *releases = atomic_load(&pool->releases);
    if (hits) *hits = atomic_load(&pool->hits);
    if (misses) *misses = atomic_load(&pool->misses);
}

double batch_pool_get_efficiency(const BatchPool* pool) {
    if (!pool) return 0.0;
    
    uint64_t hits = atomic_load(&pool->hits);
    uint64_t misses = atomic_load(&pool->misses);
    uint64_t total = hits + misses;
    
    if (total == 0) return 0.0;
    
    return (double)hits / (double)total;
}

void batch_pool_print_stats(const BatchPool* pool) {
    if (!pool) return;
    
    uint64_t allocations, releases, hits, misses;
    batch_pool_get_stats(pool, &allocations, &releases, &hits, &misses);
    
    printf("Batch Pool Statistics:\n");
    printf("  Pool Size: %u\n", pool->pool_size);
    printf("  Allocations: %lu\n", allocations);
    printf("  Releases: %lu\n", releases);
    printf("  Hits: %lu\n", hits);
    printf("  Misses: %lu\n", misses);
    printf("  Efficiency: %.2f%%\n", batch_pool_get_efficiency(pool) * 100.0);
}

// ============================================================================
// BATCH SPLITTING AND MERGING
// ============================================================================

bool batch_split(
    const GenericBatch* batch,
    uint32_t num_splits,
    GenericBatch** splits
) {
    if (!batch || num_splits == 0 || !splits) return false;
    
    if (batch->batch_size < num_splits) {
        fprintf(stderr, "ERROR: Cannot split batch into more parts than items\n");
        return false;
    }
    
    uint32_t items_per_split = batch->batch_size / num_splits;
    uint32_t remainder = batch->batch_size % num_splits;
    
    for (uint32_t i = 0; i < num_splits; i++) {
        uint32_t split_size = items_per_split + (i < remainder ? 1 : 0);
        
        // Create split batch (shallow copy of data)
        splits[i] = batch_create(
            batch->batch_id * 1000 + i,  // Unique ID
            batch->epoch_id,
            split_size,
            batch->data,  // Shallow copy
            batch->data_size / num_splits,
            NULL  // Don't free data
        );
        
        if (!splits[i]) {
            // Clean up
            for (uint32_t j = 0; j < i; j++) {
                batch_free(splits[j]);
            }
            return false;
        }
    }
    
    return true;
}

GenericBatch* batch_merge(GenericBatch** batches, uint32_t num_batches) {
    if (!batches || num_batches == 0) return NULL;
    
    // Calculate total batch size
    uint32_t total_size = 0;
    size_t total_data_size = 0;
    
    for (uint32_t i = 0; i < num_batches; i++) {
        if (!batches[i]) return NULL;
        total_size += batches[i]->batch_size;
        total_data_size += batches[i]->data_size;
    }
    
    // Create merged batch
    void* merged_data = calloc(1, total_data_size);
    if (!merged_data) return NULL;
    
    GenericBatch* merged = batch_create(
        batches[0]->batch_id,  // Use first batch ID
        batches[0]->epoch_id,
        total_size,
        merged_data,
        total_data_size,
        free
    );
    
    if (!merged) {
        free(merged_data);
        return NULL;
    }
    
    // Copy data from all batches
    size_t offset = 0;
    for (uint32_t i = 0; i < num_batches; i++) {
        if (batches[i]->data && batches[i]->data_size > 0) {
            memcpy((char*)merged_data + offset, batches[i]->data, batches[i]->data_size);
            offset += batches[i]->data_size;
        }
    }
    
    return merged;
}
