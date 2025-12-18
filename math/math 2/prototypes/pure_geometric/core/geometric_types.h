/**
 * @file geometric_types.h
 * @brief Core geometric data types - NO FLOATING POINT
 * 
 * This file defines the fundamental data structures for pure geometric computation.
 * Key principles:
 * - No float or double types
 * - All coordinates are exact rationals
 * - All magnitudes use CrystallineAbacus
 * - All operations are O(1) lookups
 */

#ifndef PURE_GEOMETRIC_TYPES_H
#define PURE_GEOMETRIC_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration - will use CrystallineAbacus from main codebase
typedef struct CrystallineAbacus CrystallineAbacus;

// ============================================================================
// RATIONAL NUMBERS (Exact Fractions)
// ============================================================================

/**
 * @brief Exact rational number representation
 * 
 * Replaces floating point with exact fractions.
 * Example: 1.618... (golden ratio) = 1618034/1000000
 */
typedef struct {
    int64_t numerator;
    int64_t denominator;
} Rational;

// Common rational constants
#define RATIONAL_ZERO ((Rational){0, 1})
#define RATIONAL_ONE ((Rational){1, 1})
#define RATIONAL_HALF ((Rational){1, 2})

// Golden ratio φ = (1 + √5) / 2 ≈ 1.618033988749895
// Approximated as 1618034/1000000 for exact computation
#define GOLDEN_RATIO_NUM 1618034LL
#define GOLDEN_RATIO_DEN 1000000LL
#define RATIONAL_PHI ((Rational){GOLDEN_RATIO_NUM, GOLDEN_RATIO_DEN})

// ============================================================================
// GEOMETRIC VERTEX (Exact 3D Coordinates)
// ============================================================================

/**
 * @brief 3D vertex with exact rational coordinates
 * 
 * Replaces (double x, double y, double z) with exact fractions.
 * Used for platonic solid vertices.
 */
typedef struct {
    Rational x;
    Rational y;
    Rational z;
} GeometricVertex;

// ============================================================================
// GEOMETRIC POSITION (The Core Type)
// ============================================================================

/**
 * @brief A position in geometric space
 * 
 * This is the fundamental type that replaces "numbers".
 * Every value is represented as a position in geometric space.
 * 
 * Properties:
 * - position: Where on the 12-fold clock (0-11)
 * - magnitude: Exact value via CrystallineAbacus
 * - layer: Which magnitude scale (0-7)
 * 
 * Example: The number 42
 * - 42 = 3×12 + 6
 * - position = 6 (6th position on clock)
 * - magnitude = 42 (stored in abacus)
 * - layer = 0 (magnitude < 1000)
 */
typedef struct {
    uint8_t position;              // Clock position (0-11)
    CrystallineAbacus* magnitude;  // Exact magnitude (no floats!)
    uint8_t layer;                 // Magnitude scale (0-7)
} GeometricPosition;

// ============================================================================
// LAYER SYSTEM (8 Layers for Different Scales)
// ============================================================================

/**
 * @brief Layer definitions for magnitude scales
 * 
 * Each layer handles a different magnitude range:
 * Layer 0: 10^0  to 10^3   (1 to 1,000)
 * Layer 1: 10^3  to 10^6   (1,000 to 1,000,000)
 * Layer 2: 10^6  to 10^9   (millions to billions)
 * Layer 3: 10^9  to 10^12  (billions to trillions)
 * Layer 4: 10^12 to 10^15  (trillions to quadrillions)
 * Layer 5: 10^15 to 10^18  (quadrillions to quintillions)
 * Layer 6: 10^18 to 10^21  (quintillions to sextillions)
 * Layer 7: 10^21+          (sextillions and beyond)
 */
#define NUM_LAYERS 8

// ============================================================================
// PLATONIC SOLID TYPES
// ============================================================================

