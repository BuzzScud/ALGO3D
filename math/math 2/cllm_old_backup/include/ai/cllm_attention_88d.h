/**
 * @file cllm_attention_88d.h
 * @brief Parallel Multi-Head Attention using 88D Threading System
 * 
 * This header defines the interface for parallel multi-head attention
 * computation that leverages the 88D threading system.
 */

#ifndef CLLM_ATTENTION_88D_H
#define CLLM_ATTENTION_88D_H

#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ATTENTION STATISTICS
// ============================================================================

/**
 * Attention statistics structure
 */
typedef struct {
    uint32_t num_threads;              // Number of threads used
    uint64_t attention_ops_computed;   // Total attention operations
    double total_attention_time_ms;    // Total computation time
    double flops_per_second;           // Floating point operations per second
} CLLMAttentionStats;

// ============================================================================
// ATTENTION COMPUTATION
// ============================================================================

/**
 * Parallel multi-head attention computation
 * 
 * Computes multi-head attention in parallel across the 88D threading system.
 * The computation is distributed across threads by splitting attention heads.
 * 
 * Algorithm:
 * 1. Compute Q×K^T in parallel (distributed by heads)
 * 2. Apply softmax in parallel (distributed by heads)
 * 3. Compute attention×V in parallel (distributed by heads)
 * 
 * @param model CLLM model
 * @param query Query tensor [batch × seq_len × (num_heads × head_dim)]
 * @param key Key tensor [batch × seq_len × (num_heads × head_dim)]
 * @param value Value tensor [batch × seq_len × (num_heads × head_dim)]
 * @param mask Attention mask [batch × seq_len × seq_len] (optional, can be NULL)
 * @param output Output tensor [batch × seq_len × (num_heads × head_dim)]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param num_heads Number of attention heads
 * @param head_dim Dimension per head
 * @return 0 on success, -1 on error
 */
int cllm_attention_compute_parallel(
    CLLMModel* model,
    const float* query,
    const float* key,
    const float* value,
    const float* mask,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t num_heads,
    uint32_t head_dim
);

/**
 * Parallel self-attention computation
 * 
 * Convenience function for self-attention where Q = K = V.
 * 
 * @param model CLLM model
 * @param input Input tensor [batch × seq_len × (num_heads × head_dim)]
 * @param mask Attention mask [batch × seq_len × seq_len] (optional)
 * @param output Output tensor [batch × seq_len × (num_heads × head_dim)]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param num_heads Number of attention heads
 * @param head_dim Dimension per head
 * @return 0 on success, -1 on error
 */
int cllm_attention_self_attention_parallel(
    CLLMModel* model,
    const float* input,
    const float* mask,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t num_heads,
    uint32_t head_dim
);

/**
 * Parallel cross-attention computation
 * 
 * Computes cross-attention where Q comes from decoder and K,V from encoder.
 * 
 * @param model CLLM model
 * @param query Query tensor from decoder [batch × query_len × (num_heads × head_dim)]
 * @param context Context tensor from encoder [batch × context_len × (num_heads × head_dim)]
 * @param mask Attention mask [batch × query_len × context_len] (optional)
 * @param output Output tensor [batch × query_len × (num_heads × head_dim)]
 * @param batch_size Batch size
 * @param query_length Query sequence length
 * @param context_length Context sequence length
 * @param num_heads Number of attention heads
 * @param head_dim Dimension per head
 * @return 0 on success, -1 on error
 */
int cllm_attention_cross_attention_parallel(
    CLLMModel* model,
    const float* query,
    const float* context,
    const float* mask,
    float* output,
    uint32_t batch_size,
    uint32_t query_length,
    uint32_t context_length,
    uint32_t num_heads,
    uint32_t head_dim
);

// ============================================================================
// ATTENTION STATISTICS
// ============================================================================

/**
 * Get attention statistics
 * 
 * @param model CLLM model
 * @param stats Output: attention statistics
 * @return 0 on success, -1 on error
 */
int cllm_attention_get_stats(
    CLLMModel* model,
    CLLMAttentionStats* stats
);

/**
 * Print attention statistics
 * 
 * @param stats Attention statistics to print
 */
void cllm_attention_print_stats(const CLLMAttentionStats* stats);

#ifdef __cplusplus
}
#endif

#endif // CLLM_ATTENTION_88D_H