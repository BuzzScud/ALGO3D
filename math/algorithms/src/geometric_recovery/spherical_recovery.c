/**
 * @file spherical_recovery.c
 * @brief Spherical Recovery using Spherical Coordinates
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 */

#include "geometric_recovery/spherical_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Spherical recovery context
 */
struct SphericalRecoveryContext {
    uint64_t n;              // System size
    int num_spheres;         // Number of spheres
    double* sphere_radii;    // Radii of spheres
    double confidence;       // Confidence score
};

/**
 * Create spherical recovery context
 */
SphericalRecoveryContext* spherical_recovery_create(
    uint64_t n,
    int num_spheres
) {
    if (n == 0 || num_spheres <= 0) {
        return NULL;
    }
    
    SphericalRecoveryContext* ctx = (SphericalRecoveryContext*)calloc(
        1, sizeof(SphericalRecoveryContext)
    );
    if (!ctx) {
        return NULL;
    }
    
    ctx->n = n;
    ctx->num_spheres = num_spheres;
    ctx->confidence = 0.0;
    
    // Allocate sphere radii
    ctx->sphere_radii = (double*)calloc(num_spheres, sizeof(double));
    if (!ctx->sphere_radii) {
        free(ctx);
        return NULL;
    }
    
    // Initialize sphere radii (logarithmic spacing)
    for (int i = 0; i < num_spheres; i++) {
        ctx->sphere_radii[i] = math_pow(2.0, (double)i);
    }
    
    return ctx;
}

/**
 * Convert value to spherical coordinates
 */
SphericalCoords value_to_spherical(uint64_t value, uint64_t n) {
    SphericalCoords coords;
    
    if (n == 0) {
        coords.theta = 0.0;
        coords.phi = 0.0;
        coords.radius = 0.0;
        return coords;
    }
    
    // Normalize value to [0, 1)
    double normalized = (double)value / (double)n;
    
    // Map to spherical coordinates
    // theta: azimuthal angle [0, 2π)
    coords.theta = 2.0 * MATH_PI * normalized;
    
    // phi: polar angle [0, π]
    // Use golden ratio for optimal distribution
    double phi_normalized = math_fmod(normalized * MATH_PHI, 1.0);
    coords.phi = MATH_PI * phi_normalized;
    
    // radius: distance from origin
    // Use logarithmic mapping for better distribution
    coords.radius = math_log(1.0 + normalized * (math_exp(1.0) - 1.0));
    
    return coords;
}

/**
 * Convert spherical coordinates to value
 */
uint64_t spherical_to_value(SphericalCoords coords, uint64_t n) {
    if (n == 0) {
        return 0;
    }
    
    // Reverse the mapping from value_to_spherical
    
    // Extract normalized value from theta
    double normalized = coords.theta / (2.0 * MATH_PI);
    
    // Ensure in [0, 1)
    normalized = math_fmod(normalized, 1.0);
    if (normalized < 0.0) {
        normalized += 1.0;
    }
    
    // Convert to value
    uint64_t value = (uint64_t)(normalized * n);
    
    // Ensure within bounds
    if (value >= n) {
        value = n - 1;
    }
    
    return value;
}

/**
 * Compute great circle distance
 */
double compute_great_circle_distance(
    SphericalCoords coords1,
    SphericalCoords coords2
) {
    // Use Haversine formula for great circle distance
    
    double dtheta = coords2.theta - coords1.theta;
    double dphi = coords2.phi - coords1.phi;
    
    double a = math_sin(dphi / 2.0) * math_sin(dphi / 2.0) +
               math_cos(coords1.phi) * math_cos(coords2.phi) *
               math_sin(dtheta / 2.0) * math_sin(dtheta / 2.0);
    
    double c = 2.0 * math_atan2(math_sqrt(a), math_sqrt(1.0 - a));
    
    // Use average radius
    double avg_radius = (coords1.radius + coords2.radius) / 2.0;
    
    return avg_radius * c;
}

/**
 * Find geodesic path
 */
bool find_geodesic_path(
    SphericalCoords start,
    SphericalCoords end,
    int num_points,
    SphericalCoords* path_out
) {
    if (num_points <= 0 || !path_out) {
        return false;
    }
    
    // Interpolate along great circle
    for (int i = 0; i < num_points; i++) {
        double t = (double)i / (double)(num_points - 1);
        
        // Spherical linear interpolation (slerp)
        double d = compute_great_circle_distance(start, end);
        
        if (d < 1e-10) {
            // Points are very close, use linear interpolation
            path_out[i].theta = start.theta + t * (end.theta - start.theta);
            path_out[i].phi = start.phi + t * (end.phi - start.phi);
            path_out[i].radius = start.radius + t * (end.radius - start.radius);
        } else {
            // Use slerp
            double a = math_sin((1.0 - t) * d) / math_sin(d);
            double b = math_sin(t * d) / math_sin(d);
            
            // Convert to Cartesian for interpolation
            double x1 = start.radius * math_sin(start.phi) * math_cos(start.theta);
            double y1 = start.radius * math_sin(start.phi) * math_sin(start.theta);
            double z1 = start.radius * math_cos(start.phi);
            
            double x2 = end.radius * math_sin(end.phi) * math_cos(end.theta);
            double y2 = end.radius * math_sin(end.phi) * math_sin(end.theta);
            double z2 = end.radius * math_cos(end.phi);
            
            double x = a * x1 + b * x2;
            double y = a * y1 + b * y2;
            double z = a * z1 + b * z2;
            
            // Convert back to spherical
            path_out[i].radius = math_sqrt(x * x + y * y + z * z);
            path_out[i].theta = math_atan2(y, x);
            path_out[i].phi = math_acos(z / path_out[i].radius);
        }
    }
    
    return true;
}

/**
 * Recover value using spherical coordinates
 */
bool spherical_recover(
    SphericalRecoveryContext* ctx,
    uint64_t target,
    uint64_t* estimate_out
) {
    if (!ctx || !estimate_out) {
        return false;
    }
    
    // Convert target to spherical coordinates
    SphericalCoords target_coords = value_to_spherical(target, ctx->n);
    
    // Find nearest sphere
    int nearest_sphere = 0;
    double min_distance = math_abs(target_coords.radius - ctx->sphere_radii[0]);
    
    for (int i = 1; i < ctx->num_spheres; i++) {
        double distance = math_abs(target_coords.radius - ctx->sphere_radii[i]);
        if (distance < min_distance) {
            min_distance = distance;
            nearest_sphere = i;
        }
    }
    
    // Project onto nearest sphere
    SphericalCoords projected = target_coords;
    projected.radius = ctx->sphere_radii[nearest_sphere];
    
    // Convert back to value
    *estimate_out = spherical_to_value(projected, ctx->n);
    
    // Compute confidence based on distance to sphere
    ctx->confidence = math_exp(-min_distance);
    
    return true;
}

/**
 * Get confidence score
 */
double spherical_recovery_get_confidence(const SphericalRecoveryContext* ctx) {
    return ctx ? ctx->confidence : 0.0;
}

/**
 * Destroy spherical recovery context
 */
void spherical_recovery_destroy(SphericalRecoveryContext* ctx) {
    if (ctx) {
        if (ctx->sphere_radii) {
            free(ctx->sphere_radii);
        }
        free(ctx);
    }
}