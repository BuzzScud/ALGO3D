/**
 * @file numerical.c
 * @brief Implementation of numerical analysis functions
 * 
 * PHASE 1 INTEGRATION: Migrated to NEW math library
 * - Replaced math_exp with math_exp
 * - Replaced math_log with math_log
 * - Replaced math_sqrt with math_sqrt
 * - Added high-precision variants using Crystalline Abacus
 * 
 * PHASE 2: Consolidated constants
 * - Removed local INFINITY definition
 * - Using MATH_INFINITY from math/types.h
 */

#include "numerical.h"
#include "math/transcendental.h"
#include "math/abacus.h"
#include "math/types.h"  // PHASE 2: For MATH_INFINITY
#include <string.h>
#include <float.h>
#include <stdbool.h>

/* ============================================================================
 * Softmax and Related Functions
 * ============================================================================ */

void numerical_softmax(const double* input, double* output, size_t size) {
    if (!input || !output || size == 0) return;
    
    // Find max for numerical stability
    double max_val = input[0];
    for (size_t i = 1; i < size; i++) {
        if (input[i] > max_val) {
            max_val = input[i];
        }
    }
    
    // Compute exp(x - max) and sum
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        output[i] = math_exp(input[i] - max_val);  // PHASE 1: NEW math library
        sum += output[i];
    }
    
    // Normalize
    for (size_t i = 0; i < size; i++) {
        output[i] /= sum;
    }
}

void numerical_log_softmax(const double* input, double* output, size_t size) {
    if (!input || !output || size == 0) return;
    
    // Compute log-sum-exp
    double lse = numerical_log_sum_exp(input, size);
    
    // Compute log-softmax
    for (size_t i = 0; i < size; i++) {
        output[i] = input[i] - lse;
    }
}

double numerical_log_sum_exp(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    // Find max for numerical stability
    double max_val = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] > max_val) {
            max_val = values[i];
        }
    }
    
    // Compute sum(exp(x - max))
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += math_exp(values[i] - max_val);  // PHASE 1: NEW math library
    }
    
    // Return max + log(sum)
    return max_val + math_log(sum);  // PHASE 1: NEW math library
}

void numerical_softmax_2d(const double* input, double* output, 
                         size_t batch_size, size_t num_classes) {
    for (size_t i = 0; i < batch_size; i++) {
        numerical_softmax(
            input + i * num_classes,
            output + i * num_classes,
            num_classes
        );
    }
}

void numerical_log_softmax_2d(const double* input, double* output,
                             size_t batch_size, size_t num_classes) {
    for (size_t i = 0; i < batch_size; i++) {
        numerical_log_softmax(
            input + i * num_classes,
            output + i * num_classes,
            num_classes
        );
    }
}

/* ============================================================================
 * Safe Mathematical Operations
 * ============================================================================ */

double numerical_safe_log(double x, double epsilon) {
    return math_log(x + epsilon);  // PHASE 1: NEW math library
}

double numerical_safe_exp(double x, double max_exp) {
    if (x > max_exp) return math_exp(max_exp);  // PHASE 1: NEW math library
    return math_exp(x);  // PHASE 1: NEW math library
}

double numerical_safe_divide(double numerator, double denominator, double epsilon) {
    return numerator / (denominator + epsilon);
}

double numerical_safe_sqrt(double x, double epsilon) {
    if (x < 0.0) x = 0.0;
    return math_sqrt(x + epsilon);  // PHASE 1: NEW math library
}

/* ============================================================================
 * Numerical Stability Checks
 * ============================================================================ */

bool numerical_is_nan(double x) {
    return x != x;
}

bool numerical_is_inf(double x) {
    return x == MATH_INFINITY || x == MATH_NEG_INFINITY;
}

bool numerical_is_finite(double x) {
    return !numerical_is_nan(x) && !numerical_is_inf(x);
}

