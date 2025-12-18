#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <string.h>
#include "ai/cllm_backprop.h"
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

int test_gradient_buffer_create() {
    size_t size = 100;
    int sphere_id = 1;
    int symmetry_group = 5;
    
    GradientBuffer* buffer = gradient_buffer_create(size, sphere_id, symmetry_group);
    
    int passed = (buffer != NULL) &&
                 (buffer->size == size) &&
                 (buffer->sphere_id == sphere_id) &&
                 (buffer->symmetry_group == symmetry_group) &&
                 (buffer->data != NULL);
    
    gradient_buffer_free(buffer);
    return passed;
}

int test_gradient_buffer_zero() {
    GradientBuffer* buffer = gradient_buffer_create(10, 1, 0);
    
    // Set some values
    for (size_t i = 0; i < buffer->size; i++) {
        buffer->data[i] = (float)i;
    }
    
    // Zero out
    gradient_buffer_zero(buffer);
    
    int passed = 1;
    for (size_t i = 0; i < buffer->size; i++) {
        if (buffer->data[i] != 0.0f) {
            passed = 0;
            break;
        }
    }
    
    gradient_buffer_free(buffer);
    return passed;
}

int test_gradient_buffer_copy() {
    GradientBuffer* buffer = gradient_buffer_create(10, 1, 0);
    
    // Set values
    for (size_t i = 0; i < buffer->size; i++) {
        buffer->data[i] = (float)i;
    }
    buffer->batch_count = 5;
    
    GradientBuffer* copy = gradient_buffer_copy(buffer);
    
    int passed = (copy != NULL) &&
                 (copy->size == buffer->size) &&
                 (copy->batch_count == buffer->batch_count);
    
    if (passed) {
        for (size_t i = 0; i < buffer->size; i++) {
            if (copy->data[i] != buffer->data[i]) {
                passed = 0;
                break;
            }
        }
    }
    
    gradient_buffer_free(buffer);
    gradient_buffer_free(copy);
    return passed;
}

int test_gradient_buffer_add() {
    GradientBuffer* buffer1 = gradient_buffer_create(10, 1, 0);
    GradientBuffer* buffer2 = gradient_buffer_create(10, 2, 0);
    
    // Set values
    for (size_t i = 0; i < buffer1->size; i++) {
        buffer1->data[i] = 1.0f;
        buffer2->data[i] = 2.0f;
    }
    buffer1->batch_count = 1;
    buffer2->batch_count = 2;
    
    gradient_buffer_add(buffer1, buffer2);
    
    int passed = (buffer1->batch_count == 3);
    
    if (passed) {
        for (size_t i = 0; i < buffer1->size; i++) {
            if (!float_equals(buffer1->data[i], 3.0f, MATH_EPSILON)) {
                passed = 0;
                break;
            }
        }
    }
    
    gradient_buffer_free(buffer1);
    gradient_buffer_free(buffer2);
    return passed;
}

int test_gradient_buffer_scale() {
    GradientBuffer* buffer = gradient_buffer_create(10, 1, 0);
    
    // Set values
    for (size_t i = 0; i < buffer->size; i++) {
        buffer->data[i] = 2.0f;
    }
    
    gradient_buffer_scale(buffer, 0.5f);
    
    int passed = 1;
    for (size_t i = 0; i < buffer->size; i++) {
        if (!float_equals(buffer->data[i], 1.0f, MATH_EPSILON)) {
            passed = 0;
            break;
        }
    }
    
    gradient_buffer_free(buffer);
    return passed;
}

int test_gradient_buffer_compute_stats() {
    GradientBuffer* buffer = gradient_buffer_create(4, 1, 0);
    
    // Set values: [1, 2, 3, 4]
    buffer->data[0] = 1.0f;
    buffer->data[1] = 2.0f;
    buffer->data[2] = 3.0f;
    buffer->data[3] = 4.0f;
    
    gradient_buffer_compute_stats(buffer);
    
    // Expected: norm = sqrt(1+4+9+16) = sqrt(30) ≈ 5.477
    // mean = (1+2+3+4)/4 = 2.5
    // min = 1, max = 4
    
    int passed = (buffer->norm > 5.4f && buffer->norm < 5.5f) &&
                 float_equals(buffer->mean_value, 2.5f, MATH_EPSILON) &&
                 float_equals(buffer->min_value, 1.0f, MATH_EPSILON) &&
                 float_equals(buffer->max_value, 4.0f, MATH_EPSILON);
    
    gradient_buffer_free(buffer);
    return passed;
}

