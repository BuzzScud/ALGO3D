/**
 * @file anchor_grid_24.c
 * @brief 24-Cell Polytope Anchor Grid
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 */

#include "geometric_recovery/anchor_grid_24.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Generate 24-cell polytope vertices
 * 
 * The 24-cell is a regular 4D polytope with 24 vertices.
 * We extend it to 13D by embedding in higher dimensions.
 */
static void generate_24cell_vertices(double positions[24][13]) {
    // Initialize all positions to zero
    memset(positions, 0, sizeof(double) * 24 * 13);
    
    // Generate 24-cell vertices in 4D, then extend to 13D
    // The 24-cell has vertices at:
    // - 8 vertices: (±1, ±1, 0, 0) and permutations
    // - 16 vertices: (±1, 0, 0, 0) and permutations
    
    int idx = 0;
    
    // 8 vertices: (±1, ±1, 0, 0) and permutations
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (idx < 24) {
                positions[idx][i] = 1.0;
                positions[idx][j] = 1.0;
                idx++;
            }
            if (idx < 24) {
                positions[idx][i] = 1.0;
                positions[idx][j] = -1.0;
                idx++;
            }
        }
    }
    
    // 16 vertices: (±2, 0, 0, 0) and permutations
    for (int i = 0; i < 4 && idx < 24; i++) {
        positions[idx][i] = 2.0;
        idx++;
        if (idx < 24) {
            positions[idx][i] = -2.0;
            idx++;
        }
    }
    
    // Normalize all vertices to unit sphere
    for (int i = 0; i < 24; i++) {
        double norm = 0.0;
        for (int j = 0; j < 13; j++) {
            norm += positions[i][j] * positions[i][j];
        }
        norm = math_sqrt(norm);
        
        if (norm > 1e-10) {
            for (int j = 0; j < 13; j++) {
                positions[i][j] /= norm;
            }
        }
    }
    
    // Distribute remaining dimensions using golden ratio
    for (int i = 0; i < 24; i++) {
        for (int j = 4; j < 13; j++) {
            double angle = 2.0 * MATH_PI * i / 24.0 + j * MATH_PHI;
            positions[i][j] = 0.1 * math_cos(angle);  // Small contribution from higher dimensions
        }
    }
}

/**
 * Create 24-cell anchor grid
 */
AnchorGrid24* create_anchor_grid_24(int dimension) {
    if (dimension != ANCHOR_GRID_24_DIMENSION) {
        fprintf(stderr, "Warning: anchor_grid_24 designed for 13D, got %dD\n", dimension);
    }
    
    AnchorGrid24* grid = (AnchorGrid24*)calloc(1, sizeof(AnchorGrid24));
    if (!grid) {
        return NULL;
    }
    
    grid->dimension = dimension;
    
    // Generate 24-cell vertices
    generate_24cell_vertices(grid->positions);
    
    // Compute coverage radius (maximum distance between any two vertices)
    grid->coverage_radius = 0.0;
    for (int i = 0; i < ANCHOR_GRID_24_VERTICES; i++) {
        for (int j = i + 1; j < ANCHOR_GRID_24_VERTICES; j++) {
            double dist = compute_anchor_distance(
                grid->positions[i],
                grid->positions[j],
                dimension
            );
            if (dist > grid->coverage_radius) {
                grid->coverage_radius = dist;
            }
        }
    }
    
    return grid;
}

/**
 * Compute distance between two points
 */
double compute_anchor_distance(
    const double* point1,
    const double* point2,
    int dimension
) {
    double sum = 0.0;
    for (int i = 0; i < dimension; i++) {
        double diff = point1[i] - point2[i];
        sum += diff * diff;
    }
    return math_sqrt(sum);
}

/**
 * Find nearest anchors
 */
