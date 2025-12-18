#ifndef DUALITY_88D_H
#define DUALITY_88D_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../../math/include/math/abacus.h"

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

/**
 * Position on 12-fold clock lattice (for duality prototype)
 */
typedef struct {
    uint8_t position;      // 0-11 (12 positions on clock)
    uint8_t quadrant;      // 0-3 (which quadrant: Q1, Q2, Q3, Q4)
    double angle;          // 0-360 degrees
    bool is_folded;        // true if folded to Q1
    uint8_t source_quad;   // Original quadrant (for unfolding)
} DualityClockPosition;

/**
 * Platonic solid types
 */
typedef enum {
    TETRAHEDRON,    // 4 vertices, 6 edges, 4 faces (self-dual)
    CUBE,           // 8 vertices, 12 edges, 6 faces
    OCTAHEDRON,     // 6 vertices, 12 edges, 8 faces
    DODECAHEDRON,   // 20 vertices, 30 edges, 12 faces
    ICOSAHEDRON     // 12 vertices, 30 edges, 20 faces
} PlatonicSolid;

/**
 * Coordinate frame using a Platonic solid
 */
typedef struct {
    PlatonicSolid solid;         // Which solid to use
    uint8_t layer;               // 0-7 (which octave/layer)
    uint8_t num_vertices;        // Number of vertices
    uint8_t num_edges;           // Number of edges
    uint8_t num_faces;           // Number of faces
} CoordinateFrame;

/**
 * Phase angle (for electrical/harmonic relationships)
 */
typedef struct {
    uint8_t phase;          // 0-2 (A, B, C)
    double angle;           // 0, 120, 240 degrees
    double frequency;       // Hz
    uint8_t harmonic;       // 1, 2, 3, ... (harmonic number)
} PhaseAngle;

/**
 * Point in 88-dimensional space
 * 88 = 8 layers × 11 dimensions per layer
 */
typedef struct {
    DualityClockPosition layers[8];     // 8 octaves/layers
    uint8_t dimension_in_layer;  // 0-10 (11 dimensions per layer)
    CrystallineAbacus* value;    // Arbitrary precision value
    bool is_dual;                // true if in dual space
    CoordinateFrame frame;       // Coordinate frame for this point
} Point88D;

// ============================================================================
// CLOCK POSITION OPERATIONS
// ============================================================================

/**
 * Create a clock position
 */
DualityClockPosition clock_position_create(uint8_t position);

/**
 * Get angle in degrees for a position
 */
double clock_position_angle(uint8_t position);

/**
 * Get quadrant for a position (0-3)
 */
uint8_t clock_position_quadrant(uint8_t position);

// ============================================================================
// QUADRANT FOLDING OPERATIONS
// ============================================================================

/**
 * Fold position to first quadrant (Q1)
 * Preserves all information by tracking source quadrant
 */
DualityClockPosition fold_to_q1(DualityClockPosition pos);

/**
 * Unfold from Q1 back to original quadrant
 */
DualityClockPosition unfold_from_q1(DualityClockPosition folded, uint8_t target_quadrant);

/**
 * Check if two positions are equivalent after folding
 */
bool positions_equivalent_folded(DualityClockPosition a, DualityClockPosition b);

// ============================================================================
// POLARITY FLIPPING OPERATIONS
// ============================================================================

/**
 * Check if position is a prime position {1, 5, 7, 11}
 */
bool is_prime_position(uint8_t pos);

/**
 * Square a prime position (universal polarity flip to 1)
 */
DualityClockPosition square_prime_position(DualityClockPosition pos);

/**
 * Verify universal polarity flip: p² ≡ 1 (mod 12) for all primes
 */
bool verify_polarity_flip(uint8_t pos);

/**
 * Square any position (mod 12)
 */
DualityClockPosition square_position(DualityClockPosition pos);

// ============================================================================
// PLATONIC SOLID OPERATIONS
// ============================================================================

/**
 * Get the dual of a Platonic solid
 */
PlatonicSolid get_dual_solid(PlatonicSolid solid);

/**
 * Get number of vertices for a solid
 */
uint8_t get_num_vertices(PlatonicSolid solid);

/**
 * Get number of edges for a solid
 */
uint8_t get_num_edges(PlatonicSolid solid);

/**
 * Get number of faces for a solid
 */
uint8_t get_num_faces(PlatonicSolid solid);

/**
 * Create coordinate frame for a layer
 */
CoordinateFrame create_coordinate_frame(uint8_t layer, PlatonicSolid solid);

/**
 * Check if a solid is self-dual
 */
bool is_self_dual(PlatonicSolid solid);

// ============================================================================
// PHASE ANGLE OPERATIONS
// ============================================================================

/**
 * Map position to phase angle
 */
PhaseAngle position_to_phase(DualityClockPosition pos, uint8_t harmonic);

/**
 * Get 3-phase positions for a given harmonic
 * Returns positions for phases A, B, C
 */
void get_three_phase_positions(uint8_t harmonic, DualityClockPosition phases[3]);

/**
 * Calculate harmonic frequency
 */
double calculate_harmonic_frequency(uint8_t harmonic_number);

/**
 * Map octave to frequency (each octave doubles)
 */
double octave_to_frequency(uint8_t octave);

/**
 * Get phase for a position (0=A, 1=B, 2=C)
 */
uint8_t get_phase_for_position(uint8_t position);

// ============================================================================
// 88D POINT OPERATIONS
// ============================================================================

/**
 * Create point in 88D space
 * layer: 0-7 (which octave)
 * dimension: 0-10 (which of 11 dimensions in layer)
 * position: 0-11 (position on clock)
 */
Point88D create_point_88d(uint8_t layer, uint8_t dimension, uint8_t position);

/**
 * Free a 88D point
 */
void free_point_88d(Point88D* point);

/**
 * Add two points in 88D space
 */
Point88D add_88d(Point88D a, Point88D b);

/**
 * Square a point (polarity flip for primes)
 */
Point88D square_88d(Point88D point);

/**
 * Flip point to dual space
 */
Point88D flip_to_dual(Point88D point);

/**
 * Check if two points are dual to each other
 */
bool are_dual_points(Point88D p1, Point88D p2);

/**
 * Get magnitude scale for a layer
 */
double get_layer_magnitude_scale(uint8_t layer);

/**
 * Print point information
 */
void print_point_88d(Point88D point);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get name of Platonic solid
 */
const char* get_solid_name(PlatonicSolid solid);

/**
 * Get name of phase
 */
const char* get_phase_name(uint8_t phase);

/**
 * Verify Euler's formula for a solid: V - E + F = 2
 */
bool verify_euler_formula(PlatonicSolid solid);

#endif // DUALITY_88D_H