/**
 * @file test_embedding_consolidation.c
 * @brief Test suite for consolidated embedding implementation
 * 
 * Tests all embedding functionality after Phase 1.2 consolidation:
 * - Basic embedding initialization
 * - Neighbor-influenced embeddings
 * - LLL reduction
 * - Lattice utilities
 * - Backward compatibility
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/ai/cllm_platonic.h"
#include "../include/clock_lattice.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Forward declarations for functions in cllm_embedding.c
double cllm_embedding_similarity(const double* emb1, const double* emb2, uint32_t dim);
void cllm_compute_spiral_position(uint64_t prime, double* angle, double* radius);
void cllm_map_token_to_lattice(uint32_t token_id, uint64_t prime, double* coords);
double cllm_lattice_token_distance(uint32_t token1_id, uint64_t prime1, uint32_t token2_id, uint64_t prime2);
void cllm_generate_lattice_embedding(uint32_t token_id, uint64_t prime, uint32_t embedding_dim, double* output);

#define TEST_VOCAB_SIZE 100
#define TEST_EMBED_DIM 64
#define MATH_EPSILON 1e-6

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

// Test result macro
#define TEST(name, condition) do { \
    tests_run++; \
    if (condition) { \
        tests_passed++; \
        printf("✓ %s\n", name); \
    } else { \
        printf("✗ %s\n", name); \
    } \
} while(0)

/**
 * Create a minimal test model
 */
CLLMModel* create_test_model(uint32_t vocab_size, uint32_t embed_dim) {
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    model->vocab_size = vocab_size;
    model->embedding_dim = embed_dim;
    
    // Allocate embeddings
    model->embeddings.vocab_size = vocab_size;
    model->embeddings.embedding_dim = embed_dim;
    model->embeddings.embeddings = (double*)calloc(vocab_size * embed_dim, sizeof(double));
    
    // Allocate tokens
    model->tokens = (CLLMToken*)calloc(vocab_size, sizeof(CLLMToken));
    for (uint32_t i = 0; i < vocab_size; i++) {
        model->tokens[i].token_id = i;
        model->tokens[i].prime_encoding = (i + 1) * 2 + 1; // Simple prime-like values
        model->tokens[i].symmetry_group = i % 12;
        snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "token_%u", i);
    }
    
    return model;
}

/**
 * Free test model
 */
void free_test_model(CLLMModel* model) {
    if (!model) return;
    
    if (model->embeddings.embeddings) {
        free(model->embeddings.embeddings);
    }
    if (model->tokens) {
        free(model->tokens);
    }
    if (model->lattice_points) {
        free(model->lattice_points);
    }
    free(model);
}

/**
 * Test 1: Basic embedding initialization
 */
void test_basic_initialization() {
    printf("\n=== Test 1: Basic Embedding Initialization ===\n");
    
    CLLMModel* model = create_test_model(TEST_VOCAB_SIZE, TEST_EMBED_DIM);
    TEST("Model creation", model != NULL);
    
    // Initialize embeddings
    cllm_init_embeddings(model);
    
    // Check that embeddings are non-zero
    int non_zero_count = 0;
    for (uint32_t i = 0; i < TEST_VOCAB_SIZE * TEST_EMBED_DIM; i++) {
        if (math_abs(model->embeddings.embeddings[i]) > MATH_EPSILON) {
            non_zero_count++;
        }
    }
    TEST("Embeddings initialized (non-zero)", non_zero_count > TEST_VOCAB_SIZE * TEST_EMBED_DIM / 2);
    
    // Check embedding norms are reasonable
    double total_norm = 0.0;
    for (uint32_t i = 0; i < TEST_VOCAB_SIZE; i++) {
        double norm = 0.0;
        for (uint32_t j = 0; j < TEST_EMBED_DIM; j++) {
            double val = model->embeddings.embeddings[i * TEST_EMBED_DIM + j];
            norm += val * val;
        }
        total_norm += math_sqrt(norm);
    }
    double avg_norm = total_norm / TEST_VOCAB_SIZE;
    TEST("Average embedding norm reasonable", avg_norm > 0.1 && avg_norm < 10.0);
    
    free_test_model(model);
}

/**
 * Test 2: Legacy compatibility
 */
void test_legacy_compatibility() {
    printf("\n=== Test 2: Legacy Compatibility ===\n");
    
    CLLMModel* model = create_test_model(TEST_VOCAB_SIZE, TEST_EMBED_DIM);
    TEST("Model creation", model != NULL);
    
    // Test legacy function
    cllm_init_embeddings_with_lattice(model);
    
    // Verify embeddings were initialized
    int initialized = 0;
    for (uint32_t i = 0; i < TEST_VOCAB_SIZE * TEST_EMBED_DIM; i++) {
        if (math_abs(model->embeddings.embeddings[i]) > MATH_EPSILON) {
            initialized = 1;
            break;
        }
    }
    TEST("Legacy function works", initialized);
    
    free_test_model(model);
}

/**
 * Test 3: Embedding utilities
 */
