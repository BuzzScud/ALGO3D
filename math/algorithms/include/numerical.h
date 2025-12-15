/**
 * @file numerical.h
 * @brief Numerical analysis and mathematical utilities
 * 
 * This library provides numerical analysis functions commonly used in
 * optimization, machine learning, and scientific computing.
 * 
 * These are PURE MATHEMATICAL FUNCTIONS:
 * - Softmax and log-softmax (probability normalization)
 * - Log-sum-exp trick (numerical stability)
 * - Safe mathematical operations
 * - Numerical stability utilities
 */

#ifndef NUMERICAL_H
#define NUMERICAL_H

#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * Softmax and Related Functions
 * ============================================================================ */

/**
 * @brief Compute softmax function
 * 
 * Softmax(x_i) = exp(x_i) / sum(exp(x_j))
 * 
 * Uses log-sum-exp trick for numerical stability.
 * 
 * @param input Input values
 * @param output Output probabilities (must be pre-allocated)
 * @param size Number of values
 */
void numerical_softmax(
    const double* input,
    double* output,
    size_t size
);

/**
 * @brief Compute log-softmax function
 * 
 * LogSoftmax(x_i) = x_i - log(sum(exp(x_j)))
 * 
 * More numerically stable than log(softmax(x)).
 * 
 * @param input Input values
 * @param output Output log-probabilities (must be pre-allocated)
 * @param size Number of values
 */
void numerical_log_softmax(
    const double* input,
    double* output,
    size_t size
);

/**
 * @brief Compute log-sum-exp with numerical stability
 * 
 * LogSumExp(x) = log(sum(exp(x_i)))
 * 
 * Uses the trick: LSE(x) = max(x) + log(sum(exp(x_i - max(x))))
 * 
 * @param values Input values
 * @param size Number of values
 * @return log(sum(exp(values)))
 */
double numerical_log_sum_exp(
    const double* values,
    size_t size
);

/**
 * @brief Compute softmax for 2D array (batch processing)
 * @param input Input values [batch_size x num_classes]
 * @param output Output probabilities [batch_size x num_classes]
 * @param batch_size Number of samples
 * @param num_classes Number of classes per sample
 */
void numerical_softmax_2d(
    const double* input,
    double* output,
    size_t batch_size,
    size_t num_classes
);

/**
 * @brief Compute log-softmax for 2D array (batch processing)
 * @param input Input values [batch_size x num_classes]
 * @param output Output log-probabilities [batch_size x num_classes]
 * @param batch_size Number of samples
 * @param num_classes Number of classes per sample
 */
void numerical_log_softmax_2d(
    const double* input,
    double* output,
    size_t batch_size,
    size_t num_classes
);

/* ============================================================================
 * Safe Mathematical Operations
 * ============================================================================ */

/**
 * @brief Safe logarithm (avoids log(0))
 * @param x Input value
 * @param epsilon Small constant to add (default: 1e-10)
 * @return log(x + epsilon)
 */
double numerical_safe_log(double x, double epsilon);

/**
 * @brief Safe exponential (clips to avoid overflow)
 * @param x Input value
 * @param max_exp Maximum exponent (default: 88.0)
 * @return exp(min(x, max_exp))
 */
double numerical_safe_exp(double x, double max_exp);

/**
 * @brief Safe division (avoids division by zero)
 * @param numerator Numerator
 * @param denominator Denominator
 * @param epsilon Small constant to add to denominator
 * @return numerator / (denominator + epsilon)
 */
double numerical_safe_divide(
    double numerator,
    double denominator,
    double epsilon
);

/**
 * @brief Safe square root (avoids sqrt of negative)
 * @param x Input value
 * @param epsilon Small constant to add
 * @return sqrt(max(x, 0) + epsilon)
 */
double numerical_safe_sqrt(double x, double epsilon);

/* ============================================================================
 * Numerical Stability Checks
 * ============================================================================ */

/**
 * @brief Check if value is NaN
 * @param x Value to check
 * @return true if NaN
 */
bool numerical_is_nan(double x);

/**
 * @brief Check if value is infinite
 * @param x Value to check
 * @return true if infinite
 */
bool numerical_is_inf(double x);

/**
 * @brief Check if value is finite (not NaN or Inf)
 * @param x Value to check
 * @return true if finite
 */
