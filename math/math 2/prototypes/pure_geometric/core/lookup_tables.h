/**
 * @file lookup_tables.h
 * @brief Pre-computed geometric lookup tables - NO COMPUTATION
 * 
 * This file contains ALL geometric relationships pre-computed.
 * Operations are O(1) lookups, not calculations.
 * 
 * Key principle: If you can pre-compute it, you should.
 */

#ifndef PURE_GEOMETRIC_LOOKUP_TABLES_H
#define PURE_GEOMETRIC_LOOKUP_TABLES_H

#include "geometric_types.h"

// ============================================================================
// DISTANCE MATRIX (12×12)
// ============================================================================

/**
 * @brief Shortest distance between any two positions on the clock
 * 
 * This is the fundamental lookup table. Distance is measured as the
 * minimum number of steps around the clock (forward or backward).
 * 
 * Example: Distance from 0 to 6 is 6 (half the clock)
 * Example: Distance from 0 to 11 is 1 (wrap around)
 * 
 * O(1) lookup - no computation needed!
 */
extern const uint8_t DISTANCE_TABLE[NUM_POSITIONS][NUM_POSITIONS];

// ============================================================================
// ANGLE MATRIX (12×12)
// ============================================================================

/**
 * @brief Angle between any two positions (in 30° increments)
 * 
 * Each position is 30° apart on the clock.
 * Position 0 = 0°, Position 1 = 30°, Position 2 = 60°, etc.
 * 
 * This table stores the angle difference between any two positions.
 * Values are in degrees (0, 30, 60, 90, 120, 150, 180).
 * 
 * O(1) lookup - no trigonometry needed!
 */
extern const uint16_t ANGLE_TABLE[NUM_POSITIONS][NUM_POSITIONS];

// ============================================================================
// NEIGHBOR MATRIX (12×5)
// ============================================================================

/**
 * @brief The 5 nearest neighbors for each position
 * 
 * In 3D sphere packing, each sphere touches exactly 5 others (kissing number).
 * This table pre-computes which positions are neighbors.
 * 
 * For icosahedron vertices, each vertex has exactly 5 neighbors.
 * 
 * O(1) lookup - no distance calculation needed!
 */
extern const uint8_t NEIGHBOR_TABLE[NUM_POSITIONS][5];

// ============================================================================
// LAYER SCALE TABLE (8 Layers)
// ============================================================================

/**
 * @brief Exact magnitude scales for each layer
 * 
 * Layer 0: 10^0  = 1
 * Layer 1: 10^3  = 1,000
 * Layer 2: 10^6  = 1,000,000
 * Layer 3: 10^9  = 1,000,000,000
 * Layer 4: 10^12 = 1,000,000,000,000
 * Layer 5: 10^15 = 1,000,000,000,000,000
 * Layer 6: 10^18 = 1,000,000,000,000,000,000
 * Layer 7: 10^21 = 1,000,000,000,000,000,000,000
 * 
 * O(1) lookup - no pow() needed!
 */
extern const uint64_t LAYER_SCALE_TABLE[NUM_LAYERS];

// ============================================================================
// POSITION ANGLE TABLE (12 Positions)
// ============================================================================

/**
 * @brief Exact angle for each position (in degrees)
 * 
 * Position 0 = 0°
 * Position 1 = 30°
 * Position 2 = 60°
 * ...
 * Position 11 = 330°
 * 
 * O(1) lookup - no multiplication needed!
 */
extern const uint16_t POSITION_ANGLE_TABLE[NUM_POSITIONS];

// ============================================================================
// PLATONIC SOLID VERTICES (All 5 Solids)
// ============================================================================

/**
 * @brief Pre-computed vertices for all 5 platonic solids
 * 
 * All coordinates are exact rationals (no floating point).
 * These are used as coordinate frames for each layer.
 */

// Tetrahedron: 4 vertices
extern const GeometricVertex TETRAHEDRON_VERTICES[4];

// Cube: 8 vertices
extern const GeometricVertex CUBE_VERTICES[8];

// Octahedron: 6 vertices
extern const GeometricVertex OCTAHEDRON_VERTICES[6];

// Dodecahedron: 20 vertices
extern const GeometricVertex DODECAHEDRON_VERTICES[20];

// Icosahedron: 12 vertices (most important for threading)
extern const GeometricVertex ICOSAHEDRON_VERTICES[12];

// ============================================================================
// LAYER TO PLATONIC SOLID MAPPING
// ============================================================================

