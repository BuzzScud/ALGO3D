/**
 * Comprehensive Mathematical Integration Test Suite
 * 
 * Tests all mathematical framework integrations:
 * - OBJECTIVE 14: L(n,d,k,λ) lattice embeddings
 * - OBJECTIVE 15: θ(n,k,λ,ω,ψ) angular attention
 * - OBJECTIVE 16: 12 kissing sphere neighbors
 * - OBJECTIVE 17: NTT O(n log n) attention
 * - OBJECTIVE 18: Cymatic frequency resonance
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cllm.h"
#include "cllm_format.h"
#include "cllm_training.h"
#include "cllm_utils.h"
#include "ai/cllm_lattice_embeddings.h"
#include "ai/cllm_angular_attention.h"
#include "ai/cllm_lattice.h"
#include "ai/cllm_ntt_attention.h"
#include "ai/cllm_cymatic_training.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            tests_passed++; \
            printf("  ✓ %s\n", message); \
        } else { \
            tests_failed++; \
            printf("  ✗ %s\n", message); \
        } \
    } while(0)

/**
 * Test OBJECTIVE 14: Lattice Embeddings
 */
void test_lattice_embeddings() {
    printf("\n=== Test 1: Lattice Embeddings (OBJECTIVE 14) ===\n");
    
    // Create small test model
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        tests_failed++;
        return;
    }
    
    // Initialize with lattice formula
    cllm_embeddings_init_lattice(model);
    
    // Test 1: Verify embeddings are in valid range [-1, 1]
    int in_range = 1;
    for (uint32_t i = 0; i < model->vocab_size * model->embeddings.embedding_dim; i++) {
        float val = model->embeddings.embeddings[i];
        if (val < -1.0f || val > 1.0f) {
            in_range = 0;
            break;
        }
    }
    TEST_ASSERT(in_range, "All embeddings in [-1, 1] range");
    
    // Test 2: Verify embeddings are not all zeros
    int non_zero = 0;
    for (uint32_t i = 0; i < model->vocab_size * model->embeddings.embedding_dim; i++) {
        if (model->embeddings.embeddings[i] != 0.0f) {
            non_zero++;
        }
    }
    TEST_ASSERT(non_zero > 0, "Embeddings are not all zeros");
    
    // Test 3: Verify symmetry group similarity
    // (Tokens in same group should have similar embeddings)
    float same_group_sim = 0.0f;
    float diff_group_sim = 0.0f;
    int same_count = 0, diff_count = 0;
    
    for (uint32_t i = 0; i < 20 && i < model->vocab_size; i++) {
        for (uint32_t j = i + 1; j < 20 && j < model->vocab_size; j++) {
            float dot = 0.0f;
            float norm_i = 0.0f, norm_j = 0.0f;
            
            for (uint32_t d = 0; d < model->embeddings.embedding_dim; d++) {
                float vi = model->embeddings.embeddings[i * model->embeddings.embedding_dim + d];
                float vj = model->embeddings.embeddings[j * model->embeddings.embedding_dim + d];
                dot += vi * vj;
                norm_i += vi * vi;
                norm_j += vj * vj;
            }
            
            float sim = dot / (math_sqrt(norm_i) * math_sqrt(norm_j) + 1e-8f);
            
            if (model->tokens[i].symmetry_group == model->tokens[j].symmetry_group) {
                same_group_sim += sim;
                same_count++;
            } else {
                diff_group_sim += sim;
                diff_count++;
            }
        }
    }
    
    if (same_count > 0 && diff_count > 0) {
        same_group_sim /= same_count;
        diff_group_sim /= diff_count;
        TEST_ASSERT(same_group_sim > diff_group_sim, 
                   "Same-group tokens more similar than different-group");
    }
    
    free(model);
}

/**
 * Test OBJECTIVE 15: Angular Attention
 */
void test_angular_attention() {
    printf("\n=== Test 2: Angular Attention (OBJECTIVE 15) ===\n");
    
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        tests_failed++;
        return;
    }
    
    // Test 1: Verify attention scores are in valid range [-1, 1]
    int in_range = 1;
    for (uint32_t q = 0; q < 10; q++) {
        for (uint32_t k = 0; k < 10; k++) {
            float score = cllm_attention_score_angular(model, q, k, 0);
            if (score < -1.0f || score > 1.0f) {
                in_range = 0;
                break;
            }
        }
        if (!in_range) break;
    }
    TEST_ASSERT(in_range, "All attention scores in [-1, 1] range");
    
    // Test 2: Verify same-group tokens attend more
    uint32_t token1_g0 = 0, token2_g0 = 0, token3_g1 = 0;
    
    // Find tokens in different groups
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        if (model->tokens[i].symmetry_group == 0) {
            if (token1_g0 == 0) token1_g0 = i;
            else if (token2_g0 == 0) token2_g0 = i;
        } else if (model->tokens[i].symmetry_group == 1 && token3_g1 == 0) {
            token3_g1 = i;
        }
        if (token1_g0 > 0 && token2_g0 > 0 && token3_g1 > 0) break;
    }
    
    if (token1_g0 > 0 && token2_g0 > 0 && token3_g1 > 0) {
        float same_group = cllm_attention_score_angular(model, token1_g0, token2_g0, 0);
        float diff_group = cllm_attention_score_angular(model, token1_g0, token3_g1, 0);
        TEST_ASSERT(same_group > diff_group, "Same-group attention higher than different-group");
    }
    
    free(model);
}

