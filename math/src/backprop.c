/**
 * @file backprop.c
 * @brief Implementation of backpropagation algorithms
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_sqrt with math_sqrt (2 calls)
 * - Replaced math_abs with math_abs (2 calls)
 * Total: 4 function calls migrated to NEW math library
 */

#include "backprop.h"
#include "numerical.h"
#include "math/transcendental.h"  // PHASE 1: NEW math library
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include "math/types.h"            // PHASE 2: For MATH_INFINITY
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Gradient Buffer Management
 * ============================================================================ */

GradientBuffer* gradient_buffer_create(
    size_t size,
    GradientAccumulationStrategy strategy
) {
    if (size == 0) return NULL;
    
    GradientBuffer* buffer = (GradientBuffer*)calloc(1, sizeof(GradientBuffer));
    if (!buffer) return NULL;
    
    buffer->gradients = (double*)calloc(size, sizeof(double));
    if (!buffer->gradients) {
        free(buffer);
        return NULL;
    }
    
    buffer->size = size;
    buffer->accumulation_count = 0;
    buffer->strategy = strategy;
    buffer->weights = NULL;
    
    return buffer;
}

void gradient_buffer_free(GradientBuffer* buffer) {
    if (!buffer) return;
    
    if (buffer->gradients) free(buffer->gradients);
    if (buffer->weights) free(buffer->weights);
    
    free(buffer);
}

void gradient_buffer_reset(GradientBuffer* buffer) {
    if (!buffer) return;
    
    memset(buffer->gradients, 0, buffer->size * sizeof(double));
    buffer->accumulation_count = 0;
}

bool gradient_buffer_set_weights(
    GradientBuffer* buffer,
    const double* weights,
    size_t size
) {
    if (!buffer || !weights || size != buffer->size) return false;
    
    if (!buffer->weights) {
        buffer->weights = (double*)malloc(size * sizeof(double));
        if (!buffer->weights) return false;
    }
    
    memcpy(buffer->weights, weights, size * sizeof(double));
    return true;
}

/* ============================================================================
 * Gradient Accumulation
 * ============================================================================ */

bool gradient_buffer_accumulate(
    GradientBuffer* buffer,
    const double* gradients,
    size_t size
) {
    if (!buffer || !gradients || size != buffer->size) return false;
    
    for (size_t i = 0; i < size; i++) {
        buffer->gradients[i] += gradients[i];
    }
    
    buffer->accumulation_count++;
    return true;
}

bool gradient_buffer_accumulate_weighted(
    GradientBuffer* buffer,
    const double* gradients,
    double weight,
    size_t size
) {
    if (!buffer || !gradients || size != buffer->size) return false;
    
    for (size_t i = 0; i < size; i++) {
        buffer->gradients[i] += weight * gradients[i];
    }
    
    buffer->accumulation_count++;
    return true;
}

bool gradient_buffer_finalize(GradientBuffer* buffer) {
    if (!buffer || buffer->accumulation_count == 0) return false;
    
    switch (buffer->strategy) {
        case GRADIENT_ACCUMULATION_MEAN:
            // Average the accumulated gradients
            for (size_t i = 0; i < buffer->size; i++) {
                buffer->gradients[i] /= (double)buffer->accumulation_count;
            }
            break;
            
        case GRADIENT_ACCUMULATION_SUM:
            // Keep as sum (no change needed)
            break;
            
        case GRADIENT_ACCUMULATION_WEIGHTED:
            // Weighted sum already applied during accumulation
            break;
    }
    
    return true;
}

bool gradient_buffer_get_gradients(
    const GradientBuffer* buffer,
    double* gradients,
    size_t size
) {
    if (!buffer || !gradients || size != buffer->size) return false;
    
    memcpy(gradients, buffer->gradients, size * sizeof(double));
    return true;
}

/* ============================================================================
 * Gradient Operations
 * ============================================================================ */

void gradient_add(
    double* result,
    const double* a,
    const double* b,
    size_t size
) {
    if (!result || !a || !b || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        result[i] = a[i] + b[i];
    }
}

void gradient_subtract(
    double* result,
    const double* a,
    const double* b,
    size_t size
) {
    if (!result || !a || !b || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        result[i] = a[i] - b[i];
    }
}

