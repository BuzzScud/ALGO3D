/**
 * Sphere Packing Geometry - Implementation
 * 
 * Generic geometric operations for sphere packing.
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_sqrt with math_sqrt (5 calls)
 * - Replaced math_sin with math_sin (1 call)
 * - Replaced math_cos with math_cos (1 call)
 * - Replaced math_abs with math_abs (1 call)
 * Total: 8 function calls migrated to NEW math library
 * 
 * PHASE 2: Consolidated constants
 * - Removed local MATH_TWO_PI definition
 * - Using MATH_TWO_PI from math/types.h
 */

#include "sphere_packing.h"
#include "math/constants.h"
#include "math/transcendental.h"  // PHASE 1: NEW math library
#include "math/constants.h"
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include "math/constants.h"
#include "math/types.h"            // PHASE 2: For MATH_TWO_PI
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Constants
#define TWELVE_FOLD_SYMMETRY 12

// ============================================================================
// SPHERE OPERATIONS
// ============================================================================

bool spheres_are_kissing(const Sphere3D* s1, const Sphere3D* s2, double tolerance) {
    if (!s1 || !s2) return false;
    
    // Calculate distance between centers
    double dx = s2->center_x - s1->center_x;
    double dy = s2->center_y - s1->center_y;
    double dz = s2->center_z - s1->center_z;
    double distance = math_sqrt(dx * dx + dy * dy + dz * dz);
    
    // Sum of radii
    double sum_radii = s1->radius + s2->radius;
    
    // Kissing if distance ≈ sum of radii (within tolerance)
    double diff = math_abs(distance - sum_radii);
    return (diff <= tolerance);
}

double sphere_gap(const Sphere3D* s1, const Sphere3D* s2) {
    if (!s1 || !s2) return 0.0;
    
    // Calculate distance between centers
    double dx = s2->center_x - s1->center_x;
    double dy = s2->center_y - s1->center_y;
    double dz = s2->center_z - s1->center_z;
    double distance = math_sqrt(dx * dx + dy * dy + dz * dz);
    
    // Gap = distance - sum of radii
    // This gap encodes the curvature of π
    double sum_radii = s1->radius + s2->radius;
    return distance - sum_radii;
}

double sphere_distance(const Sphere3D* s1, const Sphere3D* s2) {
    if (!s1 || !s2) return 0.0;
    
    double dx = s2->center_x - s1->center_x;
    double dy = s2->center_y - s1->center_y;
    double dz = s2->center_z - s1->center_z;
    
    return math_sqrt(dx * dx + dy * dy + dz * dz);
}

bool spheres_overlap(const Sphere3D* s1, const Sphere3D* s2) {
    if (!s1 || !s2) return false;
    
    double distance = sphere_distance(s1, s2);
    double sum_radii = s1->radius + s2->radius;
    
    return (distance < sum_radii);
}

double sphere_volume(const Sphere3D* sphere) {
    if (!sphere) return 0.0;
    
    // V = 4/3 * π * r³
    double r3 = sphere->radius * sphere->radius * sphere->radius;
    return (4.0 / 3.0) * MATH_PI * r3;
}

double sphere_surface_area(const Sphere3D* sphere) {
    if (!sphere) return 0.0;
    
    // A = 4 * π * r²
    double r2 = sphere->radius * sphere->radius;
    return 4.0 * MATH_PI * r2;
}

// ============================================================================
// VECTOR OPERATIONS - 2D
// ============================================================================

void vector2d_add(const Vec2D* a, const Vec2D* b, Vec2D* result) {
    if (!a || !b || !result) return;
    
    result->x = a->x + b->x;
    result->y = a->y + b->y;
}

void vector2d_subtract(const Vec2D* a, const Vec2D* b, Vec2D* result) {
    if (!a || !b || !result) return;
    
    result->x = a->x - b->x;
    result->y = a->y - b->y;
}

void vector2d_scale(const Vec2D* v, double scalar, Vec2D* result) {
    if (!v || !result) return;
    
    result->x = v->x * scalar;
    result->y = v->y * scalar;
}

double vector2d_dot(const Vec2D* a, const Vec2D* b) {
    if (!a || !b) return 0.0;
    
    return a->x * b->x + a->y * b->y;
}

double vector2d_magnitude(const Vec2D* v) {
    if (!v) return 0.0;
    
    return math_sqrt(v->x * v->x + v->y * v->y);
}

bool vector2d_normalize(const Vec2D* v, Vec2D* result) {
    if (!v || !result) return false;
    
    double mag = vector2d_magnitude(v);
    if (mag < 1e-10) return false; // Zero vector
    
    result->x = v->x / mag;
    result->y = v->y / mag;
    return true;
}

// ============================================================================
// VECTOR OPERATIONS - 3D
// ============================================================================

void vector3d_add(const Vec3D* a, const Vec3D* b, Vec3D* result) {
    if (!a || !b || !result) return;
    
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;
}

void vector3d_subtract(const Vec3D* a, const Vec3D* b, Vec3D* result) {
    if (!a || !b || !result) return;
    
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
}

void vector3d_scale(const Vec3D* v, double scalar, Vec3D* result) {
    if (!v || !result) return;
    
    result->x = v->x * scalar;
    result->y = v->y * scalar;
    result->z = v->z * scalar;
}

