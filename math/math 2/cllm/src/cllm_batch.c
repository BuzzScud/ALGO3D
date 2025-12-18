/**
 * @file cllm_batch.c
 * @brief High-Level Batch Generation API for Training
 * 
 * This is the SIMPLE, HIGH-LEVEL API for batch generation.
 * Generates training batches with proper padding and attention masking.
 * 
 * RELATIONSHIP TO OTHER FILES:
 * - This file: Simple batch generation for training loops
 * - infrastructure/cllm_batch.c: Low-level tensor operations, batch splitting,
 *   thread-safe queues, reference counting
 * 
 * USE THIS WHEN:
 * - Creating training batches from token sequences
 * - Iterating over tokens to generate batches
 * - Simple batch validation
 * 
 * USE infrastructure/cllm_batch.c WHEN:
 * - Need multi-dimensional tensor operations
 * - Splitting batches for parallel processing
 * - Thread-safe batch queue management
 * - Reference-counted batch sharing
 * 
 * ANALOGY: This is like fopen() - simple and easy to use.
 *          infrastructure/cllm_batch.c is like open() - powerful but complex.
 */

#include "cllm_training.h"
#include "cllm_tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PAD_TOKEN 0
#define BOS_TOKEN 2
#define EOS_TOKEN 3

/**
 * Batch Structure
 */
typedef struct CLLMBatch {
    uint32_t* input_ids;        // [batch_size * seq_len]
    uint32_t* target_ids;       // [batch_size * seq_len]
    double* attention_mask;     // [batch_size * seq_len] - DOUBLE PRECISION
    uint32_t batch_size;
    uint32_t seq_len;
    uint32_t num_valid_tokens;  // Total non-padding tokens
} CLLMBatch;

// Forward declaration
void cllm_batch_free(CLLMBatch* batch);

/**
 * Batch Iterator Structure
 */
typedef struct {
    uint32_t* tokens;           // Source tokens
    size_t num_tokens;          // Total number of tokens
    size_t current_pos;         // Current position in tokens
    uint32_t batch_size;
    uint32_t seq_len;
    int shuffle;                // Whether to shuffle batches
    int drop_last;              // Whether to drop incomplete last batch
} CLLMBatchIterator;

/**
 * Create Batch
 */
CLLMBatch* cllm_batch_create(uint32_t batch_size, uint32_t seq_len) {
    CLLMBatch* batch = (CLLMBatch*)calloc(1, sizeof(CLLMBatch));
    if (!batch) return NULL;
    
    batch->batch_size = batch_size;
    batch->seq_len = seq_len;
    
    size_t total_size = batch_size * seq_len;
    
    batch->input_ids = (uint32_t*)calloc(total_size, sizeof(uint32_t));
    batch->target_ids = (uint32_t*)calloc(total_size, sizeof(uint32_t));
    batch->attention_mask = (double*)calloc(total_size, sizeof(double));
    
    if (!batch->input_ids || !batch->target_ids || !batch->attention_mask) {
        cllm_batch_free(batch);
        return NULL;
    }
    
    return batch;
}

/**
 * Free Batch
 */
void cllm_batch_free(CLLMBatch* batch) {
    if (!batch) return;
    
    free(batch->input_ids);
    free(batch->target_ids);
    free(batch->attention_mask);
    free(batch);
}

/**
 * Create Batch Iterator
 */
CLLMBatchIterator* cllm_batch_iterator_create(uint32_t* tokens, size_t num_tokens,
                                               uint32_t batch_size, uint32_t seq_len,
                                               int shuffle, int drop_last) {
    if (!tokens || num_tokens == 0) return NULL;
    
    CLLMBatchIterator* iter = (CLLMBatchIterator*)calloc(1, sizeof(CLLMBatchIterator));
    if (!iter) return NULL;
    
    iter->tokens = tokens;
    iter->num_tokens = num_tokens;
    iter->current_pos = 0;
    iter->batch_size = batch_size;
    iter->seq_len = seq_len;
    iter->shuffle = shuffle;
    iter->drop_last = drop_last;
    
    return iter;
}

/**
 * Free Batch Iterator
 */
void cllm_batch_iterator_free(CLLMBatchIterator* iter) {
    if (!iter) return;
    free(iter);
}

/**
 * Reset Batch Iterator
 */
void cllm_batch_iterator_reset(CLLMBatchIterator* iter) {
    if (!iter) return;
    iter->current_pos = 0;
}

/**
 * Get Next Batch
 * 
 * Creates a batch from the token stream with proper padding and masking.
 * Input: tokens[i], Target: tokens[i+1]
 */
