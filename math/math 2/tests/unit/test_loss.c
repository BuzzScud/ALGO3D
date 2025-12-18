#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <string.h>
#include "ai/cllm_loss.h"

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

int test_loss_computation_create() {
    LossConfig config = {
        .type = LOSS_CROSS_ENTROPY,
        .reduction = LOSS_REDUCTION_MEAN,
        .label_smoothing = 0.0f,
        .huber_delta = 1.0f,
        .ignore_index = false,
        .ignore_index_value = -1,
        .use_log_softmax = true
    };
    
    LossComputation* loss_comp = loss_computation_create(&config);
    int passed = (loss_comp != NULL);
    
    if (passed) {
        passed = (loss_comp->config.type == LOSS_CROSS_ENTROPY) &&
                 (loss_comp->config.reduction == LOSS_REDUCTION_MEAN) &&
                 (loss_comp->epsilon > 0.0f);
    }
    
    loss_computation_free(loss_comp);
    return passed;
}

int test_cross_entropy_forward() {
    // Create simple predictions and targets
    // Predictions: [batch_size=2, num_classes=3]
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create((uint32_t[]){2}, 1);
    
    // Set predictions (logits)
    tensor_set(predictions, (uint32_t[]){0, 0}, 2.0f);  // Class 0
    tensor_set(predictions, (uint32_t[]){0, 1}, 1.0f);
    tensor_set(predictions, (uint32_t[]){0, 2}, 0.1f);
    
    tensor_set(predictions, (uint32_t[]){1, 0}, 0.5f);
    tensor_set(predictions, (uint32_t[]){1, 1}, 2.5f);  // Class 1
    tensor_set(predictions, (uint32_t[]){1, 2}, 0.2f);
    
    // Set targets (class indices)
    tensor_set(targets, (uint32_t[]){0}, 0.0f);  // Target class 0
    tensor_set(targets, (uint32_t[]){1}, 1.0f);  // Target class 1
    
    // Compute loss
    float loss = loss_cross_entropy_forward(
        predictions, targets, NULL,
        LOSS_REDUCTION_MEAN, 0.0f, 1e-7f, NULL
    );
    
    int passed = !isnan(loss) && !isinf(loss) && loss > 0.0f;
    
    tensor_free(predictions);
    tensor_free(targets);
    
    return passed;
}

int test_cross_entropy_backward() {
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create((uint32_t[]){2}, 1);
    
    // Set predictions
    tensor_set(predictions, (uint32_t[]){0, 0}, 2.0f);
    tensor_set(predictions, (uint32_t[]){0, 1}, 1.0f);
    tensor_set(predictions, (uint32_t[]){0, 2}, 0.1f);
    
    tensor_set(predictions, (uint32_t[]){1, 0}, 0.5f);
    tensor_set(predictions, (uint32_t[]){1, 1}, 2.5f);
    tensor_set(predictions, (uint32_t[]){1, 2}, 0.2f);
    
    // Set targets
    tensor_set(targets, (uint32_t[]){0}, 0.0f);
    tensor_set(targets, (uint32_t[]){1}, 1.0f);
    
    // Compute gradients
    Tensor* gradients = loss_cross_entropy_backward(
        predictions, targets, NULL, 0.0f, 1e-7f
    );
    
    int passed = (gradients != NULL) && (gradients->total_size == predictions->total_size);
    
    // Check that gradients sum to approximately 0 for each sample
    if (passed) {
        float sum0 = tensor_get(gradients, (uint32_t[]){0, 0}) +
                     tensor_get(gradients, (uint32_t[]){0, 1}) +
                     tensor_get(gradients, (uint32_t[]){0, 2});
        float sum1 = tensor_get(gradients, (uint32_t[]){1, 0}) +
                     tensor_get(gradients, (uint32_t[]){1, 1}) +
                     tensor_get(gradients, (uint32_t[]){1, 2});
        
        passed = float_equals(sum0, 0.0f, 1e-4f) && float_equals(sum1, 0.0f, 1e-4f);
    }
    
    tensor_free(predictions);
    tensor_free(targets);
    tensor_free(gradients);
    
    return passed;
}

