/**
 * lattice_embeddings.h - Geometric Pattern-Based Embeddings
 * 
 * This header defines the interface for embedding initialization using
 * the geometric pattern directly, without any caching or pre-computation.
 * 
 * This is a fundamental algorithm in the algorithms layer, usable by ANY system.
 */

#ifndef LATTICE_EMBEDDINGS_H
#define LATTICE_EMBEDDINGS_H

#include <stdint.h>
#include <stdbool.h>
#include "clock_lattice.h"

/**
 * Initialize embeddings using geometric pattern
 * 
 * This is INSTANT - no pre-computation, no caching, just pure geometry.
 * Works for ANY vocabulary size with O(vocab_size * embedding_dim) complexity.
 * 
 * The pattern is:
 * 1. Token ID → Clock position (O(1))
 * 2. Clock position → L(n,d,k,λ) (O(1) per dimension)
 * 3. L value → Embedding value via tanh normalization
 * 
 * @param embeddings Output embedding matrix [vocab_size × embedding_dim]
 * @param vocab_size Number of tokens (can be ANY size)
 * @param embedding_dim Embedding dimension
 */
void lattice_embeddings_init_geometric(
    float* embeddings,
    uint32_t vocab_size,
    uint32_t embedding_dim
);

/**
 * Get embedding for a single token (on-demand)
 * 
 * This demonstrates that we can compute embeddings on-demand
 * without any pre-computation. Useful for dynamic vocabularies.
 * 
 * @param token_id Token ID
 * @param embedding_dim Embedding dimension
 * @param output Output embedding vector [embedding_dim]
 */
void lattice_get_token_embedding_geometric(
    uint32_t token_id,
    uint32_t embedding_dim,
    float* output
);

/**
 * Verify geometric pattern properties
 * 
 * This checks that the geometric pattern maintains key properties:
 * - Same symmetry group → similar embeddings
 * - Same ring → similar magnitude
 * - 12-fold symmetry preserved
 * 
 * @param embeddings Embedding matrix
 * @param vocab_size Vocabulary size
 * @param embedding_dim Embedding dimension
 * @return true if properties are satisfied
 */
bool lattice_verify_geometric_embeddings(
    const float* embeddings,
    uint32_t vocab_size,
    uint32_t embedding_dim
);

#endif // LATTICE_EMBEDDINGS_H

/**
 * Initialize embeddings using geometric pattern with Crystalline Abacus (arbitrary precision)
 * 
 * MIGRATED: Now uses NEW math library (Crystalline Abacus)
 * 
 * This is INSTANT with ARBITRARY PRECISION - no overflow, no precision loss.
 * Works for ANY vocabulary size with O(vocab_size * embedding_dim) complexity.
 * Supports ALL bases >= 2 (base-60 mathematics).
 * 
 * @param embeddings Output embedding matrix [vocab_size * embedding_dim] (float)
 * @param vocab_size Number of tokens (can be ANY size)
 * @param embedding_dim Embedding dimension
 */
void lattice_embeddings_init_geometric_abacus(
    float* embeddings,
    uint32_t vocab_size,
    uint32_t embedding_dim
);

/**
 * Get embedding for a single token (on-demand) with Crystalline Abacus
 * 
 * MIGRATED: Now uses NEW math library (Crystalline Abacus)
 * 
 * This demonstrates that we can compute embeddings on-demand
 * with arbitrary precision. Useful for dynamic vocabularies.
 * 
 * @param embedding Output embedding vector [embedding_dim] (float)
 * @param token_id Token ID
 * @param embedding_dim Embedding dimension
 * @param lattice Clock lattice for mapping (optional, can be NULL, void* to avoid type conflicts)
 */
void lattice_get_token_embedding_geometric_abacus(
    float* embedding,
    uint32_t token_id,
    uint32_t embedding_dim,
    void* lattice
);

