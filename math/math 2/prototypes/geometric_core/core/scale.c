/**
 * @file scale.c
 * @brief Implementation of GeometricScale
 * 
 * Scales represent magnitude changes, which are the geometric equivalent of multiplication/division.
 */

#define _GNU_SOURCE
#include "scale.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static int8_t compute_layer_shift(int64_t factor) {
    // Compute how many layers to shift based on factor
    // Each layer represents ~3 orders of magnitude
    
    if (factor < 1000) return 0;
    if (factor < 1000000) return 1;
    if (factor < 1000000000) return 2;
    if (factor < 1000000000000LL) return 3;
    return 4;
}

// ============================================================================
// CREATION
// ============================================================================

GeometricScale geo_scale_from_int(int64_t factor) {
    GeometricScale scale;
    memset(&scale, 0, sizeof(GeometricScale));
    
    scale.factor = factor;
    scale.layer_shift = compute_layer_shift(factor > 0 ? factor : -factor);
    scale.numerator = factor;
    scale.denominator = 1;
    scale.is_exact = true;
    scale.direction = (factor >= 0) ? 1 : -1;
    
    return scale;
}

GeometricScale geo_scale_from_double(double factor) {
    GeometricScale scale;
    memset(&scale, 0, sizeof(GeometricScale));
    
    scale.factor = (int64_t)factor;
    scale.layer_shift = compute_layer_shift(scale.factor > 0 ? scale.factor : -scale.factor);
    
    // Convert to rational approximation
    scale.numerator = (int64_t)(factor * 1000000);
    scale.denominator = 1000000;
    scale.is_exact = (fabs(factor - (double)scale.factor) < 1e-10);
    scale.direction = (factor >= 0) ? 1 : -1;
    
    return scale;
}

GeometricScale geo_scale_from_point(const GeometricPoint* point) {
    int64_t value = geo_point_to_int(point);
    return geo_scale_from_int(value);
}

GeometricScale geo_scale_from_fraction(int64_t numerator, int64_t denominator) {
    GeometricScale scale;
    memset(&scale, 0, sizeof(GeometricScale));
    
    if (denominator == 0) {
        // Invalid - return identity
        return geo_scale_identity();
    }
    
    scale.factor = numerator / denominator;
    scale.layer_shift = compute_layer_shift(scale.factor > 0 ? scale.factor : -scale.factor);
    scale.numerator = numerator;
    scale.denominator = denominator;
    scale.is_exact = true;
    scale.direction = ((numerator >= 0) == (denominator >= 0)) ? 1 : -1;
    
    return scale;
}

GeometricScale geo_scale_identity(void) {
    return geo_scale_from_int(1);
}

GeometricScale geo_scale_inverse(const GeometricScale* scale) {
    // O(1) - invert scale (for division)
    return geo_scale_from_fraction(scale->denominator, scale->numerator);
}

// ============================================================================
// CONVERSION
// ============================================================================

double geo_scale_to_double(const GeometricScale* scale) {
    if (scale->denominator == 0) return 0.0;
    return (double)scale->numerator / (double)scale->denominator;
}

int64_t geo_scale_to_int(const GeometricScale* scale) {
    return scale->factor;
}

// ============================================================================
// SCALING OPERATIONS (Multiplication/Division)
// ============================================================================

GeometricPoint geo_scale_up(const GeometricPoint* point, const GeometricScale* scale) {
    // O(1) - This is geometric MULTIPLICATION
    
    // Get the value
    int64_t value = geo_point_to_int(point);
    
    // Scale it
    int64_t scaled_value = value * scale->factor;
    
    // Convert back to geometric point
    GeometricPoint result = geo_point_from_int(scaled_value);
    
    // Handle fractional part
    if (!point->is_exact || !scale->is_exact) {
        // Multiply fractions
        result.fractional_numerator = point->fractional_numerator * scale->numerator;
        result.fractional_denominator = point->fractional_denominator * scale->denominator;
        result.is_exact = false;
    }
    
    return result;
}

GeometricPoint geo_scale_down(const GeometricPoint* point, const GeometricScale* scale) {
    // O(1) - This is geometric DIVISION
    
    if (scale->factor == 0) {
        // Division by zero → infinity
        return geo_point_infinity();
    }
    
    // Get the value
    int64_t value = geo_point_to_int(point);
    
    // Scale down
    int64_t scaled_value = value / scale->factor;
    
    // Convert back to geometric point
    GeometricPoint result = geo_point_from_int(scaled_value);
    
    // Handle fractional part (remainder becomes fraction)
    int64_t remainder = value % scale->factor;
    if (remainder != 0) {
        result.fractional_numerator = remainder;
        result.fractional_denominator = scale->factor;
        result.is_exact = false;
    }
    
    return result;
}

GeometricPoint geo_scale_by_int(const GeometricPoint* point, int64_t factor) {
    GeometricScale scale = geo_scale_from_int(factor);
    return geo_scale_up(point, &scale);
}

GeometricPoint geo_divide_by_int(const GeometricPoint* point, int64_t divisor) {
    GeometricScale scale = geo_scale_from_int(divisor);
    return geo_scale_down(point, &scale);
}

// ============================================================================
// SCALE ARITHMETIC
// ============================================================================

GeometricScale geo_scale_multiply(const GeometricScale* a, const GeometricScale* b) {
    // O(1) - multiply two scales
    
    GeometricScale result;
    result.factor = a->factor * b->factor;
    result.layer_shift = a->layer_shift + b->layer_shift;
    result.numerator = a->numerator * b->numerator;
    result.denominator = a->denominator * b->denominator;
    result.is_exact = a->is_exact && b->is_exact;
    result.direction = a->direction * b->direction;
    
    return result;
}

GeometricScale geo_scale_divide(const GeometricScale* a, const GeometricScale* b) {
    // O(1) - divide two scales
    
    if (b->factor == 0) {
        // Division by zero
        return geo_scale_identity();
    }
    
    GeometricScale result;
    result.factor = a->factor / b->factor;
    result.layer_shift = a->layer_shift - b->layer_shift;
    result.numerator = a->numerator * b->denominator;
    result.denominator = a->denominator * b->numerator;
    result.is_exact = a->is_exact && b->is_exact;
    result.direction = a->direction * b->direction;
    
    return result;
}

// ============================================================================
// SCALE PROPERTIES
// ============================================================================

bool geo_scale_is_identity(const GeometricScale* scale) {
    return (scale->factor == 1 && scale->numerator == scale->denominator);
}

bool geo_scale_is_zero(const GeometricScale* scale) {
    return (scale->factor == 0 || scale->numerator == 0);
}

bool geo_scale_is_exact(const GeometricScale* scale) {
    return scale->is_exact;
}

// ============================================================================
// UTILITIES
// ============================================================================

void geo_scale_print(const GeometricScale* scale) {
    printf("Scale(×%ld) [shift=%d, dir=%d]", 
           scale->factor, scale->layer_shift, scale->direction);
    
    if (scale->is_exact && scale->denominator != 1) {
        printf(" [EXACT: %ld/%ld]", scale->numerator, scale->denominator);
    }
    
    printf("\n");
}

bool geo_scale_validate(const GeometricScale* scale) {
    // Check denominator not zero
    if (scale->denominator == 0) {
        return false;
    }
    
    // Check direction is valid
    if (scale->direction != 1 && scale->direction != -1) {
        return false;
    }
    
    // Check layer shift is reasonable
    if (scale->layer_shift < -8 || scale->layer_shift > 8) {
        return false;
    }
    
    return true;
}