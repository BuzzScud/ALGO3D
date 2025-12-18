/**
 * @file test_file_io.c
 * @brief Test File I/O for Geometric CLLMModel
 * 
 * Tests save/load functionality for the new geometric model structure
 * with all revolutionary features.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include <assert.h>
#include "cllm.h"
#include "cllm_format.h"

#define TEST_MODEL_FILE "test_model.cllm"
#define MATH_EPSILON 1e-6

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// ============================================================================
// TEST UTILITIES
// ============================================================================

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "❌ FAILED: %s\n", message); \
            fprintf(stderr, "   at %s:%d\n", __FILE__, __LINE__); \
            tests_failed++; \
            return 0; \
        } \
    } while(0)

#define TEST_PASS(message) \
    do { \
        printf("✓ PASSED: %s\n", message); \
        tests_passed++; \
        return 1; \
    } while(0)

/**
 * Compare two doubles with epsilon tolerance
 */
static int doubles_equal(double a, double b) {
    return fabs(a - b) < MATH_EPSILON;
}

/**
 * Compare two clock positions
 */
static int clock_positions_equal(const ClockPosition* a, const ClockPosition* b) {
    return a->ring == b->ring &&
           a->position == b->position &&
           doubles_equal(a->angle, b->angle) &&
           doubles_equal(a->radius, b->radius);
}

// ============================================================================
// TEST CASES
// ============================================================================

/**
 * Test 1: Save and load a simple model
 */
static int test_save_load_simple() {
    printf("\n=== Test 1: Save and Load Simple Model ===\n");
    
    // Create a simple model
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = 100,
        .max_seq_len = 64,
        .embedding_dim = 0,  // Auto
        .hidden_dim = 0,     // Auto
        .num_layers = 0,     // Auto
        .num_heads = 0,      // Auto
        .enable_blind_recovery = false,
        .enable_harmonic_integration = false,
        .enable_ntt_attention = false,
        .enable_kissing_spheres = false,
        .num_threads = 0,
        .optimizer_type = OPTIMIZER_ADAM,
        .learning_rate = 0.001,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
        .weight_decay = 0.01,
        .ntt_threshold_seq_len = 512,
        .ntt_auto_select = true
    };
    
    CLLMModel* original = cllm_create_model(&config);
    TEST_ASSERT(original != NULL, "Model creation failed");
    
    // Save model
    int save_result = cllm_write_model(original, TEST_MODEL_FILE);
    TEST_ASSERT(save_result == 0, "Model save failed");
    
    // Load model
    CLLMModel* loaded = cllm_read_model(TEST_MODEL_FILE);
    TEST_ASSERT(loaded != NULL, "Model load failed");
    
    // Verify basic parameters
    TEST_ASSERT(loaded->vocab_size == original->vocab_size, "Vocab size mismatch");
    TEST_ASSERT(loaded->embedding_dim == original->embedding_dim, "Embedding dim mismatch");
    TEST_ASSERT(loaded->hidden_dim == original->hidden_dim, "Hidden dim mismatch");
    TEST_ASSERT(loaded->num_layers == original->num_layers, "Num layers mismatch");
    TEST_ASSERT(loaded->num_heads == original->num_heads, "Num heads mismatch");
    TEST_ASSERT(loaded->max_seq_len == original->max_seq_len, "Max seq len mismatch");
    TEST_ASSERT(loaded->solid_type == original->solid_type, "Solid type mismatch");
    
    // Verify geometry
    TEST_ASSERT(loaded->geometry.vertices == original->geometry.vertices, "Vertices mismatch");
    TEST_ASSERT(loaded->geometry.edges == original->geometry.edges, "Edges mismatch");
    TEST_ASSERT(loaded->geometry.faces == original->geometry.faces, "Faces mismatch");
    
    // Verify Euler's formula
    int euler = loaded->geometry.vertices - loaded->geometry.edges + loaded->geometry.faces;
    TEST_ASSERT(euler == 2, "Euler's formula violated after load");
    
    // Cleanup
    cllm_free_model(original);
    cllm_free_model(loaded);
    remove(TEST_MODEL_FILE);
    
    TEST_PASS("Save and load simple model");
}

/**
 * Test 2: Save and load model with all Platonic solids
 */
