/**
 * Comprehensive Feed-Forward Network Test Suite
 * 
 * Tests for Day 11: Feed-Forward Networks
 * - Dense layers (linear transformations)
 * - Activation functions (ReLU, GELU, etc.)
 * - Layer initialization and cleanup
 * - Batch processing
 * - Numerical stability
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include "cllm.h"

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

// Forward declarations
void cllm_feedforward_init(FeedForwardLayer* layer, uint32_t input_dim,
                           uint32_t hidden_dim, uint32_t output_dim);
void cllm_feedforward_free(FeedForwardLayer* layer);
void cllm_feedforward(FeedForwardLayer* layer, float* input, float* output);
void cllm_feedforward_batch(FeedForwardLayer* layer, float* input, 
                            float* output, int batch_size);
void cllm_activation_gelu(float* x, int size);
void cllm_activation_relu(float* x, int size);

// ============================================================================
// Test Functions
// ============================================================================

int test_feedforward_layer_init() {
    FeedForwardLayer layer;
    uint32_t input_dim = 64;
    uint32_t hidden_dim = 256;
    uint32_t output_dim = 64;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    int passed = (layer.input_dim == input_dim) &&
                 (layer.hidden_dim == hidden_dim) &&
                 (layer.output_dim == output_dim) &&
                 (layer.w1_lattice != NULL) &&
                 (layer.w2_lattice != NULL) &&
                 (layer.bias1 != NULL) &&
                 (layer.bias2 != NULL);
    
    cllm_feedforward_free(&layer);
    return passed;
}

int test_feedforward_layer_free() {
    FeedForwardLayer layer;
    cllm_feedforward_init(&layer, 64, 256, 64);
    
    cllm_feedforward_free(&layer);
    
    int passed = (layer.w1_lattice == NULL) &&
                 (layer.w2_lattice == NULL) &&
                 (layer.bias1 == NULL) &&
                 (layer.bias2 == NULL);
    
    return passed;
}

int test_feedforward_forward_pass() {
    FeedForwardLayer layer;
    uint32_t input_dim = 8;
    uint32_t hidden_dim = 16;
    uint32_t output_dim = 8;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    // Initialize weights to small values
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    for (size_t i = 0; i < w1_size; i++) {
        layer.w1_lattice[i] = 0.01f;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer.w2_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(input_dim, sizeof(float));
    float* output = (float*)calloc(output_dim, sizeof(float));
    
    for (uint32_t i = 0; i < input_dim; i++) {
        input[i] = 0.5f;
    }
    
    cllm_feedforward(&layer, input, output);
    
    // Check output is not all zeros
    int passed = 0;
    for (uint32_t i = 0; i < output_dim; i++) {
        if (fabsf(output[i]) > MATH_EPSILON) {
            passed = 1;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_feedforward_output_shape() {
    FeedForwardLayer layer;
    uint32_t input_dim = 32;
    uint32_t hidden_dim = 128;
    uint32_t output_dim = 64;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    float* input = (float*)calloc(input_dim, sizeof(float));
    float* output = (float*)calloc(output_dim, sizeof(float));
    
    for (uint32_t i = 0; i < input_dim; i++) {
        input[i] = 0.1f;
    }
    
    cllm_feedforward(&layer, input, output);
    
    // Output should have correct dimension
    int passed = 1;
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_feedforward_batch_processing() {
    FeedForwardLayer layer;
    uint32_t input_dim = 16;
    uint32_t hidden_dim = 32;
    uint32_t output_dim = 16;
    int batch_size = 4;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    // Initialize weights
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    for (size_t i = 0; i < w1_size; i++) {
        layer.w1_lattice[i] = 0.01f;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer.w2_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(batch_size * input_dim, sizeof(float));
    float* output = (float*)calloc(batch_size * output_dim, sizeof(float));
    
    for (int i = 0; i < batch_size * input_dim; i++) {
        input[i] = 0.1f * (float)(i % 10);
    }
    
    cllm_feedforward_batch(&layer, input, output, batch_size);
    
    // Check output is valid
    int passed = 1;
    for (int i = 0; i < batch_size * output_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_activation_relu() {
    int size = 10;
    float* x = (float*)malloc(size * sizeof(float));
    
    // Test with mixed positive and negative values
    x[0] = -1.0f;
    x[1] = 0.0f;
    x[2] = 1.0f;
    x[3] = -5.0f;
    x[4] = 5.0f;
    x[5] = -0.5f;
    x[6] = 0.5f;
    x[7] = -10.0f;
    x[8] = 10.0f;
    x[9] = 0.0f;
    
    cllm_activation_relu(x, size);
    
    // Check ReLU: max(0, x)
    int passed = float_equals(x[0], 0.0f, MATH_EPSILON) &&
                 float_equals(x[1], 0.0f, MATH_EPSILON) &&
                 float_equals(x[2], 1.0f, MATH_EPSILON) &&
                 float_equals(x[3], 0.0f, MATH_EPSILON) &&
                 float_equals(x[4], 5.0f, MATH_EPSILON) &&
                 float_equals(x[5], 0.0f, MATH_EPSILON) &&
                 float_equals(x[6], 0.5f, MATH_EPSILON) &&
                 float_equals(x[7], 0.0f, MATH_EPSILON) &&
                 float_equals(x[8], 10.0f, MATH_EPSILON) &&
                 float_equals(x[9], 0.0f, MATH_EPSILON);
    
    free(x);
    return passed;
}

int test_activation_gelu() {
    int size = 5;
    float* x = (float*)malloc(size * sizeof(float));
    
    x[0] = -2.0f;
    x[1] = -1.0f;
    x[2] = 0.0f;
    x[3] = 1.0f;
    x[4] = 2.0f;
    
    cllm_activation_gelu(x, size);
    
    // GELU should produce smooth non-linear transformation
    // Check that output is not NaN or Inf
    int passed = 1;
    for (int i = 0; i < size; i++) {
        if (isnan(x[i]) || isinf(x[i])) {
            passed = 0;
            break;
        }
    }
    
    // GELU(0) should be approximately 0
    if (passed && fabsf(x[2]) > 0.1f) {
        passed = 0;
    }
    
    free(x);
    return passed;
}

int test_feedforward_numerical_stability() {
    FeedForwardLayer layer;
    uint32_t input_dim = 32;
    uint32_t hidden_dim = 64;
    uint32_t output_dim = 32;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    // Initialize weights to small values
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    for (size_t i = 0; i < w1_size; i++) {
        layer.w1_lattice[i] = 0.01f;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer.w2_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(input_dim, sizeof(float));
    float* output = (float*)calloc(output_dim, sizeof(float));
    
    // Test with large values
    for (uint32_t i = 0; i < input_dim; i++) {
        input[i] = 100.0f;
    }
    
    cllm_feedforward(&layer, input, output);
    
    // Check for NaN or Inf
    int passed = 1;
    for (uint32_t i = 0; i < output_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_feedforward_zero_input() {
    FeedForwardLayer layer;
    uint32_t input_dim = 16;
    uint32_t hidden_dim = 32;
    uint32_t output_dim = 16;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    float* input = (float*)calloc(input_dim, sizeof(float));
    float* output = (float*)calloc(output_dim, sizeof(float));
    
    // Input is all zeros
    
    cllm_feedforward(&layer, input, output);
    
    // Output should be bias values (or zeros if bias is zero)
    int passed = 1;
    for (uint32_t i = 0; i < output_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_feedforward_different_dimensions() {
    // Test 1: Small dimensions
    FeedForwardLayer layer1;
    cllm_feedforward_init(&layer1, 4, 8, 4);
    int passed1 = (layer1.input_dim == 4 && layer1.hidden_dim == 8 && layer1.output_dim == 4);
    cllm_feedforward_free(&layer1);
    
    // Test 2: Medium dimensions
    FeedForwardLayer layer2;
    cllm_feedforward_init(&layer2, 64, 256, 64);
    int passed2 = (layer2.input_dim == 64 && layer2.hidden_dim == 256 && layer2.output_dim == 64);
    cllm_feedforward_free(&layer2);
    
    // Test 3: Large dimensions
    FeedForwardLayer layer3;
    cllm_feedforward_init(&layer3, 512, 2048, 512);
    int passed3 = (layer3.input_dim == 512 && layer3.hidden_dim == 2048 && layer3.output_dim == 512);
    cllm_feedforward_free(&layer3);
    
    return passed1 && passed2 && passed3;
}

int test_feedforward_asymmetric_dimensions() {
    // Test with different input and output dimensions
    FeedForwardLayer layer;
    uint32_t input_dim = 32;
    uint32_t hidden_dim = 128;
    uint32_t output_dim = 64;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    float* input = (float*)calloc(input_dim, sizeof(float));
    float* output = (float*)calloc(output_dim, sizeof(float));
    
    for (uint32_t i = 0; i < input_dim; i++) {
        input[i] = 0.1f;
    }
    
    // Initialize weights
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    for (size_t i = 0; i < w1_size; i++) {
        layer.w1_lattice[i] = 0.01f;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer.w2_lattice[i] = 0.01f;
    }
    
    cllm_feedforward(&layer, input, output);
    
    // Should work with different dimensions
    int passed = 1;
    for (uint32_t i = 0; i < output_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_feedforward_memory_cleanup() {
    FeedForwardLayer layer;
    cllm_feedforward_init(&layer, 64, 256, 64);
    
    // Verify allocations
    int allocated = (layer.w1_lattice != NULL) &&
                    (layer.w2_lattice != NULL) &&
                    (layer.bias1 != NULL) &&
                    (layer.bias2 != NULL);
    
    cllm_feedforward_free(&layer);
    
    // Verify cleanup
    int freed = (layer.w1_lattice == NULL) &&
                (layer.w2_lattice == NULL) &&
                (layer.bias1 == NULL) &&
                (layer.bias2 == NULL);
    
    return allocated && freed;
}

int test_feedforward_null_handling() {
    // Test with NULL layer
    cllm_feedforward(NULL, NULL, NULL);
    
    // Test with NULL input/output
    FeedForwardLayer layer;
    cllm_feedforward_init(&layer, 16, 32, 16);
    cllm_feedforward(&layer, NULL, NULL);
    cllm_feedforward_free(&layer);
    
    // Should not crash
    return 1;
}

int test_feedforward_consistency() {
    FeedForwardLayer layer;
    uint32_t input_dim = 16;
    uint32_t hidden_dim = 32;
    uint32_t output_dim = 16;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    // Initialize weights
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    for (size_t i = 0; i < w1_size; i++) {
        layer.w1_lattice[i] = 0.01f * (float)(i % 100);
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer.w2_lattice[i] = 0.01f * (float)(i % 100);
    }
    
    float* input = (float*)calloc(input_dim, sizeof(float));
    float* output1 = (float*)calloc(output_dim, sizeof(float));
    float* output2 = (float*)calloc(output_dim, sizeof(float));
    
    for (uint32_t i = 0; i < input_dim; i++) {
        input[i] = 0.1f * (float)i;
    }
    
    // Run twice with same input
    cllm_feedforward(&layer, input, output1);
    cllm_feedforward(&layer, input, output2);
    
    // Outputs should be identical
    int passed = 1;
    for (uint32_t i = 0; i < output_dim; i++) {
        if (!float_equals(output1[i], output2[i], MATH_EPSILON)) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output1);
    free(output2);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_activation_relu_edge_cases() {
    int size = 6;
    float* x = (float*)malloc(size * sizeof(float));
    
    x[0] = -INFINITY;
    x[1] = -1e10f;
    x[2] = -1e-10f;
    x[3] = 1e-10f;
    x[4] = 1e10f;
    x[5] = INFINITY;
    
    cllm_activation_relu(x, size);
    
    // Check edge cases
    int passed = float_equals(x[0], 0.0f, MATH_EPSILON) &&
                 float_equals(x[1], 0.0f, MATH_EPSILON) &&
                 float_equals(x[2], 0.0f, MATH_EPSILON) &&
                 (x[3] > 0.0f) &&
                 (x[4] > 0.0f) &&
                 (x[5] > 0.0f);
    
    free(x);
    return passed;
}

int test_activation_gelu_properties() {
    int size = 3;
    float* x = (float*)malloc(size * sizeof(float));
    
    // GELU should be approximately linear for small positive values
    x[0] = 0.1f;
    x[1] = 0.2f;
    x[2] = 0.3f;
    
    float x0_orig = x[0];
    float x1_orig = x[1];
    float x2_orig = x[2];
    
    cllm_activation_gelu(x, size);
    
    // GELU(x) ≈ 0.5x for small x
    // Check that output is close to 0.5 * input
    int passed = (x[0] > 0.0f && x[0] < x0_orig) &&
                 (x[1] > 0.0f && x[1] < x1_orig) &&
                 (x[2] > 0.0f && x[2] < x2_orig);
    
    free(x);
    return passed;
}

int test_feedforward_large_batch() {
    FeedForwardLayer layer;
    uint32_t input_dim = 32;
    uint32_t hidden_dim = 64;
    uint32_t output_dim = 32;
    int batch_size = 64;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    // Initialize weights
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    for (size_t i = 0; i < w1_size; i++) {
        layer.w1_lattice[i] = 0.01f;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer.w2_lattice[i] = 0.01f;
    }
    
    float* input = (float*)calloc(batch_size * input_dim, sizeof(float));
    float* output = (float*)calloc(batch_size * output_dim, sizeof(float));
    
    for (int i = 0; i < batch_size * input_dim; i++) {
        input[i] = 0.01f * (float)(i % 100);
    }
    
    cllm_feedforward_batch(&layer, input, output, batch_size);
    
    // Check all outputs are valid
    int passed = 1;
    for (int i = 0; i < batch_size * output_dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_feedforward_identity_weights() {
    FeedForwardLayer layer;
    uint32_t dim = 8;
    
    cllm_feedforward_init(&layer, dim, dim, dim);
    
    // Set weights to identity-like pattern
    for (uint32_t i = 0; i < dim; i++) {
        for (uint32_t j = 0; j < dim; j++) {
            layer.w1_lattice[i * dim + j] = (i == j) ? 1.0f : 0.0f;
            layer.w2_lattice[i * dim + j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    
    float* input = (float*)calloc(dim, sizeof(float));
    float* output = (float*)calloc(dim, sizeof(float));
    
    for (uint32_t i = 0; i < dim; i++) {
        input[i] = (float)i;
    }
    
    cllm_feedforward(&layer, input, output);
    
    // With identity weights and GELU, output should be related to input
    int passed = 1;
    for (uint32_t i = 0; i < dim; i++) {
        if (isnan(output[i]) || isinf(output[i])) {
            passed = 0;
            break;
        }
    }
    
    free(input);
    free(output);
    cllm_feedforward_free(&layer);
    
    return passed;
}

int test_activation_relu_array_sizes() {
    // Test with different array sizes
    for (int size = 1; size <= 100; size += 10) {
        float* x = (float*)malloc(size * sizeof(float));
        
        for (int i = 0; i < size; i++) {
            x[i] = (float)i - (float)(size / 2);
        }
        
        cllm_activation_relu(x, size);
        
        // Check all negative values became zero
        int valid = 1;
        for (int i = 0; i < size; i++) {
            if (x[i] < 0.0f) {
                valid = 0;
                break;
            }
        }
        
        free(x);
        
        if (!valid) return 0;
    }
    
    return 1;
}

int test_feedforward_weight_initialization() {
    FeedForwardLayer layer;
    uint32_t input_dim = 32;
    uint32_t hidden_dim = 64;
    uint32_t output_dim = 32;
    
    cllm_feedforward_init(&layer, input_dim, hidden_dim, output_dim);
    
    // Weights should be initialized to zero (calloc)
    int passed = 1;
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    
    for (size_t i = 0; i < w1_size; i++) {
        if (layer.w1_lattice[i] != 0.0f) {
            passed = 0;
            break;
        }
    }
    
    if (passed) {
        for (size_t i = 0; i < w2_size; i++) {
            if (layer.w2_lattice[i] != 0.0f) {
                passed = 0;
                break;
            }
        }
    }
    
    cllm_feedforward_free(&layer);
    return passed;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Feed-Forward Network Tests\n");
    printf("========================================\n");
    printf("\n");
    
    // Run all tests
    print_test_result("feedforward_layer_init", test_feedforward_layer_init());
    print_test_result("feedforward_layer_free", test_feedforward_layer_free());
    print_test_result("feedforward_forward_pass", test_feedforward_forward_pass());
    print_test_result("feedforward_output_shape", test_feedforward_output_shape());
    print_test_result("feedforward_batch_processing", test_feedforward_batch_processing());
    print_test_result("activation_relu", test_activation_relu());
    print_test_result("activation_gelu", test_activation_gelu());
    print_test_result("feedforward_numerical_stability", test_feedforward_numerical_stability());
    print_test_result("feedforward_zero_input", test_feedforward_zero_input());
    print_test_result("feedforward_different_dimensions", test_feedforward_different_dimensions());
    print_test_result("feedforward_asymmetric_dimensions", test_feedforward_asymmetric_dimensions());
    print_test_result("feedforward_memory_cleanup", test_feedforward_memory_cleanup());
    print_test_result("feedforward_null_handling", test_feedforward_null_handling());
    print_test_result("feedforward_consistency", test_feedforward_consistency());
    print_test_result("activation_relu_edge_cases", test_activation_relu_edge_cases());
    print_test_result("activation_gelu_properties", test_activation_gelu_properties());
    print_test_result("feedforward_large_batch", test_feedforward_large_batch());
    print_test_result("feedforward_identity_weights", test_feedforward_identity_weights());
    print_test_result("activation_relu_array_sizes", test_activation_relu_array_sizes());
    print_test_result("feedforward_weight_initialization", test_feedforward_weight_initialization());
    
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