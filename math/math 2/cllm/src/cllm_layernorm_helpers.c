/**
 * @file cllm_layernorm_helpers.c
 * @brief Layer Normalization Helper Functions (Thread-Centric)
 * 
 * This file implements layer normalization using pure crystalline mathematics
 * (NO math.h) and thread-local storage.
 * 
 * LayerNorm Formula:
 * - LayerNorm(x) = γ * (x - μ) / σ + β
 * - μ = mean(x)
 * - σ = sqrt(variance(x) + ε)
 * - γ, β = learned parameters (stored in thread)
 * 
 * Thread-Centric Design:
 * - γ, β stored in thread CrystallineAbacus
 * - Each thread normalizes its assigned tokens
 * - Pure crystalline sqrt (Newton's method)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ai/cllm.h"
#include "hierarchical_threading.h"
#include "math/abacus.h"

// ============================================================================
// PURE CRYSTALLINE MATH FUNCTIONS (NO math.h!)
// ============================================================================

/**
 * Pure crystalline sqrt using Newton's method
 * x_{n+1} = (x_n + a/x_n) / 2
 */
static double crystalline_sqrt(double x) {
    if (x <= 0.0) return 0.0;
    
    double guess = x;
    for (int i = 0; i < 10; i++) {
        guess = (guess + x / guess) * 0.5;
    }
    return guess;
}

// ============================================================================
// LAYER NORMALIZATION OPERATIONS
// ============================================================================

/**
 * Compute mean of a vector
 */
static double compute_mean(const double* x, uint32_t dim) {
    double sum = 0.0;
    for (uint32_t i = 0; i < dim; i++) {
        sum += x[i];
    }
    return sum / (double)dim;
}

/**
 * Compute variance of a vector
 */
static double compute_variance(const double* x, double mean, uint32_t dim) {
    double sum_sq = 0.0;
    for (uint32_t i = 0; i < dim; i++) {
        double diff = x[i] - mean;
        sum_sq += diff * diff;
    }
    return sum_sq / (double)dim;
}

/**
 * Get layer norm gamma (scale) parameters from thread
 * 
 * @param thread Thread containing the parameters
 * @param output Output buffer [dim]
 * @param dim Dimension
 * @return 0 on success, -1 on error
 */
int cllm_get_layernorm_gamma_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t dim
) {
    if (!thread || !output) {
        return -1;
    }
    
    // TODO: Implement proper CrystallineAbacus extraction
    // For now, initialize gamma to 1.0 (no scaling)
    for (uint32_t i = 0; i < dim; i++) {
        output[i] = 1.0;
    }
    
    return 0;
}

/**
 * Get layer norm beta (shift) parameters from thread
 * 
 * @param thread Thread containing the parameters
 * @param output Output buffer [dim]
 * @param dim Dimension
 * @return 0 on success, -1 on error
 */
int cllm_get_layernorm_beta_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t dim
) {
    if (!thread || !output) {
        return -1;
    }
    
    // TODO: Implement proper CrystallineAbacus extraction
    // For now, initialize beta to 0.0 (no shift)
    for (uint32_t i = 0; i < dim; i++) {
        output[i] = 0.0;
    }
    
    return 0;
}

/**
 * Layer normalization forward pass using thread-local parameters
 * 
 * LayerNorm(x) = γ * (x - μ) / σ + β
 * 
 * @param thread Thread containing layer norm parameters
 * @param input Input vector [dim]
 * @param output Output vector [dim]
 * @param dim Dimension
 * @param epsilon Small constant for numerical stability (e.g., 1e-5)
 * @return 0 on success, -1 on error
 */
int cllm_layernorm_forward_thread(
    HierarchicalThread* thread,
    const double* input,
    double* output,
    uint32_t dim,
    double epsilon
) {
    if (!thread || !input || !output) {
        return -1;
    }
    
    // Allocate temporary buffers
    double* gamma = calloc(dim, sizeof(double));
    double* beta = calloc(dim, sizeof(double));
    
    if (!gamma || !beta) {
        free(gamma);
        free(beta);
        return -1;
    }
    
    // Get parameters from thread
    cllm_get_layernorm_gamma_from_thread(thread, gamma, dim);
    cllm_get_layernorm_beta_from_thread(thread, beta, dim);
    
    // Step 1: Compute mean
    double mean = compute_mean(input, dim);
    
    // Step 2: Compute variance
    double variance = compute_variance(input, mean, dim);
    
    // Step 3: Compute standard deviation (with epsilon for stability)
    double std = crystalline_sqrt(variance + epsilon);
    
    // Step 4: Normalize and scale
    for (uint32_t i = 0; i < dim; i++) {
        // Normalize: (x - μ) / σ
        double normalized = (input[i] - mean) / std;
        
        // Scale and shift: γ * normalized + β
        output[i] = gamma[i] * normalized + beta[i];
    }
    
    // Cleanup
    free(gamma);
    free(beta);
    
    return 0;
}

/**
 * Process multiple tokens through layer norm in parallel
 * 
 * @param model Model containing thread pool
 * @param layer_id Layer ID
 * @param input Input matrix [num_tokens × dim]
 * @param output Output matrix [num_tokens × dim]
 * @param num_tokens Number of tokens
 * @param epsilon Epsilon for numerical stability
 * @return 0 on success, -1 on error
 */
int cllm_layernorm_forward_batch(
    CLLMModel* model,
    uint8_t layer_id,
    const double* input,
    double* output,
    uint32_t num_tokens,
    double epsilon
) {
    if (!model || !input || !output) {
        return -1;
    }
    
    // TODO: Implement parallel batch processing
    // For now, process sequentially
    for (uint32_t t = 0; t < num_tokens; t++) {
        const double* token_input = input + t * model->embedding_dim;
        double* token_output = output + t * model->embedding_dim;
        
        // Get thread for this layer (use control thread for now)
        HierarchicalThread* thread = hierarchical_thread_get(
            model->threads, layer_id, 0
        );
        
        if (!thread) {
            return -1;
        }
        
        int result = cllm_layernorm_forward_thread(
            thread,
            token_input,
            token_output,
            model->embedding_dim,
            epsilon
        );
        
        if (result != 0) {
            return -1;
        }
    }
    
    return 0;
}