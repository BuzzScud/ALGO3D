/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file multi_scalar_analysis.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Multi-scalar recursive analysis
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Analyzes structures at multiple scales simultaneously, enabling
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * detection of patterns and corruption across different resolution levels.
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
 * Create scalar analysis for a specific scale
 */
static ScalarAnalysis* create_scalar_analysis(
    double scalar_value,
    const HyperDimensionalStructure* structure
) {
    if (!structure || scalar_value <= 0.0) return NULL;
    
    ScalarAnalysis* analysis = (ScalarAnalysis*)calloc(1, sizeof(ScalarAnalysis));
    if (!analysis) return NULL;
    
    analysis->scalar_value = scalar_value;
    analysis->oscillations = NULL;  // Would be computed from structure
    
    // Allocate variance arrays
    analysis->variance_per_axis = (double*)calloc(
        structure->num_dimensions, sizeof(double)
    );
    
    uint64_t total_elements = get_total_elements(structure);
    analysis->variance_per_vertex = (double*)calloc(
        total_elements, sizeof(double)
    );
    
    if (!analysis->variance_per_axis || !analysis->variance_per_vertex) {
        free(analysis->variance_per_axis);
        free(analysis->variance_per_vertex);
        free(analysis);
        return NULL;
    }
    
    // Initialize variances (would be computed from actual data)
    for (uint32_t i = 0; i < structure->num_dimensions; i++) {
        analysis->variance_per_axis[i] = 0.0;
    }
    
    for (uint64_t i = 0; i < total_elements; i++) {
        analysis->variance_per_vertex[i] = 0.0;
    }
    
    // Check stability (simple heuristic: scalar close to 1.0 is stable)
    analysis->is_stable = (math_abs(scalar_value - 1.0) < 0.5);
    
    return analysis;
}

/**
 * Analyze structure at multiple scales
 */
MultiScalarAnalysis* analyze_multi_scalar(
    const HyperDimensionalStructure* structure,
    const double* scalar_values,
    uint32_t num_scalars
) {
    if (!structure || !scalar_values || num_scalars == 0) {
        return NULL;
    }
    
    MultiScalarAnalysis* analysis = (MultiScalarAnalysis*)calloc(
        1, sizeof(MultiScalarAnalysis)
    );
    if (!analysis) return NULL;
    
    analysis->num_scalars = num_scalars;
    
    // Allocate analyses array
    analysis->analyses = (ScalarAnalysis*)calloc(num_scalars, sizeof(ScalarAnalysis));
    if (!analysis->analyses) {
        free(analysis);
        return NULL;
    }
    
    // Create analysis for each scalar
    for (uint32_t i = 0; i < num_scalars; i++) {
        ScalarAnalysis* scalar_analysis = create_scalar_analysis(
            scalar_values[i], structure
        );
        
        if (scalar_analysis) {
            // Copy the analysis
            analysis->analyses[i] = *scalar_analysis;
            // Free the temporary structure (but not its contents)
            free(scalar_analysis);
        } else {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                free(analysis->analyses[j].variance_per_axis);
                free(analysis->analyses[j].variance_per_vertex);
            }
            free(analysis->analyses);
            free(analysis);
            return NULL;
        }
    }
    
    // Allocate cross-scalar correlations (num_scalars × num_scalars)
    analysis->cross_scalar_correlations = (double*)calloc(
        num_scalars * num_scalars, sizeof(double)
    );
    if (!analysis->cross_scalar_correlations) {
        for (uint32_t i = 0; i < num_scalars; i++) {
            free(analysis->analyses[i].variance_per_axis);
            free(analysis->analyses[i].variance_per_vertex);
        }
        free(analysis->analyses);
        free(analysis);
        return NULL;
    }
    
    // Compute cross-scalar correlations
    for (uint32_t i = 0; i < num_scalars; i++) {
        for (uint32_t j = 0; j < num_scalars; j++) {
            // Simple correlation: based on scalar value similarity
            double diff = math_abs(scalar_values[i] - scalar_values[j]);
            analysis->cross_scalar_correlations[i * num_scalars + j] = 
                1.0 / (1.0 + diff);
        }
    }
    
    return analysis;
}

/**
 * Get scalar analysis at specific index
 */
const ScalarAnalysis* get_scalar_analysis(
    const MultiScalarAnalysis* analysis,
    uint32_t scalar_index
) {
    if (!analysis || scalar_index >= analysis->num_scalars) {
        return NULL;
    }
    return &analysis->analyses[scalar_index];
}

/**
 * Get cross-scalar correlation
 */
double get_cross_scalar_correlation(
    const MultiScalarAnalysis* analysis,
    uint32_t scalar1,
    uint32_t scalar2
) {
    if (!analysis || scalar1 >= analysis->num_scalars || 
        scalar2 >= analysis->num_scalars) {
        return 0.0;
    }
    return analysis->cross_scalar_correlations[scalar1 * analysis->num_scalars + scalar2];
}

/**
 * Check if all scalars are stable
 */
bool are_all_scalars_stable(const MultiScalarAnalysis* analysis) {
    if (!analysis) return false;
    
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        if (!analysis->analyses[i].is_stable) {
            return false;
        }
    }
    return true;
}

/**
 * Compute average variance across all scalars
 */
double compute_average_variance(
    const MultiScalarAnalysis* analysis,
    uint32_t dimension
) {
    if (!analysis || dimension >= analysis->analyses[0].variance_per_axis[0]) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        sum += analysis->analyses[i].variance_per_axis[dimension];
    }
    
    return sum / analysis->num_scalars;
}

/**
 * Find most stable scalar
 */
uint32_t find_most_stable_scalar(const MultiScalarAnalysis* analysis) {
    if (!analysis || analysis->num_scalars == 0) return 0;
    
    uint32_t most_stable = 0;
    double best_stability = 0.0;
    
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        // Stability score: closer to 1.0 is better
        double stability = 1.0 / (1.0 + math_abs(analysis->analyses[i].scalar_value - 1.0));
        if (stability > best_stability) {
            best_stability = stability;
            most_stable = i;
        }
    }
    
    return most_stable;
}

/**
 * Compute multi-scalar consistency score
 * 
 * Measures how consistent the structure is across different scales.
 * Higher score = more consistent = better recovery potential.
 */
double compute_multi_scalar_consistency(const MultiScalarAnalysis* analysis) {
    if (!analysis || analysis->num_scalars < 2) return 0.0;
    
    double consistency = 0.0;
    int comparisons = 0;
    
    // Compare adjacent scalars
    for (uint32_t i = 0; i < analysis->num_scalars - 1; i++) {
        double correlation = get_cross_scalar_correlation(analysis, i, i + 1);
        consistency += correlation;
        comparisons++;
    }
    
    return comparisons > 0 ? consistency / comparisons : 0.0;
}

/**
 * Free multi-scalar analysis
 */
void free_multi_scalar_analysis(MultiScalarAnalysis* analysis) {
    if (!analysis) return;
    
    if (analysis->analyses) {
        for (uint32_t i = 0; i < analysis->num_scalars; i++) {
            if (analysis->analyses[i].variance_per_axis) {
                free(analysis->analyses[i].variance_per_axis);
            }
            if (analysis->analyses[i].variance_per_vertex) {
                free(analysis->analyses[i].variance_per_vertex);
            }
            if (analysis->analyses[i].oscillations) {
                // Would free oscillation map if allocated
            }
        }
        free(analysis->analyses);
    }
    
    if (analysis->cross_scalar_correlations) {
        free(analysis->cross_scalar_correlations);
    }
    
    free(analysis);
}
