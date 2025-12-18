/**
 * @file test_abacus_matrix.c
 * @brief Unit tests for AbacusMatrix operations
 * 
 * Tests the CrystallineAbacus-based matrix operations for correctness,
 * comparing results with double-precision baseline.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "../../include/cllm_abacus_matrix.h"

// Test configuration
#define TEST_TOLERANCE 1e-9
#define SMALL_MATRIX_SIZE 3
#define MEDIUM_MATRIX_SIZE 10
#define LARGE_MATRIX_SIZE 50

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Check if two doubles are approximately equal
 */
static bool doubles_equal(double a, double b, double tolerance) {
    double diff = fabs(a - b);
    double max_val = fmax(fabs(a), fabs(b));
    
    if (max_val < tolerance) {
        return diff < tolerance;  // Absolute error for small values
    } else {
        return (diff / max_val) < tolerance;  // Relative error
    }
}

/**
 * @brief Compare two double arrays
 */
static bool arrays_equal(const double* a, const double* b, size_t size, double tolerance) {
    for (size_t i = 0; i < size; i++) {
        if (!doubles_equal(a[i], b[i], tolerance)) {
            printf("    Mismatch at index %zu: %.10f vs %.10f (diff: %.2e)\n", 
                   i, a[i], b[i], fabs(a[i] - b[i]));
            return false;
        }
    }
    return true;
}

/**
 * @brief Print test result
 */
static void print_test_result(const char* test_name, bool passed) {
    if (passed) {
        printf(COLOR_GREEN "  ✓ PASS" COLOR_RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(COLOR_RED "  ✗ FAIL" COLOR_RESET ": %s\n", test_name);
        tests_failed++;
    }
}

/**
 * @brief Matrix multiplication (double baseline)
 */
static void matrix_mul_double(const double* A, const double* B, double* C,
                              uint32_t m, uint32_t n, uint32_t p) {
    for (uint32_t i = 0; i < m; i++) {
        for (uint32_t j = 0; j < p; j++) {
            double sum = 0.0;
            for (uint32_t k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * p + j];
            }
            C[i * p + j] = sum;
        }
    }
}

/**
 * @brief Matrix addition (double baseline)
 */
static void matrix_add_double(const double* A, const double* B, double* C, size_t size) {
    for (size_t i = 0; i < size; i++) {
        C[i] = A[i] + B[i];
    }
}

/**
 * @brief Matrix subtraction (double baseline)
 */
static void matrix_sub_double(const double* A, const double* B, double* C, size_t size) {
    for (size_t i = 0; i < size; i++) {
        C[i] = A[i] - B[i];
    }
}

/* ============================================================================
 * TEST CASES
 * ============================================================================
 */

/**
 * @brief Test 1: Matrix creation and destruction
 */