int test_gradient_buffer_check_stability() {
    GradientBuffer* buffer = gradient_buffer_create(3, 1, 0);
    
    // Normal values
    buffer->data[0] = 1.0f;
    buffer->data[1] = 2.0f;
    buffer->data[2] = 3.0f;
    
    bool has_nan, has_inf;
    int passed = gradient_buffer_check_stability(buffer, &has_nan, &has_inf);
    passed = passed && !has_nan && !has_inf;
    
    // Add NaN
    buffer->data[0] = NAN;
    passed = passed && !gradient_buffer_check_stability(buffer, &has_nan, &has_inf);
    passed = passed && has_nan;
    
    // Add Inf
    buffer->data[0] = INFINITY;
    passed = passed && !gradient_buffer_check_stability(buffer, &has_nan, &has_inf);
    passed = passed && has_inf;
    
    gradient_buffer_free(buffer);
    return passed;
}

int test_gradient_buffer_clip_by_value() {
    GradientBuffer* buffer = gradient_buffer_create(5, 1, 0);
    
    buffer->data[0] = 5.0f;
    buffer->data[1] = -3.0f;
    buffer->data[2] = 0.5f;
    buffer->data[3] = -10.0f;
    buffer->data[4] = 2.0f;
    
    gradient_buffer_clip_by_value(buffer, 2.0f);
    
    int passed = float_equals(buffer->data[0], 2.0f, MATH_EPSILON) &&
                 float_equals(buffer->data[1], -2.0f, MATH_EPSILON) &&
                 float_equals(buffer->data[2], 0.5f, MATH_EPSILON) &&
                 float_equals(buffer->data[3], -2.0f, MATH_EPSILON) &&
                 float_equals(buffer->data[4], 2.0f, MATH_EPSILON);
    
    gradient_buffer_free(buffer);
    return passed;
}

int test_gradient_buffer_clip_by_norm() {
    GradientBuffer* buffer = gradient_buffer_create(3, 1, 0);
    
    // Set values with norm = sqrt(1+4+9) = sqrt(14) ≈ 3.74
    buffer->data[0] = 1.0f;
    buffer->data[1] = 2.0f;
    buffer->data[2] = 3.0f;
    
    float original_norm = gradient_buffer_clip_by_norm(buffer, 2.0f);
    
    int passed = (original_norm > 3.7f && original_norm < 3.8f);
    
    // Check new norm
    float sum_sq = 0.0f;
    for (size_t i = 0; i < buffer->size; i++) {
        sum_sq += buffer->data[i] * buffer->data[i];
    }
    float new_norm = sqrtf(sum_sq);
    
    passed = passed && float_equals(new_norm, 2.0f, MATH_EPSILON);
    
    gradient_buffer_free(buffer);
    return passed;
}

int test_backprop_create() {
    size_t gradient_size = 100;
    int sphere_id = 1;
    int symmetry_group = 5;
    
    BackpropContext* ctx = backprop_create(
        gradient_size, sphere_id, symmetry_group,
        GRADIENT_ACCUMULATION_IMMEDIATE
    );
    
    int passed = (ctx != NULL) &&
                 (ctx->local_gradients != NULL) &&
                 (ctx->local_gradients->size == gradient_size) &&
                 (ctx->strategy == GRADIENT_ACCUMULATION_IMMEDIATE);
    
    backprop_free(ctx);
    return passed;
}

int test_backprop_reset() {
    BackpropContext* ctx = backprop_create(10, 1, 0, GRADIENT_ACCUMULATION_IMMEDIATE);
    
    // Set some values
    for (size_t i = 0; i < ctx->local_gradients->size; i++) {
        ctx->local_gradients->data[i] = (float)i;
    }
    ctx->batches_processed = 5;
    
    backprop_reset(ctx);
    
    int passed = (ctx->batches_processed == 0);
    
    if (passed) {
        for (size_t i = 0; i < ctx->local_gradients->size; i++) {
            if (ctx->local_gradients->data[i] != 0.0f) {
                passed = 0;
                break;
            }
        }
    }
    
    backprop_free(ctx);
    return passed;
}

int test_backprop_register_child() {
    BackpropContext* ctx = backprop_create(10, 1, 0, GRADIENT_ACCUMULATION_HIERARCHICAL);
    GradientBuffer* child_buffer = gradient_buffer_create(10, 2, 1);
    
    int passed = backprop_register_child(ctx, child_buffer);
    passed = passed && (ctx->num_children == 1);
    passed = passed && (ctx->child_gradients[0] == child_buffer);
    
    gradient_buffer_free(child_buffer);
    backprop_free(ctx);
    return passed;
}

