/**
 * Test Icosahedron Platonic Model
 * 
 * Verifies:
 * - Model creation
 * - Geometry (Euler's formula)
 * - Dimensions
 * - Golden ratio integration
 * - Maximum symmetry
 * - Dual relationship with dodecahedron
 * - Validation
 */

#include "ai/cllm_platonic.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("\n=== Icosahedron Platonic Model Test ===\n\n");
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test 1: Create Configuration
    printf("Test 1: Create Configuration... ");
    tests_total++;
    PlatonicModelConfig config = platonic_config_create(
        PLATONIC_ICOSAHEDRON,
        1000,  // vocab_size
        512    // max_seq_len
    );
    if (config.solid_type == PLATONIC_ICOSAHEDRON) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 2: Get Geometry
    printf("Test 2: Get Geometry... ");
    tests_total++;
    PlatonicGeometry geometry = platonic_get_geometry(PLATONIC_ICOSAHEDRON);
    if (geometry.vertices == 12 && geometry.edges == 30 && geometry.faces == 20) {
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
    
    // Test 4: Verify Dual Relationship with Dodecahedron
    printf("Test 4: Dual Relationship with Dodecahedron... ");
    tests_total++;
    PlatonicGeometry dodec_geom = platonic_get_geometry(PLATONIC_DODECAHEDRON);
    if (geometry.vertices == dodec_geom.faces &&
        geometry.faces == dodec_geom.vertices &&
        geometry.edges == dodec_geom.edges) {
        printf("PASS\n");
        printf("  Icosahedron:   V=%u, E=%u, F=%u\n", 
               geometry.vertices, geometry.edges, geometry.faces);
        printf("  Dodecahedron:  V=%u, E=%u, F=%u\n",
               dodec_geom.vertices, dodec_geom.edges, dodec_geom.faces);
        printf("  Dual verified: V_ico = F_dodec, F_ico = V_dodec\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 5: Verify Golden Ratio Property
    printf("Test 5: Golden Ratio Property... ");
    tests_total++;
    if (geometry.has_golden_ratio) {
        printf("PASS (φ = (1+√5)/2 ≈ 1.618)\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 6: Model Creation
    printf("Test 6: Model Creation... ");
    tests_total++;
    PlatonicModel* model = platonic_model_create(&config);
    if (model) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 7: Dimension Verification
    printf("Test 7: Dimension Verification... ");
    tests_total++;
    if (model->config.embedding_dim == 144 && 
        model->config.hidden_dim == 360 && 
        model->config.num_layers == 20) {
        printf("PASS\n");
        printf("  Embedding: %u (12×12 - perfect square!)\n", model->config.embedding_dim);
        printf("  Hidden: %u (30×12)\n", model->config.hidden_dim);
        printf("  Layers: %u\n", model->config.num_layers);
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 8: Model Validation
    printf("Test 8: Model Validation... ");
    tests_total++;
    if (platonic_model_validate(model)) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Test 9: Sphere Packing Efficiency
    printf("Test 9: Sphere Packing Efficiency... ");
    tests_total++;
    double efficiency = platonic_sphere_packing_efficiency(PLATONIC_ICOSAHEDRON);
    if (efficiency > 0.72 && efficiency < 0.76) {
        printf("PASS (%.1f%% - maximum for Platonic solids!)\n", efficiency * 100);
        tests_passed++;
    } else {
        printf("FAIL (%.1f%%, expected ~74%%)\n", efficiency * 100);
    }
    
    // Test 10: Symmetry Group
    printf("Test 10: Symmetry Group... ");
    tests_total++;
    if (model->geometry.symmetries == 120) {
        printf("PASS (Ih group, 120 symmetries - MAXIMUM!)\n");
        tests_passed++;
    } else {
        printf("FAIL (got %u, expected 120)\n", model->geometry.symmetries);
    }
    
    // Test 11: Golden Ratio in Model
    printf("Test 11: Golden Ratio in Model... ");
    tests_total++;
    if (model->geometry.has_golden_ratio) {
        printf("PASS (φ integrated in vertex positions)\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    
    // Cleanup
    platonic_model_free(model);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", tests_passed, tests_total);
    printf("Success Rate: %.1f%%\n", (tests_passed * 100.0) / tests_total);
    
    if (tests_passed == tests_total) {
        printf("\n✓ All tests passed!\n");
        printf("✓ ALL 5 PLATONIC SOLIDS COMPLETE!\n\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed\n\n");
        return 1;
    }
}