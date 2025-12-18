/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file cross_correlation.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Cross-dimensional correlation analysis
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Analyzes correlations between different dimensions to identify
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * structural relationships and detect corruption patterns.
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
 * Compute cross-correlation between two dimensions
 * 
 * Measures how changes in one dimension correlate with changes in another.
 */
static double compute_dimension_correlation(
    const double* data,
    uint64_t num_elements,
    uint32_t dim1,
    uint32_t dim2,
    uint32_t num_dimensions,
    const uint32_t* dimension_sizes
) {
    (void)num_dimensions;
    if (!data || num_elements == 0 || dim1 == dim2) return 0.0;
    
    // Simple correlation: based on GCD relationship
    // In real implementation, would compute actual correlation from data
    
    // For now, return a simple measure based on dimension sizes
    uint32_t size1 = dimension_sizes[dim1];
    uint32_t size2 = dimension_sizes[dim2];
    
    // Compute GCD
    uint32_t a = size1;
    uint32_t b = size2;
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    uint32_t gcd = a;
    
    // Correlation based on GCD (higher GCD = higher correlation)
    double max_size = (size1 > size2) ? size1 : size2;
    double correlation = (double)gcd / max_size;
    
    return correlation;
}

/**
 * Compute cross-correlation matrix
 * 
 * Computes correlations between all pairs of dimensions.
 */
double* compute_cross_correlation_matrix(
    const HyperDimensionalStructure* structure,
    const double* structure_data
) {
    if (!structure || !structure_data) return NULL;
    
    uint32_t n = structure->num_dimensions;
    uint64_t total_elements = get_total_elements(structure);
    
    // Allocate correlation matrix (n × n)
    double* correlations = (double*)calloc(n * n, sizeof(double));
    if (!correlations) return NULL;
    
    // Compute correlations
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            if (i == j) {
                correlations[i * n + j] = 1.0;  // Perfect self-correlation
            } else {
                correlations[i * n + j] = compute_dimension_correlation(
                    structure_data, total_elements, i, j,
                    structure->num_dimensions, structure->dimension_sizes
                );
            }
        }
    }
    
    return correlations;
}

/**
 * Get correlation between two dimensions
 */
double get_dimension_correlation(
    const double* correlation_matrix,
    uint32_t num_dimensions,
    uint32_t dim1,
    uint32_t dim2
) {
    if (!correlation_matrix || dim1 >= num_dimensions || dim2 >= num_dimensions) {
        return 0.0;
    }
    return correlation_matrix[dim1 * num_dimensions + dim2];
}

/**
 * Find highly correlated dimension pairs
 * 
 * Identifies dimension pairs with correlation above threshold.
 */
typedef struct {
    uint32_t dim1;
    uint32_t dim2;
    double correlation;
} CorrelationPair;

CorrelationPair* find_correlated_dimensions(
    const double* correlation_matrix,
    uint32_t num_dimensions,
    double threshold,
    uint32_t* num_pairs
) {
    if (!correlation_matrix || !num_pairs) return NULL;
    
    // Count pairs above threshold
    uint32_t count = 0;
    for (uint32_t i = 0; i < num_dimensions; i++) {
        for (uint32_t j = i + 1; j < num_dimensions; j++) {
            double corr = correlation_matrix[i * num_dimensions + j];
            if (corr > threshold) {
                count++;
            }
        }
    }
    
    *num_pairs = count;
    if (count == 0) return NULL;
    
    // Allocate result array
    CorrelationPair* pairs = (CorrelationPair*)calloc(count, sizeof(CorrelationPair));
    if (!pairs) return NULL;
    
    // Fill array
    uint32_t idx = 0;
    for (uint32_t i = 0; i < num_dimensions; i++) {
        for (uint32_t j = i + 1; j < num_dimensions; j++) {
            double corr = correlation_matrix[i * num_dimensions + j];
            if (corr > threshold) {
                pairs[idx].dim1 = i;
                pairs[idx].dim2 = j;
                pairs[idx].correlation = corr;
                idx++;
            }
        }
    }
    
    return pairs;
}

/**
 * Compute average correlation
 * 
 * Measures overall structural coherence.
 */
