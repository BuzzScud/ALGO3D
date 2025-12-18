/**
 * Test Unified CLLM Model
 * 
 * Tests the new unified model structure with Platonic geometry.
 */

#include <stdio.h>
#include <stdlib.h>
#include "ai/cllm_unified_model.h"

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  UNIFIED MODEL TEST SUITE                               ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test 1: Create Tetrahedron model
    printf("──────────────────────────────────────────────────────────\n");
    printf("TEST 1: Create Tetrahedron Model\n");
    printf("──────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    UnifiedModelConfig config = {
        .solid_type = PLATONIC_TETRAHEDRON,
        .vocab_size = 1000,
        .max_seq_len = 128,
        .enable_blind_recovery = true,
        .enable_harmonic_integration = true,
        .enable_ntt_attention = false,
        .optimizer_type = OPTIMIZER_ADAM,
        .learning_rate = 0.0001,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
    };
    
    UnifiedCLLMModel* model = unified_model_create(&config);
    if (model) {
        printf("✓ PASS: Tetrahedron model created\n\n");
        tests_passed++;
        
        // Test 2: Validate model
        printf("──────────────────────────────────────────────────────────\n");
        printf("TEST 2: Validate Model Structure\n");
        printf("──────────────────────────────────────────────────────────\n\n");
        tests_total++;
        
        if (unified_model_validate(model)) {
            printf("✓ PASS: Model validation successful\n\n");
            tests_passed++;
        } else {
            printf("✗ FAIL: Model validation failed\n\n");
        }
        
        // Test 3: Print model info
        printf("──────────────────────────────────────────────────────────\n");
        printf("TEST 3: Model Information\n");
        printf("──────────────────────────────────────────────────────────\n");
        tests_total++;
        
        unified_model_print_info(model);
        printf("✓ PASS: Model info printed\n\n");
        tests_passed++;
        
        // Test 4: Check dimensions
        printf("──────────────────────────────────────────────────────────\n");
        printf("TEST 4: Dimension Verification\n");
        printf("──────────────────────────────────────────────────────────\n\n");
        tests_total++;
        
        bool dims_ok = true;
        if (model->embedding_dim != 48) {
            printf("✗ Embedding dim: expected 48, got %u\n", model->embedding_dim);
            dims_ok = false;
        }
        if (model->hidden_dim != 72) {
            printf("✗ Hidden dim: expected 72, got %u\n", model->hidden_dim);
            dims_ok = false;
        }
        if (model->num_layers != 4) {
            printf("✗ Layers: expected 4, got %u\n", model->num_layers);
            dims_ok = false;
        }
        if (model->num_heads != 12) {
            printf("✗ Heads: expected 12, got %u\n", model->num_heads);
            dims_ok = false;
        }
        
        if (dims_ok) {
            printf("✓ PASS: All dimensions correct\n");
            printf("  Embedding: 48 (4 vertices × 12)\n");
            printf("  Hidden: 72 (6 edges × 12)\n");
            printf("  Layers: 4 (4 faces)\n");
            printf("  Heads: 12 (12-fold symmetry)\n\n");
            tests_passed++;
        } else {
            printf("✗ FAIL: Dimension mismatch\n\n");
        }
        
        // Test 5: Check parameter count
        printf("──────────────────────────────────────────────────────────\n");
        printf("TEST 5: Parameter Count\n");
        printf("──────────────────────────────────────────────────────────\n\n");
        tests_total++;
        
        uint64_t param_count = unified_model_count_parameters(model);
        printf("Total parameters: %lu (%.2f K)\n", param_count, param_count / 1e3);
        
        if (param_count > 0) {
            printf("✓ PASS: Parameters counted\n\n");
            tests_passed++;
        } else {
            printf("✗ FAIL: Zero parameters\n\n");
        }
        
        // Cleanup
        unified_model_free(model);
    } else {
        printf("✗ FAIL: Failed to create model\n\n");
    }
    
    // Test 6: Create Cube model
    printf("──────────────────────────────────────────────────────────\n");
    printf("TEST 6: Create Cube Model\n");
    printf("──────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    config.solid_type = PLATONIC_CUBE;
    model = unified_model_create(&config);
    
    if (model && unified_model_validate(model)) {
        printf("✓ PASS: Cube model created and validated\n");
        printf("  Embedding: %u (8 vertices × 12)\n", model->embedding_dim);
        printf("  Hidden: %u (12 edges × 12)\n", model->hidden_dim);
        printf("  Layers: %u (6 faces)\n\n", model->num_layers);
        tests_passed++;
        unified_model_free(model);
    } else {
        printf("✗ FAIL: Cube model creation failed\n\n");
    }
    
    // Test 7: Create Icosahedron model
    printf("──────────────────────────────────────────────────────────\n");
    printf("TEST 7: Create Icosahedron Model\n");
    printf("──────────────────────────────────────────────────────────\n\n");
    tests_total++;
    
    config.solid_type = PLATONIC_ICOSAHEDRON;
    model = unified_model_create(&config);
    
    if (model && unified_model_validate(model)) {
        printf("✓ PASS: Icosahedron model created and validated\n");
        printf("  Embedding: %u (12 vertices × 12 = 144)\n", model->embedding_dim);
        printf("  Hidden: %u (30 edges × 12 = 360)\n", model->hidden_dim);
        printf("  Layers: %u (20 faces)\n\n", model->num_layers);
        tests_passed++;
        unified_model_free(model);
    } else {
        printf("✗ FAIL: Icosahedron model creation failed\n\n");
    }
    
    // Summary
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                           ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    printf("Passed: %d/%d\n", tests_passed, tests_total);
    printf("Success Rate: %.1f%%\n\n", 100.0 * tests_passed / tests_total);
    
    if (tests_passed == tests_total) {
        printf("✓ ALL TESTS PASSED!\n\n");
        return 0;
    } else {
        printf("✗ SOME TESTS FAILED\n\n");
        return 1;
    }
}