int test_mse_forward() {
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create(shape, 2);
    
    // Set predictions and targets
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i;
        targets->data[i] = (float)i + 1.0f;  // Off by 1
    }
    
    float loss = loss_mse_forward(
        predictions, targets, NULL,
        LOSS_REDUCTION_MEAN, NULL
    );
    
    // Expected: mean of (1^2) = 1.0
    int passed = float_equals(loss, 1.0f, MATH_EPSILON);
    
    tensor_free(predictions);
    tensor_free(targets);
    
    return passed;
}

int test_mse_backward() {
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create(shape, 2);
    
    // Set predictions and targets
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i;
        targets->data[i] = (float)i + 1.0f;
    }
    
    Tensor* gradients = loss_mse_backward(predictions, targets, NULL);
    
    int passed = (gradients != NULL) && (gradients->total_size == predictions->total_size);
    
    // Check gradient values: d_loss/d_pred = 2 * (pred - target) / N
    if (passed) {
        float expected_grad = 2.0f * (-1.0f) / (2.0f * 3.0f);  // -1/3
        passed = float_equals(gradients->data[0], expected_grad, MATH_EPSILON);
    }
    
    tensor_free(predictions);
    tensor_free(targets);
    tensor_free(gradients);
    
    return passed;
}

int test_mae_forward() {
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create(shape, 2);
    
    // Set predictions and targets
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i;
        targets->data[i] = (float)i + 2.0f;  // Off by 2
    }
    
    float loss = loss_mae_forward(
        predictions, targets, NULL,
        LOSS_REDUCTION_MEAN, NULL
    );
    
    // Expected: mean of |2| = 2.0
    int passed = float_equals(loss, 2.0f, MATH_EPSILON);
    
    tensor_free(predictions);
    tensor_free(targets);
    
    return passed;
}

int test_mae_backward() {
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create(shape, 2);
    
    // Set predictions and targets
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i + 1.0f;
        targets->data[i] = (float)i;
    }
    
    Tensor* gradients = loss_mae_backward(predictions, targets, NULL);
    
    int passed = (gradients != NULL) && (gradients->total_size == predictions->total_size);
    
    // Check gradient values: sign of (pred - target) / N
    if (passed) {
        float expected_grad = 1.0f / (2.0f * 3.0f);  // 1/6
        passed = float_equals(gradients->data[0], expected_grad, MATH_EPSILON);
    }
    
    tensor_free(predictions);
    tensor_free(targets);
    tensor_free(gradients);
    
    return passed;
}

int test_huber_forward() {
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create(shape, 2);
    
    // Set predictions and targets with small differences (quadratic region)
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i;
        targets->data[i] = (float)i + 0.5f;  // Small difference
    }
    
    float delta = 1.0f;
    float loss = loss_huber_forward(
        predictions, targets, NULL, delta,
        LOSS_REDUCTION_MEAN, NULL
    );
    
    // Expected: mean of 0.5 * (0.5^2) = 0.125
    int passed = float_equals(loss, 0.125f, MATH_EPSILON);
    
    tensor_free(predictions);
    tensor_free(targets);
    
    return passed;
}

int test_huber_backward() {
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create(shape, 2);
    
    // Set predictions and targets
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i + 0.5f;
        targets->data[i] = (float)i;
    }
    
    float delta = 1.0f;
    Tensor* gradients = loss_huber_backward(predictions, targets, NULL, delta);
    
    int passed = (gradients != NULL) && (gradients->total_size == predictions->total_size);
    
    tensor_free(predictions);
    tensor_free(targets);
    tensor_free(gradients);
    
    return passed;
}

