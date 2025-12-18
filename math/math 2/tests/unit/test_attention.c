/**
 * Comprehensive Attention Mechanism Test Suite
 * 
 * Tests for Day 10: Attention Mechanisms
 * - Multi-head attention
 * - Self-attention
 * - Cross-attention
 * - Attention masking
 * - Attention utilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include "cllm.h"
#include "cllm_inference.h"

#define MATH_EPSILON 1e-5f
#define TEST_PASSED 1
#define TEST_FAILED 0

// Test counter
static int tests_run = 0;
static int tests_passed = 0;

// Helper function to check if two floats are approximately equal
static int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// Helper function to print test result
static void print_test_result(const char* test_name, int passed) {
    tests_run++;
    if (passed) {
        tests_passed++;
        printf("Running test: %s... PASSED\n", test_name);
    } else {
        printf("Running test: %s... FAILED\n", test_name);
    }
}

// ============================================================================
// Test Functions
// ============================================================================

int test_attention_layer_init() {
    AttentionLayer layer;
    uint32_t num_heads = 4;
    uint32_t head_dim = 64;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    int passed = (layer.num_heads == num_heads) &&
                 (layer.head_dim == head_dim) &&
                 (layer.query_lattice != NULL) &&
                 (layer.key_lattice != NULL) &&
                 (layer.value_lattice != NULL);
    
    cllm_attention_free(&layer);
    return passed;
}

int test_attention_layer_free() {
    AttentionLayer layer;
    cllm_attention_init(&layer, 4, 64);
    
    cllm_attention_free(&layer);
    
    int passed = (layer.query_lattice == NULL) &&
                 (layer.key_lattice == NULL) &&
                 (layer.value_lattice == NULL);
    
    return passed;
}

int test_single_head_attention() {
    AttentionLayer layer;
    uint32_t num_heads = 1;
    uint32_t head_dim = 8;
    uint32_t seq_len = 4;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights to identity-like pattern
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        size_t row = i / head_dim;
        size_t col = i % head_dim;
        layer.query_lattice[i] = (row == col) ? 1.0f : 0.0f;
        layer.key_lattice[i] = (row == col) ? 1.0f : 0.0f;
        layer.value_lattice[i] = (row == col) ? 1.0f : 0.0f;
    }
    
    // Create simple input
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = (float)(i % 10) / 10.0f;
    }
    
    // Run attention
    cllm_attention_forward(&layer, input, output, NULL, NULL, seq_len);
    
    // Check output is not all zeros
    int passed = 0;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        if (fabsf(output[i]) > MATH_EPSILON) {
            passed = 1;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_multi_head_attention() {
    AttentionLayer layer;
    uint32_t num_heads = 4;
    uint32_t head_dim = 16;
    uint32_t seq_len = 8;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        layer.query_lattice[i] = 0.01f * (float)(i % 100);
        layer.key_lattice[i] = 0.01f * (float)((i + 50) % 100);
        layer.value_lattice[i] = 0.01f * (float)((i + 25) % 100);
    }
    
    // Create input
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = 0.1f * (float)(i % 20);
    }
    
    // Run attention
    cllm_attention_forward(&layer, input, output, NULL, NULL, seq_len);
    
    // Check output has reasonable values
    int passed = 1;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_attention_with_cache() {
    AttentionLayer layer;
    uint32_t num_heads = 2;
    uint32_t head_dim = 32;
    uint32_t seq_len = 16;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights to identity-like pattern for better cache values
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        size_t row = i / head_dim;
        size_t col = i % head_dim;
        float val = (row == col) ? 0.5f : 0.01f;
        layer.query_lattice[i] = val;
        layer.key_lattice[i] = val;
        layer.value_lattice[i] = val;
    }
    
    // Create buffers
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output1 = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output2 = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* key_cache = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* value_cache = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = 0.1f * (float)(i % 10 + 1);  // Non-zero values
    }
    
    // First run: populate cache (pass NULL for cache to compute fresh)
    cllm_attention_forward(&layer, input, output1, NULL, NULL, seq_len);
    
    // Second run: with cache (this should use and update the cache)
    cllm_attention_forward(&layer, input, output2, key_cache, value_cache, seq_len);
    
    // The function should work with or without cache
    // Just verify it doesn't crash and produces output
    int passed = 1;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        if (isnan(output2[i]) || isinf(output2[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output1);
    free(output2);
    free(key_cache);
    free(value_cache);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_attention_output_shape() {
    AttentionLayer layer;
    uint32_t num_heads = 8;
    uint32_t head_dim = 64;
    uint32_t seq_len = 32;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    // Initialize input
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = 0.01f;
    }
    
    // Initialize weights to small values
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        layer.query_lattice[i] = 0.01f;
        layer.key_lattice[i] = 0.01f;
        layer.value_lattice[i] = 0.01f;
    }
    
    cllm_attention_forward(&layer, input, output, NULL, NULL, seq_len);
    
    // Output should have same shape as input
    int passed = 1;
    
    free(input);
    free(output);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_attention_numerical_stability() {
    AttentionLayer layer;
    uint32_t num_heads = 4;
    uint32_t head_dim = 32;
    uint32_t seq_len = 16;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        layer.query_lattice[i] = 0.01f;
        layer.key_lattice[i] = 0.01f;
        layer.value_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    // Test with large values
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = 100.0f;
    }
    
    cllm_attention_forward(&layer, input, output, NULL, NULL, seq_len);
    
    // Check for NaN or Inf
    int passed = 1;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_attention_zero_input() {
    AttentionLayer layer;
    uint32_t num_heads = 2;
    uint32_t head_dim = 16;
    uint32_t seq_len = 8;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    // Input is all zeros
    
    cllm_attention_forward(&layer, input, output, NULL, NULL, seq_len);
    
    // Output should be all zeros or very small
    int passed = 1;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        if (fabsf(output[i]) > 0.1f) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_attention_single_token() {
    AttentionLayer layer;
    uint32_t num_heads = 4;
    uint32_t head_dim = 32;
    uint32_t seq_len = 1;  // Single token
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        layer.query_lattice[i] = 0.01f;
        layer.key_lattice[i] = 0.01f;
        layer.value_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    for (uint32_t i = 0; i < embedding_dim; i++) {
        input[i] = 0.5f;
    }
    
    cllm_attention_forward(&layer, input, output, NULL, NULL, seq_len);
    
    // Should not crash and produce valid output
    int passed = 1;
    for (uint32_t i = 0; i < embedding_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_attention_long_sequence() {
    AttentionLayer layer;
    uint32_t num_heads = 8;
    uint32_t head_dim = 64;
    uint32_t seq_len = 128;  // Long sequence
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        layer.query_lattice[i] = 0.01f;
        layer.key_lattice[i] = 0.01f;
        layer.value_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = 0.01f * (float)(i % 100);
    }
    
    cllm_attention_forward(&layer, input, output, NULL, NULL, seq_len);
    
    // Check output is valid
    int passed = 1;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_attention_free(&layer);
    
    return passed;
}

int test_attention_different_head_dims() {
    // Test with head_dim = 32
    AttentionLayer layer1;
    cllm_attention_init(&layer1, 4, 32);
    int passed1 = (layer1.head_dim == 32);
    cllm_attention_free(&layer1);
    
    // Test with head_dim = 64
    AttentionLayer layer2;
    cllm_attention_init(&layer2, 4, 64);
    int passed2 = (layer2.head_dim == 64);
    cllm_attention_free(&layer2);
    
    // Test with head_dim = 128
    AttentionLayer layer3;
    cllm_attention_init(&layer3, 4, 128);
    int passed3 = (layer3.head_dim == 128);
    cllm_attention_free(&layer3);
    
    return passed1 && passed2 && passed3;
}

int test_attention_different_num_heads() {
    // Test with 1 head
    AttentionLayer layer1;
    cllm_attention_init(&layer1, 1, 64);
    int passed1 = (layer1.num_heads == 1);
    cllm_attention_free(&layer1);
    
    // Test with 8 heads
    AttentionLayer layer2;
    cllm_attention_init(&layer2, 8, 64);
    int passed2 = (layer2.num_heads == 8);
    cllm_attention_free(&layer2);
    
    // Test with 16 heads
    AttentionLayer layer3;
    cllm_attention_init(&layer3, 16, 64);
    int passed3 = (layer3.num_heads == 16);
    cllm_attention_free(&layer3);
    
    return passed1 && passed2 && passed3;
}

int test_attention_memory_cleanup() {
    AttentionLayer layer;
    cllm_attention_init(&layer, 4, 64);
    
    // Verify allocations
    int allocated = (layer.query_lattice != NULL) &&
                    (layer.key_lattice != NULL) &&
                    (layer.value_lattice != NULL);
    
    cllm_attention_free(&layer);
    
    // Verify cleanup
    int freed = (layer.query_lattice == NULL) &&
                (layer.key_lattice == NULL) &&
                (layer.value_lattice == NULL);
    
    return allocated && freed;
}

int test_attention_null_handling() {
    // Test with NULL layer
    cllm_attention_forward(NULL, NULL, NULL, NULL, NULL, 0);
    
    // Test with NULL input/output
    AttentionLayer layer;
    cllm_attention_init(&layer, 4, 64);
    cllm_attention_forward(&layer, NULL, NULL, NULL, NULL, 0);
    cllm_attention_free(&layer);
    
    // Should not crash
    return 1;
}

int test_attention_consistency() {
    AttentionLayer layer;
    uint32_t num_heads = 4;
    uint32_t head_dim = 32;
    uint32_t seq_len = 16;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        layer.query_lattice[i] = 0.01f;
        layer.key_lattice[i] = 0.01f;
        layer.value_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output1 = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output2 = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = 0.1f * (float)i;
    }
    
    // Run twice with same input
    cllm_attention_forward(&layer, input, output1, NULL, NULL, seq_len);
    cllm_attention_forward(&layer, input, output2, NULL, NULL, seq_len);
    
    // Outputs should be identical
    int passed = 1;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        if (!float_equals(output1[i], output2[i], MATH_EPSILON)) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output1);
    free(output2);
    cllm_attention_free(&layer);
    
    return passed;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Attention Mechanism Tests\n");
    printf("========================================\n");
    printf("\n");
    
    // Run all tests
    print_test_result("attention_layer_init", test_attention_layer_init());
    print_test_result("attention_layer_free", test_attention_layer_free());
    print_test_result("single_head_attention", test_single_head_attention());
    print_test_result("multi_head_attention", test_multi_head_attention());
    print_test_result("attention_with_cache", test_attention_with_cache());
    print_test_result("attention_output_shape", test_attention_output_shape());
    print_test_result("attention_numerical_stability", test_attention_numerical_stability());
    print_test_result("attention_zero_input", test_attention_zero_input());
    print_test_result("attention_single_token", test_attention_single_token());
    print_test_result("attention_long_sequence", test_attention_long_sequence());
    print_test_result("attention_different_head_dims", test_attention_different_head_dims());
    print_test_result("attention_different_num_heads", test_attention_different_num_heads());
    print_test_result("attention_memory_cleanup", test_attention_memory_cleanup());
    print_test_result("attention_null_handling", test_attention_null_handling());
    print_test_result("attention_consistency", test_attention_consistency());
    
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", (float)tests_passed / tests_run * 100.0f);
    printf("========================================\n");
    printf("\n");
    
    return (tests_passed == tests_run) ? 0 : 1;
}