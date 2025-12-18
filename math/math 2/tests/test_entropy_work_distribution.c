/**
 * @file test_entropy_work_distribution.c
 * @brief Unit tests for entropy-based work distribution
 */

#include "ai/cllm_entropy_work_distribution.h"
#include "ai/cllm_entropy_integration.h"
#include "ai/cllm_entropy_allocation.h"
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
    
    WorkDistributionConfig config;
    work_distribution_config_init(&config);
    
    assert(config.strategy == WORK_DIST_COMBINED);
    assert(config.entropy_weight > 0.0 && config.entropy_weight <= 1.0);
    assert(config.plimpton_weight > 0.0 && config.plimpton_weight <= 1.0);
    assert(config.enforce_12fold == true);
    assert(config.min_work_per_thread > 0);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Plan creation and destruction
 */
static void test_plan_lifecycle(void) {
    printf("Test: Plan creation and destruction... ");
    
    WorkDistributionPlan* plan = work_distribution_plan_create(1000, 10);
    assert(plan != NULL);
    assert(plan->total_work_size == 1000);
    assert(plan->num_assignments == 0);
    assert(plan->assignments != NULL);
    
    work_distribution_plan_destroy(plan);
    
    // Test NULL handling
    work_distribution_plan_destroy(NULL);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Dimension work calculation
 */
static void test_dimension_work(void) {
    printf("Test: Dimension work calculation... ");
    
    // Test proportional distribution
    size_t work = calculate_dimension_work(0, 0.5, 1.0, 1000);
    assert(work == 500);
    
    // Test with different entropy
    work = calculate_dimension_work(1, 0.25, 1.0, 1000);
    assert(work == 250);
    
    // Test with zero entropy
    work = calculate_dimension_work(2, 0.0, 1.0, 1000);
    assert(work == 0);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Combined work distribution
 */
static void test_combined_distribution(void) {
    printf("Test: Combined work distribution... ");
    
    WorkDistribution dist = calculate_combined_work_distribution(
        1000,  // parent_work
        119,   // parent_id (p)
        120,   // child_id (q)
        0.6,   // parent_entropy
        0.4    // child_entropy
    );
    
    assert(dist.is_valid == true);
    assert(dist.parent_keeps > 0.0 && dist.parent_keeps < 1.0);
    assert(dist.child_gets > 0.0 && dist.child_gets < 1.0);
    assert(fabs(dist.parent_keeps + dist.child_gets - 1.0) < MATH_EPSILON);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Entropy work distribution calculation
 */
static void test_entropy_work_calculation(void) {
    printf("Test: Entropy work distribution calculation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionPlan* work_plan = work_distribution_plan_create(1000, 12);
    assert(work_plan != NULL);
    
    bool result = calculate_entropy_work_distribution(
        &entropy_ctx,
        &alloc_plan,
        1000,
        &work_config,
        work_plan
    );
    
    assert(result == true);
    assert(work_plan->distributed_work <= 1000);
    assert(work_plan->num_assignments > 0);
    
    work_distribution_plan_destroy(work_plan);
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Different work sizes
 */
static void test_different_work_sizes(void) {
    printf("Test: Different work sizes... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    size_t work_sizes[] = {100, 500, 1000, 5000, 10000};
    
    for (size_t i = 0; i < sizeof(work_sizes) / sizeof(work_sizes[0]); i++) {
        WorkDistributionPlan* plan = work_distribution_plan_create(work_sizes[i], 12);
        
        bool result = calculate_entropy_work_distribution(
            &entropy_ctx,
            &alloc_plan,
            work_sizes[i],
            &work_config,
            plan
        );
        
        assert(result == true);
        assert(plan->distributed_work <= work_sizes[i]);
        
        work_distribution_plan_destroy(plan);
    }
    
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Work distribution strategies
 */
static void test_distribution_strategies(void) {
    printf("Test: Work distribution strategies... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionStrategy strategies[] = {
        WORK_DIST_ENTROPY_ONLY,
        WORK_DIST_PLIMPTON_ONLY,
        WORK_DIST_COMBINED,
        WORK_DIST_ADAPTIVE
    };
    
    for (size_t i = 0; i < sizeof(strategies) / sizeof(strategies[0]); i++) {
        work_config.strategy = strategies[i];
        
        WorkDistributionPlan* plan = work_distribution_plan_create(1000, 12);
        
        bool result = calculate_entropy_work_distribution(
            &entropy_ctx,
            &alloc_plan,
            1000,
            &work_config,
            plan
        );
        
        assert(result == true);
        assert(validate_work_distribution_plan(plan) == true);
        
        work_distribution_plan_destroy(plan);
    }
    
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Work assignment to threads
 */
static void test_work_assignment(void) {
    printf("Test: Work assignment to threads... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionPlan* plan = work_distribution_plan_create(1000, 12);
    
    calculate_entropy_work_distribution(
        &entropy_ctx,
        &alloc_plan,
        1000,
        &work_config,
        plan
    );
    
    // Create thread IDs
    uint64_t thread_ids[12];
    for (int i = 0; i < 12; i++) {
        thread_ids[i] = 1000 + i;
    }
    
    bool result = assign_work_to_threads(plan, thread_ids, 12);
    assert(result == true);
    
    work_distribution_plan_destroy(plan);
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Get thread work assignment
 */
static void test_get_thread_assignment(void) {
    printf("Test: Get thread work assignment... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionPlan* plan = work_distribution_plan_create(1000, 12);
    
    calculate_entropy_work_distribution(
        &entropy_ctx,
        &alloc_plan,
        1000,
        &work_config,
        plan
    );
    
    // Get assignment for first thread
    if (plan->num_assignments > 0) {
        uint64_t thread_id = plan->assignments[0].thread_id;
        const WorkAssignment* assignment = get_thread_work_assignment(plan, thread_id);
        assert(assignment != NULL);
        assert(assignment->thread_id == thread_id);
    }
    
    // Get assignment for non-existent thread
    const WorkAssignment* assignment = get_thread_work_assignment(plan, 99999);
    assert(assignment == NULL);
    
    work_distribution_plan_destroy(plan);
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Work distribution validation
 */
static void test_work_validation(void) {
    printf("Test: Work distribution validation... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionPlan* plan = work_distribution_plan_create(1000, 12);
    
    calculate_entropy_work_distribution(
        &entropy_ctx,
        &alloc_plan,
        1000,
        &work_config,
        plan
    );
    
    // Should be valid
    assert(validate_work_distribution_plan(plan) == true);
    
    work_distribution_plan_destroy(plan);
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Work distribution efficiency
 */
static void test_work_efficiency(void) {
    printf("Test: Work distribution efficiency... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionPlan* plan = work_distribution_plan_create(1000, 12);
    
    calculate_entropy_work_distribution(
        &entropy_ctx,
        &alloc_plan,
        1000,
        &work_config,
        plan
    );
    
    double efficiency = calculate_work_distribution_efficiency(plan);
    assert(efficiency > 0.0 && efficiency <= 1.0);
    
    work_distribution_plan_destroy(plan);
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Work distribution balance
 */
static void test_work_balance(void) {
    printf("Test: Work distribution balance... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionPlan* plan = work_distribution_plan_create(1000, 12);
    
    calculate_entropy_work_distribution(
        &entropy_ctx,
        &alloc_plan,
        1000,
        &work_config,
        plan
    );
    
    double balance = calculate_work_distribution_balance(plan);
    assert(balance >= 0.0);
    
    work_distribution_plan_destroy(plan);
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: Print functions (visual check)
 */
static void test_print_functions(void) {
    printf("Test: Print functions... ");
    
    CLLMModel* model = create_test_model();
    EntropyIntegrationContext entropy_ctx;
    entropy_integration_init(&entropy_ctx, model);
    
    AllocationConfig alloc_config;
    allocation_config_init_default(&alloc_config);
    
    ThreadAllocationPlan alloc_plan;
    calculate_thread_allocation(&entropy_ctx, 144, &alloc_config, &alloc_plan);
    
    WorkDistributionConfig work_config;
    work_distribution_config_init(&work_config);
    
    WorkDistributionPlan* plan = work_distribution_plan_create(1000, 12);
    
    calculate_entropy_work_distribution(
        &entropy_ctx,
        &alloc_plan,
        1000,
        &work_config,
        plan
    );
    
    // Print for visual verification
    print_work_distribution_plan(plan);
    print_work_distribution_statistics(plan);
    
    work_distribution_plan_destroy(plan);
    entropy_integration_destroy(&entropy_ctx);
    free_test_model(model);
    
    printf("PASSED\n");
}

/**
 * @brief Test: NULL parameter handling
 */
static void test_null_parameters(void) {
    printf("Test: NULL parameter handling... ");
    
    WorkDistributionConfig config;
    
    // Test NULL in various functions
    work_distribution_config_init(NULL);
    
    assert(work_distribution_plan_create(0, 10) == NULL);
    assert(work_distribution_plan_create(1000, 0) == NULL);
    
    work_distribution_plan_destroy(NULL);
    
    assert(calculate_dimension_work(0, 0.5, 0.0, 1000) == 0);
    assert(calculate_dimension_work(0, 0.5, 1.0, 0) == 0);
    
    assert(calculate_entropy_work_distribution(NULL, NULL, 1000, &config, NULL) == false);
    assert(validate_work_distribution_plan(NULL) == false);
    assert(get_thread_work_assignment(NULL, 0) == NULL);
    assert(calculate_work_distribution_efficiency(NULL) == 0.0);
    assert(calculate_work_distribution_balance(NULL) == 0.0);
    
    printf("PASSED\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n=== Entropy Work Distribution Tests ===\n\n");
    
    test_config_init();
    test_plan_lifecycle();
    test_dimension_work();
    test_combined_distribution();
    test_entropy_work_calculation();
    test_different_work_sizes();
    test_distribution_strategies();
    test_work_assignment();
    test_get_thread_assignment();
    test_work_validation();
    test_work_efficiency();
    test_work_balance();
    test_print_functions();
    test_null_parameters();
    
    printf("\n=== All Tests Passed ===\n\n");
    
    return 0;
}