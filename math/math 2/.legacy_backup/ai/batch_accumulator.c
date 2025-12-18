/**
 * Batch Accumulator
 * 
 * Accumulates small token inputs until batch size is reached
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

typedef struct {
    uint32_t* token_buffer;      // Accumulated tokens
    size_t buffer_size;          // Current buffer size
    size_t buffer_capacity;      // Maximum buffer capacity
    int target_batch_size;       // Target batch size
    int sequence_length;         // Sequence length
    pthread_mutex_t lock;        // Thread safety
} BatchAccumulator;

/**
 * Initialize accumulator
 */
BatchAccumulator* batch_accumulator_create(int batch_size, int seq_len) {
    if (batch_size <= 0 || seq_len <= 0) return NULL;
    
    BatchAccumulator* acc = malloc(sizeof(BatchAccumulator));
    if (!acc) return NULL;
    
    acc->target_batch_size = batch_size;
    acc->sequence_length = seq_len;
    acc->buffer_capacity = batch_size * seq_len * 10; // 10x capacity
    acc->buffer_size = 0;
    
    acc->token_buffer = malloc(acc->buffer_capacity * sizeof(uint32_t));
    if (!acc->token_buffer) {
        free(acc);
        return NULL;
    }
    
    pthread_mutex_init(&acc->lock, NULL);
    
    return acc;
}

/**
 * Add tokens to accumulator
 */
int batch_accumulator_add(BatchAccumulator* acc, uint32_t* tokens, size_t num_tokens) {
    if (!acc || !tokens || num_tokens == 0) return -1;
    
    pthread_mutex_lock(&acc->lock);
    
    // Expand capacity if needed
    if (acc->buffer_size + num_tokens > acc->buffer_capacity) {
        size_t new_capacity = acc->buffer_capacity * 2;
        uint32_t* new_buffer = realloc(acc->token_buffer, new_capacity * sizeof(uint32_t));
        if (!new_buffer) {
            pthread_mutex_unlock(&acc->lock);
            return -1;
        }
        acc->token_buffer = new_buffer;
        acc->buffer_capacity = new_capacity;
    }
    
    // Add tokens
    memcpy(acc->token_buffer + acc->buffer_size, tokens, num_tokens * sizeof(uint32_t));
    acc->buffer_size += num_tokens;
    
    pthread_mutex_unlock(&acc->lock);
    return 0;
}

/**
 * Check if ready for training
 */
int batch_accumulator_ready(BatchAccumulator* acc) {
    if (!acc) return 0;
    
    pthread_mutex_lock(&acc->lock);
    size_t required = acc->target_batch_size * acc->sequence_length;
    int ready = (acc->buffer_size >= required);
    pthread_mutex_unlock(&acc->lock);
    
    return ready;
}

/**
 * Get batch for training
 */
int batch_accumulator_get_batch(BatchAccumulator* acc, uint32_t** batch_tokens, size_t* batch_size) {
    if (!acc || !batch_tokens || !batch_size) return -1;
    
    pthread_mutex_lock(&acc->lock);
    
    size_t required = acc->target_batch_size * acc->sequence_length;
    if (acc->buffer_size < required) {
        pthread_mutex_unlock(&acc->lock);
        return -1; // Not ready
    }
    
    // Allocate batch
    *batch_tokens = malloc(required * sizeof(uint32_t));
    if (!*batch_tokens) {
        pthread_mutex_unlock(&acc->lock);
        return -1;
    }
    
    // Copy tokens
    memcpy(*batch_tokens, acc->token_buffer, required * sizeof(uint32_t));
    *batch_size = required;
    
    // Remove used tokens from buffer
    size_t remaining = acc->buffer_size - required;
    if (remaining > 0) {
        memmove(acc->token_buffer, acc->token_buffer + required, remaining * sizeof(uint32_t));
    }
    acc->buffer_size = remaining;
    
    pthread_mutex_unlock(&acc->lock);
    return 0;
}

/**
 * Clear accumulator
 */
void batch_accumulator_clear(BatchAccumulator* acc) {
    if (!acc) return;
    
    pthread_mutex_lock(&acc->lock);
    acc->buffer_size = 0;
    pthread_mutex_unlock(&acc->lock);
}

/**
 * Destroy accumulator
 */
void batch_accumulator_destroy(BatchAccumulator* acc) {
    if (!acc) return;
    
    pthread_mutex_destroy(&acc->lock);
    free(acc->token_buffer);
    free(acc);
}

/**
 * Get current buffer size
 */
size_t batch_accumulator_get_size(BatchAccumulator* acc) {
    if (!acc) return 0;
    
    pthread_mutex_lock(&acc->lock);
    size_t size = acc->buffer_size;
    pthread_mutex_unlock(&acc->lock);
    
    return size;
}

/**
 * Get required tokens for next batch
 */
size_t batch_accumulator_get_required(BatchAccumulator* acc) {
    if (!acc) return 0;
    
    pthread_mutex_lock(&acc->lock);
    size_t required = acc->target_batch_size * acc->sequence_length;
    size_t needed = (acc->buffer_size < required) ? (required - acc->buffer_size) : 0;
    pthread_mutex_unlock(&acc->lock);
    
    return needed;
}