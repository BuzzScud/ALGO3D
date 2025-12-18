/**
 * @file angle.c
 * @brief Implementation of GeometricAngle
 * 
 * Angles represent rotations, which are the geometric equivalent of addition/subtraction.
 */

#define _GNU_SOURCE
#include "angle.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static void compute_angle_properties(GeometricAngle* angle) {
    // Normalize degrees to 0-360 range
    while (angle->degrees < 0) {
        angle->degrees += 360.0;
    }
    while (angle->degrees >= 360.0) {
        angle->degrees -= 360.0;
    }
    
    // Compute position (0-11)
    angle->position = (uint8_t)(angle->degrees / GEO_DEGREES_PER_POSITION) % GEO_CLOCK_POSITIONS;
    
    // Compute quadrant (0-3)
    angle->quadrant = (uint8_t)(angle->degrees / 90.0);
    if (angle->quadrant >= 4) angle->quadrant = 3;
    
    // Determine direction
    angle->direction = 1;  // Default clockwise
}

// ============================================================================
// CREATION
// ============================================================================

GeometricAngle geo_angle_from_degrees(double degrees) {
    GeometricAngle angle;
    memset(&angle, 0, sizeof(GeometricAngle));
    
    angle.degrees = degrees;
    angle.numerator = 0;
    angle.denominator = 1;
    angle.is_exact = false;  // Degrees are approximate
    
    compute_angle_properties(&angle);
    
    return angle;
}

GeometricAngle geo_angle_from_position(uint8_t position) {
    GeometricAngle angle;
    memset(&angle, 0, sizeof(GeometricAngle));
    
    position = position % GEO_CLOCK_POSITIONS;
    angle.position = position;
    angle.degrees = position * GEO_DEGREES_PER_POSITION;
    angle.quadrant = position / 3;
    angle.direction = 1;
    
    // Exact representation
    angle.numerator = position;
    angle.denominator = GEO_CLOCK_POSITIONS;
    angle.is_exact = true;
    
    return angle;
}

GeometricAngle geo_angle_from_point(const GeometricPoint* point) {
    return geo_angle_from_position(point->position);
}

GeometricAngle geo_angle_from_radians(double radians) {
    double degrees = radians * 180.0 / M_PI;
    return geo_angle_from_degrees(degrees);
}

// ============================================================================
// CONVERSION
// ============================================================================

double geo_angle_to_degrees(const GeometricAngle* angle) {
    return angle->degrees;
}

double geo_angle_to_radians(const GeometricAngle* angle) {
    return angle->degrees * M_PI / 180.0;
}

uint8_t geo_angle_to_position(const GeometricAngle* angle) {
    return angle->position;
}

// ============================================================================
// ROTATION OPERATIONS (Addition/Subtraction)
// ============================================================================

GeometricPoint geo_rotate(const GeometricPoint* point, const GeometricAngle* angle) {
    // O(1) - This is geometric ADDITION
    
    // Rotate position (mod 12)
    uint8_t new_position = (point->position + angle->position) % GEO_CLOCK_POSITIONS;
    
    // Magnitude stays the same for pure rotation
    // But if we wrap around (position + angle >= 12), we carry to magnitude
    int64_t new_magnitude = point->magnitude;
    if (point->position + angle->position >= GEO_CLOCK_POSITIONS) {
        new_magnitude += 1;  // Carry
    }
    
    GeometricPoint result = geo_point_create(new_position, new_magnitude);
    
    // Preserve fractional part
    result.fractional_numerator = point->fractional_numerator;
    result.fractional_denominator = point->fractional_denominator;
    result.is_exact = point->is_exact;
    
    return result;
}

