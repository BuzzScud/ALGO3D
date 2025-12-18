/**
 * Comprehensive Test Suite for Sphere Packing Geometry
 * 
 * Tests geometric operations for sphere packing and kissing spheres.
 */

#include "../include/sphere_packing.h"
#include <stdio.h>
#include <stdlib.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

// Test macros
#define TEST_START(name) \
    printf(COLOR_BLUE "TEST: %s" COLOR_RESET "\n", name)

#define TEST_PASS(name) \
    do { \
        printf(COLOR_GREEN "✓ PASS: %s" COLOR_RESET "\n", name); \
        tests_passed++; \
    } while(0)

#define TEST_FAIL(name, reason) \
    do { \
        printf(COLOR_RED "✗ FAIL: %s - %s" COLOR_RESET "\n", name, reason); \
        tests_failed++; \
    } while(0)

#define ASSERT_TRUE(condition, message) \
    do { \
        if (!(condition)) { \
            TEST_FAIL(current_test, message); \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(condition, message) \
    ASSERT_TRUE(!(condition), message)

#define ASSERT_NEAR(a, b, epsilon, message) \
    ASSERT_TRUE(math_abs((a) - (b)) < (epsilon), message)

// Helper function
static double float_abs(double x) {
    return x < 0 ? -x : x;
}

//=============================================================================
// Test 1: Kissing Spheres Detection
//=============================================================================
void test_kissing_spheres(void) {
    const char* current_test = "Kissing Spheres Detection";
    TEST_START(current_test);
    
    // Create two spheres that are kissing
    Sphere3D s1 = {0.0, 0.0, 0.0, 1.0, 1};
    Sphere3D s2 = {2.0, 0.0, 0.0, 1.0, 2}; // Distance = 2.0, sum of radii = 2.0
    
    ASSERT_TRUE(spheres_are_kissing(&s1, &s2, 0.001), "Spheres should be kissing");
    
    // Create two spheres that are not kissing (separated)
    Sphere3D s3 = {3.0, 0.0, 0.0, 1.0, 3}; // Distance = 3.0, sum of radii = 2.0
    ASSERT_FALSE(spheres_are_kissing(&s1, &s3, 0.001), "Spheres should not be kissing");
    
    // Create two spheres that overlap
    Sphere3D s4 = {1.5, 0.0, 0.0, 1.0, 4}; // Distance = 1.5, sum of radii = 2.0
    ASSERT_FALSE(spheres_are_kissing(&s1, &s4, 0.001), "Overlapping spheres should not be kissing");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 2: Sphere Gap Calculation
//=============================================================================
void test_sphere_gap(void) {
    const char* current_test = "Sphere Gap Calculation";
    TEST_START(current_test);
    
    Sphere3D s1 = {0.0, 0.0, 0.0, 1.0, 1};
    
    // Kissing spheres (gap = 0)
    Sphere3D s2 = {2.0, 0.0, 0.0, 1.0, 2};
    double gap1 = sphere_gap(&s1, &s2);
    ASSERT_NEAR(gap1, 0.0, 0.001, "Gap should be 0 for kissing spheres");
    
    // Separated spheres (gap > 0)
    Sphere3D s3 = {3.0, 0.0, 0.0, 1.0, 3};
    double gap2 = sphere_gap(&s1, &s3);
    ASSERT_NEAR(gap2, 1.0, 0.001, "Gap should be 1.0");
    
    // Overlapping spheres (gap < 0)
    Sphere3D s4 = {1.5, 0.0, 0.0, 1.0, 4};
    double gap3 = sphere_gap(&s1, &s4);
    ASSERT_TRUE(gap3 < 0, "Gap should be negative for overlapping spheres");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 3: Sphere Distance
//=============================================================================
void test_sphere_distance(void) {
    const char* current_test = "Sphere Distance";
    TEST_START(current_test);
    
    Sphere3D s1 = {0.0, 0.0, 0.0, 1.0, 1};
    Sphere3D s2 = {3.0, 4.0, 0.0, 1.0, 2}; // Distance = 5.0 (3-4-5 triangle)
    
    double distance = sphere_distance(&s1, &s2);
    ASSERT_NEAR(distance, 5.0, 0.001, "Distance should be 5.0");
    
    // Test 3D distance
    Sphere3D s3 = {1.0, 1.0, 1.0, 1.0, 3};
    double distance2 = sphere_distance(&s1, &s3);
    double expected = math_sqrt(3.0); // math_sqrt(1² + 1² + 1²)
    ASSERT_NEAR(distance2, expected, 0.001, "3D distance incorrect");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 4: Sphere Overlap Detection
//=============================================================================
void test_sphere_overlap(void) {
    const char* current_test = "Sphere Overlap Detection";
    TEST_START(current_test);
    
    Sphere3D s1 = {0.0, 0.0, 0.0, 1.0, 1};
    
    // Non-overlapping spheres
    Sphere3D s2 = {3.0, 0.0, 0.0, 1.0, 2};
    ASSERT_FALSE(spheres_overlap(&s1, &s2), "Spheres should not overlap");
    
    // Kissing spheres (not overlapping)
    Sphere3D s3 = {2.0, 0.0, 0.0, 1.0, 3};
    ASSERT_FALSE(spheres_overlap(&s1, &s3), "Kissing spheres should not overlap");
    
    // Overlapping spheres
    Sphere3D s4 = {1.5, 0.0, 0.0, 1.0, 4};
    ASSERT_TRUE(spheres_overlap(&s1, &s4), "Spheres should overlap");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 5: Sphere Volume and Surface Area
//=============================================================================
void test_sphere_volume_area(void) {
    const char* current_test = "Sphere Volume and Surface Area";
    TEST_START(current_test);
    
    Sphere3D sphere = {0.0, 0.0, 0.0, 1.0, 1};
    
    // Volume = 4/3 * π * r³ = 4/3 * π ≈ 4.189
    double volume = sphere_volume(&sphere);
    ASSERT_NEAR(volume, 4.189, 0.01, "Volume incorrect");
    
    // Surface area = 4 * π * r² = 4 * π ≈ 12.566
    double area = sphere_surface_area(&sphere);
    ASSERT_NEAR(area, 12.566, 0.01, "Surface area incorrect");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 6: 2D Vector Operations
//=============================================================================
void test_vector2d_operations(void) {
    const char* current_test = "2D Vector Operations";
    TEST_START(current_test);
    
    Vec2D v1 = {3.0, 4.0};
    Vec2D v2 = {1.0, 2.0};
    Vec2D result;
    
    // Addition
    vector2d_add(&v1, &v2, &result);
    ASSERT_NEAR(result.x, 4.0, 0.001, "Addition X incorrect");
    ASSERT_NEAR(result.y, 6.0, 0.001, "Addition Y incorrect");
    
    // Subtraction
    vector2d_subtract(&v1, &v2, &result);
    ASSERT_NEAR(result.x, 2.0, 0.001, "Subtraction X incorrect");
    ASSERT_NEAR(result.y, 2.0, 0.001, "Subtraction Y incorrect");
    
    // Scaling
    vector2d_scale(&v1, 2.0, &result);
    ASSERT_NEAR(result.x, 6.0, 0.001, "Scaling X incorrect");
    ASSERT_NEAR(result.y, 8.0, 0.001, "Scaling Y incorrect");
    
    // Dot product
    double dot = vector2d_dot(&v1, &v2);
    ASSERT_NEAR(dot, 11.0, 0.001, "Dot product incorrect"); // 3*1 + 4*2 = 11
    
    // Magnitude
    double mag = vector2d_magnitude(&v1);
    ASSERT_NEAR(mag, 5.0, 0.001, "Magnitude incorrect"); // math_sqrt(3² + 4²) = 5
    
    // Normalization
    ASSERT_TRUE(vector2d_normalize(&v1, &result), "Normalization failed");
    ASSERT_NEAR(result.x, 0.6, 0.001, "Normalized X incorrect");
    ASSERT_NEAR(result.y, 0.8, 0.001, "Normalized Y incorrect");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 7: 3D Vector Operations
//=============================================================================
void test_vector3d_operations(void) {
    const char* current_test = "3D Vector Operations";
    TEST_START(current_test);
    
    Vec3D v1 = {1.0, 2.0, 3.0};
    Vec3D v2 = {4.0, 5.0, 6.0};
    Vec3D result;
    
    // Addition
    vector3d_add(&v1, &v2, &result);
    ASSERT_NEAR(result.x, 5.0, 0.001, "Addition X incorrect");
    ASSERT_NEAR(result.y, 7.0, 0.001, "Addition Y incorrect");
    ASSERT_NEAR(result.z, 9.0, 0.001, "Addition Z incorrect");
    
    // Subtraction
    vector3d_subtract(&v2, &v1, &result);
    ASSERT_NEAR(result.x, 3.0, 0.001, "Subtraction X incorrect");
    ASSERT_NEAR(result.y, 3.0, 0.001, "Subtraction Y incorrect");
    ASSERT_NEAR(result.z, 3.0, 0.001, "Subtraction Z incorrect");
    
    // Scaling
    vector3d_scale(&v1, 2.0, &result);
    ASSERT_NEAR(result.x, 2.0, 0.001, "Scaling X incorrect");
    ASSERT_NEAR(result.y, 4.0, 0.001, "Scaling Y incorrect");
    ASSERT_NEAR(result.z, 6.0, 0.001, "Scaling Z incorrect");
    
    // Dot product
    double dot = vector3d_dot(&v1, &v2);
    ASSERT_NEAR(dot, 32.0, 0.001, "Dot product incorrect"); // 1*4 + 2*5 + 3*6 = 32
    
    // Cross product
    Vec3D i = {1.0, 0.0, 0.0};
    Vec3D j = {0.0, 1.0, 0.0};
    vector3d_cross(&i, &j, &result);
    ASSERT_NEAR(result.x, 0.0, 0.001, "Cross product X incorrect");
    ASSERT_NEAR(result.y, 0.0, 0.001, "Cross product Y incorrect");
    ASSERT_NEAR(result.z, 1.0, 0.001, "Cross product Z incorrect"); // i × j = k
    
    // Magnitude
    double mag = vector3d_magnitude(&v1);
    double expected_mag = math_sqrt(1.0 + 4.0 + 9.0); // math_sqrt(14)
    ASSERT_NEAR(mag, expected_mag, 0.001, "Magnitude incorrect");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 8: Radial Line Mapping
//=============================================================================
void test_radial_line_mapping(void) {
    const char* current_test = "Radial Line Mapping";
    TEST_START(current_test);
    
    // Test 12-fold symmetry
    for (uint64_t i = 0; i < 24; i++) {
        int line = map_to_radial_line(i);
        ASSERT_TRUE(line >= 0 && line < 12, "Radial line out of range");
        ASSERT_TRUE(line == (int)(i % 12), "Radial line mapping incorrect");
    }
    
    // Test is_on_radial_line
    ASSERT_TRUE(is_on_radial_line(0, 0), "0 should be on line 0");
    ASSERT_TRUE(is_on_radial_line(12, 0), "12 should be on line 0");
    ASSERT_TRUE(is_on_radial_line(5, 5), "5 should be on line 5");
    ASSERT_FALSE(is_on_radial_line(5, 6), "5 should not be on line 6");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 9: Concentric Ring Mapping
//=============================================================================
void test_concentric_ring_mapping(void) {
    const char* current_test = "Concentric Ring Mapping";
    TEST_START(current_test);
    
    // Test ring 0 (origin)
    ASSERT_TRUE(map_to_concentric_ring(0) == 0, "0 should be on ring 0");
    
    // Test ring 1 (first ring around origin)
    for (uint64_t i = 1; i <= 12; i++) {
        int ring = map_to_concentric_ring(i);
        ASSERT_TRUE(ring >= 0, "Ring number should be non-negative");
    }
    
    // Test is_on_concentric_ring
    ASSERT_TRUE(is_on_concentric_ring(0, 0), "0 should be on ring 0");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 10: Symmetry Group Mapping
//=============================================================================
void test_symmetry_group_mapping(void) {
    const char* current_test = "Symmetry Group Mapping";
    TEST_START(current_test);
    
    // Test 12-fold symmetry
    for (uint64_t i = 0; i < 24; i++) {
        int group = map_to_symmetry_group(i);
        ASSERT_TRUE(group >= 0 && group < 12, "Symmetry group out of range");
        ASSERT_TRUE(group == (int)(i % 12), "Symmetry group mapping incorrect");
    }
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 11: Angular Operations
//=============================================================================
void test_angular_operations(void) {
    const char* current_test = "Angular Operations";
    TEST_START(current_test);
    
    // Test map_to_angle
    double angle0 = map_to_angle(0);
    ASSERT_NEAR(angle0, 0.0, 0.001, "Angle for 0 should be 0");
    
    double angle6 = map_to_angle(6);
    ASSERT_NEAR(angle6, MATH_PI, 0.001, "Angle for 6 should be π");
    
    // Test angle_to_clock_position
    ASSERT_TRUE(angle_to_clock_position(0.0) == 0, "0 radians should be position 0");
    ASSERT_TRUE(angle_to_clock_position(MATH_PI / 2.0) == 3, "π/2 should be position 3");
    
    // Test clock_position_to_angle
    double angle_pos0 = clock_position_to_angle(0);
    ASSERT_NEAR(angle_pos0, 0.0, 0.001, "Position 0 should be 0 radians");
    
    double angle_pos3 = clock_position_to_angle(3);
    ASSERT_NEAR(angle_pos3, MATH_PI / 2.0, 0.001, "Position 3 should be π/2");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 12: Packing Density
//=============================================================================
void test_packing_density(void) {
    const char* current_test = "Packing Density";
    TEST_START(current_test);
    
    // Test with simple case
    double density = calculate_packing_density(1, 2.0, 1.0);
    ASSERT_TRUE(density > 0.0 && density <= 1.0, "Density should be between 0 and 1");
    
    // Test with multiple spheres
    double density2 = calculate_packing_density(12, 5.0, 1.0);
    ASSERT_TRUE(density2 > 0.0 && density2 <= 1.0, "Density should be between 0 and 1");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 13: Kissing Number
//=============================================================================
void test_kissing_number(void) {
    const char* current_test = "Kissing Number";
    TEST_START(current_test);
    
    // Test known kissing numbers
    ASSERT_TRUE(get_kissing_number(2) == 6, "2D kissing number should be 6");
    ASSERT_TRUE(get_kissing_number(3) == 12, "3D kissing number should be 12");
    ASSERT_TRUE(get_kissing_number(4) == 24, "4D kissing number should be 24");
    ASSERT_TRUE(get_kissing_number(8) == 240, "8D kissing number should be 240");
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 14: Generate Kissing Spheres
//=============================================================================
void test_generate_kissing_spheres(void) {
    const char* current_test = "Generate Kissing Spheres";
    TEST_START(current_test);
    
    // Create central sphere
    Sphere3D central = {0.0, 0.0, 0.0, 1.0, 0};
    
    // Generate kissing spheres
    Sphere3D kissing[12];
    int count = generate_kissing_spheres(&central, kissing, 12);
    
    ASSERT_TRUE(count == 12, "Should generate 12 kissing spheres");
    
    // Verify all spheres are kissing the central sphere
    for (int i = 0; i < 12; i++) {
        ASSERT_TRUE(spheres_are_kissing(&central, &kissing[i], 0.01), 
                   "Generated sphere should be kissing central sphere");
        ASSERT_NEAR(kissing[i].radius, 1.0, 0.001, "Radius should match central");
    }
    
    // Verify 12-fold symmetry (spheres evenly distributed)
    double expected_angle_step = 2.0 * MATH_PI / 12.0;
    for (int i = 0; i < 12; i++) {
        double dx = kissing[i].center_x - central.center_x;
        double dy = kissing[i].center_y - central.center_y;
        double angle = prime_atan2(dy, dx);
        if (angle < 0) angle += 2.0 * MATH_PI;
        
        double expected_angle = (double)i * expected_angle_step;
        double angle_diff = math_abs(angle - expected_angle);
        
        // Allow for wraparound
        if (angle_diff > MATH_PI) {
            angle_diff = 2.0 * MATH_PI - angle_diff;
        }
        
        ASSERT_TRUE(angle_diff < 0.1, "Sphere not at expected angle");
    }
    
    TEST_PASS(current_test);
}

//=============================================================================
// Main Test Runner
//=============================================================================
int main(void) {
    printf("\n");
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf(COLOR_YELLOW "  Sphere Packing Geometry Test Suite\n" COLOR_RESET);
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf("\n");
    
    // Run all tests
    test_kissing_spheres();
    test_sphere_gap();
    test_sphere_distance();
    test_sphere_overlap();
    test_sphere_volume_area();
    test_vector2d_operations();
    test_vector3d_operations();
    test_radial_line_mapping();
    test_concentric_ring_mapping();
    test_symmetry_group_mapping();
    test_angular_operations();
    test_packing_density();
    test_kissing_number();
    test_generate_kissing_spheres();
    
    // Print summary
    printf("\n");
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf(COLOR_YELLOW "  Test Summary\n" COLOR_RESET);
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf("Total Tests: %d\n", tests_passed + tests_failed);
    printf(COLOR_GREEN "Passed: %d\n" COLOR_RESET, tests_passed);
    printf(COLOR_RED "Failed: %d\n" COLOR_RESET, tests_failed);
    
    if (tests_failed == 0) {
        printf(COLOR_GREEN "\n✓ ALL TESTS PASSED!\n" COLOR_RESET);
        return 0;
    } else {
        printf(COLOR_RED "\n✗ SOME TESTS FAILED\n" COLOR_RESET);
        return 1;
    }
}
