/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file recursive_stabilization.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Recursive stabilization across multiple scales
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Stabilizes structures recursively from coarse to fine scales,
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * ensuring stability propagates through all resolution levels.
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
 * Stabilize structure at single scale
 */
bool stabilize_at_scale(
    StructuralMap* structure,
    ScaleLevel* scale,
    uint32_t max_iterations
) {
    if (!structure || !scale) {
        return false;
    }
    
    // Use iterative refinement at this scale
    bool converged = refine_structure_multi_pass(
        structure,
        scale->oscillations,
        max_iterations,
        10,  // candidates per pass
        0.01 // convergence threshold
    );
    
    // Update scale stability
    scale->is_stable = converged && (structure->corruption_percentage < 0.1);
    
    return scale->is_stable;
}

/**
 * Propagate stability from coarse to fine scale
 */
static bool propagate_stability(
    StructuralMap* structure,
    const ScaleLevel* coarse_scale,
    ScaleLevel* fine_scale
) {
    if (!structure || !coarse_scale || !fine_scale) {
        return false;
    }
    
    // If coarse scale is stable, fine scale should be easier to stabilize
    if (coarse_scale->is_stable) {
        // Use coarse scale as initial guess for fine scale
        // This helps convergence at finer resolutions
        
        // Mark fine scale as potentially stable
        fine_scale->is_stable = true;
        return true;
    }
    
    return false;
}

/**
 * Recursive stabilization (main algorithm)
 */
bool stabilize_recursive(
    StructuralMap* structure,
    MultiScaleAnalysis* analysis,
    uint32_t max_depth
) {
    if (!structure || !analysis) {
        return false;
    }
    
    // Base case: if all scales stable, done
    if (analysis->all_scales_stable) {
        return true;
    }
    
    // Base case: max depth reached
    if (max_depth == 0) {
        return false;
    }
    
    // Find coarsest unstable scale
    uint32_t coarsest_unstable = 0;
    for (int i = (int)analysis->num_scales - 1; i >= 0; i--) {
        if (!analysis->scales[i].is_stable) {
            coarsest_unstable = (uint32_t)i;
            break;
        }
    }
    
    // Stabilize from coarse to fine
    for (uint32_t i = coarsest_unstable; i > 0; i--) {
        ScaleLevel* current_scale = &analysis->scales[i];
        ScaleLevel* finer_scale = &analysis->scales[i - 1];
        
        // Stabilize current scale
        if (!current_scale->is_stable) {
            bool stabilized = stabilize_at_scale(
                structure,
                current_scale,
                10  // max iterations per scale
            );
            
            if (!stabilized) {
                // If can't stabilize at this scale, try recursion
                return stabilize_recursive(structure, analysis, max_depth - 1);
            }
        }
        
        // Propagate stability to finer scale
        propagate_stability(structure, current_scale, finer_scale);
    }
    
    // Stabilize finest scale
    ScaleLevel* finest = &analysis->scales[0];
    if (!finest->is_stable) {
        stabilize_at_scale(structure, finest, 10);
    }
    
    // Update overall stability
    bool all_stable = true;
    for (uint32_t i = 0; i < analysis->num_scales; i++) {
        if (!analysis->scales[i].is_stable) {
            all_stable = false;
            break;
        }
    }
    analysis->all_scales_stable = all_stable;
    
    return all_stable;
}

/**
 * Iterative stabilization (non-recursive version)
 */
bool stabilize_iterative(
    StructuralMap* structure,
    MultiScaleAnalysis* analysis,
    uint32_t max_iterations
) {
    if (!structure || !analysis) {
        return false;
    }
    
    for (uint32_t iter = 0; iter < max_iterations; iter++) {
        bool made_progress = false;
        
        // Try to stabilize each unstable scale
        for (uint32_t i = 0; i < analysis->num_scales; i++) {
            if (!analysis->scales[i].is_stable) {
                bool stabilized = stabilize_at_scale(
                    structure,
                    &analysis->scales[i],
                    5  // iterations per scale
                );
                
                if (stabilized) {
                    made_progress = true;
                }
            }
        }
        
        // Check if all scales stable
        bool all_stable = true;
        for (uint32_t i = 0; i < analysis->num_scales; i++) {
            if (!analysis->scales[i].is_stable) {
                all_stable = false;
                break;
            }
        }
        
        if (all_stable) {
            analysis->all_scales_stable = true;
            return true;
        }
        
        // If no progress, stop
        if (!made_progress) {
            break;
        }
    }
    
    return analysis->all_scales_stable;
}

