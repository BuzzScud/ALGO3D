#ifndef ANGULAR_ATTENTION_H
#define ANGULAR_ATTENTION_H

#include <stdint.h>
#include "math/angular_position.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * General Angular Attention Algorithm
 * 
 * This is a GENERAL algorithm that can be used by any model.
 * It computes attention scores using angular positions and dimensional frequencies.
 * 
 * Formula: attention(Q,K) = cos((θ_Q - θ_K) · φᵢ) · cymatic_resonance(θ_diff)
 */

/**
 * Compute angular attention score between two positions
 * 
 * @param q_pos Query angular position
 * @param k_pos Key angular position
 * @param phi_i Dimensional frequency φᵢ
 * @param cymatic_freq Cymatic base frequency (e.g., 432 Hz)
 * @return Attention score in [-1, 1]
 */
float angular_attention_score(
    const AngularPosition* q_pos,
    const AngularPosition* k_pos,
    uint64_t phi_i,
    double cymatic_freq
);

/**
 * Compute angular attention scores for a batch
 * 
 * @param q_positions Query angular positions [num_queries]
 * @param k_positions Key angular positions [num_keys]
 * @param num_queries Number of queries
 * @param num_keys Number of keys
 * @param phi_i Dimensional frequency φᵢ
 * @param cymatic_freq Cymatic base frequency
 * @param scores Output attention scores [num_queries x num_keys]
 */
void angular_attention_scores_batch(
    const AngularPosition* q_positions,
    const AngularPosition* k_positions,
    uint32_t num_queries,
    uint32_t num_keys,
    uint64_t phi_i,
    double cymatic_freq,
    float* scores
);

/**
 * Apply angular attention to compute output
 * 
 * @param q_positions Query angular positions [seq_len]
 * @param k_positions Key angular positions [seq_len]
 * @param values Value vectors [seq_len x head_dim]
 * @param output Output vectors [seq_len x head_dim]
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 * @param phi_i Dimensional frequency φᵢ
 * @param cymatic_freq Cymatic base frequency
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
);

#ifdef __cplusplus
}
#endif

#endif /* ANGULAR_ATTENTION_H */