static int test_all_platonic_solids() {
    printf("\n=== Test 2: All Platonic Solids ===\n");
    
    PlatonicSolidType solids[] = {
        PLATONIC_TETRAHEDRON,
        PLATONIC_CUBE,
        PLATONIC_OCTAHEDRON,
        PLATONIC_DODECAHEDRON,
        PLATONIC_ICOSAHEDRON
    };
    
    const char* names[] = {
        "Tetrahedron",
        "Cube",
        "Octahedron",
        "Dodecahedron",
        "Icosahedron"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("  Testing %s...\n", names[i]);
        
        CLLMConfig config = {
            .solid_type = solids[i],
            .vocab_size = 50,
            .max_seq_len = 32,
            .embedding_dim = 0,
            .hidden_dim = 0,
            .num_layers = 0,
            .num_heads = 0,
            .enable_blind_recovery = false,
            .enable_harmonic_integration = false,
            .enable_ntt_attention = false,
            .enable_kissing_spheres = false,
            .num_threads = 0,
            .optimizer_type = OPTIMIZER_ADAM,
            .learning_rate = 0.001,
            .beta1 = 0.9,
            .beta2 = 0.999,
            .epsilon = 1e-8,
            .weight_decay = 0.01,
            .ntt_threshold_seq_len = 512,
            .ntt_auto_select = true
        };
        
        CLLMModel* original = cllm_create_model(&config);
        TEST_ASSERT(original != NULL, "Model creation failed");
        
        // Save and load
        char filename[64];
        snprintf(filename, sizeof(filename), "test_%s.cllm", names[i]);
        
        int save_result = cllm_write_model(original, filename);
        TEST_ASSERT(save_result == 0, "Save failed");
        
        CLLMModel* loaded = cllm_read_model(filename);
        TEST_ASSERT(loaded != NULL, "Load failed");
        
        // Verify solid type
        TEST_ASSERT(loaded->solid_type == solids[i], "Solid type mismatch");
        
        // Verify Euler's formula
        int euler = loaded->geometry.vertices - loaded->geometry.edges + loaded->geometry.faces;
        TEST_ASSERT(euler == 2, "Euler's formula violated");
        
        // Cleanup
        cllm_free_model(original);
        cllm_free_model(loaded);
        remove(filename);
    }
    
    TEST_PASS("All Platonic solids save/load correctly");
}

/**
 * Test 3: Save and load with all features enabled
 */
static int test_all_features_enabled() {
    printf("\n=== Test 3: All Features Enabled ===\n");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_ICOSAHEDRON,
        .vocab_size = 100,
        .max_seq_len = 64,
        .embedding_dim = 0,
        .hidden_dim = 0,
        .num_layers = 0,
        .num_heads = 0,
        .enable_blind_recovery = true,
        .enable_harmonic_integration = true,
        .enable_ntt_attention = true,
        .enable_kissing_spheres = true,
        .num_threads = 13,
        .optimizer_type = OPTIMIZER_ADAM,
        .learning_rate = 0.001,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
        .weight_decay = 0.01,
        .ntt_threshold_seq_len = 512,
        .ntt_auto_select = true
    };
    
    CLLMModel* original = cllm_create_model(&config);
    TEST_ASSERT(original != NULL, "Model creation failed");
    
    // Save and load
    int save_result = cllm_write_model(original, TEST_MODEL_FILE);
    TEST_ASSERT(save_result == 0, "Save failed");
    
    CLLMModel* loaded = cllm_read_model(TEST_MODEL_FILE);
    TEST_ASSERT(loaded != NULL, "Load failed");
    
    // Verify feature flags
    TEST_ASSERT(loaded->recovery.enabled == original->recovery.enabled, "Blind recovery flag mismatch");
    TEST_ASSERT(loaded->harmonic.enabled == original->harmonic.enabled, "Harmonic flag mismatch");
    TEST_ASSERT(loaded->ntt.enabled == original->ntt.enabled, "NTT flag mismatch");
    
    // Cleanup
    cllm_free_model(original);
    cllm_free_model(loaded);
    remove(TEST_MODEL_FILE);
    
    TEST_PASS("All features save/load correctly");
}

/**
 * Test 4: Verify embeddings integrity after load
 */
