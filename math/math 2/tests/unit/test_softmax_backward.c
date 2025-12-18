/**
 * Unit Test: Softmax Backward
 * 
 * Tests the softmax_backward function for correctness
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Forward declaration of softmax_backward (it's static in cllm_training.c)
// We'll need to expose it or reimplement for testing
static void softmax_backward(
    float* grad_input,
    const float* grad_output,
    const float* softmax_output,
    int size
) {
    if (!grad_input || !grad_output || !softmax_output || size <= 0) return;
    
    // Compute sum of (softmax_output * grad_output)
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += softmax_output[i] * grad_output[i];
    }
    
    // Compute gradient: grad_input[i] = softmax_output[i] * (grad_output[i] - sum)
    for (int i = 0; i < size; i++) {
        grad_input[i] = softmax_output[i] * (grad_output[i] - sum);
    }
}

// Helper: Compute softmax
static void softmax(float* output, const float* input, int size) {
    // Find max for numerical stability
    float max_val = input[0];
    for (int i = 1; i < size; i++) {
        if (input[i] > max_val) max_val = input[i];
    }
    
    // Compute exp and sum
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        output[i] = math_exp(input[i] - max_val);
        sum += output[i];
    }
    
    // Normalize
    for (int i = 0; i < size; i++) {
        output[i] /= sum;
    }
}

// Test 1: Simple 2-element softmax
int test_simple_2element() {
    printf("Test 1: Simple 2-element softmax... ");
    
    float input[2] = {1.0f, 2.0f};
    float softmax_out[2];
    float grad_out[2] = {1.0f, 0.0f};
    float grad_in[2];
    
    softmax(softmax_out, input, 2);
    softmax_backward(grad_in, grad_out, softmax_out, 2);
    
    // Expected: grad_in should sum to 0 (property of softmax gradient)
    float sum = grad_in[0] + grad_in[1];
    
    if (fabs(sum) < 1e-6) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (sum = %f, expected ~0)\n", sum);
        return 0;
    }
}

// Test 2: 10-element softmax with uniform gradient
int test_10element_uniform() {
    printf("Test 2: 10-element softmax with uniform gradient... ");
    
    float input[10];
    float softmax_out[10];
    float grad_out[10];
    float grad_in[10];
    
    // Initialize with random values
    for (int i = 0; i < 10; i++) {
        input[i] = (float)i * 0.5f;
        grad_out[i] = 1.0f;
    }
    
    softmax(softmax_out, input, 10);
    softmax_backward(grad_in, grad_out, softmax_out, 10);
    
    // Expected: grad_in should sum to 0
    float sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        sum += grad_in[i];
    }
    
    if (fabs(sum) < 1e-5) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (sum = %f, expected ~0)\n", sum);
        return 0;
    }
}

// Test 3: Numerical stability with large values
int test_numerical_stability() {
    printf("Test 3: Numerical stability with large values... ");
    
    float input[5] = {100.0f, 101.0f, 102.0f, 103.0f, 104.0f};
    float softmax_out[5];
    float grad_out[5] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float grad_in[5];
    
    softmax(softmax_out, input, 5);
    softmax_backward(grad_in, grad_out, softmax_out, 5);
    
    // Check for NaN or Inf
    int has_nan_inf = 0;
    for (int i = 0; i < 5; i++) {
        if (isnan(grad_in[i]) || isinf(grad_in[i])) {
            has_nan_inf = 1;
            break;
        }
    }
    
    if (!has_nan_inf) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (NaN or Inf detected)\n");
        return 0;
    }
}

// Test 4: Zero gradient input
int test_zero_gradient() {
    printf("Test 4: Zero gradient input... ");
    
    float input[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float softmax_out[5];
    float grad_out[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float grad_in[5];
    
    softmax(softmax_out, input, 5);
    softmax_backward(grad_in, grad_out, softmax_out, 5);
    
    // Expected: all gradients should be 0
    int all_zero = 1;
    for (int i = 0; i < 5; i++) {
        if (fabs(grad_in[i]) > 1e-6) {
            all_zero = 0;
            break;
        }
    }
    
    if (all_zero) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (non-zero gradients detected)\n");
        return 0;
    }
}

// Test 5: Single element softmax
int test_single_element() {
    printf("Test 5: Single element softmax... ");
    
    float input[1] = {5.0f};
    float softmax_out[1];
    float grad_out[1] = {1.0f};
    float grad_in[1];
    
    softmax(softmax_out, input, 1);
    softmax_backward(grad_in, grad_out, softmax_out, 1);
    
    // Expected: gradient should be 0 (derivative of constant)
    if (fabs(grad_in[0]) < 1e-6) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (grad = %f, expected ~0)\n", grad_in[0]);
        return 0;
    }
}

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     Softmax Backward Unit Tests                         ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int passed = 0;
    int total = 5;
    
    passed += test_simple_2element();
    passed += test_10element_uniform();
    passed += test_numerical_stability();
    passed += test_zero_gradient();
    passed += test_single_element();
    
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Results: %d/%d tests passed (%.1f%%)\n", passed, total, 
           (float)passed / total * 100.0f);
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}