int test_log_softmax() {
    uint32_t shape[] = {2, 3};
    Tensor* logits = tensor_create(shape, 2);
    
    // Set logits
    tensor_set(logits, (uint32_t[]){0, 0}, 2.0f);
    tensor_set(logits, (uint32_t[]){0, 1}, 1.0f);
    tensor_set(logits, (uint32_t[]){0, 2}, 0.1f);
    
    tensor_set(logits, (uint32_t[]){1, 0}, 0.5f);
    tensor_set(logits, (uint32_t[]){1, 1}, 2.5f);
    tensor_set(logits, (uint32_t[]){1, 2}, 0.2f);
    
    Tensor* log_probs = loss_log_softmax(logits);
    
    int passed = (log_probs != NULL) && (log_probs->total_size == logits->total_size);
    
    // Check that exp(log_probs) sum to 1 for each sample
    if (passed) {
        float sum0 = expf(tensor_get(log_probs, (uint32_t[]){0, 0})) +
                     expf(tensor_get(log_probs, (uint32_t[]){0, 1})) +
                     expf(tensor_get(log_probs, (uint32_t[]){0, 2}));
        
        passed = float_equals(sum0, 1.0f, 1e-4f);
    }
    
    tensor_free(logits);
    tensor_free(log_probs);
    
    return passed;
}

int test_softmax() {
    uint32_t shape[] = {2, 3};
    Tensor* logits = tensor_create(shape, 2);
    
    // Set logits
    tensor_set(logits, (uint32_t[]){0, 0}, 2.0f);
    tensor_set(logits, (uint32_t[]){0, 1}, 1.0f);
    tensor_set(logits, (uint32_t[]){0, 2}, 0.1f);
    
    Tensor* probs = loss_softmax(logits);
    
    int passed = (probs != NULL) && (probs->total_size == logits->total_size);
    
    // Check that probabilities sum to 1
    if (passed) {
        float sum = tensor_get(probs, (uint32_t[]){0, 0}) +
                    tensor_get(probs, (uint32_t[]){0, 1}) +
                    tensor_get(probs, (uint32_t[]){0, 2});
        
        passed = float_equals(sum, 1.0f, 1e-4f);
    }
    
    tensor_free(logits);
    tensor_free(probs);
    
    return passed;
}

int test_label_smoothing() {
    uint32_t shape[] = {2, 3};
    Tensor* targets = tensor_create(shape, 2);
    
    // One-hot targets
    tensor_set(targets, (uint32_t[]){0, 0}, 1.0f);
    tensor_set(targets, (uint32_t[]){0, 1}, 0.0f);
    tensor_set(targets, (uint32_t[]){0, 2}, 0.0f);
    
    float smoothing = 0.1f;
    Tensor* smoothed = loss_apply_label_smoothing(targets, smoothing);
    
    int passed = (smoothed != NULL);
    
    if (passed) {
        float target_val = tensor_get(smoothed, (uint32_t[]){0, 0});
        float non_target_val = tensor_get(smoothed, (uint32_t[]){0, 1});
        
        // Target should be (1 - smoothing + smoothing/3)
        // Non-target should be (smoothing/3)
        float expected_target = 1.0f - smoothing + smoothing / 3.0f;
        float expected_non_target = smoothing / 3.0f;
        
        passed = float_equals(target_val, expected_target, MATH_EPSILON) &&
                 float_equals(non_target_val, expected_non_target, MATH_EPSILON);
    }
    
    tensor_free(targets);
    tensor_free(smoothed);
    
    return passed;
}

int test_numerical_stability() {
    uint32_t shape[] = {2, 3};
    Tensor* tensor = tensor_create(shape, 2);
    
    // Set normal values
    for (size_t i = 0; i < tensor->total_size; i++) {
        tensor->data[i] = (float)i;
    }
    
    bool has_nan, has_inf;
    int passed = loss_check_numerical_stability(tensor, &has_nan, &has_inf);
    passed = passed && !has_nan && !has_inf;
    
    // Test with NaN
    tensor->data[0] = NAN;
    passed = passed && !loss_check_numerical_stability(tensor, &has_nan, &has_inf);
    passed = passed && has_nan;
    
    // Test with Inf
    tensor->data[0] = INFINITY;
    passed = passed && !loss_check_numerical_stability(tensor, &has_nan, &has_inf);
    passed = passed && has_inf;
    
    tensor_free(tensor);
    
    return passed;
}

