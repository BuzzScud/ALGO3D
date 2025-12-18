/**
 * @file test_geometric_space_ops.c
 * @brief Test suite for geometric space operations
 */

#include "geometric_space_ops.h"
#include "abacus88d.h"
#include <stdio.h>
#include <assert.h>

#define TEST_PASS "\033[32m✓\033[0m"
#define TEST_FAIL "\033[31m✗\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\nTest: %s\n", name); \
    printf("----------------------------------------\n");

#define ASSERT_TRUE(condition, message) \
    do { \
        if (condition) { \
            printf("%s %s\n", TEST_PASS, message); \
            tests_passed++; \
        } else { \
            printf("%s %s\n", TEST_FAIL, message); \
            tests_failed++; \
        } \
    } while(0)

void test_boundary_detection() {
    TEST("Boundary Detection");
    
    Abacus88D* abacus = abacus88d_create(60);
    if (!abacus) {
        printf("%s Failed to create abacus\n", TEST_FAIL);
        tests_failed++;
        return;
    }
    
    // Create a value in layer 0
    CrystallineAbacus* value = abacus_from_uint64(500, 60);
    ASSERT_TRUE(value != NULL, "Created test value");
    
    // Detect boundaries
    BoundaryInfo boundary;
    MathError err = geometric_detect_boundary(abacus, 0, 0, value, &boundary);
    ASSERT_TRUE(err == MATH_SUCCESS, "Boundary detection succeeded");
    ASSERT_TRUE(boundary.current_layer == 0, "Current layer is 0");
    ASSERT_TRUE(boundary.current_dimension == 0, "Current dimension is 0");
    
    // Test with value near layer boundary
    CrystallineAbacus* large_value = abacus_from_uint64(950, 60);
    err = geometric_detect_boundary(abacus, 0, 0, large_value, &boundary);
    ASSERT_TRUE(err == MATH_SUCCESS, "Boundary detection for large value");
    
    // Test layer transition check
    uint8_t target_layer;
    bool should_transition = geometric_should_transition_layer(large_value, 0, &target_layer);
    ASSERT_TRUE(should_transition == false, "Value 950 stays in layer 0");
    
    // Test with value that should transition
    CrystallineAbacus* huge_value = abacus_from_uint64(1500, 60);
    should_transition = geometric_should_transition_layer(huge_value, 0, &target_layer);
    ASSERT_TRUE(should_transition == true, "Value 1500 should transition");
    ASSERT_TRUE(target_layer == 1, "Should transition to layer 1");
    
    // Test shared vertex detection
    uint8_t shared_dims[12];
    uint8_t num_shared;
    err = geometric_find_shared_vertices(abacus, 0, 5, shared_dims, &num_shared);
    ASSERT_TRUE(err == MATH_SUCCESS, "Found shared vertices");
    ASSERT_TRUE(num_shared > 0, "Has shared vertices");
    printf("  Dimension 5 shares vertices with %u dimensions\n", num_shared);
    
    abacus_free(value);
    abacus_free(large_value);
    abacus_free(huge_value);
    abacus88d_free(abacus);
}

void test_coordinate_transformation() {
    TEST("Coordinate Transformation");
    
    // Test transformation matrix calculation
    TransformMatrix transform;
    MathError err = geometric_calculate_transform(0, 2, &transform);
    ASSERT_TRUE(err == MATH_SUCCESS, "Calculated transform from layer 0 to 2");
    ASSERT_TRUE(transform.source_layer == 0, "Source layer is 0");
    ASSERT_TRUE(transform.target_layer == 2, "Target layer is 2");
    ASSERT_TRUE(transform.scale_factor > 1.0, "Scale factor is positive");
    printf("  Scale factor: %f\n", transform.scale_factor);
    
    // Test applying transformation
    CrystallineAbacus* value = abacus_from_uint64(100, 60);
    CrystallineAbacus* result = abacus_new(60);
    
    err = geometric_apply_transform(value, &transform, result);
    ASSERT_TRUE(err == MATH_SUCCESS, "Applied transformation");
    
    uint64_t result_val;
    abacus_to_uint64(result, &result_val);
    printf("  100 * %f = %lu\n", transform.scale_factor, result_val);
    ASSERT_TRUE(result_val > 100, "Result is scaled up");
    
    // Test dimension transformation
    CrystallineAbacus* dim_result = abacus_new(60);
    err = geometric_transform_dimension(value, 0, 5, dim_result);
    ASSERT_TRUE(err == MATH_SUCCESS, "Transformed between dimensions");
    
    abacus_free(value);
    abacus_free(result);
    abacus_free(dim_result);
}

