/**
 * anchor_grid_24.h - 24-Anchor Grid System
 * 
 * Provides finer granularity for k estimation by using 24 anchors
 * instead of 12 from the Icosahedron.
 */

#ifndef ANCHOR_GRID_24_H
#define ANCHOR_GRID_24_H

#include <stdint.h>
#include <stdbool.h>

// Single anchor in the 24-anchor grid
typedef struct {
    double angle;        // Angle in radians [0, 2π)
    uint64_t k_value;    // K value at this anchor
    bool is_primary;     // True if from Icosahedron, false if midpoint
    uint32_t index;      // Index in grid [0, 23]
} Anchor24;

// 24-anchor grid
typedef struct {
    Anchor24* anchors;      // Array of 24 anchors
    uint32_t num_anchors;   // Always 24
    uint64_t max_k;         // Maximum k value
} AnchorGrid24;

/**
 * Create 24-anchor grid
 * 
 * Creates 24 evenly-spaced anchors:
 * - 12 primary from Icosahedron (0°, 30°, 60°, ...)
 * - 12 secondary at midpoints (15°, 45°, 75°, ...)
 */
AnchorGrid24* create_anchor_grid_24(uint64_t max_k);

/**
 * Free anchor grid
 */
void free_anchor_grid_24(AnchorGrid24* grid);

/**
 * Find 3 nearest anchors to target angle
 * 
 * Returns pointers to the 3 nearest anchors.
 * Handles angular wraparound correctly.
 */
int find_nearest_anchors_24(
    AnchorGrid24* grid,
    double target_angle,
    Anchor24* nearest[3]
);

/**
 * Compute weighted k estimate from 3 anchors
 * 
 * Uses inverse distance weighting based on angular distance.
 */
uint64_t compute_weighted_k_24(
    Anchor24* anchors[3],
    int num_anchors,
    double target_angle
);

/**
 * Print anchor grid for debugging
 */
void print_anchor_grid_24(AnchorGrid24* grid);

#endif // ANCHOR_GRID_24_H
