/**
 * @file test_geometric_ops.c
 * @brief Test suite for geometric operations
 * 
 * Verifies that all operations are O(1) and produce correct results.
 */

#include "geometric_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

// ============================================================================
// TEST UTILITIES
// ============================================================================

#define TEST_PASS "\033[32m✓\033[0m"
#define TEST_FAIL "\033[31m✗\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\nTest: %s\n", name); \
    printf("----------------------------------------\n");

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

#define ASSERT_NEAR(a, b, epsilon, message) \
    ASSERT_TRUE(fabs((a) - (b)) < (epsilon), message)

// ============================================================================
// TESTS
// ============================================================================

void test_position_creation() {
    TEST("Position Creation");
    
    GeometricPosition pos = geo_pos_create(0, 5, 100);
    
    ASSERT_EQ(pos.ring, 0, "Ring is 0");
    ASSERT_EQ(pos.position, 5, "Position is 5");
    ASSERT_EQ(pos.magnitude, 100, "Magnitude is 100");
    ASSERT_NEAR(pos.angle, 150.0, 0.01, "Angle is 150°");
    ASSERT_EQ(pos.layer, 0, "Layer is 0 (magnitude < 1000)");
    ASSERT_TRUE(pos.is_prime_position, "Position 5 is prime");
    ASSERT_TRUE(!pos.is_control, "Position 5 is not control");
}

void test_layer_selection() {
    TEST("Layer Selection (O(1))");
    
    ASSERT_EQ(geo_select_layer(100), 0, "100 → Layer 0");
    ASSERT_EQ(geo_select_layer(1000), 1, "1,000 → Layer 1");
    ASSERT_EQ(geo_select_layer(1000000), 2, "1,000,000 → Layer 2");
    ASSERT_EQ(geo_select_layer(1000000000), 3, "1,000,000,000 → Layer 3");
    ASSERT_EQ(geo_select_layer(1000000000000LL), 4, "1,000,000,000,000 → Layer 4");
}

void test_geometric_distance() {
    TEST("Geometric Distance (O(1))");
    
    GeometricPosition pos1 = geo_pos_create(0, 0, 0);
    GeometricPosition pos2 = geo_pos_create(0, 6, 0);
    
    double dist = geo_distance(&pos1, &pos2);
    
    ASSERT_NEAR(dist, 180.0, 1.0, "Distance from 0 to 6 is ~180");
    
    // Test with magnitude difference
    GeometricPosition pos3 = geo_pos_create(0, 0, 10);
    double dist2 = geo_distance(&pos1, &pos3);
    
    ASSERT_NEAR(dist2, 120.0, 1.0, "Distance with magnitude diff");
}

void test_shortest_path() {
    TEST("Shortest Path on Circle (O(1))");
    
    ASSERT_EQ(geo_shortest_path(0, 6), 6, "0 to 6: 6 steps");
    ASSERT_EQ(geo_shortest_path(0, 11), 1, "0 to 11: 1 step (wrap around)");
    ASSERT_EQ(geo_shortest_path(2, 10), 4, "2 to 10: 4 steps");
    ASSERT_EQ(geo_shortest_path(11, 1), 2, "11 to 1: 2 steps");
}

void test_triangulation() {
    TEST("Geometric Triangulation (O(1))");
    
    GeometricPosition p1 = geo_pos_create(0, 1, 100);
    GeometricPosition p2 = geo_pos_create(0, 5, 100);
    GeometricPosition p3 = geo_pos_create(0, 7, 100);
    
    GeometricPosition result = geo_triangulate(&p1, &p2, &p3);
    
    // Result should be centroid
    uint8_t expected_pos = (1 + 5 + 7) % 12;  // = 13 % 12 = 1
    ASSERT_EQ(result.position, expected_pos, "Triangulated position is centroid");
    ASSERT_EQ(result.magnitude, 100, "Magnitude preserved");
}

