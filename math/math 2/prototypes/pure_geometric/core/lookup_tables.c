/**
 * @file lookup_tables.c
 * @brief Implementation of all pre-computed lookup tables
 * 
 * All geometric relationships are pre-computed here.
 * No computation happens at runtime - only lookups.
 */

#include "lookup_tables.h"

// ============================================================================
// DISTANCE TABLE (12×12)
// ============================================================================

/**
 * Shortest distance between any two positions on the clock.
 * Distance is minimum of forward and backward paths.
 */
const uint8_t DISTANCE_TABLE[NUM_POSITIONS][NUM_POSITIONS] = {
    // From position 0
    {0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1},
    // From position 1
    {1, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2},
    // From position 2
    {2, 1, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3},
    // From position 3
    {3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 5, 4},
    // From position 4
    {4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 5},
    // From position 5
    {5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6},
    // From position 6
    {6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5},
    // From position 7
    {5, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4},
    // From position 8
    {4, 5, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3},
    // From position 9
    {3, 4, 5, 6, 5, 4, 3, 2, 1, 0, 1, 2},
    // From position 10
    {2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0, 1},
    // From position 11
    {1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0}
};

// ============================================================================
// ANGLE TABLE (12×12)
// ============================================================================

/**
 * Angle between any two positions in degrees.
 * Each position is 30° apart.
 */
const uint16_t ANGLE_TABLE[NUM_POSITIONS][NUM_POSITIONS] = {
    // From position 0
    {0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30},
    // From position 1
    {30, 0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60},
    // From position 2
    {60, 30, 0, 30, 60, 90, 120, 150, 180, 150, 120, 90},
    // From position 3
    {90, 60, 30, 0, 30, 60, 90, 120, 150, 180, 150, 120},
    // From position 4
    {120, 90, 60, 30, 0, 30, 60, 90, 120, 150, 180, 150},
    // From position 5
    {150, 120, 90, 60, 30, 0, 30, 60, 90, 120, 150, 180},
    // From position 6
    {180, 150, 120, 90, 60, 30, 0, 30, 60, 90, 120, 150},
    // From position 7
    {150, 180, 150, 120, 90, 60, 30, 0, 30, 60, 90, 120},
    // From position 8
    {120, 150, 180, 150, 120, 90, 60, 30, 0, 30, 60, 90},
    // From position 9
    {90, 120, 150, 180, 150, 120, 90, 60, 30, 0, 30, 60},
    // From position 10
    {60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0, 30},
    // From position 11
    {30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0}
};

// ============================================================================
// NEIGHBOR TABLE (12×5)
// ============================================================================

/**
 * The 5 nearest neighbors for each position.
 * Based on icosahedron geometry (kissing number = 5).
 */
const uint8_t NEIGHBOR_TABLE[NUM_POSITIONS][5] = {
    // Position 0: neighbors are 1, 2, 10, 11, and 6 (opposite)
    {1, 2, 10, 11, 6},
    // Position 1: neighbors are 0, 2, 3, 11, 7
    {0, 2, 3, 11, 7},
    // Position 2: neighbors are 0, 1, 3, 4, 8
    {0, 1, 3, 4, 8},
    // Position 3: neighbors are 1, 2, 4, 5, 9
    {1, 2, 4, 5, 9},
    // Position 4: neighbors are 2, 3, 5, 6, 10
    {2, 3, 5, 6, 10},
    // Position 5: neighbors are 3, 4, 6, 7, 11
    {3, 4, 6, 7, 11},
    // Position 6: neighbors are 4, 5, 7, 8, 0
    {4, 5, 7, 8, 0},
    // Position 7: neighbors are 5, 6, 8, 9, 1
    {5, 6, 8, 9, 1},
    // Position 8: neighbors are 6, 7, 9, 10, 2
    {6, 7, 9, 10, 2},
    // Position 9: neighbors are 7, 8, 10, 11, 3
    {7, 8, 10, 11, 3},
    // Position 10: neighbors are 8, 9, 11, 0, 4
    {8, 9, 11, 0, 4},
    // Position 11: neighbors are 9, 10, 0, 1, 5
    {9, 10, 0, 1, 5}
};

// ============================================================================
// LAYER SCALE TABLE (8 Layers)
// ============================================================================

/**
 * Exact magnitude scales for each layer.
 * Each layer is 10^(3*layer).
 */
const uint64_t LAYER_SCALE_TABLE[NUM_LAYERS] = {
    1ULL,                    // Layer 0: 10^0  = 1
    1000ULL,                 // Layer 1: 10^3  = 1,000
    1000000ULL,              // Layer 2: 10^6  = 1,000,000
    1000000000ULL,           // Layer 3: 10^9  = 1,000,000,000
    1000000000000ULL,        // Layer 4: 10^12 = 1 trillion
    1000000000000000ULL,     // Layer 5: 10^15 = 1 quadrillion
    1000000000000000000ULL,  // Layer 6: 10^18 = 1 quintillion
    1000000000000000000ULL   // Layer 7: 10^21 (approximate, use abacus for exact)
};

