/*
 * Pure Crystalline CLLM - Embeddings Implementation
 * 
 * Implements CrystallineEmbeddings with LLL-reduced lattice basis,
 * exact BigFixed token positions, and morphology graph.
 * 
 * PURE IMPLEMENTATION: Uses ONLY arbitrary precision mathematics.
 * NO external math libraries (math.h, GMP, etc.)
 */

#include "cllm_pure_crystalline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Helper function to allocate BigFixed matrix
 */
static BigFixed** allocate_bigfixed_matrix(uint32_t rows, uint32_t cols, int precision) {
    BigFixed** matrix = (BigFixed**)malloc(rows * sizeof(BigFixed*));
    if (!matrix) return NULL;
    
    for (uint32_t i = 0; i < rows; i++) {
        matrix[i] = (BigFixed*)malloc(cols * sizeof(BigFixed));
        if (!matrix[i]) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
        
        // Initialize each BigFixed
        for (uint32_t j = 0; j < cols; j++) {
            // Allocate BigInt structures
            matrix[i][j].integer_part = (BigInt*)malloc(sizeof(BigInt));
            matrix[i][j].fractional_part = (BigInt*)malloc(sizeof(BigInt));
            if (!matrix[i][j].integer_part || !matrix[i][j].fractional_part) {
                // Cleanup on failure
                for (uint32_t k = 0; k < i; k++) {
                    for (uint32_t l = 0; l < cols; l++) {
                        big_free(matrix[k][l].integer_part);
                        free(matrix[k][l].integer_part);
                        big_free(matrix[k][l].fractional_part);
                        free(matrix[k][l].fractional_part);
                    }
                    free(matrix[k]);
                }
                if (j > 0) {
                    for (uint32_t l = 0; l < j; l++) {
                        big_free(matrix[i][l].integer_part);
                        free(matrix[i][l].integer_part);
                        big_free(matrix[i][l].fractional_part);
                        free(matrix[i][l].fractional_part);
                    }
                }
                free(matrix[i]);
                free(matrix);
                return NULL;
            }
            // Initialize BigInt structures
            big_init(matrix[i][j].integer_part);
            big_init(matrix[i][j].fractional_part);
            matrix[i][j].scale_bits = precision;
            matrix[i][j].negative = 0;
        }
    }
    
    return matrix;
}

/*
 * Helper function to free BigFixed matrix
 */
static void free_bigfixed_matrix(BigFixed** matrix, uint32_t rows, uint32_t cols) {
    if (!matrix) return;
    
    for (uint32_t i = 0; i < rows; i++) {
        if (matrix[i]) {
            for (uint32_t j = 0; j < cols; j++) {
                if (matrix[i][j].integer_part) {
                    big_free(matrix[i][j].integer_part);
                    free(matrix[i][j].integer_part);
                }
                if (matrix[i][j].fractional_part) {
                    big_free(matrix[i][j].fractional_part);
                    free(matrix[i][j].fractional_part);
                }
            }
            free(matrix[i]);
        }
    }
    free(matrix);
}

/*
 * Create new CrystallineEmbeddings structure
 */
