/**
 * @file cllm_neighbor_ops.h
 * @brief Internal Neighbor Operations (12 Kissing Spheres)
 * 
 * INTERNAL USE ONLY - Not part of public API
 * 
 * This file contains internal helper functions for neighbor-based operations.
 * Since kissing spheres is THE architecture (not an option), these operations
 * are integrated into the core attention and embedding functions.
 * 
 * DO NOT use these functions directly - use the public APIs in:
 * - cllm_inference.h (for attention)
 * - cllm.h (for embeddings)
 * - cllm_batch.h (for batch operations)
 */

#ifndef CLLM_NEIGHBOR_OPS_H
#define CLLM_NEIGHBOR_OPS_H

#include "cllm.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute attention weights for 12 neighbors using L(n,d,k,λ)
 * 
 * INTERNAL FUNCTION - Use cllm_attention_forward() instead
 * 
 * @param model CLLM model
 * @param token_id Token to compute weights for
 * @param weights Output weights [12]
 * @return 0 on success, -1 on error
 */
int cllm_internal_compute_neighbor_weights(
    CLLMModel* model,
    uint32_t token_id,
    double* weights
);

/**
 * Add neighbor attention contributions to output
 * 
 * INTERNAL FUNCTION - Use cllm_attention_forward() instead
 * 
 * @param model CLLM model
 * @param token_id Token to process
 * @param output Output vector to add contributions to [embed_dim]
 * @param embed_dim Embedding dimension
 * @param strength Contribution strength (0.0-1.0)
 * @return 0 on success, -1 on error
 */
int cllm_internal_add_neighbor_attention(
    CLLMModel* model,
    uint32_t token_id,
    double* output,
    uint32_t embed_dim,
    double strength
);

/**
 * Apply neighbor influence to embedding
 * 
 * INTERNAL FUNCTION - Use cllm_embeddings_init() or cllm_embedding_refine() instead
 * 
 * @param model CLLM model
 * @param token_id Token to refine
 * @param influence_strength Influence strength (0.0-1.0)
 * @return 0 on success, -1 on error
 */
int cllm_internal_apply_neighbor_influence(
    CLLMModel* model,
    uint32_t token_id,
    double influence_strength
);

/**
 * Apply neighbor influence to all embeddings
 * 
 * INTERNAL FUNCTION - Use cllm_embeddings_init() instead
 * 
 * @param model CLLM model
 * @param influence_strength Influence strength (0.0-1.0)
 * @return 0 on success, -1 on error
 */
int cllm_internal_apply_neighbor_influence_all(
    CLLMModel* model,
    double influence_strength
);

/**
 * Compute neighbor influence vector (for analysis)
 * 
 * INTERNAL FUNCTION
 * 
 * @param model CLLM model
 * @param token_id Token to analyze
 * @param influence_vector Output influence vector [embed_dim]
 * @param weights Optional output weights [12]
 * @return 0 on success, -1 on error
 */
int cllm_internal_compute_neighbor_influence(
    CLLMModel* model,
    uint32_t token_id,
    double* influence_vector,
    double* weights
);

#ifdef __cplusplus
}
#endif

#endif // CLLM_NEIGHBOR_OPS_H