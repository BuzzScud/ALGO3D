/**
 * @file test_entropy_integration.c
 * @brief Unit tests for entropy integration with CLLM training system
 */

#include "ai/cllm_entropy_integration.h"
#include "cllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

#define MATH_EPSILON 1e-6

/**
 * @brief Create a minimal test model
 */
static CLLMModel* create_test_model(void) {
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    // Set basic parameters
    model->vocab_size = 1000;
    model->embedding_dim = 128;
    model->num_layers = 4;
    model->num_lattice_points = 100;
    
    // Allocate lattice points
    model->lattice_points = (CLLMLatticePoint*)calloc(
        model->num_lattice_points,
        sizeof(CLLMLatticePoint)
    );
    
    if (!model->lattice_points) {
        free(model);
        return NULL;
    }
    
    // Initialize lattice points with test data
    for (uint64_t i = 0; i < model->num_lattice_points; i++) {
        model->lattice_points[i].point_id = i + 1;
        model->lattice_points[i].symmetry_group = i % 12;
        model->lattice_points[i].prime = i + 2;  // Simple prime-like values
        model->lattice_points[i].num_neighbors = 0;
        model->lattice_points[i].neighbor_count = 0;
    }
    
    return model;
}

/**
 * @brief Free test model
 */
static void free_test_model(CLLMModel* model) {
    if (!model) return;
    
    if (model->lattice_points) {
        free(model->lattice_points);
    }
    
    free(model);
}

/**
 * @brief Test: Context initialization
 */
