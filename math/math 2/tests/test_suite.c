/**
 * Comprehensive Test Suite for Crystalline CLLM
 * Focus on testing the prime math library thoroughly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Crystalline headers
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "bigint_core.h"

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            tests_passed++; \
            printf("  ✓ %s\n", message); \
        } else { \
            tests_failed++; \
            printf("  ✗ FAILED: %s\n", message); \
        } \
    } while(0)

#define TEST_SECTION(name) \
    printf("\n=== %s ===\n", name)

#define TOLERANCE 0.01f

// ============================================================================
// CRYSTALLINE LATTICE MATH LIBRARY TESTS
// ============================================================================

void test_prime_sqrt() {
    TEST_SECTION("Prime Square Root Tests");
    
    // Test basic square roots
    float result = math_sqrt(4.0f);
    TEST_ASSERT(result > 1.99f && result < 2.01f, "sqrt(4) ≈ 2");
    
    result = math_sqrt(9.0f);
    TEST_ASSERT(result > 2.99f && result < 3.01f, "sqrt(9) ≈ 3");
    
    result = math_sqrt(16.0f);
    TEST_ASSERT(result > 3.99f && result < 4.01f, "sqrt(16) ≈ 4");
    
    result = math_sqrt(25.0f);
    TEST_ASSERT(result > 4.99f && result < 5.01f, "sqrt(25) ≈ 5");
    
    result = math_sqrt(2.0f);
    TEST_ASSERT(result > 1.41f && result < 1.42f, "sqrt(2) ≈ 1.414");
    
    // Test edge cases
    result = math_sqrt(0.0f);
    TEST_ASSERT(result == 0.0f, "sqrt(0) = 0");
    
    result = math_sqrt(1.0f);
    TEST_ASSERT(result > 0.99f && result < 1.01f, "sqrt(1) = 1");
    
    // Test small values
    result = math_sqrt(0.25f);
    TEST_ASSERT(result > 0.49f && result < 0.51f, "sqrt(0.25) = 0.5");
    
    // Test large values
    result = math_sqrt(100.0f);
    TEST_ASSERT(result > 9.99f && result < 10.01f, "sqrt(100) = 10");
}

void test_prime_exp() {
    TEST_SECTION("Prime Exponential Tests");
    
    float result = math_exp(0.0f);
    TEST_ASSERT(result > 0.99f && result < 1.01f, "exp(0) = 1");
    
    result = math_exp(1.0f);
    TEST_ASSERT(result > 2.71f && result < 2.72f, "exp(1) ≈ e ≈ 2.718");
    
    result = math_exp(2.0f);
    TEST_ASSERT(result > 7.38f && result < 7.40f, "exp(2) ≈ 7.389");
    
    result = math_exp(-1.0f);
    TEST_ASSERT(result > 0.36f && result < 0.37f, "exp(-1) ≈ 0.368");
    
    result = math_exp(-2.0f);
    TEST_ASSERT(result > 0.13f && result < 0.14f, "exp(-2) ≈ 0.135");
    
    // Test small values
    result = math_exp(0.5f);
    TEST_ASSERT(result > 1.64f && result < 1.65f, "exp(0.5) ≈ 1.649");
}

void test_prime_log() {
    TEST_SECTION("Prime Logarithm Tests");
    
    float result = math_log(1.0f);
    TEST_ASSERT(result > -0.01f && result < 0.01f, "log(1) = 0");
    
    result = math_log(2.718281828f);
    TEST_ASSERT(result > 0.99f && result < 1.01f, "log(e) ≈ 1");
    
    result = math_log(10.0f);
    TEST_ASSERT(result > 2.30f && result < 2.31f, "log(10) ≈ 2.303");
    
    result = math_log(2.0f);
    TEST_ASSERT(result > 0.69f && result < 0.70f, "log(2) ≈ 0.693");
    
    result = math_log(100.0f);
    TEST_ASSERT(result > 4.60f && result < 4.61f, "log(100) ≈ 4.605");
}

void test_prime_pow() {
    TEST_SECTION("Prime Power Tests");
    
    float result = math_pow(2.0f, 3.0f);
    TEST_ASSERT(result > 7.99f && result < 8.01f, "pow(2, 3) = 8");
    
    result = math_pow(3.0f, 2.0f);
    TEST_ASSERT(result > 8.99f && result < 9.01f, "pow(3, 2) = 9");
    
    result = math_pow(10.0f, 2.0f);
    TEST_ASSERT(result > 99.9f && result < 100.1f, "pow(10, 2) = 100");
    
    result = math_pow(2.0f, 0.0f);
    TEST_ASSERT(result > 0.99f && result < 1.01f, "pow(2, 0) = 1");
    
    result = math_pow(5.0f, 1.0f);
    TEST_ASSERT(result > 4.99f && result < 5.01f, "pow(5, 1) = 5");
}

void test_prime_trig() {
    TEST_SECTION("Prime Trigonometric Tests");
    
    float result = math_sin(0.0f);
    TEST_ASSERT(result > -0.01f && result < 0.01f, "sin(0) = 0");
    
    result = math_cos(0.0f);
    TEST_ASSERT(result > 0.99f && result < 1.01f, "cos(0) = 1");
    
    result = math_sin(M_PI / 2.0f);
    TEST_ASSERT(result > 0.99f && result < 1.01f, "sin(π/2) = 1");
    
    result = math_cos(M_PI);
    TEST_ASSERT(result > -1.01f && result < -0.99f, "cos(π) = -1");
    
    result = math_sin(M_PI);
    TEST_ASSERT(result > -0.01f && result < 0.01f, "sin(π) ≈ 0");
    
    result = math_cos(M_PI / 2.0f);
    TEST_ASSERT(result > -0.01f && result < 0.01f, "cos(π/2) ≈ 0");
    
    result = math_tanh(0.0f);
    TEST_ASSERT(result > -0.01f && result < 0.01f, "tanh(0) = 0");
    
    result = math_tanh(1.0f);
    TEST_ASSERT(result > 0.76f && result < 0.77f, "tanh(1) ≈ 0.762");
}

void test_prime_special_values() {
    TEST_SECTION("Prime Special Values Tests");
    
    // Test NaN detection
    float nan_val = 0.0f / 0.0f;
    TEST_ASSERT(math_is_nan(nan_val), "isnan(NAN) = true");
    TEST_ASSERT(!math_is_nan(1.0f), "isnan(1.0) = false");
    TEST_ASSERT(!math_is_nan(0.0f), "isnan(0.0) = false");
    
    // Test infinity detection
    float inf_val = 1.0f / 0.0f;
    TEST_ASSERT(math_is_inf(inf_val), "isinf(INFINITY) = true");
    TEST_ASSERT(!math_is_inf(1.0f), "isinf(1.0) = false");
    TEST_ASSERT(!math_is_inf(0.0f), "isinf(0.0) = false");
    
    // Test absolute value
    float result = math_abs(-5.5f);
    TEST_ASSERT(result > 5.49f && result < 5.51f, "fabs(-5.5) = 5.5");
    
    result = math_abs(5.5f);
    TEST_ASSERT(result > 5.49f && result < 5.51f, "fabs(5.5) = 5.5");
    
    result = math_abs(0.0f);
    TEST_ASSERT(result == 0.0f, "fabs(0) = 0");
    
    result = math_abs(-0.0f);
    TEST_ASSERT(result == 0.0f, "fabs(-0) = 0");
}

void test_prime_floor_ceil() {
    TEST_SECTION("Prime Floor/Ceil Tests");
    
    float result = math_floor(3.7f);
    TEST_ASSERT(result == 3.0f, "floor(3.7) = 3");
    
    result = math_floor(-3.7f);
    TEST_ASSERT(result == -4.0f, "floor(-3.7) = -4");
    
    result = math_ceil(3.2f);
    TEST_ASSERT(result == 4.0f, "ceil(3.2) = 4");
    
    result = math_ceil(-3.2f);
    TEST_ASSERT(result == -3.0f, "ceil(-3.2) = -3");
    
    result = math_floor(5.0f);
    TEST_ASSERT(result == 5.0f, "floor(5.0) = 5");
    
    result = math_ceil(5.0f);
    TEST_ASSERT(result == 5.0f, "ceil(5.0) = 5");
}

void test_bigint_operations() {
    TEST_SECTION("BigInt Operations Tests");
    
    BigInt a, b, result;
    big_init(&a);
    big_init(&b);
    big_init(&result);
    
    // Test addition
    big_from_int(&a, 12345);
    big_from_int(&b, 67890);
    big_add(&result, &a, &b);
    
    char* str_result = big_to_string(&result);
    TEST_ASSERT(strcmp(str_result, "80235") == 0, "BigInt addition: 12345 + 67890 = 80235");
    free(str_result);
    
    // Test subtraction
    big_from_int(&a, 100000);
    big_from_int(&b, 12345);
    big_sub(&a, &b, &result);
    str_result = big_to_string(&result);
    TEST_ASSERT(strcmp(str_result, "87655") == 0, "BigInt subtraction: 100000 - 12345 = 87655");
    free(str_result);
    
    // Test multiplication
    big_from_int(&a, 123);
    big_from_int(&b, 456);
    big_mul(&a, &b, &result);
    str_result = big_to_string(&result);
    TEST_ASSERT(strcmp(str_result, "56088") == 0, "BigInt multiplication: 123 * 456 = 56088");
    free(str_result);
    
    // Test comparison
    big_from_int(&a, 100);
    big_from_int(&b, 200);
    TEST_ASSERT(big_cmp(&a, &b) < 0, "BigInt comparison: 100 < 200");
    TEST_ASSERT(big_cmp(&b, &a) > 0, "BigInt comparison: 200 > 100");
    TEST_ASSERT(big_cmp(&a, &a) == 0, "BigInt comparison: 100 == 100");
    
    big_free(&a);
    big_free(&b);
    big_free(&result);
}

void test_prime_math_accuracy() {
    TEST_SECTION("Prime Math Accuracy Tests");
    
    // Test that exp(log(x)) ≈ x
    float x = 5.0f;
    float result = math_exp(math_log(x));
    TEST_ASSERT(result > 4.99f && result < 5.01f, "exp(log(5)) ≈ 5");
    
    // Test that log(exp(x)) ≈ x
    x = 2.0f;
    result = math_log(math_exp(x));
    TEST_ASSERT(result > 1.99f && result < 2.01f, "log(exp(2)) ≈ 2");
    
    // Test that sin²(x) + cos²(x) = 1
    x = 0.5f;
    float sin_val = math_sin(x);
    float cos_val = math_cos(x);
    result = sin_val * sin_val + cos_val * cos_val;
    TEST_ASSERT(result > 0.99f && result < 1.01f, "sin²(x) + cos²(x) = 1");
    
    // Test that sqrt(x²) = |x|
    x = -7.0f;
    result = math_sqrt(x * x);
    TEST_ASSERT(result > 6.99f && result < 7.01f, "sqrt((-7)²) = 7");
}

void test_edge_cases() {
    TEST_SECTION("Edge Cases and Boundary Tests");
    
    // Test very small values
    float result = math_sqrt(0.0001f);
    TEST_ASSERT(result > 0.009f && result < 0.011f, "sqrt(0.0001) = 0.01");
    
    // Test values near 1
    result = math_log(1.001f);
    TEST_ASSERT(result > 0.0f && result < 0.002f, "log(1.001) ≈ 0.001");
    
    // Test negative inputs where appropriate
    result = math_abs(-1000.0f);
    TEST_ASSERT(result == 1000.0f, "fabs(-1000) = 1000");
    
    // Test zero
    result = math_exp(0.0f);
    TEST_ASSERT(result == 1.0f, "exp(0) = 1");
    
    result = math_sin(0.0f);
    TEST_ASSERT(result == 0.0f, "sin(0) = 0");
}

void test_performance() {
    TEST_SECTION("Performance Tests");
    
    clock_t start, end;
    double time_taken;
    int iterations = 100000;
    
    // Test sqrt performance
    start = clock();
    for (int i = 0; i < iterations; i++) {
        math_sqrt((float)(i % 1000 + 1));
    }
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  sqrt: %d iterations in %.3f seconds (%.2f ops/sec)\n", 
           iterations, time_taken, iterations / time_taken);
    TEST_ASSERT(time_taken < 5.0, "sqrt performance acceptable");
    
    // Test exp performance
    start = clock();
    for (int i = 0; i < iterations; i++) {
        math_exp((float)(i % 10));
    }
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  exp: %d iterations in %.3f seconds (%.2f ops/sec)\n", 
           iterations, time_taken, iterations / time_taken);
    TEST_ASSERT(time_taken < 5.0, "exp performance acceptable");
    
    // Test sin performance
    start = clock();
    for (int i = 0; i < iterations; i++) {
        math_sin((float)(i % 100) / 10.0f);
    }
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  sin: %d iterations in %.3f seconds (%.2f ops/sec)\n", 
           iterations, time_taken, iterations / time_taken);
    TEST_ASSERT(time_taken < 5.0, "sin performance acceptable");
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     CRYSTALLINE MATH LIBRARY COMPREHENSIVE TEST SUITE      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Crystalline Math Library Tests
    printf("\n┌─────────────────────────────────────────────────────────┐\n");
    printf("│  CRYSTALLINE LATTICE MATH LIBRARY TESTS                │\n");
    printf("└─────────────────────────────────────────────────────────┘\n");
    
    test_prime_sqrt();
    test_prime_exp();
    test_prime_log();
    test_prime_pow();
    test_prime_trig();
    test_prime_special_values();
    test_prime_floor_ceil();
    test_bigint_operations();
    test_prime_math_accuracy();
    test_edge_cases();
    test_performance();
    
    // Final Results
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST RESULTS                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-4d                                        ║\n", tests_passed);
    printf("║  Tests Failed: %-4d                                        ║\n", tests_failed);
    printf("║  Total Tests:  %-4d                                        ║\n", tests_passed + tests_failed);
    if (tests_passed + tests_failed > 0) {
        printf("║  Success Rate: %.1f%%                                      ║\n", 
               (float)tests_passed / (tests_passed + tests_failed) * 100.0f);
    }
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return tests_failed > 0 ? 1 : 0;
}