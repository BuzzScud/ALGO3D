#include <stdio.h>
#include <stdlib.h>
#include "ai/cllm_lattice_hierarchy.h"
#include "ai/space_88d.h"

int main() {
    printf("Testing Space88D Integration with CLLMLatticeHierarchy...\n\n");
    
    // Create a simple sphere
    int symmetry_groups[] = {0};
    CLLMLatticeHierarchy* sphere = lattice_hierarchy_create(
        1,                  // sphere_id
        0,                  // hierarchy_level
        symmetry_groups,    // symmetry_groups
        1,                  // num_symmetry_groups
        0,                  // physical_thread_id
        NULL                // parent
    );
    
    if (!sphere) {
        fprintf(stderr, "ERROR: Failed to create sphere\n");
        return 1;
    }
    
    printf("✓ Sphere created successfully\n");
    
    // Check that Space88D was initialized
    if (!sphere->space) {
        fprintf(stderr, "ERROR: Space88D not initialized\n");
        lattice_hierarchy_free(sphere);
        return 1;
    }
    
    printf("✓ Space88D initialized\n");
    
    // Verify Space88D structure
    if (sphere->space->base != 60) {
        fprintf(stderr, "ERROR: Space88D base incorrect (expected 60, got %u)\n", 
                sphere->space->base);
        lattice_hierarchy_free(sphere);
        return 1;
    }
    
    printf("✓ Space88D structure verified (base=60)\n");
    
    // Test basic operations - create abacus with value 42
    CrystallineAbacus* test_value = abacus_from_uint64(42, 60);
    if (!test_value) {
        fprintf(stderr, "ERROR: Failed to create test value\n");
        lattice_hierarchy_free(sphere);
        return 1;
    }
    
    // Set a value in the space
    if (space88d_set_value(sphere->space, 0, 0, test_value) != 0) {
        fprintf(stderr, "ERROR: Failed to set value in Space88D\n");
        abacus_free(test_value);
        lattice_hierarchy_free(sphere);
        return 1;
    }
    
    printf("✓ Successfully set value in Space88D\n");
    
    // Get the value back
    const CrystallineAbacus* retrieved = space88d_get_value(sphere->space, 0, 0);
    if (!retrieved) {
        fprintf(stderr, "ERROR: Failed to get value from Space88D\n");
        abacus_free(test_value);
        lattice_hierarchy_free(sphere);
        return 1;
    }
    
    printf("✓ Successfully retrieved value from Space88D\n");
    
    // Verify the value by comparing
    CrystallineAbacus* diff = abacus_new(60);
    MathError err = abacus_sub(diff, retrieved, test_value);
    if (err != MATH_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to compute difference\n");
        abacus_free(diff);
        abacus_free(test_value);
        lattice_hierarchy_free(sphere);
        return 1;
    }
    
    bool is_zero = abacus_is_zero(diff);
    abacus_free(diff);
    
    if (!is_zero) {
        fprintf(stderr, "ERROR: Retrieved value mismatch\n");
        abacus_free(test_value);
        lattice_hierarchy_free(sphere);
        return 1;
    }
    
    printf("✓ Value verification passed (42 == 42)\n");
    
    // Clean up
    abacus_free(test_value);
    lattice_hierarchy_free(sphere);
    
    printf("\n========================================\n");
    printf("✓ ALL TESTS PASSED\n");
    printf("========================================\n");
    printf("\nSpace88D is successfully integrated into CLLMLatticeHierarchy!\n");
    
    return 0;
}