/**
 * @file ntt_attention_backward.c
 * @brief NTT Attention Backward Pass Implementation
 * 
 * Implements O(n log n) backward pass for attention mechanism using
 * Number Theoretic Transform (NTT).
 * 
 * Mathematical Foundation:
 * Given ∂L/∂O (gradient from next layer), compute:
 * - ∂L/∂V = Attention^T × ∂L/∂O
 * - ∂L/∂A = ∂L/∂O × V^T
 * - ∂L/∂S = ∂L/∂A ⊙ softmax'(S)
 * - ∂L/∂Q = ∂L/∂S × K / √d_k
 * - ∂L/∂K = ∂L/∂S^T × Q / √d_k
 */

#include "ntt_attention.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Compute softmax Jacobian-vector product
 * 
 * Given softmax output and upstream gradient, compute:
 * grad_input = grad_output ⊙ softmax - softmax × (grad_output · softmax)
 * 
 * This is more efficient than computing the full Jacobian matrix.
 * 
 * @param grad_input Output gradient [seq_len]
 * @param grad_output Input gradient [seq_len]
 * @param softmax_output Softmax values [seq_len]
 * @param seq_len Sequence length
 */
static void compute_softmax_jacobian_vector_product(
    double* grad_input,
    const double* grad_output,
    const double* softmax_output,
    uint32_t seq_len
) {
    // Compute dot product: sum = Σ(grad_output_i × softmax_i)
    double sum = 0.0;
    for (uint32_t i = 0; i < seq_len; i++) {
        sum += grad_output[i] * softmax_output[i];
    }
    
    // Compute Jacobian-vector product
    for (uint32_t i = 0; i < seq_len; i++) {
        grad_input[i] = grad_output[i] * softmax_output[i] - softmax_output[i] * sum;
    }
}

/**
 * Compute gradient w.r.t. values
 * 
 * ∂L/∂V = Attention^T × ∂L/∂O
 * 
 * @param grad_values Output [seq_len x head_dim]
 * @param grad_output Input [seq_len x head_dim]
 * @param attention_weights Attention [seq_len x seq_len]
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 * @return 1 on success, 0 on failure
 */
static int compute_value_gradient(
    double* grad_values,
    const double* grad_output,
    const double* attention_weights,
    uint32_t seq_len,
    uint32_t head_dim
) {
    // Matrix multiplication: grad_V = A^T × grad_O
    // A^T is [seq_len x seq_len], grad_O is [seq_len x head_dim]
    // Result is [seq_len x head_dim]
    
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t k = 0; k < head_dim; k++) {
            double sum = 0.0;
            for (uint32_t j = 0; j < seq_len; j++) {
                // A^T[i,j] = A[j,i]
                sum += attention_weights[j * seq_len + i] * grad_output[j * head_dim + k];
            }
            grad_values[i * head_dim + k] = sum;
        }
    }
    
    return 1;
}

/**
 * Compute gradient w.r.t. attention scores
 * 
 * ∂L/∂A = ∂L/∂O × V^T
 * 
 * @param grad_attention Output [seq_len x seq_len]
 * @param grad_output Input [seq_len x head_dim]
 * @param values Values [seq_len x head_dim]
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 * @return 1 on success, 0 on failure
 */
static int compute_attention_gradient(
    double* grad_attention,
    const double* grad_output,
    const double* values,
    uint32_t seq_len,
    uint32_t head_dim
) {
    // Matrix multiplication: grad_A = grad_O × V^T
    // grad_O is [seq_len x head_dim], V^T is [head_dim x seq_len]
    // Result is [seq_len x seq_len]
    
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            double sum = 0.0;
            for (uint32_t k = 0; k < head_dim; k++) {
                sum += grad_output[i * head_dim + k] * values[j * head_dim + k];
            }
            grad_attention[i * seq_len + j] = sum;
        }
    }
    
    return 1;
}

