/**
 * @file test_workload_detector.c
 * @brief Unit tests for workload detection system
 */

#include "ai/cllm_workload_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

/**
 * @brief Test: Context initialization
 */
static void test_context_init(void) {
    printf("Testing context initialization...\n");
    
    WorkloadDetectorContext ctx;
    
    // Initialize with verbose off
    assert(workload_detector_init(&ctx, false));
    assert(ctx.verbose == false);
    assert(ctx.spawn_count == 0);
    assert(ctx.despawn_count == 0);
    
    workload_detector_destroy(&ctx);
    
    // Initialize with verbose on
    assert(workload_detector_init(&ctx, true));
    assert(ctx.verbose == true);
    
    workload_detector_destroy(&ctx);
    
    // NULL context
    assert(!workload_detector_init(NULL, false));
    
    printf("✓ Context initialization test passed\n");
}

/**
 * @brief Test: Metrics collection
 */
static void test_metrics_collection(void) {
    printf("Testing metrics collection...\n");
    
    WorkloadMetrics metrics;
    
    // Collect metrics
    collect_workload_metrics(100, 5, 50, 10.0, 2, &metrics);
    
    assert(metrics.pending_batches == 100);
    assert(metrics.active_workers == 5);
    assert(metrics.completed_batches == 50);
    assert(metrics.total_processing_time == 10.0);
    assert(metrics.current_depth == 2);
    assert(metrics.avg_batch_time == 0.2); // 10.0 / 50
    
    // Test with zero completed batches
    collect_workload_metrics(50, 3, 0, 0.0, 1, &metrics);
    assert(metrics.avg_batch_time == 0.0);
    
    printf("✓ Metrics collection test passed\n");
}

/**
 * @brief Test: Available cores detection
 */
static void test_available_cores(void) {
    printf("Testing available cores detection...\n");
    
    int cores = get_available_cores();
    assert(cores > 0);
    
    printf("  Detected %d cores\n", cores);
    printf("✓ Available cores test passed\n");
}

/**
 * @brief Test: Hierarchy depth check
 */
static void test_hierarchy_depth(void) {
    printf("Testing hierarchy depth check...\n");
    
    assert(!at_max_hierarchy_depth(0));
    assert(!at_max_hierarchy_depth(1));
    assert(!at_max_hierarchy_depth(4));
    assert(at_max_hierarchy_depth(5));
    assert(at_max_hierarchy_depth(6));
    
    printf("✓ Hierarchy depth test passed\n");
}

/**
 * @brief Test: Hysteresis
 */
static void test_hysteresis(void) {
    printf("Testing hysteresis...\n");
    
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    
    double current_time = get_current_time_seconds();
    
    // Initially, hysteresis should be satisfied (no previous spawn)
    assert(spawn_hysteresis_satisfied(&ctx, current_time));
    assert(despawn_hysteresis_satisfied(&ctx, current_time));
    
    // Set last spawn time to now
    ctx.last_spawn_time = current_time;
    
    // Immediately after, hysteresis should NOT be satisfied
    assert(!spawn_hysteresis_satisfied(&ctx, current_time));
    
    // After waiting, hysteresis should be satisfied
    assert(spawn_hysteresis_satisfied(&ctx, current_time + SPAWN_HYSTERESIS_SECONDS + 1.0));
    
    workload_detector_destroy(&ctx);
    
    printf("✓ Hysteresis test passed\n");
}

/**
 * @brief Test: Optimal threads calculation
 */
static void test_optimal_threads(void) {
    printf("Testing optimal threads calculation...\n");
    
    WorkloadMetrics metrics;
    int available_cores = 16;
    
    // Not enough work
    collect_workload_metrics(5, 1, 0, 0.0, 0, &metrics);
    int threads = calculate_optimal_threads(&metrics, available_cores);
    assert(threads == 1);
    
    // Moderate work
    collect_workload_metrics(100, 1, 0, 0.0, 0, &metrics);
    threads = calculate_optimal_threads(&metrics, available_cores);
    assert(threads > 1);
    assert(threads <= available_cores);
    
    // Heavy work
    collect_workload_metrics(1000, 1, 0, 0.0, 0, &metrics);
    threads = calculate_optimal_threads(&metrics, available_cores);
    assert(threads <= available_cores);
    
    printf("✓ Optimal threads test passed\n");
}

/**
 * @brief Test: Spawn decision - insufficient work
 */
static void test_spawn_decision_insufficient_work(void) {
    printf("Testing spawn decision with insufficient work...\n");
    
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    
    WorkloadMetrics metrics;
    collect_workload_metrics(50, 1, 0, 0.0, 0, &metrics); // Not enough for 12 children
    
    int available_cores = 16;
    SpawnDecision decision = should_spawn_children(&ctx, &metrics, 0, available_cores);
    
    assert(decision == SPAWN_DECISION_NO);
    
    workload_detector_destroy(&ctx);
    
    printf("✓ Insufficient work test passed\n");
}

/**
 * @brief Test: Spawn decision - insufficient cores
 */