/**
 * @brief The 5 Platonic solids
 * 
 * Each layer uses a different platonic solid as its coordinate frame:
 * - Tetrahedron: 4 vertices (minimal structure)
 * - Cube: 8 vertices (balanced structure)
 * - Octahedron: 6 vertices (dual of cube)
 * - Dodecahedron: 20 vertices (complex structure)
 * - Icosahedron: 12 vertices (optimal for threading)
 */
typedef enum {
    PLATONIC_TETRAHEDRON = 0,  // 4 vertices
    PLATONIC_CUBE = 1,         // 8 vertices
    PLATONIC_OCTAHEDRON = 2,   // 6 vertices
    PLATONIC_DODECAHEDRON = 3, // 20 vertices
    PLATONIC_ICOSAHEDRON = 4   // 12 vertices
} PlatonicSolidType;

/**
 * @brief Platonic solid coordinate frame
 * 
 * Stores the exact vertices of a platonic solid.
 * Used as coordinate frame for each layer.
 */
typedef struct {
    PlatonicSolidType type;
    uint8_t num_vertices;
    GeometricVertex* vertices;  // Array of exact vertices
} PlatonicFrame;

// ============================================================================
// CLOCK POSITIONS (12-Fold Symmetry)
// ============================================================================

/**
 * @brief Clock position constants
 * 
 * The 12 positions on the clock face.
 * These are the only valid positions in the system.
 */
#define NUM_POSITIONS 12

// Position names (for clarity)
#define POS_0  0   // 0° (control position)
#define POS_1  1   // 30°
#define POS_2  2   // 60°
#define POS_3  3   // 90°
#define POS_4  4   // 120°
#define POS_5  5   // 150°
#define POS_6  6   // 180° (opposite of 0)
#define POS_7  7   // 210°
#define POS_8  8   // 240°
#define POS_9  9   // 270°
#define POS_10 10  // 300°
#define POS_11 11  // 330°

// Prime positions (1, 5, 7, 11)
#define IS_PRIME_POSITION(pos) ((pos) == 1 || (pos) == 5 || (pos) == 7 || (pos) == 11)

// Control position (0 or 12)
#define IS_CONTROL_POSITION(pos) ((pos) % 12 == 0)

// ============================================================================
// GEOMETRIC SPACE 88D
// ============================================================================

/**
 * @brief 88-dimensional geometric space
 * 
 * Structure: 8 layers × 11 dimensions = 88 dimensions
 * - 8 layers: Different magnitude scales
 * - 11 dimensions: Clock positions 1-11 (excluding control position 0)
 * 
 * This is the complete geometric space for computation.
 */
typedef struct {
    GeometricPosition positions[NUM_LAYERS][NUM_POSITIONS - 1];  // 8×11 = 88
    PlatonicFrame frames[NUM_LAYERS];                             // One frame per layer
    uint8_t active_layer;                                         // Currently active layer
} GeometricSpace88D;

// ============================================================================
// GEOMETRIC THREAD (For Threading Model)
// ============================================================================

/**
 * @brief A thread positioned at an icosahedron vertex
 * 
 * Threads are not arbitrary - they are positioned geometrically:
 * - Control thread: At center (position 0)
 * - Worker threads: At icosahedron vertices (positions 1-12)
 */
typedef struct {
    uint8_t thread_id;              // 0-12 (0 is control)
    uint8_t position;               // Clock position (0-11)
    GeometricVertex vertex;         // 3D coordinates (exact)
    GeometricSpace88D* local_space; // Thread-local 88D space
    bool is_control;                // True for thread 0
} GeometricThread;

// ============================================================================
// HELPER MACROS
// ============================================================================

// Normalize position to 0-11 range
#define NORMALIZE_POSITION(pos) ((pos) % NUM_POSITIONS)

// Calculate ring from position and magnitude
#define CALCULATE_RING(pos, mag) ((pos) / NUM_POSITIONS)

// Check if two positions are neighbors (distance ≤ 1)
#define ARE_NEIGHBORS(pos1, pos2) (DISTANCE_TABLE[pos1][pos2] <= 1)

#endif // PURE_GEOMETRIC_TYPES_H