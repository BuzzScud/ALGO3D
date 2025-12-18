/**
 * @file test_entropy_allocation.c
 * @brief Unit tests for entropy-based thread allocation
 */

#include "ai/cllm_entropy_allocation.h"
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
    
    model->vocab_size = 1000;
    model->embedding_dim = 128;
    model->num_layers = 4;
    model->num_lattice_points = 100;
    
    model->lattice_points = (CLLMLatticePoint*)calloc(
        model->num_lattice_points,
        sizeof(CLLMLatticePoint)
    );
    
    if (!model->lattice_points) {
        free(model);
        return NULL;
    }
    
    for (uint64_t i = 0; i < model->num_lattice_points; i++) {
        model->lattice_points[i].point_id = i + 1;
        model->lattice_points[i].symmetry_group = i % 12;
        model->lattice_points[i].prime = i + 2;
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
 * @brief Test: Configuration initialization
 */
static void test_config_init(void) {
    printf("Test: Configuration initialization... ");
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    assert(config.strategy == ALLOCATION_PROPORTIONAL);
    assert(config.entropy_threshold > 0.0);
    assert(config.enforce_12fold == true);
    assert(config.min_threads_per_dimension == 12);
    assert(config.max_threads_per_dimension == 144);
    assert(config.balance_factor >= 0.0 && config.balance_factor <= 1.0);
    
    printf("PASSED\n");
}

/**
 * @brief Test: 12-fold symmetry adjustment
 */
static void test_12fold_adjustment(void) {
    printf("Test: 12-fold symmetry adjustment... ");
    
    // Test various inputs (rounds to nearest multiple of 12)
    assert(adjust_to_12fold_symmetry(0) == 0);
    assert(adjust_to_12fold_symmetry(1) == 12);
    assert(adjust_to_12fold_symmetry(6) == 12);   // 6 rounds to 12
    assert(adjust_to_12fold_symmetry(12) == 12);
    assert(adjust_to_12fold_symmetry(13) == 12);  // 13 rounds to 12
    assert(adjust_to_12fold_symmetry(18) == 24);  // 18 rounds to 24
    assert(adjust_to_12fold_symmetry(24) == 24);
    assert(adjust_to_12fold_symmetry(30) == 36);  // 30 rounds to 36
    assert(adjust_to_12fold_symmetry(36) == 36);
    assert(adjust_to_12fold_symmetry(100) == 96); // 100 rounds to 96
    assert(adjust_to_12fold_symmetry(144) == 144);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Dimension thread calculation
 */
static void test_dimension_threads(void) {
    printf("Test: Dimension thread calculation... ");
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    // Test proportional allocation
    config.strategy = ALLOCATION_PROPORTIONAL;
    int threads = calculate_dimension_threads(0.5, 1.0, 100, &config);
    assert(threads >= 40 && threads <= 60);  // Should be around 50
    
    // Test with low entropy (below threshold)
    threads = calculate_dimension_threads(0.001, 1.0, 100, &config);
    assert(threads == 0);
    
    // Test threshold strategy
    config.strategy = ALLOCATION_THRESHOLD;
    threads = calculate_dimension_threads(0.5, 1.0, 120, &config);
    assert(threads == 12);  // 120 / 12 = 10, but min is 12
    
    // Test balanced strategy
    config.strategy = ALLOCATION_BALANCED;
    config.balance_factor = 0.5;
    threads = calculate_dimension_threads(0.5, 1.0, 100, &config);
    assert(threads > 0);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Thread allocation calculation
 */
static void test_allocation_calculation(void) {
    printf("Test: Thread allocation calculation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    bool result = calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    assert(result == true);
    assert(plan.total_available_threads == 144);
    assert(plan.total_adjusted_threads <= 144);
    assert(plan.enforce_12fold == true);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Allocation with different thread counts
 */
static void test_different_thread_counts(void) {
    printf("Test: Allocation with different thread counts... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    // Test with various thread counts
    int thread_counts[] = {12, 24, 48, 96, 144, 288};
    
    for (size_t i = 0; i < sizeof(thread_counts) / sizeof(thread_counts[0]); i++) {
        ThreadAllocationPlan plan;
        bool result = calculate_thread_allocation(&ctx, thread_counts[i], &config, &plan);
        
        assert(result == true);
        assert(plan.total_available_threads == thread_counts[i]);
        assert(plan.total_adjusted_threads <= thread_counts[i]);
    }
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: 12-fold symmetry enforcement
 */
static void test_12fold_enforcement(void) {
    printf("Test: 12-fold symmetry enforcement... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    config.enforce_12fold = true;
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    // Check all allocations are multiples of 12
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        if (plan.dimensions[i].is_active) {
            assert(plan.dimensions[i].adjusted_threads % 12 == 0);
        }
    }
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Allocation validation
 */
static void test_allocation_validation(void) {
    printf("Test: Allocation validation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    // Should be valid
    assert(validate_allocation_plan(&plan) == true);
    
    // Make it invalid by exceeding available threads
    plan.total_adjusted_threads = 200;
    assert(validate_allocation_plan(&plan) == false);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Get dimension allocation
 */
static void test_get_dimension_allocation(void) {
    printf("Test: Get dimension allocation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    // Get valid dimension
    const DimensionAllocation* alloc = get_dimension_allocation(&plan, 0);
    assert(alloc != NULL);
    assert(alloc->dimension == 0);
    
    // Get invalid dimension
    alloc = get_dimension_allocation(&plan, 99);
    assert(alloc == NULL);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Allocation efficiency
 */
static void test_allocation_efficiency(void) {
    printf("Test: Allocation efficiency... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    double efficiency = calculate_allocation_efficiency(&plan);
    assert(efficiency >= 0.0 && efficiency <= 1.0);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Allocation balance
 */
static void test_allocation_balance(void) {
    printf("Test: Allocation balance... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    double balance = calculate_allocation_balance(&plan);
    assert(balance >= 0.0);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Allocation strategies
 */
static void test_allocation_strategies(void) {
    printf("Test: Allocation strategies... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    AllocationStrategy strategies[] = {
        ALLOCATION_PROPORTIONAL,
        ALLOCATION_THRESHOLD,
        ALLOCATION_BALANCED,
        ALLOCATION_ADAPTIVE
    };
    
    for (size_t i = 0; i < sizeof(strategies) / sizeof(strategies[0]); i++) {
        config.strategy = strategies[i];
        
        ThreadAllocationPlan plan;
        bool result = calculate_thread_allocation(&ctx, 144, &config, &plan);
        
        assert(result == true);
        assert(validate_allocation_plan(&plan) == true);
    }
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Rebalancing
 */
static void test_rebalancing(void) {
    printf("Test: Rebalancing... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    double original_balance = calculate_allocation_balance(&plan);
    
    // Try to rebalance
    bool result = rebalance_allocation(&plan, 0.1);
    assert(result == true);
    
    double new_balance = calculate_allocation_balance(&plan);
    // Balance should improve or stay the same
    assert(new_balance <= original_balance + MATH_EPSILON);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Plan comparison
 */
static void test_plan_comparison(void) {
    printf("Test: Plan comparison... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan1, plan2;
    calculate_thread_allocation(&ctx, 144, &config, &plan1);
    calculate_thread_allocation(&ctx, 144, &config, &plan2);
    
    // Same plans should have zero difference
    double diff = compare_allocation_plans(&plan1, &plan2);
    assert(diff < MATH_EPSILON);
    
    // Different plans should have non-zero difference
    calculate_thread_allocation(&ctx, 96, &config, &plan2);
    diff = compare_allocation_plans(&plan1, &plan2);
    assert(diff > MATH_EPSILON);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Apply allocation plan
 */
static void test_apply_plan(void) {
    printf("Test: Apply allocation plan... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    bool result = apply_allocation_plan(&plan);
    assert(result == true);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Print functions (visual check)
 */
static void test_print_functions(void) {
    printf("Test: Print functions... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext ctx;
    entropy_integration_init(&ctx, model);
    
    AllocationConfig config;
    allocation_config_init_default(&config);
    
    ThreadAllocationPlan plan;
    calculate_thread_allocation(&ctx, 144, &config, &plan);
    
    // Print for visual verification
    print_allocation_plan(&plan);
    print_allocation_statistics(&plan);
    
    entropy_integration_destroy(&ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: NULL parameter handling
 */
static void test_null_parameters(void) {
    printf("Test: NULL parameter handling... ");
    
    AllocationConfig config;
    ThreadAllocationPlan plan;
    
    // Test NULL in various functions
    allocation_config_init_default(NULL);
    
    assert(adjust_to_12fold_symmetry(-1) == 0);
    assert(calculate_dimension_threads(0.5, 1.0, 100, NULL) == 0);
    assert(calculate_thread_allocation(NULL, 100, &config, &plan) == false);
    assert(validate_allocation_plan(NULL) == false);
    assert(apply_allocation_plan(NULL) == false);
    assert(get_dimension_allocation(NULL, 0) == NULL);
    assert(calculate_allocation_efficiency(NULL) == 0.0);
    assert(calculate_allocation_balance(NULL) == 0.0);
    
    printf("PASSED\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n=== Entropy Allocation Tests ===\n\n");
    
    test_config_init();
    test_12fold_adjustment();
    test_dimension_threads();
    test_allocation_calculation();
    test_different_thread_counts();
    test_12fold_enforcement();
    test_allocation_validation();
    test_get_dimension_allocation();
    test_allocation_efficiency();
    test_allocation_balance();
    test_allocation_strategies();
    test_rebalancing();
    test_plan_comparison();
    test_apply_plan();
    test_print_functions();
    test_null_parameters();
    
    printf("\n=== All Tests Passed ===\n\n");
    
    return 0;
}