static void test_spawn_decision_insufficient_cores(void) {
    printf("Testing spawn decision with insufficient cores...\n");
    
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    
    WorkloadMetrics metrics;
    collect_workload_metrics(200, 1, 0, 0.0, 0, &metrics); // Enough work
    
    int available_cores = 8; // Not enough for 12 children
    SpawnDecision decision = should_spawn_children(&ctx, &metrics, 0, available_cores);
    
    assert(decision == SPAWN_DECISION_NO);
    
    workload_detector_destroy(&ctx);
    
    printf("✓ Insufficient cores test passed\n");
}

/**
 * @brief Test: Spawn decision - at max depth
 */
static void test_spawn_decision_max_depth(void) {
    printf("Testing spawn decision at max depth...\n");
    
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    
    WorkloadMetrics metrics;
    collect_workload_metrics(200, 1, 0, 0.0, MAX_HIERARCHY_DEPTH, &metrics);
    
    int available_cores = 16;
    SpawnDecision decision = should_spawn_children(&ctx, &metrics, 0, available_cores);
    
    assert(decision == SPAWN_DECISION_NO);
    
    workload_detector_destroy(&ctx);
    
    printf("✓ Max depth test passed\n");
}

/**
 * @brief Test: Spawn decision - should spawn
 */
static void test_spawn_decision_should_spawn(void) {
    printf("Testing spawn decision - should spawn...\n");
    
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    
    WorkloadMetrics metrics;
    collect_workload_metrics(200, 1, 0, 0.0, 0, &metrics); // Enough work
    
    int available_cores = 16; // Enough cores
    SpawnDecision decision = should_spawn_children(&ctx, &metrics, 0, available_cores);
    
    assert(decision == SPAWN_DECISION_YES);
    assert(ctx.spawn_count == 1);
    
    workload_detector_destroy(&ctx);
    
    printf("✓ Should spawn test passed\n");
}

/**
 * @brief Test: Despawn decision - low workload
 */
static void test_despawn_decision_low_workload(void) {
    printf("Testing despawn decision with low workload...\n");
    
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    
    // Set last despawn time to past to satisfy hysteresis
    ctx.last_despawn_time = get_current_time_seconds() - SPAWN_HYSTERESIS_SECONDS - 1.0;
    
    WorkloadMetrics metrics;
    collect_workload_metrics(5, 12, 0, 0.0, 1, &metrics); // Low work
    
    int available_cores = 16;
    SpawnDecision decision = should_spawn_children(&ctx, &metrics, 12, available_cores);
    
    assert(decision == SPAWN_DECISION_DESPAWN);
    assert(ctx.despawn_count == 1);
    
    workload_detector_destroy(&ctx);
    
    printf("✓ Despawn decision test passed\n");
}

/**
 * @brief Test: Statistics tracking
 */
static void test_statistics(void) {
    printf("Testing statistics tracking...\n");
    
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    
    // Simulate some spawns and despawns
    ctx.spawn_count = 5;
    ctx.despawn_count = 3;
    
    size_t spawn_count, despawn_count;
    workload_detector_get_stats(&ctx, &spawn_count, &despawn_count);
    
    assert(spawn_count == 5);
    assert(despawn_count == 3);
    
    // Reset stats
    workload_detector_reset_stats(&ctx);
    workload_detector_get_stats(&ctx, &spawn_count, &despawn_count);
    
    assert(spawn_count == 0);
    assert(despawn_count == 0);
    
    workload_detector_destroy(&ctx);
    
    printf("✓ Statistics test passed\n");
}

/**
 * @brief Test: NULL parameter handling
 */
static void test_null_parameters(void) {
    printf("Testing NULL parameter handling...\n");
    
    WorkloadMetrics metrics;
    
    // NULL metrics
    collect_workload_metrics(100, 5, 50, 10.0, 2, NULL);
    
    // NULL context in decision
    SpawnDecision decision = should_spawn_children(NULL, &metrics, 0, 16);
    assert(decision == SPAWN_DECISION_NO);
    
    // NULL metrics in decision
    WorkloadDetectorContext ctx;
    workload_detector_init(&ctx, false);
    decision = should_spawn_children(&ctx, NULL, 0, 16);
    assert(decision == SPAWN_DECISION_NO);
    
    // NULL in stats
    workload_detector_get_stats(NULL, NULL, NULL);
    
    workload_detector_destroy(&ctx);
    
    printf("✓ NULL parameter handling test passed\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n=== Workload Detector Tests ===\n\n");
    
    test_context_init();
    test_metrics_collection();
    test_available_cores();
    test_hierarchy_depth();
    test_hysteresis();
    test_optimal_threads();
    test_spawn_decision_insufficient_work();
    test_spawn_decision_insufficient_cores();
    test_spawn_decision_max_depth();
    test_spawn_decision_should_spawn();
    test_despawn_decision_low_workload();
    test_statistics();
    test_null_parameters();
    
    printf("\n=== All Workload Detector Tests Passed ===\n\n");
    
    return 0;
}