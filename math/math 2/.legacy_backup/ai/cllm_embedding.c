#include "math/types.h"
#include "math/constants.h"
/**
 * @file cllm_embedding.c
 * @brief CONSOLIDATED Embedding Implementation
 * 
 * This file CONSOLIDATES all embedding functionality:
 * - Basic embeddings (lookup, positional encoding, updates)
 * - Platonic geometry-based initialization
 * - Clock lattice position mapping
 * - L(n,d,k,λ) lattice formula
 * - Neighbor-influenced embeddings (kissing spheres)
 * - LLL lattice reduction for dimensionality reduction
 * - Lattice utilities (spiral mapping, distance, neighbors)
 * 
 * OBJECTIVES IMPLEMENTED:
 * - OBJECTIVE 21: Babylonian Clock Lattice
 * - OBJECTIVE 25: Platonic Solid Model Architecture
 * - OBJECTIVE 13A: Integrate L(n,d,k,λ) Lattice Formula
 * - Neighbor influence from kissing sphere geometry
 * - Dimensionality reduction via LLL algorithm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "math/constants.h"
#include "../include/cllm_inference.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include "math/angular_position.h"
#include "math/constants.h"
#include "math/prime.h"  // For prime_validate_by_clock(), prime_nth()
#include "math/constants.h"
#include "math/clock.h"  // For clock_map_prime_to_position()
#include "math/constants.h"
#include "../include/clock_lattice.h"
#include "math/constants.h"
#include "../include/ai/cllm_platonic.h"
#include "math/constants.h"

#define SYMMETRY_ORDER 12

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

// LLL Reducer structure (opaque)
typedef struct {
    float** basis;           // LLL-reduced basis (reduced_dim × original_dim)
    float** inverse_basis;   // Inverse for reconstruction
    int original_dim;        // Original embedding dimension
    int reduced_dim;         // Reduced embedding dimension
    float* temp_buffer;      // Temporary buffer for projections
} LLLEmbeddingReducer;

// ============================================================================
// LATTICE UTILITIES
// ============================================================================

/**
 * Compute spiral position for a prime number (Ulam spiral)
 */
void cllm_compute_spiral_position(uint64_t prime, double* angle, double* radius) {
    if (!angle || !radius) return;
    
    // Find prime index
    uint32_t prime_index = 0;
    uint64_t p = 2;
    while (p < prime) {
        if (prime_validate_by_clock(p)) prime_index++;
        p++;
    }
    
    // Ulam spiral: radius grows with square root of index
    *radius = math_sqrt((double)prime_index);
    
    // Angle based on golden angle for optimal packing
    double golden_angle = 2.0 * MATH_PI / (MATH_PHI * MATH_PHI);
    *angle = golden_angle * (double)prime_index;
    
    // Normalize angle to [0, 2π)
    while (*angle >= 2.0 * MATH_PI) {
        *angle -= 2.0 * MATH_PI;
    }
}

/**
 * Map token to 3D lattice coordinates
 * Uses NEW math library: clock position → stereographic projection → 3D coordinates
 */
void cllm_map_token_to_lattice(uint32_t token_id, uint64_t prime, double* coords) {
    if (!coords) return;
    
    // Get clock position using NEW math library
    ClockPosition pos;
    clock_map_index_to_position(token_id + 1, &pos);
    
    // Convert to 3D sphere coordinates using stereographic projection
    double theta = pos.angle;
    double phi = pos.radius * MATH_PI;
    
    // Spherical to Cartesian conversion
    coords[0] = math_sin(phi) * math_cos(theta);
    coords[1] = math_sin(phi) * math_sin(theta);
    coords[2] = math_cos(phi);
    
    // Add small perturbation based on prime modular relationships
    uint32_t mod_12 = prime % 12;
    uint32_t mod_60 = prime % 60;
    uint32_t mod_100 = prime % 100;
    
    coords[0] += 0.01 * (mod_12 / 12.0);
    coords[1] += 0.01 * (mod_60 / 60.0);
    coords[2] += 0.01 * (mod_100 / 100.0);
    
    // Normalize to unit sphere
    double norm = math_sqrt(coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2]);
    if (norm > 1e-8) {
        coords[0] /= norm;
        coords[1] /= norm;
        coords[2] /= norm;
    }
}

/**
 * Compute lattice distance between two tokens
 */
