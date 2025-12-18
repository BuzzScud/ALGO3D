/**
 * @file cllm_inference_88d.h
 * @brief Parallel Inference Operations using 88D Threading System
 * 
 * This header defines the interface for parallel inference operations that
 * leverage the 88D threading system for efficient distributed computation.
 */

#ifndef CLLM_INFERENCE_88D_H
#define CLLM_INFERENCE_88D_H

#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// INFERENCE STATISTICS
// ============================================================================

/**
 * Inference statistics structure
 */
typedef struct {
    uint32_t num_threads;           // Number of threads used
    uint64_t tokens_generated;      // Total tokens generated
    double total_inference_time_ms; // Total inference time
    double tokens_per_second;       // Throughput
    double cache_hit_rate;          // KV cache hit rate
} CLLMInferenceStats;

// ============================================================================
// INFERENCE OPERATIONS
// ============================================================================

/**
 * Parallel inference across 88D threads
 * 
 * Distributes inference computation across all available threads in the
 * 88D threading system. Work is distributed geometrically based on the
 * Platonic solid structure.
 * 
 * @param model CLLM model
 * @param input_ids Input token IDs [batch_size × seq_length]
 * @param logits Output logits [batch_size × vocab_size]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @return 0 on success, -1 on error
 */
int cllm_inference_forward_parallel(
    CLLMModel* model,
    const int32_t* input_ids,
    float* logits,
    uint32_t batch_size,
    uint32_t seq_length
);

// ============================================================================
// TOKEN GENERATION
// ============================================================================

/**
 * Generate next token with parallel computation
 * 
 * Uses the 88D threading system to compute logits in parallel, then samples
 * the next token based on the specified sampling strategy.
 * 
 * @param model CLLM model
 * @param input_ids Input token IDs [seq_length]
 * @param seq_length Current sequence length
 * @param next_token Output: next token ID
 * @param temperature Sampling temperature (0.0 = greedy, >1.0 = more random)
 * @param top_k Top-k sampling parameter (0 = disabled)
 * @return 0 on success, -1 on error
 */
int cllm_inference_generate_token(
    CLLMModel* model,
    const int32_t* input_ids,
    uint32_t seq_length,
    int32_t* next_token,
    float temperature,
    uint32_t top_k
);

/**
 * Generate sequence with parallel computation
 * 
 * Generates a complete sequence by repeatedly calling generate_token.
 * Uses the 88D threading system for efficient parallel computation.
 * 
 * @param model CLLM model
 * @param prompt_ids Prompt token IDs [prompt_length]
 * @param prompt_length Prompt length
 * @param output_ids Output token IDs [max_length]
 * @param max_length Maximum sequence length
 * @param temperature Sampling temperature
 * @param top_k Top-k sampling parameter
 * @return Actual sequence length on success, -1 on error
 */
int cllm_inference_generate_sequence(
    CLLMModel* model,
    const int32_t* prompt_ids,
    uint32_t prompt_length,
    int32_t* output_ids,
    uint32_t max_length,
    float temperature,
    uint32_t top_k
);

// ============================================================================
// BATCH INFERENCE
// ============================================================================

/**
 * Batch inference with parallel computation
 * 
 * Processes multiple sequences in parallel using the 88D threading system.
 * More efficient than processing sequences one by one.
 * 
 * @param model CLLM model
 * @param input_ids Input token IDs [batch_size × seq_length]
 * @param logits Output logits [batch_size × vocab_size]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @return 0 on success, -1 on error
 */
int cllm_inference_batch_forward(
    CLLMModel* model,
    const int32_t* input_ids,
    float* logits,
    uint32_t batch_size,
    uint32_t seq_length
);

// ============================================================================
// ATTENTION OPERATIONS
// ============================================================================

/**
 * Parallel attention computation
 * 
 * Computes multi-head attention in parallel across the 88D threading system.
 * Each thread handles a subset of attention heads or sequence positions.
 * 
 * @param model CLLM model
 * @param query Query tensor [batch_size × seq_length × hidden_size]
 * @param key Key tensor [batch_size × seq_length × hidden_size]
 * @param value Value tensor [batch_size × seq_length × hidden_size]
 * @param output Output tensor [batch_size × seq_length × hidden_size]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param num_heads Number of attention heads
 * @return 0 on success, -1 on error
 */
int cllm_inference_attention_parallel(
    CLLMModel* model,
    const float* query,
    const float* key,
    const float* value,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t num_heads
);

// ============================================================================
// KV CACHE MANAGEMENT
// ============================================================================

/**
 * Initialize KV cache for efficient inference
 * 
 * Allocates memory for key-value cache to speed up autoregressive generation.
 * The cache is distributed across threads in the 88D system.
 * 
 * @param model CLLM model
 * @param max_batch_size Maximum batch size
 * @param max_seq_length Maximum sequence length
 * @return 0 on success, -1 on error
 */
int cllm_inference_init_kv_cache(
    CLLMModel* model,
    uint32_t max_batch_size,
    uint32_t max_seq_length
);

/**
 * Update KV cache with new keys and values
 * 
 * @param model CLLM model
 * @param new_keys New key vectors
 * @param new_values New value vectors
 * @param batch_idx Batch index
 * @param position Position in sequence
 * @return 0 on success, -1 on error
 */
int cllm_inference_update_kv_cache(
    CLLMModel* model,
    const float* new_keys,
    const float* new_values,
    uint32_t batch_idx,
    uint32_t position
);

/**
 * Clear KV cache
 * 
 * @param model CLLM model
 * @return 0 on success, -1 on error
 */
int cllm_inference_clear_kv_cache(CLLMModel* model);

// ============================================================================
// INFERENCE STATISTICS
// ============================================================================

/**
 * Get inference statistics
 * 
 * @param model CLLM model
 * @param stats Output: inference statistics
 * @return 0 on success, -1 on error
 */
int cllm_inference_get_stats(
    CLLMModel* model,
    CLLMInferenceStats* stats
);

/**
 * Print inference statistics
 * 
 * @param stats Inference statistics to print
 */
void cllm_inference_print_stats(const CLLMInferenceStats* stats);

#ifdef __cplusplus
}
#endif

#endif // CLLM_INFERENCE_88D_H