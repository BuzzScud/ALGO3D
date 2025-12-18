/*
 * Pure Crystalline CLLM - Core Types and Structures
 * 
 * PURE implementation using base-60 arbitrary precision mathematics.
 * ALL internal calculations use NEW Crystalline Math Library (double precision for 3D coordinates).
 * Floats ONLY for input/output conversion.
 */

#ifndef CLLM_PURE_CRYSTALLINE_H
#define CLLM_PURE_CRYSTALLINE_H

// OLD library headers removed - now using NEW Crystalline Math Library
// No BigInt/BigFixed dependencies - using double precision for 3D coordinates
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PRIME_FACTORS 16
#define MAX_NEIGHBORS 12
#define MAX_DERIVED_TOKENS 64

/*
 * CrystallineToken - Pure Prime-Based Token Representation
 */
typedef struct {
    uint32_t token_id;
    char token_str[64];
    uint64_t prime;
    uint64_t prime_factors[MAX_PRIME_FACTORS];
    uint8_t num_factors;
    bool is_root;
    double lattice_coords[3];  // 3D lattice position (double precision sufficient)
    uint32_t neighbors[MAX_NEIGHBORS];
    uint8_t num_neighbors;
    uint32_t root_token_id;
    uint64_t usage_count;
    double root_score;
} CrystallineToken;

/*
 * LatticeReductionParams - Parameters for LLL reduction
 */
typedef struct {
    double delta;              // LLL parameter (typically 0.75)
    uint32_t max_iterations;   // Maximum LLL iterations
    int precision;             // Precision parameter (deprecated, kept for compatibility)
    bool verbose;              // Print progress
} LatticeReductionParams;

/*
 * CrystallineEmbeddings - Pure Lattice-Based Embeddings
 */
typedef struct {
    uint32_t vocab_size;           // Total vocabulary size
    uint32_t lattice_dim;          // Lattice dimension (3 for now)
    
    // LLL-Reduced Lattice Basis
    double** lattice_basis;        // [lattice_dim][lattice_dim] matrix (double precision sufficient)
    double** inverse_basis;        // For coordinate transformations
    
    // Token Storage
    CrystallineToken** tokens;     // Array of all tokens
    double** token_positions;      // [vocab_size][lattice_dim] positions
    uint64_t* token_primes;        // Prime for each token
    
    // Morphology Graph
    uint32_t** morphology_graph;   // [vocab_size][MAX_DERIVED_TOKENS]
    uint8_t* morphology_counts;    // Number of derived tokens per root
    
    // Optimization State
    bool basis_optimized;          // True if LLL reduction applied
    uint64_t optimization_epoch;   // Last optimization timestamp
    
    // Statistics
    uint64_t total_lookups;
    uint64_t cache_hits;
    double avg_lookup_time;
} CrystallineEmbeddings;

/*
 * Prime Operations
 */
bool crystalline_is_prime(uint64_t n);
uint64_t crystalline_get_nth_prime(uint32_t n);
void crystalline_factorize(uint64_t number, uint64_t* factors, uint8_t* num_factors);

/*
 * Token Operations
 */
CrystallineToken* crystalline_token_create(uint32_t token_id, const char* token_str, uint64_t prime);
void crystalline_token_free(CrystallineToken* token);
void crystalline_compute_ulam_position(uint64_t prime, double coords[3], int precision);

/*
 * Lattice Operations
 */
void crystalline_lattice_distance(const double pos1[3], const double pos2[3], double* distance);
void crystalline_prime_similarity(uint64_t prime1, uint64_t prime2, double* similarity);
void crystalline_phase_alignment(uint64_t prime1, uint64_t prime2, double* alignment);

/*
 * Embeddings Creation & Management
 */
CrystallineEmbeddings* crystalline_embeddings_create(uint32_t vocab_size, uint32_t lattice_dim);
void crystalline_embeddings_free(CrystallineEmbeddings* embeddings);
bool crystalline_embeddings_add_token(CrystallineEmbeddings* embeddings, CrystallineToken* token);
CrystallineToken* crystalline_embeddings_get_token(CrystallineEmbeddings* embeddings, uint32_t token_id);

/*
 * Lattice Basis Operations
 */
bool crystalline_initialize_basis(CrystallineEmbeddings* embeddings);
bool crystalline_optimize_basis(CrystallineEmbeddings* embeddings, LatticeReductionParams* params);
void crystalline_transform_to_lattice(CrystallineEmbeddings* embeddings, const double coords[3], double lattice_coords[3]);
void crystalline_transform_from_lattice(CrystallineEmbeddings* embeddings, const double lattice_coords[3], double coords[3]);

/*
 * Token Position Operations
 */
void crystalline_compute_token_position(CrystallineEmbeddings* embeddings, uint32_t token_id, double position[3]);
bool crystalline_find_nearest_token(CrystallineEmbeddings* embeddings, const double query[3], uint32_t* nearest_id, double* distance);
bool crystalline_get_k_nearest_tokens(CrystallineEmbeddings* embeddings, const double query[3], uint32_t k, uint32_t* nearest_ids, double* distances);

/*
 * Morphology Graph Operations
 */
bool crystalline_build_morphology_graph(CrystallineEmbeddings* embeddings);
uint32_t crystalline_get_root_token(CrystallineEmbeddings* embeddings, uint32_t token_id);
bool crystalline_get_derived_tokens(CrystallineEmbeddings* embeddings, uint32_t root_id, uint32_t* derived_ids, uint8_t* count);

/*
 * Similarity & Distance Operations
 */
void crystalline_token_similarity(CrystallineEmbeddings* embeddings, uint32_t token1_id, uint32_t token2_id, double* similarity);
bool crystalline_batch_similarities(CrystallineEmbeddings* embeddings, uint32_t query_id, uint32_t* token_ids, uint32_t num_tokens, double* similarities);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_PURE_CRYSTALLINE_H */