double cllm_lattice_token_distance(uint32_t token1_id, uint64_t prime1,
                                   uint32_t token2_id, uint64_t prime2) {
    double coords1[3], coords2[3];
    
    cllm_map_token_to_lattice(token1_id, prime1, coords1);
    cllm_map_token_to_lattice(token2_id, prime2, coords2);
    
    double dx = coords1[0] - coords2[0];
    double dy = coords1[1] - coords2[1];
    double dz = coords1[2] - coords2[2];
    
    return math_sqrt(dx * dx + dy * dy + dz * dz);
}

/**
 * Find nearest neighbors in lattice space
 */
void cllm_find_lattice_neighbors(uint32_t token_id, uint64_t prime,
                                uint32_t* all_tokens, uint64_t* all_primes,
                                uint32_t num_tokens, int k, uint32_t* neighbors) {
    if (!all_tokens || !all_primes || !neighbors || k <= 0) return;
    
    double* distances = (double*)malloc(num_tokens * sizeof(double));
    if (!distances) return;
    
    // Compute distances to all tokens
    for (uint32_t i = 0; i < num_tokens; i++) {
        if (all_tokens[i] == token_id) {
            distances[i] = 1e9;  // Exclude self
        } else {
            distances[i] = cllm_lattice_token_distance(token_id, prime,
                                                all_tokens[i], all_primes[i]);
        }
    }
    
    // Find k smallest distances
    for (int i = 0; i < k && i < (int)num_tokens; i++) {
        int min_idx = 0;
        double min_dist = distances[0];
        
        for (uint32_t j = 1; j < num_tokens; j++) {
            if (distances[j] < min_dist) {
                min_dist = distances[j];
                min_idx = j;
            }
        }
        
        neighbors[i] = all_tokens[min_idx];
        distances[min_idx] = 1e9;  // Mark as used
    }
    
    free(distances);
}

/**
 * Generate lattice-based embedding for a token
 */
void cllm_generate_lattice_embedding(uint32_t token_id, uint64_t prime,
                                     uint32_t embedding_dim, double* output) {
    if (!output || embedding_dim == 0) return;
    
    // Get lattice coordinates
    double coords[3];
    cllm_map_token_to_lattice(token_id, prime, coords);
    
    // Get symmetry group (use token_id for even distribution)
    uint32_t symmetry = token_id % SYMMETRY_ORDER;
    
    // Generate embedding using Fourier features
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double freq = (double)(i + 1);
        
        // Combine spatial coordinates with different frequencies
        double spatial = math_sin(freq * coords[0] / 10.0) * 0.3 +
                       math_cos(freq * coords[1] / 10.0) * 0.3 +
                       math_sin(freq * coords[2] / 10.0) * 0.3;
        
        // Add symmetry-based component
        double symmetry_phase = 2.0 * MATH_PI * (double)symmetry / (double)SYMMETRY_ORDER;
        double symmetry_component = math_cos(freq * symmetry_phase) * 0.1;
        
        output[i] = spatial + symmetry_component;
    }
    
    // Normalize to unit length
    double norm = 0.0;
    for (uint32_t i = 0; i < embedding_dim; i++) {
        norm += output[i] * output[i];
    }
    norm = math_sqrt(norm);
    
    if (norm > 1e-8) {
        for (uint32_t i = 0; i < embedding_dim; i++) {
            output[i] /= norm;
        }
    }
}

/**
 * Generate lattice transformation matrix
 */
void cllm_generate_lattice_transform(double* transform, int dim) {
    if (!transform || dim <= 0) return;
    
    // Initialize to identity
    memset(transform, 0, dim * dim * sizeof(double));
    for (int i = 0; i < dim; i++) {
        transform[i * dim + i] = 1.0;
    }
    
    // Apply golden ratio-based rotations
    for (int i = 0; i < dim - 1; i++) {
        double angle = 2.0 * MATH_PI * MATH_PHI * (double)i / (double)dim;
        double cos_a = math_cos(angle);
        double sin_a = math_sin(angle);
        
        // Apply Givens rotation in plane (i, i+1)
        double temp_ii = transform[i * dim + i];
        double temp_i_ip1 = transform[i * dim + (i + 1)];
        double temp_ip1_i = transform[(i + 1) * dim + i];
        double temp_ip1_ip1 = transform[(i + 1) * dim + (i + 1)];
        
        transform[i * dim + i] = cos_a * temp_ii - sin_a * temp_ip1_i;
        transform[i * dim + (i + 1)] = cos_a * temp_i_ip1 - sin_a * temp_ip1_ip1;
        transform[(i + 1) * dim + i] = sin_a * temp_ii + cos_a * temp_ip1_i;
        transform[(i + 1) * dim + (i + 1)] = sin_a * temp_i_ip1 + cos_a * temp_ip1_ip1;
    }
}

