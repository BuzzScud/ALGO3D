/**
 * test_anchor_estimation.c - Debug anchor k estimation
 * 
 * This test checks if the 3-anchor weighted average is giving good estimates
 */

#include "search_recovery_v2.h"
#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Test k values
static const uint64_t TEST_K_VALUES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71
};
#define NUM_TEST_K 20

int main() {
    printf("\n=== Anchor Estimation Analysis ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    printf("Created overlay with %u shared vertices\n\n", overlay->num_shared_vertices);
    
    printf("Testing anchor-based k estimation:\n");
    printf("%-4s %-8s %-12s %-12s %-12s %-12s\n",
           "k", "Angle", "Anchor1_k", "Anchor2_k", "Anchor3_k", "Weighted_k");
    printf("------------------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        // Find 3 nearest anchors
        AnchorInfo anchors[3];
        int num_anchors = find_3_nearest_anchors(pos, overlay, anchors);
        
        // Compute weighted estimate
        uint64_t weighted_k = compute_weighted_k_estimate(anchors, num_anchors);
        
        int64_t error = (int64_t)weighted_k - (int64_t)k;
        
        double angle_deg = pos.angle * 180.0 / M_PI;
        
        printf("%-4lu %-8.2f ", k, angle_deg);
        
        for (int j = 0; j < num_anchors; j++) {
            printf("%-12lu ", anchors[j].k_estimate);
        }
        for (int j = num_anchors; j < 3; j++) {
            printf("%-12s ", "-");
        }
        
        printf("%-12lu (error=%ld)\n", weighted_k, error);
    }
    
    printf("\nAnalysis:\n");
    printf("- If weighted_k is close to actual k, anchor estimation is working\n");
    printf("- If weighted_k is far from actual k, we need wider search ranges\n");
    printf("- The search range should be at least 2x the typical error\n");
    
    free_platonic_overlay(overlay);
    return 0;
}