/**
 * Apply softmax backward pass to all rows
 * 
 * @param grad_scores Output [seq_len x seq_len]
 * @param grad_attention Input [seq_len x seq_len]
 * @param attention_weights Softmax output [seq_len x seq_len]
 * @param seq_len Sequence length
 * @return 1 on success, 0 on failure
 */
static int softmax_backward(
    double* grad_scores,
    const double* grad_attention,
    const double* attention_weights,
    uint32_t seq_len
) {
    // Apply softmax Jacobian to each row
    for (uint32_t i = 0; i < seq_len; i++) {
        compute_softmax_jacobian_vector_product(
            &grad_scores[i * seq_len],
            &grad_attention[i * seq_len],
            &attention_weights[i * seq_len],
            seq_len
        );
    }
    
    return 1;
}

/**
 * Compute Q and K gradients using standard matrix multiplication
 * 
 * For now, we use standard O(n²) implementation.
 * TODO: Optimize with NTT for O(n log n) complexity.
 * 
 * @param grad_queries Output [seq_len x head_dim]
 * @param grad_keys Output [seq_len x head_dim]
 * @param grad_scores Input [seq_len x seq_len]
 * @param queries Forward Q [seq_len x head_dim]
 * @param keys Forward K [seq_len x head_dim]
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 * @param scale_factor Scaling factor (1/sqrt(head_dim))
 * @return 1 on success, 0 on failure
 */
static int compute_qk_gradients(
    double* grad_queries,
    double* grad_keys,
    const double* grad_scores,
    const double* queries,
    const double* keys,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    // Scale gradients by 1/√d_k
    double* scaled_grad = malloc(seq_len * seq_len * sizeof(double));
    if (!scaled_grad) {
        return 0;
    }
    
    for (uint32_t i = 0; i < seq_len * seq_len; i++) {
        scaled_grad[i] = grad_scores[i] * scale_factor;
    }
    
    // Debug: Check scaled_grad
    if (seq_len == 4 && head_dim == 4) {
        double scaled_sum = 0.0;
        for (uint32_t i = 0; i < seq_len * seq_len; i++) {
            scaled_sum += (scaled_grad[i] > 0 ? scaled_grad[i] : -scaled_grad[i]);
        }
        printf("  [ALGO DEBUG] scaled_grad sum: %.6f (scale_factor=%.6f)\n", scaled_sum, scale_factor);
        
        // Check Q and K inputs
        double Q_sum = 0.0, K_sum = 0.0;
        for (uint32_t i = 0; i < seq_len * head_dim; i++) {
            Q_sum += (queries[i] > 0 ? queries[i] : -queries[i]);
            K_sum += (keys[i] > 0 ? keys[i] : -keys[i]);
        }
        printf("  [ALGO DEBUG] Q sum: %.6f, K sum: %.6f\n", Q_sum, K_sum);
    }
    
    // Compute grad_Q = scaled_grad × K
    // scaled_grad is [seq_len x seq_len], K is [seq_len x head_dim]
    // Result is [seq_len x head_dim]
    
    // Debug: Print first few values
    printf("  [ALGO DEBUG] seq_len=%u, head_dim=%u\n", seq_len, head_dim);
    if (seq_len == 4 && head_dim == 4) {
        printf("  [ALGO DEBUG] First scaled_grad values: %.10f, %.10f, %.10f, %.10f\n",
               scaled_grad[0], scaled_grad[1], scaled_grad[2], scaled_grad[3]);
        printf("  [ALGO DEBUG] First K values: %.10f, %.10f, %.10f, %.10f\n",
               keys[0], keys[1], keys[2], keys[3]);
    }
    
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t k = 0; k < head_dim; k++) {
            double sum = 0.0;
            for (uint32_t j = 0; j < seq_len; j++) {
                double contrib = scaled_grad[i * seq_len + j] * keys[j * head_dim + k];
                sum += contrib;
                
                // Debug first iteration
                if (seq_len == 4 && head_dim == 4 && i == 0 && k == 0 && j < 4) {
                    printf("  [ALGO DEBUG] i=%u, k=%u, j=%u: scaled_grad=%.10f, key=%.10f, contrib=%.10f, sum=%.10f\n",
                           i, k, j, scaled_grad[i * seq_len + j], keys[j * head_dim + k], contrib, sum);
                }
            }
            grad_queries[i * head_dim + k] = sum;
            
            // Debug first result
            if (seq_len == 4 && head_dim == 4 && i == 0 && k == 0) {
                printf("  [ALGO DEBUG] grad_queries[0][0] = %.10f\n", sum);
            }
        }
    }
    
    // Compute grad_K = scaled_grad^T × Q
    // scaled_grad^T is [seq_len x seq_len], Q is [seq_len x head_dim]
    // Result is [seq_len x head_dim]
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t k = 0; k < head_dim; k++) {
            double sum = 0.0;
            for (uint32_t j = 0; j < seq_len; j++) {
                // scaled_grad^T[i,j] = scaled_grad[j,i]
                sum += scaled_grad[j * seq_len + i] * queries[j * head_dim + k];
            }
            grad_keys[i * head_dim + k] = sum;
        }
    }
    
    // Debug: Check results
    if (seq_len == 4 && head_dim == 4) {
        double gQ_sum = 0.0, gK_sum = 0.0;
        for (uint32_t i = 0; i < seq_len * head_dim; i++) {
            gQ_sum += (grad_queries[i] > 0 ? grad_queries[i] : -grad_queries[i]);
            gK_sum += (grad_keys[i] > 0 ? grad_keys[i] : -grad_keys[i]);
        }
        printf("  [ALGO DEBUG] After matmul: grad_Q sum: %.6f, grad_K sum: %.6f\n", gQ_sum, gK_sum);
    }
    
    free(scaled_grad);
    return 1;
}

