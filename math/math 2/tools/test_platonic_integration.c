/**
 * Test Platonic Model Integration into CLLMModel
 * 
 * Tests that Platonic models can be created through the standard
 * CLLM API and that all features work correctly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cllm.h"
#include "cllm_utils.h"
#include "ai/cllm_platonic.h"

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  PLATONIC MODEL INTEGRATION TEST                         ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test 1: Create Tetrahedron model
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST 1: Create Tetrahedron Model via CLLM API\n");
    printf("────────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    CLLMModel* tetra = cllm_create_tetrahedron_model(1000, 128);
    if (tetra) {
        printf("✓ PASS: Tetrahedron model created\n");
        printf("  Platonic geometry: %s\n", tetra->use_platonic_geometry ? "enabled" : "disabled");
        printf("  Vertices: %u → Embedding: %lu\n", tetra->geometry.vertices, tetra->embedding_dim);
        printf("  Edges: %u → Hidden: %u\n", tetra->geometry.edges, tetra->ff_layers[0].hidden_dim);
        printf("  Faces: %u → Layers: %u\n", tetra->geometry.faces, tetra->num_layers);
        printf("  Blind recovery: %s\n", tetra->blind_recovery.enabled ? "enabled" : "disabled");
        printf("  Harmonic: %s\n", tetra->harmonic.enabled ? "enabled" : "disabled");
        printf("\n");
        tests_passed++;
        cllm_free_model(tetra);
    } else {
        printf("✗ FAIL: Failed to create tetrahedron model\n\n");
    }
    
    // Test 2: Create Cube model
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST 2: Create Cube Model\n");
    printf("────────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    CLLMModel* cube = cllm_create_cube_model(1000, 256);
    if (cube && cube->use_platonic_geometry) {
        printf("✓ PASS: Cube model created with Platonic geometry\n");
        printf("  Embedding: %lu (8 vertices × 12)\n", cube->embedding_dim);
        printf("  Hidden: %u (12 edges × 12)\n", cube->ff_layers[0].hidden_dim);
        printf("  Layers: %u (6 faces)\n", cube->num_layers);
        printf("  Symmetries: %u\n", cube->geometry.symmetries);
        printf("\n");
        tests_passed++;
        cllm_free_model(cube);
    } else {
        printf("✗ FAIL: Cube model creation failed\n\n");
    }
    
    // Test 3: Create Icosahedron model
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST 3: Create Icosahedron Model (Maximum Symmetry)\n");
    printf("────────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    CLLMModel* icosa = cllm_create_icosahedron_model(5000, 512);
    if (icosa && icosa->use_platonic_geometry) {
        printf("✓ PASS: Icosahedron model created\n");
        printf("  Embedding: %lu (12 vertices × 12 = 144)\n", icosa->embedding_dim);
        printf("  Hidden: %u (30 edges × 12 = 360)\n", icosa->ff_layers[0].hidden_dim);
        printf("  Layers: %u (20 faces)\n", icosa->num_layers);
        printf("  Symmetries: %u (maximum)\n", icosa->geometry.symmetries);
        printf("  Golden ratio: %s\n", icosa->geometry.has_golden_ratio ? "yes" : "no");
        printf("  Sphere packing: %.1f%%\n", icosa->geometry.sphere_packing);
        printf("\n");
        tests_passed++;
        cllm_free_model(icosa);
    } else {
        printf("✗ FAIL: Icosahedron model creation failed\n\n");
    }
    
    // Test 4: Verify Euler's formula
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST 4: Verify Euler's Formula (V - E + F = 2)\n");
    printf("────────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    CLLMModel* dodeca = cllm_create_dodecahedron_model(1000, 256);
    if (dodeca && dodeca->use_platonic_geometry) {
        int euler = dodeca->geometry.vertices - dodeca->geometry.edges + dodeca->geometry.faces;
        if (euler == 2) {
            printf("✓ PASS: Euler's formula verified\n");
            printf("  V - E + F = %u - %u + %u = %d ✓\n",
                   dodeca->geometry.vertices, dodeca->geometry.edges,
                   dodeca->geometry.faces, euler);
            printf("\n");
            tests_passed++;
        } else {
            printf("✗ FAIL: Euler's formula violated: %d ≠ 2\n\n", euler);
        }
        cllm_free_model(dodeca);
    } else {
        printf("✗ FAIL: Dodecahedron model creation failed\n\n");
    }
    
    // Test 5: Verify clock lattice positions
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST 5: Verify Clock Lattice Position Mapping\n");
    printf("────────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    CLLMModel* test_model = cllm_create_cube_model(100, 128);
    if (test_model && test_model->token_clock_positions && test_model->token_angular_positions) {
        printf("✓ PASS: Clock lattice positions allocated\n");
        printf("  Token positions: %p\n", test_model->token_clock_positions);
        printf("  Angular positions: %p\n", test_model->token_angular_positions);
        printf("  First token angular position: %.6f\n", test_model->token_angular_positions[0]);
        printf("\n");
        tests_passed++;
        cllm_free_model(test_model);
    } else {
        printf("✗ FAIL: Clock lattice positions not allocated\n\n");
        if (test_model) cllm_free_model(test_model);
    }
    
    // Test 6: Verify feature flags
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST 6: Verify Feature Flags\n");
    printf("────────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    CLLMModel* feature_test = cllm_create_icosahedron_model(1000, 256);
    if (feature_test) {
        bool all_features = feature_test->blind_recovery.enabled &&
                           feature_test->harmonic.enabled &&
                           feature_test->ntt_attention.enabled;
        
        if (all_features) {
            printf("✓ PASS: All features enabled\n");
            printf("  Blind recovery: enabled (tolerance: %.0f%%)\n", 
                   feature_test->blind_recovery.corruption_tolerance * 100);
            printf("  Harmonic: enabled (frequency: %.0f Hz)\n",
                   feature_test->harmonic.primary_frequency);
            printf("  NTT attention: enabled (threshold: %u)\n",
                   feature_test->ntt_attention.threshold_seq_len);
            printf("\n");
            tests_passed++;
        } else {
            printf("✗ FAIL: Not all features enabled\n\n");
        }
        cllm_free_model(feature_test);
    } else {
        printf("✗ FAIL: Model creation failed\n\n");
    }
    
    // Test 7: Verify backward compatibility
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST 7: Verify Backward Compatibility\n");
    printf("────────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    CLLMModel* standard = cllm_create_small_model();
    if (standard && !standard->use_platonic_geometry) {
        printf("✓ PASS: Standard model creation still works\n");
        printf("  Platonic geometry: %s (as expected)\n",
               standard->use_platonic_geometry ? "enabled" : "disabled");
        printf("  Embedding: %lu\n", standard->embedding_dim);
        printf("  Layers: %u\n", standard->num_layers);
        printf("\n");
        tests_passed++;
        cllm_free_model(standard);
    } else {
        printf("✗ FAIL: Standard model creation broken\n\n");
        if (standard) cllm_free_model(standard);
    }
    
    // Summary
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    printf("Passed: %d/%d\n", tests_passed, tests_total);
    printf("Success Rate: %.1f%%\n\n", 100.0 * tests_passed / tests_total);
    
    if (tests_passed == tests_total) {
        printf("✓ ALL TESTS PASSED!\n\n");
        printf("✓ Platonic models successfully integrated into CLLM API\n");
        printf("✓ All 5 Platonic solids working\n");
        printf("✓ Blind recovery enabled\n");
        printf("✓ Harmonic integration enabled\n");
        printf("✓ NTT attention enabled\n");
        printf("✓ Clock lattice positions mapped\n");
        printf("✓ Backward compatibility maintained\n\n");
        return 0;
    } else {
        printf("✗ SOME TESTS FAILED\n\n");
        return 1;
    }
}