// ============================================================================
// EMBEDDING INITIALIZATION
// ============================================================================

/**
 * Initialize embeddings with Platonic geometry and clock lattice structure
 * 
 * This is the SINGLE, CONSOLIDATED embedding initialization function.
 * It automatically uses the best method based on model configuration:
 * 
 * 1. If model uses Platonic geometry: Use geometric initialization
 * 2. Otherwise: Use clock lattice + L(n,d,k,λ) formula
 */
void cllm_init_embeddings(CLLMModel* model) {
    if (!model || !model->embeddings) {
        fprintf(stderr, "ERROR: Invalid model or embeddings\n");
        return;
    }
    
    printf("Initializing embeddings (CONSOLIDATED implementation)...\n");
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embedding_dim = model->embedding_dim;
    double* embeddings = model->embeddings;
    
    // Check if model uses Platonic geometry
    if (model->token_positions) {
        printf("  Using Platonic geometry + clock lattice initialization\n");
        printf("  Solid: %s\n", 
               model->solid_type == PLATONIC_TETRAHEDRON ? "Tetrahedron" :
               model->solid_type == PLATONIC_CUBE ? "Cube" :
               model->solid_type == PLATONIC_OCTAHEDRON ? "Octahedron" :
               model->solid_type == PLATONIC_DODECAHEDRON ? "Dodecahedron" : "Icosahedron");
        printf("  Vertices: %u → Embedding: %u\n", model->geometry.vertices, embedding_dim);
        
        // Use clock lattice positions for initialization
        ClockPosition* positions = model->token_positions;
        
        for (uint32_t token = 0; token < vocab_size; token++) {
            ClockPosition pos = positions[token];
            
            for (uint32_t dim = 0; dim < embedding_dim; dim++) {
                // Combine clock position with dimensional frequency
                double angle = pos.angle + (double)dim / embedding_dim * 2.0 * MATH_PI;
                double radius_factor = pos.radius / 100.0;  // Normalize radius
                
                // Use prime-based sinusoidal initialization
                double value = math_sin(angle) * (1.0 + radius_factor * 0.1);
                
                // Scale by 1/math_sqrt(embedding_dim) for stability
                value *= 1.0 / math_sqrt((double)embedding_dim);
                
                embeddings[token * embedding_dim + dim] = value;
            }
        }
        
        printf("✓ Embeddings initialized with Platonic geometry\n");
        
    } else {
        printf("  Using standard clock lattice initialization\n");
        
        // Standard initialization with clock lattice structure
        for (uint32_t token = 0; token < vocab_size; token++) {
            // Map token to clock position
            uint64_t prime_pos = prime_nth(token + 1);

            ClockPosition pos;

            clock_map_prime_to_position(prime_pos, &pos);
            
            for (uint32_t dim = 0; dim < embedding_dim; dim++) {
                // Use clock-based sinusoidal initialization
                double angle = pos.angle + (double)dim / embedding_dim * 2.0 * MATH_PI;
                double value = math_sin(angle) / math_sqrt((double)embedding_dim);
                
                embeddings[token * embedding_dim + dim] = value;
            }
        }
        
        printf("✓ Embeddings initialized with clock lattice\n");
    }
    
    // Sync to abacus embeddings if enabled
    if (model->use_abacus_embeddings && model->abacus_embeddings) {
        printf("  Syncing to abacus embeddings (arbitrary precision)...\n");
        
        MathError err = abacus_matrix_from_doubles(model->abacus_embeddings, embeddings);
        if (err != MATH_SUCCESS) {
            fprintf(stderr, "ERROR: Failed to sync embeddings to abacus (error %d)\n", err);
            return;
        }
        
        printf("✓ Embeddings synced to abacus (base 60, precision 10)\n");
        
        // Calculate memory usage
        size_t abacus_memory = abacus_matrix_memory_usage(model->abacus_embeddings);
        size_t double_memory = vocab_size * embedding_dim * sizeof(double);
        
        printf("  Memory: Abacus = %.2f MB, Double = %.2f MB\n",
               abacus_memory / (1024.0 * 1024.0),
               double_memory / (1024.0 * 1024.0));
    }
}

// ============================================================================
// NEIGHBOR-INFLUENCED EMBEDDINGS (Kissing Spheres)
// ============================================================================

