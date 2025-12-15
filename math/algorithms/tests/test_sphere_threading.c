/**
 * Test Suite for Generic Sphere Threading
 * 
 * Comprehensive tests for the algorithms layer sphere threading model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/sphere_threading.h"

// Test 1: Initialize sphere threading model
static int test_init_sphere_threading(void) {
    printf("Test 1: Initialize sphere threading model... ");
    
    SphereThreadingModel* model = sphere_threading_init(100);
    assert(model != NULL);
    assert(model->num_spheres == 100);
    assert(model->spheres != NULL);
    assert(model->neighbor_array != NULL);
    assert(model->cache_optimized == 0);
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 2: Verify 12 neighbors per sphere
static int test_12_neighbors(void) {
    printf("Test 2: Verify 12 neighbors per sphere... ");
    
    SphereThreadingModel* model = sphere_threading_init(500);
    assert(model != NULL);
    
    // Verify all spheres have exactly 12 neighbors
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        assert(model->spheres[i].num_neighbors == SPHERE_NUM_NEIGHBORS);
        assert(model->spheres[i].num_neighbors == 12);
    }
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 3: Verify 12-fold symmetry
static int test_12_fold_symmetry(void) {
    printf("Test 3: Verify 12-fold symmetry... ");
    
    SphereThreadingModel* model = sphere_threading_init(1200); // 100 per group
    assert(model != NULL);
    
    // Count spheres in each symmetry group
    int counts[12] = {0};
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        uint32_t group = sphere_get_symmetry_group(i);
        assert(group < 12);
        counts[group]++;
    }
    
    // Verify even distribution
    for (int i = 0; i < 12; i++) {
        assert(counts[i] == 100);
    }
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 4: Verify neighbor validity
static int test_neighbor_validity(void) {
    printf("Test 4: Verify neighbor validity... ");
    
    SphereThreadingModel* model = sphere_threading_init(500);
    assert(model != NULL);
    
    // Verify all neighbors are valid
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        for (uint32_t j = 0; j < SPHERE_NUM_NEIGHBORS; j++) {
            uint32_t neighbor = model->spheres[i].neighbors[j];
            assert(neighbor < model->num_spheres);
        }
    }
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 5: Test get neighbors function
static int test_get_neighbors(void) {
    printf("Test 5: Test get neighbors function... ");
    
    SphereThreadingModel* model = sphere_threading_init(100);
    assert(model != NULL);
    
    uint32_t neighbors[SPHERE_NUM_NEIGHBORS];
    int result = sphere_get_neighbors(model, 0, neighbors);
    
    assert(result == SPHERE_NUM_NEIGHBORS);
    
    // Verify neighbors match
    for (int i = 0; i < SPHERE_NUM_NEIGHBORS; i++) {
        assert(neighbors[i] == model->spheres[0].neighbors[i]);
    }
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 6: Test cache optimization
static int test_cache_optimization(void) {
    printf("Test 6: Test cache optimization... ");
    
    SphereThreadingModel* model = sphere_threading_init(500);
    assert(model != NULL);
    assert(model->cache_optimized == 0);
    
    int result = sphere_optimize_cache_locality(model, 64 * sizeof(float));
    assert(result == 0);
    assert(model->cache_optimized == 1);
    
    // Verify neighbors are still valid after optimization
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        for (uint32_t j = 0; j < SPHERE_NUM_NEIGHBORS; j++) {
            uint32_t neighbor = model->spheres[i].neighbors[j];
            assert(neighbor < model->num_spheres);
        }
    }
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 7: Test model verification
static int test_verify_model(void) {
    printf("Test 7: Test model verification... ");
    
    SphereThreadingModel* model = sphere_threading_init(500);
    assert(model != NULL);
    
    int valid = sphere_verify_model(model);
    assert(valid == 1);
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 8: Test sphere statistics
static int test_sphere_statistics(void) {
    printf("Test 8: Test sphere statistics... ");
    
    SphereThreadingModel* model = sphere_threading_init(100);
    assert(model != NULL);
    
    SphereStatistics stats;
    int result = sphere_get_statistics(model, 0, &stats);
    
    assert(result == 0);
    assert(stats.sphere_id == 0);
    assert(stats.num_neighbors == 12);
    assert(stats.symmetry_group == 0);
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 9: Test system statistics
static int test_system_statistics(void) {
    printf("Test 9: Test system statistics... ");
    
    SphereThreadingModel* model = sphere_threading_init(240); // 20 per group
    assert(model != NULL);
    
    SystemStatistics stats;
    int result = sphere_get_system_statistics(model, &stats);
    
    assert(result == 0);
    assert(stats.num_spheres == 240);
    assert(stats.load_balance_score > 0.95); // Should be well-balanced
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 10: Test work assignment
static int test_work_assignment(void) {
    printf("Test 10: Test work assignment... ");
    
    SphereThreadingModel* model = sphere_threading_init(100);
    assert(model != NULL);
    
    // Create test work units
    int work_units[1000];
    for (int i = 0; i < 1000; i++) {
        work_units[i] = i;
    }
    
    int result = sphere_assign_work(model, work_units, 1000, sizeof(int));
    assert(result == 0);
    assert(model->total_work_units == 1000);
    
    // Verify work was distributed
    uint32_t total_assigned = 0;
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        if (model->spheres[i].work_data != NULL) {
            total_assigned += model->spheres[i].work_size / sizeof(int);
        }
    }
    assert(total_assigned == 1000);
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 11: Test symmetry group calculation
static int test_symmetry_group(void) {
    printf("Test 11: Test symmetry group calculation... ");
    
    for (uint32_t i = 0; i < 100; i++) {
        uint32_t group = sphere_get_symmetry_group(i);
        assert(group == i % 12);
        assert(group < 12);
    }
    
    printf("PASSED\n");
    return 1;
}

// Test 12: Test neighbor geometry
static int test_neighbor_geometry(void) {
    printf("Test 12: Test neighbor geometry... ");
    
    // Test neighbor finding for various sphere counts
    for (uint32_t num_spheres = 100; num_spheres <= 1000; num_spheres += 100) {
        for (uint32_t sphere_id = 0; sphere_id < 10; sphere_id++) {
            for (uint32_t group = 0; group < 12; group++) {
                uint32_t neighbor = sphere_find_neighbor_by_geometry(
                    num_spheres, sphere_id, group
                );
                assert(neighbor < num_spheres);
            }
        }
    }
    
    printf("PASSED\n");
    return 1;
}

// Test 13: Test edge cases - small models
static int test_edge_cases_small(void) {
    printf("Test 13: Test edge cases - small models... ");
    
    // Test with very small number of spheres
    SphereThreadingModel* model = sphere_threading_init(12);
    assert(model != NULL);
    assert(model->num_spheres == 12);
    
    int valid = sphere_verify_model(model);
    assert(valid == 1);
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 14: Test edge cases - large models
static int test_edge_cases_large(void) {
    printf("Test 14: Test edge cases - large models... ");
    
    // Test with large number of spheres
    SphereThreadingModel* model = sphere_threading_init(10000);
    assert(model != NULL);
    assert(model->num_spheres == 10000);
    
    int valid = sphere_verify_model(model);
    assert(valid == 1);
    
    sphere_threading_free(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 15: Test NULL pointer handling
static int test_null_pointers(void) {
    printf("Test 15: Test NULL pointer handling... ");
    
    // These should not crash
    sphere_threading_free(NULL);
    
    int result = sphere_get_neighbors(NULL, 0, NULL);
    assert(result == -1);
    
    result = sphere_optimize_cache_locality(NULL, 0);
    assert(result == -1);
    
    result = sphere_assign_work(NULL, NULL, 0, 0);
    assert(result == -1);
    
    result = sphere_process_work(NULL, NULL, NULL);
    assert(result == -1);
    
    result = sphere_get_statistics(NULL, 0, NULL);
    assert(result == -1);
    
    result = sphere_get_system_statistics(NULL, NULL);
    assert(result == -1);
    
    int valid = sphere_verify_model(NULL);
    assert(valid == 0);
    
    printf("PASSED\n");
    return 1;
}

int main(void) {
    printf("\n=== Sphere Threading Test Suite (Algorithms Layer) ===\n\n");
    
    int passed = 0;
    int total = 15;
    
    passed += test_init_sphere_threading();
    passed += test_12_neighbors();
    passed += test_12_fold_symmetry();
    passed += test_neighbor_validity();
    passed += test_get_neighbors();
    passed += test_cache_optimization();
    passed += test_verify_model();
    passed += test_sphere_statistics();
    passed += test_system_statistics();
    passed += test_work_assignment();
    passed += test_symmetry_group();
    passed += test_neighbor_geometry();
    passed += test_edge_cases_small();
    passed += test_edge_cases_large();
    passed += test_null_pointers();
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    if (passed == total) {
        printf("✅ All tests passed!\n\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n\n");
        return 1;
    }
}