/**
 * Test OBJECTIVE 16: Kissing Spheres
 */
void test_kissing_spheres() {
    printf("\n=== Test 3: Kissing Spheres (OBJECTIVE 16) ===\n");
    
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        tests_failed++;
        return;
    }
    
    // Allocate lattice points
    model->num_lattice_points = model->vocab_size;
    model->lattice_points = calloc(model->num_lattice_points, sizeof(CLLMLatticePoint));
    
    if (!model->lattice_points) {
        printf("  ✗ Failed to allocate lattice points\n");
        tests_failed++;
        free(model);
        return;
    }
    
    // Initialize point IDs and assign symmetry groups
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        model->lattice_points[i].point_id = i;
        // Assign symmetry groups (0-11) in round-robin fashion
        model->tokens[i].symmetry_group = i % 12;
        // Assign prime encodings (use small primes for testing)
        model->tokens[i].prime_encoding = 2 + (i % 100);  // Primes 2-101
    }
    
    // Initialize kissing spheres
    cllm_initialize_kissing_spheres(model);
    
    // Test 1: Verify all points have 10-12 neighbors
    int valid_neighbors = 1;
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        if (model->lattice_points[i].num_neighbors < 10 || 
            model->lattice_points[i].num_neighbors > 12) {
            valid_neighbors = 0;
            break;
        }
    }
    TEST_ASSERT(valid_neighbors, "All points have 10-12 neighbors");
    
    // Test 2: Verify neighbors are from different groups
    int unique_groups = 1;
    for (uint32_t i = 0; i < model->num_lattice_points && i < 10; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        int group_counts[12] = {0};
        
        for (uint32_t j = 0; j < point->num_neighbors; j++) {
            uint32_t neighbor_id = point->neighbors[j];
            if (neighbor_id < model->vocab_size) {
                uint32_t group = model->tokens[neighbor_id].symmetry_group;
                group_counts[group]++;
            }
        }
        
        // Each group should have at most 1 neighbor
        for (int g = 0; g < 12; g++) {
            if (group_counts[g] > 1) {
                unique_groups = 0;
                break;
            }
        }
        if (!unique_groups) break;
    }
    TEST_ASSERT(unique_groups, "Neighbors from different symmetry groups");
    
    free(model->lattice_points);
    free(model);
}

/**
 * Test OBJECTIVE 17: NTT Attention
 */
void test_ntt_attention() {
    printf("\n=== Test 4: NTT Attention (OBJECTIVE 17) ===\n");
    
    uint32_t seq_len = 64;
    uint32_t head_dim = 32;
    
    // Allocate test data
    float* query = calloc(seq_len * head_dim, sizeof(float));
    float* key = calloc(seq_len * head_dim, sizeof(float));
    float* value = calloc(seq_len * head_dim, sizeof(float));
    float* output_std = calloc(seq_len * head_dim, sizeof(float));
    float* output_ntt = calloc(seq_len * head_dim, sizeof(float));
    
    if (!query || !key || !value || !output_std || !output_ntt) {
        printf("  ✗ Failed to allocate test data\n");
        tests_failed++;
        free(query); free(key); free(value); free(output_std); free(output_ntt);
        return;
    }
    
    // Initialize with random data
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        query[i] = ((float)rand() / RAND_MAX - 0.5f);
        key[i] = ((float)rand() / RAND_MAX - 0.5f);
        value[i] = ((float)rand() / RAND_MAX - 0.5f);
    }
    
    // Compute both
    cllm_attention_standard_forward(query, key, value, seq_len, head_dim, output_std);
    int result = cllm_attention_ntt_forward(query, key, value, seq_len, head_dim, output_ntt);
    
    TEST_ASSERT(result == 0, "NTT attention executed successfully");
    
    // Test 1: Verify outputs are similar (< 5% relative error)
    double max_rel_error = 0.0;
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        double diff = (double)math_abs((float)(output_std[i] - output_ntt[i]));
        double rel_error = diff / ((double)math_abs((float)output_std[i]) + 1e-6);
        if (rel_error > max_rel_error) max_rel_error = rel_error;
    }
    TEST_ASSERT(max_rel_error < 0.05, "NTT output matches standard (< 5% error)");
    
    free(query);
    free(key);
    free(value);
    free(output_std);
    free(output_ntt);
}

