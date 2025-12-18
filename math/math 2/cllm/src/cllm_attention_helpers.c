/**
 * @file cllm_attention_helpers.c
 * @brief Helper functions for thread-centric attention mechanism
 * 
 * This file provides helper functions to work with attention parameters
 * stored in thread-local CrystallineAbacus storage.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ai/cllm.h"
#include "ai/cllm_attention.h"
#include "hierarchical_threading.h"
#include "math/abacus.h"
#include "math/abacus_fractional.h"

// ============================================================================
// PURE CRYSTALLINE MATH FUNCTIONS (NO math.h!)
// ============================================================================

/**
 * Pure crystalline sqrt using Newton's method (NO math.h)
 * This is a temporary implementation until we integrate CrystallineAbacus fully
 */
static double crystalline_sqrt(double x) {
    if (x <= 0.0) return 0.0;
    
    // Newton's method: x_{n+1} = (x_n + a/x_n) / 2
    double guess = x;
    for (int i = 0; i < 10; i++) {  // 10 iterations for convergence
        guess = (guess + x / guess) * 0.5;
    }
    return guess;
}

/**
 * Pure crystalline exp using Taylor series (NO math.h)
 * exp(x) = 1 + x + x^2/2! + x^3/3! + ...
 */
static double crystalline_exp(double x) {
    double result = 1.0;
    double term = 1.0;
    
    for (int n = 1; n < 20; n++) {  // 20 terms for convergence
        term *= x / n;
        result += term;
        // Convergence check
        if (term < 1e-10 && term > -1e-10) break;
    }
    return result;
}

// ============================================================================
// Q/K/V WEIGHT ACCESS FUNCTIONS
// ============================================================================

/**
 * Get Q/K/V projection weights from a thread's CrystallineAbacus
 * 
 * Each worker thread owns a portion of the projection matrices.
 * This function extracts the weights from the thread's storage.
 * 
 * @param thread Worker thread
 * @param query_weights Output buffer for query weights
 * @param key_weights Output buffer for key weights
 * @param value_weights Output buffer for value weights
 * @param embedding_dim Embedding dimension
 * @param head_dim Head dimension
 */
void cllm_get_qkv_weights_from_thread(
    HierarchicalThread* thread,
    double* query_weights,
    double* key_weights,
    double* value_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
) {
    if (!thread || !query_weights || !key_weights || !value_weights) {
        return;
    }
    
    // TODO: Extract weights from thread->value (CrystallineAbacus)
    // For now, initialize to small random values
    for (uint32_t i = 0; i < embedding_dim * head_dim; i++) {
        query_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.02;
        key_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.02;
        value_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.02;
    }
}

/**
 * Set Q/K/V projection weights to a thread's CrystallineAbacus
 * 
 * @param thread Worker thread
 * @param query_weights Query weights to store
 * @param key_weights Key weights to store
 * @param value_weights Value weights to store
 * @param embedding_dim Embedding dimension
 * @param head_dim Head dimension
 */
void cllm_set_qkv_weights_to_thread(
    HierarchicalThread* thread,
    const double* query_weights,
    const double* key_weights,
    const double* value_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
) {
    if (!thread || !query_weights || !key_weights || !value_weights) {
        return;
    }
    
    // TODO: Store weights in thread->value (CrystallineAbacus)
    // For now, this is a no-op
}

/**
 * Get output projection weights from a thread's CrystallineAbacus
 * 
 * @param thread Worker thread
 * @param output_weights Output buffer for output weights
 * @param embedding_dim Embedding dimension
 * @param head_dim Head dimension
 */
void cllm_get_output_weights_from_thread(
    HierarchicalThread* thread,
    double* output_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
) {
    if (!thread || !output_weights) {
        return;
    }
    
    // TODO: Extract weights from thread->value (CrystallineAbacus)
    // For now, initialize to small random values
    for (uint32_t i = 0; i < embedding_dim * head_dim; i++) {
        output_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.02;
    }
}

/**
 * Set output projection weights to a thread's CrystallineAbacus
 * 
 * @param thread Worker thread
 * @param output_weights Output weights to store
 * @param embedding_dim Embedding dimension
 * @param head_dim Head dimension
 */
void cllm_set_output_weights_to_thread(
    HierarchicalThread* thread,
    const double* output_weights,
    uint32_t embedding_dim,
    uint32_t head_dim
) {
    if (!thread || !output_weights) {
        return;
    }
    
    // TODO: Store weights in thread->value (CrystallineAbacus)
    // For now, this is a no-op
}

// ============================================================================
// Q/K/V COMPUTATION FUNCTIONS
// ============================================================================

