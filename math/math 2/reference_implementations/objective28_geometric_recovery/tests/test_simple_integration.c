/**
 * test_simple_integration.c - Simple Integration Test
 * 
 * Simplified test to verify basic integration
 */

#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    printf("\n=== Simple Integration Test ===\n\n");
    
    printf("Test 1: Create Platonic overlay\n");
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED: Could not create overlay\n");
        return 1;
    }
    printf("SUCCESS: Created overlay\n");
    printf("  Shared vertices: %u\n", overlay->num_shared_vertices);
    
    printf("\nTest 2: Access Icosahedron\n");
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    printf("SUCCESS: Accessed Icosahedron\n");
    printf("  Vertices: %u\n", icosa->num_vertices);
    printf("  Name: %s\n", icosa->name);
    
    printf("\nTest 3: Check clock positions\n");
    if (icosa->clock_positions) {
        printf("SUCCESS: Clock positions exist\n");
        printf("  First vertex: ring=%d, pos=%d\n",
               icosa->clock_positions[0].ring,
               icosa->clock_positions[0].position);
    } else {
        printf("FAILED: No clock positions\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    printf("\nTest 4: Test dimensional frequencies\n");
    DimensionalFrequency dims[13];
    init_dimensional_frequencies(dims);
    printf("SUCCESS: Initialized dimensional frequencies\n");
    printf("  First frequency: %.1f\n", dims[0].frequency);
    
    printf("\nTest 5: Compute alignment\n");
    double angle = icosa->clock_positions[0].angle;
    double alignment = compute_total_alignment(angle, dims);
    printf("SUCCESS: Computed alignment\n");
    printf("  Alignment: %.6f\n", alignment);
    
    printf("\nTest 6: Test Pythagorean triple\n");
    bool is_triple = is_pythagorean_triple(
        icosa->clock_positions[0],
        icosa->clock_positions[1],
        icosa->clock_positions[2]
    );
    printf("SUCCESS: Tested Pythagorean triple\n");
    printf("  Is triple: %s\n", is_triple ? "yes" : "no");
    
    printf("\nTest 7: Cleanup\n");
    free_platonic_overlay(overlay);
    printf("SUCCESS: Cleaned up\n");
    
    printf("\n=== All Tests Passed ===\n\n");
    
    return 0;
}