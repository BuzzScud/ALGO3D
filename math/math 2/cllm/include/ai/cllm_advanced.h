#ifndef CLLM_CRYSTALLINE_ADVANCED_H
#define CLLM_CRYSTALLINE_ADVANCED_H

#include "cllm.h"
#include <stdint.h>

/**
 * Advanced Crystalline State
 * Opaque structure for advanced crystalline features
 */
typedef struct CrystallineAdvancedState CrystallineAdvancedState;

/**
 * Create advanced crystalline state
 * 
 * Initializes:
 * - Prime factorization cache (10000 entries)
 * - Ulam spiral spatial index
 * - CVP/SVP algorithms
 * 
 * @param model Model to optimize
 * @return Advanced state or NULL on failure
 */
CrystallineAdvancedState* crystalline_advanced_create(CLLMModel* model);

/**
 * Free advanced crystalline state
 */
void crystalline_advanced_free(CrystallineAdvancedState* state);

/**
 * CVP (Closest Vector Problem) - Find closest token
 * 
 * Given an embedding vector, find the closest token in the lattice.
 * More accurate than dot product similarity.
 * 
 * @param model Model with embeddings
 * @param query_embedding Query vector (embedding_dim)
 * @return Closest token ID
 */
uint32_t cvp_find_closest_token(CLLMModel* model, const float* query_embedding);

/**
 * SVP (Shortest Vector Problem) - Find shortest vector
 * 
 * Find the shortest non-zero vector in the embedding lattice.
 * Useful for embedding initialization.
 * 
 * @param model Model with embeddings
 * @return Shortest vector (embedding_dim) - caller must free
 */
float* svp_find_shortest_vector(CLLMModel* model);

/**
 * Compute similarity using advanced features
 * 
 * Uses cached prime factorizations for faster GCD computation.
 * 
 * @param state Advanced state
 * @param token1 First token ID
 * @param token2 Second token ID
 * @return Similarity score [0, 1]
 */
float crystalline_advanced_similarity(CrystallineAdvancedState* state, 
                                     uint32_t token1, uint32_t token2);

/**
 * Prefetch spatially nearby tokens
 * 
 * Uses Ulam spiral spatial index to prefetch nearby token embeddings
 * into CPU cache, improving cache hit rate.
 * 
 * @param state Advanced state
 * @param model Model with embeddings
 * @param token_id Token to prefetch neighbors for
 * @param k Number of neighbors to prefetch
 */
void crystalline_prefetch_nearby(CrystallineAdvancedState* state, 
                                CLLMModel* model, uint32_t token_id, int k);

#endif // CLLM_CRYSTALLINE_ADVANCED_H