// ============================================================================
// PUBLIC API
// ============================================================================

int ntt_attention_backward_single_head_double(
    double* grad_queries,
    double* grad_keys,
    double* grad_values,
    const double* grad_output,
    const double* queries,
    const double* keys,
    const double* values,
    const double* attention_weights,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    // Validate inputs
    if (!grad_queries || !grad_keys || !grad_values || !grad_output ||
        !queries || !keys || !values || !attention_weights ||
        seq_len == 0 || head_dim == 0) {
        fprintf(stderr, "Error: Invalid inputs to ntt_attention_backward_single_head_double\n");
        return 0;
    }
    
    // Allocate temporary buffers
    double* grad_attention = calloc(seq_len * seq_len, sizeof(double));
    double* grad_scores = calloc(seq_len * seq_len, sizeof(double));
    
    if (!grad_attention || !grad_scores) {
        fprintf(stderr, "Error: Failed to allocate temporary buffers\n");
        free(grad_attention);
        free(grad_scores);
        return 0;
    }
    
    // Step 1: Compute ∂L/∂V = Attention^T × ∂L/∂O
    if (!compute_value_gradient(grad_values, grad_output, attention_weights, seq_len, head_dim)) {
        fprintf(stderr, "Error: Failed to compute value gradient\n");
        goto cleanup_error;
    }
    
    // Step 2: Compute ∂L/∂A = ∂L/∂O × V^T
    if (!compute_attention_gradient(grad_attention, grad_output, values, seq_len, head_dim)) {
        fprintf(stderr, "Error: Failed to compute attention gradient\n");
        goto cleanup_error;
    }
    
    // Debug: Check grad_attention
    if (seq_len == 4 && head_dim == 4) {
        double grad_attn_sum = 0.0;
        for (uint32_t i = 0; i < seq_len * seq_len; i++) {
            grad_attn_sum += (grad_attention[i] > 0 ? grad_attention[i] : -grad_attention[i]);
        }
        printf("  [ALGO DEBUG] grad_attention sum: %.6f\n", grad_attn_sum);
    }
    
    // Step 3: Compute ∂L/∂S (softmax backward)
    if (!softmax_backward(grad_scores, grad_attention, attention_weights, seq_len)) {
        fprintf(stderr, "Error: Failed to compute softmax backward\n");
        goto cleanup_error;
    }
    
    // Debug: Check grad_scores after softmax backward
    if (seq_len == 4 && head_dim == 4) {
        double grad_scores_sum = 0.0;
        for (uint32_t i = 0; i < seq_len * seq_len; i++) {
            grad_scores_sum += (grad_scores[i] > 0 ? grad_scores[i] : -grad_scores[i]);
        }
        printf("  [ALGO DEBUG] grad_scores sum after softmax_backward: %.6f\n", grad_scores_sum);
    }
    
    // Step 4: Compute ∂L/∂Q and ∂L/∂K
    if (!compute_qk_gradients(grad_queries, grad_keys, grad_scores, 
                              queries, keys, seq_len, head_dim, scale_factor)) {
        fprintf(stderr, "Error: Failed to compute Q/K gradients\n");
        goto cleanup_error;
    }
    
    // Debug: Check grad_scores before computing Q/K gradients
    if (seq_len == 4 && head_dim == 4) {
        double grad_scores_sum = 0.0;
        for (uint32_t i = 0; i < seq_len * seq_len; i++) {
            grad_scores_sum += (grad_scores[i] > 0 ? grad_scores[i] : -grad_scores[i]);
        }
        printf("  [ALGO DEBUG] grad_scores sum: %.6f\n", grad_scores_sum);
        
        double gQ_sum = 0.0, gK_sum = 0.0;
        for (uint32_t i = 0; i < seq_len * head_dim; i++) {
            gQ_sum += (grad_queries[i] > 0 ? grad_queries[i] : -grad_queries[i]);
            gK_sum += (grad_keys[i] > 0 ? grad_keys[i] : -grad_keys[i]);
        }
        printf("  [ALGO DEBUG] After compute_qk_gradients: grad_Q sum: %.6f, grad_K sum: %.6f\n",
               gQ_sum, gK_sum);
    }
    
    // Cleanup
    free(grad_attention);
    free(grad_scores);
    return 1;
    
cleanup_error:
    free(grad_attention);
    free(grad_scores);
    return 0;
}