GeometricPoint geo_rotate_reverse(const GeometricPoint* point, const GeometricAngle* angle) {
    // O(1) - This is geometric SUBTRACTION
    
    int new_position = point->position - angle->position;
    int64_t new_magnitude = point->magnitude;
    
    // Handle borrow
    if (new_position < 0) {
        new_position += GEO_CLOCK_POSITIONS;
        new_magnitude -= 1;
    }
    
    GeometricPoint result = geo_point_create((uint8_t)new_position, new_magnitude);
    
    // Preserve fractional part
    result.fractional_numerator = point->fractional_numerator;
    result.fractional_denominator = point->fractional_denominator;
    result.is_exact = point->is_exact;
    
    return result;
}

GeometricPoint geo_rotate_by_positions(const GeometricPoint* point, int8_t positions) {
    if (positions >= 0) {
        GeometricAngle angle = geo_angle_from_position((uint8_t)positions);
        return geo_rotate(point, &angle);
    } else {
        GeometricAngle angle = geo_angle_from_position((uint8_t)(-positions));
        return geo_rotate_reverse(point, &angle);
    }
}

// ============================================================================
// ANGLE ARITHMETIC
// ============================================================================

GeometricAngle geo_angle_add(const GeometricAngle* a, const GeometricAngle* b) {
    // O(1) - add two angles
    double sum_degrees = a->degrees + b->degrees;
    return geo_angle_from_degrees(sum_degrees);
}

GeometricAngle geo_angle_subtract(const GeometricAngle* a, const GeometricAngle* b) {
    // O(1) - subtract two angles
    double diff_degrees = a->degrees - b->degrees;
    return geo_angle_from_degrees(diff_degrees);
}

GeometricAngle geo_angle_negate(const GeometricAngle* angle) {
    // O(1) - reverse direction
    GeometricAngle negated = *angle;
    negated.direction = -angle->direction;
    negated.degrees = 360.0 - angle->degrees;
    compute_angle_properties(&negated);
    return negated;
}

GeometricAngle geo_angle_normalize(const GeometricAngle* angle) {
    // O(1) - normalize to 0-360 range
    GeometricAngle normalized = *angle;
    compute_angle_properties(&normalized);
    return normalized;
}

// ============================================================================
// ANGLE PROPERTIES
// ============================================================================

GeometricAngle geo_angle_shortest_rotation(const GeometricAngle* from, const GeometricAngle* to) {
    // O(1) - shortest rotation between two angles
    
    double diff = to->degrees - from->degrees;
    
    // Normalize to [-180, 180]
    while (diff > 180.0) diff -= 360.0;
    while (diff < -180.0) diff += 360.0;
    
    return geo_angle_from_degrees(diff);
}

double geo_angle_distance(const GeometricAngle* a, const GeometricAngle* b) {
    // O(1) - angular distance (always positive)
    
    double diff = fabs(a->degrees - b->degrees);
    
    // Take shorter path
    if (diff > 180.0) {
        diff = 360.0 - diff;
    }
    
    return diff;
}

bool geo_angle_in_quadrant(const GeometricAngle* angle, uint8_t quadrant) {
    return angle->quadrant == quadrant;
}

// ============================================================================
// UTILITIES
// ============================================================================

void geo_angle_print(const GeometricAngle* angle) {
    printf("Angle(%.1f°) [pos=%u, quad=%u, dir=%d]",
           angle->degrees, angle->position, angle->quadrant, angle->direction);
    
    if (angle->is_exact) {
        printf(" [EXACT: %d/%d]", angle->numerator, angle->denominator);
    }
    
    printf("\n");
}

bool geo_angle_validate(const GeometricAngle* angle) {
    // Check degrees in range
    if (angle->degrees < 0.0 || angle->degrees >= 360.0) {
        return false;
    }
    
    // Check position in range
    if (angle->position >= GEO_CLOCK_POSITIONS) {
        return false;
    }
    
    // Check quadrant in range
    if (angle->quadrant >= 4) {
        return false;
    }
    
    // Check direction is valid
    if (angle->direction != 1 && angle->direction != -1) {
        return false;
    }
    
    // Check denominator not zero
    if (angle->denominator == 0) {
        return false;
    }
    
    return true;
}