double vector3d_dot(const Vec3D* a, const Vec3D* b) {
    if (!a || !b) return 0.0;
    
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

void vector3d_cross(const Vec3D* a, const Vec3D* b, Vec3D* result) {
    if (!a || !b || !result) return;
    
    result->x = a->y * b->z - a->z * b->y;
    result->y = a->z * b->x - a->x * b->z;
    result->z = a->x * b->y - a->y * b->x;
}

double vector3d_magnitude(const Vec3D* v) {
    if (!v) return 0.0;
    
    return math_sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

bool vector3d_normalize(const Vec3D* v, Vec3D* result) {
    if (!v || !result) return false;
    
    double mag = vector3d_magnitude(v);
    if (mag < 1e-10) return false; // Zero vector
    
    result->x = v->x / mag;
    result->y = v->y / mag;
    result->z = v->z / mag;
    return true;
}

// ============================================================================
// LATTICE OPERATIONS
// ============================================================================

int map_to_radial_line(uint64_t value) {
    // Map to one of 12 radial lines (12-fold symmetry)
    return (int)(value % TWELVE_FOLD_SYMMETRY);
}

int map_to_concentric_ring(uint64_t value) {
    // Approximate ring number using square root
    // Ring 0: value 0
    // Ring 1: values 1-12
    // Ring 2: values 13-36
    // etc.
    
    if (value == 0) return 0;
    
    // Use integer square root approximation
    uint64_t ring = 0;
    uint64_t threshold = TWELVE_FOLD_SYMMETRY;
    
    while (value > threshold) {
        ring++;
        threshold += TWELVE_FOLD_SYMMETRY * (ring + 1);
    }
    
    return (int)ring;
}

bool is_on_radial_line(uint64_t value, int line_index) {
    if (line_index < 0 || line_index >= TWELVE_FOLD_SYMMETRY) {
        return false;
    }
    
    return (map_to_radial_line(value) == line_index);
}

bool is_on_concentric_ring(uint64_t value, int ring_number) {
    if (ring_number < 0) return false;
    
    return (map_to_concentric_ring(value) == ring_number);
}

int map_to_symmetry_group(uint64_t value) {
    // Map to one of 12 symmetry groups (fundamental to crystalline lattice)
    return (int)(value % TWELVE_FOLD_SYMMETRY);
}

// ============================================================================
// ANGULAR OPERATIONS
// ============================================================================

double map_to_angle(uint64_t value) {
    // Map value to angle using 12-fold symmetry
    int group = map_to_symmetry_group(value);
    return (double)group * MATH_TWO_PI / (double)TWELVE_FOLD_SYMMETRY;
}

int angle_to_clock_position(double angle_radians) {
    // Normalize angle to [0, 2π)
    while (angle_radians < 0) angle_radians += MATH_TWO_PI;
    while (angle_radians >= MATH_TWO_PI) angle_radians -= MATH_TWO_PI;
    
    // Map to 12 positions (0-11)
    // 0 = 12 o'clock (top), 3 = 3 o'clock (right), etc.
    int position = (int)(angle_radians * TWELVE_FOLD_SYMMETRY / MATH_TWO_PI);
    
    if (position >= TWELVE_FOLD_SYMMETRY) position = TWELVE_FOLD_SYMMETRY - 1;
    if (position < 0) position = 0;
    
    return position;
}

double clock_position_to_angle(int position) {
    if (position < 0 || position >= TWELVE_FOLD_SYMMETRY) {
        return 0.0;
    }
    
    return (double)position * MATH_TWO_PI / (double)TWELVE_FOLD_SYMMETRY;
}

// ============================================================================
// PACKING OPERATIONS
// ============================================================================

double calculate_packing_density(int num_spheres, double container_radius, double sphere_radius) {
    if (num_spheres <= 0 || container_radius <= 0.0 || sphere_radius <= 0.0) {
        return 0.0;
    }
    
    // Volume of container sphere
    double container_volume = (4.0 / 3.0) * MATH_PI * 
                             container_radius * container_radius * container_radius;
    
    // Volume of packed spheres
    double sphere_volume = (4.0 / 3.0) * MATH_PI * 
                          sphere_radius * sphere_radius * sphere_radius;
    double total_sphere_volume = (double)num_spheres * sphere_volume;
    
    // Packing density
    return total_sphere_volume / container_volume;
}

int get_kissing_number(int dimension) {
    switch (dimension) {
        case 2:
            return 6;  // 2D: hexagonal packing
        case 3:
            return 12; // 3D: FCC/HCP packing (fundamental to crystalline lattice)
        case 4:
            return 24; // 4D: D4 lattice
        case 8:
            return 240; // 8D: E8 lattice
        case 24:
            return 196560; // 24D: Leech lattice
        default:
            return -1; // Unknown
    }
}

int generate_kissing_spheres(const Sphere3D* central, Sphere3D* kissing_spheres, int max_spheres) {
    if (!central || !kissing_spheres || max_spheres < 12) {
        return 0;
    }
    
    // Generate 12 kissing spheres using 12-fold symmetry
    // Place them at equal angles around the central sphere
    
    double distance = 2.0 * central->radius; // Centers are 2r apart for kissing
    
    for (int i = 0; i < 12; i++) {
        double angle = (double)i * MATH_TWO_PI / 12.0;
        
        // Place sphere on XY plane at angle
        kissing_spheres[i].center_x = central->center_x + distance * math_cos(angle);
        kissing_spheres[i].center_y = central->center_y + distance * math_sin(angle);
        kissing_spheres[i].center_z = central->center_z; // Same Z level
        kissing_spheres[i].radius = central->radius;
        kissing_spheres[i].id = i + 1;
    }
    
    return 12;
}