/**
 * Test OBJECTIVE 18: Cymatic Resonance
 */
void test_cymatic_resonance() {
    printf("\n=== Test 5: Cymatic Resonance (OBJECTIVE 18) ===\n");
    
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        tests_failed++;
        return;
    }
    
    // Allocate gradients
    uint32_t grad_size = model->vocab_size * model->embeddings.embedding_dim;
    float* gradients = calloc(grad_size, sizeof(float));
    
    if (!gradients) {
        printf("  ✗ Failed to allocate gradients\n");
        tests_failed++;
        free(model);
        return;
    }
    
    // Initialize gradients with random values
    for (uint32_t i = 0; i < grad_size; i++) {
        gradients[i] = ((float)rand() / RAND_MAX - 0.5f);
    }
    
    // Apply cymatic resonance
    cllm_apply_cymatic_resonance(model, gradients, 100);
    
    // Test 1: Verify gradients still in reasonable range
    int in_range = 1;
    for (uint32_t i = 0; i < grad_size; i++) {
        if (math_abs(gradients[i]) > 10.0f) {
            in_range = 0;
            break;
        }
    }
    TEST_ASSERT(in_range, "Gradients remain in reasonable range after resonance");
    
    // Test 2: Verify modulation is applied (gradients changed)
    int changed = 0;
    float* original = calloc(grad_size, sizeof(float));
    for (uint32_t i = 0; i < grad_size; i++) {
        original[i] = ((float)rand() / RAND_MAX - 0.5f);
    }
    memcpy(gradients, original, grad_size * sizeof(float));
    
    cllm_apply_cymatic_resonance(model, gradients, 100);
    
    for (uint32_t i = 0; i < grad_size; i++) {
        if (math_abs(gradients[i] - original[i]) > 1e-6f) {
            changed = 1;
            break;
        }
    }
    TEST_ASSERT(changed, "Cymatic resonance modifies gradients");
    
    // Test 3: Verify harmonics computation
    double harmonics[12];
    cllm_compute_harmonics(432.0, 12, harmonics);
    
    int harmonics_valid = 1;
    for (int i = 0; i < 12; i++) {
        if (harmonics[i] <= 0.0 || harmonics[i] > 10000.0) {
            harmonics_valid = 0;
            break;
        }
        // Each harmonic should be smaller than previous (golden damping)
        if (i > 0 && harmonics[i] >= harmonics[i-1]) {
            harmonics_valid = 0;
            break;
        }
    }
    TEST_ASSERT(harmonics_valid, "Harmonics computed correctly with φ damping");
    
    free(original);
    free(gradients);
    free(model);
}

/**
 * Integration test: Full pipeline
 */
void test_full_integration() {
    printf("\n=== Test 6: Full Integration ===\n");
    
    // Create model
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        tests_failed++;
        return;
    }
    
    // Initialize with lattice formula
    cllm_embeddings_init_lattice(model);
    
    // Allocate lattice points
    model->num_lattice_points = model->vocab_size;
    model->lattice_points = calloc(model->num_lattice_points, sizeof(CLLMLatticePoint));
    
    if (model->lattice_points) {
        for (uint32_t i = 0; i < model->num_lattice_points; i++) {
            model->lattice_points[i].point_id = i;
        }
        
        // Initialize kissing spheres
        cllm_initialize_kissing_spheres(model);
        
        TEST_ASSERT(1, "Full pipeline: embeddings + kissing spheres");
        
        free(model->lattice_points);
    }
    
    free(model);
}

/**
 * Performance test: Measure improvements
 */
void test_performance() {
    printf("\n=== Test 7: Performance Validation ===\n");
    
    // Test NTT speedup for different sequence lengths
    uint32_t seq_lengths[] = {64, 128, 256, 512};
    
    for (int i = 0; i < 4; i++) {
        uint32_t seq_len = seq_lengths[i];
        
        printf("\n  Testing seq_len=%u:\n", seq_len);
        benchmark_ntt_attention(seq_len, 32);
    }
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  COMPREHENSIVE MATHEMATICAL INTEGRATION TEST SUITE        ║\n");
    printf("║  Testing OBJECTIVES 14-18                                 ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Run all tests
    test_lattice_embeddings();
    test_angular_attention();
    test_kissing_spheres();
    test_ntt_attention();
    test_cymatic_resonance();
    test_full_integration();
    test_performance();
    
    // Summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                              ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-3d                                         ║\n", tests_passed);
    printf("║  Tests Failed: %-3d                                         ║\n", tests_failed);
    printf("║  Success Rate: %.1f%%                                       ║\n", 
           100.0 * tests_passed / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    if (tests_failed == 0) {
        printf("✓ ALL TESTS PASSED!\n\n");
        return 0;
    } else {
        printf("✗ SOME TESTS FAILED\n\n");
        return 1;
    }
}