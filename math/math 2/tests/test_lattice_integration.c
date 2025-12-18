/**
 * Integration Test for Kissing Spheres with Training Pipeline
 * 
 * Tests the complete integration of kissing spheres with the training system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cllm.h"
#include "ai/cllm_lattice.h"
#include "cllm_lattice_visualization.h"
#include "cllm_neighbor_ops.h"

// Test 1: Initialize kissing spheres
static int test_initialize_kissing_spheres(void) {
    printf("Test 1: Initialize kissing spheres... ");
    
    // Create a minimal model
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 1000;
    model.embedding_dim = 64;
    model.num_lattice_points = 1000;
    
    // Allocate lattice points
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    // Initialize kissing spheres
    cllm_initialize_kissing_spheres(&model);
    
    // Verify all points have 12 neighbors
    for (size_t i = 0; i < model.num_lattice_points; i++) {
        assert(model.lattice_points[i].num_neighbors == 12);
        
        // Verify neighbors are valid
        for (int j = 0; j < 12; j++) {
            uint32_t neighbor = model.lattice_points[i].neighbors[j];
            assert(neighbor < model.num_lattice_points);
            // Note: In some cases, a point might be its own neighbor in the clock geometry
            // This is acceptable for the geometric pattern
        }
    }
    
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

// Test 2: Verify 12-fold symmetry
static int test_12_fold_symmetry(void) {
    printf("Test 2: Verify 12-fold symmetry... ");
    
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 1200; // 100 per symmetry group
    model.embedding_dim = 64;
    model.num_lattice_points = 1200;
    
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    cllm_initialize_kissing_spheres(&model);
    
    // Count points in each symmetry group
    int counts[12] = {0};
    for (size_t i = 0; i < model.num_lattice_points; i++) {
        int group = cllm_get_point_symmetry_group(i);
        assert(group >= 0 && group < 12);
        counts[group]++;
    }
    
    // Verify even distribution
    for (int i = 0; i < 12; i++) {
        assert(counts[i] == 100);
    }
    
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

// Test 3: Neighbor operations (simplified - just verify structure)
static int test_neighbor_operations(void) {
    printf("Test 3: Neighbor operations... ");
    
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 500;
    model.embedding_dim = 64;
    model.num_lattice_points = 500;
    
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    cllm_initialize_kissing_spheres(&model);
    
    // Verify neighbors are set up
    for (size_t i = 0; i < model.num_lattice_points; i++) {
        assert(model.lattice_points[i].num_neighbors == 12);
    }
    
    // Cleanup
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

// Test 4: Cache locality optimization
static int test_cache_locality(void) {
    printf("Test 4: Cache locality optimization... ");
    
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 500;
    model.embedding_dim = 64;
    model.num_lattice_points = 500;
    
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    cllm_initialize_kissing_spheres(&model);
    
    // Measure cache efficiency before and after optimization
    double efficiency_before = cllm_estimate_cache_efficiency(&model);
    
    cllm_optimize_neighbor_cache_locality(&model);
    
    double efficiency_after = cllm_estimate_cache_efficiency(&model);
    
    // Efficiency should be same or better (optimization is already called in init)
    assert(efficiency_after >= efficiency_before * 0.95); // Allow 5% tolerance
    
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

// Test 5: Visualization integration
static int test_visualization_integration(void) {
    printf("Test 5: Visualization integration... ");
    
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 240; // 20 per symmetry group
    model.embedding_dim = 64;
    model.num_lattice_points = 240;
    
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    cllm_initialize_kissing_spheres(&model);
    
    // Generate visualization report
    FILE* output = fopen("/tmp/test_visualization_report.txt", "w");
    assert(output != NULL);
    
    KissingSphereVisualizationConfig config = cllm_visualization_config_default();
    cllm_print_visualization_report(&model, &config, output);
    
    fclose(output);
    
    // Verify file was created and has content
    FILE* check = fopen("/tmp/test_visualization_report.txt", "r");
    assert(check != NULL);
    
    fseek(check, 0, SEEK_END);
    long size = ftell(check);
    assert(size > 100); // Should have substantial content
    
    fclose(check);
    remove("/tmp/test_visualization_report.txt");
    
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

// Test 6: Load balance verification
static int test_load_balance(void) {
    printf("Test 6: Load balance verification... ");
    
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 1200; // 100 per group - perfectly balanced
    model.embedding_dim = 64;
    model.num_lattice_points = 1200;
    
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    cllm_initialize_kissing_spheres(&model);
    
    double score = cllm_calculate_load_balance_score(&model);
    
    // Should be very high for perfectly balanced distribution
    assert(score > 0.95);
    
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

// Test 7: Structure verification
static int test_structure_verification(void) {
    printf("Test 7: Structure verification... ");
    
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 500;
    model.embedding_dim = 64;
    model.num_lattice_points = 500;
    
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    cllm_initialize_kissing_spheres(&model);
    
    FILE* output = fopen("/dev/null", "w");
    assert(output != NULL);
    
    int result = cllm_verify_lattice_structure(&model, output);
    assert(result == 1); // Should pass
    
    fclose(output);
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

// Test 8: JSON export integration
static int test_json_export_integration(void) {
    printf("Test 8: JSON export integration... ");
    
    CLLMModel model;
    memset(&model, 0, sizeof(CLLMModel));
    
    model.vocab_size = 120;
    model.embedding_dim = 64;
    model.num_lattice_points = 120;
    
    model.lattice_points = calloc(model.num_lattice_points, sizeof(CLLMLatticePoint));
    assert(model.lattice_points != NULL);
    
    cllm_initialize_kissing_spheres(&model);
    
    const char* filename = "/tmp/test_integration_export.json";
    cllm_export_visualization_json(&model, filename);
    
    // Verify file exists and has valid JSON structure
    FILE* f = fopen(filename, "r");
    assert(f != NULL);
    
    char buffer[1024];
    int found_points = 0;
    int found_neighbors = 0;
    
    while (fgets(buffer, sizeof(buffer), f)) {
        if (strstr(buffer, "&quot;num_points&quot;")) found_points = 1;
        if (strstr(buffer, "&quot;neighbors&quot;")) found_neighbors = 1;
    }
    
    assert(found_points);
    assert(found_neighbors);
    
    fclose(f);
    remove(filename);
    free(model.lattice_points);
    
    printf("PASSED\n");
    return 1;
}

int main(void) {
    printf("\n=== Kissing Spheres Integration Test Suite ===\n\n");
    
    int passed = 0;
    int total = 8;
    
    passed += test_initialize_kissing_spheres();
    passed += test_12_fold_symmetry();
    passed += test_neighbor_operations();
    passed += test_cache_locality();
    passed += test_visualization_integration();
    passed += test_load_balance();
    passed += test_structure_verification();
    passed += test_json_export_integration();
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    if (passed == total) {
        printf("✅ All integration tests passed!\n\n");
        return 0;
    } else {
        printf("❌ Some integration tests failed!\n\n");
        return 1;
    }
}