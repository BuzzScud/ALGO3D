/*
 * Pure Crystalline CLLM - Embeddings Implementation
 * 
 * Implements CrystallineEmbeddings with LLL-reduced lattice basis,
 * token positions, and morphology graph.
 * 
 * MIGRATED TO NEW MATH LIBRARY: Uses double precision for 3D coordinates.
 */

#include "cllm_pure_crystalline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Helper function to allocate double matrix
 */
static double** allocate_double_matrix(uint32_t rows, uint32_t cols) {
    double** matrix = (double**)malloc(rows * sizeof(double*));
    if (!matrix) return NULL;
    
    for (uint32_t i = 0; i < rows; i++) {
        matrix[i] = (double*)calloc(cols, sizeof(double));
        if (!matrix[i]) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }
    
    return matrix;
}

/*
 * Helper function to free double matrix
 */
static void free_double_matrix(double** matrix, uint32_t rows) {
    if (!matrix) return;
    
    for (uint32_t i = 0; i < rows; i++) {
        if (matrix[i]) {
            free(matrix[i]);
        }
    }
    free(matrix);
}

/*
 * Create new CrystallineEmbeddings structure
 */
CrystallineEmbeddings* crystalline_embeddings_create(uint32_t vocab_size, uint32_t lattice_dim) {
    if (vocab_size == 0 || lattice_dim == 0) return NULL;
    
    CrystallineEmbeddings* embeddings = (CrystallineEmbeddings*)calloc(1, sizeof(CrystallineEmbeddings));
    if (!embeddings) return NULL;
    
    embeddings->vocab_size = vocab_size;
    embeddings->lattice_dim = lattice_dim;
    
    // Allocate lattice basis matrices
    embeddings->lattice_basis = allocate_double_matrix(lattice_dim, lattice_dim);
    if (!embeddings->lattice_basis) {
        free(embeddings);
        return NULL;
    }
    
    embeddings->inverse_basis = allocate_double_matrix(lattice_dim, lattice_dim);
    if (!embeddings->inverse_basis) {
        free_double_matrix(embeddings->lattice_basis, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    // Allocate token storage
    embeddings->tokens = (CrystallineToken**)calloc(vocab_size, sizeof(CrystallineToken*));
    if (!embeddings->tokens) {
        free_double_matrix(embeddings->inverse_basis, lattice_dim);
        free_double_matrix(embeddings->lattice_basis, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    embeddings->token_positions = allocate_double_matrix(vocab_size, lattice_dim);
    if (!embeddings->token_positions) {
        free(embeddings->tokens);
        free_double_matrix(embeddings->inverse_basis, lattice_dim);
        free_double_matrix(embeddings->lattice_basis, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    embeddings->token_primes = (uint64_t*)calloc(vocab_size, sizeof(uint64_t));
    if (!embeddings->token_primes) {
        free_double_matrix(embeddings->token_positions, vocab_size);
        free(embeddings->tokens);
        free_double_matrix(embeddings->inverse_basis, lattice_dim);
        free_double_matrix(embeddings->lattice_basis, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    // Allocate morphology graph
    embeddings->morphology_graph = (uint32_t**)calloc(vocab_size, sizeof(uint32_t*));
    if (!embeddings->morphology_graph) {
        free(embeddings->token_primes);
        free_double_matrix(embeddings->token_positions, vocab_size);
        free(embeddings->tokens);
        free_double_matrix(embeddings->inverse_basis, lattice_dim);
        free_double_matrix(embeddings->lattice_basis, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        embeddings->morphology_graph[i] = (uint32_t*)calloc(MAX_DERIVED_TOKENS, sizeof(uint32_t));
        if (!embeddings->morphology_graph[i]) {
            for (uint32_t j = 0; j < i; j++) {
                free(embeddings->morphology_graph[j]);
            }
            free(embeddings->morphology_graph);
            free(embeddings->token_primes);
            free_double_matrix(embeddings->token_positions, vocab_size);
            free(embeddings->tokens);
            free_double_matrix(embeddings->inverse_basis, lattice_dim);
            free_double_matrix(embeddings->lattice_basis, lattice_dim);
            free(embeddings);
            return NULL;
        }
    }
    
    embeddings->morphology_counts = (uint8_t*)calloc(vocab_size, sizeof(uint8_t));
    if (!embeddings->morphology_counts) {
        for (uint32_t i = 0; i < vocab_size; i++) {
            free(embeddings->morphology_graph[i]);
        }
        free(embeddings->morphology_graph);
        free(embeddings->token_primes);
        free_double_matrix(embeddings->token_positions, vocab_size);
        free(embeddings->tokens);
        free_double_matrix(embeddings->inverse_basis, lattice_dim);
        free_double_matrix(embeddings->lattice_basis, lattice_dim);
        free(embeddings);
        return NULL;
    }
    
    // Initialize identity basis
    for (uint32_t i = 0; i < lattice_dim; i++) {
        for (uint32_t j = 0; j < lattice_dim; j++) {
            if (i == j) {
                embeddings->lattice_basis[i][j] = 1.0;
                embeddings->inverse_basis[i][j] = 1.0;
            } else {
                embeddings->lattice_basis[i][j] = 0.0;
                embeddings->inverse_basis[i][j] = 0.0;
            }
        }
    }
    
    embeddings->basis_optimized = false;
    embeddings->optimization_epoch = 0;
    embeddings->total_lookups = 0;
    embeddings->cache_hits = 0;
    embeddings->avg_lookup_time = 0.0;
    
    return embeddings;
}

/*
 * Free CrystallineEmbeddings structure
 */
void crystalline_embeddings_free(CrystallineEmbeddings* embeddings) {
    if (!embeddings) return;
    
    // Free tokens
    if (embeddings->tokens) {
        for (uint32_t i = 0; i < embeddings->vocab_size; i++) {
            if (embeddings->tokens[i]) {
                crystalline_token_free(embeddings->tokens[i]);
            }
        }
        free(embeddings->tokens);
    }
    
    // Free matrices
    if (embeddings->lattice_basis) {
        free_double_matrix(embeddings->lattice_basis, embeddings->lattice_dim);
    }
    if (embeddings->inverse_basis) {
        free_double_matrix(embeddings->inverse_basis, embeddings->lattice_dim);
    }
    if (embeddings->token_positions) {
        free_double_matrix(embeddings->token_positions, embeddings->vocab_size);
    }
    
    // Free arrays
    if (embeddings->token_primes) {
        free(embeddings->token_primes);
    }
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
    
    free(embeddings);
}

/*
 * Add token to embeddings
 */
bool crystalline_embeddings_add_token(CrystallineEmbeddings* embeddings, CrystallineToken* token) {
    if (!embeddings || !token) return false;
    if (token->token_id >= embeddings->vocab_size) return false;
    
    // Store token
    embeddings->tokens[token->token_id] = token;
    embeddings->token_primes[token->token_id] = token->prime;
    
    // Copy lattice coordinates
    for (uint32_t i = 0; i < embeddings->lattice_dim && i < 3; i++) {
        embeddings->token_positions[token->token_id][i] = token->lattice_coords[i];
    }
    
    return true;
}

/*
 * Compute token position (lookup from stored positions)
 */
void crystalline_compute_token_position(CrystallineEmbeddings* embeddings, uint32_t token_id, double position[3]) {
    if (!embeddings || !position) return;
    if (token_id >= embeddings->vocab_size) return;
    
    // Copy stored position
    for (uint32_t i = 0; i < embeddings->lattice_dim && i < 3; i++) {
        position[i] = embeddings->token_positions[token_id][i];
    }
}