// ============================================================================
// POSITION ANGLE TABLE (12 Positions)
// ============================================================================

/**
 * Exact angle for each position in degrees.
 * Position i = i * 30°
 */
const uint16_t POSITION_ANGLE_TABLE[NUM_POSITIONS] = {
    0,    // Position 0: 0°
    30,   // Position 1: 30°
    60,   // Position 2: 60°
    90,   // Position 3: 90°
    120,  // Position 4: 120°
    150,  // Position 5: 150°
    180,  // Position 6: 180°
    210,  // Position 7: 210°
    240,  // Position 8: 240°
    270,  // Position 9: 270°
    300,  // Position 10: 300°
    330   // Position 11: 330°
};

// ============================================================================
// PLATONIC SOLID VERTICES
// ============================================================================

/**
 * All vertices are stored as exact rationals.
 * No floating point - all coordinates are fractions.
 */

// Tetrahedron: 4 vertices
// Vertices at: (1,1,1), (1,-1,-1), (-1,1,-1), (-1,-1,1)
const GeometricVertex TETRAHEDRON_VERTICES[4] = {
    {{1, 1}, {1, 1}, {1, 1}},      // (1, 1, 1)
    {{1, 1}, {-1, 1}, {-1, 1}},    // (1, -1, -1)
    {{-1, 1}, {1, 1}, {-1, 1}},    // (-1, 1, -1)
    {{-1, 1}, {-1, 1}, {1, 1}}     // (-1, -1, 1)
};

// Cube: 8 vertices
// Vertices at: (±1, ±1, ±1)
const GeometricVertex CUBE_VERTICES[8] = {
    {{1, 1}, {1, 1}, {1, 1}},      // (1, 1, 1)
    {{1, 1}, {1, 1}, {-1, 1}},     // (1, 1, -1)
    {{1, 1}, {-1, 1}, {1, 1}},     // (1, -1, 1)
    {{1, 1}, {-1, 1}, {-1, 1}},    // (1, -1, -1)
    {{-1, 1}, {1, 1}, {1, 1}},     // (-1, 1, 1)
    {{-1, 1}, {1, 1}, {-1, 1}},    // (-1, 1, -1)
    {{-1, 1}, {-1, 1}, {1, 1}},    // (-1, -1, 1)
    {{-1, 1}, {-1, 1}, {-1, 1}}    // (-1, -1, -1)
};

// Octahedron: 6 vertices
// Vertices at: (±1, 0, 0), (0, ±1, 0), (0, 0, ±1)
const GeometricVertex OCTAHEDRON_VERTICES[6] = {
    {{1, 1}, {0, 1}, {0, 1}},      // (1, 0, 0)
    {{-1, 1}, {0, 1}, {0, 1}},     // (-1, 0, 0)
    {{0, 1}, {1, 1}, {0, 1}},      // (0, 1, 0)
    {{0, 1}, {-1, 1}, {0, 1}},     // (0, -1, 0)
    {{0, 1}, {0, 1}, {1, 1}},      // (0, 0, 1)
    {{0, 1}, {0, 1}, {-1, 1}}      // (0, 0, -1)
};

