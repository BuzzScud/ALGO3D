/**
 * cllm_lattice_lookup.h - Direct Lookup Table for Lattice Embeddings
 * 
 * Uses deterministic geometric patterns from Babylonian clock structure
 * instead of computing L_lattice() for every token.
 */

#ifndef CLLM_LATTICE_LOOKUP_H
#define CLLM_LATTICE_LOOKUP_H

#include "cllm_inference.h"
#include <stdint.h>

/**
 * Initialize the ring pattern lookup table
 * This computes the deterministic patterns once (instant)
 */
void cllm_init_ring_patterns(uint32_t max_dim);

/**
 * Get embedding value using direct lookup (O(1))
 */
float cllm_get_embedding_from_pattern(uint32_t token_id, uint32_t dim, 
                                      uint32_t symmetry_group, uint32_t vocab_size);

/**
 * Initialize embeddings using direct pattern lookup (INSTANT)
 */
void cllm_embeddings_init_from_patterns(CLLMModel* model);

/**
 * Cleanup patterns
 */
void cllm_cleanup_ring_patterns(void);

#endif // CLLM_LATTICE_LOOKUP_H