void gradient_scale(
    double* result,
    const double* gradients,
    double scalar,
    size_t size
) {
    if (!result || !gradients || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        result[i] = gradients[i] * scalar;
    }
}

void gradient_multiply(
    double* result,
    const double* a,
    const double* b,
    size_t size
) {
    if (!result || !a || !b || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        result[i] = a[i] * b[i];
    }
}

/* ============================================================================
 * Gradient Clipping
 * ============================================================================ */

size_t gradient_clip_by_value(
    double* gradients,
    size_t size,
    double min_value,
    double max_value
) {
    if (!gradients || size == 0) return 0;
    
    size_t clipped_count = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (gradients[i] < min_value) {
            gradients[i] = min_value;
            clipped_count++;
        } else if (gradients[i] > max_value) {
            gradients[i] = max_value;
            clipped_count++;
        }
    }
    
    return clipped_count;
}

bool gradient_clip_by_norm(
    double* gradients,
    size_t size,
    double max_norm
) {
    if (!gradients || size == 0 || max_norm <= 0.0) return false;
    
    double norm = gradient_l2_norm(gradients, size);
    
    if (norm > max_norm) {
        double scale = max_norm / norm;
        for (size_t i = 0; i < size; i++) {
            gradients[i] *= scale;
        }
        return true;
    }
    
    return false;
}

bool gradient_clip_by_global_norm(
    double** gradient_arrays,
    const size_t* sizes,
    size_t num_arrays,
    double max_norm
) {
    if (!gradient_arrays || !sizes || num_arrays == 0 || max_norm <= 0.0) {
        return false;
    }
    
    // Compute global norm
    double global_norm = gradient_global_norm(gradient_arrays, sizes, num_arrays);
    
    if (global_norm > max_norm) {
        double scale = max_norm / global_norm;
        
        // Scale all gradient arrays
        for (size_t i = 0; i < num_arrays; i++) {
            for (size_t j = 0; j < sizes[i]; j++) {
                gradient_arrays[i][j] *= scale;
            }
        }
        return true;
    }
    
    return false;
}

/* ============================================================================
 * Gradient Statistics
 * ============================================================================ */

GradientStats gradient_compute_stats(const double* gradients, size_t size) {
    GradientStats stats = {0};
    
    if (!gradients || size == 0) return stats;
    
    // Compute mean
    stats.mean = numerical_mean(gradients, size);
    
    // Compute variance
    stats.variance = numerical_variance(gradients, size, stats.mean, false);
    
    // Compute norms
    stats.l1_norm = gradient_l1_norm(gradients, size);
    stats.l2_norm = gradient_l2_norm(gradients, size);
    
    // Find max and min absolute values, count zeros, NaNs, Infs
    stats.max_abs = 0.0;
    stats.min_abs = MATH_INFINITY;
    stats.num_zero = 0;
    stats.num_nan = 0;
    stats.num_inf = 0;
    
    for (size_t i = 0; i < size; i++) {
        double val = gradients[i];
        double abs_val = (val < 0) ? -val : val;
        
        if (numerical_is_nan(val)) {
            stats.num_nan++;
        } else if (numerical_is_inf(val)) {
            stats.num_inf++;
        } else {
            if (abs_val > stats.max_abs) stats.max_abs = abs_val;
            if (abs_val < stats.min_abs) stats.min_abs = abs_val;
            if (abs_val < 1e-10) stats.num_zero++;
        }
    }
    
    return stats;
}

double gradient_l1_norm(const double* gradients, size_t size) {
    if (!gradients || size == 0) return 0.0;
    
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        double val = gradients[i];
        norm += (val < 0) ? -val : val;
    }
    return norm;
}

double gradient_l2_norm(const double* gradients, size_t size) {
    if (!gradients || size == 0) return 0.0;
    
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    return math_sqrt(norm);
}

double gradient_global_norm(
    double** gradient_arrays,
    const size_t* sizes,
    size_t num_arrays
) {
    if (!gradient_arrays || !sizes || num_arrays == 0) return 0.0;
    
    double global_norm_sq = 0.0;
    
    for (size_t i = 0; i < num_arrays; i++) {
        if (!gradient_arrays[i]) continue;
        
        for (size_t j = 0; j < sizes[i]; j++) {
            double val = gradient_arrays[i][j];
            global_norm_sq += val * val;
        }
    }
    
    return math_sqrt(global_norm_sq);
}