int test_backprop_accumulate_batch() {
    BackpropContext* ctx = backprop_create(10, 1, 0, GRADIENT_ACCUMULATION_IMMEDIATE);
    GradientBuffer* batch_gradients = gradient_buffer_create(10, 1, 0);
    
    // Set batch gradients
    for (size_t i = 0; i < batch_gradients->size; i++) {
        batch_gradients->data[i] = 1.0f;
    }
    batch_gradients->batch_count = 1;
    
    // Accumulate twice
    backprop_accumulate_batch(ctx, batch_gradients);
    backprop_accumulate_batch(ctx, batch_gradients);
    
    int passed = (ctx->batches_processed == 2);
    
    if (passed) {
        for (size_t i = 0; i < ctx->local_gradients->size; i++) {
            if (!float_equals(ctx->local_gradients->data[i], 2.0f, MATH_EPSILON)) {
                passed = 0;
                break;
            }
        }
    }
    
    gradient_buffer_free(batch_gradients);
    backprop_free(ctx);
    return passed;
}

int test_backprop_accumulate_from_children() {
    BackpropContext* ctx = backprop_create(10, 1, 0, GRADIENT_ACCUMULATION_HIERARCHICAL);
    
    // Create and register two children
    GradientBuffer* child1 = gradient_buffer_create(10, 2, 1);
    GradientBuffer* child2 = gradient_buffer_create(10, 3, 2);
    
    for (size_t i = 0; i < child1->size; i++) {
        child1->data[i] = 1.0f;
        child2->data[i] = 2.0f;
    }
    child1->batch_count = 1;
    child2->batch_count = 1;
    child1->is_ready = true;
    child2->is_ready = true;
    
    backprop_register_child(ctx, child1);
    backprop_register_child(ctx, child2);
    
    int passed = backprop_accumulate_from_children(ctx);
    
    if (passed) {
        for (size_t i = 0; i < ctx->local_gradients->size; i++) {
            if (!float_equals(ctx->local_gradients->data[i], 3.0f, MATH_EPSILON)) {
                passed = 0;
                break;
            }
        }
    }
    
    gradient_buffer_free(child1);
    gradient_buffer_free(child2);
    backprop_free(ctx);
    return passed;
}

int test_backprop_average_gradients() {
    BackpropContext* ctx = backprop_create(10, 1, 0, GRADIENT_ACCUMULATION_IMMEDIATE);
    
    // Set gradients
    for (size_t i = 0; i < ctx->local_gradients->size; i++) {
        ctx->local_gradients->data[i] = 10.0f;
    }
    
    backprop_average_gradients(ctx, 5);
    
    int passed = 1;
    for (size_t i = 0; i < ctx->local_gradients->size; i++) {
        if (!float_equals(ctx->local_gradients->data[i], 2.0f, MATH_EPSILON)) {
            passed = 0;
            break;
        }
    }
    
    backprop_free(ctx);
    return passed;
}

int test_backprop_finalize_gradients() {
    BackpropContext* ctx = backprop_create(10, 1, 0, GRADIENT_ACCUMULATION_IMMEDIATE);
    
    // Set gradients
    for (size_t i = 0; i < ctx->local_gradients->size; i++) {
        ctx->local_gradients->data[i] = (float)i;
    }
    
    int passed = backprop_finalize_gradients(ctx);
    passed = passed && ctx->local_gradients->is_ready;
    passed = passed && (ctx->gradient_updates == 1);
    
    backprop_free(ctx);
    return passed;
}

int test_backprop_compute_batch() {
    // Create loss computation
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
    
    // Create backprop context
    BackpropContext* ctx = backprop_create(6, 1, 0, GRADIENT_ACCUMULATION_IMMEDIATE);
    
    // Create predictions and targets
    uint32_t shape[] = {2, 3};
    Tensor* predictions = tensor_create(shape, 2);
    Tensor* targets = tensor_create((uint32_t[]){2}, 1);
    
    for (size_t i = 0; i < predictions->total_size; i++) {
        predictions->data[i] = (float)i;
    }
    tensor_set(targets, (uint32_t[]){0}, 0.0f);
    tensor_set(targets, (uint32_t[]){1}, 1.0f);
    
    // Compute batch
    BackpropResult* result = backprop_compute_batch(ctx, loss_comp, predictions, targets, NULL);
    
    int passed = (result != NULL) &&
                 (result->gradients != NULL) &&
                 !result->has_nan &&
                 !result->has_inf;
    
    backprop_result_free(result);
    tensor_free(predictions);
    tensor_free(targets);
    backprop_free(ctx);
    loss_computation_free(loss_comp);
    
    return passed;
}