void numerical_check_array(const double* values, size_t size,
                          bool* has_nan, bool* has_inf) {
    if (!values || size == 0) return;
    
    *has_nan = false;
    *has_inf = false;
    
    for (size_t i = 0; i < size; i++) {
        if (numerical_is_nan(values[i])) {
            *has_nan = true;
        }
        if (numerical_is_inf(values[i])) {
            *has_inf = true;
        }
        if (*has_nan && *has_inf) break;
    }
}

double numerical_clip(double x, double min_value, double max_value) {
    if (x < min_value) return min_value;
    if (x > max_value) return max_value;
    return x;
}

void numerical_clip_array(double* values, size_t size,
                         double min_value, double max_value) {
    if (!values || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = numerical_clip(values[i], min_value, max_value);
    }
}

/* ============================================================================
 * Array Operations
 * ============================================================================ */

double numerical_max(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double max_val = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] > max_val) {
            max_val = values[i];
        }
    }
    return max_val;
}

double numerical_min(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double min_val = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] < min_val) {
            min_val = values[i];
        }
    }
    return min_val;
}

double numerical_sum(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum;
}

double numerical_mean(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    return numerical_sum(values, size) / (double)size;
}

double numerical_variance(const double* values, size_t size,
                         double mean, bool compute_mean) {
    if (!values || size == 0) return 0.0;
    
    if (compute_mean) {
        mean = numerical_mean(values, size);
    }
    
    double var = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = values[i] - mean;
        var += diff * diff;
    }
    
    return var / (double)size;
}

double numerical_std_dev(const double* values, size_t size,
                        double mean, bool compute_mean) {
    double var = numerical_variance(values, size, mean, compute_mean);
    return math_sqrt(var);  // PHASE 1: NEW math library
}

/* ============================================================================
 * Normalization Functions
 * ============================================================================ */

void numerical_normalize_01(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double min_val = numerical_min(values, size);
    double max_val = numerical_max(values, size);
    double range = max_val - min_val;
    
    if (range < 1e-10) return; // Avoid division by zero
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - min_val) / range;
    }
}

void numerical_standardize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double mean = numerical_mean(values, size);
    double std = numerical_std_dev(values, size, mean, false);
    
    if (std < 1e-10) return; // Avoid division by zero
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - mean) / std;
    }
}

void numerical_l2_normalize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm += values[i] * values[i];
    }
    norm = math_sqrt(norm);  // PHASE 1: NEW math library
    
    if (norm < 1e-10) return; // Avoid division by zero
    
    for (size_t i = 0; i < size; i++) {
        values[i] /= norm;
    }
}

/* ============================================================================
 * Distance and Similarity Functions
 * ============================================================================ */

double numerical_l2_distance(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dist = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = a[i] - b[i];
        dist += diff * diff;
    }
    return math_sqrt(dist);  // PHASE 1: NEW math library
}

double numerical_l1_distance(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dist = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = a[i] - b[i];
        dist += (diff < 0) ? -diff : diff;
    }
    return dist;
}

double numerical_cosine_similarity(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dot = numerical_dot_product(a, b, size);
    
    double norm_a = 0.0, norm_b = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    norm_a = math_sqrt(norm_a);  // PHASE 1: NEW math library
    norm_b = math_sqrt(norm_b);  // PHASE 1: NEW math library
    
    if (norm_a < 1e-10 || norm_b < 1e-10) return 0.0;
    
    return dot / (norm_a * norm_b);
}

double numerical_dot_product(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dot = 0.0;
    for (size_t i = 0; i < size; i++) {
        dot += a[i] * b[i];
    }
    return dot;
}

double numerical_scalar_decay(double r, double C, double alpha) {
    // decay(r) = C / (r^alpha + epsilon)
    // epsilon prevents division by zero
    double denominator = math_pow(r, alpha) + 1e-10;  // PHASE 1: NEW math library
    return C / denominator;
}
