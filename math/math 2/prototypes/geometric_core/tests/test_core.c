/**
 * @file test_core.c
 * @brief Test suite for geometric core primitives
 * 
 * Tests the foundation: GeometricPoint, GeometricAngle, GeometricScale
 */

#include "../core/point.h"
#include "../core/angle.h"
#include "../core/scale.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>

// ============================================================================
// TEST UTILITIES
// ============================================================================

#define TEST_PASS "\033[32m✓\033[0m"
#define TEST_FAIL "\033[31m✗\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n" name "\n"); \
    for(int i=0; i<60; i++) printf("="); \
    printf("\n");

#define ASSERT_TRUE(condition, message) \
    if (condition) { \
        printf("%s %s\n", TEST_PASS, message); \
        tests_passed++; \
    } else { \
        printf("%s %s\n", TEST_FAIL, message); \
        tests_failed++; \
    }

#define ASSERT_EQ(a, b, message) \
    ASSERT_TRUE((a) == (b), message)

// ============================================================================
// GEOMETRIC POINT TESTS
// ============================================================================

void test_point_creation() {
    TEST("GeometricPoint Creation");
    
    // Test 1: Create from position and magnitude
    GeometricPoint p1 = geo_point_create(6, 3);
    ASSERT_EQ(p1.position, 6, "Position is 6");
    ASSERT_EQ(p1.magnitude, 3, "Magnitude is 3");
    ASSERT_EQ(geo_point_to_int(&p1), 42, "Value is 42 (3×12 + 6)");
    
    // Test 2: Create from integer
    GeometricPoint p2 = geo_point_from_int(42);
    ASSERT_EQ(p2.position, 6, "42 → position 6");
    ASSERT_EQ(p2.magnitude, 3, "42 → magnitude 3");
    
    // Test 3: Zero point
    GeometricPoint zero = geo_point_zero();
    ASSERT_TRUE(geo_point_is_zero(&zero), "Zero point is zero");
    ASSERT_EQ(geo_point_to_int(&zero), 0, "Zero converts to 0");
    
    // Test 4: One point
    GeometricPoint one = geo_point_one();
    ASSERT_TRUE(geo_point_is_one(&one), "One point is one");
    ASSERT_EQ(geo_point_to_int(&one), 1, "One converts to 1");
    
    // Test 5: Negative values
    GeometricPoint neg = geo_point_from_int(-42);
    ASSERT_TRUE(geo_point_is_negative(&neg), "Negative point is negative");
    ASSERT_EQ(geo_point_to_int(&neg), -42, "Negative value preserved");
}

void test_point_properties() {
    TEST("GeometricPoint Properties");
    
    // Test prime positions
    GeometricPoint p1 = geo_point_create(1, 0);
    GeometricPoint p5 = geo_point_create(5, 0);
    GeometricPoint p7 = geo_point_create(7, 0);
    GeometricPoint p11 = geo_point_create(11, 0);
    GeometricPoint p2 = geo_point_create(2, 0);
    
    ASSERT_TRUE(geo_point_is_prime_position(&p1), "Position 1 is prime");
    ASSERT_TRUE(geo_point_is_prime_position(&p5), "Position 5 is prime");
    ASSERT_TRUE(geo_point_is_prime_position(&p7), "Position 7 is prime");
    ASSERT_TRUE(geo_point_is_prime_position(&p11), "Position 11 is prime");
    ASSERT_TRUE(!geo_point_is_prime_position(&p2), "Position 2 is not prime");
    
    // Test control position
    GeometricPoint control = geo_point_create(0, 0);
    ASSERT_TRUE(geo_point_is_control(&control), "Position 0 is control");
    
    // Test angles
    GeometricPoint p6 = geo_point_create(6, 0);
    ASSERT_EQ((int)geo_point_angle(&p6), 180, "Position 6 = 180°");
}

void test_point_comparison() {
    TEST("GeometricPoint Comparison");
    
    GeometricPoint p1 = geo_point_from_int(42);
    GeometricPoint p2 = geo_point_from_int(42);
    GeometricPoint p3 = geo_point_from_int(17);
    
    ASSERT_TRUE(geo_point_equals(&p1, &p2), "42 == 42");
    ASSERT_TRUE(!geo_point_equals(&p1, &p3), "42 != 17");
    ASSERT_TRUE(geo_point_greater_than(&p1, &p3), "42 > 17");
    ASSERT_TRUE(geo_point_less_than(&p3, &p1), "17 < 42");
}