int test_tensor_to_buffer_conversion() {
    uint32_t shape[] = {2, 3};
    Tensor* tensor = tensor_create(shape, 2);
    
    for (size_t i = 0; i < tensor->total_size; i++) {
        tensor->data[i] = (float)i;
    }
    
    GradientBuffer* buffer = backprop_tensor_to_buffer(tensor, 1, 0);
    
    int passed = (buffer != NULL) &&
                 (buffer->size == tensor->total_size);
    
    if (passed) {
        for (size_t i = 0; i < tensor->total_size; i++) {
            if (!float_equals(buffer->data[i], tensor->data[i], MATH_EPSILON)) {
                passed = 0;
                break;
            }
        }
    }
    
    tensor_free(tensor);
    gradient_buffer_free(buffer);
    return passed;
}

int test_buffer_to_tensor_conversion() {
    GradientBuffer* buffer = gradient_buffer_create(6, 1, 0);
    
    for (size_t i = 0; i < buffer->size; i++) {
        buffer->data[i] = (float)i;
    }
    
    size_t shape[] = {2, 3};
    Tensor* tensor = backprop_buffer_to_tensor(buffer, shape, 2);
    
    int passed = (tensor != NULL) &&
                 (tensor->total_size == buffer->size);
    
    if (passed) {
        for (size_t i = 0; i < buffer->size; i++) {
            if (!float_equals(tensor->data[i], buffer->data[i], MATH_EPSILON)) {
                passed = 0;
                break;
            }
        }
    }
    
    gradient_buffer_free(buffer);
    tensor_free(tensor);
    return passed;
}

int test_merge_buffers() {
    GradientBuffer* buffer1 = gradient_buffer_create(10, 1, 0);
    GradientBuffer* buffer2 = gradient_buffer_create(10, 2, 1);
    
    for (size_t i = 0; i < buffer1->size; i++) {
        buffer1->data[i] = 1.0f;
        buffer2->data[i] = 2.0f;
    }
    buffer1->batch_count = 1;
    buffer2->batch_count = 1;
    
    GradientBuffer* buffers[] = {buffer1, buffer2};
    GradientBuffer* merged = backprop_merge_buffers(buffers, 2);
    
    int passed = (merged != NULL) &&
                 (merged->batch_count == 2);
    
    if (passed) {
        for (size_t i = 0; i < merged->size; i++) {
            if (!float_equals(merged->data[i], 3.0f, MATH_EPSILON)) {
                passed = 0;
                break;
            }
        }
    }
    
    gradient_buffer_free(buffer1);
    gradient_buffer_free(buffer2);
    gradient_buffer_free(merged);
    return passed;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    printf("\n========================================\n");
    printf("  Backpropagation Tests\n");
    printf("========================================\n\n");
    
    // Run tests
    print_test_result("gradient_buffer_create", test_gradient_buffer_create());
    print_test_result("gradient_buffer_zero", test_gradient_buffer_zero());
    print_test_result("gradient_buffer_copy", test_gradient_buffer_copy());
    print_test_result("gradient_buffer_add", test_gradient_buffer_add());
    print_test_result("gradient_buffer_scale", test_gradient_buffer_scale());
    print_test_result("gradient_buffer_compute_stats", test_gradient_buffer_compute_stats());
    print_test_result("gradient_buffer_check_stability", test_gradient_buffer_check_stability());
    print_test_result("gradient_buffer_clip_by_value", test_gradient_buffer_clip_by_value());
    print_test_result("gradient_buffer_clip_by_norm", test_gradient_buffer_clip_by_norm());
    print_test_result("backprop_create", test_backprop_create());
    print_test_result("backprop_reset", test_backprop_reset());
    print_test_result("backprop_register_child", test_backprop_register_child());
    print_test_result("backprop_accumulate_batch", test_backprop_accumulate_batch());
    print_test_result("backprop_accumulate_from_children", test_backprop_accumulate_from_children());
    print_test_result("backprop_average_gradients", test_backprop_average_gradients());
    print_test_result("backprop_finalize_gradients", test_backprop_finalize_gradients());
    print_test_result("backprop_compute_batch", test_backprop_compute_batch());
    print_test_result("tensor_to_buffer_conversion", test_tensor_to_buffer_conversion());
    print_test_result("buffer_to_tensor_conversion", test_buffer_to_tensor_conversion());
    print_test_result("merge_buffers", test_merge_buffers());
    
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