/**
 * Compute Q/K/V for a single token using thread-local weights
 * 
 * This function computes the query, key, and value projections for a token
 * using the projection weights stored in the thread's CrystallineAbacus.
 * 
 * @param thread Worker thread with projection weights
 * @param embedding Input embedding vector
 * @param Q Output query vector
 * @param K Output key vector
 * @param V Output value vector
 * @param embedding_dim Embedding dimension
 * @param num_heads Number of attention heads
 * @param head_dim Head dimension
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
) {
    if (!thread || !embedding || !Q || !K || !V) {
        return;
    }
    
    // Get projection weights from thread
    double* query_weights = (double*)malloc(embedding_dim * head_dim * sizeof(double));
    double* key_weights = (double*)malloc(embedding_dim * head_dim * sizeof(double));
    double* value_weights = (double*)malloc(embedding_dim * head_dim * sizeof(double));
    
    cllm_get_qkv_weights_from_thread(thread, query_weights, key_weights, value_weights,
                                      embedding_dim, head_dim);
    
    // Compute Q = embedding · W_Q
    for (uint32_t h = 0; h < num_heads; h++) {
        for (uint32_t d = 0; d < head_dim; d++) {
            double sum = 0.0;
            for (uint32_t i = 0; i < embedding_dim; i++) {
                sum += embedding[i] * query_weights[i * head_dim + d];
            }
            Q[h * head_dim + d] = sum;
        }
    }
    
    // Compute K = embedding · W_K
    for (uint32_t h = 0; h < num_heads; h++) {
        for (uint32_t d = 0; d < head_dim; d++) {
            double sum = 0.0;
            for (uint32_t i = 0; i < embedding_dim; i++) {
                sum += embedding[i] * key_weights[i * head_dim + d];
            }
            K[h * head_dim + d] = sum;
        }
    }
    
    // Compute V = embedding · W_V
    for (uint32_t h = 0; h < num_heads; h++) {
        for (uint32_t d = 0; d < head_dim; d++) {
            double sum = 0.0;
            for (uint32_t i = 0; i < embedding_dim; i++) {
                sum += embedding[i] * value_weights[i * head_dim + d];
            }
            V[h * head_dim + d] = sum;
        }
    }
    
    free(query_weights);
    free(key_weights);
    free(value_weights);
}

/**
 * Apply output projection using thread-local weights
 * 
 * @param thread Worker thread with output weights
 * @param attention_output Input attention output
 * @param output Output vector after projection
 * @param embedding_dim Embedding dimension
 * @param num_heads Number of attention heads
 * @param head_dim Head dimension
 */
void cllm_apply_output_projection(
    HierarchicalThread* thread,
    const double* attention_output,
    double* output,
    uint32_t embedding_dim,
    uint32_t num_heads,
    uint32_t head_dim
) {
    if (!thread || !attention_output || !output) {
        return;
    }
    
    // Get output weights from thread
    double* output_weights = (double*)malloc(embedding_dim * head_dim * sizeof(double));
    cllm_get_output_weights_from_thread(thread, output_weights, embedding_dim, head_dim);
    
    // Compute output = attention_output · W_O
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double sum = 0.0;
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                sum += attention_output[h * head_dim + d] * output_weights[i * head_dim + d];
            }
        }
        output[i] = sum;
    }
    
    free(output_weights);
}

// ============================================================================
// ATTENTION COMPUTATION HELPERS
// ============================================================================

/**
 * Compute attention scores: scores = Q · K^T / sqrt(head_dim)
 * 
 * @param Q Query matrix [num_heads × seq_len × head_dim]
 * @param K Key matrix [num_heads × seq_len × head_dim]
 * @param scores Output scores [num_heads × seq_len × seq_len]
 * @param num_heads Number of attention heads
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 */
void cllm_compute_attention_scores(
    const double* Q,
    const double* K,
    double* scores,
    uint32_t num_heads,
    uint32_t seq_len,
    uint32_t head_dim
) {
    if (!Q || !K || !scores) {
        return;
    }
    
    double scale = 1.0 / crystalline_sqrt((double)head_dim);
    
    for (uint32_t h = 0; h < num_heads; h++) {
        for (uint32_t i = 0; i < seq_len; i++) {
            for (uint32_t j = 0; j < seq_len; j++) {
                double sum = 0.0;
                for (uint32_t d = 0; d < head_dim; d++) {
                    sum += Q[h * seq_len * head_dim + i * head_dim + d] *
                           K[h * seq_len * head_dim + j * head_dim + d];
                }
                scores[h * seq_len * seq_len + i * seq_len + j] = sum * scale;
            }
        }
    }
}

/**
 * Apply softmax to attention scores
 * 
 * @param scores Attention scores [num_heads × seq_len × seq_len]
 * @param num_heads Number of attention heads
 * @param seq_len Sequence length
 */
void cllm_apply_softmax_to_scores(
    double* scores,
    uint32_t num_heads,
    uint32_t seq_len
) {
    if (!scores) {
        return;
    }
    
    for (uint32_t h = 0; h < num_heads; h++) {
        for (uint32_t i = 0; i < seq_len; i++) {
            double* row = &scores[h * seq_len * seq_len + i * seq_len];
            
            // Find max for numerical stability
            double max_val = row[0];
            for (uint32_t j = 1; j < seq_len; j++) {
                if (row[j] > max_val) {
                    max_val = row[j];
                }
            }
            
            // Compute exp and sum
            double sum = 0.0;
            for (uint32_t j = 0; j < seq_len; j++) {
                row[j] = crystalline_exp(row[j] - max_val);
                sum += row[j];
            }
            
            // Normalize
            for (uint32_t j = 0; j < seq_len; j++) {
                row[j] /= sum;
            }
        }
    }
}

/**
 * Compute attention output: output = softmax(scores) · V
 * 
 * @param scores Attention scores [num_heads × seq_len × seq_len]
 * @param V Value matrix [num_heads × seq_len × head_dim]
 * @param output Output matrix [num_heads × seq_len × head_dim]
 * @param num_heads Number of attention heads
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 */
void cllm_compute_attention_output(
    const double* scores,
    const double* V,
    double* output,
    uint32_t num_heads,
    uint32_t seq_len,
    uint32_t head_dim
) {
    if (!scores || !V || !output) {
        return;
    }
    
    for (uint32_t h = 0; h < num_heads; h++) {
        for (uint32_t i = 0; i < seq_len; i++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                double sum = 0.0;
                for (uint32_t j = 0; j < seq_len; j++) {
                    sum += scores[h * seq_len * seq_len + i * seq_len + j] *
                           V[h * seq_len * head_dim + j * head_dim + d];
                }
                output[h * seq_len * head_dim + i * head_dim + d] = sum;
            }
        }
    }
}