static bool test_creation_destruction(void) {
    printf("\n" COLOR_BLUE "Test 1: Matrix Creation and Destruction" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    // Test 1.1: Create small matrix
    AbacusMatrix* matrix = abacus_matrix_create(3, 3, 12, 10);
    bool test1 = (matrix != NULL && matrix->rows == 3 && matrix->cols == 3 && 
                  matrix->base == 12 && matrix->precision == 10);
    print_test_result("Create 3x3 matrix", test1);
    all_passed &= test1;
    
    if (matrix) {
        abacus_matrix_free(matrix);
    }
    
    // Test 1.2: Create large matrix
    matrix = abacus_matrix_create(100, 100, 12, 10);
    bool test2 = (matrix != NULL && matrix->rows == 100 && matrix->cols == 100);
    print_test_result("Create 100x100 matrix", test2);
    all_passed &= test2;
    
    if (matrix) {
        abacus_matrix_free(matrix);
    }
    
    // Test 1.3: Create with different bases
    matrix = abacus_matrix_create(5, 5, 10, 10);
    bool test3 = (matrix != NULL && matrix->base == 10);
    print_test_result("Create with base-10", test3);
    all_passed &= test3;
    
    if (matrix) {
        abacus_matrix_free(matrix);
    }
    
    // Test 1.4: Invalid parameters
    matrix = abacus_matrix_create(0, 5, 12, 10);
    bool test4 = (matrix == NULL);
    print_test_result("Reject zero rows", test4);
    all_passed &= test4;
    
    matrix = abacus_matrix_create(5, 0, 12, 10);
    bool test5 = (matrix == NULL);
    print_test_result("Reject zero cols", test5);
    all_passed &= test5;
    
    matrix = abacus_matrix_create(5, 5, 1, 10);
    bool test6 = (matrix == NULL);
    print_test_result("Reject invalid base", test6);
    all_passed &= test6;
    
    return all_passed;
}

/**
 * @brief Test 2: Matrix initialization
 */
static bool test_initialization(void) {
    printf("\n" COLOR_BLUE "Test 2: Matrix Initialization" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    // Test 2.1: Zero initialization
    AbacusMatrix* matrix = abacus_matrix_create(3, 3, 12, 10);
    MathError err = abacus_matrix_init_zero(matrix);
    
    double values[9];
    abacus_matrix_to_doubles(matrix, values);
    
    bool all_zero = true;
    for (int i = 0; i < 9; i++) {
        if (fabs(values[i]) > TEST_TOLERANCE) {
            all_zero = false;
            break;
        }
    }
    
    bool test1 = (err == MATH_SUCCESS && all_zero);
    print_test_result("Zero initialization", test1);
    all_passed &= test1;
    
    // Test 2.2: Random initialization
    err = abacus_matrix_init_random(matrix, 0.1);
    abacus_matrix_to_doubles(matrix, values);
    
    bool has_nonzero = false;
    bool in_range = true;
    for (int i = 0; i < 9; i++) {
        if (fabs(values[i]) > TEST_TOLERANCE) {
            has_nonzero = true;
        }
        if (fabs(values[i]) > 0.1) {
            in_range = false;
        }
    }
    
    bool test2 = (err == MATH_SUCCESS && has_nonzero && in_range);
    print_test_result("Random initialization (scale=0.1)", test2);
    all_passed &= test2;
    
    // Test 2.3: Xavier initialization
    err = abacus_matrix_init_xavier(matrix, 3, 3);
    abacus_matrix_to_doubles(matrix, values);
    
    double expected_scale = sqrt(6.0 / (3 + 3));
    has_nonzero = false;
    in_range = true;
    for (int i = 0; i < 9; i++) {
        if (fabs(values[i]) > TEST_TOLERANCE) {
            has_nonzero = true;
        }
        if (fabs(values[i]) > expected_scale * 1.5) {  // Allow some variance
            in_range = false;
        }
    }
    
    bool test3 = (err == MATH_SUCCESS && has_nonzero && in_range);
    print_test_result("Xavier initialization", test3);
    all_passed &= test3;
    
    // Test 2.4: He initialization
    err = abacus_matrix_init_he(matrix, 3);
    abacus_matrix_to_doubles(matrix, values);
    
    expected_scale = sqrt(2.0 / 3);
    has_nonzero = false;
    in_range = true;
    for (int i = 0; i < 9; i++) {
        if (fabs(values[i]) > TEST_TOLERANCE) {
            has_nonzero = true;
        }
        if (fabs(values[i]) > expected_scale * 1.5) {  // Allow some variance
            in_range = false;
        }
    }
    
    bool test4 = (err == MATH_SUCCESS && has_nonzero && in_range);
    print_test_result("He initialization", test4);
    all_passed &= test4;
    
    abacus_matrix_free(matrix);
    return all_passed;
}

/**
 * @brief Test 3: Conversion between double and abacus
 */
static bool test_conversion(void) {
    printf("\n" COLOR_BLUE "Test 3: Double ↔ Abacus Conversion" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    // Test 3.1: Round-trip conversion (base-12)
    double input[9] = {1.5, -2.3, 3.7, -4.1, 5.9, -6.2, 7.8, -8.4, 9.6};
    double output[9];
    
    AbacusMatrix* matrix = abacus_matrix_create(3, 3, 12, 10);
    abacus_matrix_from_doubles(matrix, input);
    abacus_matrix_to_doubles(matrix, output);
    
    bool test1 = arrays_equal(input, output, 9, TEST_TOLERANCE);
    print_test_result("Round-trip conversion (base-12)", test1);
    all_passed &= test1;
    
    abacus_matrix_free(matrix);
    
    // Test 3.2: Round-trip conversion (base-10)
    matrix = abacus_matrix_create(3, 3, 10, 10);
    abacus_matrix_from_doubles(matrix, input);
    abacus_matrix_to_doubles(matrix, output);
    
    bool test2 = arrays_equal(input, output, 9, TEST_TOLERANCE);
    print_test_result("Round-trip conversion (base-10)", test2);
    all_passed &= test2;
    
    abacus_matrix_free(matrix);
    
    // Test 3.3: Round-trip conversion (base-60)
    matrix = abacus_matrix_create(3, 3, 60, 10);
    abacus_matrix_from_doubles(matrix, input);
    abacus_matrix_to_doubles(matrix, output);
    
    bool test3 = arrays_equal(input, output, 9, TEST_TOLERANCE);
    print_test_result("Round-trip conversion (base-60)", test3);
    all_passed &= test3;
    
    abacus_matrix_free(matrix);
    
    // Test 3.4: Precision test (5 digits)
    matrix = abacus_matrix_create(3, 3, 12, 5);
    abacus_matrix_from_doubles(matrix, input);
    abacus_matrix_to_doubles(matrix, output);
    
    bool test4 = arrays_equal(input, output, 9, 1e-5);  // Lower precision
    print_test_result("Precision test (5 digits)", test4);
    all_passed &= test4;
    
    abacus_matrix_free(matrix);
    
    // Test 3.5: Precision test (15 digits)
    matrix = abacus_matrix_create(3, 3, 12, 15);
    abacus_matrix_from_doubles(matrix, input);
    abacus_matrix_to_doubles(matrix, output);
    
    bool test5 = arrays_equal(input, output, 9, 1e-14);  // Higher precision
    print_test_result("Precision test (15 digits)", test5);
    all_passed &= test5;
    
    abacus_matrix_free(matrix);
    
    return all_passed;
}

/**
 * @brief Test 4: Element access
 */
static bool test_element_access(void) {
    printf("\n" COLOR_BLUE "Test 4: Element Access" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    AbacusMatrix* matrix = abacus_matrix_create(3, 3, 12, 10);
    abacus_matrix_init_zero(matrix);
    
    // Test 4.1: Set and get element
    abacus_matrix_set_double(matrix, 1, 1, 3.14159);
    double value;
    abacus_matrix_get_double(matrix, 1, 1, &value);
    
    bool test1 = doubles_equal(value, 3.14159, TEST_TOLERANCE);
    print_test_result("Set and get element", test1);
    all_passed &= test1;
    
    // Test 4.2: Set all elements
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            abacus_matrix_set_double(matrix, i, j, (double)(i * 3 + j));
        }
    }
    
    bool all_correct = true;
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            abacus_matrix_get_double(matrix, i, j, &value);
            if (!doubles_equal(value, (double)(i * 3 + j), TEST_TOLERANCE)) {
                all_correct = false;
            }
        }
    }
    
    bool test2 = all_correct;
    print_test_result("Set and get all elements", test2);
    all_passed &= test2;
    
    // Test 4.3: Bounds checking
    CrystallineAbacus* elem = abacus_matrix_get(matrix, 5, 5);
    bool test3 = (elem == NULL);
    print_test_result("Bounds checking (out of range)", test3);
    all_passed &= test3;
    
    abacus_matrix_free(matrix);
    return all_passed;
}