static int test_embeddings_integrity() {
    printf("\n=== Test 4: Embeddings Integrity ===\n");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = 50,
        .max_seq_len = 32,
        .embedding_dim = 0,
        .hidden_dim = 0,
        .num_layers = 0,
        .num_heads = 0,
        .enable_blind_recovery = false,
        .enable_harmonic_integration = false,
        .enable_ntt_attention = false,
        .enable_kissing_spheres = false,
        .num_threads = 0,
        .optimizer_type = OPTIMIZER_ADAM,
        .learning_rate = 0.001,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
        .weight_decay = 0.01,
        .ntt_threshold_seq_len = 512,
        .ntt_auto_select = true
    };
    
    CLLMModel* original = cllm_create_model(&config);
    TEST_ASSERT(original != NULL, "Model creation failed");
    
    // Save and load
    int save_result = cllm_write_model(original, TEST_MODEL_FILE);
    TEST_ASSERT(save_result == 0, "Save failed");
    
    CLLMModel* loaded = cllm_read_model(TEST_MODEL_FILE);
    TEST_ASSERT(loaded != NULL, "Load failed");
    
    // Verify embeddings match
    int mismatches = 0;
    for (uint32_t i = 0; i < original->vocab_size && i < 10; i++) {
        for (uint32_t j = 0; j < original->embedding_dim && j < 10; j++) {
            double orig_val = original->embeddings[i * original->embedding_dim + j];
            double load_val = loaded->embeddings[i * loaded->embedding_dim + j];
            
            if (!doubles_equal(orig_val, load_val)) {
                mismatches++;
                if (mismatches <= 3) {
                    printf("  Mismatch at [%u][%u]: %.6f vs %.6f\n", i, j, orig_val, load_val);
                }
            }
        }
    }
    
    TEST_ASSERT(mismatches == 0, "Embeddings have mismatches after load");
    
    // Cleanup
    cllm_free_model(original);
    cllm_free_model(loaded);
    remove(TEST_MODEL_FILE);
    
    TEST_PASS("Embeddings integrity preserved");
}

/**
 * Test 5: Verify clock lattice positions after load
 */
static int test_clock_lattice_positions() {
    printf("\n=== Test 5: Clock Lattice Positions ===\n");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_OCTAHEDRON,
        .vocab_size = 50,
        .max_seq_len = 32,
        .embedding_dim = 0,
        .hidden_dim = 0,
        .num_layers = 0,
        .num_heads = 0,
        .enable_blind_recovery = false,
        .enable_harmonic_integration = false,
        .enable_ntt_attention = false,
        .enable_kissing_spheres = false,
        .num_threads = 0,
        .optimizer_type = OPTIMIZER_ADAM,
        .learning_rate = 0.001,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
        .weight_decay = 0.01,
        .ntt_threshold_seq_len = 512,
        .ntt_auto_select = true
    };
    
    CLLMModel* original = cllm_create_model(&config);
    TEST_ASSERT(original != NULL, "Model creation failed");
    
    // Save and load
    int save_result = cllm_write_model(original, TEST_MODEL_FILE);
    TEST_ASSERT(save_result == 0, "Save failed");
    
    CLLMModel* loaded = cllm_read_model(TEST_MODEL_FILE);
    TEST_ASSERT(loaded != NULL, "Load failed");
    
    // Verify vertex positions
    for (uint32_t i = 0; i < original->geometry.vertices; i++) {
        TEST_ASSERT(clock_positions_equal(&original->vertex_positions[i], 
                                         &loaded->vertex_positions[i]),
                   "Vertex position mismatch");
    }
    
    // Verify token positions (sample)
    for (uint32_t i = 0; i < 10 && i < original->vocab_size; i++) {
        TEST_ASSERT(clock_positions_equal(&original->token_positions[i],
                                         &loaded->token_positions[i]),
                   "Token position mismatch");
    }
    
    // Verify angular positions (sample)
    for (uint32_t i = 0; i < 10 && i < original->vocab_size; i++) {
        TEST_ASSERT(doubles_equal(original->token_angular_positions[i],
                                 loaded->token_angular_positions[i]),
                   "Angular position mismatch");
    }
    
    // Cleanup
    cllm_free_model(original);
    cllm_free_model(loaded);
    remove(TEST_MODEL_FILE);
    
    TEST_PASS("Clock lattice positions preserved");
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          CLLM FILE I/O TEST SUITE                         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Run tests
    test_save_load_simple();
    test_all_platonic_solids();
    test_all_features_enabled();
    test_embeddings_integrity();
    test_clock_lattice_positions();
    
    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-3d                                        ║\n", tests_passed);
    printf("║  Tests Failed: %-3d                                        ║\n", tests_failed);
    printf("║  Total Tests:  %-3d                                        ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    if (tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! 🎉\n\n");
        return 0;
    } else {
        printf("\n❌ SOME TESTS FAILED ❌\n\n");
        return 1;
    }
}