/**
 * Adaptive stabilization (chooses best strategy)
 */
bool stabilize_adaptive(
    StructuralMap* structure,
    MultiScaleAnalysis* analysis
) {
    if (!structure || !analysis) {
        return false;
    }
    
    // Count unstable scales
    uint32_t num_unstable = 0;
    for (uint32_t i = 0; i < analysis->num_scales; i++) {
        if (!analysis->scales[i].is_stable) {
            num_unstable++;
        }
    }
    
    // Choose strategy based on number of unstable scales
    if (num_unstable <= 2) {
        // Few unstable scales: use iterative approach
        return stabilize_iterative(structure, analysis, 10);
    } else {
        // Many unstable scales: use recursive approach
        return stabilize_recursive(structure, analysis, 5);
    }
}

/**
 * Stabilize with convergence tracking
 */
typedef struct {
    uint32_t iteration;
    uint32_t num_stable_scales;
    double corruption_percentage;
    bool converged;
} StabilizationProgress;

bool stabilize_with_tracking(
    StructuralMap* structure,
    MultiScaleAnalysis* analysis,
    StabilizationProgress* progress,
    uint32_t max_iterations
) {
    if (!structure || !analysis || !progress) {
        return false;
    }
    
    memset(progress, 0, sizeof(StabilizationProgress));
    
    for (uint32_t iter = 0; iter < max_iterations; iter++) {
        progress->iteration = iter;
        
        // Stabilize one iteration
        bool made_progress = false;
        for (uint32_t i = 0; i < analysis->num_scales; i++) {
            if (!analysis->scales[i].is_stable) {
                bool stabilized = stabilize_at_scale(
                    structure,
                    &analysis->scales[i],
                    1  // one iteration per scale
                );
                
                if (stabilized) {
                    made_progress = true;
                }
            }
        }
        
        // Update progress
        progress->num_stable_scales = 0;
        for (uint32_t i = 0; i < analysis->num_scales; i++) {
            if (analysis->scales[i].is_stable) {
                progress->num_stable_scales++;
            }
        }
        progress->corruption_percentage = structure->corruption_percentage;
        
        // Check convergence
        if (progress->num_stable_scales == analysis->num_scales) {
            progress->converged = true;
            analysis->all_scales_stable = true;
            return true;
        }
        
        // If no progress, stop
        if (!made_progress) {
            break;
        }
    }
    
    progress->converged = false;
    return false;
}

/**
 * Compute stabilization quality
 */
double compute_stabilization_quality(const MultiScaleAnalysis* analysis) {
    if (!analysis) {
        return 0.0;
    }
    
    double quality = 0.0;
    
    // Factor 1: Ratio of stable scales
    uint32_t num_stable = 0;
    for (uint32_t i = 0; i < analysis->num_scales; i++) {
        if (analysis->scales[i].is_stable) {
            num_stable++;
        }
    }
    double stability_ratio = (double)num_stable / analysis->num_scales;
    quality += stability_ratio * 0.5;
    
    // Factor 2: All scales stable bonus
    if (analysis->all_scales_stable) {
        quality += 0.3;
    }
    
    // Factor 3: Coarse scales stable (more important)
    uint32_t coarse_stable = 0;
    uint32_t num_coarse = analysis->num_scales / 2;
    for (uint32_t i = num_coarse; i < analysis->num_scales; i++) {
        if (analysis->scales[i].is_stable) {
            coarse_stable++;
        }
    }
    if (num_coarse > 0) {
        quality += ((double)coarse_stable / num_coarse) * 0.2;
    }
    
    return quality;
}