void test_handoff_operations() {
    TEST("Handoff Operations");
    
    Abacus88D* abacus = abacus88d_create(60);
    if (!abacus) {
        printf("%s Failed to create abacus\n", TEST_FAIL);
        tests_failed++;
        return;
    }
    
    // Create boundary info
    BoundaryInfo boundary;
    boundary.current_layer = 0;
    boundary.current_dimension = 3;
    boundary.near_layer_boundary = true;
    boundary.target_layer = 1;
    boundary.requires_handoff = true;
    
    // Prepare handoff context
    HandoffContext context;
    MathError err = geometric_prepare_handoff(&boundary, &context);
    ASSERT_TRUE(err == MATH_SUCCESS, "Prepared handoff context");
    ASSERT_TRUE(context.source_layer == 0, "Source layer is 0");
    ASSERT_TRUE(context.target_layer == 1, "Target layer is 1");
    ASSERT_TRUE(context.requires_scaling == true, "Requires scaling");
    
    // Execute handoff
    CrystallineAbacus* value = abacus_from_uint64(500, 60);
    err = geometric_execute_handoff(abacus, abacus, &context, value);
    ASSERT_TRUE(err == MATH_SUCCESS, "Executed handoff");
    
    // Verify value was set in target location
    CrystallineAbacus* retrieved = abacus_new(60);
    err = abacus88d_get(abacus, 1, 3, retrieved);
    ASSERT_TRUE(err == MATH_SUCCESS, "Retrieved value from target");
    
    uint64_t retrieved_val;
    abacus_to_uint64(retrieved, &retrieved_val);
    printf("  Handed off value: %lu\n", retrieved_val);
    ASSERT_TRUE(retrieved_val > 0, "Value was transferred");
    
    // Test handoff value calculation
    CrystallineAbacus* handoff_value = abacus_new(60);
    err = geometric_calculate_handoff_value(value, &boundary, handoff_value);
    ASSERT_TRUE(err == MATH_SUCCESS, "Calculated handoff value");
    
    abacus_free(value);
    abacus_free(retrieved);
    abacus_free(handoff_value);
    abacus88d_free(abacus);
}

void test_work_distribution() {
    TEST("Work Distribution");
    
    Abacus88D* abacus = abacus88d_create(60);
    if (!abacus) {
        printf("%s Failed to create abacus\n", TEST_FAIL);
        tests_failed++;
        return;
    }
    
    // Create work items
    WorkItem items[10];
    for (int i = 0; i < 10; i++) {
        items[i].layer = i % 8;
        items[i].dimension = i % 11;
        items[i].work_size = 100 + i * 10;
        items[i].priority = i;
        items[i].data = NULL;
    }
    
    // Distribute work across 3 workers
    WorkDistribution distribution;
    MathError err = geometric_distribute_work(abacus, items, 10, 3, &distribution);
    ASSERT_TRUE(err == MATH_SUCCESS, "Distributed work");
    ASSERT_TRUE(distribution.num_workers == 3, "Has 3 workers");
    
    // Check distribution
    uint32_t total_load = 0;
    for (uint32_t i = 0; i < distribution.num_workers; i++) {
        printf("  Worker %u: %u items, load %u\n", 
               i, distribution.items_per_worker[i], distribution.worker_loads[i]);
        total_load += distribution.worker_loads[i];
        ASSERT_TRUE(distribution.items_per_worker[i] > 0, "Worker has items");
    }
    
    printf("  Total load: %u\n", total_load);
    ASSERT_TRUE(total_load > 0, "Total load is positive");
    
    // Free distribution
    geometric_free_distribution(&distribution);
    ASSERT_TRUE(distribution.worker_items == NULL, "Distribution freed");
    
    abacus88d_free(abacus);
}

int main() {
    printf("========================================\n");
    printf("Geometric Space Operations Test Suite\n");
    printf("========================================\n");
    
    test_boundary_detection();
    test_coordinate_transformation();
    test_handoff_operations();
    test_work_distribution();
    
    printf("\n========================================\n");
    printf("Test Results\n");
    printf("========================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n%s ALL TESTS PASSED\n", TEST_PASS);
        return 0;
    } else {
        printf("\n%s SOME TESTS FAILED\n", TEST_FAIL);
        return 1;
    }
}