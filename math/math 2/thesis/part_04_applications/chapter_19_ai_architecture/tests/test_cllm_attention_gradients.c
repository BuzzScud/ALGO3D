/**
 * @file test_cllm_attention_gradients.c
 * @brief Gradient checking tests for CLLM attention backward pass
 * 
 * MASTER_PLAN RULE 1 EXCEPTION: Test files MAY use math.h for validation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>  // ALLOWED in test files per MASTER_PLAN RULE 1
#include <stdbool.h>
#include "../include/cllm.h"
#include "../include/cllm_attention.h"

// Test configuration
#define EPSILON 1e-4
#define GRADIENT_TOLERANCE 1e-3
#define SMALL_SEQ_LEN 4
#define SMALL_EMBEDDING_DIM 8
#define SMALL_BATCH_SIZE 2

/**
 * Helper: Initialize small test model
 */
static CLLMModel* create_test_model(void) {
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    model->num_layers = 1;
    model->embedding_dim = SMALL_EMBEDDING_DIM;
    model->num_heads = 2;  // 2 heads, 4 dims per head
    model->vocab_size = 100;
    model->max_seq_len = 16;
    
    // Allocate layers
    model->layers = calloc(1, sizeof(*model->layers));
    if (!model->layers) {
        free(model);
        return NULL;
    }
    
    // Allocate weights (small random values)
    size_t weight_size = SMALL_EMBEDDING_DIM * SMALL_EMBEDDING_DIM;
    model->layers[0].query_weights = calloc(weight_size, sizeof(double));
    model->layers[0].key_weights = calloc(weight_size, sizeof(double));
    model->layers[0].value_weights = calloc(weight_size, sizeof(double));
    model->layers[0].output_weights = calloc(weight_size, sizeof(double));
    
    // Allocate gradients
    model->layers[0].query_grad = calloc(weight_size, sizeof(double));
    model->layers[0].key_grad = calloc(weight_size, sizeof(double));
    model->layers[0].value_grad = calloc(weight_size, sizeof(double));
    model->layers[0].output_grad = calloc(weight_size, sizeof(double));
    
    // Initialize with small random values
    for (size_t i = 0; i < weight_size; i++) {
        model->layers[0].query_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
        model->layers[0].key_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
        model->layers[0].value_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
        model->layers[0].output_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    // Disable NTT for testing (use standard attention which caches attention_weights)
    model->ntt.enabled = false;
    model->ntt.threshold_seq_len = 512;
    model->ntt.auto_select = false;
    
    return model;
}

/**
 * Helper: Free test model
 */
static void free_test_model(CLLMModel* model) {
    if (!model) return;
    
    if (model->layers) {
        free(model->layers[0].query_weights);
        free(model->layers[0].key_weights);
        free(model->layers[0].value_weights);
        free(model->layers[0].output_weights);
        free(model->layers[0].query_grad);
        free(model->layers[0].key_grad);
        free(model->layers[0].value_grad);
        free(model->layers[0].output_grad);
        free(model->layers);
    }
    
    free(model);
}

/**
 * Helper: Compute simple loss (sum of squares)
 */
static double compute_loss(const double* output, const double* target, size_t size) {
    double loss = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = output[i] - target[i];
        loss += diff * diff;
    }
    return loss * 0.5;  // 0.5 * sum(diff^2)
}

/**
 * Helper: Compute loss gradient (output - target)
 */
static void compute_loss_gradient(double* grad, const double* output, const double* target, size_t size) {
    for (size_t i = 0; i < size; i++) {
        grad[i] = output[i] - target[i];
    }
}

/**
 * Test 1: Gradient checking for query weights
 */
