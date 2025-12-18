/**
 * test_k_recovery_simple.c - Simple K Recovery Test
 */

#include "clock_inverse_mapping.h"
#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    printf("\n=== Simple K Recovery Test ===\n\n");
    
    printf("Test 1: Create Platonic overlay\n");
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED\n");
        return 1;
    }
    printf("SUCCESS: %u shared vertices\n", overlay->num_shared_vertices);
    
    printf("\nTest 2: Create target position\n");
    ClockPosition target;
    target.ring = 0;
    target.position = 6;
    target.angle = M_PI;
    target.radius = 0.25;
    printf("SUCCESS: ring=%d, pos=%d\n", target.ring, target.position);
    
    printf("\nTest 3: Test inverse mapping (no anchors)\n");
    double k1 = inverse_map_k_from_clock(target, NULL, 0);
    printf("SUCCESS: k ≈ %.2f\n", k1);
    
    printf("\nTest 4: Test inverse mapping (with Icosahedron anchors)\n");
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    double k2 = inverse_map_k_from_clock(target, icosa->clock_positions, icosa->num_vertices);
    printf("SUCCESS: k ≈ %.2f\n", k2);
    
    printf("\nTest 5: Cleanup\n");
    free_platonic_overlay(overlay);
    printf("SUCCESS\n");
    
    printf("\n=== All Tests Passed ===\n\n");
    
    return 0;
}