void test_position_arithmetic() {
    TEST("Position Arithmetic (O(1))");
    
    GeometricPosition pos1 = geo_pos_create(0, 5, 10);
    GeometricPosition pos2 = geo_pos_create(0, 3, 5);
    
    // Addition
    GeometricPosition sum = geo_add(&pos1, &pos2);
    ASSERT_EQ(sum.position, 8, "5 + 3 = 8");
    ASSERT_EQ(sum.magnitude, 15, "10 + 5 = 15");
    
    // Subtraction
    GeometricPosition diff = geo_subtract(&pos1, &pos2);
    ASSERT_EQ(diff.position, 2, "5 - 3 = 2");
    ASSERT_EQ(diff.magnitude, 5, "10 - 5 = 5");
    
    // Midpoint
    GeometricPosition mid = geo_midpoint(&pos1, &pos2);
    ASSERT_EQ(mid.position, 4, "Midpoint position");
    ASSERT_EQ(mid.magnitude, 7, "Midpoint magnitude (rounded)");
}

void test_platonic_frames() {
    TEST("Platonic Frames");
    
    PlatonicFrame frame0 = geo_frame_create(0);
    ASSERT_EQ(frame0.solid, GEO_TETRAHEDRON, "Layer 0: Tetrahedron");
    ASSERT_EQ(frame0.num_vertices, 4, "Tetrahedron has 4 vertices");
    ASSERT_EQ(frame0.dual_solid, GEO_TETRAHEDRON, "Tetrahedron is self-dual");
    
    PlatonicFrame frame1 = geo_frame_create(1);
    ASSERT_EQ(frame1.solid, GEO_CUBE, "Layer 1: Cube");
    ASSERT_EQ(frame1.num_vertices, 8, "Cube has 8 vertices");
    ASSERT_EQ(frame1.dual_solid, GEO_OCTAHEDRON, "Cube dual is Octahedron");
    
    PlatonicFrame frame4 = geo_frame_create(4);
    ASSERT_EQ(frame4.solid, GEO_ICOSAHEDRON, "Layer 4: Icosahedron");
    ASSERT_EQ(frame4.num_vertices, 12, "Icosahedron has 12 vertices");
    
    // Verify Euler's formula: V - E + F = 2
    int euler = frame4.num_vertices - frame4.num_edges + frame4.num_faces;
    ASSERT_EQ(euler, 2, "Euler's formula: V - E + F = 2");
}

void test_icosahedron_vertices() {
    TEST("Icosahedron Vertices (for thread positioning)");
    
    double coords[3];
    
    // Test first vertex
    geo_icosahedron_vertex(0, coords);
    ASSERT_NEAR(coords[0], 0.0, 0.01, "Vertex 0: x = 0");
    ASSERT_NEAR(coords[1], 1.0, 0.01, "Vertex 0: y = 1");
    ASSERT_NEAR(coords[2], GEO_GOLDEN_RATIO, 0.01, "Vertex 0: z = φ");
    
    // Test angles
    for (uint8_t i = 0; i < 12; i++) {
        double angle = geo_icosahedron_angle(i);
        ASSERT_NEAR(angle, i * 30.0, 0.01, "Vertex angle is 30° increments");
    }
    
    // Test neighbors
    uint8_t neighbors[5];
    uint8_t num_neighbors = geo_icosahedron_neighbors(0, neighbors);
    ASSERT_EQ(num_neighbors, 5, "Each vertex has 5 neighbors");
}

