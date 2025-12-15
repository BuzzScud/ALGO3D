/**
 * @file backprop.h
 * @brief Backpropagation algorithms for gradient computation
 * 
 * This library provides mathematical algorithms for computing gradients
 * via backpropagation (reverse-mode automatic differentiation).
 * 
 * These are PURE MATHEMATICAL ALGORITHMS based on:
 * - Chain rule of calculus
 * - Reverse-mode automatic differentiation
 * - Gradient accumulation and aggregation
 * 
 * They can be used for ANY differentiable function, not just neural networks.
 */

#ifndef BACKPROP_H
#define BACKPROP_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Gradient accumulation strategy
 */
typedef enum {
    GRADIENT_ACCUMULATION_SUM,      /**< Sum gradients */
    GRADIENT_ACCUMULATION_MEAN,     /**< Average gradients */
    GRADIENT_ACCUMULATION_WEIGHTED  /**< Weighted sum of gradients */
} GradientAccumulationStrategy;

/**
 * @brief Gradient buffer for storing and accumulating gradients
 */
typedef struct {
    double* gradients;              /**< Gradient values */
    size_t size;                    /**< Number of gradient values */
    size_t accumulation_count;      /**< Number of accumulated gradients */
    GradientAccumulationStrategy strategy; /**< Accumulation strategy */
    double* weights;                /**< Weights for weighted accumulation */
} GradientBuffer;

/**
 * @brief Gradient statistics
 */
typedef struct {
    double mean;                    /**< Mean gradient value */
    double variance;                /**< Gradient variance */
    double l1_norm;                 /**< L1 norm of gradients */
    double l2_norm;                 /**< L2 norm of gradients */
    double max_abs;                 /**< Maximum absolute gradient */
    double min_abs;                 /**< Minimum absolute gradient */
    size_t num_zero;                /**< Number of zero gradients */
    size_t num_nan;                 /**< Number of NaN gradients */
    size_t num_inf;                 /**< Number of Inf gradients */
} GradientStats;

/* ============================================================================
 * Gradient Buffer Management
 * ============================================================================ */

/**
 * @brief Create gradient buffer
 * @param size Number of gradient values
 * @param strategy Accumulation strategy
 * @return Gradient buffer (caller must free with gradient_buffer_free)
 */
GradientBuffer* gradient_buffer_create(
    size_t size,
    GradientAccumulationStrategy strategy
);

/**
 * @brief Free gradient buffer
 * @param buffer Gradient buffer to free
 */
void gradient_buffer_free(GradientBuffer* buffer);

/**
 * @brief Reset gradient buffer (zero out gradients)
 * @param buffer Gradient buffer to reset
 */
void gradient_buffer_reset(GradientBuffer* buffer);

/**
 * @brief Set accumulation weights
 * @param buffer Gradient buffer
 * @param weights Weights for weighted accumulation
 * @param size Number of weights (must match buffer size)
 * @return Success status
 */
bool gradient_buffer_set_weights(
    GradientBuffer* buffer,
    const double* weights,
    size_t size
);

/* ============================================================================
 * Gradient Accumulation
 * ============================================================================ */

/**
 * @brief Accumulate gradients into buffer
 * @param buffer Gradient buffer
 * @param gradients Gradients to accumulate
 * @param size Number of gradients (must match buffer size)
 * @return Success status
 */
bool gradient_buffer_accumulate(
    GradientBuffer* buffer,
    const double* gradients,
    size_t size
);

/**
 * @brief Accumulate weighted gradients
 * @param buffer Gradient buffer
 * @param gradients Gradients to accumulate
 * @param weight Weight for this accumulation
 * @param size Number of gradients
 * @return Success status
 */
bool gradient_buffer_accumulate_weighted(
    GradientBuffer* buffer,
    const double* gradients,
    double weight,
    size_t size
);

/**
 * @brief Finalize gradient accumulation (apply strategy)
 * @param buffer Gradient buffer
 * @return Success status
 */
bool gradient_buffer_finalize(GradientBuffer* buffer);

/**
 * @brief Get accumulated gradients
 * @param buffer Gradient buffer
 * @param gradients Output buffer for gradients
 * @param size Size of output buffer
 * @return Success status
 */
bool gradient_buffer_get_gradients(
    const GradientBuffer* buffer,
    double* gradients,
    size_t size
);

/* ============================================================================
 * Gradient Operations
 * ============================================================================ */

/**
 * @brief Add two gradient arrays
 * @param result Output array (a + b)
 * @param a First gradient array
 * @param b Second gradient array
 * @param size Number of gradients
 */
void gradient_add(
    double* result,
    const double* a,
    const double* b,
    size_t size
);

/**
 * @brief Subtract gradient arrays
 * @param result Output array (a - b)
 * @param a First gradient array
 * @param b Second gradient array
 * @param size Number of gradients
 */
void gradient_subtract(
    double* result,
    const double* a,
    const double* b,
    size_t size
);

/**
 * @brief Multiply gradients by scalar
 * @param result Output array (gradients * scalar)
 * @param gradients Input gradients
 * @param scalar Scalar multiplier
 * @param size Number of gradients
 */
