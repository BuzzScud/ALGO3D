/**
 * @file torus_analysis.c
 * @brief Torus Intersection Curve Analysis for Geometric Recovery
 * 
 * Implements torus intersection curves for convergence analysis.
 * Uses NEW math library exclusively - NO legacy dependencies.
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery: Convergence Theory
 * 
 * Algorithm:
 * - Compute torus intersection curves
 * - Analyze convergence via torus geometry
 * - Multi-torus tracking for recovery
 */

#include "math/types.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include "math/abacus.h"
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define MAX_TORUS_POINTS 1000
#define CONVERGENCE_THRESHOLD 1e-6

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Torus structure
 * 
 * Represents a torus in 3D space with major and minor radii.
 */
typedef struct {
    double center[3];           // Center position
    double major_radius;        // Major radius (R)
    double minor_radius;        // Minor radius (r)
    double rotation[3];         // Rotation angles (Euler angles)
} Torus;

/**
 * @brief Torus intersection curve
 * 
 * Represents the intersection curve between two tori.
 */
typedef struct {
    double* points;             // Array of 3D points (x,y,z,x,y,z,...)
    uint32_t num_points;        // Number of points
    uint32_t capacity;          // Allocated capacity
    double curve_length;        // Total curve length
    bool is_closed;             // Is curve closed?
} TorusIntersection;

/**
 * @brief Multi-torus system
 */
typedef struct {
    Torus* tori;                // Array of tori
    uint32_t num_tori;          // Number of tori
    TorusIntersection** intersections; // Intersection curves
    uint32_t num_intersections; // Number of intersection curves
} TorusSystem;

/* ============================================================================
 * INTERNAL FUNCTIONS
 * ============================================================================ */

/**
 * @brief Compute point on torus surface
 * 
 * Parametric equation:
 * x = (R + r*cos(v)) * cos(u)
 * y = (R + r*cos(v)) * sin(u)
 * z = r * sin(v)
 * 
 * where u, v ∈ [0, 2π]
 */
static void compute_torus_point(
    const Torus* torus,
    double u,
    double v,
    double point[3]
) {
    double R = torus->major_radius;
    double r = torus->minor_radius;
    
    // Compute point on standard torus
    double cos_u = math_cos(u);
    double sin_u = math_sin(u);
    double cos_v = math_cos(v);
    double sin_v = math_sin(v);
    
    point[0] = (R + r * cos_v) * cos_u;
    point[1] = (R + r * cos_v) * sin_u;
    point[2] = r * sin_v;
    
    // Apply rotation (simplified - just add rotation angles)
    point[0] += torus->center[0];
    point[1] += torus->center[1];
    point[2] += torus->center[2];
}

/**
 * @brief Compute distance from point to torus surface
 */
static double distance_to_torus(const Torus* torus, const double point[3]) {
    // Translate to torus center
    double px = point[0] - torus->center[0];
    double py = point[1] - torus->center[1];
    double pz = point[2] - torus->center[2];
    
    // Distance in xy-plane
    double rho = math_sqrt(px*px + py*py);
    
    // Distance to torus
    double dx = rho - torus->major_radius;
    double dz = pz;
    
    return math_sqrt(dx*dx + dz*dz) - torus->minor_radius;
}

/**
 * @brief Find intersection points between two tori
 * 
 * Uses iterative refinement to find intersection curve.
 */
