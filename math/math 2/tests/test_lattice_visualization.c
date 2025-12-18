/**
 * Test Suite for Kissing Spheres Visualization
 * 
 * Comprehensive tests for all visualization and debugging functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cllm.h"
#include "cllm_lattice_visualization.h"
#include "ai/cllm_lattice.h"

// Test helper: Create a minimal test model
static CLLMModel* create_test_model(size_t num_points) {
    CLLMModel* model = calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    model->num_lattice_points = num_points;
    model->lattice_points = calloc(num_points, sizeof(CLLMLatticePoint));
    
    if (!model->lattice_points) {
        free(model);
        return NULL;
    }
    
    // Initialize lattice points with test data
    for (size_t i = 0; i < num_points; i++) {
        model->lattice_points[i].point_id = i;
        model->lattice_points[i].prime = (i + 1) * 2 + 1; // Simple prime-like values
        model->lattice_points[i].num_neighbors = 12;
        model->lattice_points[i].symmetry_group = i % 12;
        
        // Set up neighbors (simple pattern for testing)
        for (int j = 0; j < 12; j++) {
            model->lattice_points[i].neighbors[j] = (i + j + 1) % num_points;
        }
    }
    
    return model;
}

static void free_test_model(CLLMModel* model) {
    if (model) {
        if (model->lattice_points) {
            free(model->lattice_points);
        }
        free(model);
    }
}

// Test 1: Visualization config defaults
static int test_visualization_config_default(void) {
    printf("Test 1: Visualization config defaults... ");
    
    KissingSphereVisualizationConfig config = cllm_visualization_config_default();
    
    assert(config.show_neighbors == 1);
    assert(config.show_statistics == 1);
    assert(config.show_symmetry_groups == 1);
    assert(config.show_cache_info == 1);
    assert(config.show_load_balance == 1);
    assert(config.verbose == 0);
    
    printf("PASSED\n");
    return 1;
}

// Test 2: Point statistics collection
static int test_collect_point_statistics(void) {
    printf("Test 2: Point statistics collection... ");
    
    CLLMModel* model = create_test_model(100);
    assert(model != NULL);
    
    LatticePointStatistics stats = cllm_collect_point_statistics(model, 0);
    
    assert(stats.point_id == 0);
    assert(stats.num_neighbors == 12);
    assert(stats.symmetry_group == 0);
    assert(stats.avg_neighbor_distance > 0.0);
    assert(stats.cache_hit_rate > 0.0 && stats.cache_hit_rate <= 1.0);
    
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 3: System statistics collection
static int test_collect_system_statistics(void) {
    printf("Test 3: System statistics collection... ");
    
    CLLMModel* model = create_test_model(120); // 10 points per symmetry group
    assert(model != NULL);
    
    SystemStatistics stats = cllm_collect_system_statistics(model);
    
    assert(stats.total_points == 120);
    assert(stats.total_connections == 120 * 12);
    assert(stats.avg_neighbors_per_point == 12.0);
    assert(stats.load_balance_score > 0.0 && stats.load_balance_score <= 1.0);
    assert(stats.cache_efficiency > 0.0 && stats.cache_efficiency <= 1.0);
    
    // Check symmetry distribution
    double total_distribution = 0.0;
    for (int i = 0; i < 12; i++) {
        total_distribution += stats.symmetry_distribution[i];
    }
    assert(total_distribution > 0.99 && total_distribution < 1.01); // Should sum to ~1.0
    
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 4: All points statistics collection
static int test_collect_all_point_statistics(void) {
    printf("Test 4: All points statistics collection... ");
    
    CLLMModel* model = create_test_model(50);
    assert(model != NULL);
    
    int num_stats = 0;
    LatticePointStatistics* stats = cllm_collect_all_point_statistics(model, &num_stats);
    
    assert(stats != NULL);
    assert(num_stats == 50);
    
    // Verify each stat
    for (int i = 0; i < num_stats; i++) {
        assert(stats[i].point_id == i);
        assert(stats[i].num_neighbors == 12);
        assert(stats[i].symmetry_group >= 0 && stats[i].symmetry_group < 12);
    }
    
    free(stats);
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 5: Lattice structure verification
static int test_verify_lattice_structure(void) {
    printf("Test 5: Lattice structure verification... ");
    
    CLLMModel* model = create_test_model(100);
    assert(model != NULL);
    
    FILE* output = fopen("/dev/null", "w");
    assert(output != NULL);
    
    int result = cllm_verify_lattice_structure(model, output);
    assert(result == 1); // Should pass verification
    
    fclose(output);
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 6: Cache efficiency estimation
static int test_estimate_cache_efficiency(void) {
    printf("Test 6: Cache efficiency estimation... ");
    
    CLLMModel* model = create_test_model(100);
    assert(model != NULL);
    
    double efficiency = cllm_estimate_cache_efficiency(model);
    
    assert(efficiency > 0.0 && efficiency <= 1.0);
    
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 7: Load balance score calculation
static int test_calculate_load_balance_score(void) {
    printf("Test 7: Load balance score calculation... ");
    
    // Test with perfectly balanced distribution
    CLLMModel* model = create_test_model(120); // 10 per group
    assert(model != NULL);
    
    double score = cllm_calculate_load_balance_score(model);
    
    assert(score > 0.95); // Should be very high for balanced distribution
    
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 8: Symmetry group calculation
static int test_get_point_symmetry_group(void) {
    printf("Test 8: Symmetry group calculation... ");
    
    for (int i = 0; i < 100; i++) {
        int group = cllm_get_point_symmetry_group(i);
        assert(group >= 0 && group < 12);
        assert(group == i % 12);
    }
    
    printf("PASSED\n");
    return 1;
}

// Test 9: JSON export
static int test_export_visualization_json(void) {
    printf("Test 9: JSON export... ");
    
    CLLMModel* model = create_test_model(24); // 2 per group
    assert(model != NULL);
    
    const char* filename = "/tmp/test_visualization.json";
    cllm_export_visualization_json(model, filename);
    
    // Verify file was created
    FILE* f = fopen(filename, "r");
    assert(f != NULL);
    
    // Read and verify basic structure
    char buffer[1024];
    int found_num_points = 0;
    while (fgets(buffer, sizeof(buffer), f)) {
        if (strstr(buffer, "&quot;num_points&quot;")) {
            found_num_points = 1;
        }
    }
    assert(found_num_points);
    
    fclose(f);
    remove(filename);
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 10: Print functions (basic smoke test)
static int test_print_functions(void) {
    printf("Test 10: Print functions... ");
    
    CLLMModel* model = create_test_model(50);
    assert(model != NULL);
    
    FILE* output = fopen("/dev/null", "w");
    assert(output != NULL);
    
    // Test all print functions
    cllm_print_point_info(model, 0, output);
    cllm_print_point_neighbors(model, 0, output);
    
    LatticePointStatistics stats = cllm_collect_point_statistics(model, 0);
    cllm_print_point_statistics(&stats, output);
    
    SystemStatistics sys_stats = cllm_collect_system_statistics(model);
    cllm_print_system_statistics(&sys_stats, output);
    
    cllm_visualize_neighbor_connections(model, 0, output);
    cllm_visualize_symmetry_distribution(model, output);
    cllm_visualize_load_balance(model, output);
    
    KissingSphereVisualizationConfig config = cllm_visualization_config_default();
    cllm_print_visualization_report(model, &config, output);
    
    cllm_debug_check_lattice(model, output);
    cllm_debug_print_memory_layout(model, output);
    
    fclose(output);
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

// Test 11: Edge cases - NULL pointers
static int test_null_pointer_handling(void) {
    printf("Test 11: NULL pointer handling... ");
    
    FILE* output = fopen("/dev/null", "w");
    assert(output != NULL);
    
    // These should not crash
    cllm_print_point_info(NULL, 0, output);
    cllm_print_point_neighbors(NULL, 0, output);
    cllm_print_point_statistics(NULL, output);
    cllm_print_system_statistics(NULL, output);
    cllm_visualize_neighbor_connections(NULL, 0, output);
    cllm_visualize_symmetry_distribution(NULL, output);
    cllm_visualize_load_balance(NULL, output);
    cllm_print_visualization_report(NULL, NULL, output);
    cllm_export_visualization_json(NULL, NULL);
    cllm_verify_lattice_structure(NULL, output);
    cllm_debug_check_lattice(NULL, output);
    cllm_debug_print_memory_layout(NULL, output);
    
    double eff = cllm_estimate_cache_efficiency(NULL);
    assert(eff == 0.0);
    
    double score = cllm_calculate_load_balance_score(NULL);
    assert(score == 0.0);
    
    int num_stats = 0;
    LatticePointStatistics* stats = cllm_collect_all_point_statistics(NULL, &num_stats);
    assert(stats == NULL);
    assert(num_stats == 0);
    
    fclose(output);
    
    printf("PASSED\n");
    return 1;
}

// Test 12: Edge cases - Invalid indices
static int test_invalid_indices(void) {
    printf("Test 12: Invalid indices... ");
    
    CLLMModel* model = create_test_model(50);
    assert(model != NULL);
    
    FILE* output = fopen("/dev/null", "w");
    assert(output != NULL);
    
    // These should handle invalid indices gracefully
    cllm_print_point_info(model, -1, output);
    cllm_print_point_info(model, 1000, output);
    cllm_print_point_neighbors(model, -1, output);
    cllm_print_point_neighbors(model, 1000, output);
    
    LatticePointStatistics stats = cllm_collect_point_statistics(model, -1);
    assert(stats.point_id == 0); // Should return zeroed struct
    
    stats = cllm_collect_point_statistics(model, 1000);
    assert(stats.point_id == 0);
    
    fclose(output);
    free_test_model(model);
    
    printf("PASSED\n");
    return 1;
}

int main(void) {
    printf("\n=== Kissing Spheres Visualization Test Suite ===\n\n");
    
    int passed = 0;
    int total = 12;
    
    passed += test_visualization_config_default();
    passed += test_collect_point_statistics();
    passed += test_collect_system_statistics();
    passed += test_collect_all_point_statistics();
    passed += test_verify_lattice_structure();
    passed += test_estimate_cache_efficiency();
    passed += test_calculate_load_balance_score();
    passed += test_get_point_symmetry_group();
    passed += test_export_visualization_json();
    passed += test_print_functions();
    passed += test_null_pointer_handling();
    passed += test_invalid_indices();
    
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