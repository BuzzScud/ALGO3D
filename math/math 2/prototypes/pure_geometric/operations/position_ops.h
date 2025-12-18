/**
 * @file position_ops.h
 * @brief Pure geometric position operations - NO MATH.H
 * 
 * All operations are O(1) lookups or simple integer arithmetic.
 * No floating point, no trigonometry, no computation.
 */

#ifndef PURE_GEOMETRIC_POSITION_OPS_H
#define PURE_GEOMETRIC_POSITION_OPS_H

#include "../core/geometric_types.h"
#include "../core/lookup_tables.h"

// Forward declaration for CrystallineAbacus operations
// These will be linked from the main codebase
extern CrystallineAbacus* abacus_create(uint32_t base, uint32_t precision);
extern void abacus_free(CrystallineAbacus* abacus);
extern CrystallineAbacus* abacus_from_uint64(uint64_t value, uint32_t base, uint32_t precision);
extern CrystallineAbacus* abacus_add(const CrystallineAbacus* a, const CrystallineAbacus* b);
extern CrystallineAbacus* abacus_subtract(const CrystallineAbacus* a, const CrystallineAbacus* b);
extern CrystallineAbacus* abacus_multiply(const CrystallineAbacus* a, const CrystallineAbacus* b);
extern CrystallineAbacus* abacus_multiply_uint64(const CrystallineAbacus* a, uint64_t b);
extern uint64_t abacus_to_uint64(const CrystallineAbacus* abacus);
extern int abacus_compare(const CrystallineAbacus* a, const CrystallineAbacus* b);
extern CrystallineAbacus* abacus_copy(const CrystallineAbacus* src);

// ============================================================================
// POSITION CREATION
// ============================================================================

/**
 * @brief Create a geometric position from a value
 * @param value The numeric value
 * @return GeometricPosition with position, magnitude, and layer
 * 
 * This is how we convert a "number" into geometric space.
 * Example: 42 = 3×12 + 6 → position 6, magnitude 42, layer 0
 */
GeometricPosition geo_position_create(uint64_t value);

/**
 * @brief Create a geometric position with explicit components
 * @param position Clock position (0-11)
 * @param magnitude Exact magnitude (via abacus)
 * @return GeometricPosition
 */
GeometricPosition geo_position_create_exact(uint8_t position, CrystallineAbacus* magnitude);

/**
 * @brief Free a geometric position
 * @param pos Position to free
 */
void geo_position_free(GeometricPosition* pos);

/**
 * @brief Convert geometric position back to uint64
 * @param pos Position to convert
 * @return Numeric value (if it fits in uint64)
 */
uint64_t geo_position_to_uint64(const GeometricPosition* pos);

// ============================================================================
// POSITION ARITHMETIC (Pure Geometric)
// ============================================================================

/**
 * @brief Add two positions (rotation on clock)
 * @param a First position
 * @param b Second position
 * @return Sum as geometric position
 * 
 * Addition is rotation: (a.position + b.position) % 12
 * Magnitude is exact addition via abacus.
 * O(1) for position, O(1) for abacus addition.
 */
GeometricPosition geo_add(const GeometricPosition* a, const GeometricPosition* b);

/**
 * @brief Subtract two positions (inverse rotation)
 * @param a First position
 * @param b Second position
 * @return Difference as geometric position
 * 
 * Subtraction is inverse rotation: (a.position - b.position + 12) % 12
 * Magnitude is exact subtraction via abacus.
 */
GeometricPosition geo_subtract(const GeometricPosition* a, const GeometricPosition* b);

/**
 * @brief Multiply position by scalar (scaling)
 * @param pos Position to scale
 * @param scalar Scaling factor
 * @return Scaled position
 * 
 * Multiplication is scaling: position stays same, magnitude scales.
 * This is NOT repeated addition - it's a geometric transformation.
 */
GeometricPosition geo_multiply(const GeometricPosition* pos, uint64_t scalar);

/**
 * @brief Multiply two positions
 * @param a First position
 * @param b Second position
 * @return Product as geometric position
 */
GeometricPosition geo_multiply_positions(const GeometricPosition* a, const GeometricPosition* b);

// ============================================================================
// DISTANCE AND ANGLE (Pure Lookup)
// ============================================================================

/**
 * @brief Get distance between two positions
 * @param a First position
 * @param b Second position
 * @return Distance (0-6 steps on clock)
 * 
 * O(1) lookup in DISTANCE_TABLE - no computation!
 */