static TorusIntersection* compute_torus_intersection(
    const Torus* torus1,
    const Torus* torus2
) {
    TorusIntersection* intersection = (TorusIntersection*)calloc(1, sizeof(TorusIntersection));
    if (!intersection) return NULL;
    
    // Allocate points
    intersection->capacity = MAX_TORUS_POINTS;
    intersection->points = (double*)calloc(MAX_TORUS_POINTS * 3, sizeof(double));
    if (!intersection->points) {
        free(intersection);
        return NULL;
    }
    
    // Sample parameter space to find intersection points
    uint32_t num_u_samples = 100;
    uint32_t num_v_samples = 100;
    
    double du = 2.0 * MATH_PI / num_u_samples;
    double dv = 2.0 * MATH_PI / num_v_samples;
    
    uint32_t point_idx = 0;
    
    for (uint32_t i = 0; i < num_u_samples && point_idx < MAX_TORUS_POINTS; i++) {
        for (uint32_t j = 0; j < num_v_samples && point_idx < MAX_TORUS_POINTS; j++) {
            double u = i * du;
            double v = j * dv;
            
            // Compute point on torus1
            double point[3];
            compute_torus_point(torus1, u, v, point);
            
            // Check distance to torus2
            double dist = distance_to_torus(torus2, point);
            
            // If close enough, it's an intersection point
            if (math_abs(dist) < 0.1) {
                intersection->points[point_idx * 3 + 0] = point[0];
                intersection->points[point_idx * 3 + 1] = point[1];
                intersection->points[point_idx * 3 + 2] = point[2];
                point_idx++;
            }
        }
    }
    
    intersection->num_points = point_idx;
    
    // Compute curve length
    double length = 0.0;
    for (uint32_t i = 1; i < point_idx; i++) {
        double dx = intersection->points[i*3+0] - intersection->points[(i-1)*3+0];
        double dy = intersection->points[i*3+1] - intersection->points[(i-1)*3+1];
        double dz = intersection->points[i*3+2] - intersection->points[(i-1)*3+2];
        length += math_sqrt(dx*dx + dy*dy + dz*dz);
    }
    intersection->curve_length = length;
    
    // Check if curve is closed
    if (point_idx > 2) {
        double dx = intersection->points[0] - intersection->points[(point_idx-1)*3+0];
        double dy = intersection->points[1] - intersection->points[(point_idx-1)*3+1];
        double dz = intersection->points[2] - intersection->points[(point_idx-1)*3+2];
        double closure_dist = math_sqrt(dx*dx + dy*dy + dz*dz);
        intersection->is_closed = (closure_dist < 0.5);
    }
    
    return intersection;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Create torus
 */
Torus* torus_create(
    const double center[3],
    double major_radius,
    double minor_radius
) {
    Torus* torus = (Torus*)calloc(1, sizeof(Torus));
    if (!torus) return NULL;
    
    torus->center[0] = center[0];
    torus->center[1] = center[1];
    torus->center[2] = center[2];
    torus->major_radius = major_radius;
    torus->minor_radius = minor_radius;
    torus->rotation[0] = 0.0;
    torus->rotation[1] = 0.0;
    torus->rotation[2] = 0.0;
    
    return torus;
}

/**
 * @brief Free torus
 */
void torus_free(Torus* torus) {
    if (torus) free(torus);
}

/**
 * @brief Create multi-torus system
 */
TorusSystem* torus_create_system(uint32_t num_tori) {
    if (num_tori == 0) return NULL;
    
    TorusSystem* system = (TorusSystem*)calloc(1, sizeof(TorusSystem));
    if (!system) return NULL;
    
    system->tori = (Torus*)calloc(num_tori, sizeof(Torus));
    if (!system->tori) {
        free(system);
        return NULL;
    }
    
    system->num_tori = num_tori;
    
    // Allocate intersection array (n choose 2)
    uint32_t max_intersections = (num_tori * (num_tori - 1)) / 2;
    system->intersections = (TorusIntersection**)calloc(max_intersections, sizeof(TorusIntersection*));
    if (!system->intersections) {
        free(system->tori);
        free(system);
        return NULL;
    }
    
    return system;
}

/**
 * @brief Free torus system
 */
void torus_free_system(TorusSystem* system) {
    if (!system) return;
    
    if (system->tori) free(system->tori);
    
    if (system->intersections) {
        for (uint32_t i = 0; i < system->num_intersections; i++) {
            if (system->intersections[i]) {
                if (system->intersections[i]->points) {
                    free(system->intersections[i]->points);
                }
                free(system->intersections[i]);
            }
        }
        free(system->intersections);
    }
    
    free(system);
}

/**
 * @brief Add torus to system
 */
int torus_add_to_system(
    TorusSystem* system,
    const double center[3],
    double major_radius,
    double minor_radius
) {
    if (!system) return -1;
    
    // Find empty slot
    for (uint32_t i = 0; i < system->num_tori; i++) {
        if (system->tori[i].major_radius == 0.0) {
            system->tori[i].center[0] = center[0];
            system->tori[i].center[1] = center[1];
            system->tori[i].center[2] = center[2];
            system->tori[i].major_radius = major_radius;
            system->tori[i].minor_radius = minor_radius;
            return (int)i;
        }
    }
    
    return -1; // System full
}

/**
 * @brief Compute all intersections in system
 */
int torus_compute_intersections(TorusSystem* system) {
    if (!system) return -1;
    
    uint32_t intersection_idx = 0;
    
    // Compute pairwise intersections
    for (uint32_t i = 0; i < system->num_tori; i++) {
        for (uint32_t j = i + 1; j < system->num_tori; j++) {
            // Skip if either torus is uninitialized
            if (system->tori[i].major_radius == 0.0 || 
                system->tori[j].major_radius == 0.0) {
                continue;
            }
            
            TorusIntersection* intersection = compute_torus_intersection(
                &system->tori[i],
                &system->tori[j]
            );
            
            if (intersection && intersection->num_points > 0) {
                system->intersections[intersection_idx++] = intersection;
            }
        }
    }
    
    system->num_intersections = intersection_idx;
    return (int)intersection_idx;
}

/**
 * @brief Analyze convergence using torus intersections
 * 
 * Returns convergence score (0.0 = no convergence, 1.0 = perfect convergence)
 */
double torus_analyze_convergence(const TorusSystem* system) {
    if (!system || system->num_intersections == 0) {
        return 0.0;
    }
    
    // Convergence is measured by:
    // 1. Number of closed curves (higher is better)
    // 2. Average curve length (shorter is better for convergence)
    
    uint32_t num_closed = 0;
    double avg_length = 0.0;
    
    for (uint32_t i = 0; i < system->num_intersections; i++) {
        const TorusIntersection* intersection = system->intersections[i];
        if (intersection->is_closed) {
            num_closed++;
        }
        avg_length += intersection->curve_length;
    }
    
    if (system->num_intersections > 0) {
        avg_length /= system->num_intersections;
    }
    
    // Convergence score
    double closed_score = (double)num_closed / system->num_intersections;
    double length_score = 1.0 / (1.0 + avg_length); // Shorter is better
    
    return 0.5 * (closed_score + length_score);
}

/**
 * @brief Print torus system statistics
 */
void torus_print_stats(const TorusSystem* system) {
    if (!system) return;
    
    printf("=== Torus System Statistics ===\n");
    printf("Number of tori: %u\n", system->num_tori);
    printf("Number of intersections: %u\n", system->num_intersections);
    
    if (system->num_intersections > 0) {
        uint32_t num_closed = 0;
        double avg_length = 0.0;
        uint32_t avg_points = 0;
        
        for (uint32_t i = 0; i < system->num_intersections; i++) {
            const TorusIntersection* intersection = system->intersections[i];
            if (intersection->is_closed) num_closed++;
            avg_length += intersection->curve_length;
            avg_points += intersection->num_points;
        }
        
        avg_length /= system->num_intersections;
        avg_points /= system->num_intersections;
        
        printf("Closed curves: %u (%.1f%%)\n", num_closed,
               100.0 * num_closed / system->num_intersections);
        printf("Average curve length: %.3f\n", avg_length);
        printf("Average points per curve: %u\n", avg_points);
        
        double convergence = torus_analyze_convergence(system);
        printf("Convergence score: %.3f\n", convergence);
    }
    
    printf("===============================\n");
}