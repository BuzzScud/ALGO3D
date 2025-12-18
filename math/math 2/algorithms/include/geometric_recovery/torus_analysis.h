/**
 * @file torus_analysis.h
 * @brief Torus Intersection Curve Analysis for Geometric Recovery
 * 
 * Public API for torus analysis system.
 */

#ifndef TORUS_ANALYSIS_H
#define TORUS_ANALYSIS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct Torus Torus;
typedef struct TorusIntersection TorusIntersection;
typedef struct TorusSystem TorusSystem;

/**
 * @brief Create torus
 */
Torus* torus_create(
    const double center[3],
    double major_radius,
    double minor_radius
);

/**
 * @brief Free torus
 */
void torus_free(Torus* torus);

/**
 * @brief Create multi-torus system
 */
TorusSystem* torus_create_system(uint32_t num_tori);

/**
 * @brief Free torus system
 */
void torus_free_system(TorusSystem* system);

/**
 * @brief Add torus to system
 */
int torus_add_to_system(
    TorusSystem* system,
    const double center[3],
    double major_radius,
    double minor_radius
);

/**
 * @brief Compute all intersections in system
 */
int torus_compute_intersections(TorusSystem* system);

/**
 * @brief Analyze convergence using torus intersections
 * 
 * Returns convergence score (0.0 = no convergence, 1.0 = perfect convergence)
 */
double torus_analyze_convergence(const TorusSystem* system);

/**
 * @brief Print torus system statistics
 */
void torus_print_stats(const TorusSystem* system);

#ifdef __cplusplus
}
#endif

#endif /* TORUS_ANALYSIS_H */