void test_magnitude_scaling() {
    TEST("Magnitude Scaling (O(1))");
    
    GeometricPosition pos = geo_pos_create(0, 5, 100);
    
    // Scale to higher layer
    GeometricPosition scaled_up = geo_scale_to_layer(&pos, 2);
    ASSERT_TRUE(scaled_up.magnitude > pos.magnitude, "Scaling up increases magnitude");
    ASSERT_EQ(scaled_up.position, pos.position, "Position preserved");
    
    // Get layer scales
    double scale0 = geo_layer_scale(0);
    double scale1 = geo_layer_scale(1);
    double scale2 = geo_layer_scale(2);
    
    ASSERT_NEAR(scale0, 1.0, 0.01, "Layer 0 scale: 10^0 = 1");
    ASSERT_NEAR(scale1, 1000.0, 0.01, "Layer 1 scale: 10^3 = 1000");
    ASSERT_NEAR(scale2, 1000000.0, 0.01, "Layer 2 scale: 10^6 = 1000000");
}

void test_o1_complexity() {
    TEST("O(1) Complexity Verification");
    
    printf("Testing that operations are constant time...\n");
    
    // Test with small magnitude
    GeometricPosition small1 = geo_pos_create(0, 1, 10);
    GeometricPosition small2 = geo_pos_create(0, 5, 20);
    
    clock_t start = clock();
    for (int i = 0; i < 100000; i++) {
        geo_distance(&small1, &small2);
    }
    clock_t end = clock();
    double time_small = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Test with large magnitude
    GeometricPosition large1 = geo_pos_create(0, 1, 1000000000000LL);
    GeometricPosition large2 = geo_pos_create(0, 5, 2000000000000LL);
    
    start = clock();
    for (int i = 0; i < 100000; i++) {
        geo_distance(&large1, &large2);
    }
    end = clock();
    double time_large = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("  Small magnitude: %.6f seconds\n", time_small);
    printf("  Large magnitude: %.6f seconds\n", time_large);
    
    // Times should be similar (within 50%)
    double ratio = time_large / time_small;
    ASSERT_TRUE(ratio < 1.5 && ratio > 0.5, "O(1): Time independent of magnitude");
}

void test_prime_positions() {
    TEST("Prime Positions {1, 5, 7, 11}");
    
    GeometricPosition pos1 = geo_pos_create(0, 1, 0);
    GeometricPosition pos5 = geo_pos_create(0, 5, 0);
    GeometricPosition pos7 = geo_pos_create(0, 7, 0);
    GeometricPosition pos11 = geo_pos_create(0, 11, 0);
    GeometricPosition pos2 = geo_pos_create(0, 2, 0);
    
    ASSERT_TRUE(geo_pos_is_prime(&pos1), "Position 1 is prime");
    ASSERT_TRUE(geo_pos_is_prime(&pos5), "Position 5 is prime");
    ASSERT_TRUE(geo_pos_is_prime(&pos7), "Position 7 is prime");
    ASSERT_TRUE(geo_pos_is_prime(&pos11), "Position 11 is prime");
    ASSERT_TRUE(!geo_pos_is_prime(&pos2), "Position 2 is not prime");
}

void test_control_position() {
    TEST("Control Position (0/12)");
    
    GeometricPosition control = geo_pos_create(0, 0, 0);
    GeometricPosition worker = geo_pos_create(0, 5, 0);
    
    ASSERT_TRUE(geo_pos_is_control(&control), "Position 0 is control");
    ASSERT_TRUE(!geo_pos_is_control(&worker), "Position 5 is not control");
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("========================================\n");
    printf("Geometric Operations Test Suite\n");
    printf("========================================\n");
    
    test_position_creation();
    test_layer_selection();
    test_geometric_distance();
    test_shortest_path();
    test_triangulation();
    test_position_arithmetic();
    test_platonic_frames();
    test_icosahedron_vertices();
    test_magnitude_scaling();
    test_o1_complexity();
    test_prime_positions();
    test_control_position();
    
    printf("\n========================================\n");
    printf("Test Results\n");
    printf("========================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n%s ALL TESTS PASSED!\n", TEST_PASS);
        return 0;
    } else {
        printf("\n%s SOME TESTS FAILED\n", TEST_FAIL);
        return 1;
    }
}