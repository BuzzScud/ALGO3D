/**
 * @file test_tetrahedron.c
 * @brief Test program for Tetrahedron Platonic model
 * 
 * This program tests the basic functionality of the Tetrahedron model:
 * - Model creation
 * - Geometry verification
 * - Embedding initialization
 * - Layer initialization
 * - Basic validation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai/cllm_platonic.h"

// Forward declarations from tetrahedron implementation
extern PlatonicModel* platonic_tetrahedron_create(uint32_t vocab_size, uint32_t max_seq_len);
extern bool platonic_tetrahedron_verify_edges(const PlatonicModel* model);

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TETRAHEDRON MODEL TEST SUITE                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Test parameters
    uint32_t vocab_size = 1000;
    uint32_t max_seq_len = 128;
    
    printf("Test Configuration:\n");
    printf("  Vocab size: %u\n", vocab_size);
    printf("  Max sequence length: %u\n", max_seq_len);
    printf("\n");
    
    // ========================================================================
    // TEST 1: Model Creation
    // ========================================================================
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("TEST 1: Model Creation\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    PlatonicModel* model = platonic_tetrahedron_create(vocab_size, max_seq_len);
    
    if (!model) {
        printf("✗ FAILED: Model creation failed\n");
        return 1;
    }
    
    printf("✓ PASSED: Model created successfully\n");
    printf("\n");
    
    // ========================================================================
    // TEST 2: Geometry Verification
    // ========================================================================
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("TEST 2: Geometry Verification\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    // Verify Euler's formula
    bool euler_valid = platonic_verify_euler(&model->geometry);
    printf("Euler's formula (V - E + F = 2): %s\n",
           euler_valid ? "✓ PASSED" : "✗ FAILED");
    
    if (!euler_valid) {
        platonic_model_free(model);
        return 1;
    }
    
    // Verify edge lengths
    bool edges_valid = platonic_tetrahedron_verify_edges(model);
    printf("\nEdge length uniformity: %s\n",
           edges_valid ? "✓ PASSED" : "✗ FAILED");
    
    if (!edges_valid) {
        platonic_model_free(model);
        return 1;
    }
    
    printf("\n");
    
    // ========================================================================
    // TEST 3: Dimension Verification
    // ========================================================================
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("TEST 3: Dimension Verification\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    bool dims_valid = true;
    
    // Check embedding dimension (should be 48 = 4 × 12)
    if (model->config.embedding_dim != 48) {
        printf("✗ FAILED: Embedding dim is %u, expected 48\n",
               model->config.embedding_dim);
        dims_valid = false;
    } else {
        printf("✓ PASSED: Embedding dim = 48 (4 vertices × 12)\n");
    }
    
    // Check hidden dimension (should be 72 = 6 × 12)
    if (model->config.hidden_dim != 72) {
        printf("✗ FAILED: Hidden dim is %u, expected 72\n",
               model->config.hidden_dim);
        dims_valid = false;
    } else {
        printf("✓ PASSED: Hidden dim = 72 (6 edges × 12)\n");
    }
    
    // Check number of layers (should be 4)
    if (model->config.num_layers != 4) {
        printf("✗ FAILED: Num layers is %u, expected 4\n",
               model->config.num_layers);
        dims_valid = false;
    } else {
        printf("✓ PASSED: Num layers = 4 (4 faces)\n");
    }
    
    // Check attention heads (should be 12)
    if (model->config.num_heads != 12) {
        printf("✗ FAILED: Num heads is %u, expected 12\n",
               model->config.num_heads);
        dims_valid = false;
    } else {
        printf("✓ PASSED: Num heads = 12 (12-fold symmetry)\n");
    }
    
    if (!dims_valid) {
        platonic_model_free(model);
        return 1;
    }
    
    printf("\n");
    
    // ========================================================================
    // TEST 4: Model Validation
    // ========================================================================
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("TEST 4: Model Validation\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    bool validation_passed = platonic_model_validate(model);
    
    if (!validation_passed) {
        printf("✗ FAILED: Model validation failed\n");
        platonic_model_free(model);
        return 1;
    }
    
    printf("✓ PASSED: Model validation successful\n");
    printf("\n");
    
    // ========================================================================
    // TEST 5: Sphere Packing Efficiency
    // ========================================================================
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("TEST 5: Sphere Packing Efficiency\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    double packing = platonic_sphere_packing_efficiency(PLATONIC_TETRAHEDRON);
    printf("Tetrahedron sphere packing: %.1f%%\n", packing * 100.0);
    
    if (packing < 0.3 || packing > 0.4) {
        printf("✗ FAILED: Expected ~34%%, got %.1f%%\n", packing * 100.0);
        platonic_model_free(model);
        return 1;
    }
    
    printf("✓ PASSED: Packing efficiency in expected range\n");
    printf("\n");
    
    // ========================================================================
    // SUMMARY
    // ========================================================================
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("TEST SUMMARY\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("All tests passed! ✓\n");
    printf("\n");
    printf("Model Properties:\n");
    printf("  Solid: Tetrahedron\n");
    printf("  Vertices: 4\n");
    printf("  Edges: 6\n");
    printf("  Faces: 4\n");
    printf("  Symmetries: 12\n");
    printf("  Embedding: 48-dim (4 × 12)\n");
    printf("  Hidden: 72-dim (6 × 12)\n");
    printf("  Layers: 4\n");
    printf("  Sphere packing: %.1f%%\n", packing * 100.0);
    printf("\n");
    
    // Clean up
    platonic_model_free(model);
    
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  ALL TESTS PASSED                                        ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}
