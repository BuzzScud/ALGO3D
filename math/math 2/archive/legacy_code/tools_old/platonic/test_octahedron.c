/**
 * Test Octahedron Platonic Model
 * 
 * Verifies:
 * - Model creation
 * - Geometry (Euler's formula)
 * - Dimensions
 * - Dual relationship with cube
 * - Validation
 */

#include "ai/cllm_platonic.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("\n=== Octahedron Platonic Model Test ===\n\n");
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test 1: Create Configuration
    printf("Test 1: Create Configuration... ");
    tests_total++;
    PlatonicModelConfig config = platonic_config_create(
        PLATONIC_OCTAHEDRON,
        1000,  // vocab_size
        512    // max_seq_len
    );
    if (config.solid_type == PLATONIC_OCTAHEDRON) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 2: Get Geometry
    printf("Test 2: Get Geometry... ");
    tests_total++;
    PlatonicGeometry geometry = platonic_get_geometry(PLATONIC_OCTAHEDRON);
    if (geometry.vertices == 6 && geometry.edges == 12 && geometry.faces == 8) {
        printf("PASS (V=%u, E=%u, F=%u)\n", 
               geometry.vertices, geometry.edges, geometry.faces);
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 3: Verify Euler's Formula
    printf("Test 3: Euler's Formula (V - E + F = 2)... ");
    tests_total++;
    if (platonic_verify_euler(&geometry)) {
        int euler = geometry.vertices - geometry.edges + geometry.faces;
        printf("PASS (%u - %u + %u = %d)\n", 
               geometry.vertices, geometry.edges, geometry.faces, euler);
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 4: Verify Dual Relationship with Cube
    printf("Test 4: Dual Relationship with Cube... ");
    tests_total++;
    PlatonicGeometry cube_geom = platonic_get_geometry(PLATONIC_CUBE);
    // In dual relationship: vertices ↔ faces, edges stay same
    if (geometry.vertices == cube_geom.faces &&
        geometry.faces == cube_geom.vertices &&
        geometry.edges == cube_geom.edges) {
        printf("PASS\n");
        printf("  Octahedron: V=%u, E=%u, F=%u\n", 
               geometry.vertices, geometry.edges, geometry.faces);
        printf("  Cube:       V=%u, E=%u, F=%u\n",
               cube_geom.vertices, cube_geom.edges, cube_geom.faces);
        printf("  Dual verified: V_oct = F_cube, F_oct = V_cube\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 5: Model Creation
    printf("Test 5: Model Creation... ");
    tests_total++;
    PlatonicModel* model = platonic_model_create(&config);
    if (model) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 6: Dimension Verification
    printf("Test 6: Dimension Verification... ");
    tests_total++;
    if (model->config.embedding_dim == 72 && 
        model->config.hidden_dim == 144 && 
        model->config.num_layers == 8) {
        printf("PASS\n");
        printf("  Embedding: %u (6×12)\n", model->config.embedding_dim);
        printf("  Hidden: %u (12×12)\n", model->config.hidden_dim);
        printf("  Layers: %u\n", model->config.num_layers);
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 7: Model Validation
    printf("Test 7: Model Validation... ");
    tests_total++;
    if (platonic_model_validate(model)) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 8: Sphere Packing Efficiency
    printf("Test 8: Sphere Packing Efficiency... ");
    tests_total++;
    double efficiency = platonic_sphere_packing_efficiency(PLATONIC_OCTAHEDRON);
    if (efficiency > 0.65 && efficiency < 0.70) {
        printf("PASS (%.1f%%)\n", efficiency * 100);
        tests_passed++;
    } else {
        printf("FAIL (%.1f%%, expected ~68%%)\n", efficiency * 100);
    }
    
    // Test 9: Symmetry Group
    printf("Test 9: Symmetry Group... ");
    tests_total++;
    if (model->geometry.symmetries == 48) {
        printf("PASS (Oh group, 48 symmetries - same as cube)\n");
        tests_passed++;
    } else {
        printf("FAIL (got %u, expected 48)\n", model->geometry.symmetries);
    }
    
    // Cleanup
    platonic_model_free(model);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", tests_passed, tests_total);
    printf("Success Rate: %.1f%%\n", (tests_passed * 100.0) / tests_total);
    
    if (tests_passed == tests_total) {
        printf("\n✓ All tests passed!\n\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed\n\n");
        return 1;
    }
}