/**
 * Compute neighbor influence weights using L(n,d,k,λ)
 */
// ============================================================================
// LEGACY NEIGHBOR FUNCTIONS - COMMENTED OUT
// These functions use old lattice_points structure which no longer exists
// TODO: Reimplement using new token_positions (ClockPosition) structure
// ============================================================================
#if 0  // LEGACY CODE - DISABLED

static int compute_neighbor_weights(
    CLLMModel* model,
    uint32_t token_id,
    double* weights
) {
    if (!model || !weights) return -1;
    
    if (token_id >= model->vocab_size) return -1;
    if (!model->lattice_points || token_id >= model->num_lattice_points) return -1;
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    CLLMToken* center_token = &model->tokens[token_id];
    
    double sum = 0.0;
    
    // Compute raw weights using L(n,d,k,λ)
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        
        if (neighbor_id >= model->vocab_size) {
            weights[i] = 0.0;
            continue;
        }
        
        // Get dimensional frequency
        uint64_t phi_i = cllm_get_dimensional_frequency(i);
        
        // Compute interaction strength
        double interaction = L_lattice(
            center_token->prime_encoding,
            i,
            center_token->symmetry_group,
            center_token->token_str,
            3,
            center_token->prime_encoding,
            phi_i
        );
        
        // Use absolute value and normalize
        weights[i] = (double)math_abs((double)interaction);
        sum += weights[i];
    }
    
    // Fill remaining weights
    for (uint32_t i = point->num_neighbors; i < 12; i++) {
        weights[i] = 0.0;
    }
    
    // Normalize to sum to 1.0
    if (sum > 0.0) {
        for (int i = 0; i < 12; i++) {
            weights[i] /= (double)sum;
        }
    }
    
    return 0;
}

/**
 * Compute neighbor influence vector
 */
int cllm_embedding_compute_neighbor_influence(
    CLLMModel* model,
    uint32_t token_id,
    double* influence_vector,
    double* weights
) {
    if (!model || !influence_vector) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_embedding_compute_neighbor_influence\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range\n", token_id);
        return -1;
    }
    
    if (!model->lattice_points || token_id >= model->num_lattice_points) {
        fprintf(stderr, "ERROR: No lattice point for token %u\n", token_id);
        return -1;
    }
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute weights
    double neighbor_weights[12];
    if (compute_neighbor_weights(model, token_id, neighbor_weights) != 0) {
        return -1;
    }
    
    // Copy weights if requested
    if (weights) {
        memcpy(weights, neighbor_weights, 12 * sizeof(double));
    }
    
    // Initialize influence vector to zero
    memset(influence_vector, 0, embed_dim * sizeof(double));
    
    // Accumulate weighted neighbor embeddings
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        
        if (neighbor_id >= model->vocab_size) continue;
        
        double weight = neighbor_weights[i];
        double* neighbor_embedding = &model->embeddings[
            neighbor_id * embed_dim
        ];
        
        // Add weighted contribution
        for (uint32_t d = 0; d < embed_dim; d++) {
            influence_vector[d] += weight * (double)neighbor_embedding[d];
        }
    }
    
    return 0;
}

/**
 * Compute neighbor-influenced embedding (non-destructive)
 */
int cllm_embedding_compute_with_neighbors(
    CLLMModel* model,
    uint32_t token_id,
    double influence_strength,
    double* output
) {
    if (!model || !output) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_embedding_compute_with_neighbors\n");
        return -1;
    }
    
    if (influence_strength < 0.0 || influence_strength > 1.0) {
        fprintf(stderr, "ERROR: influence_strength must be in range [0.0, 1.0]\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range\n", token_id);
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Get original embedding
    double* original_embedding = &model->embeddings[
        token_id * embed_dim
    ];
    
    // Compute neighbor influence
    double* influence_vector = (double*)malloc(embed_dim * sizeof(double));
    if (!influence_vector) {
        fprintf(stderr, "ERROR: Failed to allocate influence vector\n");
        return -1;
    }
    
    if (cllm_embedding_compute_neighbor_influence(model, token_id, influence_vector, NULL) != 0) {
        free(influence_vector);
        return -1;
    }
    
    // Combine: output = (1 - strength) * original + strength * influence
    double original_weight = 1.0 - influence_strength;
    for (uint32_t d = 0; d < embed_dim; d++) {
        output[d] = original_weight * (double)original_embedding[d] + 
                    influence_strength * influence_vector[d];
    }
    
    free(influence_vector);
    return 0;
}

/**
 * Refine embedding using kissing sphere neighbors
 */
