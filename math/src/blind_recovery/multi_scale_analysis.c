/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file multi_scale_analysis.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Multi-scale structure analysis for recursive stabilization
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Analyzes structures at multiple scales (coarse to fine) to detect
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * oscillations and instabilities at different resolution levels.
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
 * Compute resolution for scale level
 * 
 * Scale 0 = finest (original resolution)
 * Scale N = coarsest (most downsampled)
 */
static double compute_scale_resolution(uint32_t scale_level) {
    // Each scale level doubles the resolution
    return math_pow(2.0, (double)scale_level);
}

/**
 * Downsample structure to coarser scale
 */
static StructuralMap* downsample_structure(
    const StructuralMap* structure,
    double scale_factor
) {
    if (!structure || scale_factor < 1.0) {
        return NULL;
    }
    
    StructuralMap* downsampled = (StructuralMap*)calloc(1, sizeof(StructuralMap));
    if (!downsampled) {
        return NULL;
    }
    
    // Downsample dimensions
    downsampled->num_vertices = (uint32_t)(structure->num_vertices / scale_factor);
    downsampled->num_edges = (uint32_t)(structure->num_edges / scale_factor);
    downsampled->num_faces = (uint32_t)(structure->num_faces / scale_factor);
    
    // Ensure at least 1 of each
    if (downsampled->num_vertices < 1) downsampled->num_vertices = 1;
    if (downsampled->num_edges < 1) downsampled->num_edges = 1;
    if (downsampled->num_faces < 1) downsampled->num_faces = 1;
    
    // Allocate arrays
    downsampled->coprime_matrix = NULL;  // Will be computed if needed
    downsampled->dimensional_offsets = NULL;
    downsampled->corruption_mask = (bool*)calloc(
        downsampled->num_vertices,
        sizeof(bool)
    );
    
    // Scale corruption percentage
    downsampled->corruption_percentage = structure->corruption_percentage;
    
    return downsampled;
}

/**
 * Create scale level
 */
static ScaleLevel* create_scale_level(
    uint32_t scale_level,
    const StructuralMap* structure
) __attribute__((unused));

static ScaleLevel* create_scale_level(
    uint32_t scale_level,
    const StructuralMap* structure
) {
    (void)structure;  // Unused in this implementation
    ScaleLevel* level = (ScaleLevel*)calloc(1, sizeof(ScaleLevel));
    if (!level) {
        return NULL;
    }
    
    level->scale_level = scale_level;
    level->resolution = compute_scale_resolution(scale_level);
    level->oscillations = NULL;  // Will be computed later
    level->is_stable = false;
    
    return level;
}

/**
 * Free scale level contents (not the structure itself)
 */
static void free_scale_level_contents(ScaleLevel* level) {
    if (!level) return;
    
    if (level->oscillations) {
        // Free oscillation map
        if (level->oscillations->signatures) {
            free(level->oscillations->signatures);
        }
        if (level->oscillations->cross_correlations) {
            free(level->oscillations->cross_correlations);
        }
        free(level->oscillations);
        level->oscillations = NULL;
    }
}

/**
 * Analyze structure at multiple scales
 */
MultiScaleAnalysis* analyze_multi_scale(
    const StructuralMap* structure,
    uint32_t num_scales
) {
    if (!structure || num_scales == 0) {
        return NULL;
    }
    
    MultiScaleAnalysis* analysis = (MultiScaleAnalysis*)calloc(
        1, sizeof(MultiScaleAnalysis)
    );
    
    if (!analysis) {
        return NULL;
    }
    
    analysis->num_scales = num_scales;
    analysis->scales = (ScaleLevel*)calloc(num_scales, sizeof(ScaleLevel));
    
    if (!analysis->scales) {
        free(analysis);
        return NULL;
    }
    
    analysis->current_scale = 0;
    analysis->all_scales_stable = false;
    
    // Create scale levels from fine to coarse
    for (uint32_t i = 0; i < num_scales; i++) {
        // Initialize scale level directly
        analysis->scales[i].scale_level = i;
        analysis->scales[i].resolution = compute_scale_resolution(i);
        analysis->scales[i].oscillations = NULL;
        analysis->scales[i].is_stable = false;
        
        // Downsample structure for this scale
        double scale_factor = compute_scale_resolution(i);
        StructuralMap* downsampled = downsample_structure(structure, scale_factor);
        
        if (downsampled) {
            // Analyze oscillations at this scale
            // For now, create empty oscillation map
            analysis->scales[i].oscillations = (OscillationMap*)calloc(
                1, sizeof(OscillationMap)
            );
            
            if (analysis->scales[i].oscillations) {
                analysis->scales[i].oscillations->num_dimensions = 3;
                analysis->scales[i].oscillations->signatures = (OscillationSignature*)calloc(
                    3, sizeof(OscillationSignature)
                );
                analysis->scales[i].oscillations->is_converging = false;
                analysis->scales[i].oscillations->iterations_to_convergence = 0;
            }
            
            // Check stability at this scale
            // A scale is stable if corruption is low
            analysis->scales[i].is_stable = 
                (downsampled->corruption_percentage < 0.1);
            
            free(downsampled->corruption_mask);
            free(downsampled);
        }
    }
    
    // Check if all scales are stable
    bool all_stable = true;
    for (uint32_t i = 0; i < num_scales; i++) {
        if (!analysis->scales[i].is_stable) {
            all_stable = false;
            break;
        }
    }
    analysis->all_scales_stable = all_stable;
    
    return analysis;
}

