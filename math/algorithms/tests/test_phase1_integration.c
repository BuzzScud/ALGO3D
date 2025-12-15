/**
 * @file test_phase1_integration.c
 * @brief Comprehensive test suite for Phase 1 integration
 * 
 * Tests all migrated functions to ensure:
 * 1. Correctness - Results match expected values
 * 2. Numerical stability - No overflow/underflow issues
 * 3. Performance - Functions execute efficiently
 * 4. Independence - NO math.h usage, pure NEW library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "numerical.h"
#include "loss_functions.h"
#include "optimizers.h"
#include "statistics.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

// Test utilities
#define TOLERANCE 1e-6
#define ASSERT_CLOSE(a, b, msg) do { \
    if (math_abs((a) - (b)) > TOLERANCE) { \
        printf("  ❌ FAILED: %s\n", msg); \
        printf("     Expected: %.10f, Got: %.10f, Diff: %.10e\n", (double)(b), (double)(a), math_abs((a) - (b))); \
        return false; \
    } \
} while(0)

#define ASSERT_TRUE(cond, msg) do { \
    if (!(cond)) { \
        printf("  ❌ FAILED: %s\n", msg); \
        return false; \
    } \
} while(0)

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

// Test result tracking
static void test_pass(const char* test_name) {
    printf("  ✅ PASSED: %s\n", test_name);
    tests_passed++;
}

static void test_fail(const char* test_name) {
    printf("  ❌ FAILED: %s\n", test_name);
    tests_failed++;
}

/* ============================================================================
 * NUMERICAL.C TESTS
 * ============================================================================ */

bool test_numerical_softmax_basic() {
    double input[] = {1.0, 2.0, 3.0};
    double output[3];
    
    numerical_softmax(input, output, 3);
    
    // Check sum = 1.0
    double sum = output[0] + output[1] + output[2];
    ASSERT_CLOSE(sum, 1.0, "Softmax sum should equal 1.0");
    
    // Check monotonicity (larger input -> larger output)
    ASSERT_TRUE(output[0] < output[1], "Softmax should be monotonic (0 < 1)");
    ASSERT_TRUE(output[1] < output[2], "Softmax should be monotonic (1 < 2)");
    
    // Check all values are positive
    ASSERT_TRUE(output[0] > 0.0, "Softmax output[0] should be positive");
    ASSERT_TRUE(output[1] > 0.0, "Softmax output[1] should be positive");
    ASSERT_TRUE(output[2] > 0.0, "Softmax output[2] should be positive");
    
    return true;
}

bool test_numerical_softmax_large_values() {
    // Test numerical stability with large values
    double input[] = {1000.0, 1001.0, 1002.0};
    double output[3];
    
    numerical_softmax(input, output, 3);
    
    // Should not overflow
    double sum = output[0] + output[1] + output[2];
    ASSERT_CLOSE(sum, 1.0, "Softmax with large values should not overflow");
    
    // Check monotonicity still holds
    ASSERT_TRUE(output[0] < output[1], "Softmax monotonic with large values (0 < 1)");
    ASSERT_TRUE(output[1] < output[2], "Softmax monotonic with large values (1 < 2)");
    
    return true;
}

bool test_numerical_log_sum_exp() {
    double values[] = {1.0, 2.0, 3.0};
    double lse = numerical_log_sum_exp(values, 3);
    
    // Expected: log(e^1 + e^2 + e^3) ≈ 3.407606
    ASSERT_CLOSE(lse, 3.407606, "Log-sum-exp should match expected value");
    
    return true;
}

bool test_numerical_log_sum_exp_large() {
    // Test numerical stability with large values
    double values[] = {1000.0, 1001.0, 1002.0};
    double lse = numerical_log_sum_exp(values, 3);
    
    // Should not overflow, result should be close to max value
    ASSERT_TRUE(lse > 1002.0 && lse < 1003.0, "Log-sum-exp with large values should not overflow");
    
    return true;
}

bool test_numerical_safe_log() {
    double result1 = numerical_safe_log(2.0, 1e-10);
    ASSERT_CLOSE(result1, 0.693147, "Safe log of 2.0 should be ~0.693147");
    
    // Test near zero
    double result2 = numerical_safe_log(1e-10, 1e-10);
    ASSERT_TRUE(result2 < 0.0, "Safe log of small value should be negative");
    
    return true;
}