static bool test_query_weight_gradients(void) {
    printf("\nTest 1: Query weight gradient checking\n");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        return false;
    }
    
    // Enable training mode
    if (cllm_enable_training_mode(model, SMALL_BATCH_SIZE, SMALL_SEQ_LEN) != 0) {
        printf("  ✗ Failed to enable training mode\n");
        free_test_model(model);
        return false;
    }
    
    // Create test data
    size_t input_size = SMALL_BATCH_SIZE * SMALL_SEQ_LEN * SMALL_EMBEDDING_DIM;
    double* input = calloc(input_size, sizeof(double));
    double* output = calloc(input_size, sizeof(double));
    double* target = calloc(input_size, sizeof(double));
    double* grad_output = calloc(input_size, sizeof(double));
    
    // Initialize with small random values
    for (size_t i = 0; i < input_size; i++) {
        input[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
        target[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    // Forward pass
    cllm_attention_forward(model, 0, input, output, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
    
    // Debug: Check if output is non-zero
    double output_sum = 0.0;
    for (size_t i = 0; i < input_size; i++) {
        output_sum += fabs(output[i]);
    }
    printf("  Debug: Output sum = %.6f\n", output_sum);
    
    // Compute loss and gradient
    double loss = compute_loss(output, target, input_size);
    printf("  Debug: Loss = %.6f\n", loss);
    compute_loss_gradient(grad_output, output, target, input_size);
    
    // Debug: Check grad_output
    double grad_sum = 0.0;
    for (size_t i = 0; i < input_size; i++) {
        grad_sum += fabs(grad_output[i]);
    }
    printf("  Debug: Grad output sum = %.6f\n", grad_sum);
    
    // Backward pass (computes analytical gradients)
    cllm_attention_backward(model, 0, grad_output, input, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
    
    // Debug: Check if gradients were computed
    double query_grad_sum = 0.0;
    size_t weight_size = SMALL_EMBEDDING_DIM * SMALL_EMBEDDING_DIM;
    for (size_t i = 0; i < weight_size; i++) {
        query_grad_sum += fabs(model->layers[0].query_grad[i]);
    }
    printf("  Debug: Query grad sum = %.6f\n", query_grad_sum);
    
    // Check a few query weight gradients numerically
    int num_checks = 5;
    int passed = 0;
    
    for (int check = 0; check < num_checks; check++) {
        // Pick a random weight
        int idx = rand() % (SMALL_EMBEDDING_DIM * SMALL_EMBEDDING_DIM);
        
        // Save original weight
        double original_weight = model->layers[0].query_weights[idx];
        double analytical_grad = model->layers[0].query_grad[idx];
        
        // Compute numerical gradient
        // f(w + epsilon)
        model->layers[0].query_weights[idx] = original_weight + EPSILON;
        memset(output, 0, input_size * sizeof(double));
        cllm_attention_forward(model, 0, input, output, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
        double loss_plus = compute_loss(output, target, input_size);
        
        // f(w - epsilon)
        model->layers[0].query_weights[idx] = original_weight - EPSILON;
        memset(output, 0, input_size * sizeof(double));
        cllm_attention_forward(model, 0, input, output, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
        double loss_minus = compute_loss(output, target, input_size);
        
        // Numerical gradient
        double numerical_grad = (loss_plus - loss_minus) / (2.0 * EPSILON);
        
        // Restore original weight
        model->layers[0].query_weights[idx] = original_weight;
        
        // Compare
        double diff = fabs(analytical_grad - numerical_grad);
        
        // Use absolute error for near-zero gradients, relative error otherwise
        double max_grad = fabs(analytical_grad) > fabs(numerical_grad) ? 
                         fabs(analytical_grad) : fabs(numerical_grad);
        
        bool matches;
        if (max_grad < 1e-6) {
            // Both gradients are near zero - use absolute error
            matches = (diff < 1e-6);
        } else {
            // At least one gradient is non-zero - use relative error
            double relative_error = diff / (fabs(numerical_grad) + 1e-8);
            matches = (relative_error < GRADIENT_TOLERANCE);
        }
        
        if (matches) {
            passed++;
        } else {
            double relative_error = diff / (fabs(numerical_grad) + 1e-8);
            printf("  ⚠ Weight %d: analytical=%.6f, numerical=%.6f, error=%.6f\n",
                   idx, analytical_grad, numerical_grad, relative_error);
        }
    }
    
    // Cleanup
    free(input);
    free(output);
    free(target);
    free(grad_output);
    cllm_disable_training_mode(model);
    free_test_model(model);
    
    bool success = (passed >= num_checks - 1);  // Allow 1 failure
    printf("  %s Passed %d/%d gradient checks\n", 
           success ? "✓" : "✗", passed, num_checks);
    
    return success;
}

/**
 * Test 2: Zero gradient test
 */
static bool test_zero_gradients(void) {
    printf("\nTest 2: Zero gradient test\n");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        return false;
    }
    
    // Enable training mode
    if (cllm_enable_training_mode(model, SMALL_BATCH_SIZE, SMALL_SEQ_LEN) != 0) {
        printf("  ✗ Failed to enable training mode\n");
        free_test_model(model);
        return false;
    }
    
    // Create test data
    size_t input_size = SMALL_BATCH_SIZE * SMALL_SEQ_LEN * SMALL_EMBEDDING_DIM;
    double* input = calloc(input_size, sizeof(double));
    double* output = calloc(input_size, sizeof(double));
    double* grad_output = calloc(input_size, sizeof(double));  // All zeros
    
    // Initialize input
    for (size_t i = 0; i < input_size; i++) {
        input[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    // Forward pass
    cllm_attention_forward(model, 0, input, output, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
    
    // Backward pass with zero gradient
    cllm_attention_backward(model, 0, grad_output, input, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
    
    // Check that all gradients are zero (or very small)
    size_t weight_size = SMALL_EMBEDDING_DIM * SMALL_EMBEDDING_DIM;
    bool all_zero = true;
    
    for (size_t i = 0; i < weight_size; i++) {
        if (fabs(model->layers[0].query_grad[i]) > 1e-6 ||
            fabs(model->layers[0].key_grad[i]) > 1e-6 ||
            fabs(model->layers[0].value_grad[i]) > 1e-6 ||
            fabs(model->layers[0].output_grad[i]) > 1e-6) {
            all_zero = false;
            break;
        }
    }
    
    // Cleanup
    free(input);
    free(output);
    free(grad_output);
    cllm_disable_training_mode(model);
    free_test_model(model);
    
    printf("  %s All gradients are zero\n", all_zero ? "✓" : "✗");
    return all_zero;
}

/**
 * Test 3: Training mode statistics
 */
static bool test_training_statistics(void) {
    printf("\nTest 3: Training mode statistics\n");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("  ✗ Failed to create model\n");
        return false;
    }
    
    // Enable training mode
    if (cllm_enable_training_mode(model, SMALL_BATCH_SIZE, SMALL_SEQ_LEN) != 0) {
        printf("  ✗ Failed to enable training mode\n");
        free_test_model(model);
        return false;
    }
    
    // Create test data
    size_t input_size = SMALL_BATCH_SIZE * SMALL_SEQ_LEN * SMALL_EMBEDDING_DIM;
    double* input = calloc(input_size, sizeof(double));
    double* output = calloc(input_size, sizeof(double));
    double* grad_output = calloc(input_size, sizeof(double));
    
    // Initialize
    for (size_t i = 0; i < input_size; i++) {
        input[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
        grad_output[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    // Do 3 forward-backward passes
    for (int i = 0; i < 3; i++) {
        cllm_attention_forward(model, 0, input, output, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
        cllm_attention_backward(model, 0, grad_output, input, SMALL_BATCH_SIZE, SMALL_SEQ_LEN);
    }
    
    // Check statistics
    bool stats_correct = (model->training.forward_passes == 3 && 
                         model->training.backward_passes == 3);
    
    printf("  Forward passes: %lu (expected 3)\n", model->training.forward_passes);
    printf("  Backward passes: %lu (expected 3)\n", model->training.backward_passes);
    
    // Cleanup
    free(input);
    free(output);
    free(grad_output);
    cllm_disable_training_mode(model);
    free_test_model(model);
    
    printf("  %s Statistics tracking correct\n", stats_correct ? "✓" : "✗");
    return stats_correct;
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== CLLM Attention Gradient Tests ===\n");
    printf("Testing backward pass gradient computation\n");
    
    srand(42);  // Fixed seed for reproducibility
    
    int passed = 0;
    int total = 3;
    
    if (test_query_weight_gradients()) passed++;
    if (test_zero_gradients()) passed++;
    if (test_training_statistics()) passed++;
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    if (passed == total) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}