/**
 * @brief Which platonic solid to use for each layer
 * 
 * Layer 0: Tetrahedron (4 vertices)
 * Layer 1: Cube (8 vertices)
 * Layer 2: Octahedron (6 vertices)
 * Layer 3: Dodecahedron (20 vertices)
 * Layer 4: Icosahedron (12 vertices)
 * Layers 5-7: Repeat pattern
 */
extern const PlatonicSolidType LAYER_PLATONIC_TABLE[NUM_LAYERS];

// ============================================================================
// PRIME POSITION TABLE
// ============================================================================

/**
 * @brief Which positions are prime (1, 5, 7, 11)
 * 
 * Prime positions have special properties in the geometric system.
 * Pre-computed for O(1) lookup.
 */
extern const bool PRIME_POSITION_TABLE[NUM_POSITIONS];

// ============================================================================
// ROTATION TABLE (12×12)
// ============================================================================

/**
 * @brief Result of rotating position A by position B
 * 
 * This is addition on the clock: (A + B) % 12
 * Pre-computed for O(1) lookup.
 * 
 * Example: ROTATION_TABLE[5][3] = 8 (position 5 rotated by 3 = position 8)
 */
extern const uint8_t ROTATION_TABLE[NUM_POSITIONS][NUM_POSITIONS];

// ============================================================================
// INVERSE ROTATION TABLE (12×12)
// ============================================================================

/**
 * @brief Result of rotating position A by -B (subtraction)
 * 
 * This is subtraction on the clock: (A - B + 12) % 12
 * Pre-computed for O(1) lookup.
 * 
 * Example: INVERSE_ROTATION_TABLE[8][3] = 5 (position 8 rotated by -3 = position 5)
 */
extern const uint8_t INVERSE_ROTATION_TABLE[NUM_POSITIONS][NUM_POSITIONS];

// ============================================================================
// MIDPOINT TABLE (12×12)
// ============================================================================

/**
 * @brief Midpoint between any two positions on the clock
 * 
 * Pre-computed for O(1) lookup.
 * Handles wrap-around correctly.
 * 
 * Example: MIDPOINT_TABLE[0][6] = 3 or 9 (both are valid midpoints)
 */
extern const uint8_t MIDPOINT_TABLE[NUM_POSITIONS][NUM_POSITIONS];

// ============================================================================
// HELPER FUNCTIONS (Still O(1), but with simple logic)
// ============================================================================

/**
 * @brief Get distance between two positions
 * @param pos1 First position (0-11)
 * @param pos2 Second position (0-11)
 * @return Shortest distance (0-6)
 */
static inline uint8_t geo_lookup_distance(uint8_t pos1, uint8_t pos2) {
    return DISTANCE_TABLE[pos1 % NUM_POSITIONS][pos2 % NUM_POSITIONS];
}

/**
 * @brief Get angle between two positions
 * @param pos1 First position (0-11)
 * @param pos2 Second position (0-11)
 * @return Angle in degrees (0-180)
 */
static inline uint16_t geo_lookup_angle(uint8_t pos1, uint8_t pos2) {
    return ANGLE_TABLE[pos1 % NUM_POSITIONS][pos2 % NUM_POSITIONS];
}

/**
 * @brief Rotate position by offset
 * @param pos Position to rotate (0-11)
 * @param offset Rotation amount (0-11)
 * @return Rotated position (0-11)
 */
static inline uint8_t geo_lookup_rotate(uint8_t pos, uint8_t offset) {
    return ROTATION_TABLE[pos % NUM_POSITIONS][offset % NUM_POSITIONS];
}

/**
 * @brief Get layer scale
 * @param layer Layer number (0-7)
 * @return Scale factor (10^(3*layer))
 */
static inline uint64_t geo_lookup_layer_scale(uint8_t layer) {
    return LAYER_SCALE_TABLE[layer % NUM_LAYERS];
}

/**
 * @brief Check if position is prime
 * @param pos Position to check (0-11)
 * @return true if prime (1, 5, 7, 11)
 */
static inline bool geo_lookup_is_prime(uint8_t pos) {
    return PRIME_POSITION_TABLE[pos % NUM_POSITIONS];
}

/**
 * @brief Get neighbors of a position
 * @param pos Position (0-11)
 * @param neighbors Output array (must have space for 5 elements)
 */
static inline void geo_lookup_neighbors(uint8_t pos, uint8_t neighbors[5]) {
    pos = pos % NUM_POSITIONS;
    for (int i = 0; i < 5; i++) {
        neighbors[i] = NEIGHBOR_TABLE[pos][i];
    }
}

#endif // PURE_GEOMETRIC_LOOKUP_TABLES_H