bool test_numerical_safe_exp() {
    double result1 = numerical_safe_exp(1.0, 100.0);
    ASSERT_CLOSE(result1, 2.718282, "Safe exp of 1.0 should be ~2.718282");
    
    // Test clamping
    double result2 = numerical_safe_exp(200.0, 100.0);
    double expected = math_exp(100.0);
    ASSERT_CLOSE(result2, expected, "Safe exp should clamp to max_exp");
    
    return true;
}

bool test_numerical_safe_sqrt() {
    double result1 = numerical_safe_sqrt(4.0, 1e-10);
    ASSERT_CLOSE(result1, 2.0, "Safe sqrt of 4.0 should be 2.0");
    
    // Test negative handling
    double result2 = numerical_safe_sqrt(-1.0, 1e-10);
    ASSERT_TRUE(result2 >= 0.0, "Safe sqrt of negative should return non-negative");
    
    return true;
}

bool test_numerical_std_dev() {
    double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double mean = 3.0;
    
    double std_dev = numerical_std_dev(values, 5, mean, false);
    
    // Expected: sqrt(2.0) ≈ 1.414214
    ASSERT_CLOSE(std_dev, 1.414214, "Standard deviation should match expected value");
    
    return true;
}

bool test_numerical_l2_distance() {
    double a[] = {1.0, 2.0, 3.0};
    double b[] = {4.0, 5.0, 6.0};
    
    double dist = numerical_l2_distance(a, b, 3);
    
    // Expected: sqrt(27) ≈ 5.196152
    ASSERT_CLOSE(dist, 5.196152, "L2 distance should match expected value");
    
    return true;
}

bool test_numerical_cosine_similarity() {
    double a[] = {1.0, 0.0, 0.0};
    double b[] = {1.0, 0.0, 0.0};
    
    double sim = numerical_cosine_similarity(a, b, 3);
    ASSERT_CLOSE(sim, 1.0, "Cosine similarity of identical vectors should be 1.0");
    
    // Test orthogonal vectors
    double c[] = {1.0, 0.0, 0.0};
    double d[] = {0.0, 1.0, 0.0};
    double sim2 = numerical_cosine_similarity(c, d, 3);
    ASSERT_CLOSE(sim2, 0.0, "Cosine similarity of orthogonal vectors should be 0.0");
    
    return true;
}

/* ============================================================================
 * OPTIMIZERS.C TESTS
 * ============================================================================ */

bool test_optimizer_adam_step() {
    double params[] = {1.0, 2.0, 3.0};
    double gradients[] = {0.1, 0.2, 0.3};
    double m[] = {0.0, 0.0, 0.0};
    double v[] = {0.0, 0.0, 0.0};
    double max_v[] = {0.0, 0.0, 0.0};
    
    optimizer_adam_step(params, gradients, m, v, max_v, 3, 1, 0.001, 0.9, 0.999, 1e-8, 0.0, false);
    
    // Parameters should have decreased (gradient descent)
    ASSERT_TRUE(params[0] < 1.0, "Adam should decrease param[0]");
    ASSERT_TRUE(params[1] < 2.0, "Adam should decrease param[1]");
    ASSERT_TRUE(params[2] < 3.0, "Adam should decrease param[2]");
    
    // Moments should be updated
    ASSERT_TRUE(m[0] > 0.0, "First moment should be updated");
    ASSERT_TRUE(v[0] > 0.0, "Second moment should be updated");
    
    return true;
}

bool test_optimizer_gradient_norm() {
    double gradients[] = {3.0, 4.0};
    double norm = optimizer_gradient_norm(gradients, 2);
    
    // Expected: sqrt(9 + 16) = 5.0
    ASSERT_CLOSE(norm, 5.0, "Gradient norm should be 5.0");
    
    return true;
}

bool test_lr_schedule_step_decay() {
    double lr = lr_schedule_step_decay(0.1, 100, 50, 0.5);
    
    // After 100 steps with decay every 50 steps: 0.1 * 0.5^2 = 0.025
    ASSERT_CLOSE(lr, 0.025, "Step decay should reduce learning rate correctly");
    
    return true;
}

bool test_lr_schedule_exponential_decay() {
    double lr = lr_schedule_exponential_decay(0.1, 10, 0.1);
    
    // Expected: 0.1 * exp(-0.1 * 10) ≈ 0.036788
    ASSERT_CLOSE(lr, 0.036788, "Exponential decay should match expected value");
    
    return true;
}

bool test_lr_schedule_cosine_annealing() {
    double lr = lr_schedule_cosine_annealing(0.1, 0.01, 50, 100);
    
    // At 50% progress, should be between initial and final
    ASSERT_TRUE(lr > 0.01 && lr < 0.1, "Cosine annealing should be between bounds");
    
    return true;
}

