/**
 * Optimizer Test Suite
 * 
 * Comprehensive tests for CLLM optimizer system.
 */

#include "ai/cllm_optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== TEST: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ PASSED: %s\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ FAILED: %s\n", #name); \
        tests_failed++; \
    }

#define ASSERT_FLOAT_EQ(a, b, epsilon) \
    if (fabs((a) - (b)) > (epsilon)) { \
        printf("ASSERTION FAILED: %f != %f (diff: %f, epsilon: %f)\n", \
               (float)(a), (float)(b), fabs((a) - (b)), (float)(epsilon)); \
        return false; \
    }

// ============================================================================
// TEST CASES
// ============================================================================

bool test_optimizer_create_sgd(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    config.learning_rate = 0.01f;
    
    float params[10] = {0};
    OptimizerState* state = optimizer_create(&config, params, 10);
    
    if (!state) {
        printf("ERROR: Failed to create optimizer\n");
        return false;
    }
    
    printf("Created SGD optimizer\n");
    printf("Learning rate: %f\n", state->current_lr);
    printf("Step: %d\n", state->step);
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_create_adam(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    config.beta1 = 0.9f;
    config.beta2 = 0.999f;
    
    float params[10] = {0};
    OptimizerState* state = optimizer_create(&config, params, 10);
    
    if (!state) {
        printf("ERROR: Failed to create optimizer\n");
        return false;
    }
    
    printf("Created Adam optimizer\n");
    printf("Beta1: %f, Beta2: %f\n", config.beta1, config.beta2);
    
    if (!state->momentum_buffer || !state->variance_buffer) {
        printf("ERROR: Missing optimizer buffers\n");
        optimizer_free(state);
        return false;
    }
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_sgd_step(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD);
    config.learning_rate = 0.1f;
    
    float params[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float gradients[5] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
    
    OptimizerState* state = optimizer_create(&config, params, 5);
    if (!state) return false;
    
    // Perform one step
    int result = optimizer_step(state, gradients);
    if (result != 0) {
        printf("ERROR: optimizer_step failed\n");
        optimizer_free(state);
        return false;
    }
    
    // Check parameters were updated: w = w - lr * g
    // params[0] should be 1.0 - 0.1 * 0.1 = 0.99
    ASSERT_FLOAT_EQ(params[0], 0.99f, 1e-5f);
    ASSERT_FLOAT_EQ(params[1], 1.98f, 1e-5f);
    ASSERT_FLOAT_EQ(params[2], 2.97f, 1e-5f);
    
    printf("Parameters updated correctly\n");
    printf("Step: %d\n", state->step);
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_sgd_momentum(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_SGD_MOMENTUM);
    config.learning_rate = 0.1f;
    config.momentum = 0.9f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    // First step
    optimizer_step(state, gradients);
    
    // Second step with same gradients
    optimizer_step(state, gradients);
    
    // With momentum, the second step should have larger updates
    printf("After 2 steps with momentum:\n");
    printf("params[0] = %f\n", params[0]);
    
    // First step: v = 0.1, w = 1.0 - 0.1 * 0.1 = 0.99
    // Second step: v = 0.9 * 0.1 + 0.1 = 0.19, w = 0.99 - 0.1 * 0.19 = 0.971
    ASSERT_FLOAT_EQ(params[0], 0.971f, 1e-4f);
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_adam_step(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    config.beta1 = 0.9f;
    config.beta2 = 0.999f;
    config.epsilon = 1e-8f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    // Perform one step
    int result = optimizer_step(state, gradients);
    if (result != 0) {
        printf("ERROR: optimizer_step failed\n");
        optimizer_free(state);
        return false;
    }
    
    // Parameters should be updated
    printf("After Adam step:\n");
    printf("params[0] = %f (was 1.0)\n", params[0]);
    printf("params[1] = %f (was 2.0)\n", params[1]);
    
    // Check that parameters changed
    if (params[0] >= 1.0f || params[1] >= 2.0f) {
        printf("ERROR: Parameters not updated\n");
        optimizer_free(state);
        return false;
    }
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_adamw_step(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAMW);
    config.learning_rate = 0.001f;
    config.weight_decay = 0.01f;
    config.decoupled_weight_decay = true;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    // Perform one step
    int result = optimizer_step(state, gradients);
    if (result != 0) {
        printf("ERROR: optimizer_step failed\n");
        optimizer_free(state);
        return false;
    }
    
    printf("AdamW with weight decay applied\n");
    printf("params[0] = %f\n", params[0]);
    
    optimizer_free(state);
    return true;
}

bool test_gradient_clipping_by_value(void) {
    float gradients[5] = {-2.0f, -0.5f, 0.0f, 0.5f, 2.0f};
    
    optimizer_clip_gradients_by_value(gradients, 5, 1.0f);
    
    // Check clipping
    ASSERT_FLOAT_EQ(gradients[0], -1.0f, 1e-5f);
    ASSERT_FLOAT_EQ(gradients[1], -0.5f, 1e-5f);
    ASSERT_FLOAT_EQ(gradients[2], 0.0f, 1e-5f);
    ASSERT_FLOAT_EQ(gradients[3], 0.5f, 1e-5f);
    ASSERT_FLOAT_EQ(gradients[4], 1.0f, 1e-5f);
    
    printf("Gradients clipped by value correctly\n");
    return true;
}

bool test_gradient_clipping_by_norm(void) {
    float gradients[3] = {3.0f, 4.0f, 0.0f};
    
    // Norm = sqrt(9 + 16) = 5.0
    float norm = optimizer_clip_gradients_by_norm(gradients, 3, 2.5f);
    
    printf("Original norm: %f\n", norm);
    ASSERT_FLOAT_EQ(norm, 5.0f, 1e-5f);
    
    // After clipping, norm should be 2.5
    // gradients should be scaled by 2.5/5.0 = 0.5
    ASSERT_FLOAT_EQ(gradients[0], 1.5f, 1e-5f);
    ASSERT_FLOAT_EQ(gradients[1], 2.0f, 1e-5f);
    
    printf("Gradients clipped by norm correctly\n");
    return true;
}

bool test_gradient_norm_computation(void) {
    float gradients[3] = {3.0f, 4.0f, 0.0f};
    
    float norm = optimizer_compute_gradient_norm(gradients, 3);
    
    // Norm = sqrt(9 + 16) = 5.0
    ASSERT_FLOAT_EQ(norm, 5.0f, 1e-5f);
    
    printf("Gradient norm: %f\n", norm);
    return true;
}

bool test_warmup_lr(void) {
    float initial_lr = 0.001f;
    int warmup_steps = 1000;
    
    // At step 0, lr should be 0
    float lr0 = optimizer_warmup_lr(0, warmup_steps, initial_lr);
    ASSERT_FLOAT_EQ(lr0, 0.0f, 1e-6f);
    
    // At step 500, lr should be 0.0005
    float lr500 = optimizer_warmup_lr(500, warmup_steps, initial_lr);
    ASSERT_FLOAT_EQ(lr500, 0.0005f, 1e-6f);
    
    // At step 1000, lr should be 0.001
    float lr1000 = optimizer_warmup_lr(1000, warmup_steps, initial_lr);
    ASSERT_FLOAT_EQ(lr1000, 0.001f, 1e-6f);
    
    printf("Warmup LR: step 0 = %f, step 500 = %f, step 1000 = %f\n",
           lr0, lr500, lr1000);
    
    return true;
}

bool test_linear_decay_lr(void) {
    float initial_lr = 0.001f;
    float min_lr = 0.0001f;
    int total_steps = 1000;
    
    // At step 0, lr should be initial_lr
    float lr0 = optimizer_linear_decay_lr(0, total_steps, initial_lr, min_lr);
    ASSERT_FLOAT_EQ(lr0, 0.001f, 1e-6f);
    
    // At step 500, lr should be halfway
    float lr500 = optimizer_linear_decay_lr(500, total_steps, initial_lr, min_lr);
    ASSERT_FLOAT_EQ(lr500, 0.00055f, 1e-6f);
    
    // At step 1000, lr should be min_lr
    float lr1000 = optimizer_linear_decay_lr(1000, total_steps, initial_lr, min_lr);
    ASSERT_FLOAT_EQ(lr1000, 0.0001f, 1e-6f);
    
    printf("Linear decay LR: step 0 = %f, step 500 = %f, step 1000 = %f\n",
           lr0, lr500, lr1000);
    
    return true;
}

bool test_cosine_annealing_lr(void) {
    float initial_lr = 0.001f;
    float min_lr = 0.0001f;
    int total_steps = 1000;
    
    // At step 0, lr should be initial_lr
    float lr0 = optimizer_cosine_annealing_lr(0, total_steps, initial_lr, min_lr);
    ASSERT_FLOAT_EQ(lr0, 0.001f, 1e-5f);
    
    // At step 1000, lr should be min_lr
    float lr1000 = optimizer_cosine_annealing_lr(1000, total_steps, initial_lr, min_lr);
    ASSERT_FLOAT_EQ(lr1000, 0.0001f, 1e-5f);
    
    printf("Cosine annealing LR: step 0 = %f, step 1000 = %f\n", lr0, lr1000);
    
    return true;
}

bool test_step_decay_lr(void) {
    float initial_lr = 0.001f;
    float min_lr = 0.00001f;
    float decay_rate = 0.1f;
    int decay_steps = 100;
    
    // At step 0, lr should be initial_lr
    float lr0 = optimizer_step_decay_lr(0, decay_steps, initial_lr, decay_rate, min_lr);
    ASSERT_FLOAT_EQ(lr0, 0.001f, 1e-6f);
    
    // At step 100, lr should be initial_lr * 0.1 = 0.0001
    float lr100 = optimizer_step_decay_lr(100, decay_steps, initial_lr, decay_rate, min_lr);
    ASSERT_FLOAT_EQ(lr100, 0.0001f, 1e-6f);
    
    // At step 200, lr should be initial_lr * 0.01 = 0.00001
    float lr200 = optimizer_step_decay_lr(200, decay_steps, initial_lr, decay_rate, min_lr);
    ASSERT_FLOAT_EQ(lr200, 0.00001f, 1e-6f);
    
    printf("Step decay LR: step 0 = %f, step 100 = %f, step 200 = %f\n",
           lr0, lr100, lr200);
    
    return true;
}

bool test_optimizer_with_warmup(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    config.scheduler = LR_SCHEDULER_WARMUP;
    config.warmup_steps = 100;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    // Step 0: LR should be 0
    optimizer_step(state, gradients);
    printf("Step 0 LR: %f\n", state->current_lr);
    
    // Step 50: LR should be 0.0005
    for (int i = 1; i < 50; i++) {
        optimizer_step(state, gradients);
    }
    printf("Step 50 LR: %f\n", state->current_lr);
    ASSERT_FLOAT_EQ(state->current_lr, 0.0005f, 1e-5f);
    
    // Step 100: LR should be 0.001
    for (int i = 50; i < 100; i++) {
        optimizer_step(state, gradients);
    }
    printf("Step 100 LR: %f\n", state->current_lr);
    ASSERT_FLOAT_EQ(state->current_lr, 0.001f, 1e-5f);
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_with_cosine_schedule(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    config.min_lr = 0.0001f;
    config.scheduler = LR_SCHEDULER_COSINE;
    config.total_steps = 1000;
    config.warmup_steps = 100;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    // Warmup phase
    for (int i = 0; i < 100; i++) {
        optimizer_step(state, gradients);
    }
    printf("After warmup (step 100) LR: %f\n", state->current_lr);
    
    // Cosine decay phase
    for (int i = 100; i < 1000; i++) {
        optimizer_step(state, gradients);
    }
    printf("After cosine decay (step 1000) LR: %f\n", state->current_lr);
    
    // Should be close to min_lr
    if (state->current_lr > config.min_lr * 1.1f) {
        printf("ERROR: LR not decayed properly\n");
        optimizer_free(state);
        return false;
    }
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_reset(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    
    // Perform some steps
    for (int i = 0; i < 10; i++) {
        optimizer_step(state, gradients);
    }
    
    printf("Before reset: step = %d\n", state->step);
    
    // Reset
    optimizer_reset(state);
    
    printf("After reset: step = %d\n", state->step);
    
    if (state->step != 0) {
        printf("ERROR: Step not reset\n");
        optimizer_free(state);
        return false;
    }
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_statistics(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    optimizer_step(state, gradients);
    
    float grad_norm, param_norm, lr;
    int step;
    
    optimizer_get_statistics(state, &grad_norm, &param_norm, &lr, &step);
    
    printf("Statistics:\n");
    printf("  Gradient norm: %f\n", grad_norm);
    printf("  Parameter norm: %f\n", param_norm);
    printf("  Learning rate: %f\n", lr);
    printf("  Step: %d\n", step);
    
    if (step != 1) {
        printf("ERROR: Step should be 1\n");
        optimizer_free(state);
        return false;
    }
    
    optimizer_free(state);
    return true;
}

bool test_optimizer_print(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    config.learning_rate = 0.001f;
    
    float params[3] = {1.0f, 2.0f, 3.0f};
    OptimizerState* state = optimizer_create(&config, params, 3);
    if (!state) return false;
    
    optimizer_print_state(state);
    
    optimizer_free(state);
    return true;
}

bool test_config_validation(void) {
    OptimizerConfig config = optimizer_default_config(OPTIMIZER_ADAM);
    
    // Valid config
    if (!optimizer_validate_config(&config)) {
        printf("ERROR: Valid config rejected\n");
        return false;
    }
    
    // Invalid learning rate
    config.learning_rate = -0.001f;
    if (optimizer_validate_config(&config)) {
        printf("ERROR: Invalid learning rate accepted\n");
        return false;
    }
    config.learning_rate = 0.001f;
    
    // Invalid beta1
    config.beta1 = 1.5f;
    if (optimizer_validate_config(&config)) {
        printf("ERROR: Invalid beta1 accepted\n");
        return false;
    }
    config.beta1 = 0.9f;
    
    printf("Config validation working correctly\n");
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM OPTIMIZER TEST SUITE                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    TEST(optimizer_create_sgd);
    TEST(optimizer_create_adam);
    TEST(optimizer_sgd_step);
    TEST(optimizer_sgd_momentum);
    TEST(optimizer_adam_step);
    TEST(optimizer_adamw_step);
    TEST(gradient_clipping_by_value);
    TEST(gradient_clipping_by_norm);
    TEST(gradient_norm_computation);
    TEST(warmup_lr);
    TEST(linear_decay_lr);
    TEST(cosine_annealing_lr);
    TEST(step_decay_lr);
    TEST(optimizer_with_warmup);
    TEST(optimizer_with_cosine_schedule);
    TEST(optimizer_reset);
    TEST(optimizer_statistics);
    TEST(optimizer_print);
    TEST(config_validation);
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                        ║\n", tests_passed + tests_failed);
    printf("║  Passed:       %3d                                        ║\n", tests_passed);
    printf("║  Failed:       %3d                                        ║\n", tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return (tests_failed == 0) ? 0 : 1;
}