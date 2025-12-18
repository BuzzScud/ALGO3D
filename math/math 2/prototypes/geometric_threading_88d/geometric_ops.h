/**
 * @file geometric_ops.h
 * @brief Core geometric operations (all O(1))
 * 
 * This module implements the fundamental geometric operations that enable
 * O(1) complexity for traditionally O(n) operations.
 * 
 * Key Operations:
 * - Geometric distance (O(1))
 * - Geometric triangulation (O(1))
 * - Layer selection (O(1))
 * - Coordinate transforms (O(1))
 * - Position arithmetic (O(1))
 */

#ifndef GEOMETRIC_OPS_H
#define GEOMETRIC_OPS_H

#include "geometric_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// POSITION OPERATIONS
// ============================================================================

/**
 * @brief Create geometric position
 * @param ring Ring number (0-3)
 * @param position Position on ring (0-11)
 * @param magnitude Magnitude (lap number)
 * @return Initialized geometric position
 * 
 * Complexity: O(1)
 */
GeometricPosition geo_pos_create(uint8_t ring, uint8_t position, int64_t magnitude);

/**
 * @brief Create position from clock position
 * @param clock_pos Clock position from math library
 * @return Geometric position
 * 
 * Complexity: O(1)
 */
GeometricPosition geo_pos_from_clock(const ClockPosition* clock_pos);

/**
 * @brief Convert to clock position
 * @param pos Geometric position
 * @param clock_pos Output clock position
 * 
 * Complexity: O(1)
 */
void geo_pos_to_clock(const GeometricPosition* pos, ClockPosition* clock_pos);

/**
 * @brief Check if position is prime position {1, 5, 7, 11}
 * @param pos Position to check
 * @return True if prime position
 * 
 * Complexity: O(1)
 */
bool geo_pos_is_prime(const GeometricPosition* pos);

/**
 * @brief Check if position is control position (0/12)
 * @param pos Position to check
 * @return True if control position
 * 
 * Complexity: O(1)
 */
bool geo_pos_is_control(const GeometricPosition* pos);

/**
 * @brief Get angle for position (0-360°)
 * @param pos Position
 * @return Angle in degrees
 * 
 * Complexity: O(1)
 */
double geo_pos_angle(const GeometricPosition* pos);

/**
 * @brief Select layer based on magnitude
 * @param magnitude Magnitude value
 * @return Layer number (0-7)
 * 
 * Complexity: O(1)
 * 
 * Layer selection:
 * - Layer 0: 10^0 to 10^3
 * - Layer 1: 10^3 to 10^6
 * - Layer 2: 10^6 to 10^9
 * - Layer 3: 10^9 to 10^12
 * - Layer 4: 10^12 to 10^15
 * - Layer 5: 10^15 to 10^18
 * - Layer 6: 10^18 to 10^21
 * - Layer 7: > 10^21
 */
uint8_t geo_select_layer(int64_t magnitude);

// ============================================================================
// DISTANCE & ANGLE OPERATIONS (O(1))
// ============================================================================

/**
 * @brief Compute geometric distance between two positions
 * @param pos1 First position
 * @param pos2 Second position
 * @return Geometric distance
 * 
 * Complexity: O(1)
 * 
 * Uses clock lattice geometry:
 * - Position difference (shortest path on circle)
 * - Magnitude difference
 * - Pythagorean combination
 */
double geo_distance(const GeometricPosition* pos1, const GeometricPosition* pos2);

/**
 * @brief Compute angle between two positions
 * @param pos1 First position
 * @param pos2 Second position
 * @return Angle in degrees (0-360°)
 * 
 * Complexity: O(1)
 */
double geo_angle_between(const GeometricPosition* pos1, const GeometricPosition* pos2);

/**
 * @brief Compute shortest path on clock circle
 * @param pos1 First position (0-11)
 * @param pos2 Second position (0-11)
 * @return Shortest distance (0-6)
 * 
 * Complexity: O(1)
 */
uint8_t geo_shortest_path(uint8_t pos1, uint8_t pos2);

// ============================================================================
// TRIANGULATION (O(1))
// ============================================================================

/**
 * @brief Geometric triangulation using three reference points
 * @param p1 First reference point
 * @param p2 Second reference point
 * @param p3 Third reference point
 * @return Triangulated position
 * 
 * Complexity: O(1)
 * 
 * Uses Plimpton 322 formula (Babylonian mathematics):
 * - a = p² - q²
 * - b = 2pq
 * - c = p² + q²
 * 
 * This is pure geometric calculation, no iteration needed.
 */
GeometricPosition geo_triangulate(
    const GeometricPosition* p1,
    const GeometricPosition* p2,
    const GeometricPosition* p3
);

/**
 * @brief Triangulate with weights
 * @param p1 First point
 * @param w1 Weight for first point
 * @param p2 Second point
 * @param w2 Weight for second point
 * @param p3 Third point
 * @param w3 Weight for third point
 * @return Weighted triangulated position
 * 
 * Complexity: O(1)
 */
GeometricPosition geo_triangulate_weighted(
    const GeometricPosition* p1, double w1,
    const GeometricPosition* p2, double w2,
    const GeometricPosition* p3, double w3
);

// ============================================================================
// POSITION ARITHMETIC (O(1))
// ============================================================================

/**
 * @brief Add two positions geometrically
 * @param pos1 First position
 * @param pos2 Second position
 * @return Sum position
 * 
 * Complexity: O(1)
 * 
 * Geometric addition:
 * - Add positions (mod 12)
 * - Add magnitudes
 * - Handle carry
 */