CLLMBatch* cllm_batch_iterator_next(CLLMBatchIterator* iter) {
    if (!iter) return NULL;
    
    // Check if we've already processed all data
    if (iter->current_pos >= iter->num_tokens) {
        return NULL;  // No more data
    }
    
    // Check if we have enough tokens left
    // We need +1 because each position needs a next token for target
    size_t tokens_needed = iter->batch_size * iter->seq_len + 1;
    size_t tokens_remaining = iter->num_tokens - iter->current_pos;
    
    if (tokens_remaining < tokens_needed && iter->drop_last) {
        return NULL;  // Not enough for full batch
    }
    
    if (tokens_remaining <= 1) {
        return NULL;  // No tokens left (need at least 2 for input->target)
    }
    
    // Create batch
    CLLMBatch* batch = cllm_batch_create(iter->batch_size, iter->seq_len);
    if (!batch) return NULL;
    
    batch->num_valid_tokens = 0;
    
    // Fill batch
    for (uint32_t b = 0; b < iter->batch_size; b++) {
        uint32_t seq_start = b * iter->seq_len;
        
        // Fill sequence
        for (uint32_t s = 0; s < iter->seq_len; s++) {
            uint32_t idx = seq_start + s;
            size_t token_pos = iter->current_pos + b * iter->seq_len + s;
            
            if (token_pos < iter->num_tokens - 1) {
                // Valid token
                batch->input_ids[idx] = iter->tokens[token_pos];
                batch->target_ids[idx] = iter->tokens[token_pos + 1];
                batch->attention_mask[idx] = 1.0;
                batch->num_valid_tokens++;
            } else {
                // Padding
                batch->input_ids[idx] = PAD_TOKEN;
                batch->target_ids[idx] = PAD_TOKEN;
                batch->attention_mask[idx] = 0.0;
            }
        }
    }
    
    // Advance position
    iter->current_pos += iter->batch_size * iter->seq_len;
    
    return batch;
}

/**
 * Get Number of Batches
 */
size_t cllm_batch_iterator_num_batches(CLLMBatchIterator* iter) {
    if (!iter) return 0;
    
    // We need num_tokens - 1 because each token needs a next token for target
    size_t usable_tokens = (iter->num_tokens > 0) ? iter->num_tokens - 1 : 0;
    size_t tokens_per_batch = iter->batch_size * iter->seq_len;
    size_t num_batches = usable_tokens / tokens_per_batch;
    
    if (!iter->drop_last && (usable_tokens % tokens_per_batch) > 0) {
        num_batches++;
    }
    
    return num_batches;
}

/**
 * Create Batch from Token Array (Simple Version)
 * 
 * Creates a single batch from a token array with automatic padding
 */
CLLMBatch* cllm_create_batch_from_tokens(uint32_t* tokens, size_t num_tokens,
                                          uint32_t batch_size, uint32_t seq_len) {
    if (!tokens || num_tokens == 0) return NULL;
    
    CLLMBatch* batch = cllm_batch_create(batch_size, seq_len);
    if (!batch) return NULL;
    
    batch->num_valid_tokens = 0;
    
    // Fill batch
    for (uint32_t b = 0; b < batch_size; b++) {
        uint32_t seq_start = b * seq_len;
        size_t token_start = b * seq_len;
        
        for (uint32_t s = 0; s < seq_len; s++) {
            uint32_t idx = seq_start + s;
            size_t token_idx = token_start + s;
            
            if (token_idx < num_tokens - 1) {
                batch->input_ids[idx] = tokens[token_idx];
                batch->target_ids[idx] = tokens[token_idx + 1];
                batch->attention_mask[idx] = 1.0;
                batch->num_valid_tokens++;
            } else {
                batch->input_ids[idx] = PAD_TOKEN;
                batch->target_ids[idx] = PAD_TOKEN;
                batch->attention_mask[idx] = 0.0;
            }
        }
    }
    
    return batch;
}

/**
 * Print Batch Statistics
 */
void cllm_batch_print_stats(CLLMBatch* batch) {
    if (!batch) return;
    
    printf("Batch Statistics:\n");
    printf("  Batch size: %u\n", batch->batch_size);
    printf("  Sequence length: %u\n", batch->seq_len);
    printf("  Total tokens: %u\n", batch->batch_size * batch->seq_len);
    printf("  Valid tokens: %u\n", batch->num_valid_tokens);
    printf("  Padding tokens: %u\n", 
           batch->batch_size * batch->seq_len - batch->num_valid_tokens);
    printf("  Padding ratio: %.2f%%\n",
           100.0 * (1.0 - (double)batch->num_valid_tokens / 
                    (batch->batch_size * batch->seq_len)));
}

/**
 * Validate Batch
 * 
 * Checks batch integrity and returns 1 if valid, 0 otherwise
 */