// Dodecahedron: 20 vertices
// Using golden ratio φ = 1.618034
// Vertices include (±1, ±1, ±1), (0, ±1/φ, ±φ), (±1/φ, ±φ, 0), (±φ, 0, ±1/φ)
const GeometricVertex DODECAHEDRON_VERTICES[20] = {
    // (±1, ±1, ±1) - 8 vertices
    {{1, 1}, {1, 1}, {1, 1}},
    {{1, 1}, {1, 1}, {-1, 1}},
    {{1, 1}, {-1, 1}, {1, 1}},
    {{1, 1}, {-1, 1}, {-1, 1}},
    {{-1, 1}, {1, 1}, {1, 1}},
    {{-1, 1}, {1, 1}, {-1, 1}},
    {{-1, 1}, {-1, 1}, {1, 1}},
    {{-1, 1}, {-1, 1}, {-1, 1}},
    // (0, ±1/φ, ±φ) - 4 vertices
    {{0, 1}, {GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},
    {{0, 1}, {GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},
    {{0, 1}, {-GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},
    {{0, 1}, {-GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},
    // (±1/φ, ±φ, 0) - 4 vertices
    {{GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},
    {{GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},
    {{-GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},
    {{-GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},
    // (±φ, 0, ±1/φ) - 4 vertices
    {{GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}},
    {{GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {-GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}},
    {{-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}},
    {{-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {-GOLDEN_RATIO_DEN, GOLDEN_RATIO_NUM}}
};

// Icosahedron: 12 vertices (MOST IMPORTANT for threading)
// Vertices at: (0, ±1, ±φ), (±1, ±φ, 0), (±φ, 0, ±1)
const GeometricVertex ICOSAHEDRON_VERTICES[12] = {
    // (0, ±1, ±φ) - 4 vertices
    {{0, 1}, {1, 1}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},      // Position 0: (0, 1, φ)
    {{0, 1}, {1, 1}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},     // Position 1: (0, 1, -φ)
    {{0, 1}, {-1, 1}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},     // Position 2: (0, -1, φ)
    {{0, 1}, {-1, 1}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}},    // Position 3: (0, -1, -φ)
    // (±1, ±φ, 0) - 4 vertices
    {{1, 1}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},      // Position 4: (1, φ, 0)
    {{1, 1}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},     // Position 5: (1, -φ, 0)
    {{-1, 1}, {GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},     // Position 6: (-1, φ, 0)
    {{-1, 1}, {-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}},    // Position 7: (-1, -φ, 0)
    // (±φ, 0, ±1) - 4 vertices
    {{GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {1, 1}},      // Position 8: (φ, 0, 1)
    {{GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {-1, 1}},     // Position 9: (φ, 0, -1)
    {{-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {1, 1}},     // Position 10: (-φ, 0, 1)
    {{-GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN}, {0, 1}, {-1, 1}}     // Position 11: (-φ, 0, -1)
};

// ============================================================================
// LAYER TO PLATONIC SOLID MAPPING
// ============================================================================

const PlatonicSolidType LAYER_PLATONIC_TABLE[NUM_LAYERS] = {
    PLATONIC_TETRAHEDRON,   // Layer 0: Tetrahedron (4 vertices)
    PLATONIC_CUBE,          // Layer 1: Cube (8 vertices)
    PLATONIC_OCTAHEDRON,    // Layer 2: Octahedron (6 vertices)
    PLATONIC_DODECAHEDRON,  // Layer 3: Dodecahedron (20 vertices)
    PLATONIC_ICOSAHEDRON,   // Layer 4: Icosahedron (12 vertices)
    PLATONIC_TETRAHEDRON,   // Layer 5: Repeat - Tetrahedron
    PLATONIC_CUBE,          // Layer 6: Repeat - Cube
    PLATONIC_OCTAHEDRON     // Layer 7: Repeat - Octahedron
};

// ============================================================================
// PRIME POSITION TABLE
// ============================================================================

const bool PRIME_POSITION_TABLE[NUM_POSITIONS] = {
    false,  // 0 is not prime
    true,   // 1 is prime
    false,  // 2 is not prime (even)
    false,  // 3 is not prime (composite)
    false,  // 4 is not prime (even)
    true,   // 5 is prime
    false,  // 6 is not prime (even)
    true,   // 7 is prime
    false,  // 8 is not prime (even)
    false,  // 9 is not prime (composite)
    false,  // 10 is not prime (even)
    true    // 11 is prime
};

// ============================================================================
// ROTATION TABLE (12×12)
// ============================================================================

const uint8_t ROTATION_TABLE[NUM_POSITIONS][NUM_POSITIONS] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1},
    {3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2},
    {4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3},
    {5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4},
    {6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5},
    {7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6},
    {8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7},
    {9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8},
    {10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
    {11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
};

// ============================================================================
// INVERSE ROTATION TABLE (12×12)
// ============================================================================

const uint8_t INVERSE_ROTATION_TABLE[NUM_POSITIONS][NUM_POSITIONS] = {
    {0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
    {1, 0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2},
    {2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4, 3},
    {3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4},
    {4, 3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5},
    {5, 4, 3, 2, 1, 0, 11, 10, 9, 8, 7, 6},
    {6, 5, 4, 3, 2, 1, 0, 11, 10, 9, 8, 7},
    {7, 6, 5, 4, 3, 2, 1, 0, 11, 10, 9, 8},
    {8, 7, 6, 5, 4, 3, 2, 1, 0, 11, 10, 9},
    {9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 11, 10},
    {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 11},
    {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}
};

// ============================================================================
// MIDPOINT TABLE (12×12)
// ============================================================================

const uint8_t MIDPOINT_TABLE[NUM_POSITIONS][NUM_POSITIONS] = {
    {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5},
    {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6},
    {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6},
    {1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7},
    {2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7},
    {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8},
    {3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8},
    {3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9},
    {4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9},
    {4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10},
    {5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10},
    {5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11}
};