/**
 * @brief Test 5: Matrix addition
 */
static bool test_matrix_addition(void) {
    printf("\n" COLOR_BLUE "Test 5: Matrix Addition" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    // Test 5.1: Simple addition
    double A_data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    double B_data[9] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    double expected[9];
    matrix_add_double(A_data, B_data, expected, 9);
    
    AbacusMatrix* A = abacus_matrix_create(3, 3, 12, 10);
    AbacusMatrix* B = abacus_matrix_create(3, 3, 12, 10);
    AbacusMatrix* C = abacus_matrix_create(3, 3, 12, 10);
    
    abacus_matrix_from_doubles(A, A_data);
    abacus_matrix_from_doubles(B, B_data);
    
    MathError err = abacus_matrix_add(C, A, B);
    
    double result[9];
    abacus_matrix_to_doubles(C, result);
    
    bool test1 = (err == MATH_SUCCESS && arrays_equal(result, expected, 9, TEST_TOLERANCE));
    print_test_result("Simple addition", test1);
    all_passed &= test1;
    
    // Test 5.2: Commutativity (A + B = B + A)
    AbacusMatrix* C2 = abacus_matrix_create(3, 3, 12, 10);
    abacus_matrix_add(C2, B, A);
    
    double result2[9];
    abacus_matrix_to_doubles(C2, result2);
    
    bool test2 = arrays_equal(result, result2, 9, TEST_TOLERANCE);
    print_test_result("Commutativity (A + B = B + A)", test2);
    all_passed &= test2;
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    abacus_matrix_free(C);
    abacus_matrix_free(C2);
    
    return all_passed;
}

/**
 * @brief Test 6: Matrix subtraction
 */
static bool test_matrix_subtraction(void) {
    printf("\n" COLOR_BLUE "Test 6: Matrix Subtraction" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    double A_data[9] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
    double B_data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    double expected[9];
    matrix_sub_double(A_data, B_data, expected, 9);
    
    AbacusMatrix* A = abacus_matrix_create(3, 3, 12, 10);
    AbacusMatrix* B = abacus_matrix_create(3, 3, 12, 10);
    AbacusMatrix* C = abacus_matrix_create(3, 3, 12, 10);
    
    abacus_matrix_from_doubles(A, A_data);
    abacus_matrix_from_doubles(B, B_data);
    
    MathError err = abacus_matrix_sub(C, A, B);
    
    double result[9];
    abacus_matrix_to_doubles(C, result);
    
    bool test1 = (err == MATH_SUCCESS && arrays_equal(result, expected, 9, TEST_TOLERANCE));
    print_test_result("Simple subtraction", test1);
    all_passed &= test1;
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    abacus_matrix_free(C);
    
    return all_passed;
}

/**
 * @brief Test 7: Matrix multiplication
 */
static bool test_matrix_multiplication(void) {
    printf("\n" COLOR_BLUE "Test 7: Matrix Multiplication" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    // Test 7.1: Simple 2x2 multiplication
    double A_data[4] = {1, 2, 3, 4};
    double B_data[4] = {5, 6, 7, 8};
    double expected[4];
    matrix_mul_double(A_data, B_data, expected, 2, 2, 2);
    
    AbacusMatrix* A = abacus_matrix_create(2, 2, 12, 10);
    AbacusMatrix* B = abacus_matrix_create(2, 2, 12, 10);
    AbacusMatrix* C = abacus_matrix_create(2, 2, 12, 10);
    
    abacus_matrix_from_doubles(A, A_data);
    abacus_matrix_from_doubles(B, B_data);
    
    MathError err = abacus_matrix_mul(C, A, B);
    
    double result[4];
    abacus_matrix_to_doubles(C, result);
    
    bool test1 = (err == MATH_SUCCESS && arrays_equal(result, expected, 4, TEST_TOLERANCE));
    print_test_result("2x2 multiplication", test1);
    all_passed &= test1;
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    abacus_matrix_free(C);
    
    // Test 7.2: 3x3 multiplication
    double A3_data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    double B3_data[9] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    double expected3[9];
    matrix_mul_double(A3_data, B3_data, expected3, 3, 3, 3);
    
    A = abacus_matrix_create(3, 3, 12, 10);
    B = abacus_matrix_create(3, 3, 12, 10);
    C = abacus_matrix_create(3, 3, 12, 10);
    
    abacus_matrix_from_doubles(A, A3_data);
    abacus_matrix_from_doubles(B, B3_data);
    
    err = abacus_matrix_mul(C, A, B);
    
    double result3[9];
    abacus_matrix_to_doubles(C, result3);
    
    bool test2 = (err == MATH_SUCCESS && arrays_equal(result3, expected3, 9, TEST_TOLERANCE));
    print_test_result("3x3 multiplication", test2);
    all_passed &= test2;
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    abacus_matrix_free(C);
    
    // Test 7.3: Non-square multiplication (2x3 × 3x2)
    double A_rect[6] = {1, 2, 3, 4, 5, 6};
    double B_rect[6] = {7, 8, 9, 10, 11, 12};
    double expected_rect[4];
    matrix_mul_double(A_rect, B_rect, expected_rect, 2, 3, 2);
    
    A = abacus_matrix_create(2, 3, 12, 10);
    B = abacus_matrix_create(3, 2, 12, 10);
    C = abacus_matrix_create(2, 2, 12, 10);
    
    abacus_matrix_from_doubles(A, A_rect);
    abacus_matrix_from_doubles(B, B_rect);
    
    err = abacus_matrix_mul(C, A, B);
    
    double result_rect[4];
    abacus_matrix_to_doubles(C, result_rect);
    
    bool test3 = (err == MATH_SUCCESS && arrays_equal(result_rect, expected_rect, 4, TEST_TOLERANCE));
    print_test_result("Non-square multiplication (2x3 × 3x2)", test3);
    all_passed &= test3;
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    abacus_matrix_free(C);
    
    return all_passed;
}

/**
 * @brief Test 8: Scalar multiplication
 */
static bool test_scalar_multiplication(void) {
    printf("\n" COLOR_BLUE "Test 8: Scalar Multiplication" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    double A_data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    double scalar = 2.5;
    double expected[9];
    for (int i = 0; i < 9; i++) {
        expected[i] = A_data[i] * scalar;
    }
    
    AbacusMatrix* A = abacus_matrix_create(3, 3, 12, 10);
    AbacusMatrix* C = abacus_matrix_create(3, 3, 12, 10);
    
    abacus_matrix_from_doubles(A, A_data);
    
    MathError err = abacus_matrix_scale_double(C, A, scalar);
    
    double result[9];
    abacus_matrix_to_doubles(C, result);
    
    bool test1 = (err == MATH_SUCCESS && arrays_equal(result, expected, 9, TEST_TOLERANCE));
    print_test_result("Scalar multiplication (2.5)", test1);
    all_passed &= test1;
    
    abacus_matrix_free(A);
    abacus_matrix_free(C);
    
    return all_passed;
}

/**
 * @brief Test 9: Utility functions
 */
static bool test_utilities(void) {
    printf("\n" COLOR_BLUE "Test 9: Utility Functions" COLOR_RESET "\n");
    
    bool all_passed = true;
    
    double A_data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    AbacusMatrix* A = abacus_matrix_create(3, 3, 12, 10);
    abacus_matrix_from_doubles(A, A_data);
    
    // Test 9.1: Copy
    AbacusMatrix* B = abacus_matrix_copy(A);
    
    double B_data[9];
    abacus_matrix_to_doubles(B, B_data);
    
    bool test1 = (B != NULL && arrays_equal(A_data, B_data, 9, TEST_TOLERANCE));
    print_test_result("Matrix copy", test1);
    all_passed &= test1;
    
    // Test 9.2: Sum
    double sum;
    MathError err = abacus_matrix_sum_double(A, &sum);
    double expected_sum = 0;
    for (int i = 0; i < 9; i++) {
        expected_sum += A_data[i];
    }
    
    bool test2 = (err == MATH_SUCCESS && doubles_equal(sum, expected_sum, TEST_TOLERANCE));
    print_test_result("Matrix sum", test2);
    all_passed &= test2;
    
    // Test 9.3: Validation
    bool test3 = abacus_matrix_is_valid(A);
    print_test_result("Matrix validation (valid)", test3);
    all_passed &= test3;
    
    // Test 9.4: Memory usage
    size_t memory = abacus_matrix_memory_usage(A);
    bool test4 = (memory > 0);
    print_test_result("Memory usage calculation", test4);
    all_passed &= test4;
    
    if (test4) {
        printf("    Memory usage: %zu bytes\n", memory);
    }
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    
    return all_passed;
}

/**
 * @brief Test 10: Performance benchmark
 */
static bool test_performance(void) {
    printf("\n" COLOR_BLUE "Test 10: Performance Benchmark" COLOR_RESET "\n");
    
    // This is not a pass/fail test, just informational
    printf("  " COLOR_YELLOW "Note: Performance tests are informational only" COLOR_RESET "\n");
    
    // Small matrix (10x10)
    AbacusMatrix* A = abacus_matrix_create(10, 10, 12, 10);
    AbacusMatrix* B = abacus_matrix_create(10, 10, 12, 10);
    AbacusMatrix* C = abacus_matrix_create(10, 10, 12, 10);
    
    abacus_matrix_init_random(A, 1.0);
    abacus_matrix_init_random(B, 1.0);
    
    // Measure multiplication time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    abacus_matrix_mul(C, A, B);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("  10x10 matrix multiplication: %.6f seconds\n", elapsed);
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    abacus_matrix_free(C);
    
    // Medium matrix (50x50)
    A = abacus_matrix_create(50, 50, 12, 10);
    B = abacus_matrix_create(50, 50, 12, 10);
    C = abacus_matrix_create(50, 50, 12, 10);
    
    abacus_matrix_init_random(A, 1.0);
    abacus_matrix_init_random(B, 1.0);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    abacus_matrix_mul(C, A, B);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    elapsed = (end.tv_sec - start.tv_sec) + 
              (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("  50x50 matrix multiplication: %.6f seconds\n", elapsed);
    
    abacus_matrix_free(A);
    abacus_matrix_free(B);
    abacus_matrix_free(C);
    
    return true;  // Always pass
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================
 */

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  ABACUS MATRIX UNIT TESTS\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    // Run all tests
    test_creation_destruction();
    test_initialization();
    test_conversion();
    test_element_access();
    test_matrix_addition();
    test_matrix_subtraction();
    test_matrix_multiplication();
    test_scalar_multiplication();
    test_utilities();
    test_performance();
    
    // Print summary
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  TEST SUMMARY\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  Total tests: %d\n", tests_passed + tests_failed);
    printf("  " COLOR_GREEN "Passed: %d" COLOR_RESET "\n", tests_passed);
    printf("  " COLOR_RED "Failed: %d" COLOR_RESET "\n", tests_failed);
    printf("═══════════════════════════════════════════════════════════════\n");
    
    if (tests_failed == 0) {
        printf("\n" COLOR_GREEN "✓ ALL TESTS PASSED!" COLOR_RESET "\n\n");
        return 0;
    } else {
        printf("\n" COLOR_RED "✗ SOME TESTS FAILED" COLOR_RESET "\n\n");
        return 1;
    }
}