int find_nearest_anchors(
    const AnchorGrid24* grid,
    const double target[ANCHOR_GRID_24_DIMENSION],
    int k,
    int* nearest_indices
) {
    if (!grid || !target || !nearest_indices || k <= 0) {
        return 0;
    }
    
    // Limit k to number of vertices
    if (k > ANCHOR_GRID_24_VERTICES) {
        k = ANCHOR_GRID_24_VERTICES;
    }
    
    // Compute distances to all anchors
    double distances[ANCHOR_GRID_24_VERTICES];
    for (int i = 0; i < ANCHOR_GRID_24_VERTICES; i++) {
        distances[i] = compute_anchor_distance(
            grid->positions[i],
            target,
            grid->dimension
        );
    }
    
    // Find k nearest using selection
    for (int i = 0; i < k; i++) {
        int min_idx = i;
        double min_dist = distances[i];
        
        for (int j = i + 1; j < ANCHOR_GRID_24_VERTICES; j++) {
            if (distances[j] < min_dist) {
                min_dist = distances[j];
                min_idx = j;
            }
        }
        
        // Swap
        if (min_idx != i) {
            double temp_dist = distances[i];
            distances[i] = distances[min_idx];
            distances[min_idx] = temp_dist;
        }
        
        nearest_indices[i] = min_idx;
    }
    
    return k;
}

/**
 * Get anchor position
 */
bool get_anchor_position(
    const AnchorGrid24* grid,
    int index,
    double position_out[ANCHOR_GRID_24_DIMENSION]
) {
    if (!grid || !position_out || index < 0 || index >= ANCHOR_GRID_24_VERTICES) {
        return false;
    }
    
    memcpy(position_out, grid->positions[index], sizeof(double) * ANCHOR_GRID_24_DIMENSION);
    return true;
}

/**
 * Interpolate between anchors
 */
bool interpolate_anchors(
    const AnchorGrid24* grid,
    const int* anchor_indices,
    const double* weights,
    int num_anchors,
    double result_out[ANCHOR_GRID_24_DIMENSION]
) {
    if (!grid || !anchor_indices || !weights || !result_out || num_anchors <= 0) {
        return false;
    }
    
    // Initialize result to zero
    memset(result_out, 0, sizeof(double) * ANCHOR_GRID_24_DIMENSION);
    
    // Weighted sum of anchor positions
    double weight_sum = 0.0;
    for (int i = 0; i < num_anchors; i++) {
        int idx = anchor_indices[i];
        if (idx < 0 || idx >= ANCHOR_GRID_24_VERTICES) {
            continue;
        }
        
        double w = weights[i];
        weight_sum += w;
        
        for (int j = 0; j < ANCHOR_GRID_24_DIMENSION; j++) {
            result_out[j] += w * grid->positions[idx][j];
        }
    }
    
    // Normalize by total weight
    if (weight_sum > 1e-10) {
        for (int j = 0; j < ANCHOR_GRID_24_DIMENSION; j++) {
            result_out[j] /= weight_sum;
        }
    }
    
    return true;
}

/**
 * Print anchor grid information
 */
void print_anchor_grid_24(const AnchorGrid24* grid) {
    if (!grid) return;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  24-Cell Anchor Grid                                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Dimension: %d\n", grid->dimension);
    printf("  Number of Anchors: %d\n", ANCHOR_GRID_24_VERTICES);
    printf("  Coverage Radius: %.6f\n\n", grid->coverage_radius);
    
    printf("  Anchor Positions (first 5):\n");
    for (int i = 0; i < 5 && i < ANCHOR_GRID_24_VERTICES; i++) {
        printf("    Anchor %2d: [", i);
        for (int j = 0; j < 4; j++) {  // Print first 4 dimensions
            printf("%.3f", grid->positions[i][j]);
            if (j < 3) printf(", ");
        }
        printf(", ...]\n");
    }
    
    printf("\n");
}

/**
 * Destroy anchor grid
 */
void destroy_anchor_grid_24(AnchorGrid24* grid) {
    if (grid) {
        free(grid);
    }
}