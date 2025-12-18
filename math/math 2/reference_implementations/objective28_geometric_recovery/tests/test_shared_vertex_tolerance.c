/**
 * test_shared_vertex_tolerance.c - Find optimal tolerance for shared vertices
 */

#include "geometric_anchors.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("\n=== Shared Vertex Tolerance Analysis ===\n\n");
    
    // Generate anchors
    int num_anchors;
    GeometricAnchor* anchors = generate_platonic_anchors_13d(&num_anchors);
    if (!anchors) {
        printf("FAILED\n");
        return 1;
    }
    printf("Generated %d anchors\n\n", num_anchors);
    
    // Test different tolerances
    double tolerances[] = {0.01, 0.05, 0.1, 0.5, 1.0, 2.0, 5.0, 10.0};
    int num_tolerances = 8;
    
    printf("Testing different tolerances:\n");
    for (int t = 0; t < num_tolerances; t++) {
        int num_shared;
        SharedVertex* shared = find_shared_geometric_vertices(
            anchors, num_anchors, tolerances[t], &num_shared
        );
        
        printf("  Tolerance %.2f: %d shared vertices\n", tolerances[t], num_shared);
        
        if (shared) free(shared);
    }
    
    printf("\n");
    
    // Analyze pairwise distances
    printf("Analyzing pairwise distances between all anchors:\n");
    double min_dist = 1e9;
    double max_dist = 0.0;
    double sum_dist = 0.0;
    int count = 0;
    
    for (int i = 0; i < num_anchors; i++) {
        for (int j = i+1; j < num_anchors; j++) {
            if (anchors[i].solid_type == anchors[j].solid_type) continue;
            
            double dist = pi_phi_distance_13d(anchors[i].position, anchors[j].position);
            
            if (dist < min_dist) min_dist = dist;
            if (dist > max_dist) max_dist = dist;
            sum_dist += dist;
            count++;
        }
    }
    
    double avg_dist = sum_dist / count;
    
    printf("  Minimum distance: %.4f\n", min_dist);
    printf("  Maximum distance: %.4f\n", max_dist);
    printf("  Average distance: %.4f\n", avg_dist);
    printf("  Pairs analyzed: %d\n", count);
    printf("\n");
    
    printf("Recommended tolerance: %.4f (10%% of minimum distance)\n", min_dist * 0.1);
    printf("\n");
    
    free(anchors);
    
    return 0;
}