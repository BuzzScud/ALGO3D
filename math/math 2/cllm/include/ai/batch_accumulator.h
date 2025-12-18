#ifndef BATCH_ACCUMULATOR_H
#define BATCH_ACCUMULATOR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BatchAccumulator BatchAccumulator;

/**
 * Initialize accumulator
 * Creates buffer with capacity for batch_size * seq_len * 10 tokens
 */
BatchAccumulator* batch_accumulator_create(int batch_size, int seq_len);

/**
 * Add tokens to accumulator
 * Automatically expands capacity if needed
 */
int batch_accumulator_add(BatchAccumulator* acc, uint32_t* tokens, size_t num_tokens);

/**
 * Check if ready for training
 * Returns 1 if buffer has enough tokens for a batch, 0 otherwise
 */
int batch_accumulator_ready(BatchAccumulator* acc);

/**
 * Get batch for training
 * Extracts batch_size * sequence_length tokens from buffer
 * Remaining tokens stay in buffer
 * Caller must free batch_tokens
 */
int batch_accumulator_get_batch(BatchAccumulator* acc, uint32_t** batch_tokens, size_t* batch_size);

/**
 * Clear accumulator
 * Removes all tokens from buffer
 */
void batch_accumulator_clear(BatchAccumulator* acc);

/**
 * Destroy accumulator
 * Frees all resources
 */
void batch_accumulator_destroy(BatchAccumulator* acc);

/**
 * Get current buffer size
 * Returns number of tokens currently in buffer
 */
size_t batch_accumulator_get_size(BatchAccumulator* acc);

/**
 * Get required tokens for next batch
 * Returns number of additional tokens needed to create a batch
 */
size_t batch_accumulator_get_required(BatchAccumulator* acc);

#ifdef __cplusplus
}
#endif

#endif /* BATCH_ACCUMULATOR_H */