void test_layer_selection() {
    TEST("Layer Selection (O(1))");
    
    GeometricPoint p1 = geo_point_from_int(100);
    GeometricPoint p2 = geo_point_from_int(1000);
    GeometricPoint p3 = geo_point_from_int(1000000);
    GeometricPoint p4 = geo_point_from_int(1000000000);
    
    ASSERT_EQ(p1.layer, 0, "100 → Layer 0");
    ASSERT_EQ(p2.layer, 1, "1,000 → Layer 1");
    ASSERT_EQ(p3.layer, 2, "1,000,000 → Layer 2");
    ASSERT_EQ(p4.layer, 3, "1,000,000,000 → Layer 3");
}

// ============================================================================
// GEOMETRIC ANGLE TESTS
// ============================================================================

void test_angle_creation() {
    TEST("GeometricAngle Creation");
    
    // Test 1: From degrees
    GeometricAngle a1 = geo_angle_from_degrees(90.0);
    ASSERT_EQ((int)a1.degrees, 90, "90° angle");
    ASSERT_EQ(a1.position, 3, "90° = position 3");
    ASSERT_EQ(a1.quadrant, 1, "90° in quadrant 1");
    
    // Test 2: From position
    GeometricAngle a2 = geo_angle_from_position(6);
    ASSERT_EQ((int)a2.degrees, 180, "Position 6 = 180°");
    ASSERT_TRUE(a2.is_exact, "Position-based angle is exact");
    
    // Test 3: From point
    GeometricPoint p = geo_point_create(9, 0);
    GeometricAngle a3 = geo_angle_from_point(&p);
    ASSERT_EQ((int)a3.degrees, 270, "Position 9 = 270°");
}

void test_rotation() {
    TEST("Rotation Operations (Addition/Subtraction)");
    
    // Test addition as rotation
    GeometricPoint p1 = geo_point_from_int(5);   // position 5, magnitude 0
    GeometricPoint p2 = geo_point_from_int(3);   // position 3, magnitude 0
    
    GeometricAngle angle = geo_angle_from_point(&p2);
    GeometricPoint sum = geo_rotate(&p1, &angle);
    
    ASSERT_EQ(geo_point_to_int(&sum), 8, "5 + 3 = 8 (rotation)");
    
    // Test subtraction as reverse rotation
    GeometricPoint diff = geo_rotate_reverse(&sum, &angle);
    ASSERT_EQ(geo_point_to_int(&diff), 5, "8 - 3 = 5 (reverse rotation)");
    
    // Test with carry
    GeometricPoint p3 = geo_point_from_int(10);
    GeometricPoint p4 = geo_point_from_int(5);
    GeometricAngle angle2 = geo_angle_from_point(&p4);
    GeometricPoint sum2 = geo_rotate(&p3, &angle2);
    
    ASSERT_EQ(geo_point_to_int(&sum2), 15, "10 + 5 = 15 (with carry)");
}

// ============================================================================
// GEOMETRIC SCALE TESTS
// ============================================================================

void test_scale_creation() {
    TEST("GeometricScale Creation");
    
    // Test 1: From integer
    GeometricScale s1 = geo_scale_from_int(3);
    ASSERT_EQ(s1.factor, 3, "Factor is 3");
    ASSERT_TRUE(s1.is_exact, "Integer scale is exact");
    
    // Test 2: From fraction
    GeometricScale s2 = geo_scale_from_fraction(1, 2);
    ASSERT_EQ(s2.numerator, 1, "Numerator is 1");
    ASSERT_EQ(s2.denominator, 2, "Denominator is 2");
    ASSERT_TRUE(s2.is_exact, "Fractional scale is exact");
    
    // Test 3: Identity
    GeometricScale identity = geo_scale_identity();
    ASSERT_TRUE(geo_scale_is_identity(&identity), "Identity scale is ×1");
}

