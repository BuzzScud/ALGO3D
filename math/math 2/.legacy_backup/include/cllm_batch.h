/**
 * CLLM Batch Generation Header
 * 
 * Provides batch generation with padding and attention masking
 */

#ifndef CLLM_BATCH_H
#define CLLM_BATCH_H

#include <stdint.h>
#include <stddef.h>

// Forward declaration for CLLMModel (defined in cllm.h)
struct CLLMModel;

/**
 * Batch Structure
 */
typedef struct {
    uint32_t* input_ids;        // [batch_size * seq_len]
    uint32_t* target_ids;       // [batch_size * seq_len]
    float* attention_mask;      // [batch_size * seq_len]
    uint32_t batch_size;
    uint32_t seq_len;
    uint32_t num_valid_tokens;  // Total non-padding tokens
} CLLMBatch;

/**
 * Batch Iterator Structure (opaque)
 */
typedef struct CLLMBatchIterator CLLMBatchIterator;

/**
 * Create Batch
 * 
 * Allocates a new batch with given dimensions
 */
CLLMBatch* cllm_batch_create(uint32_t batch_size, uint32_t seq_len);

/**
 * Free Batch
 */
void cllm_batch_free(CLLMBatch* batch);

/**
 * Create Batch Iterator
 * 
 * Creates an iterator over a token array that generates batches
 * 
 * @param tokens Source token array
 * @param num_tokens Number of tokens in array
 * @param batch_size Batch size
 * @param seq_len Sequence length
 * @param shuffle Whether to shuffle (not implemented yet)
 * @param drop_last Whether to drop incomplete last batch
 */
CLLMBatchIterator* cllm_batch_iterator_create(uint32_t* tokens, size_t num_tokens,
                                               uint32_t batch_size, uint32_t seq_len,
                                               int shuffle, int drop_last);

/**
 * Free Batch Iterator
 */
void cllm_batch_iterator_free(CLLMBatchIterator* iter);

/**
 * Reset Batch Iterator
 * 
 * Resets iterator to beginning
 */
void cllm_batch_iterator_reset(CLLMBatchIterator* iter);

/**
 * Get Next Batch
 * 
 * Returns next batch from iterator, or NULL if no more batches
 * Caller is responsible for freeing the returned batch
 */
CLLMBatch* cllm_batch_iterator_next(CLLMBatchIterator* iter);

/**
 * Get Number of Batches
 * 
 * Returns total number of batches that will be generated
 */
size_t cllm_batch_iterator_num_batches(CLLMBatchIterator* iter);

/**
 * Create Batch from Token Array
 * 
 * Simple helper to create a single batch from tokens
 */
CLLMBatch* cllm_create_batch_from_tokens(uint32_t* tokens, size_t num_tokens,
                                          uint32_t batch_size, uint32_t seq_len);

/**
 * Print Batch Statistics
 */
void cllm_batch_print_stats(CLLMBatch* batch);

/**
 * Validate Batch
 * 
 * Checks batch integrity, returns 1 if valid, 0 otherwise
 */
int cllm_batch_validate(CLLMBatch* batch);

/**
 * Assign batch tokens to spheres (using symmetry groups)
 * 
 * This is THE way to assign batches - using the token's symmetry group
 * from the 12-fold kissing spheres structure. Since this is the only
 * assignment method, the name is simplified (no "by_symmetry" suffix).
 * 
 * @param batch Batch to analyze
 * @param model CLLM model with token symmetry information
 * @param sphere_assignments Output array [batch_size * seq_len] with sphere IDs
 * @param num_spheres Number of available spheres (typically 12)
 * 
 * @return 0 on success, -1 on error
 */
int cllm_batch_assign(
    CLLMBatch* batch,
    struct CLLMModel* model,
    int* sphere_assignments,
    int num_spheres
);

/**
 * DEPRECATED: Use cllm_batch_assign() instead
 * Kept for backward compatibility only.
 */
__attribute__((deprecated("Use cllm_batch_assign() instead")))
int cllm_batch_assign_by_symmetry(
    CLLMBatch* batch,
    struct CLLMModel* model,
    int* sphere_assignments,
    int num_spheres
);

/**
 * Get batch distribution statistics by symmetry group
 * 
 * Analyzes how tokens in the batch are distributed across symmetry groups.
 * Useful for understanding load balance and batch composition.
 * 
 * @param batch Batch to analyze
 * @param model CLLM model
 * @param group_counts Output array [12] with token counts per group
 * 
 * @return 0 on success, -1 on error
 */
int cllm_batch_symmetry_stats(
    CLLMBatch* batch,
    struct CLLMModel* model,
    int* group_counts
);

/**
 * Print batch symmetry distribution
 * 
 * @param batch Batch to analyze
 * @param model CLLM model
 */
void cllm_batch_print_symmetry_distribution(
    CLLMBatch* batch,
    struct CLLMModel* model
);

#endif // CLLM_BATCH_H