int test_gradient_clipping_by_value() {
    uint32_t shape[] = {2, 3};
    Tensor* gradients = tensor_create(shape, 2);
    
    // Set gradients with large values
    gradients->data[0] = 5.0f;
    gradients->data[1] = -3.0f;
    gradients->data[2] = 0.5f;
    
    float clip_value = 2.0f;
    loss_clip_gradients_by_value(gradients, clip_value);
    
    int passed = float_equals(gradients->data[0], 2.0f, MATH_EPSILON) &&
                 float_equals(gradients->data[1], -2.0f, MATH_EPSILON) &&
                 float_equals(gradients->data[2], 0.5f, MATH_EPSILON);
    
    tensor_free(gradients);
    
    return passed;
}

int test_gradient_clipping_by_norm() {
    uint32_t shape[] = {2, 3};
    Tensor* gradients = tensor_create(shape, 2);
    
    // Set gradients
    for (size_t i = 0; i < gradients->total_size; i++) {
        gradients->data[i] = 1.0f;
    }
    
    float max_norm = 2.0f;
    float original_norm = loss_clip_gradients_by_norm(gradients, max_norm);
    
    // Original norm should be sqrt(6) ≈ 2.45
    int passed = (original_norm > 2.4f && original_norm < 2.5f);
    
    // New norm should be max_norm
    float new_norm = loss_compute_gradient_norm(gradients);
    passed = passed && float_equals(new_norm, max_norm, MATH_EPSILON);
    
    tensor_free(gradients);
    
    return passed;
}

int test_loss_with_mask() {
    uint32_t shape[] = {3, 2};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create((uint32_t[]){3}, 1);
    Tensor* mask = tensor_create((uint32_t[]){3}, 1);
    
    // Set predictions
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i;
    }
    
    // Set targets
    tensor_set(targets, (uint32_t[]){0}, 0.0f);
    tensor_set(targets, (uint32_t[]){1}, 1.0f);
    tensor_set(targets, (uint32_t[]){2}, 0.0f);
    
    // Set mask (mask out second sample)
    tensor_set(mask, (uint32_t[]){0}, 1.0f);
    tensor_set(mask, (uint32_t[]){1}, 0.0f);  // Masked
    tensor_set(mask, (uint32_t[]){2}, 1.0f);
    
    float loss = loss_cross_entropy_forward(
        predictions, targets, mask,
        LOSS_REDUCTION_MEAN, 0.0f, 1e-7f, NULL
    );
    
    int passed = !isnan(loss) && !isinf(loss);
    
    tensor_free(predictions);
    tensor_free(targets);
    tensor_free(mask);
    
    return passed;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("\n========================================\n");
    printf("  Loss Computation Tests\n");
    printf("========================================\n\n");
    
    // Run tests
    print_test_result("loss_computation_create", test_loss_computation_create());
    print_test_result("cross_entropy_forward", test_cross_entropy_forward());
    print_test_result("cross_entropy_backward", test_cross_entropy_backward());
    print_test_result("mse_forward", test_mse_forward());
    print_test_result("mse_backward", test_mse_backward());
    print_test_result("mae_forward", test_mae_forward());
    print_test_result("mae_backward", test_mae_backward());
    print_test_result("huber_forward", test_huber_forward());
    print_test_result("huber_backward", test_huber_backward());
    print_test_result("log_softmax", test_log_softmax());
    print_test_result("softmax", test_softmax());
    print_test_result("label_smoothing", test_label_smoothing());
    print_test_result("numerical_stability", test_numerical_stability());
    print_test_result("gradient_clipping_by_value", test_gradient_clipping_by_value());
    print_test_result("gradient_clipping_by_norm", test_gradient_clipping_by_norm());
    print_test_result("loss_with_mask", test_loss_with_mask());
    
    // Print summary
    printf("\n========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", (float)tests_passed / tests_run * 100.0f);
    printf("========================================\n\n");
    
    return (tests_passed == tests_run) ? 0 : 1;
}