/**
 * spherical_recovery.h - Spherical Coordinate Recovery System
 * 
 * This implements the correct inverse mapping using spherical coordinates
 * instead of linear corrections. Based on the discovery that:
 * - Rings = concentric spheres (radial dimension)
 * - Positions = angular sectors (angular dimension)
 * - Anchors = icosahedral vertices (optimal sampling)
 * 
 * Key insight: The 2 successful recoveries (k=5, k=199) are both in the
 * TOP-RIGHT quadrant near 12 o'clock, suggesting this is the optimal zone.
 */

#ifndef SPHERICAL_RECOVERY_H
#define SPHERICAL_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include "clock_recovery.h"
#include "platonic_solids.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Spherical coordinates in the clock lattice
 */
typedef struct {
    double r;      // Radial distance (ring)
    double theta;  // Polar angle (vertical position)
    double phi;    // Azimuthal angle (horizontal position)
} SphericalCoord;

/**
 * Quadrant information for k recovery
 */
typedef enum {
    QUADRANT_TOP_RIGHT = 0,    // Q1: 0-90° (optimal zone!)
    QUADRANT_BOTTOM_RIGHT = 1, // Q2: 90-180°
    QUADRANT_BOTTOM_LEFT = 2,  // Q3: 180-270°
    QUADRANT_TOP_LEFT = 3      // Q4: 270-360°
} Quadrant;

/**
 * Polarity (left/right indicator)
 */
typedef enum {
    POLARITY_POSITIVE = 0,  // Right side (Q1, Q4)
    POLARITY_NEGATIVE = 1   // Left side (Q2, Q3)
} Polarity;

/**
 * Vertical position (top/bottom indicator)
 */
typedef enum {
    VERTICAL_TOP = 0,     // Top half (Q1, Q4)
    VERTICAL_BOTTOM = 1   // Bottom half (Q2, Q3)
} VerticalPos;

/**
 * Spherical recovery context
 */
typedef struct {
    PlatonicOverlay* overlay;      // Platonic solid anchors
    ClockPosition* anchor_positions; // Anchor clock positions
    uint32_t num_anchors;           // Number of anchors
    
    // Quadrant filtering (focus on optimal zone)
    bool use_quadrant_filter;
    Quadrant target_quadrant;
    
    // Polarity tracking
    bool track_polarity;
    Polarity last_polarity;
    uint32_t polarity_flips;
    
    // Statistics
    uint32_t total_recoveries;
    uint32_t successful_recoveries;
    double avg_error;
} SphericalRecoveryContext;

/**
 * Initialize spherical recovery context
 */
SphericalRecoveryContext* init_spherical_recovery(
    PlatonicOverlay* overlay,
    bool use_quadrant_filter,
    Quadrant target_quadrant
);

/**
 * Free spherical recovery context
 */
void free_spherical_recovery(SphericalRecoveryContext* ctx);

/**
 * Convert clock position to spherical coordinates
 */
SphericalCoord clock_to_spherical(ClockPosition pos);

/**
 * Convert spherical coordinates to k value
 * This is the CORRECT inverse mapping using spherical geometry
 */
double spherical_to_k(
    SphericalCoord coord,
    SphericalRecoveryContext* ctx
);

/**
 * Radial transformation for ring correction
 * Maps radial distance to k contribution
 */
double radial_transform(double r, uint32_t ring);

/**
 * Angular transformation for position correction
 * Maps angular position to k contribution
 */
double angular_transform(double theta, double phi, uint32_t position);

/**
 * Spherical barycentric interpolation for anchor correction
 * Uses the 3 nearest anchors on the sphere
 */
double spherical_barycentric_interpolation(
    SphericalCoord target,
    SphericalCoord* anchors,
    double* anchor_k_values,
    uint32_t num_anchors
);

/**
 * Get quadrant from clock position
 */
Quadrant get_quadrant_from_position(ClockPosition pos);

/**
 * Get polarity from quadrant
 */
Polarity get_polarity_from_quadrant(Quadrant q);

/**
 * Get vertical position from quadrant
 */
VerticalPos get_vertical_from_quadrant(Quadrant q);

/**
 * Check if position is in optimal recovery zone
 * (TOP-RIGHT quadrant near 12 o'clock)
 */
bool is_in_optimal_zone(ClockPosition pos);

/**
 * Recover k from clock position using spherical coordinates
 * This is the main recovery function
 */
double recover_k_spherical(
    ClockPosition pos,
    SphericalRecoveryContext* ctx,
    double* confidence  // Output: confidence score [0, 1]
);

/**
 * Batch recovery with statistics
 */
void batch_recover_spherical(
    ClockPosition* positions,
    uint32_t num_positions,
    double* recovered_k_values,
    double* confidence_scores,
    SphericalRecoveryContext* ctx
);

/**
 * Get recovery statistics
 */
void get_recovery_statistics(
    SphericalRecoveryContext* ctx,
    uint32_t* total,
    uint32_t* successful,
    double* success_rate,
    double* avg_error
);

#ifdef __cplusplus
}
#endif

#endif // SPHERICAL_RECOVERY_H
