/**
 * @file point.c
 * @brief Implementation of GeometricPoint
 */

#define _GNU_SOURCE
#include "point.h"
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

static void compute_derived_properties(GeometricPoint* point) {
    // Compute angle
    point->angle = point->position * GEO_DEGREES_PER_POSITION;
    
    // Compute quadrant
    point->quadrant = point->position / 3;
    
    // Compute ring
    point->ring = point->position / 3;
    
    // Compute layer
    point->layer = geo_point_compute_layer(point->magnitude);
    
    // Check if prime position
    point->is_prime_position = false;
    for (int i = 0; i < GEO_PRIME_POSITIONS_COUNT; i++) {
        if (point->position == GEO_PRIME_POSITIONS[i]) {
            point->is_prime_position = true;
            break;
        }
    }
    
    // Check if control position
    point->is_control = (point->position == 0);
    
    // Check if negative
    point->is_negative = (point->magnitude < 0);
    
    // Check if zero
    point->is_zero = (point->position == 0 && point->magnitude == 0);
    
    // Check if exact (no fractional part)
    point->is_exact = (point->fractional_numerator == 0);
}

// ============================================================================
// CREATION & INITIALIZATION
// ============================================================================

GeometricPoint geo_point_create(uint8_t position, int64_t magnitude) {
    GeometricPoint point;
    memset(&point, 0, sizeof(GeometricPoint));
    
    point.position = position % GEO_CLOCK_POSITIONS;
    point.magnitude = magnitude;
    point.fractional_numerator = 0;
    point.fractional_denominator = 1;
    
    compute_derived_properties(&point);
    
    return point;
}

GeometricPoint geo_point_from_int(int64_t value) {
    // Convert integer to position and magnitude
    // value = magnitude × 12 + position
    
    int64_t magnitude = value / GEO_CLOCK_POSITIONS;
    int64_t position = value % GEO_CLOCK_POSITIONS;
    
    // Handle negative values
    if (position < 0) {
        position += GEO_CLOCK_POSITIONS;
        magnitude -= 1;
    }
    
    return geo_point_create((uint8_t)position, magnitude);
}

GeometricPoint geo_point_from_double(double value) {
    // Split into integer and fractional parts
    double int_part;
    double frac_part = modf(value, &int_part);
    
    // Create point from integer part
    GeometricPoint point = geo_point_from_int((int64_t)int_part);
    
    // Add fractional part (convert to rational approximation)
    if (fabs(frac_part) > 1e-10) {
        // Simple continued fraction approximation
        // For now, use denominator of 1000000 for precision
        point.fractional_numerator = (int64_t)(frac_part * 1000000);
        point.fractional_denominator = 1000000;
        point.is_exact = false;
    }
    
    return point;
}

GeometricPoint geo_point_zero(void) {
    return geo_point_create(0, 0);
}

GeometricPoint geo_point_one(void) {
    return geo_point_create(1, 0);
}

GeometricPoint geo_point_infinity(void) {
    GeometricPoint point = geo_point_create(0, INT64_MAX);
    point.is_control = true;
    return point;
}

// ============================================================================
// CONVERSION
// ============================================================================

int64_t geo_point_to_int(const GeometricPoint* point) {
    // value = magnitude × 12 + position
    return point->magnitude * GEO_CLOCK_POSITIONS + point->position;
}

double geo_point_to_double(const GeometricPoint* point) {
    double value = (double)geo_point_to_int(point);
    
    // Add fractional part if present
    if (point->fractional_numerator != 0 && point->fractional_denominator != 0) {
        value += (double)point->fractional_numerator / (double)point->fractional_denominator;
    }
    
    return value;
}

double geo_point_angle(const GeometricPoint* point) {
    return point->angle;
}

double geo_point_angle_radians(const GeometricPoint* point) {
    return point->angle * M_PI / 180.0;
}

// ============================================================================
// PROPERTIES
// ============================================================================

bool geo_point_is_prime_position(const GeometricPoint* point) {
    return point->is_prime_position;
}

bool geo_point_is_control(const GeometricPoint* point) {
    return point->is_control;
}

bool geo_point_is_zero(const GeometricPoint* point) {
    return point->is_zero;
}

bool geo_point_is_one(const GeometricPoint* point) {
    return (point->position == 1 && point->magnitude == 0 && 
            point->fractional_numerator == 0);
}

bool geo_point_is_negative(const GeometricPoint* point) {
    return point->is_negative;
}

bool geo_point_is_exact(const GeometricPoint* point) {
    return point->is_exact;
}

// ============================================================================
// COMPARISON
// ============================================================================

int geo_point_compare(const GeometricPoint* a, const GeometricPoint* b) {
    // Compare by converting to integers
    int64_t val_a = geo_point_to_int(a);
    int64_t val_b = geo_point_to_int(b);
    
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    
    // If integer parts equal, compare fractional parts
    if (a->fractional_numerator * b->fractional_denominator < 
        b->fractional_numerator * a->fractional_denominator) {
        return -1;
    }
    if (a->fractional_numerator * b->fractional_denominator > 
        b->fractional_numerator * a->fractional_denominator) {
        return 1;
    }
    
    return 0;
}