void test_scaling() {
    TEST("Scaling Operations (Multiplication/Division)");
    
    // Test multiplication as scaling
    GeometricPoint p1 = geo_point_from_int(5);
    GeometricScale s1 = geo_scale_from_int(3);
    
    GeometricPoint product = geo_scale_up(&p1, &s1);
    ASSERT_EQ(geo_point_to_int(&product), 15, "5 × 3 = 15 (scaling)");
    
    // Test division as inverse scaling
    GeometricPoint quotient = geo_scale_down(&product, &s1);
    ASSERT_EQ(geo_point_to_int(&quotient), 5, "15 ÷ 3 = 5 (inverse scaling)");
    
    // Test with larger numbers
    GeometricPoint p2 = geo_point_from_int(1000);
    GeometricScale s2 = geo_scale_from_int(1000);
    GeometricPoint product2 = geo_scale_up(&p2, &s2);
    
    ASSERT_EQ(geo_point_to_int(&product2), 1000000, "1000 × 1000 = 1,000,000");
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_arithmetic_integration() {
    TEST("Arithmetic Integration (Rotation + Scaling)");
    
    // Test: (5 + 3) × 2 = 16
    GeometricPoint p1 = geo_point_from_int(5);
    GeometricPoint p2 = geo_point_from_int(3);
    
    // Add (rotate)
    GeometricAngle angle = geo_angle_from_point(&p2);
    GeometricPoint sum = geo_rotate(&p1, &angle);
    ASSERT_EQ(geo_point_to_int(&sum), 8, "5 + 3 = 8");
    
    // Multiply (scale)
    GeometricScale scale = geo_scale_from_int(2);
    GeometricPoint product = geo_scale_up(&sum, &scale);
    ASSERT_EQ(geo_point_to_int(&product), 16, "(5 + 3) × 2 = 16");
}

void test_o1_complexity() {
    TEST("O(1) Complexity Verification");
    
    printf("Testing that operations are constant time...\n");
    
    // Small values
    GeometricPoint small1 = geo_point_from_int(10);
    GeometricPoint small2 = geo_point_from_int(20);
    
    // Large values
    GeometricPoint large1 = geo_point_from_int(1000000000);
    GeometricPoint large2 = geo_point_from_int(2000000000);
    
    // Time small addition
    clock_t start = clock();
    for (int i = 0; i < 100000; i++) {
        GeometricAngle angle = geo_angle_from_point(&small2);
        geo_rotate(&small1, &angle);
    }
    clock_t end = clock();
    double time_small = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Time large addition
    start = clock();
    for (int i = 0; i < 100000; i++) {
        GeometricAngle angle = geo_angle_from_point(&large2);
        geo_rotate(&large1, &angle);
    }
    end = clock();
    double time_large = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("  Small values: %.6f seconds\n", time_small);
    printf("  Large values: %.6f seconds\n", time_large);
    
    double ratio = time_large / time_small;
    printf("  Ratio: %.2f\n", ratio);
    
    ASSERT_TRUE(ratio < 2.0 && ratio > 0.5, "O(1): Time independent of magnitude");
}

void test_large_numbers() {
    TEST("Large Number Handling");
    
    // Test with very large numbers
    GeometricPoint big = geo_point_from_int(1000000000000LL);
    ASSERT_EQ(big.layer, 4, "1 trillion → Layer 4");
    
    // Add to it
    GeometricPoint small = geo_point_from_int(1);
    GeometricAngle angle = geo_angle_from_point(&small);
    GeometricPoint result = geo_rotate(&big, &angle);
    
    ASSERT_EQ(geo_point_to_int(&result), 1000000000001LL, "1T + 1 = 1T+1");
    
    // Multiply it
    GeometricScale scale = geo_scale_from_int(2);
    GeometricPoint doubled = geo_scale_up(&big, &scale);
    
    ASSERT_EQ(geo_point_to_int(&doubled), 2000000000000LL, "1T × 2 = 2T");
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("============================================================\n");
    printf("  GEOMETRIC CORE - Test Suite\n");
    printf("  Revolutionary Computational System\n");
    printf("============================================================\n");
    
    test_point_creation();
    test_point_properties();
    test_point_comparison();
    test_layer_selection();
    test_angle_creation();
    test_rotation();
    test_scale_creation();
    test_scaling();
    test_arithmetic_integration();
    test_o1_complexity();
    test_large_numbers();
    
    printf("\n");
    printf("============================================================\n");
    printf("  Test Results\n");
    printf("============================================================\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("  Total:  %d\n", tests_passed + tests_failed);
    printf("============================================================\n");
    
    if (tests_failed == 0) {
        printf("\n%s ALL TESTS PASSED - Geometric core is working!\n\n", TEST_PASS);
        return 0;
    } else {
        printf("\n%s SOME TESTS FAILED\n\n", TEST_FAIL);
        return 1;
    }
}