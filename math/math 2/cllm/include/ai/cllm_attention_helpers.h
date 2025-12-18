/**
 * @file cllm_attention_helpers.h
 * @brief Helper functions for thread-centric attention mechanism
 */

#ifndef CLLM_ATTENTION_HELPERS_H
#define CLLM_ATTENTION_HELPERS_H

#include <stdint.h>

// Forward declaration to avoid circular dependency
typedef struct HierarchicalThread HierarchicalThread;

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Q/K/V WEIGHT ACCESS FUNCTIONS
// ============================================================================

/**
 * Get Q/K/V projection weights from a thread's CrystallineAbacus
 */
void cllm_get_qkv_weights_from_thread(
    HierarchicalThread* thread,
    double* query_weights,
    double* key_weights,
    double* value_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
);

/**
 * Set Q/K/V projection weights to a thread's CrystallineAbacus
 */
void cllm_set_qkv_weights_to_thread(
    HierarchicalThread* thread,
    const double* query_weights,
    const double* key_weights,
    const double* value_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
);

/**
 * Get output projection weights from a thread's CrystallineAbacus
 */
void cllm_get_output_weights_from_thread(
    HierarchicalThread* thread,
    double* output_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
);

/**
 * Set output projection weights to a thread's CrystallineAbacus
 */
void cllm_set_output_weights_to_thread(
    HierarchicalThread* thread,
    const double* output_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
);

// ============================================================================
// Q/K/V COMPUTATION FUNCTIONS
// ============================================================================

/**
 * Compute Q/K/V for a single token using thread-local weights
 */
void cllm_compute_qkv(
    HierarchicalThread* thread,
    const double* embedding,
    double* Q,
    double* K,
    double* V,
    uint32_t embedding_dim,
    uint32_t num_heads,
    uint32_t head_dim
);

/**
 * Apply output projection using thread-local weights
 */
void cllm_apply_output_projection(
    HierarchicalThread* thread,
    const double* attention_output,
    double* output,
    uint32_t embedding_dim,
    uint32_t num_heads,
    uint32_t head_dim
);

// ============================================================================
// ATTENTION COMPUTATION HELPERS
// ============================================================================

/**
 * Compute attention scores: scores = Q · K^T / sqrt(head_dim)
 */
void cllm_compute_attention_scores(
    const double* Q,
    const double* K,
    double* scores,
    uint32_t num_heads,
    uint32_t seq_len,
    uint32_t head_dim
);

/**
 * Apply softmax to attention scores
 */
void cllm_apply_softmax_to_scores(
    double* scores,
    uint32_t num_heads,
    uint32_t seq_len
);

/**
 * Compute attention output: output = softmax(scores) · V
 */
void cllm_compute_attention_output(
    const double* scores,
    const double* V,
    double* output,
    uint32_t num_heads,
    uint32_t seq_len,
    uint32_t head_dim
);

#ifdef __cplusplus
}
#endif

#endif // CLLM_ATTENTION_HELPERS_H