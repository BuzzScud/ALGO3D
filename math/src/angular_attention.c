/**
 * Angular Attention Algorithm (General Purpose)
 * 
 * This is a GENERAL algorithm that computes attention using angular positions.
 * It can be used by ANY model, not just CLLM.
 * 
 * Formula: attention(Q,K) = cos((θ_Q - θ_K) · φᵢ) · cymatic_resonance(θ_diff)
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_cos with math_cos (2 calls)
 * - Replaced prime_expf with math_exp (1 call)
 * Total: 3 function calls migrated to NEW math library
 */

#include <stdlib.h>
#include <string.h>
#include "../include/angular_attention.h"
#include "math/transcendental.h"  // NEW math library
#include "math/arithmetic.h"      // NEW math library
#include "math/types.h"           // NEW math library (for MATH_PI)

/**
 * Compute angular attention score between two positions
 */
float angular_attention_score(
    const AngularPosition* q_pos,
    const AngularPosition* k_pos,
    uint64_t phi_i,
    double cymatic_freq
) {
    if (!q_pos || !k_pos) return 0.0f;
    
    // Compute attention score: cos((θ_q - θ_k) · φᵢ)
    double theta_diff = q_pos->theta - k_pos->theta;
    double score = math_cos(theta_diff * (double)phi_i);
    
    // Apply cymatic resonance
    // This adds a frequency-domain modulation to the attention
    double cymatic_phase = 2.0 * MATH_PI * cymatic_freq * theta_diff / 1000.0;
    double resonance = math_cos(cymatic_phase);
    
    // Combine: attention × (0.8 + 0.2 × resonance)
    // This gives 20% modulation from cymatic resonance
    score *= (0.8 + 0.2 * resonance);
    
    return (float)score;
}

/**
 * Compute angular attention scores for a batch
 */
void angular_attention_scores_batch(
    const AngularPosition* q_positions,
    const AngularPosition* k_positions,
    uint32_t num_queries,
    uint32_t num_keys,
    uint64_t phi_i,
    double cymatic_freq,
    float* scores
) {
    if (!q_positions || !k_positions || !scores) return;
    
    // Compute all query-key attention scores
    for (uint32_t i = 0; i < num_queries; i++) {
        for (uint32_t j = 0; j < num_keys; j++) {
            scores[i * num_keys + j] = angular_attention_score(
                &q_positions[i],
                &k_positions[j],
                phi_i,
                cymatic_freq
            );
        }
    }
}

/**
 * Apply softmax to attention scores (in-place)
 */
static void softmax_inplace(float* scores, uint32_t len) {
    if (!scores || len == 0) return;
    
    // Find max for numerical stability
    float max_score = scores[0];
    for (uint32_t i = 1; i < len; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
        }
    }
    
    // Compute exp and sum
    double sum = 0.0;
    for (uint32_t i = 0; i < len; i++) {
        scores[i] = math_exp(scores[i] - max_score);
        sum += scores[i];
    }
    
    // Normalize
    if (sum > 1e-10) {
        float inv_sum = 1.0f / (float)sum;
        for (uint32_t i = 0; i < len; i++) {
            scores[i] *= inv_sum;
        }
    }
}

/**
 * Apply angular attention to compute output
 */
void angular_attention_forward(
    const AngularPosition* q_positions,
    const AngularPosition* k_positions,
    const float* values,
    float* output,
    uint32_t seq_len,
    uint32_t head_dim,
    uint64_t phi_i,
    double cymatic_freq
) {
    if (!q_positions || !k_positions || !values || !output) return;
    
    // Allocate temporary buffer for attention scores
    float* scores = (float*)malloc(seq_len * sizeof(float));
    if (!scores) return;
    
    // For each query position
    for (uint32_t i = 0; i < seq_len; i++) {
        // Compute attention scores with all keys
        for (uint32_t j = 0; j < seq_len; j++) {
            scores[j] = angular_attention_score(
                &q_positions[i],
                &k_positions[j],
                phi_i,
                cymatic_freq
            );
        }
        
        // Apply softmax to get attention weights
        softmax_inplace(scores, seq_len);
        
        // Compute weighted sum of values
        float* out_vec = &output[i * head_dim];
        memset(out_vec, 0, head_dim * sizeof(float));
        
        for (uint32_t j = 0; j < seq_len; j++) {
            const float* val_vec = &values[j * head_dim];
            float weight = scores[j];
            
            for (uint32_t d = 0; d < head_dim; d++) {
                out_vec[d] += weight * val_vec[d];
            }
        }
    }
    
    free(scores);
}