void gradient_scale(
    double* result,
    const double* gradients,
    double scalar,
    size_t size
);

/**
 * @brief Element-wise multiply gradients
 * @param result Output array (a * b element-wise)
 * @param a First gradient array
 * @param b Second gradient array
 * @param size Number of gradients
 */
void gradient_multiply(
    double* result,
    const double* a,
    const double* b,
    size_t size
);

/* ============================================================================
 * Gradient Clipping
 * ============================================================================ */

/**
 * @brief Clip gradients by value
 * @param gradients Gradients to clip (modified in place)
 * @param size Number of gradients
 * @param min_value Minimum value
 * @param max_value Maximum value
 * @return Number of gradients clipped
 */
size_t gradient_clip_by_value(
    double* gradients,
    size_t size,
    double min_value,
    double max_value
);

/**
 * @brief Clip gradients by norm
 * @param gradients Gradients to clip (modified in place)
 * @param size Number of gradients
 * @param max_norm Maximum L2 norm
 * @return Whether gradients were clipped
 */
bool gradient_clip_by_norm(
    double* gradients,
    size_t size,
    double max_norm
);

/**
 * @brief Clip gradients by global norm (across multiple arrays)
 * @param gradient_arrays Array of gradient arrays
 * @param sizes Array of sizes for each gradient array
 * @param num_arrays Number of gradient arrays
 * @param max_norm Maximum global norm
 * @return Whether gradients were clipped
 */
bool gradient_clip_by_global_norm(
    double** gradient_arrays,
    const size_t* sizes,
    size_t num_arrays,
    double max_norm
);

/* ============================================================================
 * Gradient Statistics
 * ============================================================================ */

/**
 * @brief Compute gradient statistics
 * @param gradients Gradient array
 * @param size Number of gradients
 * @return Gradient statistics
 */
GradientStats gradient_compute_stats(
    const double* gradients,
    size_t size
);

/**
 * @brief Compute L1 norm of gradients
 * @param gradients Gradient array
 * @param size Number of gradients
 * @return L1 norm
 */
double gradient_l1_norm(const double* gradients, size_t size);

/**
 * @brief Compute L2 norm of gradients
 * @param gradients Gradient array
 * @param size Number of gradients
 * @return L2 norm
 */
double gradient_l2_norm(const double* gradients, size_t size);

/**
 * @brief Compute global norm across multiple gradient arrays
 * @param gradient_arrays Array of gradient arrays
 * @param sizes Array of sizes for each gradient array
 * @param num_arrays Number of gradient arrays
 * @return Global L2 norm
 */
double gradient_global_norm(
    double** gradient_arrays,
    const size_t* sizes,
    size_t num_arrays
);

/* ============================================================================
 * Gradient Checking and Validation
 * ============================================================================ */

/**
 * @brief Check gradients for numerical issues
 * @param gradients Gradient array
 * @param size Number of gradients
 * @param has_nan Output: whether NaN was found
 * @param has_inf Output: whether Inf was found
 * @param num_zero Output: number of zero gradients
 */
void gradient_check_numerical_issues(
    const double* gradients,
    size_t size,
    bool* has_nan,
    bool* has_inf,
    size_t* num_zero
);

/**
 * @brief Validate gradients are finite and reasonable
 * @param gradients Gradient array
 * @param size Number of gradients
 * @param max_abs_value Maximum reasonable absolute value
 * @return true if gradients are valid
 */
bool gradient_validate(
    const double* gradients,
    size_t size,
    double max_abs_value
);

/* ============================================================================
 * Hierarchical Gradient Aggregation
 * ============================================================================ */

/**
 * @brief Aggregate gradients from multiple sources (tree reduction)
 * @param result Output aggregated gradients
 * @param gradient_arrays Array of gradient arrays to aggregate
 * @param sizes Array of sizes (all must be equal)
 * @param num_arrays Number of gradient arrays
 * @param strategy Aggregation strategy
 * @return Success status
 */
bool gradient_aggregate(
    double* result,
    double** gradient_arrays,
    const size_t* sizes,
    size_t num_arrays,
    GradientAccumulationStrategy strategy
);

/**
 * @brief Aggregate gradients with weights
 * @param result Output aggregated gradients
 * @param gradient_arrays Array of gradient arrays
 * @param weights Weight for each gradient array
 * @param sizes Array of sizes (all must be equal)
 * @param num_arrays Number of gradient arrays
 * @return Success status
 */
bool gradient_aggregate_weighted(
    double* result,
    double** gradient_arrays,
    const double* weights,
    const size_t* sizes,
    size_t num_arrays
);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Copy gradients
 * @param dest Destination array
 * @param src Source array
 * @param size Number of gradients
 */
void gradient_copy(
    double* dest,
    const double* src,
    size_t size
);

/**
 * @brief Zero out gradients
 * @param gradients Gradient array to zero
 * @param size Number of gradients
 */
void gradient_zero(double* gradients, size_t size);

/**
 * @brief Print gradient statistics
 * @param stats Gradient statistics to print
 * @param name Name/label for the gradients
 */
void gradient_print_stats(
    const GradientStats* stats,
    const char* name
);

#endif /* BACKPROP_H */
