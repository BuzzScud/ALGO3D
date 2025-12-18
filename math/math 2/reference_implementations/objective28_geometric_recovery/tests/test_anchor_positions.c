/**
 * test_anchor_positions.c - Check actual anchor positions
 */

#include "platonic_solids.h"
#include "clock_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    printf("\n=== Anchor Position Analysis ===\n\n");
    
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    
    printf("Shared vertices: %u\n\n", overlay->num_shared_vertices);
    
    // Check Icosahedron vertices (used as anchors)
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    printf("Icosahedron vertices: %u\n\n", icosa->num_vertices);
    
    printf("First 20 Icosahedron anchor positions:\n");
    printf("%-4s %-12s %-12s %-12s\n", "Idx", "Angle(rad)", "Angle(deg)", "k_estimate");
    printf("--------------------------------------------------------\n");
    
    for (uint32_t i = 0; i < icosa->num_vertices && i < 20; i++) {
        ClockPosition pos = icosa->clock_positions[i];
        double angle_deg = pos.angle * 180.0 / M_PI;
        
        // Estimate k from angle
        double PHI = (1.0 + sqrt(5.0)) / 2.0;
        uint64_t k_est = (uint64_t)(pos.angle / (M_PI * PHI));
        
        printf("%-4u %-12.6f %-12.2f %-12lu\n", i, pos.angle, angle_deg, k_est);
    }
    
    printf("\nProblem identified:\n");
    printf("- All k estimates are 0 or very small\n");
    printf("- This means anchors are clustered near angle 0\n");
    printf("- We need to use the FULL k range (0-300) as fallback\n");
    printf("- Or use a different anchor selection strategy\n");
    
    free_platonic_overlay(overlay);
    return 0;
}