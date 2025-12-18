#ifndef ALGORITHMS_SPHERE_PACKING_H
#define ALGORITHMS_SPHERE_PACKING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sphere Packing Geometry
 * 
 * Generic geometric operations for sphere packing and kissing spheres.
 * Based on crystalline lattice geometry principles.
 * 
 * Key Concepts:
 * - Kissing spheres: Spheres that touch at exactly one point
 * - Sphere gap: Distance between sphere surfaces (encodes π curvature)
 * - Radial lines: Lines emanating from origin through sphere centers
 * - Concentric rings: Rings of spheres at equal distance from origin
 */

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * 3D Sphere
 * 
 * Represents a sphere in 3D space.
 */
typedef struct {
    double center_x;          // X coordinate of center
    double center_y;          // Y coordinate of center
    double center_z;          // Z coordinate of center
    double radius;            // Sphere radius
    uint64_t id;              // Optional identifier
} Sphere3D;

/**
 * 2D Vector
 * 
 * Represents a 2D vector for geometric operations.
 */
typedef struct {
    double x;                 // X component
    double y;                 // Y component
} Vec2D;

/**
 * 3D Vector
 * 
 * Represents a 3D vector for geometric operations.
 */
typedef struct {
    double x;                 // X component
    double y;                 // Y component
    double z;                 // Z component
} Vec3D;

// ============================================================================
// SPHERE OPERATIONS
// ============================================================================

/**
 * Check if two spheres are kissing (touching at exactly one point)
 * 
 * @param s1 First sphere
 * @param s2 Second sphere
 * @param tolerance Tolerance for floating point comparison
 * @return true if spheres are kissing, false otherwise
 */
bool spheres_are_kissing(const Sphere3D* s1, const Sphere3D* s2, double tolerance);

/**
 * Calculate gap between two spheres
 * 
 * The gap encodes the curvature of π in the crystalline lattice.
 * Positive gap: spheres are separated
 * Zero gap: spheres are kissing
 * Negative gap: spheres overlap
 * 
 * @param s1 First sphere
 * @param s2 Second sphere
 * @return Gap distance (can be negative for overlap)
 */
double sphere_gap(const Sphere3D* s1, const Sphere3D* s2);

/**
 * Calculate distance between sphere centers
 * 
 * @param s1 First sphere
 * @param s2 Second sphere
 * @return Distance between centers
 */
double sphere_distance(const Sphere3D* s1, const Sphere3D* s2);

/**
 * Check if two spheres overlap
 * 
 * @param s1 First sphere
 * @param s2 Second sphere
 * @return true if spheres overlap, false otherwise
 */
bool spheres_overlap(const Sphere3D* s1, const Sphere3D* s2);

/**
 * Calculate volume of sphere
 * 
 * @param sphere Sphere to calculate volume for
 * @return Volume (4/3 * π * r³)
 */
double sphere_volume(const Sphere3D* sphere);

/**
 * Calculate surface area of sphere
 * 
 * @param sphere Sphere to calculate surface area for
 * @return Surface area (4 * π * r²)
 */
double sphere_surface_area(const Sphere3D* sphere);

// ============================================================================
// VECTOR OPERATIONS
// ============================================================================

/**
 * Add two 2D vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @param result Output vector (a + b)
 */
void vector2d_add(const Vec2D* a, const Vec2D* b, Vec2D* result);

/**
 * Subtract two 2D vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @param result Output vector (a - b)
 */
void vector2d_subtract(const Vec2D* a, const Vec2D* b, Vec2D* result);

/**
 * Scale a 2D vector
 * 
 * @param v Vector to scale
 * @param scalar Scaling factor
 * @param result Output vector (v * scalar)
 */
void vector2d_scale(const Vec2D* v, double scalar, Vec2D* result);

/**
 * Calculate dot product of two 2D vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @return Dot product (a · b)
 */
double vector2d_dot(const Vec2D* a, const Vec2D* b);

/**
 * Calculate magnitude of 2D vector
 * 
 * @param v Vector
 * @return Magnitude (||v||)
 */
double vector2d_magnitude(const Vec2D* v);

/**
 * Normalize a 2D vector
 * 
 * @param v Vector to normalize
 * @param result Output normalized vector
 * @return true on success, false if vector is zero
 */
bool vector2d_normalize(const Vec2D* v, Vec2D* result);

/**
 * Add two 3D vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @param result Output vector (a + b)
 */
void vector3d_add(const Vec3D* a, const Vec3D* b, Vec3D* result);

/**
 * Subtract two 3D vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @param result Output vector (a - b)
 */