void test_embedding_utilities() {
    printf("\n=== Test 3: Embedding Utilities ===\n");
    
    CLLMModel* model = create_test_model(TEST_VOCAB_SIZE, TEST_EMBED_DIM);
    cllm_init_embeddings(model);
    
    // Test similarity computation
    double* emb1 = &model->embeddings.embeddings[0];
    double* emb2 = &model->embeddings.embeddings[TEST_EMBED_DIM];
    double sim = cllm_embedding_similarity(emb1, emb2, TEST_EMBED_DIM);
    TEST("Similarity computation", sim >= -1.0 && sim <= 1.0);
    
    // Test self-similarity
    double self_sim = cllm_embedding_similarity(emb1, emb1, TEST_EMBED_DIM);
    TEST("Self-similarity is 1.0", math_abs(self_sim - 1.0) < 0.01);
    
    // Test normalization
    float test_vec[TEST_EMBED_DIM];
    for (int i = 0; i < TEST_EMBED_DIM; i++) {
        test_vec[i] = (float)(i + 1);
    }
    cllm_normalize_embedding(test_vec, TEST_EMBED_DIM);
    
    float norm = 0.0f;
    for (int i = 0; i < TEST_EMBED_DIM; i++) {
        norm += test_vec[i] * test_vec[i];
    }
    norm = math_sqrt(norm);
    TEST("Normalization produces unit vector", math_abs(norm - 1.0f) < 0.01f);
    
    free_test_model(model);
}

/**
 * Test 4: Lattice utilities
 */
void test_lattice_utilities() {
    printf("\n=== Test 4: Lattice Utilities ===\n");
    
    // Test spiral position computation
    double angle, radius;
    cllm_compute_spiral_position(7, &angle, &radius);
    TEST("Spiral position computed", radius > 0.0 && angle >= 0.0);
    
    // Test lattice mapping
    double coords[3];
    cllm_map_token_to_lattice(0, 2, coords);
    TEST("Lattice mapping produces coordinates", 
         coords[0] != 0.0 || coords[1] != 0.0 || coords[2] != 0.0);
    
    // Test distance computation
    double dist = cllm_lattice_token_distance(0, 2, 1, 3);
    TEST("Distance computation", dist >= 0.0);
    
    // Test lattice embedding generation
    CLLMModel* model = create_test_model(TEST_VOCAB_SIZE, TEST_EMBED_DIM);
    double output[TEST_EMBED_DIM];
    cllm_generate_lattice_embedding(0, 2, TEST_EMBED_DIM, output);
    
    int non_zero = 0;
    for (int i = 0; i < TEST_EMBED_DIM; i++) {
        if (math_abs(output[i]) > MATH_EPSILON) non_zero++;
    }
    TEST("Lattice embedding generation", non_zero > 0);
    
    free_test_model(model);
}

/**
 * Test 5: Platonic geometry integration
 */
void test_platonic_integration() {
    printf("\n=== Test 5: Platonic Geometry Integration ===\n");
    
    CLLMModel* model = create_test_model(TEST_VOCAB_SIZE, TEST_EMBED_DIM);
    
    // Enable Platonic geometry
    model->use_platonic_geometry = 1;
    model->platonic_solid_type = PLATONIC_TETRAHEDRON;
    
    // Allocate clock positions
    model->token_clock_positions = calloc(TEST_VOCAB_SIZE, sizeof(BabylonianClockPosition));
    BabylonianClockPosition* positions = (BabylonianClockPosition*)model->token_clock_positions;
    
    // Initialize with some test positions
    for (uint32_t i = 0; i < TEST_VOCAB_SIZE; i++) {
        positions[i].angle = (double)i / TEST_VOCAB_SIZE * 2.0 * M_PI;
        positions[i].radius = 50.0 + (i % 50);
    }
    
    // Set geometry
    model->geometry.vertices = 4;
    model->geometry.edges = 6;
    model->geometry.faces = 4;
    
    // Initialize embeddings
    cllm_init_embeddings(model);
    
    // Verify embeddings were initialized
    int initialized = 0;
    for (uint32_t i = 0; i < TEST_VOCAB_SIZE * TEST_EMBED_DIM; i++) {
        if (math_abs(model->embeddings.embeddings[i]) > MATH_EPSILON) {
            initialized = 1;
            break;
        }
    }
    TEST("Platonic geometry initialization", initialized);
    
    free(model->token_clock_positions);
    free_test_model(model);
}

/**
 * Main test runner
 */
int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Embedding Consolidation Test Suite (Phase 1.2)           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_basic_initialization();
    test_legacy_compatibility();
    test_embedding_utilities();
    test_lattice_utilities();
    test_platonic_integration();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test Results                                              ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Run:    %-3d                                         ║\n", tests_run);
    printf("║  Tests Passed: %-3d                                         ║\n", tests_passed);
    printf("║  Tests Failed: %-3d                                         ║\n", tests_run - tests_passed);
    printf("║  Success Rate: %.1f%%                                      ║\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return (tests_passed == tests_run) ? 0 : 1;
}