GeometricPosition geo_add(const GeometricPosition* pos1, const GeometricPosition* pos2);

/**
 * @brief Subtract two positions geometrically
 * @param pos1 First position
 * @param pos2 Second position
 * @return Difference position
 * 
 * Complexity: O(1)
 */
GeometricPosition geo_subtract(const GeometricPosition* pos1, const GeometricPosition* pos2);

/**
 * @brief Multiply position by scalar
 * @param pos Position
 * @param scalar Scalar multiplier
 * @return Scaled position
 * 
 * Complexity: O(1)
 */
GeometricPosition geo_scale(const GeometricPosition* pos, int64_t scalar);

/**
 * @brief Compute midpoint between two positions
 * @param pos1 First position
 * @param pos2 Second position
 * @return Midpoint
 * 
 * Complexity: O(1)
 */
GeometricPosition geo_midpoint(const GeometricPosition* pos1, const GeometricPosition* pos2);

// ============================================================================
// PLATONIC FRAME OPERATIONS
// ============================================================================

/**
 * @brief Create platonic frame for layer
 * @param layer Layer number (0-7)
 * @return Initialized platonic frame
 * 
 * Complexity: O(1)
 * 
 * Frame assignment:
 * - Layer 0: Tetrahedron
 * - Layer 1: Cube
 * - Layer 2: Octahedron
 * - Layer 3: Dodecahedron
 * - Layer 4: Icosahedron
 * - Layers 5-7: Repeat pattern
 */
PlatonicFrame geo_frame_create(uint8_t layer);

/**
 * @brief Get vertex coordinates for position
 * @param frame Platonic frame
 * @param position Position (0-11)
 * @param vertex Output vertex coordinates [x, y, z]
 * 
 * Complexity: O(1)
 */
void geo_frame_get_vertex(const PlatonicFrame* frame, uint8_t position, double vertex[3]);

/**
 * @brief Transform position between frames
 * @param pos Position in source frame
 * @param from_frame Source frame
 * @param to_frame Destination frame
 * @return Position in destination frame
 * 
 * Complexity: O(1)
 */
GeometricPosition geo_frame_transform(
    const GeometricPosition* pos,
    const PlatonicFrame* from_frame,
    const PlatonicFrame* to_frame
);

/**
 * @brief Get dual frame
 * @param frame Original frame
 * @return Dual frame
 * 
 * Complexity: O(1)
 * 
 * Duality:
 * - Tetrahedron ↔ Tetrahedron (self-dual)
 * - Cube ↔ Octahedron
 * - Dodecahedron ↔ Icosahedron
 */
PlatonicFrame geo_frame_dual(const PlatonicFrame* frame);

// ============================================================================
// ICOSAHEDRON OPERATIONS (for thread positioning)
// ============================================================================

/**
 * @brief Get icosahedron vertex coordinates
 * @param vertex_id Vertex ID (0-11)
 * @param coords Output coordinates [x, y, z]
 * 
 * Complexity: O(1)
 * 
 * Uses golden ratio φ = 1.618...
 * Vertices: (0, ±1, ±φ), (±1, ±φ, 0), (±φ, 0, ±1)
 */
void geo_icosahedron_vertex(uint8_t vertex_id, double coords[3]);

/**
 * @brief Get angle for icosahedron vertex
 * @param vertex_id Vertex ID (0-11)
 * @return Angle in degrees (0-360°)
 * 
 * Complexity: O(1)
 */
double geo_icosahedron_angle(uint8_t vertex_id);

/**
 * @brief Get neighbors for icosahedron vertex
 * @param vertex_id Vertex ID (0-11)
 * @param neighbors Output array of neighbor IDs (size 5)
 * @return Number of neighbors (always 5 for icosahedron)
 * 
 * Complexity: O(1)
 */
uint8_t geo_icosahedron_neighbors(uint8_t vertex_id, uint8_t neighbors[5]);

// ============================================================================
// MAGNITUDE SCALING
// ============================================================================

/**
 * @brief Scale position to different layer
 * @param pos Position to scale
 * @param target_layer Target layer
 * @return Scaled position
 * 
 * Complexity: O(1)
 * 
 * Scaling factor: 12^(layer_diff)
 * Each layer represents 3 orders of magnitude
 */
GeometricPosition geo_scale_to_layer(const GeometricPosition* pos, uint8_t target_layer);

/**
 * @brief Get magnitude scale for layer
 * @param layer Layer number (0-7)
 * @return Magnitude scale (10^(3*layer))
 * 
 * Complexity: O(1)
 */
double geo_layer_scale(uint8_t layer);

// ============================================================================
// VALIDATION & UTILITIES
// ============================================================================

/**
 * @brief Validate geometric position
 * @param pos Position to validate
 * @return True if valid
 * 
 * Complexity: O(1)
 */
bool geo_pos_validate(const GeometricPosition* pos);

/**
 * @brief Compare two positions
 * @param pos1 First position
 * @param pos2 Second position
 * @return 0 if equal, <0 if pos1 < pos2, >0 if pos1 > pos2
 * 
 * Complexity: O(1)
 */
int geo_pos_compare(const GeometricPosition* pos1, const GeometricPosition* pos2);

/**
 * @brief Print position (for debugging)
 * @param pos Position to print
 * 
 * Complexity: O(1)
 */
void geo_pos_print(const GeometricPosition* pos);

/**
 * @brief Print frame information
 * @param frame Frame to print
 * 
 * Complexity: O(1)
 */
void geo_frame_print(const PlatonicFrame* frame);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_OPS_H