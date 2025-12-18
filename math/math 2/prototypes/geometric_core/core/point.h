/**
 * @file point.h
 * @brief GeometricPoint - The Foundation of Everything
 * 
 * In this system, numbers don't exist. Only geometric positions.
 * Every "number" is actually a position on a 12-fold clock lattice.
 * 
 * Key Insight: 42 is not a number - it's a position.
 *   42 = 3 × 12 + 6
 *        ↑       ↑
 *     magnitude position
 * 
 * Position 6 = 180° on clock
 * Magnitude 3 = Layer 0 (small values)
 */

#ifndef GEOMETRIC_CORE_POINT_H
#define GEOMETRIC_CORE_POINT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define GEO_CLOCK_POSITIONS 12
#define GEO_DEGREES_PER_POSITION 30.0
#define GEO_PRIME_POSITIONS_COUNT 4

// Prime positions on clock: {1, 5, 7, 11}
static const uint8_t GEO_PRIME_POSITIONS[GEO_PRIME_POSITIONS_COUNT] = {1, 5, 7, 11};

// ============================================================================
// GEOMETRIC POINT
// ============================================================================

/**
 * @brief A point in geometric space
 * 
 * This is the fundamental unit of computation. Everything is a point.
 * Points have:
 * - Position on 12-fold clock (0-11)
 * - Magnitude (which "lap" around the clock)
 * - Layer (computed from magnitude)
 * 
 * Properties:
 * - Exact (no floating point)
 * - Infinite precision (magnitude can grow)
 * - O(1) operations (modular arithmetic)
 */
typedef struct {
    // ========================================================================
    // CORE PROPERTIES
    // ========================================================================
    
    /**
     * Position on 12-fold clock (0-11)
     * 
     * This determines the angle:
     *   angle = position × 30°
     * 
     * Examples:
     *   position 0  = 0°   (control position)
     *   position 3  = 90°
     *   position 6  = 180°
     *   position 9  = 270°
     */
    uint8_t position;
    
    /**
     * Magnitude - which "lap" around the clock
     * 
     * Can be negative for values < 0
     * Can be arbitrarily large (infinite precision)
     * 
     * Examples:
     *   magnitude 0  = first lap (0-11)
     *   magnitude 1  = second lap (12-23)
     *   magnitude 10 = eleventh lap (120-131)
     */
    int64_t magnitude;
    
    /**
     * Layer in 88D space (0-7)
     * 
     * Computed automatically from magnitude:
     *   Layer 0: 10^0  to 10^3   (small)
     *   Layer 1: 10^3  to 10^6   (medium)
     *   Layer 2: 10^6  to 10^9   (large)
     *   Layer 3: 10^9  to 10^12  (very large)
     *   Layer 4: 10^12 to 10^15  (huge)
     *   Layer 5: 10^15 to 10^18  (massive)
     *   Layer 6: 10^18 to 10^21  (enormous)
     *   Layer 7: > 10^21         (infinite)
     */
    uint8_t layer;
    
    // ========================================================================
    // GEOMETRIC PROPERTIES
    // ========================================================================
    
    /**
     * Angle in degrees (0-360°)
     * 
     * Computed from position:
     *   angle = position × 30°
     */
    double angle;
    
    /**
     * Quadrant (0-3)
     * 
     * Which quarter of the clock:
     *   Q0: positions 0-2   (0° to 90°)
     *   Q1: positions 3-5   (90° to 180°)
     *   Q2: positions 6-8   (180° to 270°)
     *   Q3: positions 9-11  (270° to 360°)
     */
    uint8_t quadrant;
    
    /**
     * Ring (0-3)
     * 
     * Which ring in the clock lattice:
     *   Ring 0: Inner ring (positions 0-2)
     *   Ring 1: Second ring (positions 3-5)
     *   Ring 2: Third ring (positions 6-8)
     *   Ring 3: Outer ring (positions 9-11)
     */
    uint8_t ring;
    
    // ========================================================================
    // CLASSIFICATION
    // ========================================================================
    
    /**
     * Is this a prime position? {1, 5, 7, 11}
     * 
     * Prime positions have special properties:
     * - p² ≡ 1 (mod 12) for all primes
     * - Universal polarity flip
     * - Used for prime generation
     */
    bool is_prime_position;
    
    /**
     * Is this the control position? (0/12)
     * 
     * Position 0/12 is special:
     * - Represents infinity (∞)
     * - Control thread position
     * - Observer position
     */
    bool is_control;
    
    /**
     * Is this value negative?
     * 
     * Negative values have negative magnitude
     */
    bool is_negative;
    
    /**
     * Is this value zero?
     * 
     * Zero has position=0, magnitude=0
     */
    bool is_zero;
    
    // ========================================================================
    // EXACT REPRESENTATION
    // ========================================================================
    
    /**
     * Is this an exact value?
     * 
     * Always true for integer positions
     * May be false for fractional values
     */
    bool is_exact;
    
    /**
     * Fractional part (for non-integer values)
     * 
     * For values like π, √2, etc.
     * Stored as numerator/denominator for exactness
     */
    int64_t fractional_numerator;
    int64_t fractional_denominator;
    
} GeometricPoint;

