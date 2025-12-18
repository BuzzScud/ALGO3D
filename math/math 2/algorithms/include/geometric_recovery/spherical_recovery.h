/**
 * @file spherical_recovery.h
 * @brief Spherical Recovery using Spherical Coordinates
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements recovery using spherical coordinate mapping:
 * - Maps values to spherical coordinates (θ, φ, r)
 * - Computes great circle distances
 * - Finds geodesic paths
 * - Optimal sphere selection
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#ifndef SPHERICAL_RECOVERY_H
#define SPHERICAL_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Spherical coordinates
 */
typedef struct {
    double theta;    // Azimuthal angle [0, 2π)
    double phi;      // Polar angle [0, π]
    double radius;   // Distance from origin
} SphericalCoords;

/**
 * Spherical recovery context
 */
typedef struct SphericalRecoveryContext SphericalRecoveryContext;

/**
 * Create spherical recovery context
 * 
 * @param n System size
 * @param num_spheres Number of spheres to use
 * @return New context or NULL on error
 */
SphericalRecoveryContext* spherical_recovery_create(
    uint64_t n,
    int num_spheres
);

/**
 * Convert value to spherical coordinates
 * 
 * @param value Value to convert
 * @param n System size
 * @return Spherical coordinates
 */
SphericalCoords value_to_spherical(uint64_t value, uint64_t n);

/**
 * Convert spherical coordinates to value
 * 
 * @param coords Spherical coordinates
 * @param n System size
 * @return Recovered value
 */
uint64_t spherical_to_value(SphericalCoords coords, uint64_t n);

/**
 * Compute great circle distance between two points
 * 
 * @param coords1 First point
 * @param coords2 Second point
 * @return Great circle distance
 */
double compute_great_circle_distance(
    SphericalCoords coords1,
    SphericalCoords coords2
);

/**
 * Find geodesic path between two points
 * 
 * @param start Start point
 * @param end End point
 * @param num_points Number of points on path
 * @param path_out Array to store path points
 * @return true on success, false on error
 */
bool find_geodesic_path(
    SphericalCoords start,
    SphericalCoords end,
    int num_points,
    SphericalCoords* path_out
);

/**
 * Recover value using spherical coordinates
 * 
 * @param ctx Context
 * @param target Target output value
 * @param estimate_out Pointer to store estimated input
 * @return true on success, false on error
 */
bool spherical_recover(
    SphericalRecoveryContext* ctx,
    uint64_t target,
    uint64_t* estimate_out
);

/**
 * Get confidence score
 * 
 * @param ctx Context
 * @return Confidence score [0.0, 1.0]
 */
double spherical_recovery_get_confidence(const SphericalRecoveryContext* ctx);

/**
 * Destroy spherical recovery context
 * 
 * @param ctx Context to destroy
 */
void spherical_recovery_destroy(SphericalRecoveryContext* ctx);

#endif // SPHERICAL_RECOVERY_H