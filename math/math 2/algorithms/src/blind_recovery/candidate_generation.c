/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file candidate_generation.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief SFT-based candidate generation for iterative search
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Generates candidate values using Sieve-Free Testing (SFT) heuristics
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * combined with nonce-based randomization and geometric constraints.
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * MIGRATED: Uses NEW math library (prime_is_prime)
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 */
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "blind_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include "math/prime.h"  // NEW math library

// Helper function to check primality using NEW math library
static inline bool check_prime(uint64_t n) {
    return prime_is_prime(n);
}

/**
 * Find next prime after n
 */
static uint64_t next_prime(uint64_t n) {
    if (n < 2) return 2;
    if (n == 2) return 3;
    
    // Start with next odd number
    uint64_t candidate = (n % 2 == 0) ? n + 1 : n + 2;
    
    // Search for next prime (limit search to avoid infinite loop)
    for (uint32_t i = 0; i < 1000; i++) {
        if (check_prime(candidate)) {
            return candidate;
        }
        candidate += 2;
    }
    
    // Fallback: return n if no prime found
    return n;
}

/**
 * Generate nonce-based candidate using SFT
 * 
 * Uses nonce as seed for deterministic but pseudo-random candidate generation
 */
static uint64_t generate_nonce_candidate(
    uint64_t nonce,
    uint32_t dimension,
    const StructuralMap* structure
) {
    // Combine nonce with dimension and structural properties
    uint64_t seed = nonce ^ (dimension * 0x9e3779b97f4a7c15ULL);
    
    // Add structural information
    seed ^= structure->num_vertices * 0x517cc1b727220a95ULL;
    seed ^= structure->num_edges * 0x85ebca6b;
    seed ^= structure->num_faces * 0xc2b2ae35;
    
    // Find next prime near seed
    uint64_t candidate = next_prime(seed % 1000000);
    
    return candidate;
}

/**
 * Compute clock lattice position for candidate
 */
static uint32_t compute_clock_position(uint64_t candidate) {
    // Map to clock lattice (12, 60, 60, 100)
    uint32_t ring1 = candidate % 12;
    uint32_t ring2 = (candidate / 12) % 60;
    uint32_t ring3 = (candidate / 720) % 60;
    uint32_t ring4 = (candidate / 43200) % 100;
    
    // Combine into single position
    return ring1 + ring2 * 12 + ring3 * 720 + ring4 * 43200;
}

/**
 * Score candidate using SFT heuristic
 * 
 * Higher score = better candidate
 * Factors: primality, structural fit, geometric consistency
 */
static double score_candidate_sft(
    uint64_t candidate,
    uint32_t dimension,
    const StructuralMap* structure
) {
    double score = 0.0;
    
    // Factor 1: Primality (primes score higher)
    if (check_prime(candidate)) {
        score += 10.0;
    }
    
    // Factor 2: Structural fit (how well it fits Euler's formula)
    // V - E + F = 2
    int euler_delta = abs((int)structure->num_vertices - 
                         (int)structure->num_edges + 
                         (int)structure->num_faces - 2);
    score += 5.0 / (1.0 + euler_delta);
    
    // Factor 3: Dimension consistency
    // Prefer candidates that align with dimension size
    uint64_t dim_size = 0;
    if (dimension == 0) dim_size = structure->num_vertices;
    else if (dimension == 1) dim_size = structure->num_edges;
    else if (dimension == 2) dim_size = structure->num_faces;
    
    if (dim_size > 0) {
        double ratio = (double)candidate / dim_size;
        // Prefer ratios close to small integers (2, 3, 5, 7, 11, 13)
        double best_ratio_score = 0.0;
        int small_primes[] = {2, 3, 5, 7, 11, 13};
        for (int i = 0; i < 6; i++) {
            double diff_val = ratio - small_primes[i];
            double diff = (diff_val < 0) ? -diff_val : diff_val;
            double ratio_score = 3.0 / (1.0 + diff);
            if (ratio_score > best_ratio_score) {
                best_ratio_score = ratio_score;
            }
        }
        score += best_ratio_score;
    }
    
    // Factor 4: Clock lattice alignment
    uint32_t clock_pos = compute_clock_position(candidate);
    // Prefer positions that are multiples of 12 (symmetry)
    if (clock_pos % 12 == 0) {
        score += 2.0;
    }
    
    return score;
}

/**
 * Generate candidates using SFT
 */
SearchCandidate* generate_candidates(
    uint64_t nonce,
    uint32_t dimension,
    uint32_t num_candidates,
    const StructuralMap* structure
) {
    if (!structure || num_candidates == 0) {
        return NULL;
    }
    
    SearchCandidate* candidates = (SearchCandidate*)calloc(
        num_candidates,
        sizeof(SearchCandidate)
    );
    
    if (!candidates) {
        return NULL;
    }
    
    // Generate candidates with varying nonces
    for (uint32_t i = 0; i < num_candidates; i++) {
        uint64_t candidate_nonce = nonce + i * 0x9e3779b97f4a7c15ULL;
        uint64_t value = generate_nonce_candidate(
            candidate_nonce,
            dimension,
            structure
        );
        
        candidates[i].candidate = value;
        candidates[i].dimension = dimension;
        candidates[i].sft_score = score_candidate_sft(value, dimension, structure);
        candidates[i].is_prime = check_prime(value);
        candidates[i].clock_position = compute_clock_position(value);
    }
    
    // Sort candidates by score (descending)
    // Simple bubble sort for small arrays
    for (uint32_t i = 0; i < num_candidates - 1; i++) {
        for (uint32_t j = 0; j < num_candidates - i - 1; j++) {
            if (candidates[j].sft_score < candidates[j + 1].sft_score) {
                SearchCandidate temp = candidates[j];
                candidates[j] = candidates[j + 1];
                candidates[j + 1] = temp;
            }
        }
    }
    
    return candidates;
}

/**
 * Generate candidates for all dimensions
 */
SearchCandidate** generate_candidates_all_dimensions(
    uint64_t nonce,
    uint32_t num_candidates_per_dim,
    const StructuralMap* structure,
    uint32_t* total_candidates
) {
    if (!structure || !total_candidates) {
        return NULL;
    }
    
    // Generate for 3 dimensions (vertices, edges, faces)
    const uint32_t NUM_DIMS = 3;
    *total_candidates = num_candidates_per_dim * NUM_DIMS;
    
    SearchCandidate** all_candidates = (SearchCandidate**)calloc(
        NUM_DIMS,
        sizeof(SearchCandidate*)
    );
    
    if (!all_candidates) {
        return NULL;
    }
    
    for (uint32_t dim = 0; dim < NUM_DIMS; dim++) {
        all_candidates[dim] = generate_candidates(
            nonce + dim * 0x517cc1b727220a95ULL,
            dim,
            num_candidates_per_dim,
            structure
        );
        
        if (!all_candidates[dim]) {
            // Cleanup on failure
            for (uint32_t j = 0; j < dim; j++) {
                free(all_candidates[j]);
            }
            free(all_candidates);
            return NULL;
        }
    }
    
    return all_candidates;
}

/**
 * Free candidate array
 */
void free_candidates(SearchCandidate* candidates) {
    free(candidates);
}

/**
 * Free all candidates
 */
void free_all_candidates(SearchCandidate** candidates, uint32_t num_dimensions) {
    if (!candidates) return;
    
    for (uint32_t i = 0; i < num_dimensions; i++) {
        free(candidates[i]);
    }
    free(candidates);
}
