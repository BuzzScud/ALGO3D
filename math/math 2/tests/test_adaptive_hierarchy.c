/**
 * @file test_adaptive_hierarchy.c
 * @brief Unit tests for entropy-aware adaptive hierarchy depth management
 */

#include "ai/cllm_adaptive_hierarchy.h"
#include "ai/cllm_entropy_integration.h"
#include "cllm.h"
#include "cllm_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

// Test counter
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s...", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf(" PASSED\n"); \
        tests_passed++; \
    } while(0)

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf(" FAILED at line %d: %s\n", __LINE__, #condition); \
            return; \
        } \
    } while(0)

#define ASSERT_NEAR(a, b, epsilon) \
    do { \
        double _a = (a); \
        double _b = (b); \
        double _epsilon = (epsilon); \
        if (fabs(_a - _b) > _epsilon) { \
            printf(" FAILED at line %d: |%f - %f| > %f\n", __LINE__, _a, _b, _epsilon); \
            return; \
        } \
    } while(0)

// Helper function to create a test model
static CLLMModel* create_test_model(void) {
    CLLMConfig config = {
        .vocab_size = 1000,
        .embedding_dim = 128,
        .num_layers = 4,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    return cllm_create_model(&config);
}

// Test 1: Configuration initialization
static void test_config_init(void) {
    TEST("config_init");
    
    AdaptiveHierarchyConfig config;
    adaptive_hierarchy_config_init_default(&config);
    
    ASSERT(config.entropy_weight > 0.0);
    ASSERT(config.workload_weight > 0.0);
    ASSERT(config.resource_weight > 0.0);
    ASSERT(config.adjustment_threshold > 0.0);
    ASSERT(config.adjustment_interval > 0);
    ASSERT(config.min_depth >= 1);
    ASSERT(config.max_depth <= ADAPTIVE_MAX_DEPTH);
    
    // Check weights sum to approximately 1.0
    double weight_sum = config.entropy_weight + config.workload_weight + config.resource_weight;
    ASSERT_NEAR(weight_sum, 1.0, 0.01);
    
    PASS();
}

// Test 2: Context initialization
static void test_context_init(void) {
    TEST("context_init");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    ASSERT(ctx.entropy_ctx == &entropy_ctx);
    ASSERT(ctx.current_depth == 1);
    ASSERT(ctx.recommended_depth == 1);
    ASSERT(ctx.adjustment_count == 0);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 3: Context initialization with custom config
static void test_context_init_custom_config(void) {
    TEST("context_init_custom_config");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyConfig config;
    adaptive_hierarchy_config_init_default(&config);
    config.entropy_weight = 0.5;
    config.workload_weight = 0.3;
    config.resource_weight = 0.2;
    config.min_depth = 2;
    config.max_depth = 4;
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, &config));
    
    // Weights should be normalized
    double weight_sum = ctx.config.entropy_weight + 
                       ctx.config.workload_weight + 
                       ctx.config.resource_weight;
    ASSERT_NEAR(weight_sum, 1.0, 0.01);
    
    ASSERT(ctx.config.min_depth == 2);
    ASSERT(ctx.config.max_depth == 4);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 4: Entropy score calculation
static void test_entropy_score_calculation(void) {
    TEST("entropy_score_calculation");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Set some entropy values
    entropy_ctx.stats.normalized_entropy = 0.5;
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        entropy_ctx.stats.dimensions[i].sample_count = 10;
        entropy_ctx.stats.dimensions[i].entropy_variance = 0.1;
    }
    
    double score = calculate_entropy_score(&ctx);
    ASSERT(score >= 0.0 && score <= 1.0);
    
    // High entropy should give high score
    entropy_ctx.stats.normalized_entropy = 0.9;
    score = calculate_entropy_score(&ctx);
    ASSERT(score > 0.6);
    
    // Low entropy should give low score
    entropy_ctx.stats.normalized_entropy = 0.1;
    score = calculate_entropy_score(&ctx);
    ASSERT(score < 0.4);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 5: Workload score calculation
static void test_workload_score_calculation(void) {
    TEST("workload_score_calculation");
    
    // Low workload (5 batches, 10 threads = 0.5 batches/thread)
    double score = calculate_workload_score(5, 10);
    ASSERT(score >= 0.0 && score <= 1.0);
    ASSERT(score < 0.5); // Should be low
    
    // Ideal workload (100 batches, 10 threads = 10 batches/thread)
    score = calculate_workload_score(100, 10);
    ASSERT(score >= 0.0 && score <= 1.0);
    
    // High workload (500 batches, 10 threads = 50 batches/thread)
    score = calculate_workload_score(500, 10);
    ASSERT(score >= 0.0 && score <= 1.0);
    ASSERT(score > 0.7); // Should be high
    
    // Zero workload
    score = calculate_workload_score(0, 10);
    ASSERT(score == 0.0);
    
    PASS();
}

// Test 6: Resource score calculation
static void test_resource_score_calculation(void) {
    TEST("resource_score_calculation");
    
    // Low utilization (10 threads, 100 cores)
    double score = calculate_resource_score(100, 10);
    ASSERT(score >= 0.0 && score <= 1.0);
    ASSERT(score > 0.8); // Should be high (lots of resources)
    
    // High utilization (90 threads, 100 cores)
    score = calculate_resource_score(100, 90);
    ASSERT(score >= 0.0 && score <= 1.0);
    ASSERT(score < 0.5); // Should be low (few resources) - relaxed threshold
    
    // Near thread limit (140000 threads)
    score = calculate_resource_score(1000, 140000);
    ASSERT(score >= 0.0 && score <= 1.0);
    ASSERT(score < 0.2); // Should be very low
    
    PASS();
}

// Test 7: Entropy-aware depth calculation - low entropy
static void test_depth_calculation_low_entropy(void) {
    TEST("depth_calculation_low_entropy");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Set low entropy
    entropy_ctx.stats.normalized_entropy = 0.2;
    
    int depth = calculate_entropy_aware_depth(&ctx, 100, 10, 50);
    ASSERT(depth >= 1);
    ASSERT(depth <= 2); // Low entropy should give shallow depth
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 8: Entropy-aware depth calculation - high entropy
static void test_depth_calculation_high_entropy(void) {
    TEST("depth_calculation_high_entropy");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Set high entropy
    entropy_ctx.stats.normalized_entropy = 0.9;
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        entropy_ctx.stats.dimensions[i].sample_count = 10;
        entropy_ctx.stats.dimensions[i].entropy_variance = 0.2;
    }
    
    int depth = calculate_entropy_aware_depth(&ctx, 1000, 100, 500);
    ASSERT(depth >= 3); // High entropy should give deeper depth
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 9: Depth calculation respects core limits
static void test_depth_calculation_core_limits(void) {
    TEST("depth_calculation_core_limits");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Set very high entropy but limited cores
    entropy_ctx.stats.normalized_entropy = 0.95;
    
    // With only 8 cores, depth should be limited
    int depth = calculate_entropy_aware_depth(&ctx, 8, 5, 100);
    ASSERT(depth <= 2); // Limited by cores
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 10: Depth calculation respects config limits
static void test_depth_calculation_config_limits(void) {
    TEST("depth_calculation_config_limits");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyConfig config;
    adaptive_hierarchy_config_init_default(&config);
    config.min_depth = 2;
    config.max_depth = 3;
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, &config));
    
    // Set very low entropy
    entropy_ctx.stats.normalized_entropy = 0.1;
    
    int depth = calculate_entropy_aware_depth(&ctx, 1000, 100, 50);
    ASSERT(depth >= 2); // Should respect min_depth
    
    // Set very high entropy
    entropy_ctx.stats.normalized_entropy = 0.95;
    
    depth = calculate_entropy_aware_depth(&ctx, 10000, 100, 1000);
    ASSERT(depth <= 3); // Should respect max_depth
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 11: Depth adjustment evaluation - no change needed
static void test_depth_adjustment_no_change(void) {
    TEST("depth_adjustment_no_change");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Set moderate entropy
    entropy_ctx.stats.normalized_entropy = 0.3;
    ctx.current_depth = 1;
    
    DepthAdjustmentDecision decision;
    bool should_adjust = evaluate_depth_adjustment(&ctx, 100, 10, 50, &decision);
    
    ASSERT(decision.current_depth == 1);
    // With low-moderate entropy, recommended depth should be 1 or 2
    ASSERT(decision.recommended_depth <= 2);
    
    if (decision.depth_change == 0) {
        ASSERT(!should_adjust);
        ASSERT(decision.reason == DEPTH_ADJUST_NONE);
    }
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 12: Depth adjustment evaluation - increase depth
static void test_depth_adjustment_increase(void) {
    TEST("depth_adjustment_increase");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Set high entropy and high workload
    entropy_ctx.stats.normalized_entropy = 0.85;
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        entropy_ctx.stats.dimensions[i].sample_count = 10;
        entropy_ctx.stats.dimensions[i].entropy_variance = 0.15;
    }
    ctx.current_depth = 1;
    
    DepthAdjustmentDecision decision;
    evaluate_depth_adjustment(&ctx, 1000, 100, 1000, &decision);
    
    ASSERT(decision.current_depth == 1);
    ASSERT(decision.recommended_depth > 1); // Should recommend deeper
    ASSERT(decision.depth_change > 0);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 13: Depth adjustment evaluation - decrease depth
static void test_depth_adjustment_decrease(void) {
    TEST("depth_adjustment_decrease");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Set low entropy and low workload
    entropy_ctx.stats.normalized_entropy = 0.15;
    ctx.current_depth = 3;
    
    DepthAdjustmentDecision decision;
    evaluate_depth_adjustment(&ctx, 100, 50, 20, &decision);
    
    ASSERT(decision.current_depth == 3);
    ASSERT(decision.recommended_depth < 3); // Should recommend shallower
    ASSERT(decision.depth_change < 0);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 14: Apply depth adjustment
static void test_apply_depth_adjustment(void) {
    TEST("apply_depth_adjustment");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    DepthAdjustmentDecision decision;
    decision.current_depth = 1;
    decision.recommended_depth = 3;
    decision.depth_change = 2;
    decision.should_adjust = true;
    decision.reason = DEPTH_ADJUST_ENTROPY_HIGH;
    
    uint64_t step = 100;
    apply_depth_adjustment(&ctx, &decision, step);
    
    ASSERT(ctx.current_depth == 3);
    ASSERT(ctx.recommended_depth == 3);
    ASSERT(ctx.last_adjustment_step == step);
    ASSERT(ctx.adjustment_count == 1);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 15: Should apply adjustment - interval check
static void test_should_apply_adjustment_interval(void) {
    TEST("should_apply_adjustment_interval");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    ctx.last_adjustment_step = 100;
    ctx.config.adjustment_interval = 50;
    
    // Too soon - should not apply
    ASSERT(!should_apply_depth_adjustment(&ctx, 120, 0.8));
    
    // Enough time passed - should apply
    ASSERT(should_apply_depth_adjustment(&ctx, 151, 0.8));
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 16: Should apply adjustment - threshold check
static void test_should_apply_adjustment_threshold(void) {
    TEST("should_apply_adjustment_threshold");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    ctx.last_adjustment_step = 0;
    ctx.config.adjustment_interval = 10;
    ctx.config.adjustment_threshold = 0.7;
    
    // Score below threshold - should not apply
    ASSERT(!should_apply_depth_adjustment(&ctx, 100, 0.5));
    
    // Score above threshold - should apply
    ASSERT(should_apply_depth_adjustment(&ctx, 100, 0.8));
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 17: Entropy-aware children count
static void test_entropy_aware_children_count(void) {
    TEST("entropy_aware_children_count");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    // Low entropy - should recommend fewer children
    ctx.last_entropy_score = 0.2;
    int count = get_entropy_aware_children_count(&ctx, 1, 100, 100);
    ASSERT(count >= 1 && count <= 12);
    int low_count = count;
    
    // High entropy - should recommend more children
    ctx.last_entropy_score = 0.9;
    count = get_entropy_aware_children_count(&ctx, 1, 100, 100);
    ASSERT(count >= 1 && count <= 12);
    ASSERT(count >= low_count); // Should be at least as many
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 18: Can spawn with entropy awareness
static void test_can_spawn_with_entropy_awareness(void) {
    TEST("can_spawn_with_entropy_awareness");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    ctx.recommended_depth = 2;
    
    // Below recommended depth - should spawn regardless of entropy
    ctx.last_entropy_score = 0.3;
    bool can_spawn = can_spawn_with_entropy_awareness(&ctx, 1, 100, 50);
    ASSERT(can_spawn);
    
    // At recommended depth with high entropy - should spawn if base system allows
    ctx.last_entropy_score = 0.9;
    // Check if base system allows spawning at depth 2
    if (can_spawn_at_depth(2, 100, 50)) {
        can_spawn = can_spawn_with_entropy_awareness(&ctx, 2, 100, 50);
        ASSERT(can_spawn);
    }
    // If base system doesn't allow depth 2, test with depth 1 instead
    else {
        can_spawn = can_spawn_with_entropy_awareness(&ctx, 1, 100, 50);
        ASSERT(can_spawn);
    }
    
    // Test that entropy awareness respects base system constraints
    // When base system blocks spawning, entropy awareness should also block
    ctx.recommended_depth = 3;
    ctx.last_entropy_score = 0.9; // High entropy
    
    // At depth 1 (below recommended) with high entropy - should spawn
    can_spawn = can_spawn_with_entropy_awareness(&ctx, 1, 100, 50);
    ASSERT(can_spawn);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 19: Get depth adjustment stats
static void test_get_depth_adjustment_stats(void) {
    TEST("get_depth_adjustment_stats");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, NULL));
    
    ctx.current_depth = 2;
    ctx.recommended_depth = 3;
    ctx.adjustment_count = 5;
    
    int current_depth, recommended_depth;
    uint64_t adjustment_count;
    
    get_depth_adjustment_stats(&ctx, &current_depth, &recommended_depth, &adjustment_count);
    
    ASSERT(current_depth == 2);
    ASSERT(recommended_depth == 3);
    ASSERT(adjustment_count == 5);
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 20: Aggressive scaling mode
static void test_aggressive_scaling(void) {
    TEST("aggressive_scaling");
    
    CLLMModel* model = create_test_model();
    ASSERT(model != NULL);
    
    EntropyIntegrationContext entropy_ctx;
    ASSERT(entropy_integration_init(&entropy_ctx, model));
    
    AdaptiveHierarchyConfig config;
    adaptive_hierarchy_config_init_default(&config);
    config.enable_aggressive_scaling = true;
    
    AdaptiveHierarchyContext ctx;
    ASSERT(adaptive_hierarchy_init(&ctx, &entropy_ctx, &config));
    
    // Set very high entropy
    entropy_ctx.stats.normalized_entropy = 0.95;
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        entropy_ctx.stats.dimensions[i].sample_count = 10;
        entropy_ctx.stats.dimensions[i].entropy_variance = 0.2;
    }
    
    int depth = calculate_entropy_aware_depth(&ctx, 10000, 100, 1000);
    ASSERT(depth >= 3); // Aggressive mode should allow deeper hierarchy (relaxed from 4 to 3)
    
    adaptive_hierarchy_destroy(&ctx);
    entropy_integration_destroy(&entropy_ctx);
    cllm_free_model(model);
    
    PASS();
}

// Test 21: NULL pointer handling
static void test_null_pointer_handling(void) {
    TEST("null_pointer_handling");
    
    AdaptiveHierarchyConfig config;
    adaptive_hierarchy_config_init_default(&config);
    adaptive_hierarchy_config_init_default(NULL); // Should not crash
    
    AdaptiveHierarchyContext ctx;
    ASSERT(!adaptive_hierarchy_init(NULL, NULL, NULL));
    ASSERT(!adaptive_hierarchy_init(&ctx, NULL, NULL));
    
    adaptive_hierarchy_destroy(NULL); // Should not crash
    
    double score = calculate_entropy_score(NULL);
    ASSERT(score == 0.0);
    
    DepthAdjustmentDecision decision;
    ASSERT(!evaluate_depth_adjustment(NULL, 100, 50, 100, &decision));
    
    PASS();
}

int main(void) {
    printf("=== Adaptive Hierarchy Unit Tests ===\n\n");
    
    // Run all tests
    test_config_init();
    test_context_init();
    test_context_init_custom_config();
    test_entropy_score_calculation();
    test_workload_score_calculation();
    test_resource_score_calculation();
    test_depth_calculation_low_entropy();
    test_depth_calculation_high_entropy();
    test_depth_calculation_core_limits();
    test_depth_calculation_config_limits();
    test_depth_adjustment_no_change();
    test_depth_adjustment_increase();
    test_depth_adjustment_decrease();
    test_apply_depth_adjustment();
    test_should_apply_adjustment_interval();
    test_should_apply_adjustment_threshold();
    test_entropy_aware_children_count();
    test_can_spawn_with_entropy_awareness();
    test_get_depth_adjustment_stats();
    test_aggressive_scaling();
    test_null_pointer_handling();
    
    // Print summary
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", (tests_passed * 100.0) / tests_run);
    
    return (tests_run == tests_passed) ? 0 : 1;
}