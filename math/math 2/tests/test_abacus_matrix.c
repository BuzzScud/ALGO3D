/**
 * Unit tests for CrystallineAbacus matrix utilities
 * 
 * Tests:
 * 1. Matrix creation and destruction
 * 2. Initialization methods (zero, random, Xavier, He)
 * 3. Conversion between double and abacus
 * 4. Matrix operations (add, sub, mul, scale, transpose)
 * 5. Different bases (10, 12, 60)
 * 6. Different precisions (5, 10, 15)
 * 7. Correctness vs double baseline
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "../math/include/math.h"
#include "cllm_abacus_matrix.h"

#define TOLERANCE 1e-6
#define TEST_ROWS 3
#define TEST_COLS 4

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// Helper function to compare doubles with tolerance
static int doubles_equal(double a, double b, double tol) {
    return fabs(a - b) < tol;
}

// Helper function to print test result
static void print_test_result(const char* test_name, int passed) {
    if (passed) {
        printf("✓ %s\n", test_name);
        tests_passed++;
    } else {
        printf("✗ %s\n", test_name);
        tests_failed++;
    }
}

// Test 1: Matrix creation and destruction
static int test_creation_destruction(void) {
    printf("\n=== Test 1: Matrix Creation and Destruction ===\n");
    
    // Test with base 10, precision 5
    AbacusMatrix* mat = abacus_matrix_create(TEST_ROWS, TEST_COLS, 10, 5);
    if (!mat) {
        printf("Failed to create matrix\n");
        return 0;
    }
    
    // Verify dimensions
    if (mat->rows != TEST_ROWS || mat->cols != TEST_COLS) {
        printf("Incorrect dimensions: expected %dx%d, got %dx%d\n",
               TEST_ROWS, TEST_COLS, mat->rows, mat->cols);
        abacus_matrix_free(mat);
        return 0;
    }
    
    // Verify base and precision
    if (mat->base != 10 || mat->precision != 5) {
        printf("Incorrect base/precision: expected 10/5, got %d/%d\n",
               mat->base, mat->precision);
        abacus_matrix_free(mat);
        return 0;
    }
    
    abacus_matrix_free(mat);
    return 1;
}

// Test 2: Zero initialization
static int test_zero_initialization(void) {
    printf("\n=== Test 2: Zero Initialization ===\n");
    
    AbacusMatrix* mat = abacus_matrix_create(TEST_ROWS, TEST_COLS, 10, 5);
    if (!mat) return 0;
    
    abacus_matrix_init_zero(mat);
    
    // Verify all elements are zero
    for (int i = 0; i < TEST_ROWS; i++) {
        for (int j = 0; j < TEST_COLS; j++) {
            double val;
            abacus_matrix_get_double(mat, i, j, &val);
            if (!doubles_equal(val, 0.0, TOLERANCE)) {
                printf("Non-zero element at [%d,%d]: %f\n", i, j, val);
                abacus_matrix_free(mat);
                return 0;
            }
        }
    }
    
    abacus_matrix_free(mat);
    return 1;
}

// Test 3: Conversion between double and abacus
static int test_conversion(void) {
    printf("\n=== Test 3: Double <-> Abacus Conversion ===\n");
    
    // Create test data
    double test_data[TEST_ROWS][TEST_COLS] = {
        {1.5, -2.3, 0.0, 4.7},
        {-1.2, 3.8, -5.1, 0.9},
        {2.4, -0.6, 1.8, -3.3}
    };
    
    AbacusMatrix* mat = abacus_matrix_create(TEST_ROWS, TEST_COLS, 10, 8);
    if (!mat) return 0;
    
    // Convert from double to abacus
    abacus_matrix_from_doubles(mat, (double*)test_data);
    
    // Convert back to double and verify
    for (int i = 0; i < TEST_ROWS; i++) {
        for (int j = 0; j < TEST_COLS; j++) {
            double original = test_data[i][j];
            double converted;
            abacus_matrix_get_double(mat, i, j, &converted);
            
            if (!doubles_equal(original, converted, TOLERANCE)) {
                printf("Conversion error at [%d,%d]: expected %f, got %f\n",
                       i, j, original, converted);
                abacus_matrix_free(mat);
                return 0;
            }
        }
    }
    
    abacus_matrix_free(mat);
    return 1;
}

// Test 4: Matrix addition
static int test_addition(void) {
    printf("\n=== Test 4: Matrix Addition ===\n");
    
    double a_data[TEST_ROWS][TEST_COLS] = {
        {1.0, 2.0, 3.0, 4.0},
        {5.0, 6.0, 7.0, 8.0},
        {9.0, 10.0, 11.0, 12.0}
    };
    
    double b_data[TEST_ROWS][TEST_COLS] = {
        {0.5, 1.5, 2.5, 3.5},
        {4.5, 5.5, 6.5, 7.5},
        {8.5, 9.5, 10.5, 11.5}
    };
    
    AbacusMatrix* a = abacus_matrix_create(TEST_ROWS, TEST_COLS, 10, 5);
    AbacusMatrix* b = abacus_matrix_create(TEST_ROWS, TEST_COLS, 10, 5);
    AbacusMatrix* result = abacus_matrix_create(TEST_ROWS, TEST_COLS, 10, 5);
    
    if (!a || !b || !result) {
        if (a) abacus_matrix_free(a);
        if (b) abacus_matrix_free(b);
        if (result) abacus_matrix_free(result);
        return 0;
    }
    
    abacus_matrix_from_doubles(a, (double*)a_data);
    abacus_matrix_from_doubles(b, (double*)b_data);
    
    abacus_matrix_add(result, a, b);
    
    // Verify result
    for (int i = 0; i < TEST_ROWS; i++) {
        for (int j = 0; j < TEST_COLS; j++) {
            double expected = a_data[i][j] + b_data[i][j];
            double actual;
            abacus_matrix_get_double(result, i, j, &actual);
            
            if (!doubles_equal(expected, actual, TOLERANCE)) {
                printf("Addition error at [%d,%d]: expected %f, got %f\n",
                       i, j, expected, actual);
                abacus_matrix_free(a);
                abacus_matrix_free(b);
                abacus_matrix_free(result);
                return 0;
            }
        }
    }
    
    abacus_matrix_free(a);
    abacus_matrix_free(b);
    abacus_matrix_free(result);
    return 1;
}

// Test 5: Matrix multiplication
static int test_multiplication(void) {
    printf("\n=== Test 5: Matrix Multiplication ===\n");
    
    // A: 2x3, B: 3x2, Result: 2x2
    double a_data[2][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    double b_data[3][2] = {
        {7.0, 8.0},
        {9.0, 10.0},
        {11.0, 12.0}
    };
    
    // Expected result:
    // [1*7 + 2*9 + 3*11, 1*8 + 2*10 + 3*12] = [58, 64]
    // [4*7 + 5*9 + 6*11, 4*8 + 5*10 + 6*12] = [139, 154]
    double expected[2][2] = {
        {58.0, 64.0},
        {139.0, 154.0}
    };
    
    AbacusMatrix* a = abacus_matrix_create(2, 3, 10, 5);
    AbacusMatrix* b = abacus_matrix_create(3, 2, 10, 5);
    AbacusMatrix* result = abacus_matrix_create(2, 2, 10, 5);
    
    if (!a || !b || !result) {
        if (a) abacus_matrix_free(a);
        if (b) abacus_matrix_free(b);
        if (result) abacus_matrix_free(result);
        return 0;
    }
    
    abacus_matrix_from_doubles(a, (double*)a_data);
    abacus_matrix_from_doubles(b, (double*)b_data);
    
    abacus_matrix_mul(result, a, b);
    
    // Verify result
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            double exp = expected[i][j];
            double actual;
            abacus_matrix_get_double(result, i, j, &actual);
            
            if (!doubles_equal(exp, actual, TOLERANCE)) {
                printf("Multiplication error at [%d,%d]: expected %f, got %f\n",
                       i, j, exp, actual);
                abacus_matrix_free(a);
                abacus_matrix_free(b);
                abacus_matrix_free(result);
                return 0;
            }
        }
    }
    
    abacus_matrix_free(a);
    abacus_matrix_free(b);
    abacus_matrix_free(result);
    return 1;
}

// Test 6: Matrix scaling
static int test_scaling(void) {
    printf("\n=== Test 6: Matrix Scaling ===\n");
    
    double data[2][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    double scale_factor = 2.5;
    
    AbacusMatrix* mat = abacus_matrix_create(2, 3, 10, 5);
    AbacusMatrix* result = abacus_matrix_create(2, 3, 10, 5);
    
    if (!mat || !result) {
        if (mat) abacus_matrix_free(mat);
        if (result) abacus_matrix_free(result);
        return 0;
    }
    
    abacus_matrix_from_doubles(mat, (double*)data);
    
    // Create scalar abacus
    CrystallineAbacus* scalar = abacus_from_double(scale_factor, 10, 5);
    
    abacus_matrix_scale(result, mat, scalar);
    
    // Verify result
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            double expected = data[i][j] * scale_factor;
            double actual;
            abacus_matrix_get_double(result, i, j, &actual);
            
            if (!doubles_equal(expected, actual, TOLERANCE)) {
                printf("Scaling error at [%d,%d]: expected %f, got %f\n",
                       i, j, expected, actual);
                abacus_free(scalar);
                abacus_matrix_free(mat);
                abacus_matrix_free(result);
                return 0;
            }
        }
    }
    
    abacus_free(scalar);
    abacus_matrix_free(mat);
    abacus_matrix_free(result);
    return 1;
}

// Test 7: Matrix transpose
static int test_transpose(void) {
    printf("\n=== Test 7: Matrix Transpose ===\n");
    
    double data[2][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    AbacusMatrix* mat = abacus_matrix_create(2, 3, 10, 5);
    AbacusMatrix* result = abacus_matrix_create(3, 2, 10, 5);
    
    if (!mat || !result) {
        if (mat) abacus_matrix_free(mat);
        if (result) abacus_matrix_free(result);
        return 0;
    }
    
    abacus_matrix_from_doubles(mat, (double*)data);
    abacus_matrix_transpose(result, mat);
    
    // Verify result
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            double original = data[i][j];
            double transposed;
            abacus_matrix_get_double(result, j, i, &transposed);
            
            if (!doubles_equal(original, transposed, TOLERANCE)) {
                printf("Transpose error: [%d,%d] = %f, but [%d,%d] = %f\n",
                       i, j, original, j, i, transposed);
                abacus_matrix_free(mat);
                abacus_matrix_free(result);
                return 0;
            }
        }
    }
    
    abacus_matrix_free(mat);
    abacus_matrix_free(result);
    return 1;
}

// Test 8: Different bases (10, 12, 60)
static int test_different_bases(void) {
    printf("\n=== Test 8: Different Bases (10, 12, 60) ===\n");
    
    double test_value = 123.456;
    int bases[] = {10, 12, 60};
    int num_bases = 3;
    
    for (int b = 0; b < num_bases; b++) {
        int base = bases[b];
        
        AbacusMatrix* mat = abacus_matrix_create(1, 1, base, 8);
        if (!mat) {
            printf("Failed to create matrix with base %d\n", base);
            return 0;
        }
        
        abacus_matrix_set_double(mat, 0, 0, test_value);
        double retrieved;
        abacus_matrix_get_double(mat, 0, 0, &retrieved);
        
        if (!doubles_equal(test_value, retrieved, TOLERANCE)) {
            printf("Base %d conversion error: expected %f, got %f\n",
                   base, test_value, retrieved);
            abacus_matrix_free(mat);
            return 0;
        }
        
        printf("  Base %d: ✓\n", base);
        abacus_matrix_free(mat);
    }
    
    return 1;
}

// Test 9: Different precisions (5, 10, 15)
static int test_different_precisions(void) {
    printf("\n=== Test 9: Different Precisions (5, 10, 15) ===\n");
    
    double test_value = 3.141592653589793;
    int precisions[] = {5, 10, 15};
    int num_precisions = 3;
    
    for (int p = 0; p < num_precisions; p++) {
        int precision = precisions[p];
        
        AbacusMatrix* mat = abacus_matrix_create(1, 1, 10, precision);
        if (!mat) {
            printf("Failed to create matrix with precision %d\n", precision);
            return 0;
        }
        
        abacus_matrix_set_double(mat, 0, 0, test_value);
        double retrieved;
        abacus_matrix_get_double(mat, 0, 0, &retrieved);
        
        // Calculate expected tolerance based on precision
        double expected_tol = pow(10.0, -(double)precision);
        
        if (!doubles_equal(test_value, retrieved, expected_tol * 10)) {
            printf("Precision %d error: expected %.*f, got %.*f\n",
                   precision, precision, test_value, precision, retrieved);
            abacus_matrix_free(mat);
            return 0;
        }
        
        printf("  Precision %d: ✓ (error: %.2e)\n", 
               precision, fabs(test_value - retrieved));
        abacus_matrix_free(mat);
    }
    
    return 1;
}

// Test 10: Xavier initialization
static int test_xavier_initialization(void) {
    printf("\n=== Test 10: Xavier Initialization ===\n");
    
    int rows = 100;
    int cols = 100;
    
    AbacusMatrix* mat = abacus_matrix_create(rows, cols, 10, 8);
    if (!mat) return 0;
    
    abacus_matrix_init_xavier(mat, rows, cols);
    
    // Calculate statistics
    double sum = 0.0;
    double sum_sq = 0.0;
    int count = rows * cols;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double val;
            abacus_matrix_get_double(mat, i, j, &val);
            sum += val;
            sum_sq += val * val;
        }
    }
    
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double expected_variance = 2.0 / (rows + cols);
    
    printf("  Mean: %f (expected ~0)\n", mean);
    printf("  Variance: %f (expected ~%f)\n", variance, expected_variance);
    
    // Check if mean is close to 0 and variance is reasonable
    if (fabs(mean) > 0.1 || fabs(variance - expected_variance) > expected_variance) {
        printf("Xavier initialization statistics out of range\n");
        abacus_matrix_free(mat);
        return 0;
    }
    
    abacus_matrix_free(mat);
    return 1;
}

// Test 11: He initialization
static int test_he_initialization(void) {
    printf("\n=== Test 11: He Initialization ===\n");
    
    int rows = 100;
    int cols = 100;
    
    AbacusMatrix* mat = abacus_matrix_create(rows, cols, 10, 8);
    if (!mat) return 0;
    
    abacus_matrix_init_he(mat, rows);
    
    // Calculate statistics
    double sum = 0.0;
    double sum_sq = 0.0;
    int count = rows * cols;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double val;
            abacus_matrix_get_double(mat, i, j, &val);
            sum += val;
            sum_sq += val * val;
        }
    }
    
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double expected_variance = 2.0 / rows;
    
    printf("  Mean: %f (expected ~0)\n", mean);
    printf("  Variance: %f (expected ~%f)\n", variance, expected_variance);
    
    // Check if mean is close to 0 and variance is reasonable
    if (fabs(mean) > 0.1 || fabs(variance - expected_variance) > expected_variance) {
        printf("He initialization statistics out of range\n");
        abacus_matrix_free(mat);
        return 0;
    }
    
    abacus_matrix_free(mat);
    return 1;
}

// Test 12: Hadamard product
static int test_hadamard_product(void) {
    printf("\n=== Test 12: Hadamard Product (Element-wise Multiplication) ===\n");
    
    double a_data[2][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    double b_data[2][3] = {
        {2.0, 3.0, 4.0},
        {5.0, 6.0, 7.0}
    };
    
    AbacusMatrix* a = abacus_matrix_create(2, 3, 10, 5);
    AbacusMatrix* b = abacus_matrix_create(2, 3, 10, 5);
    AbacusMatrix* result = abacus_matrix_create(2, 3, 10, 5);
    
    if (!a || !b || !result) {
        if (a) abacus_matrix_free(a);
        if (b) abacus_matrix_free(b);
        if (result) abacus_matrix_free(result);
        return 0;
    }
    
    abacus_matrix_from_doubles(a, (double*)a_data);
    abacus_matrix_from_doubles(b, (double*)b_data);
    
    abacus_matrix_hadamard(result, a, b);
    
    // Verify result
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            double expected = a_data[i][j] * b_data[i][j];
            double actual;
            abacus_matrix_get_double(result, i, j, &actual);
            
            if (!doubles_equal(expected, actual, TOLERANCE)) {
                printf("Hadamard error at [%d,%d]: expected %f, got %f\n",
                       i, j, expected, actual);
                abacus_matrix_free(a);
                abacus_matrix_free(b);
                abacus_matrix_free(result);
                return 0;
            }
        }
    }
    
    abacus_matrix_free(a);
    abacus_matrix_free(b);
    abacus_matrix_free(result);
    return 1;
}

int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  CrystallineAbacus Matrix Utilities - Unit Test Suite     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Seed random number generator
    srand(time(NULL));
    
    // Run all tests
    print_test_result("Test 1: Creation and Destruction", 
                     test_creation_destruction());
    print_test_result("Test 2: Zero Initialization", 
                     test_zero_initialization());
    print_test_result("Test 3: Double <-> Abacus Conversion", 
                     test_conversion());
    print_test_result("Test 4: Matrix Addition", 
                     test_addition());
    print_test_result("Test 5: Matrix Multiplication", 
                     test_multiplication());
    print_test_result("Test 6: Matrix Scaling", 
                     test_scaling());
    print_test_result("Test 7: Matrix Transpose", 
                     test_transpose());
    print_test_result("Test 8: Different Bases (10, 12, 60)", 
                     test_different_bases());
    print_test_result("Test 9: Different Precisions (5, 10, 15)", 
                     test_different_precisions());
    print_test_result("Test 10: Xavier Initialization", 
                     test_xavier_initialization());
    print_test_result("Test 11: He Initialization", 
                     test_he_initialization());
    print_test_result("Test 12: Hadamard Product", 
                     test_hadamard_product());
    
    // Print summary
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test Summary                                              ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %2d                                          ║\n", 
           tests_passed + tests_failed);
    printf("║  Passed:       %2d                                          ║\n", 
           tests_passed);
    printf("║  Failed:       %2d                                          ║\n", 
           tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    if (tests_failed == 0) {
        printf("\n🎉 All tests passed! 🎉\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed. Please review the output above.\n");
        return 1;
    }
}