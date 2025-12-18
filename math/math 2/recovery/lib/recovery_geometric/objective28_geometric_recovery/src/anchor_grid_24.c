/**
 * anchor_grid_24.c - 24-Anchor Grid System
 * 
 * Extends the 12-anchor Icosahedron system to 24 anchors by adding midpoints.
 * This provides finer granularity for k estimation.
 * 
 * Anchor distribution:
 * - 12 primary anchors from Icosahedron vertices (0°, 30°, 60°, ..., 330°)
 * - 12 secondary anchors at midpoints (15°, 45°, 75°, ..., 345°)
 * 
 * K mapping:
 * - Primary anchors: k = 0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275
 * - Secondary anchors: k = 12, 37, 62, 87, 112, 137, 162, 187, 212, 237, 262, 287
 */

#include "anchor_grid_24.h"
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define PI M_PI
#define PHI ((1.0 + math_sqrt(5.0)) / 2.0)

AnchorGrid24* create_anchor_grid_24(uint64_t max_k) {
    AnchorGrid24* grid = calloc(1, sizeof(AnchorGrid24));
    if (!grid) return NULL;
    
    grid->num_anchors = 24;
    grid->max_k = max_k;
    
    // Allocate anchor array
    grid->anchors = calloc(24, sizeof(Anchor24));
    if (!grid->anchors) {
        free(grid);
        return NULL;
    }
    
    // Create 24 anchors
    for (int i = 0; i < 24; i++) {
        Anchor24* anchor = &grid->anchors[i];
        
        // Angle: 0°, 15°, 30°, 45°, ..., 345°
        anchor->angle = (i * 15.0) * PI / 180.0;
        
        // K value: Evenly distributed across [0, max_k]
        anchor->k_value = (uint64_t)((double)i * max_k / 24.0);
        
        // Type: primary (even index) or secondary (odd index)
        anchor->is_primary = (i % 2 == 0);
        
        // Index
        anchor->index = i;
    }
    
    return grid;
}

void free_anchor_grid_24(AnchorGrid24* grid) {
    if (grid) {
        if (grid->anchors) {
            free(grid->anchors);
        }
        free(grid);
    }
}

int find_nearest_anchors_24(
    AnchorGrid24* grid,
    double target_angle,
    Anchor24* nearest[3]
) {
    if (!grid || !nearest) return 0;
    
    // Normalize target angle to [0, 2π)
    double normalized = math_fmod(target_angle, 2.0 * PI);
    if (normalized < 0) normalized += 2.0 * PI;
    
    // Find distances to all anchors
    typedef struct {
        double distance;
        int index;
    } DistanceIndex;
    
    DistanceIndex* distances = malloc(grid->num_anchors * sizeof(DistanceIndex));
    if (!distances) return 0;
    
    for (uint32_t i = 0; i < grid->num_anchors; i++) {
        // Angular distance (handle wraparound)
        double diff = math_abs(grid->anchors[i].angle - normalized);
        if (diff > PI) diff = 2.0 * PI - diff;
        
        distances[i].distance = diff;
        distances[i].index = i;
    }
    
    // Sort to find 3 nearest (simple selection sort for small n)
    for (int i = 0; i < 3 && i < (int)grid->num_anchors; i++) {
        for (uint32_t j = i + 1; j < grid->num_anchors; j++) {
            if (distances[j].distance < distances[i].distance) {
                DistanceIndex temp = distances[i];
                distances[i] = distances[j];
                distances[j] = temp;
            }
        }
    }
    
    // Fill nearest array
    int num_found = (grid->num_anchors < 3) ? grid->num_anchors : 3;
    for (int i = 0; i < num_found; i++) {
        nearest[i] = &grid->anchors[distances[i].index];
    }
    
    free(distances);
    return num_found;
}

uint64_t compute_weighted_k_24(
    Anchor24* anchors[3],
    int num_anchors,
    double target_angle
) {
    if (num_anchors == 0) return 0;
    
    // Normalize target angle
    double normalized = math_fmod(target_angle, 2.0 * PI);
    if (normalized < 0) normalized += 2.0 * PI;
    
    // Compute weights based on angular distance
    double total_weight = 0.0;
    double weights[3];
    
    for (int i = 0; i < num_anchors; i++) {
        // Angular distance
        double diff = math_abs(anchors[i]->angle - normalized);
        if (diff > PI) diff = 2.0 * PI - diff;
        
        // Avoid division by zero
        if (diff < 0.001) {
            return anchors[i]->k_value;
        }
        
        // Weight = 1 / distance
        weights[i] = 1.0 / diff;
        total_weight += weights[i];
    }
    
    // Normalize weights
    for (int i = 0; i < num_anchors; i++) {
        weights[i] /= total_weight;
    }
    
    // Weighted average
    double weighted_sum = 0.0;
    for (int i = 0; i < num_anchors; i++) {
        weighted_sum += weights[i] * anchors[i]->k_value;
    }
    
    return (uint64_t)weighted_sum;
}

void print_anchor_grid_24(AnchorGrid24* grid) {
    if (!grid) return;
    
    printf("\n=== 24-Anchor Grid ===\n");
    printf("Total anchors: %u\n", grid->num_anchors);
    printf("Max k: %lu\n\n", grid->max_k);
    
    printf("%-4s %-8s %-12s %-12s %-8s\n",
           "Idx", "Type", "Angle(deg)", "K Value", "Primary");
    printf("--------------------------------------------------------\n");
    
    for (uint32_t i = 0; i < grid->num_anchors; i++) {
        Anchor24* anchor = &grid->anchors[i];
        double angle_deg = anchor->angle * 180.0 / PI;
        
        printf("%-4u %-8s %-12.2f %-12lu %-8s\n",
               anchor->index,
               anchor->is_primary ? "Primary" : "Secondary",
               angle_deg,
               anchor->k_value,
               anchor->is_primary ? "Yes" : "No");
    }
}