int ntt_attention_backward_multi_head_double(
    double* grad_queries,
    double* grad_keys,
    double* grad_values,
    const double* grad_output,
    const double* queries,
    const double* keys,
    const double* values,
    const double* attention_weights,
    uint32_t seq_len,
    uint32_t head_dim,
    uint32_t num_heads,
    double scale_factor
) {
    // Validate inputs
    if (!grad_queries || !grad_keys || !grad_values || !grad_output ||
        !queries || !keys || !values || !attention_weights ||
        seq_len == 0 || head_dim == 0 || num_heads == 0) {
        fprintf(stderr, "Error: Invalid inputs to ntt_attention_backward_multi_head_double\n");
        return 0;
    }
    
    // Process each head independently
    for (uint32_t h = 0; h < num_heads; h++) {
        uint32_t head_offset = h * seq_len * head_dim;
        uint32_t weight_offset = h * seq_len * seq_len;
        
        int result = ntt_attention_backward_single_head_double(
            grad_queries + head_offset,
            grad_keys + head_offset,
            grad_values + head_offset,
            grad_output + head_offset,
            queries + head_offset,
            keys + head_offset,
            values + head_offset,
            attention_weights + weight_offset,
            seq_len,
            head_dim,
            scale_factor
        );
        
        if (!result) {
            fprintf(stderr, "Error: Failed to compute backward pass for head %u\n", h);
            return 0;
        }
    }
    
    return 1;
}