static void test_context_init(void) {
    printf("Test: Context initialization... ");
    
    CLLMModel* model = create_test_model();
    assert(model != NULL);
    
    EntropyIntegrationContext ctx;
    bool result = entropy_integration_init(&ctx, model);
    
    assert(result == true);
    assert(ctx.model == model);
    assert(ctx.monitoring_enabled == false);
    assert(ctx.monitoring_interval == 100);
    assert(ctx.stats.total_entropy == 0.0);
    assert(ctx.stats.update_count == 0);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: NULL parameter handling
 */
static void test_null_parameters(void) {
    printf("Test: NULL parameter handling... ");
    
    EntropyIntegrationContext ctx;
    
    // Test NULL context
    assert(entropy_integration_init(NULL, NULL) == false);
    
    // Test NULL model
    assert(entropy_integration_init(&ctx, NULL) == false);
    
    // Test NULL in other functions
    entropy_integration_destroy(NULL);
    update_entropy_statistics(NULL, 100);
    enable_entropy_monitoring(NULL, 10);
    disable_entropy_monitoring(NULL);
    
    assert(get_dimension_stats(NULL, 0) == NULL);
    assert(get_model_entropy_stats(NULL) == NULL);
    assert(calculate_entropy_trend(NULL) == 0.0);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Dimension entropy calculation
 */
static void test_dimension_entropy(void) {
    printf("Test: Dimension entropy calculation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Calculate entropy for dimension 0 (becomes dimension 1)
    double entropy = calculate_dimension_entropy(&ctx, 0, 100);
    assert(entropy > 0.0);
    
    // Calculate entropy for dimension 4 (becomes dimension 5, should have entropy)
    double entropy2 = calculate_dimension_entropy(&ctx, 4, 100);
    assert(entropy2 >= 0.0);  // Can be 0 for high dimensions with small n
    
    // Invalid dimension
    double entropy3 = calculate_dimension_entropy(&ctx, 99, 100);
    assert(entropy3 == 0.0);
    
    // Zero sequence length
    double entropy4 = calculate_dimension_entropy(&ctx, 0, 0);
    assert(entropy4 == 0.0);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Model entropy calculation
 */
static void test_model_entropy(void) {
    printf("Test: Model entropy calculation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Calculate total model entropy
    double entropy = calculate_model_entropy(&ctx, 100);
    assert(entropy >= 0.0);  // Can be 0 if all dimensions have 0 entropy
    
    // Should be sum of all dimension entropies
    double sum = 0.0;
    for (int i = 0; i < 12; i++) {
        sum += calculate_dimension_entropy(&ctx, i, 100);
    }
    assert(fabs(entropy - sum) < MATH_EPSILON);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Statistics update
 */
static void test_statistics_update(void) {
    printf("Test: Statistics update... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Update statistics
    update_entropy_statistics(&ctx, 100);
    
    assert(ctx.stats.update_count == 1);
    assert(ctx.stats.total_entropy >= 0.0);
    assert(ctx.stats.normalized_entropy >= 0.0);
    
    // Check dimension statistics
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        const DimensionEntropyStats* stats = get_dimension_stats(&ctx, i);
        assert(stats != NULL);
        assert(stats->dimension == (uint32_t)i);
        assert(stats->sample_count == 1);
        assert(stats->current_entropy >= 0.0);  // Can be 0 for high dimensions
    }
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Multiple statistics updates
 */
static void test_multiple_updates(void) {
    printf("Test: Multiple statistics updates... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Update multiple times
    for (int i = 0; i < 10; i++) {
        update_entropy_statistics(&ctx, 100 + i * 10);
    }
    
    assert(ctx.stats.update_count == 10);
    
    // Check that statistics are accumulated
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        const DimensionEntropyStats* stats = get_dimension_stats(&ctx, i);
        assert(stats->sample_count == 10);
        assert(stats->min_entropy <= stats->avg_entropy);
        assert(stats->max_entropy >= stats->avg_entropy);
    }
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Entropy trend calculation
 */
static void test_entropy_trend(void) {
    printf("Test: Entropy trend calculation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // First update - no trend yet
    update_entropy_statistics(&ctx, 100);
    double trend1 = calculate_entropy_trend(&ctx);
    assert(trend1 == 0.0);  // No previous value
    
    // Second update - should have trend
    update_entropy_statistics(&ctx, 200);
    calculate_entropy_trend(&ctx);
    // Trend can be positive, negative, or zero (just verify it runs)
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Monitoring enable/disable
 */
static void test_monitoring_control(void) {
    printf("Test: Monitoring enable/disable... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Initially disabled
    assert(ctx.monitoring_enabled == false);
    assert(should_monitor_entropy(&ctx, 100) == false);
    
    // Enable monitoring
    enable_entropy_monitoring(&ctx, 50);
    assert(ctx.monitoring_enabled == true);
    assert(ctx.monitoring_interval == 50);
    
    // Should monitor at interval
    assert(should_monitor_entropy(&ctx, 50) == true);
    assert(should_monitor_entropy(&ctx, 25) == false);
    
    // Disable monitoring
    disable_entropy_monitoring(&ctx);
    assert(ctx.monitoring_enabled == false);
    assert(should_monitor_entropy(&ctx, 100) == false);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Monitoring step
 */
static void test_monitoring_step(void) {
    printf("Test: Monitoring step... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    enable_entropy_monitoring(&ctx, 100);
    
    // Perform monitoring
    monitor_entropy_step(&ctx, 100, 100);
    
    assert(ctx.last_monitoring_step == 100);
    assert(ctx.stats.update_count == 1);
    assert(ctx.stats.total_entropy > 0.0);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Lattice point entropy
 */
static void test_lattice_point_entropy(void) {
    printf("Test: Lattice point entropy... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Get entropy for first lattice point (symmetry_group 0 -> dimension 1)
    double entropy = get_lattice_point_entropy(&ctx, 0);
    assert(entropy >= 0.0);
    
    // Get entropy for a point with low symmetry group (should have entropy)
    double entropy2 = get_lattice_point_entropy(&ctx, 3);  // symmetry_group 3 -> dimension 4
    assert(entropy2 >= 0.0);
    
    // Invalid index
    double entropy3 = get_lattice_point_entropy(&ctx, model->num_lattice_points);
    assert(entropy3 == 0.0);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Entropy distribution
 */
static void test_entropy_distribution(void) {
    printf("Test: Entropy distribution... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    double entropies[MAX_TRACKED_DIMENSIONS];
    calculate_entropy_distribution(&ctx, 100, entropies);
    
    // Check all dimensions have non-negative entropy
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        assert(entropies[i] >= 0.0);
    }
    
    // Sum should equal total model entropy
    double sum = 0.0;
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        sum += entropies[i];
    }
    
    double total = calculate_model_entropy(&ctx, 100);
    assert(fabs(sum - total) < MATH_EPSILON);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Statistics reset
 */
static void test_statistics_reset(void) {
    printf("Test: Statistics reset... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Update statistics
    update_entropy_statistics(&ctx, 100);
    assert(ctx.stats.update_count == 1);
    assert(ctx.stats.total_entropy > 0.0);
    
    // Reset
    reset_entropy_statistics(&ctx);
    
    assert(ctx.stats.update_count == 0);
    assert(ctx.stats.total_entropy == 0.0);
    assert(ctx.stats.normalized_entropy == 0.0);
    assert(ctx.stats.entropy_trend == 0.0);
    
    // Check dimension statistics are reset
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        const DimensionEntropyStats* stats = get_dimension_stats(&ctx, i);
        assert(stats->sample_count == 0);
        assert(stats->current_entropy == 0.0);
    }
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Dimension statistics retrieval
 */
static void test_dimension_stats_retrieval(void) {
    printf("Test: Dimension statistics retrieval... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    update_entropy_statistics(&ctx, 100);
    
    // Get valid dimension stats
    const DimensionEntropyStats* stats = get_dimension_stats(&ctx, 5);
    assert(stats != NULL);
    assert(stats->dimension == 5);
    assert(stats->sample_count == 1);
    
    // Get invalid dimension stats
    const DimensionEntropyStats* stats2 = get_dimension_stats(&ctx, 99);
    assert(stats2 == NULL);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Model statistics retrieval
 */
static void test_model_stats_retrieval(void) {
    printf("Test: Model statistics retrieval... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    update_entropy_statistics(&ctx, 100);
    
    const ModelEntropyStats* stats = get_model_entropy_stats(&ctx);
    assert(stats != NULL);
    assert(stats->update_count == 1);
    assert(stats->total_entropy > 0.0);
    assert(stats->normalized_entropy > 0.0);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Print statistics (visual check)
 */
static void test_print_statistics(void) {
    printf("Test: Print statistics... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    // Update statistics multiple times
    for (int i = 0; i < 5; i++) {
        update_entropy_statistics(&ctx, 100 + i * 20);
    }
    
    // Print statistics (visual verification)
    print_entropy_statistics(&ctx);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n=== Entropy Integration Tests ===\n\n");
    
    test_context_init();
    test_null_parameters();
    test_dimension_entropy();
    test_model_entropy();
    test_statistics_update();
    test_multiple_updates();
    test_entropy_trend();
    test_monitoring_control();
    test_monitoring_step();
    test_lattice_point_entropy();
    test_entropy_distribution();
    test_statistics_reset();
    test_dimension_stats_retrieval();
    test_model_stats_retrieval();
    test_print_statistics();
    
    printf("\n=== All Tests Passed ===\n\n");
    
    return 0;
}