/**
 * Get scale level by index
 */
const ScaleLevel* get_scale_level(
    const MultiScaleAnalysis* analysis,
    uint32_t scale_index
) {
    if (!analysis || scale_index >= analysis->num_scales) {
        return NULL;
    }
    
    return &analysis->scales[scale_index];
}

/**
 * Check if scale is stable
 */
bool is_scale_stable(
    const MultiScaleAnalysis* analysis,
    uint32_t scale_index
) {
    const ScaleLevel* level = get_scale_level(analysis, scale_index);
    if (!level) {
        return false;
    }
    
    return level->is_stable;
}

/**
 * Get coarsest stable scale
 */
uint32_t get_coarsest_stable_scale(const MultiScaleAnalysis* analysis) {
    if (!analysis) {
        return 0;
    }
    
    // Search from coarsest to finest
    for (int i = (int)analysis->num_scales - 1; i >= 0; i--) {
        if (analysis->scales[i].is_stable) {
            return (uint32_t)i;
        }
    }
    
    return 0;  // Return finest scale if none stable
}

/**
 * Get finest unstable scale
 */
uint32_t get_finest_unstable_scale(const MultiScaleAnalysis* analysis) {
    if (!analysis) {
        return 0;
    }
    
    // Search from finest to coarsest
    for (uint32_t i = 0; i < analysis->num_scales; i++) {
        if (!analysis->scales[i].is_stable) {
            return i;
        }
    }
    
    return analysis->num_scales - 1;  // Return coarsest if all stable
}

/**
 * Update scale stability
 */
void update_scale_stability(
    MultiScaleAnalysis* analysis,
    uint32_t scale_index,
    bool is_stable
) {
    if (!analysis || scale_index >= analysis->num_scales) {
        return;
    }
    
    analysis->scales[scale_index].is_stable = is_stable;
    
    // Update overall stability
    bool all_stable = true;
    for (uint32_t i = 0; i < analysis->num_scales; i++) {
        if (!analysis->scales[i].is_stable) {
            all_stable = false;
            break;
        }
    }
    analysis->all_scales_stable = all_stable;
}

/**
 * Free multi-scale analysis
 */
void free_multi_scale_analysis(MultiScaleAnalysis* analysis) {
    if (!analysis) return;
    
    if (analysis->scales) {
        for (uint32_t i = 0; i < analysis->num_scales; i++) {
            free_scale_level_contents(&analysis->scales[i]);
        }
        free(analysis->scales);
    }
    
    free(analysis);
}

/**
 * Compute scale statistics
 */
typedef struct {
    uint32_t num_stable_scales;
    uint32_t num_unstable_scales;
    double avg_resolution;
    double stability_ratio;
} ScaleStatistics;

ScaleStatistics compute_scale_statistics(const MultiScaleAnalysis* analysis) {
    ScaleStatistics stats = {0};
    
    if (!analysis) {
        return stats;
    }
    
    double total_resolution = 0.0;
    
    for (uint32_t i = 0; i < analysis->num_scales; i++) {
        if (analysis->scales[i].is_stable) {
            stats.num_stable_scales++;
        } else {
            stats.num_unstable_scales++;
        }
        total_resolution += analysis->scales[i].resolution;
    }
    
    stats.avg_resolution = total_resolution / analysis->num_scales;
    stats.stability_ratio = (double)stats.num_stable_scales / analysis->num_scales;
    
    return stats;
}