/* ============================================================================
 * Gradient Checking and Validation
 * ============================================================================ */

void gradient_check_numerical_issues(
    const double* gradients,
    size_t size,
    bool* has_nan,
    bool* has_inf,
    size_t* num_zero
) {
    if (!gradients || size == 0) return;
    
    *has_nan = false;
    *has_inf = false;
    *num_zero = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (numerical_is_nan(gradients[i])) {
            *has_nan = true;
        }
        if (numerical_is_inf(gradients[i])) {
            *has_inf = true;
        }
        if (math_abs(gradients[i]) < 1e-10) {
            (*num_zero)++;
        }
    }
}

bool gradient_validate(
    const double* gradients,
    size_t size,
    double max_abs_value
) {
    if (!gradients || size == 0) return false;
    
    for (size_t i = 0; i < size; i++) {
        if (numerical_is_nan(gradients[i]) || numerical_is_inf(gradients[i])) {
            return false;
        }
        
        double abs_val = math_abs(gradients[i]);
        if (abs_val > max_abs_value) {
            return false;
        }
    }
    
    return true;
}

/* ============================================================================
 * Hierarchical Gradient Aggregation
 * ============================================================================ */

bool gradient_aggregate(
    double* result,
    double** gradient_arrays,
    const size_t* sizes,
    size_t num_arrays,
    GradientAccumulationStrategy strategy
) {
    if (!result || !gradient_arrays || !sizes || num_arrays == 0) {
        return false;
    }
    
    // Verify all sizes are equal
    size_t size = sizes[0];
    for (size_t i = 1; i < num_arrays; i++) {
        if (sizes[i] != size) return false;
    }
    
    // Initialize result to zero
    memset(result, 0, size * sizeof(double));
    
    // Accumulate gradients
    for (size_t i = 0; i < num_arrays; i++) {
        if (!gradient_arrays[i]) continue;
        
        for (size_t j = 0; j < size; j++) {
            result[j] += gradient_arrays[i][j];
        }
    }
    
    // Apply strategy
    if (strategy == GRADIENT_ACCUMULATION_MEAN) {
        for (size_t j = 0; j < size; j++) {
            result[j] /= (double)num_arrays;
        }
    }
    
    return true;
}

bool gradient_aggregate_weighted(
    double* result,
    double** gradient_arrays,
    const double* weights,
    const size_t* sizes,
    size_t num_arrays
) {
    if (!result || !gradient_arrays || !weights || !sizes || num_arrays == 0) {
        return false;
    }
    
    // Verify all sizes are equal
    size_t size = sizes[0];
    for (size_t i = 1; i < num_arrays; i++) {
        if (sizes[i] != size) return false;
    }
    
    // Initialize result to zero
    memset(result, 0, size * sizeof(double));
    
    // Accumulate weighted gradients
    for (size_t i = 0; i < num_arrays; i++) {
        if (!gradient_arrays[i]) continue;
        
        double weight = weights[i];
        for (size_t j = 0; j < size; j++) {
            result[j] += weight * gradient_arrays[i][j];
        }
    }
    
    return true;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

void gradient_copy(double* dest, const double* src, size_t size) {
    if (!dest || !src || size == 0) return;
    memcpy(dest, src, size * sizeof(double));
}

void gradient_zero(double* gradients, size_t size) {
    if (!gradients || size == 0) return;
    memset(gradients, 0, size * sizeof(double));
}

void gradient_print_stats(const GradientStats* stats, const char* name) {
    if (!stats) return;
    
    printf("Gradient Statistics for %s:\n", name ? name : "Unknown");
    printf("  Mean: %.6e\n", stats->mean);
    printf("  Variance: %.6e\n", stats->variance);
    printf("  L1 Norm: %.6e\n", stats->l1_norm);
    printf("  L2 Norm: %.6e\n", stats->l2_norm);
    printf("  Max Abs: %.6e\n", stats->max_abs);
    printf("  Min Abs: %.6e\n", stats->min_abs);
    printf("  Num Zero: %zu\n", stats->num_zero);
    printf("  Num NaN: %zu\n", stats->num_nan);
    printf("  Num Inf: %zu\n", stats->num_inf);
}
