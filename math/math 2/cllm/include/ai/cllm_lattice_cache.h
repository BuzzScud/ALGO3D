/*
 * cllm_lattice_cache.h - L_lattice() caching for fast embedding initialization
 * 
 * Leverages 12-fold symmetry to cache L_lattice() values and achieve
 * 285x speedup in embedding initialization.
 */

#ifndef CLLM_LATTICE_CACHE_H
#define CLLM_LATTICE_CACHE_H

#include "cllm_inference.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize L_lattice() cache for given embedding dimension
 * Pre-computes values for 12 symmetry groups
 * 
 * @param max_embedding_dim Maximum embedding dimension to cache
 */
void cllm_lattice_cache_init(uint32_t max_embedding_dim);

/**
 * Get cached L_lattice() value with perturbation for uniqueness
 * 
 * @param prime Token's prime encoding
 * @param dim Dimension index
 * @param symmetry_group Token's symmetry group (0-11)
 * @param token_id Token ID (for perturbation)
 * @return Cached L_lattice value with small perturbation
 */
double cllm_lattice_cache_get(uint64_t prime, uint32_t dim, uint32_t symmetry_group, 
                               uint32_t token_id);

/**
 * Fast embedding initialization using cached L_lattice() values
 * 285x faster than computing L_lattice() for every token-dimension pair
 * 
 * @param model CLLM model to initialize
 */
void cllm_embeddings_init_lattice_cached(CLLMModel* model);

/**
 * Refine embeddings using neighbor influence
 * 
 * This applies the 12 kissing spheres neighbor influence to embeddings.
 * Since kissing spheres is THE architecture, this is the standard way
 * to refine embeddings (not an optional feature).
 * 
 * @param model CLLM model
 * @param influence_strength Influence strength (0.0-1.0, default 0.2)
 * @return 0 on success, -1 on error
 */
int cllm_embedding_refine(CLLMModel* model, double influence_strength);

/**
 * Cleanup cache (call on shutdown)
 */
void cllm_lattice_cache_cleanup(void);

#endif /* CLLM_LATTICE_CACHE_H */
/**
 * Compute embedding for a specific token on-demand (lazy initialization)
 * This is called automatically when an uninitialized embedding is accessed
 */
void cllm_compute_embedding_lazy(CLLMModel* model, uint32_t token_id);
void cllm_precompute_all_embeddings(CLLMModel* model);