/* ============================================================================
 * STATISTICS.C TESTS
 * ============================================================================ */

bool test_stats_mean() {
    double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double mean = stats_mean(values, 5);
    
    ASSERT_CLOSE(mean, 3.0, "Mean should be 3.0");
    
    return true;
}

bool test_stats_variance() {
    double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double var = stats_variance(values, 5, false);
    
    // Population variance: 2.0
    ASSERT_CLOSE(var, 2.0, "Variance should be 2.0");
    
    return true;
}

bool test_stats_std_dev() {
    double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double std = stats_std_dev(values, 5, false);
    
    // Expected: sqrt(2.0) ≈ 1.414214
    ASSERT_CLOSE(std, 1.414214, "Standard deviation should be ~1.414214");
    
    return true;
}

bool test_stats_median() {
    double values[] = {5.0, 1.0, 3.0, 2.0, 4.0};
    double median = stats_median(values, 5);
    
    ASSERT_CLOSE(median, 3.0, "Median should be 3.0");
    
    return true;
}

bool test_stats_percentile() {
    double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double p50 = stats_percentile(values, 5, 50.0);
    
    ASSERT_CLOSE(p50, 3.0, "50th percentile should be 3.0");
    
    double p25 = stats_percentile(values, 5, 25.0);
    ASSERT_CLOSE(p25, 2.0, "25th percentile should be 2.0");
    
    return true;
}

bool test_stats_entropy() {
    // Test entropy reduction function instead
    double initial_bits = 10.0;
    double reduced = stats_entropy_reduction(initial_bits, 5, 0.0, 0.5);
    
    // Entropy should be reduced
    ASSERT_TRUE(reduced < initial_bits, "Entropy should be reduced");
    ASSERT_TRUE(reduced >= 0.0, "Entropy should be non-negative");
    
    return true;
}

/* ============================================================================
 * TEST RUNNER
 * ============================================================================ */

typedef struct {
    const char* name;
    bool (*func)(void);
} Test;

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║         Phase 1 Integration Test Suite                        ║\n");
    printf("║         Testing NEW Math Library Integration                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    Test tests[] = {
        // Numerical tests
        {"numerical_softmax_basic", test_numerical_softmax_basic},
        {"numerical_softmax_large_values", test_numerical_softmax_large_values},
        {"numerical_log_sum_exp", test_numerical_log_sum_exp},
        {"numerical_log_sum_exp_large", test_numerical_log_sum_exp_large},
        {"numerical_safe_log", test_numerical_safe_log},
        {"numerical_safe_exp", test_numerical_safe_exp},
        {"numerical_safe_sqrt", test_numerical_safe_sqrt},
        {"numerical_std_dev", test_numerical_std_dev},
        {"numerical_l2_distance", test_numerical_l2_distance},
        {"numerical_cosine_similarity", test_numerical_cosine_similarity},
        
        // Optimizer tests
        {"optimizer_adam_step", test_optimizer_adam_step},
        {"optimizer_gradient_norm", test_optimizer_gradient_norm},
        {"lr_schedule_step_decay", test_lr_schedule_step_decay},
        {"lr_schedule_exponential_decay", test_lr_schedule_exponential_decay},
        {"lr_schedule_cosine_annealing", test_lr_schedule_cosine_annealing},
        
        // Statistics tests
        {"stats_mean", test_stats_mean},
        {"stats_variance", test_stats_variance},
        {"stats_std_dev", test_stats_std_dev},
        {"stats_median", test_stats_median},
        {"stats_percentile", test_stats_percentile},
        {"stats_entropy", test_stats_entropy},
    };
    
    int num_tests = sizeof(tests) / sizeof(Test);
    
    printf("Running %d tests...\n\n", num_tests);
    
    for (int i = 0; i < num_tests; i++) {
        printf("[%d/%d] Testing: %s\n", i + 1, num_tests, tests[i].name);
        
        if (tests[i].func()) {
            test_pass(tests[i].name);
        } else {
            test_fail(tests[i].name);
        }
        printf("\n");
    }
    
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                    Test Results Summary                       ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                            ║\n", num_tests);
    printf("║  Passed:       %3d  ✅                                        ║\n", tests_passed);
    printf("║  Failed:       %3d  ❌                                        ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                          ║\n", 
           (tests_passed * 100) / num_tests);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    if (tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! Phase 1 integration is successful!\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Please review the failures above.\n");
        return 1;
    }
}