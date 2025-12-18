/**
 * @file angle.h
 * @brief GeometricAngle - Rotation Operations
 * 
 * In geometric computation, addition and subtraction are rotations.
 * 
 * Key Insight: Adding numbers is rotating positions on the clock.
 *   5 + 3 = rotate position 5 by 3 positions = position 8
 * 
 * This is why addition is O(1) - it's just modular arithmetic!
 */

#ifndef GEOMETRIC_CORE_ANGLE_H
#define GEOMETRIC_CORE_ANGLE_H

#include "point.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// GEOMETRIC ANGLE
// ============================================================================

/**
 * @brief An angle representing rotation
 * 
 * Angles are used for rotation operations (addition/subtraction).
 * Every angle corresponds to a position on the clock.
 */
typedef struct {
    // Angle in degrees (0-360°)
    double degrees;
    
    // Position on clock (0-11)
    uint8_t position;
    
    // Quadrant (0-3)
    uint8_t quadrant;
    
    // Direction (1 = clockwise, -1 = counter-clockwise)
    int8_t direction;
    
    // Exact representation (for rational angles)
    int32_t numerator;      // For exact fractions
    int32_t denominator;
    bool is_exact;
    
} GeometricAngle;

// ============================================================================
// CREATION
// ============================================================================

/**
 * @brief Create angle from degrees
 * 
 * @param degrees Angle in degrees (0-360°)
 * @return Geometric angle
 * 
 * Complexity: O(1)
 */
GeometricAngle geo_angle_from_degrees(double degrees);

/**
 * @brief Create angle from position
 * 
 * @param position Position on clock (0-11)
 * @return Geometric angle
 * 
 * Complexity: O(1)
 * 
 * Formula: degrees = position × 30°
 */
GeometricAngle geo_angle_from_position(uint8_t position);

/**
 * @brief Create angle from point
 * 
 * @param point Geometric point
 * @return Geometric angle
 * 
 * Complexity: O(1)
 */
GeometricAngle geo_angle_from_point(const GeometricPoint* point);

/**
 * @brief Create angle from radians
 * 
 * @param radians Angle in radians (0-2π)
 * @return Geometric angle
 * 
 * Complexity: O(1)
 */
GeometricAngle geo_angle_from_radians(double radians);

// ============================================================================
// CONVERSION
// ============================================================================

/**
 * @brief Convert angle to degrees
 * 
 * @param angle Geometric angle
 * @return Degrees (0-360°)
 * 
 * Complexity: O(1)
 */
double geo_angle_to_degrees(const GeometricAngle* angle);

/**
 * @brief Convert angle to radians
 * 
 * @param angle Geometric angle
 * @return Radians (0-2π)
 * 
 * Complexity: O(1)
 */
double geo_angle_to_radians(const GeometricAngle* angle);

/**
 * @brief Convert angle to position
 * 
 * @param angle Geometric angle
 * @return Position (0-11)
 * 
 * Complexity: O(1)
 */
uint8_t geo_angle_to_position(const GeometricAngle* angle);

// ============================================================================
// ROTATION OPERATIONS (Addition/Subtraction)
// ============================================================================

/**
 * @brief Rotate point by angle (ADDITION)
 * 
 * @param point Point to rotate
 * @param angle Angle to rotate by
 * @return Rotated point
 * 
 * Complexity: O(1)
 * 
 * This is geometric addition:
 *   a + b = rotate(a, angle_from(b))
 * 
 * Example:
 *   5 + 3 = rotate position 5 by 3 positions = position 8
 */
GeometricPoint geo_rotate(const GeometricPoint* point, const GeometricAngle* angle);

/**
 * @brief Rotate point by angle in reverse (SUBTRACTION)
 * 
 * @param point Point to rotate
 * @param angle Angle to rotate by (reversed)
 * @return Rotated point
 * 
 * Complexity: O(1)
 * 
 * This is geometric subtraction:
 *   a - b = rotate_reverse(a, angle_from(b))
 * 
 * Example:
 *   8 - 3 = rotate position 8 by -3 positions = position 5
 */
GeometricPoint geo_rotate_reverse(const GeometricPoint* point, const GeometricAngle* angle);

/**
 * @brief Rotate by specific number of positions
 * 
 * @param point Point to rotate
 * @param positions Number of positions to rotate (can be negative)
 * @return Rotated point
 * 
 * Complexity: O(1)
 */
GeometricPoint geo_rotate_by_positions(const GeometricPoint* point, int8_t positions);

// ============================================================================
// ANGLE ARITHMETIC
// ============================================================================

/**
 * @brief Add two angles
 * 
 * @param a First angle
 * @param b Second angle
 * @return Sum of angles
 * 
 * Complexity: O(1)
 */
GeometricAngle geo_angle_add(const GeometricAngle* a, const GeometricAngle* b);

/**
 * @brief Subtract two angles
 * 
 * @param a First angle
 * @param b Second angle
 * @return Difference of angles
 * 
 * Complexity: O(1)
 */
GeometricAngle geo_angle_subtract(const GeometricAngle* a, const GeometricAngle* b);

/**
 * @brief Negate angle (reverse direction)
 * 
 * @param angle Angle to negate
 * @return Negated angle
 * 
 * Complexity: O(1)
 */
GeometricAngle geo_angle_negate(const GeometricAngle* angle);

/**
 * @brief Normalize angle to 0-360° range
 * 
 * @param angle Angle to normalize
 * @return Normalized angle
 * 
 * Complexity: O(1)
 */
GeometricAngle geo_angle_normalize(const GeometricAngle* angle);

// ============================================================================
// ANGLE PROPERTIES
// ============================================================================

/**
 * @brief Get shortest rotation between two angles
 * 
 * @param from Starting angle
 * @param to Target angle
 * @return Shortest rotation angle
 * 
 * Complexity: O(1)
 * 
 * Returns angle in range [-180°, 180°]
 */
GeometricAngle geo_angle_shortest_rotation(const GeometricAngle* from, const GeometricAngle* to);

/**
 * @brief Get distance between two angles
 * 
 * @param a First angle
 * @param b Second angle
 * @return Angular distance (0-180°)
 * 
 * Complexity: O(1)
 */
double geo_angle_distance(const GeometricAngle* a, const GeometricAngle* b);

/**
 * @brief Check if angle is in specific quadrant
 * 
 * @param angle Angle to check
 * @param quadrant Quadrant (0-3)
 * @return True if in quadrant
 * 
 * Complexity: O(1)
 */
bool geo_angle_in_quadrant(const GeometricAngle* angle, uint8_t quadrant);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * @brief Print angle
 * 
 * @param angle Angle to print
 * 
 * Complexity: O(1)
 */
void geo_angle_print(const GeometricAngle* angle);

/**
 * @brief Validate angle
 * 
 * @param angle Angle to validate
 * @return True if valid
 * 
 * Complexity: O(1)
 */
bool geo_angle_validate(const GeometricAngle* angle);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_CORE_ANGLE_H