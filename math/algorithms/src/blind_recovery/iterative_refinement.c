/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file iterative_refinement.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Iterative structure refinement using candidate search
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Refines corrupted structures through iterative candidate evaluation
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * and application of best-fit values.
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
 * Apply candidate to structure
 * 
 * Updates structure with candidate value if it improves consistency
 */
static bool apply_candidate_to_structure(
    StructuralMap* structure,
    const SearchCandidate* candidate
) {
    if (!structure || !candidate) {
        return false;
    }
    
    // Determine which dimension to update
    uint32_t* target = NULL;
    if (candidate->dimension == 0) {
        target = &structure->num_vertices;
    } else if (candidate->dimension == 1) {
        target = &structure->num_edges;
    } else if (candidate->dimension == 2) {
        target = &structure->num_faces;
    } else {
        return false;
    }
    
    // Store old value
    uint32_t old_value = *target;
    
    // Apply candidate
    *target = (uint32_t)candidate->candidate;
    
    // Check if Euler's formula is satisfied
    int euler_result = (int)structure->num_vertices - 
                      (int)structure->num_edges + 
                      (int)structure->num_faces;
    
    // If Euler's formula is worse, revert
    int old_euler = (int)old_value;
    if (candidate->dimension == 0) {
        old_euler = old_value - (int)structure->num_edges + (int)structure->num_faces;
    } else if (candidate->dimension == 1) {
        old_euler = (int)structure->num_vertices - old_value + (int)structure->num_faces;
    } else if (candidate->dimension == 2) {
        old_euler = (int)structure->num_vertices - (int)structure->num_edges + old_value;
    }
    
    int old_delta = abs(old_euler - 2);
    int new_delta = abs(euler_result - 2);
    
    if (new_delta > old_delta) {
        // Revert - made things worse
        *target = old_value;
        return false;
    }
    
    // Update corruption percentage
    structure->corruption_percentage = (double)new_delta / 
                                      (structure->num_vertices + 
                                       structure->num_edges + 
                                       structure->num_faces);
    
    return true;
}

/**
 * Refine structure using iterative search
 */
bool refine_structure(
    StructuralMap* structure,
    IterativeSearch* search,
    double convergence_threshold
) {
    if (!structure || !search) {
        return false;
    }
    
    bool converged = false;
    uint32_t iteration = 0;
    double prev_corruption = structure->corruption_percentage;
    
    while (iteration < search->max_iterations && !converged) {
        // Try to apply best candidates
        uint32_t improvements = 0;
        
        for (uint32_t i = 0; i < search->num_candidates; i++) {
            if (apply_candidate_to_structure(structure, &search->candidates[i])) {
                improvements++;
            }
        }
        
        // Check convergence
        double corruption_change = math_abs(structure->corruption_percentage - prev_corruption);
        if (corruption_change < convergence_threshold) {
            converged = true;
        }
        
        prev_corruption = structure->corruption_percentage;
        iteration++;
        
        // If no improvements, stop
        if (improvements == 0) {
            break;
        }
    }
    
    return converged;
}

/**
 * Create iterative search state
 */
IterativeSearch* create_iterative_search(
    uint32_t max_iterations,
    double convergence_threshold,
    uint64_t nonce
) {
    IterativeSearch* search = (IterativeSearch*)calloc(1, sizeof(IterativeSearch));
    if (!search) {
        return NULL;
    }
    
    search->max_iterations = max_iterations;
    search->convergence_threshold = convergence_threshold;
    search->nonce = nonce;
    search->candidates = NULL;
    search->num_candidates = 0;
    
    return search;
}

/**
 * Initialize search with candidates
 */
bool initialize_search_candidates(
    IterativeSearch* search,
    SearchCandidate* candidates,
    uint32_t num_candidates
) {
    if (!search || !candidates) {
        return false;
    }
    
    // Free existing candidates if any
    if (search->candidates) {
        free(search->candidates);
    }
    
    // Allocate and copy candidates
    search->candidates = (SearchCandidate*)malloc(
        num_candidates * sizeof(SearchCandidate)
    );
    
    if (!search->candidates) {
        return false;
    }
    
    memcpy(search->candidates, candidates, 
           num_candidates * sizeof(SearchCandidate));
    search->num_candidates = num_candidates;
    
    return true;
}

/**
 * Perform multi-iteration refinement
 */
bool refine_structure_multi_pass(
    StructuralMap* structure,
    const OscillationMap* oscillations,
    uint32_t num_passes,
    uint32_t candidates_per_pass,
    double convergence_threshold
) {
    if (!structure) {
        return false;
    }
    
    bool overall_converged = false;
    
    for (uint32_t pass = 0; pass < num_passes; pass++) {
        // Generate nonce for this pass
        uint64_t nonce = 0x9e3779b97f4a7c15ULL * (pass + 1);
        
        // Generate candidates
        SearchCandidate* candidates = generate_candidates(
            nonce,
            pass % 3,  // Cycle through dimensions
            candidates_per_pass,
            structure
        );
        
        if (!candidates) {
            continue;
        }
        
        // Score candidates
        score_and_sort_candidates(
            candidates,
            candidates_per_pass,
            structure,
            oscillations
        );
        
        // Create search state
        IterativeSearch* search = create_iterative_search(
            10,  // max iterations per pass
            convergence_threshold,
            nonce
        );
        
        if (search) {
            initialize_search_candidates(search, candidates, candidates_per_pass);
            
            // Refine
            bool pass_converged = refine_structure(
                structure,
                search,
                convergence_threshold
            );
            
            if (pass_converged) {
                overall_converged = true;
            }
            
            free_iterative_search(search);
        }
        
        free(candidates);
        
        // If converged, stop early
        if (overall_converged) {
            break;
        }
    }
    
    return overall_converged;
}

/**
 * Free iterative search
 */
void free_iterative_search(IterativeSearch* search) {
    if (!search) return;
    
    if (search->candidates) {
        free(search->candidates);
    }
    
    free(search);
}
