/**
 * @file position_ops.c
 * @brief Implementation of pure geometric position operations
 * 
 * NO MATH.H - All operations use lookup tables or exact arithmetic.
 */

#include "position_ops.h"
#include <stdlib.h>
#include <stdint.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Calculate GCD for rational simplification
 */
static int64_t gcd(int64_t a, int64_t b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// ============================================================================
// POSITION CREATION
// ============================================================================

GeometricPosition geo_position_create(uint64_t value) {
    GeometricPosition pos;
    
    // Position is value % 12 (where on the clock)
    pos.position = value % NUM_POSITIONS;
    
    // Magnitude is the exact value (stored in abacus)
    pos.magnitude = abacus_from_uint64(value, 60, 32);  // Base-60, 32 digits precision
    
    // Layer is determined by magnitude
    pos.layer = geo_select_layer(value);
    
    return pos;
}

GeometricPosition geo_position_create_exact(uint8_t position, CrystallineAbacus* magnitude) {
    GeometricPosition pos;
    pos.position = position % NUM_POSITIONS;
    pos.magnitude = abacus_copy(magnitude);
    pos.layer = geo_select_layer_from_abacus(magnitude);
    return pos;
}

void geo_position_free(GeometricPosition* pos) {
    if (pos && pos->magnitude) {
        abacus_free(pos->magnitude);
        pos->magnitude = NULL;
    }
}

uint64_t geo_position_to_uint64(const GeometricPosition* pos) {
    return abacus_to_uint64(pos->magnitude);
}

// ============================================================================
// POSITION ARITHMETIC (Pure Geometric)
// ============================================================================

GeometricPosition geo_add(const GeometricPosition* a, const GeometricPosition* b) {
    GeometricPosition result;
    
    // Addition is rotation on the clock - O(1) lookup
    result.position = geo_lookup_rotate(a->position, b->position);
    
    // Magnitude is exact addition via abacus
    result.magnitude = abacus_add(a->magnitude, b->magnitude);
    
    // Layer is determined by new magnitude
    result.layer = geo_select_layer_from_abacus(result.magnitude);
    
    return result;
}

GeometricPosition geo_subtract(const GeometricPosition* a, const GeometricPosition* b) {
    GeometricPosition result;
    
    // Subtraction is inverse rotation - O(1) lookup
    result.position = INVERSE_ROTATION_TABLE[a->position][b->position];
    
    // Magnitude is exact subtraction via abacus
    result.magnitude = abacus_subtract(a->magnitude, b->magnitude);
    
    // Layer is determined by new magnitude
    result.layer = geo_select_layer_from_abacus(result.magnitude);
    
    return result;
}

GeometricPosition geo_multiply(const GeometricPosition* pos, uint64_t scalar) {
    GeometricPosition result;
    
    // Position stays the same (multiplication is scaling, not rotation)
    result.position = pos->position;
    
    // Magnitude is exact multiplication via abacus
    result.magnitude = abacus_multiply_uint64(pos->magnitude, scalar);
    
    // Layer is determined by new magnitude
    result.layer = geo_select_layer_from_abacus(result.magnitude);
    
    return result;
}

GeometricPosition geo_multiply_positions(const GeometricPosition* a, const GeometricPosition* b) {
    GeometricPosition result;
    
    // Position stays the same as first operand
    result.position = a->position;
    
    // Magnitude is exact multiplication via abacus
    result.magnitude = abacus_multiply(a->magnitude, b->magnitude);
    
    // Layer is determined by new magnitude
    result.layer = geo_select_layer_from_abacus(result.magnitude);
    
    return result;
}

// ============================================================================
// MIDPOINT
// ============================================================================

GeometricPosition geo_midpoint(const GeometricPosition* a, const GeometricPosition* b) {
    GeometricPosition result;
    
    // Position is midpoint lookup - O(1)
    result.position = MIDPOINT_TABLE[a->position][b->position];
    
    // Magnitude is average (add and divide by 2)
    CrystallineAbacus* sum = abacus_add(a->magnitude, b->magnitude);
    result.magnitude = abacus_multiply_uint64(sum, 1);  // For now, just copy
    abacus_free(sum);
    
    // TODO: Implement proper division by 2 in abacus
    // For now, this is approximate
    
    // Layer is determined by magnitude
    result.layer = geo_select_layer_from_abacus(result.magnitude);
    
    return result;
}

// ============================================================================
// LAYER OPERATIONS
// ============================================================================

uint8_t geo_select_layer(uint64_t magnitude) {
    // O(1) comparison against layer boundaries
    if (magnitude < 1000ULL) return 0;                    // 10^0 to 10^3
    if (magnitude < 1000000ULL) return 1;                 // 10^3 to 10^6
    if (magnitude < 1000000000ULL) return 2;              // 10^6 to 10^9
    if (magnitude < 1000000000000ULL) return 3;           // 10^9 to 10^12
    if (magnitude < 1000000000000000ULL) return 4;        // 10^12 to 10^15
    if (magnitude < 1000000000000000000ULL) return 5;     // 10^15 to 10^18
    return 6;  // 10^18 and above (layer 6 or 7)
}

uint8_t geo_select_layer_from_abacus(const CrystallineAbacus* magnitude) {
    // Convert to uint64 for comparison (if it fits)
    uint64_t value = abacus_to_uint64(magnitude);
    return geo_select_layer(value);
}

GeometricPosition geo_scale_to_layer(const GeometricPosition* pos, uint8_t target_layer) {
    GeometricPosition result;
    
    // Position stays the same
    result.position = pos->position;
    
    // Get scale factor from lookup table - O(1)
    uint64_t scale = geo_lookup_layer_scale(target_layer);
    
    // Scale magnitude
    result.magnitude = abacus_multiply_uint64(pos->magnitude, scale);
    
    // Set target layer
    result.layer = target_layer;
    
    return result;
}

// ============================================================================
// RATIONAL OPERATIONS (Exact Arithmetic)
// ============================================================================

Rational rational_add(Rational a, Rational b) {
    // a/b + c/d = (ad + bc) / (bd)
    Rational result;
    result.numerator = a.numerator * b.denominator + b.numerator * a.denominator;
    result.denominator = a.denominator * b.denominator;
    return rational_simplify(result);
}

Rational rational_multiply(Rational a, Rational b) {
    // (a/b) * (c/d) = (ac) / (bd)
    Rational result;
    result.numerator = a.numerator * b.numerator;
    result.denominator = a.denominator * b.denominator;
    return rational_simplify(result);
}

Rational rational_simplify(Rational r) {
    int64_t g = gcd(r.numerator, r.denominator);
    if (g > 1) {
        r.numerator /= g;
        r.denominator /= g;
    }
    // Keep denominator positive
    if (r.denominator < 0) {
        r.numerator = -r.numerator;
        r.denominator = -r.denominator;
    }
    return r;
}

uint64_t rational_to_uint64(Rational r) {
    if (r.denominator == 0) return 0;
    return (uint64_t)(r.numerator / r.denominator);
}

// ============================================================================
// VERTEX OPERATIONS (Exact Coordinates)
// ============================================================================

GeometricVertex vertex_add(GeometricVertex a, GeometricVertex b) {
    GeometricVertex result;
    result.x = rational_add(a.x, b.x);
    result.y = rational_add(a.y, b.y);
    result.z = rational_add(a.z, b.z);
    return result;
}

GeometricVertex vertex_scale(GeometricVertex v, Rational scale) {
    GeometricVertex result;
    result.x = rational_multiply(v.x, scale);
    result.y = rational_multiply(v.y, scale);
    result.z = rational_multiply(v.z, scale);
    return result;
}