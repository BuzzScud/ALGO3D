/**
 * Test Suite for Unified Visualization Framework
 * 
 * Tests all visualization modes: 2D, 3D, and Crystalline
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/visualization.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

// Test macros
#define TEST_START(name) \
    do { \
        printf("\n" COLOR_YELLOW "TEST: %s" COLOR_RESET "\n", name); \
        tests_run++; \
    } while(0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf(COLOR_RED "  ✗ FAILED: %s" COLOR_RESET "\n", message); \
            tests_failed++; \
            return 0; \
        } \
    } while(0)

#define TEST_PASS() \
    do { \
        printf(COLOR_GREEN "  ✓ PASSED" COLOR_RESET "\n"); \
        tests_passed++; \
        return 1; \
    } while(0)

// ============================================================================
// TEST: Basic Creation and Destruction
// ============================================================================

int test_create_destroy_2d(void) {
    TEST_START("Create and Destroy 2D Visualization");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 100);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    TEST_ASSERT(data->mode == VIZ_MODE_2D, "Incorrect mode");
    TEST_ASSERT(data->num_points == 100, "Incorrect number of points");
    TEST_ASSERT(data->points != NULL, "Points array not allocated");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_create_destroy_3d(void) {
    TEST_START("Create and Destroy 3D Visualization");
    
    VisualizationData* data = viz_create(VIZ_MODE_3D, 50);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    TEST_ASSERT(data->mode == VIZ_MODE_3D, "Incorrect mode");
    TEST_ASSERT(data->num_points == 50, "Incorrect number of points");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_create_destroy_crystalline(void) {
    TEST_START("Create and Destroy Crystalline Visualization");
    
    VisualizationData* data = viz_create(VIZ_MODE_CRYSTALLINE, 720);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    TEST_ASSERT(data->mode == VIZ_MODE_CRYSTALLINE, "Incorrect mode");
    TEST_ASSERT(data->num_points == 720, "Incorrect number of points");
    
    viz_free(data);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Data Population
// ============================================================================

int test_set_point_position(void) {
    TEST_START("Set Point Position");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    double position[2] = {1.5, 2.5};
    int result = viz_set_point_position(data, 0, position);
    TEST_ASSERT(result == 0, "Failed to set point position");
    TEST_ASSERT(math_abs(data->points[0].position[0] - 1.5) < 1e-9, "X position incorrect");
    TEST_ASSERT(math_abs(data->points[0].position[1] - 2.5) < 1e-9, "Y position incorrect");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_set_point_value(void) {
    TEST_START("Set Point Value");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    int result = viz_set_point_value(data, 0, 42.5);
    TEST_ASSERT(result == 0, "Failed to set point value");
    TEST_ASSERT(math_abs(data->points[0].value - 42.5) < 1e-9, "Value incorrect");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_set_point_neighbors(void) {
    TEST_START("Set Point Neighbors");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    uint32_t neighbors[3] = {1, 2, 3};
    int result = viz_set_point_neighbors(data, 0, neighbors, 3);
    TEST_ASSERT(result == 0, "Failed to set neighbors");
    TEST_ASSERT(data->points[0].num_neighbors == 3, "Incorrect neighbor count");
    TEST_ASSERT(data->points[0].neighbors[0] == 1, "Neighbor 0 incorrect");
    TEST_ASSERT(data->points[0].neighbors[1] == 2, "Neighbor 1 incorrect");
    TEST_ASSERT(data->points[0].neighbors[2] == 3, "Neighbor 2 incorrect");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_set_point_label(void) {
    TEST_START("Set Point Label");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    int result = viz_set_point_label(data, 0, "TestPoint");
    TEST_ASSERT(result == 0, "Failed to set label");
    TEST_ASSERT(strcmp(data->points[0].label, "TestPoint") == 0, "Label incorrect");
    
    viz_free(data);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Projection Algorithms
// ============================================================================

int test_project_2d(void) {
    TEST_START("2D Projection");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set some positions
    for (uint32_t i = 0; i < 10; i++) {
        double pos[2] = {(double)i, (double)(i * 2)};
        viz_set_point_position(data, i, pos);
    }
    
    int result = viz_project_2d(data);
    TEST_ASSERT(result == 0, "2D projection failed");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_project_3d(void) {
    TEST_START("3D Projection");
    
    VisualizationData* data = viz_create(VIZ_MODE_3D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set some positions
    for (uint32_t i = 0; i < 10; i++) {
        double pos[3] = {(double)i, (double)(i * 2), (double)(i * 3)};
        viz_set_point_position(data, i, pos);
    }
    
    int result = viz_project_3d(data);
    TEST_ASSERT(result == 0, "3D projection failed");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_project_crystalline(void) {
    TEST_START("Crystalline Projection");
    
    VisualizationData* data = viz_create(VIZ_MODE_CRYSTALLINE, 720);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set symmetry groups for clock geometry
    for (uint32_t i = 0; i < 720; i++) {
        data->points[i].symmetry_group = i % 12;
    }
    
    int result = viz_project_crystalline(data);
    TEST_ASSERT(result == 0, "Crystalline projection failed");
    
    viz_free(data);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Statistics
// ============================================================================

int test_calculate_statistics(void) {
    TEST_START("Calculate Statistics");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set up some data
    for (uint32_t i = 0; i < 10; i++) {
        double pos[2] = {(double)i, (double)i};
        viz_set_point_position(data, i, pos);
        viz_set_point_value(data, i, (double)i);
        
        uint32_t neighbors[2] = {(i + 1) % 10, (i + 2) % 10};
        viz_set_point_neighbors(data, i, neighbors, 2);
    }
    
    VisualizationStatistics stats;
    int result = viz_calculate_statistics(data, &stats);
    TEST_ASSERT(result == 0, "Failed to calculate statistics");
    TEST_ASSERT(stats.num_points == 10, "Incorrect point count");
    TEST_ASSERT(stats.avg_neighbors == 2.0, "Incorrect average neighbors");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_update_bounds(void) {
    TEST_START("Update Bounds");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set positions with known bounds
    for (uint32_t i = 0; i < 10; i++) {
        double pos[2] = {(double)i, (double)(i * 2)};
        viz_set_point_position(data, i, pos);
    }
    
    int result = viz_update_bounds(data);
    TEST_ASSERT(result == 0, "Failed to update bounds");
    TEST_ASSERT(math_abs(data->min_bounds[0] - 0.0) < 1e-9, "Min X bound incorrect");
    TEST_ASSERT(math_abs(data->max_bounds[0] - 9.0) < 1e-9, "Max X bound incorrect");
    TEST_ASSERT(math_abs(data->min_bounds[1] - 0.0) < 1e-9, "Min Y bound incorrect");
    TEST_ASSERT(math_abs(data->max_bounds[1] - 18.0) < 1e-9, "Max Y bound incorrect");
    
    viz_free(data);
    
    TEST_PASS();
}

int test_calculate_symmetry_distribution(void) {
    TEST_START("Calculate Symmetry Distribution");
    
    VisualizationData* data = viz_create(VIZ_MODE_CRYSTALLINE, 120);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set symmetry groups evenly
    for (uint32_t i = 0; i < 120; i++) {
        data->points[i].symmetry_group = i % 12;
    }
    
    int result = viz_calculate_symmetry_distribution(data);
    TEST_ASSERT(result == 0, "Failed to calculate symmetry distribution");
    
    // Each group should have 10 points
    for (int i = 0; i < 12; i++) {
        TEST_ASSERT(data->symmetry_counts[i] == 10, "Symmetry count incorrect");
    }
    
    viz_free(data);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Export Functions
// ============================================================================

int test_export_json(void) {
    TEST_START("Export to JSON");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 5);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set up minimal data
    for (uint32_t i = 0; i < 5; i++) {
        double pos[2] = {(double)i, (double)i};
        viz_set_point_position(data, i, pos);
        viz_set_point_value(data, i, (double)i);
    }
    
    int result = viz_export_json(data, "test_output.json");
    TEST_ASSERT(result == 0, "Failed to export JSON");
    
    // Check if file exists
    FILE* f = fopen("test_output.json", "r");
    TEST_ASSERT(f != NULL, "JSON file not created");
    fclose(f);
    
    // Clean up
    remove("test_output.json");
    viz_free(data);
    
    TEST_PASS();
}

int test_export_csv(void) {
    TEST_START("Export to CSV");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 5);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set up minimal data
    for (uint32_t i = 0; i < 5; i++) {
        double pos[2] = {(double)i, (double)i};
        viz_set_point_position(data, i, pos);
        viz_set_point_value(data, i, (double)i);
    }
    
    int result = viz_export_csv(data, "test_output.csv");
    TEST_ASSERT(result == 0, "Failed to export CSV");
    
    // Check if file exists
    FILE* f = fopen("test_output.csv", "r");
    TEST_ASSERT(f != NULL, "CSV file not created");
    fclose(f);
    
    // Clean up
    remove("test_output.csv");
    viz_free(data);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Validation
// ============================================================================

int test_validate(void) {
    TEST_START("Validate Visualization Data");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 10);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set up valid data
    for (uint32_t i = 0; i < 10; i++) {
        double pos[2] = {(double)i, (double)i};
        viz_set_point_position(data, i, pos);
        viz_set_point_value(data, i, (double)i);
    }
    
    int result = viz_validate(data);
    TEST_ASSERT(result == 1, "Validation failed for valid data");
    
    viz_free(data);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Rendering Data Preparation
// ============================================================================

int test_get_2d_rendering_data(void) {
    TEST_START("Get 2D Rendering Data");
    
    VisualizationData* data = viz_create(VIZ_MODE_2D, 5);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set up data with connections
    for (uint32_t i = 0; i < 5; i++) {
        double pos[2] = {(double)i, (double)i};
        viz_set_point_position(data, i, pos);
        
        uint32_t neighbors[1] = {(i + 1) % 5};
        viz_set_point_neighbors(data, i, neighbors, 1);
    }
    
    double* positions = NULL;
    uint32_t* connections = NULL;
    uint32_t num_connections = 0;
    
    int result = viz_get_2d_rendering_data(data, &positions, &connections, &num_connections);
    TEST_ASSERT(result == 0, "Failed to get rendering data");
    TEST_ASSERT(positions != NULL, "Positions not allocated");
    TEST_ASSERT(connections != NULL, "Connections not allocated");
    TEST_ASSERT(num_connections == 5, "Incorrect connection count");
    
    free(positions);
    free(connections);
    viz_free(data);
    
    TEST_PASS();
}

int test_get_3d_rendering_data(void) {
    TEST_START("Get 3D Rendering Data");
    
    VisualizationData* data = viz_create(VIZ_MODE_3D, 5);
    TEST_ASSERT(data != NULL, "Failed to create visualization data");
    
    // Set up data with connections
    for (uint32_t i = 0; i < 5; i++) {
        double pos[3] = {(double)i, (double)i, (double)i};
        viz_set_point_position(data, i, pos);
        
        uint32_t neighbors[1] = {(i + 1) % 5};
        viz_set_point_neighbors(data, i, neighbors, 1);
    }
    
    double* positions = NULL;
    uint32_t* connections = NULL;
    uint32_t num_connections = 0;
    
    int result = viz_get_3d_rendering_data(data, &positions, &connections, &num_connections);
    TEST_ASSERT(result == 0, "Failed to get rendering data");
    TEST_ASSERT(positions != NULL, "Positions not allocated");
    TEST_ASSERT(connections != NULL, "Connections not allocated");
    
    free(positions);
    free(connections);
    viz_free(data);
    
    TEST_PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Visualization Framework Test Suite\n");
    printf("========================================\n");
    
    // Basic tests
    test_create_destroy_2d();
    test_create_destroy_3d();
    test_create_destroy_crystalline();
    
    // Data population tests
    test_set_point_position();
    test_set_point_value();
    test_set_point_neighbors();
    test_set_point_label();
    
    // Projection tests
    test_project_2d();
    test_project_3d();
    test_project_crystalline();
    
    // Statistics tests
    test_calculate_statistics();
    test_update_bounds();
    test_calculate_symmetry_distribution();
    
    // Export tests
    test_export_json();
    test_export_csv();
    
    // Validation tests
    test_validate();
    
    // Rendering data tests
    test_get_2d_rendering_data();
    test_get_3d_rendering_data();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("  Total Tests:  %d\n", tests_run);
    printf("  " COLOR_GREEN "Passed:       %d" COLOR_RESET "\n", tests_passed);
    printf("  " COLOR_RED "Failed:       %d" COLOR_RESET "\n", tests_failed);
    printf("  Success Rate: %.1f%%\n", (tests_passed * 100.0) / tests_run);
    printf("========================================\n\n");
    
    return (tests_failed == 0) ? 0 : 1;
}
