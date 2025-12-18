/**
 * @file scale.h
 * @brief GeometricScale - Scaling Operations
 * 
 * In geometric computation, multiplication and division are scaling operations.
 * 
 * Key Insight: Multiplying numbers is scaling magnitudes.
 *   5 × 3 = scale magnitude of 5 by factor 3
 * 
 * This is why multiplication can be O(1) - it's just magnitude arithmetic!
 */

#ifndef GEOMETRIC_CORE_SCALE_H
#define GEOMETRIC_CORE_SCALE_H

#include "point.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// GEOMETRIC SCALE
// ============================================================================

/**
 * @brief A scale factor for magnitude operations
 * 
 * Scales are used for multiplication and division.
 * They represent how much to scale the magnitude.
 */
typedef struct {
    // Scale factor (integer part)
    int64_t factor;
    
    // Layer shift (how many layers to move)
    int8_t layer_shift;
    
    // Exact representation (for rational scales)
    int64_t numerator;
    int64_t denominator;
    bool is_exact;
    
    // Direction (1 = scale up, -1 = scale down)
    int8_t direction;
    
} GeometricScale;

// ============================================================================
// CREATION
// ============================================================================

/**
 * @brief Create scale from integer factor
 * 
 * @param factor Scale factor
 * @return Geometric scale
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_from_int(int64_t factor);

/**
 * @brief Create scale from double factor
 * 
 * @param factor Scale factor
 * @return Geometric scale
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_from_double(double factor);

/**
 * @brief Create scale from point
 * 
 * @param point Geometric point
 * @return Geometric scale
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_from_point(const GeometricPoint* point);

/**
 * @brief Create scale from fraction
 * 
 * @param numerator Numerator
 * @param denominator Denominator
 * @return Geometric scale
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_from_fraction(int64_t numerator, int64_t denominator);

/**
 * @brief Create identity scale (×1)
 * 
 * @return Identity scale
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_identity(void);

/**
 * @brief Create inverse scale (÷factor)
 * 
 * @param scale Scale to invert
 * @return Inverse scale
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_inverse(const GeometricScale* scale);

// ============================================================================
// CONVERSION
// ============================================================================

/**
 * @brief Convert scale to double
 * 
 * @param scale Geometric scale
 * @return Scale factor as double
 * 
 * Complexity: O(1)
 */
double geo_scale_to_double(const GeometricScale* scale);

/**
 * @brief Convert scale to integer
 * 
 * @param scale Geometric scale
 * @return Scale factor as integer (truncated)
 * 
 * Complexity: O(1)
 */
int64_t geo_scale_to_int(const GeometricScale* scale);

// ============================================================================
// SCALING OPERATIONS (Multiplication/Division)
// ============================================================================

/**
 * @brief Scale point up (MULTIPLICATION)
 * 
 * @param point Point to scale
 * @param scale Scale factor
 * @return Scaled point
 * 
 * Complexity: O(1)
 * 
 * This is geometric multiplication:
 *   a × b = scale(a, factor_from(b))
 * 
 * Example:
 *   5 × 3 = scale position 5 by factor 3
 *         = position 3, magnitude increased
 */
GeometricPoint geo_scale_up(const GeometricPoint* point, const GeometricScale* scale);

/**
 * @brief Scale point down (DIVISION)
 * 
 * @param point Point to scale
 * @param scale Scale factor
 * @return Scaled point
 * 
 * Complexity: O(1)
 * 
 * This is geometric division:
 *   a ÷ b = scale_down(a, factor_from(b))
 * 
 * Example:
 *   15 ÷ 3 = scale position 3 by inverse factor 3
 *          = position 5, magnitude decreased
 */
GeometricPoint geo_scale_down(const GeometricPoint* point, const GeometricScale* scale);

/**
 * @brief Scale point by integer factor
 * 
 * @param point Point to scale
 * @param factor Integer factor
 * @return Scaled point
 * 
 * Complexity: O(1)
 */
GeometricPoint geo_scale_by_int(const GeometricPoint* point, int64_t factor);

/**
 * @brief Divide point by integer factor
 * 
 * @param point Point to divide
 * @param divisor Integer divisor
 * @return Divided point
 * 
 * Complexity: O(1)
 */
GeometricPoint geo_divide_by_int(const GeometricPoint* point, int64_t divisor);

// ============================================================================
// SCALE ARITHMETIC
// ============================================================================

/**
 * @brief Multiply two scales
 * 
 * @param a First scale
 * @param b Second scale
 * @return Product of scales
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_multiply(const GeometricScale* a, const GeometricScale* b);

/**
 * @brief Divide two scales
 * 
 * @param a First scale
 * @param b Second scale
 * @return Quotient of scales
 * 
 * Complexity: O(1)
 */
GeometricScale geo_scale_divide(const GeometricScale* a, const GeometricScale* b);

// ============================================================================
// SCALE PROPERTIES
// ============================================================================

/**
 * @brief Check if scale is identity (×1)
 * 
 * @param scale Scale to check
 * @return True if identity
 * 
 * Complexity: O(1)
 */
bool geo_scale_is_identity(const GeometricScale* scale);

/**
 * @brief Check if scale is zero (×0)
 * 
 * @param scale Scale to check
 * @return True if zero
 * 
 * Complexity: O(1)
 */
bool geo_scale_is_zero(const GeometricScale* scale);

/**
 * @brief Check if scale is exact
 * 
 * @param scale Scale to check
 * @return True if exact
 * 
 * Complexity: O(1)
 */
bool geo_scale_is_exact(const GeometricScale* scale);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * @brief Print scale
 * 
 * @param scale Scale to print
 * 
 * Complexity: O(1)
 */
void geo_scale_print(const GeometricScale* scale);

/**
 * @brief Validate scale
 * 
 * @param scale Scale to validate
 * @return True if valid
 * 
 * Complexity: O(1)
 */
bool geo_scale_validate(const GeometricScale* scale);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_CORE_SCALE_H