/**
 * @file cllm_feedforward_helpers.c
 * @brief Feed-Forward Network Helper Functions (Thread-Centric)
 * 
 * This file implements feed-forward networks using thread-local storage
 * and pure crystalline mathematics (NO math.h).
 * 
 * FFN Architecture:
 * - FFN(x) = W2 * activation(W1 * x + b1) + b2
 * - W1: [embedding_dim × hidden_dim]
 * - W2: [hidden_dim × embedding_dim]
 * - b1: [hidden_dim]
 * - b2: [embedding_dim]
 * 
 * Thread-Centric Design:
 * - All weights stored in thread CrystallineAbacus
 * - Each thread processes its assigned tokens
 * - Control thread coordinates operations
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ai/cllm.h"
#include "hierarchical_threading.h"
#include "math/abacus.h"

// ============================================================================
// PURE CRYSTALLINE ACTIVATION FUNCTIONS (NO math.h!)
// ============================================================================

/**
 * ReLU activation: max(0, x)
 * Pure crystalline - no external dependencies
 */
static double crystalline_relu(double x) {
    return (x > 0.0) ? x : 0.0;
}

/**
 * GELU activation (Gaussian Error Linear Unit)
 * GELU(x) ≈ 0.5 * x * (1 + tanh(sqrt(2/π) * (x + 0.044715 * x^3)))
 * 
 * Using pure crystalline math - no math.h
 */
static double crystalline_gelu(double x) {
    // Constants (pure crystalline - no math.h)
    const double sqrt_2_over_pi = 0.7978845608028654;  // sqrt(2/π)
    const double coeff = 0.044715;
    
    // Compute x^3
    double x3 = x * x * x;
    
    // Inner value for tanh
    double inner = sqrt_2_over_pi * (x + coeff * x3);
    
    // tanh approximation using Taylor series (pure crystalline)
    double tanh_approx;
    if (inner > 5.0) {
        tanh_approx = 1.0;
    } else if (inner < -5.0) {
        tanh_approx = -1.0;
    } else {
        // tanh(x) ≈ x - x^3/3 + 2x^5/15 - 17x^7/315
        double inner2 = inner * inner;
        double inner4 = inner2 * inner2;
        tanh_approx = inner * (1.0 - inner2 * (1.0/3.0 - inner2 * (2.0/15.0 - inner2 * (17.0/315.0))));
    }
    
    return 0.5 * x * (1.0 + tanh_approx);
}

/**
 * Swish activation: x * sigmoid(x)
 * sigmoid(x) = 1 / (1 + exp(-x))
 * 
 * Using pure crystalline exp (Taylor series)
 */
static double crystalline_swish(double x) {
    // exp(-x) using Taylor series
    double neg_x = -x;
    double exp_neg_x = 1.0;
    double term = 1.0;
    
    for (int n = 1; n < 20; n++) {
        term *= neg_x / n;
        exp_neg_x += term;
        if (term < 1e-10 && term > -1e-10) break;
    }
    
    // sigmoid(x) = 1 / (1 + exp(-x))
    double sigmoid = 1.0 / (1.0 + exp_neg_x);
    
    return x * sigmoid;
}

// ============================================================================
// FEED-FORWARD NETWORK OPERATIONS
// ============================================================================

/**
 * Get FFN W1 weights from thread
 * 
 * @param thread Thread containing the weights
 * @param output Output buffer [embedding_dim × hidden_dim]
 * @param embedding_dim Input dimension
 * @param hidden_dim Hidden dimension
 * @return 0 on success, -1 on error
 */
