/**
 * @file test_ntt_attention_backward.c
 * @brief Tests for NTT Attention Backward Pass
 */

#include "../include/ntt_attention.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define EPSILON 1e-5
#define GRADIENT_CHECK_EPSILON 1e-4

// ============================================================================
// TEST UTILITIES
// ============================================================================

/**
 * Check if two values are approximately equal
 */
static int approx_equal(double a, double b, double epsilon) {
    return fabs(a - b) < epsilon;
}

/**
 * Print matrix for debugging
 */
static void print_matrix(const char* name, const double* mat, uint32_t rows, uint32_t cols) {
    printf("%s:\n", name);
    for (uint32_t i = 0; i < rows; i++) {
        for (uint32_t j = 0; j < cols; j++) {
            printf("%.4f ", mat[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

/**
 * Compute numerical gradient using finite differences
 * 
 * f'(x) ≈ (f(x + ε) - f(x - ε)) / (2ε)
 */
static double compute_numerical_gradient(
    double* queries,
    const double* keys,
    const double* values,
    const double* grad_output,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor,
    uint32_t param_idx
) {
    double epsilon = GRADIENT_CHECK_EPSILON;
    double* output_plus = malloc(seq_len * head_dim * sizeof(double));
    double* output_minus = malloc(seq_len * head_dim * sizeof(double));
    
    // Save original value
    double original = queries[param_idx];
    
    // Compute f(x + ε)
    queries[param_idx] = original + epsilon;
    ntt_attention_single_head_double(
        output_plus, queries, keys, values,
        seq_len, head_dim, scale_factor
    );
    
    // Compute f(x - ε)
    queries[param_idx] = original - epsilon;
    ntt_attention_single_head_double(
        output_minus, queries, keys, values,
        seq_len, head_dim, scale_factor
    );
    
    // Restore original value
    queries[param_idx] = original;
    
    // Compute loss difference
    double loss_plus = 0.0;
    double loss_minus = 0.0;
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        loss_plus += output_plus[i] * grad_output[i];
        loss_minus += output_minus[i] * grad_output[i];
    }
    
    double numerical_grad = (loss_plus - loss_minus) / (2.0 * epsilon);
    
    free(output_plus);
    free(output_minus);
    
    return numerical_grad;
}

// ============================================================================
// TESTS
// ============================================================================

/**
 * Test 1: Small sequence gradient check
 */
static int test_small_sequence_gradient_check() {
    printf("Test 1: Small sequence gradient check\n");
    
    uint32_t seq_len = 4;
    uint32_t head_dim = 8;
    double scale_factor = 1.0 / sqrt((double)head_dim);
    
    // Allocate matrices
    double* queries = malloc(seq_len * head_dim * sizeof(double));
    double* keys = malloc(seq_len * head_dim * sizeof(double));
    double* values = malloc(seq_len * head_dim * sizeof(double));
    double* output = malloc(seq_len * head_dim * sizeof(double));
    double* attention_weights = malloc(seq_len * seq_len * sizeof(double));
    double* grad_output = malloc(seq_len * head_dim * sizeof(double));
    double* grad_queries = malloc(seq_len * head_dim * sizeof(double));
    double* grad_keys = malloc(seq_len * head_dim * sizeof(double));
    double* grad_values = malloc(seq_len * head_dim * sizeof(double));
    
    // Initialize with small random values
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        queries[i] = ((double)rand() / RAND_MAX) * 0.1;
        keys[i] = ((double)rand() / RAND_MAX) * 0.1;
        values[i] = ((double)rand() / RAND_MAX) * 0.1;
        grad_output[i] = ((double)rand() / RAND_MAX) * 0.1;
    }
    
    // Forward pass
    int forward_result = ntt_attention_single_head_double(
        output, queries, keys, values,
        seq_len, head_dim, scale_factor
    );
    
    if (!forward_result) {
        printf("  ✗ Forward pass failed\n");
        goto cleanup;
    }
    
    // Compute attention weights (needed for backward pass)
    // For now, we'll compute them manually
    // TODO: Store attention weights in forward pass
    
    // Backward pass
    int backward_result = ntt_attention_backward_single_head_double(
        grad_queries, grad_keys, grad_values,
        grad_output, queries, keys, values, attention_weights,
        seq_len, head_dim, scale_factor
    );
    
    if (!backward_result) {
        printf("  ✗ Backward pass failed\n");
        goto cleanup;
    }
    
    // Check a few gradients numerically
    int num_checks = 3;
    int passed = 1;
    
    for (int i = 0; i < num_checks; i++) {
        uint32_t idx = rand() % (seq_len * head_dim);
        double numerical = compute_numerical_gradient(
            queries, keys, values, grad_output,
            seq_len, head_dim, scale_factor, idx
        );
        double analytical = grad_queries[idx];
        
        if (!approx_equal(numerical, analytical, 1e-3)) {
            printf("  ✗ Gradient mismatch at index %u: numerical=%.6f, analytical=%.6f\n",
                   idx, numerical, analytical);
            passed = 0;
        }
    }
    
    if (passed) {
        printf("  ✓ Gradient check passed\n");
    }
    
cleanup:
    free(queries);
    free(keys);
    free(values);
    free(output);
    free(attention_weights);
    free(grad_output);
    free(grad_queries);
    free(grad_keys);
    free(grad_values);
    
    return passed;
}

/**
 * Test 2: Zero gradient test
 */
static int test_zero_gradients() {
    printf("Test 2: Zero gradient test\n");
    
    uint32_t seq_len = 4;
    uint32_t head_dim = 8;
    double scale_factor = 1.0 / sqrt((double)head_dim);
    
    // Allocate matrices
    double* queries = malloc(seq_len * head_dim * sizeof(double));
    double* keys = malloc(seq_len * head_dim * sizeof(double));
    double* values = malloc(seq_len * head_dim * sizeof(double));
    double* attention_weights = malloc(seq_len * seq_len * sizeof(double));
    double* grad_output = calloc(seq_len * head_dim, sizeof(double)); // All zeros
    double* grad_queries = malloc(seq_len * head_dim * sizeof(double));
    double* grad_keys = malloc(seq_len * head_dim * sizeof(double));
    double* grad_values = malloc(seq_len * head_dim * sizeof(double));
    
    // Initialize inputs
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        queries[i] = ((double)rand() / RAND_MAX);
        keys[i] = ((double)rand() / RAND_MAX);
        values[i] = ((double)rand() / RAND_MAX);
    }
    
    // Initialize attention weights (uniform for simplicity)
    for (uint32_t i = 0; i < seq_len * seq_len; i++) {
        attention_weights[i] = 1.0 / seq_len;
    }
    
    // Backward pass with zero gradients
    int result = ntt_attention_backward_single_head_double(
        grad_queries, grad_keys, grad_values,
        grad_output, queries, keys, values, attention_weights,
        seq_len, head_dim, scale_factor
    );
    
    if (!result) {
        printf("  ✗ Backward pass failed\n");
        goto cleanup;
    }
    
    // Check that all gradients are zero
    int passed = 1;
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        if (fabs(grad_queries[i]) > EPSILON || 
            fabs(grad_keys[i]) > EPSILON || 
            fabs(grad_values[i]) > EPSILON) {
            printf("  ✗ Non-zero gradient found\n");
            passed = 0;
            break;
        }
    }
    
    if (passed) {
        printf("  ✓ Zero gradient test passed\n");
    }
    
cleanup:
    free(queries);
    free(keys);
    free(values);
    free(attention_weights);
    free(grad_output);
    free(grad_queries);
    free(grad_keys);
    free(grad_values);
    
    return passed;
}

/**
 * Test 3: Multi-head backward pass
 */
static int test_multi_head_backward() {
    printf("Test 3: Multi-head backward pass\n");
    
    uint32_t seq_len = 4;
    uint32_t head_dim = 8;
    uint32_t num_heads = 2;
    double scale_factor = 1.0 / sqrt((double)head_dim);
    
    uint32_t total_size = num_heads * seq_len * head_dim;
    uint32_t weight_size = num_heads * seq_len * seq_len;
    
    // Allocate matrices
    double* queries = malloc(total_size * sizeof(double));
    double* keys = malloc(total_size * sizeof(double));
    double* values = malloc(total_size * sizeof(double));
    double* attention_weights = malloc(weight_size * sizeof(double));
    double* grad_output = malloc(total_size * sizeof(double));
    double* grad_queries = malloc(total_size * sizeof(double));
    double* grad_keys = malloc(total_size * sizeof(double));
    double* grad_values = malloc(total_size * sizeof(double));
    
    // Initialize
    for (uint32_t i = 0; i < total_size; i++) {
        queries[i] = ((double)rand() / RAND_MAX) * 0.1;
        keys[i] = ((double)rand() / RAND_MAX) * 0.1;
        values[i] = ((double)rand() / RAND_MAX) * 0.1;
        grad_output[i] = ((double)rand() / RAND_MAX) * 0.1;
    }
    
    // Initialize attention weights
    for (uint32_t i = 0; i < weight_size; i++) {
        attention_weights[i] = 1.0 / seq_len;
    }
    
    // Backward pass
    int result = ntt_attention_backward_multi_head_double(
        grad_queries, grad_keys, grad_values,
        grad_output, queries, keys, values, attention_weights,
        seq_len, head_dim, num_heads, scale_factor
    );
    
    if (!result) {
        printf("  ✗ Multi-head backward pass failed\n");
        goto cleanup;
    }
    
    printf("  ✓ Multi-head backward pass succeeded\n");
    
cleanup:
    free(queries);
    free(keys);
    free(values);
    free(attention_weights);
    free(grad_output);
    free(grad_queries);
    free(grad_keys);
    free(grad_values);
    
    return result;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("=== NTT Attention Backward Pass Tests ===\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: Small sequence gradient check
    total_tests++;
    if (test_small_sequence_gradient_check()) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 2: Zero gradients
    total_tests++;
    if (test_zero_gradients()) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 3: Multi-head
    total_tests++;
    if (test_multi_head_backward()) {
        passed_tests++;
    }
    printf("\n");
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed_tests, total_tests);
    
    if (passed_tests == total_tests) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}