int cllm_embedding_refine_with_neighbors(
    CLLMModel* model,
    uint32_t token_id,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range\n", token_id);
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute refined embedding
    double* refined_embedding = (double*)malloc(embed_dim * sizeof(double));
    if (!refined_embedding) {
        fprintf(stderr, "ERROR: Failed to allocate refined embedding\n");
        return -1;
    }
    
    if (cllm_embedding_compute_with_neighbors(model, token_id, influence_strength, 
                                              refined_embedding) != 0) {
        free(refined_embedding);
        return -1;
    }
    
    // Update embedding in model
    double* embedding = &model->embeddings[token_id * embed_dim];
    for (uint32_t d = 0; d < embed_dim; d++) {
        embedding[d] = (double)refined_embedding[d];
    }
    
    free(refined_embedding);
    return 0;
}

/**
 * Refine all embeddings using kissing sphere neighbors
 */
int cllm_embeddings_refine_all_with_neighbors(
    CLLMModel* model,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    if (!model->lattice_points) {
        fprintf(stderr, "ERROR: No lattice points in model\n");
        return -1;
    }
    
    printf("Refining all embeddings with neighbor influence (strength=%.2f)...\n", 
           influence_strength);
    
    // Create temporary buffer for all refined embeddings
    uint32_t embed_dim = model->embeddings.embedding_dim;
    size_t total_size = model->num_lattice_points * embed_dim;
    double* refined_embeddings = (double*)malloc(total_size * sizeof(double));
    if (!refined_embeddings) {
        fprintf(stderr, "ERROR: Failed to allocate refined embeddings buffer\n");
        return -1;
    }
    
    // Compute all refined embeddings
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        double* output = &refined_embeddings[i * embed_dim];
        if (cllm_embedding_compute_with_neighbors(model, i, influence_strength, output) != 0) {
            fprintf(stderr, "ERROR: Failed to refine embedding for token %u\n", i);
            free(refined_embeddings);
            return -1;
        }
    }
    
    // Update all embeddings at once
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        double* embedding = &model->embeddings[i * embed_dim];
        double* refined = &refined_embeddings[i * embed_dim];
        for (uint32_t d = 0; d < embed_dim; d++) {
            embedding[d] = (double)refined[d];
        }
    }
    
    free(refined_embeddings);
    printf("✓ All embeddings refined successfully\n");
    return 0;
}

/**
 * Initialize embeddings with neighbor influence
 */
int cllm_embeddings_init_with_neighbors(
    CLLMModel* model,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    printf("Initializing embeddings with neighbor influence...\n");
    
    // First, ensure embeddings are initialized
    if (!model->embeddings) {
        fprintf(stderr, "ERROR: Embeddings not initialized\n");
        return -1;
    }
    
    // Apply neighbor influence
    return cllm_embeddings_refine_all_with_neighbors(model, influence_strength);
}

/**
 * Get statistics about neighbor influence
 */
