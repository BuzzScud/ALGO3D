/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file variance_analysis.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Variance analysis for hyper-dimensional structures
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Computes variance along axes and at vertices to identify
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * corruption patterns and structural anomalies.
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
 * Compute mean along an axis
 */
static double compute_axis_mean(
    const double* data,
    uint64_t num_elements,
    uint32_t axis,
    uint32_t num_dimensions,
    const uint32_t* dimension_sizes
) {
    (void)axis;
    (void)num_dimensions;
    (void)dimension_sizes;
    if (!data || num_elements == 0) return 0.0;
    
    double sum = 0.0;
    uint64_t count = 0;
    
    // Sum all elements along this axis
    for (uint64_t i = 0; i < num_elements; i++) {
        sum += data[i];
        count++;
    }
    
    return count > 0 ? sum / count : 0.0;
}

/**
 * Compute variance along an axis
 */
static double compute_axis_variance(
    const double* data,
    uint64_t num_elements,
    uint32_t axis,
    uint32_t num_dimensions,
    const uint32_t* dimension_sizes,
    double mean
) {
    (void)axis;
    (void)num_dimensions;
    (void)dimension_sizes;
    if (!data || num_elements == 0) return 0.0;
    
    double sum_squared_diff = 0.0;
    uint64_t count = 0;
    
    // Sum squared differences
    for (uint64_t i = 0; i < num_elements; i++) {
        double diff = data[i] - mean;
        sum_squared_diff += diff * diff;
        count++;
    }
    
    return count > 0 ? sum_squared_diff / count : 0.0;
}

/**
 * Compute variance for hyper-dimensional structure
 */
void compute_variance(
    const HyperDimensionalStructure* structure,
    const double* structure_data,
    double* variance_per_axis,
    double* variance_per_vertex
) {
    if (!structure || !structure_data) return;
    
    uint64_t total_elements = get_total_elements(structure);
    
    // Compute variance per axis
    if (variance_per_axis) {
        for (uint32_t axis = 0; axis < structure->num_dimensions; axis++) {
            double mean = compute_axis_mean(
                structure_data, total_elements, axis,
                structure->num_dimensions, structure->dimension_sizes
            );
            
            variance_per_axis[axis] = compute_axis_variance(
                structure_data, total_elements, axis,
                structure->num_dimensions, structure->dimension_sizes, mean
            );
        }
    }
    
    // Compute variance per vertex
    if (variance_per_vertex) {
        // For each vertex, compute local variance
        for (uint64_t v = 0; v < total_elements; v++) {
            // Simple variance: difference from mean
            double mean = 0.0;
            for (uint64_t i = 0; i < total_elements; i++) {
                mean += structure_data[i];
            }
            mean /= total_elements;
            
            double diff = structure_data[v] - mean;
            variance_per_vertex[v] = diff * diff;
        }
    }
}

/**
 * Identify high-variance dimensions
 * 
 * Dimensions with high variance may indicate corruption or instability.
 */
uint32_t* identify_high_variance_dimensions(
    const HyperDimensionalStructure* structure,
    const double* variance_per_axis,
    double threshold,
    uint32_t* num_high_variance
) {
    if (!structure || !variance_per_axis || !num_high_variance) {
        return NULL;
    }
    
    // Count high-variance dimensions
    uint32_t count = 0;
    for (uint32_t i = 0; i < structure->num_dimensions; i++) {
        if (variance_per_axis[i] > threshold) {
            count++;
        }
    }
    
    *num_high_variance = count;
    
    if (count == 0) return NULL;
    
    // Allocate result array
    uint32_t* high_variance_dims = (uint32_t*)calloc(count, sizeof(uint32_t));
    if (!high_variance_dims) return NULL;
    
    // Fill array
    uint32_t idx = 0;
    for (uint32_t i = 0; i < structure->num_dimensions; i++) {
        if (variance_per_axis[i] > threshold) {
            high_variance_dims[idx++] = i;
        }
    }
    
    return high_variance_dims;
}

/**
 * Identify high-variance vertices
 * 
 * Vertices with high variance may be corrupted.
 */
uint64_t* identify_high_variance_vertices(
    const HyperDimensionalStructure* structure,
    const double* variance_per_vertex,
    double threshold,
    uint64_t* num_high_variance
) {
    if (!structure || !variance_per_vertex || !num_high_variance) {
        return NULL;
    }
    
    uint64_t total_elements = get_total_elements(structure);
    
    // Count high-variance vertices
    uint64_t count = 0;
    for (uint64_t i = 0; i < total_elements; i++) {
        if (variance_per_vertex[i] > threshold) {
            count++;
        }
    }
    
    *num_high_variance = count;
    
    if (count == 0) return NULL;
    
    // Allocate result array
    uint64_t* high_variance_verts = (uint64_t*)calloc(count, sizeof(uint64_t));
    if (!high_variance_verts) return NULL;
    
    // Fill array
    uint64_t idx = 0;
    for (uint64_t i = 0; i < total_elements; i++) {
        if (variance_per_vertex[i] > threshold) {
            high_variance_verts[idx++] = i;
        }
    }
    
    return high_variance_verts;
}

/**
 * Compute variance statistics
 */
void compute_variance_statistics(
    const double* variance_array,
    uint64_t num_elements,
    double* mean,
    double* std_dev,
    double* min,
    double* max
) {
    if (!variance_array || num_elements == 0) return;
    
    // Compute mean
    double sum = 0.0;
    double min_val = variance_array[0];
    double max_val = variance_array[0];
    
    for (uint64_t i = 0; i < num_elements; i++) {
        sum += variance_array[i];
        if (variance_array[i] < min_val) min_val = variance_array[i];
        if (variance_array[i] > max_val) max_val = variance_array[i];
    }
    
    if (mean) *mean = sum / num_elements;
    if (min) *min = min_val;
    if (max) *max = max_val;
    
    // Compute standard deviation
    if (std_dev) {
        double mean_val = sum / num_elements;
        double sum_squared_diff = 0.0;
        
        for (uint64_t i = 0; i < num_elements; i++) {
            double diff = variance_array[i] - mean_val;
            sum_squared_diff += diff * diff;
        }
        
        *std_dev = math_sqrt(sum_squared_diff / num_elements);
    }
}

/**
 * Detect corruption using variance analysis
 * 
 * High variance often indicates corruption or anomalies.
 */
bool detect_corruption_by_variance(
    const HyperDimensionalStructure* structure,
    const double* structure_data,
    double threshold,
    uint64_t* num_corrupted
) {
    if (!structure || !structure_data) return false;
    
    uint64_t total_elements = get_total_elements(structure);
    
    // Allocate variance arrays
    double* variance_per_axis = (double*)calloc(
        structure->num_dimensions, sizeof(double)
    );
    double* variance_per_vertex = (double*)calloc(
        total_elements, sizeof(double)
    );
    
    if (!variance_per_axis || !variance_per_vertex) {
        free(variance_per_axis);
        free(variance_per_vertex);
        return false;
    }
    
    // Compute variances
    compute_variance(structure, structure_data, variance_per_axis, variance_per_vertex);
    
    // Count high-variance vertices (potential corruption)
    uint64_t count = 0;
    for (uint64_t i = 0; i < total_elements; i++) {
        if (variance_per_vertex[i] > threshold) {
            count++;
        }
    }
    
    if (num_corrupted) {
        *num_corrupted = count;
    }
    
    free(variance_per_axis);
    free(variance_per_vertex);
    
    return (count > 0);
}