bool geo_point_equals(const GeometricPoint* a, const GeometricPoint* b) {
    return geo_point_compare(a, b) == 0;
}

bool geo_point_less_than(const GeometricPoint* a, const GeometricPoint* b) {
    return geo_point_compare(a, b) < 0;
}

bool geo_point_greater_than(const GeometricPoint* a, const GeometricPoint* b) {
    return geo_point_compare(a, b) > 0;
}

// ============================================================================
// UTILITIES
// ============================================================================

GeometricPoint geo_point_copy(const GeometricPoint* src) {
    GeometricPoint copy;
    memcpy(&copy, src, sizeof(GeometricPoint));
    return copy;
}

void geo_point_print(const GeometricPoint* point) {
    int64_t value = geo_point_to_int(point);
    printf("Point(%ld) [pos=%u, mag=%ld, layer=%u, angle=%.1f°]",
           value, point->position, point->magnitude, point->layer, point->angle);
    
    if (point->is_prime_position) printf(" [PRIME]");
    if (point->is_control) printf(" [CONTROL]");
    if (point->is_negative) printf(" [NEG]");
    
    printf("\n");
}

void geo_point_print_detailed(const GeometricPoint* point) {
    printf("GeometricPoint:\n");
    printf("  Value: %ld", geo_point_to_int(point));
    
    if (!point->is_exact) {
        printf(" + %ld/%ld", point->fractional_numerator, point->fractional_denominator);
    }
    printf("\n");
    
    printf("  Position: %u (angle: %.1f°)\n", point->position, point->angle);
    printf("  Magnitude: %ld\n", point->magnitude);
    printf("  Layer: %u\n", point->layer);
    printf("  Quadrant: %u\n", point->quadrant);
    printf("  Ring: %u\n", point->ring);
    
    printf("  Properties:\n");
    printf("    Prime position: %s\n", point->is_prime_position ? "yes" : "no");
    printf("    Control: %s\n", point->is_control ? "yes" : "no");
    printf("    Negative: %s\n", point->is_negative ? "yes" : "no");
    printf("    Zero: %s\n", point->is_zero ? "yes" : "no");
    printf("    Exact: %s\n", point->is_exact ? "yes" : "no");
}

bool geo_point_validate(const GeometricPoint* point) {
    // Check position is in range
    if (point->position >= GEO_CLOCK_POSITIONS) {
        return false;
    }
    
    // Check layer is in range
    if (point->layer >= 8) {
        return false;
    }
    
    // Check quadrant is in range
    if (point->quadrant >= 4) {
        return false;
    }
    
    // Check ring is in range
    if (point->ring >= 4) {
        return false;
    }
    
    // Check fractional denominator is not zero
    if (point->fractional_denominator == 0) {
        return false;
    }
    
    return true;
}

// ============================================================================
// LAYER OPERATIONS
// ============================================================================

uint8_t geo_point_compute_layer(int64_t magnitude) {
    // Compute absolute magnitude
    int64_t abs_mag = (magnitude < 0) ? -magnitude : magnitude;
    
    // Convert to approximate value
    int64_t value = abs_mag * GEO_CLOCK_POSITIONS;
    
    // Layer selection based on magnitude
    // Each layer represents ~3 orders of magnitude
    if (value < 1000LL) return 0;                    // 10^0 to 10^3
    if (value < 1000000LL) return 1;                 // 10^3 to 10^6
    if (value < 1000000000LL) return 2;              // 10^6 to 10^9
    if (value < 1000000000000LL) return 3;           // 10^9 to 10^12
    if (value < 1000000000000000LL) return 4;        // 10^12 to 10^15
    if (value < 1000000000000000000LL) return 5;     // 10^15 to 10^18
    
    // For very large values, use layer 6 or 7
    return (value < 1000000000000000000LL * 1000LL) ? 6 : 7;
}

void geo_point_layer_range(uint8_t layer, int64_t* min_magnitude, int64_t* max_magnitude) {
    // Approximate ranges (in terms of value, not magnitude)
    static const int64_t ranges[8][2] = {
        {0, 1000LL},                              // Layer 0
        {1000LL, 1000000LL},                      // Layer 1
        {1000000LL, 1000000000LL},                // Layer 2
        {1000000000LL, 1000000000000LL},          // Layer 3
        {1000000000000LL, 1000000000000000LL},    // Layer 4
        {1000000000000000LL, 1000000000000000000LL}, // Layer 5
        {1000000000000000000LL, INT64_MAX},       // Layer 6
        {INT64_MAX, INT64_MAX}                    // Layer 7 (overflow)
    };
    
    if (layer < 8) {
        *min_magnitude = ranges[layer][0] / GEO_CLOCK_POSITIONS;
        *max_magnitude = ranges[layer][1] / GEO_CLOCK_POSITIONS;
    } else {
        *min_magnitude = 0;
        *max_magnitude = 0;
    }
}