int cllm_get_ffn_w1_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t embedding_dim,
    uint32_t hidden_dim
) {
    if (!thread || !output) {
        return -1;
    }
    
    // TODO: Implement proper CrystallineAbacus extraction
    // For now, use cached_qkv as temporary storage
    if (thread->cached_qkv && thread->cached_qkv_size >= embedding_dim * hidden_dim) {
        memcpy(output, thread->cached_qkv, embedding_dim * hidden_dim * sizeof(double));
        return 0;
    }
    
    // Fallback: Initialize with small random values
    for (uint32_t i = 0; i < embedding_dim * hidden_dim; i++) {
        output[i] = ((double)(i % 100) - 50.0) / 1000.0;  // Range: [-0.05, 0.05]
    }
    
    return 0;
}

/**
 * Get FFN W2 weights from thread
 */
int cllm_get_ffn_w2_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t hidden_dim,
    uint32_t embedding_dim
) {
    if (!thread || !output) {
        return -1;
    }
    
    // TODO: Implement proper CrystallineAbacus extraction
    // For now, initialize with small random values
    for (uint32_t i = 0; i < hidden_dim * embedding_dim; i++) {
        output[i] = ((double)(i % 100) - 50.0) / 1000.0;
    }
    
    return 0;
}

/**
 * Feed-forward forward pass using thread-local weights
 * 
 * @param thread Thread containing FFN weights
 * @param input Input vector [embedding_dim]
 * @param output Output vector [embedding_dim]
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension
 * @return 0 on success, -1 on error
 */
int cllm_ffn_forward_thread(
    HierarchicalThread* thread,
    const double* input,
    double* output,
    uint32_t embedding_dim,
    uint32_t hidden_dim
) {
    if (!thread || !input || !output) {
        return -1;
    }
    
    // Allocate temporary buffers
    double* w1 = calloc(embedding_dim * hidden_dim, sizeof(double));
    double* w2 = calloc(hidden_dim * embedding_dim, sizeof(double));
    double* hidden = calloc(hidden_dim, sizeof(double));
    
    if (!w1 || !w2 || !hidden) {
        free(w1);
        free(w2);
        free(hidden);
        return -1;
    }
    
    // Get weights from thread
    cllm_get_ffn_w1_from_thread(thread, w1, embedding_dim, hidden_dim);
    cllm_get_ffn_w2_from_thread(thread, w2, hidden_dim, embedding_dim);
    
    // Step 1: hidden = W1 * input (+ b1, but bias is 0 for now)
    for (uint32_t h = 0; h < hidden_dim; h++) {
        double sum = 0.0;
        for (uint32_t e = 0; e < embedding_dim; e++) {
            sum += w1[h * embedding_dim + e] * input[e];
        }
        hidden[h] = sum;
    }
    
    // Step 2: Apply activation (GELU)
    for (uint32_t h = 0; h < hidden_dim; h++) {
        hidden[h] = crystalline_gelu(hidden[h]);
    }
    
    // Step 3: output = W2 * hidden (+ b2, but bias is 0 for now)
    for (uint32_t e = 0; e < embedding_dim; e++) {
        double sum = 0.0;
        for (uint32_t h = 0; h < hidden_dim; h++) {
            sum += w2[e * hidden_dim + h] * hidden[h];
        }
        output[e] = sum;
    }
    
    // Cleanup
    free(w1);
    free(w2);
    free(hidden);
    
    return 0;
}

// ============================================================================
// BATCH PROCESSING
// ============================================================================

/**
 * Process multiple tokens through FFN in parallel
 * 
 * @param model Model containing thread pool
 * @param layer_id Layer ID
 * @param input Input matrix [num_tokens × embedding_dim]
 * @param output Output matrix [num_tokens × embedding_dim]
 * @param num_tokens Number of tokens
 * @return 0 on success, -1 on error
 */
int cllm_ffn_forward_batch(
    CLLMModel* model,
    uint8_t layer_id,
    const double* input,
    double* output,
    uint32_t num_tokens
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
        
        int result = cllm_ffn_forward_thread(
            thread,
            token_input,
            token_output,
            model->embedding_dim,
            model->hidden_dim
        );
        
        if (result != 0) {
            return -1;
        }
    }
    
    return 0;
}