/**
 * @file anchor_grid_24.h
 * @brief 24-Cell Polytope Anchor Grid
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements 24-cell polytope structure for optimal coverage:
 * - 24 vertices in 13D space
 * - Fast nearest-neighbor search
 * - Hierarchical refinement
 * - Optimal geometric distribution
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#ifndef ANCHOR_GRID_24_H
#define ANCHOR_GRID_24_H

#include <stdint.h>
#include <stdbool.h>

#define ANCHOR_GRID_24_VERTICES 24
#define ANCHOR_GRID_24_DIMENSION 13

/**
 * 24-cell anchor grid
 */
typedef struct {
    double positions[ANCHOR_GRID_24_VERTICES][ANCHOR_GRID_24_DIMENSION];
    double coverage_radius;
    int dimension;
} AnchorGrid24;

/**
 * Create 24-cell anchor grid
 * 
 * @param dimension Dimension of space (typically 13 for clock lattice)
 * @return New anchor grid or NULL on error
 */
AnchorGrid24* create_anchor_grid_24(int dimension);

/**
 * Find nearest anchors to target point
 * 
 * @param grid Anchor grid
 * @param target Target point in 13D space
 * @param k Number of nearest anchors to find
 * @param nearest_indices Array to store indices of nearest anchors
 * @return Number of anchors found
 */
int find_nearest_anchors(
    const AnchorGrid24* grid,
    const double target[ANCHOR_GRID_24_DIMENSION],
    int k,
    int* nearest_indices
);

/**
 * Compute distance between two points
 * 
 * @param point1 First point
 * @param point2 Second point
 * @param dimension Dimension of space
 * @return Euclidean distance
 */
double compute_anchor_distance(
    const double* point1,
    const double* point2,
    int dimension
);

/**
 * Get anchor position
 * 
 * @param grid Anchor grid
 * @param index Anchor index
 * @param position_out Array to store position
 * @return true on success, false on error
 */
bool get_anchor_position(
    const AnchorGrid24* grid,
    int index,
    double position_out[ANCHOR_GRID_24_DIMENSION]
);

/**
 * Interpolate between anchors
 * 
 * @param grid Anchor grid
 * @param anchor_indices Indices of anchors to interpolate
 * @param weights Weights for each anchor
 * @param num_anchors Number of anchors
 * @param result_out Array to store interpolated position
 * @return true on success, false on error
 */
bool interpolate_anchors(
    const AnchorGrid24* grid,
    const int* anchor_indices,
    const double* weights,
    int num_anchors,
    double result_out[ANCHOR_GRID_24_DIMENSION]
);

/**
 * Print anchor grid information
 * 
 * @param grid Anchor grid to print
 */
void print_anchor_grid_24(const AnchorGrid24* grid);

/**
 * Destroy anchor grid
 * 
 * @param grid Anchor grid to destroy
 */
void destroy_anchor_grid_24(AnchorGrid24* grid);

#endif // ANCHOR_GRID_24_H