int cllm_batch_validate(CLLMBatch* batch) {
    if (!batch) return 0;
    if (!batch->input_ids || !batch->target_ids || !batch->attention_mask) return 0;
    if (batch->batch_size == 0 || batch->seq_len == 0) return 0;
    
    // Check that attention mask matches padding
    size_t total_size = batch->batch_size * batch->seq_len;
    uint32_t counted_valid = 0;
    
    for (size_t i = 0; i < total_size; i++) {
        if (batch->attention_mask[i] > 0.5) {
            counted_valid++;
            // Valid tokens should not be PAD_TOKEN
            if (batch->input_ids[i] == PAD_TOKEN || batch->target_ids[i] == PAD_TOKEN) {
                fprintf(stderr, "Error: Attention mask indicates valid but tokens are PAD\n");
                return 0;
            }
        } else {
            // Padding tokens should be PAD_TOKEN
            if (batch->input_ids[i] != PAD_TOKEN || batch->target_ids[i] != PAD_TOKEN) {
                fprintf(stderr, "Error: Attention mask indicates padding but tokens are not PAD\n");
                return 0;
            }
        }
    }
    
    if (counted_valid != batch->num_valid_tokens) {
        fprintf(stderr, "Error: Counted valid tokens (%u) != stored valid tokens (%u)\n",
                counted_valid, batch->num_valid_tokens);
        return 0;
    }
    
    return 1;
}

/**
 * Assign batch tokens to spheres (using symmetry groups)
 * 
 * This is THE way to assign batches - using the token's symmetry group.
 * Implementation function used by both the new and deprecated APIs.
 */
static int cllm_batch_assign_impl(
    CLLMBatch* batch,
    CLLMModel* model,
    int* sphere_assignments,
    int num_spheres
) {
    if (!batch || !model || !sphere_assignments) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_batch_assign_by_symmetry\n");
        return -1;
    }
    
    if (num_spheres <= 0 || num_spheres > 12) {
        fprintf(stderr, "ERROR: num_spheres must be between 1 and 12\n");
        return -1;
    }
    
    // Assign each token to a sphere based on its symmetry group
    for (uint32_t i = 0; i < batch->batch_size * batch->seq_len; i++) {
        uint32_t token_id = batch->input_ids[i];
        
        // Skip padding tokens
        if (token_id == PAD_TOKEN) {
            sphere_assignments[i] = -1;  // No sphere assignment
            continue;
        }
        
        // Get token's symmetry group
        uint32_t symmetry_group;
        if (token_id < model->vocab_size && model->tokens) {
            symmetry_group = model->tokens[token_id].symmetry_group;
        } else {
            // Fallback: use modulo
            symmetry_group = token_id % 12;
        }
        
        // Assign to sphere based on symmetry group
        // If we have fewer spheres than groups, distribute evenly
        sphere_assignments[i] = (int)(symmetry_group % num_spheres);
    }
    
    return 0;
}

/**
 * New simplified API - calls implementation
 */
int cllm_batch_assign(
    CLLMBatch* batch,
    CLLMModel* model,
    int* sphere_assignments,
    int num_spheres
) {
    return cllm_batch_assign_impl(batch, model, sphere_assignments, num_spheres);
}

/**
 * DEPRECATED: Use cllm_batch_assign() instead
 */
int cllm_batch_assign_by_symmetry(
    CLLMBatch* batch,
    CLLMModel* model,
    int* sphere_assignments,
    int num_spheres
) {
    return cllm_batch_assign_impl(batch, model, sphere_assignments, num_spheres);
}

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
    CLLMModel* model,
    int* group_counts
) {
    if (!batch || !model || !group_counts) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_batch_symmetry_stats\n");
        return -1;
    }
    
    // Initialize counts
    memset(group_counts, 0, 12 * sizeof(int));
    
    // Count tokens in each symmetry group
    for (uint32_t i = 0; i < batch->batch_size * batch->seq_len; i++) {
        uint32_t token_id = batch->input_ids[i];
        
        // Skip padding tokens
        if (token_id == PAD_TOKEN) continue;
        
        // Get symmetry group
        uint32_t symmetry_group;
        if (token_id < model->vocab_size && model->tokens) {
            symmetry_group = model->tokens[token_id].symmetry_group;
        } else {
            symmetry_group = token_id % 12;
        }
        
        if (symmetry_group < 12) {
            group_counts[symmetry_group]++;
        }
    }
    
    return 0;
}

/**
 * Print batch symmetry distribution
 * 
 * @param batch Batch to analyze
 * @param model CLLM model
 */
void cllm_batch_print_symmetry_distribution(
    CLLMBatch* batch,
    CLLMModel* model
) {
    if (!batch || !model) return;
    
    int group_counts[12];
    if (cllm_batch_symmetry_stats(batch, model, group_counts) != 0) {
        return;
    }
    
    printf("\nBatch Symmetry Distribution:\n");
    int total = 0;
    for (int i = 0; i < 12; i++) {
        total += group_counts[i];
    }
    
    if (total == 0) {
        printf("  No valid tokens in batch\n");
        return;
    }
    
    for (int i = 0; i < 12; i++) {
        if (group_counts[i] > 0) {
            printf("  Group %2d: %5d tokens (%.1f%%)\n",
                   i, group_counts[i],
                   100.0 * group_counts[i] / total);
        }
    }
    printf("  Total: %d tokens\n", total);
}