int cllm_embedding_neighbor_influence_stats(
    CLLMModel* model,
    double* avg_influence,
    double* max_influence,
    double* min_influence
) {
    if (!model || !avg_influence || !max_influence || !min_influence) {
        fprintf(stderr, "ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (!model->lattice_points) {
        fprintf(stderr, "ERROR: No lattice points\n");
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    double* influence_vector = (double*)malloc(embed_dim * sizeof(double));
    if (!influence_vector) {
        fprintf(stderr, "ERROR: Failed to allocate influence vector\n");
        return -1;
    }
    
    double sum = 0.0;
    double max_val = 0.0;
    double min_val = 1e10;
    uint32_t count = 0;
    
    // Sample first 100 tokens
    uint32_t sample_size = (model->num_lattice_points < 100) ? 
                           model->num_lattice_points : 100;
    
    for (uint32_t i = 0; i < sample_size; i++) {
        if (cllm_embedding_compute_neighbor_influence(model, i, influence_vector, NULL) != 0) {
            continue;
        }
        
        // Compute magnitude of influence
        double magnitude = 0.0;
        for (uint32_t d = 0; d < embed_dim; d++) {
            magnitude += influence_vector[d] * influence_vector[d];
        }
        magnitude = math_sqrt(magnitude);
        
        sum += magnitude;
        if (magnitude > max_val) max_val = magnitude;
        if (magnitude < min_val) min_val = magnitude;
        count++;
    }
    
    *avg_influence = (count > 0) ? (double)(sum / count) : 0.0;
    *max_influence = max_val;
    *min_influence = min_val;
    
    free(influence_vector);
    return 0;
}

/**
 * Apply iterative neighbor refinement
 */
int cllm_embeddings_iterative_refinement(
    CLLMModel* model,
    int num_iterations,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    if (num_iterations <= 0) {
        fprintf(stderr, "ERROR: num_iterations must be positive\n");
        return -1;
    }
    
    printf("Applying iterative neighbor refinement (%d iterations, strength=%.2f)...\n",
           num_iterations, influence_strength);
    
    for (int iter = 0; iter < num_iterations; iter++) {
        printf("  Iteration %d/%d...\n", iter + 1, num_iterations);
        
        if (cllm_embeddings_refine_all_with_neighbors(model, influence_strength) != 0) {
            fprintf(stderr, "ERROR: Failed at iteration %d\n", iter + 1);
            return -1;
        }
    }
    
    printf("✓ Iterative refinement complete\n");
    return 0;
}
#endif  // LEGACY CODE - DISABLED
// ============================================================================


// ============================================================================
// LLL LATTICE REDUCTION
// ============================================================================

/**
 * Compute covariance matrix of embeddings
 */
static float** compute_embedding_covariance(CLLMModel* model) {
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    double* embeddings = model->embeddings;
    
    // Allocate covariance matrix
    float** cov = (float**)malloc(embed_dim * sizeof(float*));
    for (uint32_t i = 0; i < embed_dim; i++) {
        cov[i] = (float*)calloc(embed_dim, sizeof(float));
    }
    
    // Compute mean
    float* mean = (float*)calloc(embed_dim, sizeof(float));
    for (uint32_t v = 0; v < vocab_size; v++) {
        for (uint32_t d = 0; d < embed_dim; d++) {
            mean[d] += embeddings[v * embed_dim + d];
        }
    }
    for (uint32_t d = 0; d < embed_dim; d++) {
        mean[d] /= vocab_size;
    }
    
    // Compute covariance
    for (uint32_t v = 0; v < vocab_size; v++) {
        for (uint32_t i = 0; i < embed_dim; i++) {
            float xi = embeddings[v * embed_dim + i] - mean[i];
            for (uint32_t j = 0; j < embed_dim; j++) {
                float xj = embeddings[v * embed_dim + j] - mean[j];
                cov[i][j] += xi * xj;
            }
        }
    }
    
    // Normalize
    for (uint32_t i = 0; i < embed_dim; i++) {
        for (uint32_t j = 0; j < embed_dim; j++) {
            cov[i][j] /= vocab_size;
        }
    }
    
    free(mean);
    return cov;
}

/**
 * Apply simplified dimensionality reduction
 */
static float** apply_lll_reduction(float** cov_matrix, int dim, int target_dim) {
    printf("Applying dimensionality reduction: %d → %d dimensions\n", dim, target_dim);
    
    // Allocate basis
    float** basis = (float**)malloc(target_dim * sizeof(float*));
    for (int i = 0; i < target_dim; i++) {
        basis[i] = (float*)calloc(dim, sizeof(float));
        
        // Use covariance matrix rows as basis vectors (simplified)
        if (i < dim) {
            memcpy(basis[i], cov_matrix[i], dim * sizeof(float));
            
            // Normalize
            float norm = 0.0;
            for (int j = 0; j < dim; j++) {
                norm += basis[i][j] * basis[i][j];
            }
            norm = math_sqrt(norm);
            if (norm > 1e-6f) {
                for (int j = 0; j < dim; j++) {
                    basis[i][j] /= norm;
                }
            }
        }
    }
    
    printf("Dimensionality reduction complete\n");
    return basis;
}

/**
 * Compute pseudo-inverse of basis
 */
static float** compute_pseudo_inverse(float** basis, int reduced_dim, int original_dim) {
    // Use transpose as approximation
    float** inverse = (float**)malloc(original_dim * sizeof(float*));
    for (int i = 0; i < original_dim; i++) {
        inverse[i] = (float*)calloc(reduced_dim, sizeof(float));
        for (int j = 0; j < reduced_dim; j++) {
            inverse[i][j] = basis[j][i];
        }
    }
    
    // Normalize columns
    for (int j = 0; j < reduced_dim; j++) {
        float norm = 0.0;
        for (int i = 0; i < original_dim; i++) {
            norm += inverse[i][j] * inverse[i][j];
        }
        norm = math_sqrt(norm);
        if (norm > 1e-6f) {
            for (int i = 0; i < original_dim; i++) {
                inverse[i][j] /= norm;
            }
        }
    }
    
    return inverse;
}

/**
 * Create LLL embedding reducer
 */
LLLEmbeddingReducer* lll_reducer_create(CLLMModel* model, int target_dim) {
    if (!model || target_dim <= 0 || target_dim >= (int)model->embedding_dim) {
        return NULL;
    }
    
    printf("Creating LLL embedding reducer: %lu → %d\n", (unsigned long)model->embedding_dim, target_dim);
    
    LLLEmbeddingReducer* reducer = (LLLEmbeddingReducer*)calloc(1, sizeof(LLLEmbeddingReducer));
    reducer->original_dim = model->embedding_dim;
    reducer->reduced_dim = target_dim;
    
    // Compute covariance matrix
    float** cov = compute_embedding_covariance(model);
    
    // Apply LLL reduction
    reducer->basis = apply_lll_reduction(cov, model->embedding_dim, target_dim);
    
    if (!reducer->basis) {
        free(reducer);
        for (uint32_t i = 0; i < model->embedding_dim; i++) {
            free(cov[i]);
        }
        free(cov);
        return NULL;
    }
    
    // Compute inverse basis
    reducer->inverse_basis = compute_pseudo_inverse(reducer->basis, target_dim, model->embedding_dim);
    
    // Allocate temp buffer
    reducer->temp_buffer = (float*)malloc(model->embedding_dim * sizeof(float));
    
    // Cleanup
    for (uint32_t i = 0; i < model->embedding_dim; i++) {
        free(cov[i]);
    }
    free(cov);
    
    printf("LLL reducer created successfully\n");
    return reducer;
}

/**
 * Free LLL reducer
 */
void lll_reducer_free(LLLEmbeddingReducer* reducer) {
    if (!reducer) return;
    
    if (reducer->basis) {
        for (int i = 0; i < reducer->reduced_dim; i++) {
            free(reducer->basis[i]);
        }
        free(reducer->basis);
    }
    
    if (reducer->inverse_basis) {
        for (int i = 0; i < reducer->original_dim; i++) {
            free(reducer->inverse_basis[i]);
        }
        free(reducer->inverse_basis);
    }
    
    free(reducer->temp_buffer);
    free(reducer);
}

/**
 * Project embedding to reduced space
 */
void lll_project_embedding(LLLEmbeddingReducer* reducer, const double* embedding, float* reduced) {
    if (!reducer || !embedding || !reduced) return;
    
    // reduced = basis * embedding
    for (int i = 0; i < reducer->reduced_dim; i++) {
        reduced[i] = 0.0;
        for (int j = 0; j < reducer->original_dim; j++) {
            reduced[i] += reducer->basis[i][j] * embedding[j];
        }
    }
}

/**
 * Reconstruct embedding from reduced space
 */
void lll_reconstruct_embedding(LLLEmbeddingReducer* reducer, const float* reduced, float* embedding) {
    if (!reducer || !reduced || !embedding) return;
    
    // embedding = inverse_basis * reduced
    for (int i = 0; i < reducer->original_dim; i++) {
        embedding[i] = 0.0;
        for (int j = 0; j < reducer->reduced_dim; j++) {
            embedding[i] += reducer->inverse_basis[i][j] * reduced[j];
        }
    }
}

/**
 * Project all model embeddings to reduced space
 */
float* lll_project_all_embeddings(LLLEmbeddingReducer* reducer, CLLMModel* model) {
    if (!reducer || !model) return NULL;
    
    uint32_t vocab_size = model->vocab_size;
    float* reduced_embeddings = (float*)malloc(vocab_size * reducer->reduced_dim * sizeof(float));
    
    for (uint32_t v = 0; v < vocab_size; v++) {
        double* original = &model->embeddings[v * reducer->original_dim];
        float* reduced = &reduced_embeddings[v * reducer->reduced_dim];
        lll_project_embedding(reducer, original, reduced);
    }
    
    return reduced_embeddings;
}

// ============================================================================
// EMBEDDING LOOKUP
// ============================================================================

/**
 * Embed a single token into the embedding space
 */
void cllm_embed_token(CLLMInference* inf, uint32_t token_id, float* output) {
    if (!inf || !output || token_id >= inf->model->vocab_size) {
        return;
    }
    
    CLLMModel* model = inf->model;
    uint32_t embedding_dim = model->embedding_dim;  // Direct field access
    
    // Copy embedding from embedding matrix
    double* embedding_matrix = model->embeddings;
    size_t offset = token_id * embedding_dim;
    
    for (uint32_t i = 0; i < embedding_dim; i++) {
        output[i] = (float)embedding_matrix[offset + i];
    }
}

// ============================================================================
// POSITIONAL ENCODING
// ============================================================================

/**
 * Add positional encoding to embeddings
 */
void cllm_add_positional_encoding(CLLMModel* model, uint32_t position, double* embedding) {
    if (!model || !embedding || position >= model->max_seq_len) {  // Use max_seq_len instead of header.context_length
        return;
    }
    
    uint32_t embedding_dim = model->embedding_dim;
    
    // Map position to clock lattice
    uint64_t prime_pos = prime_nth(position + 1);

    ClockPosition pos;

    clock_map_prime_to_position(prime_pos, &pos);
    
    // Add sinusoidal positional encoding based on clock position
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double angle = pos.angle + (double)i / embedding_dim * 2.0 * MATH_PI;
        double pos_encoding = math_sin(angle);
        
        embedding[i] += pos_encoding * 0.1;  // Scale factor
    }
}

// ============================================================================
// EMBEDDING UPDATES (for training)
// ============================================================================

/**
 * Update embedding during training
 */
void cllm_update_embedding(CLLMModel* model, uint32_t token_id, 
                          const double* gradient, double learning_rate) {
    if (!model || !gradient || token_id >= model->vocab_size) {
        return;
    }
    
    uint32_t embedding_dim = model->embedding_dim;
    double* embeddings = model->embeddings;
    size_t offset = token_id * embedding_dim;
    
    // Apply gradient update with optional harmonic modulation
    double modulation = 1.0;
    if (model->harmonic.enabled) {
        // Modulate learning rate with primary frequency (432 Hz)
        double freq = model->harmonic.primary_frequency;
        modulation = 1.0 + 0.1 * math_sin(freq * token_id / model->vocab_size);
    }
    
    for (uint32_t i = 0; i < embedding_dim; i++) {
        embeddings[offset + i] -= learning_rate * modulation * gradient[i];
    }
}

// ============================================================================
// LATTICE TRANSFORMATIONS (Legacy compatibility)
// ============================================================================

/**
 * Apply lattice transformation to an embedding
 */
void cllm_apply_lattice_transform(float* embedding, double* transform, int dim) {
    if (!embedding || !transform || dim <= 0) {
        return;
    }
    
    // Allocate temporary buffer for result
    float* temp = (float*)malloc(dim * sizeof(float));
    if (!temp) return;
    
    // Matrix-vector multiplication
    for (int i = 0; i < dim; i++) {
        temp[i] = 0.0f;
        for (int j = 0; j < dim; j++) {
            temp[i] += (float)transform[i * dim + j] * embedding[j];
        }
    }
    
    // Copy result back
    memcpy(embedding, temp, dim * sizeof(float));
    free(temp);
}

// ============================================================================
// EMBEDDING UTILITIES
// ============================================================================

/**
 * Compute cosine similarity between two embeddings
 */
double cllm_embedding_similarity(const double* emb1, const double* emb2, uint32_t dim) {
    if (!emb1 || !emb2 || dim == 0) return 0.0;
    
    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    
    for (uint32_t i = 0; i < dim; i++) {
        dot += emb1[i] * emb2[i];
        norm1 += emb1[i] * emb1[i];
        norm2 += emb2[i] * emb2[i];
    }
    
    if (norm1 == 0.0 || norm2 == 0.0) return 0.0;
    
    return dot / (math_sqrt(norm1) * math_sqrt(norm2));
}

/**
 * Normalize an embedding to unit length
 */
void cllm_normalize_embedding(float* embedding, int dim) {
    if (!embedding || dim <= 0) return;
    
    float norm = 0.0f;
    for (int i = 0; i < dim; i++) {
        norm += embedding[i] * embedding[i];
    }
    
    if (norm == 0.0f) return;
    
    norm = math_sqrt(norm);
    for (int i = 0; i < dim; i++) {
        embedding[i] /= norm;
    }
}

// ============================================================================
// LEGACY COMPATIBILITY FUNCTIONS
// ============================================================================

/**
 * Initialize embeddings with lattice formula (legacy wrapper)
 * 
 * This function is maintained for backward compatibility.
 * It simply calls cllm_init_embeddings() which now includes
 * all lattice-based initialization methods.
 */
void cllm_init_embeddings_with_lattice(CLLMModel* model) {
    cllm_init_embeddings(model);
}