// ============================================================================
// CREATION & INITIALIZATION
// ============================================================================

/**
 * @brief Create a geometric point from position and magnitude
 * 
 * @param position Position on clock (0-11)
 * @param magnitude Which lap around clock
 * @return Initialized geometric point
 * 
 * Complexity: O(1)
 * 
 * Example:
 *   geo_point_create(6, 3) creates point for value 42
 *   (because 42 = 3 × 12 + 6)
 */
GeometricPoint geo_point_create(uint8_t position, int64_t magnitude);

/**
 * @brief Create point from traditional integer
 * 
 * @param value Integer value
 * @return Geometric point
 * 
 * Complexity: O(1)
 * 
 * Example:
 *   geo_point_from_int(42) → position=6, magnitude=3
 */
GeometricPoint geo_point_from_int(int64_t value);

/**
 * @brief Create point from traditional double
 * 
 * @param value Double value
 * @return Geometric point (may have fractional part)
 * 
 * Complexity: O(1)
 * 
 * Note: Converts to exact fractional representation
 */
GeometricPoint geo_point_from_double(double value);

/**
 * @brief Create zero point
 * 
 * @return Point representing zero
 * 
 * Complexity: O(1)
 */
GeometricPoint geo_point_zero(void);

/**
 * @brief Create unit point (one)
 * 
 * @return Point representing one
 * 
 * Complexity: O(1)
 */
GeometricPoint geo_point_one(void);

/**
 * @brief Create infinity point
 * 
 * @return Point representing infinity (position 0/12)
 * 
 * Complexity: O(1)
 */
GeometricPoint geo_point_infinity(void);

// ============================================================================
// CONVERSION
// ============================================================================

/**
 * @brief Convert point to traditional integer
 * 
 * @param point Geometric point
 * @return Integer value
 * 
 * Complexity: O(1)
 * 
 * Formula: value = magnitude × 12 + position
 */
int64_t geo_point_to_int(const GeometricPoint* point);

/**
 * @brief Convert point to traditional double
 * 
 * @param point Geometric point
 * @return Double value
 * 
 * Complexity: O(1)
 * 
 * Includes fractional part if present
 */
double geo_point_to_double(const GeometricPoint* point);

/**
 * @brief Get angle in degrees
 * 
 * @param point Geometric point
 * @return Angle (0-360°)
 * 
 * Complexity: O(1)
 * 
 * Formula: angle = position × 30°
 */
double geo_point_angle(const GeometricPoint* point);

/**
 * @brief Get angle in radians
 * 
 * @param point Geometric point
 * @return Angle (0-2π)
 * 
 * Complexity: O(1)
 */
double geo_point_angle_radians(const GeometricPoint* point);

// ============================================================================
// PROPERTIES
// ============================================================================

/**
 * @brief Check if point is at prime position
 * 
 * @param point Geometric point
 * @return True if position ∈ {1, 5, 7, 11}
 * 
 * Complexity: O(1)
 */