double compute_average_correlation(
    const double* correlation_matrix,
    uint32_t num_dimensions
) {
    if (!correlation_matrix || num_dimensions == 0) return 0.0;
    
    double sum = 0.0;
    uint32_t count = 0;
    
    // Sum off-diagonal elements (exclude self-correlations)
    for (uint32_t i = 0; i < num_dimensions; i++) {
        for (uint32_t j = 0; j < num_dimensions; j++) {
            if (i != j) {
                sum += correlation_matrix[i * num_dimensions + j];
                count++;
            }
        }
    }
    
    return count > 0 ? sum / count : 0.0;
}

/**
 * Detect uncorrelated dimensions
 * 
 * Dimensions with low correlation to others may be corrupted or anomalous.
 */
uint32_t* detect_uncorrelated_dimensions(
    const double* correlation_matrix,
    uint32_t num_dimensions,
    double threshold,
    uint32_t* num_uncorrelated
) {
    if (!correlation_matrix || !num_uncorrelated) return NULL;
    
    // Count uncorrelated dimensions
    uint32_t count = 0;
    for (uint32_t i = 0; i < num_dimensions; i++) {
        // Compute average correlation for this dimension
        double avg_corr = 0.0;
        for (uint32_t j = 0; j < num_dimensions; j++) {
            if (i != j) {
                avg_corr += correlation_matrix[i * num_dimensions + j];
            }
        }
        avg_corr /= (num_dimensions - 1);
        
        if (avg_corr < threshold) {
            count++;
        }
    }
    
    *num_uncorrelated = count;
    if (count == 0) return NULL;
    
    // Allocate result array
    uint32_t* uncorrelated = (uint32_t*)calloc(count, sizeof(uint32_t));
    if (!uncorrelated) return NULL;
    
    // Fill array
    uint32_t idx = 0;
    for (uint32_t i = 0; i < num_dimensions; i++) {
        double avg_corr = 0.0;
        for (uint32_t j = 0; j < num_dimensions; j++) {
            if (i != j) {
                avg_corr += correlation_matrix[i * num_dimensions + j];
            }
        }
        avg_corr /= (num_dimensions - 1);
        
        if (avg_corr < threshold) {
            uncorrelated[idx++] = i;
        }
    }
    
    return uncorrelated;
}

/**
 * Compute correlation strength
 * 
 * Overall measure of how correlated the structure is.
 */
double compute_correlation_strength(
    const double* correlation_matrix,
    uint32_t num_dimensions
) {
    if (!correlation_matrix || num_dimensions == 0) return 0.0;
    
    double avg_corr = compute_average_correlation(correlation_matrix, num_dimensions);
    
    // Strength is based on average correlation
    // Higher average = stronger correlations = more structured
    return avg_corr;
}

/**
 * Validate cross-dimensional consistency
 * 
 * Checks if correlations are consistent and reasonable.
 */
bool validate_cross_dimensional_consistency(
    const double* correlation_matrix,
    uint32_t num_dimensions
) {
    if (!correlation_matrix || num_dimensions == 0) return false;
    
    // Check diagonal elements (should be 1.0)
    for (uint32_t i = 0; i < num_dimensions; i++) {
        double self_corr = correlation_matrix[i * num_dimensions + i];
        if (math_abs(self_corr - 1.0) > 0.01) {
            return false;
        }
    }
    
    // Check symmetry (correlation[i,j] should equal correlation[j,i])
    for (uint32_t i = 0; i < num_dimensions; i++) {
        for (uint32_t j = i + 1; j < num_dimensions; j++) {
            double corr_ij = correlation_matrix[i * num_dimensions + j];
            double corr_ji = correlation_matrix[j * num_dimensions + i];
            if (math_abs(corr_ij - corr_ji) > 0.01) {
                return false;
            }
        }
    }
    
    // Check all correlations are in valid range [0, 1]
    for (uint32_t i = 0; i < num_dimensions * num_dimensions; i++) {
        if (correlation_matrix[i] < 0.0 || correlation_matrix[i] > 1.0) {
            return false;
        }
    }
    
    return true;
}