static inline uint8_t geo_distance(const GeometricPosition* a, const GeometricPosition* b) {
    return geo_lookup_distance(a->position, b->position);
}

/**
 * @brief Get angle between two positions
 * @param a First position
 * @param b Second position
 * @return Angle in degrees (0-180)
 * 
 * O(1) lookup in ANGLE_TABLE - no trigonometry!
 */
static inline uint16_t geo_angle(const GeometricPosition* a, const GeometricPosition* b) {
    return geo_lookup_angle(a->position, b->position);
}

/**
 * @brief Get midpoint between two positions
 * @param a First position
 * @param b Second position
 * @return Midpoint position
 * 
 * O(1) lookup in MIDPOINT_TABLE.
 */
GeometricPosition geo_midpoint(const GeometricPosition* a, const GeometricPosition* b);

// ============================================================================
// LAYER OPERATIONS
// ============================================================================

/**
 * @brief Select layer based on magnitude
 * @param magnitude Value to check
 * @return Layer number (0-7)
 * 
 * O(1) comparison against layer boundaries.
 */
uint8_t geo_select_layer(uint64_t magnitude);

/**
 * @brief Select layer from abacus magnitude
 * @param magnitude Abacus value
 * @return Layer number (0-7)
 */
uint8_t geo_select_layer_from_abacus(const CrystallineAbacus* magnitude);

/**
 * @brief Scale position to different layer
 * @param pos Position to scale
 * @param target_layer Target layer (0-7)
 * @return Scaled position
 * 
 * O(1) layer transition using LAYER_SCALE_TABLE.
 */
GeometricPosition geo_scale_to_layer(const GeometricPosition* pos, uint8_t target_layer);

// ============================================================================
// POSITION PROPERTIES (Pure Lookup)
// ============================================================================

/**
 * @brief Check if position is prime
 * @param pos Position to check
 * @return true if prime (1, 5, 7, 11)
 * 
 * O(1) lookup in PRIME_POSITION_TABLE.
 */
static inline bool geo_is_prime(const GeometricPosition* pos) {
    return geo_lookup_is_prime(pos->position);
}

/**
 * @brief Check if position is control
 * @param pos Position to check
 * @return true if control (0 or 12)
 */
static inline bool geo_is_control(const GeometricPosition* pos) {
    return IS_CONTROL_POSITION(pos->position);
}

/**
 * @brief Get neighbors of a position
 * @param pos Position
 * @param neighbors Output array (must have space for 5 elements)
 * 
 * O(1) lookup in NEIGHBOR_TABLE.
 */
static inline void geo_get_neighbors(const GeometricPosition* pos, uint8_t neighbors[5]) {
    geo_lookup_neighbors(pos->position, neighbors);
}

// ============================================================================
// RATIONAL OPERATIONS (Exact Arithmetic)
// ============================================================================

/**
 * @brief Add two rational numbers
 * @param a First rational
 * @param b Second rational
 * @return Sum as rational
 * 
 * Exact addition: a/b + c/d = (ad + bc) / (bd)
 */
Rational rational_add(Rational a, Rational b);

/**
 * @brief Multiply two rational numbers
 * @param a First rational
 * @param b Second rational
 * @return Product as rational
 * 
 * Exact multiplication: (a/b) * (c/d) = (ac) / (bd)
 */
Rational rational_multiply(Rational a, Rational b);

/**
 * @brief Simplify rational number
 * @param r Rational to simplify
 * @return Simplified rational
 * 
 * Reduces fraction to lowest terms using GCD.
 */
Rational rational_simplify(Rational r);

/**
 * @brief Convert rational to approximate uint64
 * @param r Rational to convert
 * @return Approximate integer value
 */
uint64_t rational_to_uint64(Rational r);

// ============================================================================
// VERTEX OPERATIONS (Exact Coordinates)
// ============================================================================

/**
 * @brief Add two vertices
 * @param a First vertex
 * @param b Second vertex
 * @return Sum vertex
 */
GeometricVertex vertex_add(GeometricVertex a, GeometricVertex b);

/**
 * @brief Scale vertex by rational
 * @param v Vertex to scale
 * @param scale Scale factor
 * @return Scaled vertex
 */
GeometricVertex vertex_scale(GeometricVertex v, Rational scale);

#endif // PURE_GEOMETRIC_POSITION_OPS_H