bool geo_point_is_prime_position(const GeometricPoint* point);

/**
 * @brief Check if point is at control position
 * 
 * @param point Geometric point
 * @return True if position = 0/12
 * 
 * Complexity: O(1)
 */
bool geo_point_is_control(const GeometricPoint* point);

/**
 * @brief Check if point is zero
 * 
 * @param point Geometric point
 * @return True if value = 0
 * 
 * Complexity: O(1)
 */
bool geo_point_is_zero(const GeometricPoint* point);

/**
 * @brief Check if point is one
 * 
 * @param point Geometric point
 * @return True if value = 1
 * 
 * Complexity: O(1)
 */
bool geo_point_is_one(const GeometricPoint* point);

/**
 * @brief Check if point is negative
 * 
 * @param point Geometric point
 * @return True if magnitude < 0
 * 
 * Complexity: O(1)
 */
bool geo_point_is_negative(const GeometricPoint* point);

/**
 * @brief Check if point is exact (no fractional part)
 * 
 * @param point Geometric point
 * @return True if exact
 * 
 * Complexity: O(1)
 */
bool geo_point_is_exact(const GeometricPoint* point);

// ============================================================================
// COMPARISON
// ============================================================================

/**
 * @brief Compare two points
 * 
 * @param a First point
 * @param b Second point
 * @return -1 if a < b, 0 if a = b, 1 if a > b
 * 
 * Complexity: O(1)
 */
int geo_point_compare(const GeometricPoint* a, const GeometricPoint* b);

/**
 * @brief Check if two points are equal
 * 
 * @param a First point
 * @param b Second point
 * @return True if equal
 * 
 * Complexity: O(1)
 */
bool geo_point_equals(const GeometricPoint* a, const GeometricPoint* b);

/**
 * @brief Check if point is less than another
 * 
 * @param a First point
 * @param b Second point
 * @return True if a < b
 * 
 * Complexity: O(1)
 */
bool geo_point_less_than(const GeometricPoint* a, const GeometricPoint* b);

/**
 * @brief Check if point is greater than another
 * 
 * @param a First point
 * @param b Second point
 * @return True if a > b
 * 
 * Complexity: O(1)
 */
bool geo_point_greater_than(const GeometricPoint* a, const GeometricPoint* b);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * @brief Copy point
 * 
 * @param src Source point
 * @return Copy of point
 * 
 * Complexity: O(1)
 */
GeometricPoint geo_point_copy(const GeometricPoint* src);

/**
 * @brief Print point (for debugging)
 * 
 * @param point Point to print
 * 
 * Complexity: O(1)
 */
void geo_point_print(const GeometricPoint* point);

/**
 * @brief Print point in detail
 * 
 * @param point Point to print
 * 
 * Complexity: O(1)
 */
void geo_point_print_detailed(const GeometricPoint* point);

/**
 * @brief Validate point
 * 
 * @param point Point to validate
 * @return True if valid
 * 
 * Complexity: O(1)
 */
bool geo_point_validate(const GeometricPoint* point);

// ============================================================================
// LAYER OPERATIONS
// ============================================================================

/**
 * @brief Compute layer from magnitude
 * 
 * @param magnitude Magnitude value
 * @return Layer (0-7)
 * 
 * Complexity: O(1)
 * 
 * Layer selection:
 *   Layer 0: magnitude < 83        (< 1000)
 *   Layer 1: magnitude < 83,333    (< 1M)
 *   Layer 2: magnitude < 83,333,333 (< 1B)
 *   etc.
 */
uint8_t geo_point_compute_layer(int64_t magnitude);

/**
 * @brief Get magnitude range for layer
 * 
 * @param layer Layer (0-7)
 * @param min_magnitude Output: minimum magnitude
 * @param max_magnitude Output: maximum magnitude
 * 
 * Complexity: O(1)
 */
void geo_point_layer_range(uint8_t layer, int64_t* min_magnitude, int64_t* max_magnitude);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_CORE_POINT_H