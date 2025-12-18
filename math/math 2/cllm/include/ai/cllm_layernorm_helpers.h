/**
 * @file cllm_layernorm_helpers.h
 * @brief Layer Normalization Helper Functions (Thread-Centric)
 */

#ifndef CLLM_LAYERNORM_HELPERS_H
#define CLLM_LAYERNORM_HELPERS_H

#include "cllm.h"
#include "hierarchical_threading.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// LAYER NORM PARAMETER ACCESS
// ============================================================================

/**
 * Get layer norm gamma (scale) parameters from thread
 * @return 0 on success, -1 on error
 */
int cllm_get_layernorm_gamma_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t dim
);

/**
 * Get layer norm beta (shift) parameters from thread
 * @return 0 on success, -1 on error
 */
int cllm_get_layernorm_beta_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t dim
);

// ============================================================================
// LAYER NORM FORWARD PASS
// ============================================================================

/**
 * Layer normalization forward pass using thread-local parameters
 * @return 0 on success, -1 on error
 */
int cllm_layernorm_forward_thread(
    HierarchicalThread* thread,
    const double* input,
    double* output,
    uint32_t dim,
    double epsilon
);

/**
 * Process multiple tokens through layer norm in parallel
 * @return 0 on success, -1 on error
 */
int cllm_layernorm_forward_batch(
    CLLMModel* model,
    uint8_t layer_id,
    const double* input,
    double* output,
    uint32_t num_tokens,
    double epsilon
);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_LAYERNORM_HELPERS_H */