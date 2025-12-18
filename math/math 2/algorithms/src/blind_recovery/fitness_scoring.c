/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file fitness_scoring.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Fitness scoring for candidate evaluation
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Evaluates candidate fitness based on structural consistency,
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * oscillation patterns, and geometric constraints.
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

/**
 * Score candidate based on structural consistency
 */
static double score_structural_consistency(
    const SearchCandidate* candidate,
    const StructuralMap* structure
) {
    double score = 0.0;
    
    // Check Euler's formula consistency
    int euler_result = (int)structure->num_vertices - 
                      (int)structure->num_edges + 
                      (int)structure->num_faces;
    
    if (euler_result == 2) {
        score += 10.0;  // Perfect Euler's formula
    } else {
        score += 5.0 / (1.0 + abs(euler_result - 2));
    }
    
    // Check dimensional consistency
    uint64_t expected_size = 0;
    if (candidate->dimension == 0) {
        expected_size = structure->num_vertices;
    } else if (candidate->dimension == 1) {
        expected_size = structure->num_edges;
    } else if (candidate->dimension == 2) {
        expected_size = structure->num_faces;
    }
    
    if (expected_size > 0) {
        double ratio = (double)candidate->candidate / expected_size;
        // Prefer ratios close to 1.0 (natural scale)
        score += 5.0 / (1.0 + math_abs(ratio - 1.0));
    }
    
    // Bonus for prime candidates (structural stability)
    if (candidate->is_prime) {
        score += 3.0;
    }
    
    return score;
}

/**
 * Score candidate based on oscillation patterns
 */
static double score_oscillation_fit(
    const SearchCandidate* candidate,
    const OscillationMap* oscillations
) {
    if (!oscillations) {
        return 0.0;
    }
    
    double score = 0.0;
    
    // Check if candidate aligns with oscillation frequencies
    for (uint32_t i = 0; i < oscillations->num_dimensions; i++) {
        const OscillationSignature* sig = &oscillations->signatures[i];
        
        // Prefer candidates that match oscillation periods
        if (sig->period > 0) {
            uint64_t period_match = candidate->candidate % sig->period;
            if (period_match == 0) {
                score += 5.0;  // Perfect period match
            } else {
                score += 2.0 / (1.0 + (double)period_match / sig->period);
            }
        }
        
        // Bonus for stable oscillations
        if (sig->is_stable) {
            score += 2.0;
        }
    }
    
    // Check convergence
    if (oscillations->is_converging) {
        score += 5.0;
    }
    
    return score;
}

/**
 * Score candidate based on clock lattice position
 */
static double score_clock_lattice_fit(
    const SearchCandidate* candidate
) {
    double score = 0.0;
    
    uint32_t pos = candidate->clock_position;
    
    // Prefer positions at key symmetry points
    // Ring 1 (12): multiples of 3 (4-fold symmetry)
    if ((pos % 12) % 3 == 0) {
        score += 3.0;
    }
    
    // Ring 2 (60): multiples of 12 (5-fold symmetry)
    if ((pos % 720) % 12 == 0) {
        score += 2.0;
    }
    
    // Ring 3 (60): multiples of 12
    if ((pos % 43200) % 720 == 0) {
        score += 2.0;
    }
    
    // Ring 4 (100): multiples of 10 (decimal symmetry)
    if (pos % 4320000 == 0) {
        score += 1.0;
    }
    
    return score;
}

/**
 * Score candidate fitness (main function)
 */
double score_candidate(
    const SearchCandidate* candidate,
    const StructuralMap* structure,
    const OscillationMap* oscillations
) {
    if (!candidate || !structure) {
        return 0.0;
    }
    
    double total_score = 0.0;
    
    // Component 1: SFT score (already computed)
    total_score += candidate->sft_score * 1.0;
    
    // Component 2: Structural consistency
    total_score += score_structural_consistency(candidate, structure) * 1.5;
    
    // Component 3: Oscillation fit
    if (oscillations) {
        total_score += score_oscillation_fit(candidate, oscillations) * 1.2;
    }
    
    // Component 4: Clock lattice fit
    total_score += score_clock_lattice_fit(candidate) * 0.8;
    
    return total_score;
}

/**
 * Score all candidates and sort by fitness
 */
void score_and_sort_candidates(
    SearchCandidate* candidates,
    uint32_t num_candidates,
    const StructuralMap* structure,
    const OscillationMap* oscillations
) {
    if (!candidates || num_candidates == 0) {
        return;
    }
    
    // Compute fitness scores
    for (uint32_t i = 0; i < num_candidates; i++) {
        candidates[i].sft_score = score_candidate(
            &candidates[i],
            structure,
            oscillations
        );
    }
    
    // Sort by fitness (descending)
    for (uint32_t i = 0; i < num_candidates - 1; i++) {
        for (uint32_t j = 0; j < num_candidates - i - 1; j++) {
            if (candidates[j].sft_score < candidates[j + 1].sft_score) {
                SearchCandidate temp = candidates[j];
                candidates[j] = candidates[j + 1];
                candidates[j + 1] = temp;
            }
        }
    }
}

/**
 * Select top N candidates
 */
SearchCandidate* select_top_candidates(
    const SearchCandidate* candidates,
    uint32_t num_candidates,
    uint32_t num_to_select
) {
    if (!candidates || num_candidates == 0 || num_to_select == 0) {
        return NULL;
    }
    
    uint32_t actual_select = (num_to_select < num_candidates) ? 
                             num_to_select : num_candidates;
    
    SearchCandidate* top = (SearchCandidate*)malloc(
        actual_select * sizeof(SearchCandidate)
    );
    
    if (!top) {
        return NULL;
    }
    
    // Copy top candidates (already sorted)
    memcpy(top, candidates, actual_select * sizeof(SearchCandidate));
    
    return top;
}

/**
 * Compute average fitness of candidates
 */
double compute_average_fitness(
    const SearchCandidate* candidates,
    uint32_t num_candidates
) {
    if (!candidates || num_candidates == 0) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (uint32_t i = 0; i < num_candidates; i++) {
        sum += candidates[i].sft_score;
    }
    
    return sum / num_candidates;
}

/**
 * Find best candidate
 */
const SearchCandidate* find_best_candidate(
    const SearchCandidate* candidates,
    uint32_t num_candidates
) {
    if (!candidates || num_candidates == 0) {
        return NULL;
    }
    
    const SearchCandidate* best = &candidates[0];
    double best_score = best->sft_score;
    
    for (uint32_t i = 1; i < num_candidates; i++) {
        if (candidates[i].sft_score > best_score) {
            best = &candidates[i];
            best_score = best->sft_score;
        }
    }
    
    return best;
}