bool numerical_is_finite(double x);

/**
 * @brief Check array for NaN or Inf
 * @param values Array to check
 * @param size Number of values
 * @param has_nan Output: whether NaN was found
 * @param has_inf Output: whether Inf was found
 */
void numerical_check_array(
    const double* values,
    size_t size,
    bool* has_nan,
    bool* has_inf
);

/**
 * @brief Clip value to range
 * @param x Value to clip
 * @param min_value Minimum value
 * @param max_value Maximum value
 * @return Clipped value
 */
double numerical_clip(double x, double min_value, double max_value);

/**
 * @brief Clip array values to range
 * @param values Array to clip (modified in place)
 * @param size Number of values
 * @param min_value Minimum value
 * @param max_value Maximum value
 */
void numerical_clip_array(
    double* values,
    size_t size,
    double min_value,
    double max_value
);

/* ============================================================================
 * Array Operations
 * ============================================================================ */

/**
 * @brief Find maximum value in array
 * @param values Array of values
 * @param size Number of values
 * @return Maximum value
 */
double numerical_max(const double* values, size_t size);

/**
 * @brief Find minimum value in array
 * @param values Array of values
 * @param size Number of values
 * @return Minimum value
 */
double numerical_min(const double* values, size_t size);

/**
 * @brief Compute sum of array
 * @param values Array of values
 * @param size Number of values
 * @return Sum of values
 */
double numerical_sum(const double* values, size_t size);

/**
 * @brief Compute mean of array
 * @param values Array of values
 * @param size Number of values
 * @return Mean value
 */
double numerical_mean(const double* values, size_t size);

/**
 * @brief Compute variance of array
 * @param values Array of values
 * @param size Number of values
 * @param mean Mean value (if known, else pass 0 and set compute_mean=true)
 * @param compute_mean Whether to compute mean first
 * @return Variance
 */
double numerical_variance(
    const double* values,
    size_t size,
    double mean,
    bool compute_mean
);

/**
 * @brief Compute standard deviation of array
 * @param values Array of values
 * @param size Number of values
 * @param mean Mean value (if known, else pass 0 and set compute_mean=true)
 * @param compute_mean Whether to compute mean first
 * @return Standard deviation
 */
double numerical_std_dev(
    const double* values,
    size_t size,
    double mean,
    bool compute_mean
);

/* ============================================================================
 * Normalization Functions
 * ============================================================================ */

/**
 * @brief Normalize array to [0, 1] range
 * @param values Array to normalize (modified in place)
 * @param size Number of values
 */
void numerical_normalize_01(double* values, size_t size);

/**
 * @brief Standardize array (zero mean, unit variance)
 * @param values Array to standardize (modified in place)
 * @param size Number of values
 */
void numerical_standardize(double* values, size_t size);

/**
 * @brief L2 normalize array (unit norm)
 * @param values Array to normalize (modified in place)
 * @param size Number of values
 */
void numerical_l2_normalize(double* values, size_t size);

/* ============================================================================
 * Distance and Similarity Functions
 * ============================================================================ */

/**
 * @brief Compute L2 (Euclidean) distance
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return L2 distance
 */
double numerical_l2_distance(
    const double* a,
    const double* b,
    size_t size
);

/**
 * @brief Compute L1 (Manhattan) distance
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return L1 distance
 */
double numerical_l1_distance(
    const double* a,
    const double* b,
    size_t size
);

/**
 * @brief Compute cosine similarity
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return Cosine similarity [-1, 1]
 */
double numerical_cosine_similarity(
    const double* a,
    const double* b,
    size_t size
);

/**
 * @brief Compute dot product
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return Dot product
 */
double numerical_dot_product(
    const double* a,
    const double* b,
    size_t size
);

/**
 * @brief Calculate scalar decay with distance
 * 
 * Computes: decay(r) = C / (r^alpha + epsilon)
 * 
 * Used for distance-based weighting in:
 * - Inverse distance weighting
 * - Gravitational/electromagnetic decay
 * - Influence functions
 * - Kernel functions
 * 
 * @param r Distance/radius
 * @param C Constant multiplier (default: 1.0)
 * @param alpha Decay exponent (default: 2.0 for inverse square)
 * @return Decay value
 */
double numerical_scalar_decay(double r, double C, double alpha);

#endif /* NUMERICAL_H */