CrystallineEmbeddings* crystalline_embeddings_create(uint32_t vocab_size, uint32_t lattice_dim) {
    if (vocab_size == 0 || lattice_dim == 0) {
        fprintf(stderr, "Error: vocab_size and lattice_dim must be > 0\n");
        return NULL;
    }
    
    CrystallineEmbeddings* embeddings = (CrystallineEmbeddings*)malloc(sizeof(CrystallineEmbeddings));
    if (!embeddings) {
        fprintf(stderr, "Error: Failed to allocate CrystallineEmbeddings\n");
        return NULL;
    }
    
    embeddings->vocab_size = vocab_size;
    embeddings->lattice_dim = lattice_dim;
    embeddings->basis_optimized = false;
    embeddings->optimization_epoch = 0;
    embeddings->total_lookups = 0;
    embeddings->cache_hits = 0;
    embeddings->avg_lookup_time = 0.0;
    
    // Allocate lattice basis (identity matrix initially)
    embeddings->lattice_basis = allocate_bigfixed_matrix(lattice_dim, lattice_dim, 256);
    if (!embeddings->lattice_basis) {
        free(embeddings);
        return NULL;
    }
    
    // Allocate inverse basis
    embeddings->inverse_basis = allocate_bigfixed_matrix(lattice_dim, lattice_dim, 256);
    if (!embeddings->inverse_basis) {
        free_bigfixed_matrix(embeddings->lattice_basis, lattice_dim, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    // Allocate token array
    embeddings->tokens = (CrystallineToken**)calloc(vocab_size, sizeof(CrystallineToken*));
    if (!embeddings->tokens) {
        free_bigfixed_matrix(embeddings->inverse_basis, lattice_dim, lattice_dim);
        free_bigfixed_matrix(embeddings->lattice_basis, lattice_dim, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    // Allocate token positions
    embeddings->token_positions = allocate_bigfixed_matrix(vocab_size, lattice_dim, 256);
    if (!embeddings->token_positions) {
        free(embeddings->tokens);
        free_bigfixed_matrix(embeddings->inverse_basis, lattice_dim, lattice_dim);
        free_bigfixed_matrix(embeddings->lattice_basis, lattice_dim, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    // Allocate token primes
    embeddings->token_primes = (uint64_t*)calloc(vocab_size, sizeof(uint64_t));
    if (!embeddings->token_primes) {
        free_bigfixed_matrix(embeddings->token_positions, vocab_size, lattice_dim);
        free(embeddings->tokens);
        free_bigfixed_matrix(embeddings->inverse_basis, lattice_dim, lattice_dim);
        free_bigfixed_matrix(embeddings->lattice_basis, lattice_dim, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    // Allocate morphology graph
    embeddings->morphology_graph = (uint32_t**)malloc(vocab_size * sizeof(uint32_t*));
    if (!embeddings->morphology_graph) {
        free(embeddings->token_primes);
        free_bigfixed_matrix(embeddings->token_positions, vocab_size, lattice_dim);
        free(embeddings->tokens);
        free_bigfixed_matrix(embeddings->inverse_basis, lattice_dim, lattice_dim);
        free_bigfixed_matrix(embeddings->lattice_basis, lattice_dim, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        embeddings->morphology_graph[i] = (uint32_t*)calloc(MAX_DERIVED_TOKENS, sizeof(uint32_t));
        if (!embeddings->morphology_graph[i]) {
            // Cleanup
            for (uint32_t j = 0; j < i; j++) {
                free(embeddings->morphology_graph[j]);
            }
            free(embeddings->morphology_graph);
            free(embeddings->token_primes);
            free_bigfixed_matrix(embeddings->token_positions, vocab_size, lattice_dim);
            free(embeddings->tokens);
            free_bigfixed_matrix(embeddings->inverse_basis, lattice_dim, lattice_dim);
            free_bigfixed_matrix(embeddings->lattice_basis, lattice_dim, lattice_dim);
            free(embeddings);
            return NULL;
        }
    }
    
    // Allocate morphology counts
    embeddings->morphology_counts = (uint8_t*)calloc(vocab_size, sizeof(uint8_t));
    if (!embeddings->morphology_counts) {
        for (uint32_t i = 0; i < vocab_size; i++) {
            free(embeddings->morphology_graph[i]);
        }
        free(embeddings->morphology_graph);
        free(embeddings->token_primes);
        free_bigfixed_matrix(embeddings->token_positions, vocab_size, lattice_dim);
        free(embeddings->tokens);
        free_bigfixed_matrix(embeddings->inverse_basis, lattice_dim, lattice_dim);
        free_bigfixed_matrix(embeddings->lattice_basis, lattice_dim, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    return embeddings;
}

/*
 * Free CrystallineEmbeddings structure
 */
void crystalline_embeddings_free(CrystallineEmbeddings* embeddings) {
    if (!embeddings) return;
    
    // Free morphology counts
    if (embeddings->morphology_counts) {
        free(embeddings->morphology_counts);
    }
    
    // Free morphology graph
    if (embeddings->morphology_graph) {
        for (uint32_t i = 0; i < embeddings->vocab_size; i++) {
            if (embeddings->morphology_graph[i]) {
                free(embeddings->morphology_graph[i]);
            }
        }
        free(embeddings->morphology_graph);
    }
    
    // Free token primes
    if (embeddings->token_primes) {
        free(embeddings->token_primes);
    }
    
    // Free token positions
    if (embeddings->token_positions) {
        free_bigfixed_matrix(embeddings->token_positions, embeddings->vocab_size, embeddings->lattice_dim);
    }
    
    // Free tokens (but don't free the token objects themselves - they're owned by caller)
    if (embeddings->tokens) {
        free(embeddings->tokens);
    }
    
    // Free inverse basis
    if (embeddings->inverse_basis) {
        free_bigfixed_matrix(embeddings->inverse_basis, embeddings->lattice_dim, embeddings->lattice_dim);
    }
    
    // Free lattice basis
    if (embeddings->lattice_basis) {
        free_bigfixed_matrix(embeddings->lattice_basis, embeddings->lattice_dim, embeddings->lattice_dim);
    }
    
    free(embeddings);
}

/*
 * Add token to embeddings
 */
bool crystalline_embeddings_add_token(CrystallineEmbeddings* embeddings, CrystallineToken* token) {
    if (!embeddings || !token) {
        fprintf(stderr, "Error: NULL embeddings or token\n");
        return false;
    }
    
    if (token->token_id >= embeddings->vocab_size) {
        fprintf(stderr, "Error: token_id %u >= vocab_size %u\n", token->token_id, embeddings->vocab_size);
        return false;
    }
    
    // Store token pointer
    embeddings->tokens[token->token_id] = token;
    
    // Store prime
    embeddings->token_primes[token->token_id] = token->prime;
    
    // Copy token position (from Ulam spiral coordinates)
    for (uint32_t i = 0; i < embeddings->lattice_dim && i < 3; i++) {
        big_fixed_assign(&embeddings->token_positions[token->token_id][i], &token->lattice_coords[i]);
    }
    
    return true;
}

/*
 * Get token from embeddings
 */
CrystallineToken* crystalline_embeddings_get_token(CrystallineEmbeddings* embeddings, uint32_t token_id) {
    if (!embeddings) {
        fprintf(stderr, "Error: NULL embeddings\n");
        return NULL;
    }
    
    if (token_id >= embeddings->vocab_size) {
        fprintf(stderr, "Error: token_id %u >= vocab_size %u\n", token_id, embeddings->vocab_size);
        return NULL;
    }
    
    return embeddings->tokens[token_id];
}

/*
 * Initialize lattice basis to identity matrix
 */
bool crystalline_initialize_basis(CrystallineEmbeddings* embeddings) {
    if (!embeddings) {
        fprintf(stderr, "Error: NULL embeddings\n");
        return false;
    }
    
    // Set basis to identity matrix
    for (uint32_t i = 0; i < embeddings->lattice_dim; i++) {
        for (uint32_t j = 0; j < embeddings->lattice_dim; j++) {
            if (i == j) {
                // Set to 1.0
                big_from_int(embeddings->lattice_basis[i][j].integer_part, 1);
                big_from_int(embeddings->lattice_basis[i][j].fractional_part, 0);
            } else {
                // Set to 0.0
                big_from_int(embeddings->lattice_basis[i][j].integer_part, 0);
                big_from_int(embeddings->lattice_basis[i][j].fractional_part, 0);
            }
            embeddings->lattice_basis[i][j].negative = 0;
        }
    }
    
    // Set inverse basis to identity matrix (same as basis for identity)
    for (uint32_t i = 0; i < embeddings->lattice_dim; i++) {
        for (uint32_t j = 0; j < embeddings->lattice_dim; j++) {
            if (i == j) {
                big_from_int(embeddings->inverse_basis[i][j].integer_part, 1);
                big_from_int(embeddings->inverse_basis[i][j].fractional_part, 0);
            } else {
                big_from_int(embeddings->inverse_basis[i][j].integer_part, 0);
                big_from_int(embeddings->inverse_basis[i][j].fractional_part, 0);
            }
            embeddings->inverse_basis[i][j].negative = 0;
        }
    }
    
    return true;
}

/*
 * Compute token position (currently just copies from token's lattice_coords)
 */
void crystalline_compute_token_position(CrystallineEmbeddings* embeddings, uint32_t token_id, BigFixed position[3]) {
    if (!embeddings || !position) {
        fprintf(stderr, "Error: NULL embeddings or position\n");
        return;
    }
    
    if (token_id >= embeddings->vocab_size) {
        fprintf(stderr, "Error: token_id %u >= vocab_size %u\n", token_id, embeddings->vocab_size);
        return;
    }
    
    // Copy position from stored token positions
    for (uint32_t i = 0; i < embeddings->lattice_dim && i < 3; i++) {
        big_fixed_assign(&position[i], &embeddings->token_positions[token_id][i]);
    }
}
