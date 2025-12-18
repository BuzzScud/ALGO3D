/**
 * @file cllm_layernorm_88d.h
 * @brief Parallel Layer Normalization using 88D Threading System
 * 
 * This header defines the interface for parallel layer normalization
 * computation that leverages the 88D threading system.
 */

#ifndef CLLM_LAYERNORM_88D_H
#define CLLM_LAYERNORM_88D_H

#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// LAYER NORM STATISTICS
// ============================================================================

/**
 * Layer normalization statistics structure
 */
typedef struct {
    uint32_t num_threads;                  // Number of threads used
    uint64_t normalizations_computed;      // Total normalizations computed
    double total_layernorm_time_ms;        // Total computation time
    double normalizations_per_second;      // Normalizations per second
} CLLMLayerNormStats;

// ============================================================================
// LAYER NORMALIZATION OPERATIONS
// ============================================================================

/**
 * Parallel layer normalization
 * 
 * Computes layer normalization in parallel across the 88D threading system.
 * 
 * Algorithm:
 * 1. Compute mean in parallel (distributed by batch)
 * 2. Compute variance in parallel (distributed by batch)
 * 3. Normalize in parallel (distributed by batch)
 * 
 * Formula: output = gamma * (input - mean) / sqrt(variance + epsilon) + beta
 * 
 * @param model CLLM model
 * @param input Input tensor [batch × seq_len × hidden_dim]
 * @param gamma Scale parameter [hidden_dim] (optional, can be NULL)
 * @param beta Shift parameter [hidden_dim] (optional, can be NULL)
 * @param output Output tensor [batch × seq_len × hidden_dim]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param hidden_dim Hidden dimension
 * @param epsilon Small constant for numerical stability (typically 1e-5)
 * @return 0 on success, -1 on error
 */
int cllm_layernorm_forward_parallel(
    CLLMModel* model,
    const float* input,
    const float* gamma,
    const float* beta,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t hidden_dim,
    float epsilon
);

/**
 * Parallel RMS normalization
 * 
 * Computes Root Mean Square (RMS) normalization in parallel.
 * RMS norm is a simplified version of layer norm without mean centering.
 * 
 * Formula: output = gamma * input / sqrt(mean(input^2) + epsilon)
 * 
 * @param model CLLM model
 * @param input Input tensor [batch × seq_len × hidden_dim]
 * @param gamma Scale parameter [hidden_dim] (optional, can be NULL)
 * @param output Output tensor [batch × seq_len × hidden_dim]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param hidden_dim Hidden dimension
 * @param epsilon Small constant for numerical stability (typically 1e-5)
 * @return 0 on success, -1 on error
 */
int cllm_layernorm_rms_forward_parallel(
    CLLMModel* model,
    const float* input,
    const float* gamma,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t hidden_dim,
    float epsilon
);

// ============================================================================
// LAYER NORM STATISTICS
// ============================================================================

/**
 * Get layer normalization statistics
 * 
 * @param model CLLM model
 * @param stats Output: layer norm statistics
 * @return 0 on success, -1 on error
 */
int cllm_layernorm_get_stats(
    CLLMModel* model,
    CLLMLayerNormStats* stats
);

/**
 * Print layer normalization statistics
 * 
 * @param stats Layer norm statistics to print
 */
void cllm_layernorm_print_stats(const CLLMLayerNormStats* stats);

#ifdef __cplusplus
}
#endif

#endif // CLLM_LAYERNORM_88D_H