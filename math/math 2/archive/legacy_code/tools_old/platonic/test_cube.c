/**
 * Test Cube Platonic Model
 * 
 * Verifies:
 * - Model creation
 * - Geometry (Euler's formula)
 * - Dimensions
 * - Edge lengths
 * - Validation
 */

#include "ai/cllm_platonic.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("\n=== Cube Platonic Model Test ===\n\n");
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test 1: Create Configuration
    printf("Test 1: Create Configuration... ");
    tests_total++;
    PlatonicModelConfig config = platonic_config_create(
        PLATONIC_CUBE,
        1000,  // vocab_size
        512    // max_seq_len
    );
    if (config.solid_type == PLATONIC_CUBE) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 2: Get Geometry
    printf("Test 2: Get Geometry... ");
    tests_total++;
    PlatonicGeometry geometry = platonic_get_geometry(PLATONIC_CUBE);
    if (geometry.vertices == 8 && geometry.edges == 12 && geometry.faces == 6) {
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
    
    // Test 4: Model Creation
    printf("Test 4: Model Creation... ");
    tests_total++;
    PlatonicModel* model = platonic_model_create(&config);
    if (model) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 5: Dimension Verification
    printf("Test 5: Dimension Verification... ");
    tests_total++;
    if (model->config.embedding_dim == 96 && 
        model->config.hidden_dim == 144 && 
        model->config.num_layers == 6) {
        printf("PASS\n");
        printf("  Embedding: %u (8×12)\n", model->config.embedding_dim);
        printf("  Hidden: %u (12×12)\n", model->config.hidden_dim);
        printf("  Layers: %u\n", model->config.num_layers);
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 6: Model Validation
    printf("Test 6: Model Validation... ");
    tests_total++;
    if (platonic_model_validate(model)) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 7: Sphere Packing Efficiency
    printf("Test 7: Sphere Packing Efficiency... ");
    tests_total++;
    double efficiency = platonic_sphere_packing_efficiency(PLATONIC_CUBE);
    if (efficiency > 0.50 && efficiency < 0.55) {
        printf("PASS (%.1f%%)\n", efficiency * 100);
        tests_passed++;
    } else {
        printf("FAIL (%.1f%%, expected ~52.4%%)\n", efficiency * 100);
    }
    
    // Test 8: Symmetry Group
    printf("Test 8: Symmetry Group... ");
    tests_total++;
    if (model->geometry.symmetries == 48) {
        printf("PASS (Oh group, 48 symmetries)\n");
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
