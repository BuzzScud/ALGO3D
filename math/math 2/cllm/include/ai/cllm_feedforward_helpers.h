/**
 * @file cllm_feedforward_helpers.h
 * @brief Feed-Forward Network Helper Functions (Thread-Centric)
 */

#ifndef CLLM_FEEDFORWARD_HELPERS_H
#define CLLM_FEEDFORWARD_HELPERS_H

#include "cllm.h"
#include "hierarchical_threading.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// FFN WEIGHT ACCESS
// ============================================================================

/**
 * Get FFN W1 weights from thread
 * @return 0 on success, -1 on error
 */
int cllm_get_ffn_w1_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t embedding_dim,
    uint32_t hidden_dim
);

/**
 * Get FFN W2 weights from thread
 * @return 0 on success, -1 on error
 */
int cllm_get_ffn_w2_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t hidden_dim,
    uint32_t embedding_dim
);

// ============================================================================
// FFN FORWARD PASS
// ============================================================================

/**
 * Feed-forward forward pass using thread-local weights
 * @return 0 on success, -1 on error
 */
int cllm_ffn_forward_thread(
    HierarchicalThread* thread,
    const double* input,
    double* output,
    uint32_t embedding_dim,
    uint32_t hidden_dim
);

/**
 * Process multiple tokens through FFN in parallel
 * @return 0 on success, -1 on error
 */
int cllm_ffn_forward_batch(
    CLLMModel* model,
    uint8_t layer_id,
    const double* input,
    double* output,
    uint32_t num_tokens
);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_FEEDFORWARD_HELPERS_H */