void vector3d_subtract(const Vec3D* a, const Vec3D* b, Vec3D* result);

/**
 * Scale a 3D vector
 * 
 * @param v Vector to scale
 * @param scalar Scaling factor
 * @param result Output vector (v * scalar)
 */
void vector3d_scale(const Vec3D* v, double scalar, Vec3D* result);

/**
 * Calculate dot product of two 3D vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @return Dot product (a · b)
 */
double vector3d_dot(const Vec3D* a, const Vec3D* b);

/**
 * Calculate cross product of two 3D vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @param result Output vector (a × b)
 */
void vector3d_cross(const Vec3D* a, const Vec3D* b, Vec3D* result);

/**
 * Calculate magnitude of 3D vector
 * 
 * @param v Vector
 * @return Magnitude (||v||)
 */
double vector3d_magnitude(const Vec3D* v);

/**
 * Normalize a 3D vector
 * 
 * @param v Vector to normalize
 * @param result Output normalized vector
 * @return true on success, false if vector is zero
 */
bool vector3d_normalize(const Vec3D* v, Vec3D* result);

// ============================================================================
// LATTICE OPERATIONS
// ============================================================================

/**
 * Map value to radial line
 * 
 * Determines which radial line (emanating from origin) a value belongs to.
 * 
 * @param value Value to map
 * @return Radial line index (0-11 for 12-fold symmetry)
 */
int map_to_radial_line(uint64_t value);

/**
 * Map value to concentric ring
 * 
 * Determines which concentric ring (distance from origin) a value belongs to.
 * 
 * @param value Value to map
 * @return Ring number (0 = origin, 1 = first ring, etc.)
 */
int map_to_concentric_ring(uint64_t value);

/**
 * Check if value is on specific radial line
 * 
 * @param value Value to check
 * @param line_index Radial line index (0-11)
 * @return true if on line, false otherwise
 */
bool is_on_radial_line(uint64_t value, int line_index);

/**
 * Check if value is on specific concentric ring
 * 
 * @param value Value to check
 * @param ring_number Ring number
 * @return true if on ring, false otherwise
 */
bool is_on_concentric_ring(uint64_t value, int ring_number);

/**
 * Map value to 12-fold symmetry group
 * 
 * Maps a value to one of 12 symmetry groups (mod 12).
 * This is fundamental to the crystalline lattice structure.
 * 
 * @param value Value to map
 * @return Symmetry group (0-11)
 */
int map_to_symmetry_group(uint64_t value);

// ============================================================================
// ANGULAR OPERATIONS
// ============================================================================

/**
 * Map value to angle (0 to 2π)
 * 
 * @param value Value to map
 * @return Angle in radians
 */
double map_to_angle(uint64_t value);

/**
 * Map angle to clock position (0-11)
 * 
 * 0 = 12 o'clock (top)
 * 3 = 3 o'clock (right)
 * 6 = 6 o'clock (bottom)
 * 9 = 9 o'clock (left)
 * 
 * @param angle_radians Angle in radians
 * @return Clock position (0-11)
 */
int angle_to_clock_position(double angle_radians);

/**
 * Map clock position to angle
 * 
 * @param position Clock position (0-11)
 * @return Angle in radians
 */
double clock_position_to_angle(int position);

// ============================================================================
// PACKING OPERATIONS
// ============================================================================

/**
 * Calculate optimal packing density for spheres
 * 
 * @param num_spheres Number of spheres
 * @param container_radius Radius of container
 * @param sphere_radius Radius of each sphere
 * @return Packing density (0.0 to 1.0)
 */
double calculate_packing_density(int num_spheres, double container_radius, double sphere_radius);

/**
 * Find kissing number for a sphere
 * 
 * The kissing number is the maximum number of non-overlapping unit spheres
 * that can touch a central unit sphere.
 * 
 * For 3D: kissing number = 12 (fundamental to crystalline lattice)
 * 
 * @param dimension Dimension (2D or 3D)
 * @return Kissing number
 */
int get_kissing_number(int dimension);

/**
 * Generate kissing spheres around a central sphere
 * 
 * Generates spheres that kiss (touch) a central sphere.
 * In 3D, this generates 12 spheres (12-fold symmetry).
 * 
 * @param central Central sphere
 * @param kissing_spheres Output array (must be pre-allocated)
 * @param max_spheres Maximum number of spheres to generate
 * @return Number of spheres generated
 */
int generate_kissing_spheres(const Sphere3D* central, Sphere3D* kissing_spheres, int max_spheres);

#ifdef __cplusplus
}
#endif

#endif // ALGORITHMS_SPHERE_PACKING_H
