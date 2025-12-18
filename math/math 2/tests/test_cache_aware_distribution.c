/**
 * Unit Tests for Cache-Aware Work Distribution
 * 
 * Tests NUMA-aware and cache-optimized work distribution.
 */

#include "ai/cllm_cache_aware_distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing " #name "...\n"); \
    if (test_##name()) { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED\n"); \
        tests_failed++; \
    }

// ============================================================================
// TOPOLOGY DETECTION TESTS
// ============================================================================

int test_detect_cpu_topology() {
    CpuTopology topology = detect_cpu_topology();
    
    assert(topology.num_cpus > 0);
    assert(topology.num_numa_nodes > 0);
    assert(topology.numa_nodes[0].is_available == true);
    
    return 1;
}

int test_get_num_numa_nodes() {
    uint32_t num_nodes = cache_aware_get_num_numa_nodes();
    assert(num_nodes > 0);
    
    return 1;
}

int test_get_cache_size() {
    // L1 cache
    uint64_t l1_size = cache_aware_get_cache_size(1);
    // May be 0 if not detected, but shouldn't crash
    
    // L2 cache
    uint64_t l2_size = cache_aware_get_cache_size(2);
    
    // L3 cache
    uint64_t l3_size = cache_aware_get_cache_size(3);
    
    // Invalid level
    uint64_t invalid = cache_aware_get_cache_size(0);
    assert(invalid == 0);
    
    invalid = cache_aware_get_cache_size(4);
    assert(invalid == 0);
    
    (void)l1_size;
    (void)l2_size;
    (void)l3_size;
    
    return 1;
}

int test_is_numa_available() {
    bool available = cache_aware_is_numa_available();
    // Just check it doesn't crash
    (void)available;
    
    return 1;
}

// ============================================================================
// CACHE-AWARE DISTRIBUTION TESTS
// ============================================================================

int test_create_cache_aware_distribution() {
    CpuTopology topology = detect_cpu_topology();
    
    uint64_t work_sizes[] = {1000, 2000, 3000, 4000};
    size_t num_workers = 4;
    
    CacheAwareDistribution dist = create_cache_aware_distribution(
        num_workers,
        work_sizes,
        &topology
    );
    
    assert(dist.is_valid == true);
    assert(dist.num_placements == num_workers);
    assert(dist.placements != NULL);
    assert(dist.total_work_size == 10000);
    
    cache_aware_distribution_free(&dist);
    
    return 1;
}

int test_optimize_for_cache_locality() {
    CpuTopology topology = detect_cpu_topology();
    
    uint64_t work_sizes[] = {1000, 2000};
    CacheAwareDistribution dist = create_cache_aware_distribution(
        2, work_sizes, &topology
    );
    
    bool optimized = optimize_for_cache_locality(&dist, &topology);
    assert(optimized == true);
    
    cache_aware_distribution_free(&dist);
    
    return 1;
}

int test_optimize_for_numa() {
    CpuTopology topology = detect_cpu_topology();
    
    uint64_t work_sizes[] = {1000, 2000, 3000};
    CacheAwareDistribution dist = create_cache_aware_distribution(
        3, work_sizes, &topology
    );
    
    bool optimized = optimize_for_numa(&dist, &topology);
    assert(optimized == true);
    
    cache_aware_distribution_free(&dist);
    
    return 1;
}

int test_balance_distribution() {
    CpuTopology topology = detect_cpu_topology();
    
    // Balanced work sizes
    uint64_t work_sizes[] = {1000, 1000, 1000, 1000};
    CacheAwareDistribution dist = create_cache_aware_distribution(
        4, work_sizes, &topology
    );
    
    bool balanced = balance_distribution(&dist);
    assert(balanced == true);
    
    cache_aware_distribution_free(&dist);
    
    return 1;
}

// ============================================================================
// WORK STEALING TESTS
// ============================================================================

int test_work_stealing_queue_create_destroy() {
    WorkStealingQueue* queue = work_stealing_queue_create(10);
    assert(queue != NULL);
    assert(queue->capacity == 10);
    
    work_stealing_queue_destroy(queue);
    
    // Test with zero capacity (should use default)
    queue = work_stealing_queue_create(0);
    assert(queue != NULL);
    assert(queue->capacity > 0);
    
    work_stealing_queue_destroy(queue);
    
    return 1;
}

int test_work_stealing_queue_push_pop() {
    WorkStealingQueue* queue = work_stealing_queue_create(10);
    assert(queue != NULL);
    
    // Push items
    int item1 = 1, item2 = 2, item3 = 3;
    assert(work_stealing_queue_push(queue, &item1) == true);
    assert(work_stealing_queue_push(queue, &item2) == true);
    assert(work_stealing_queue_push(queue, &item3) == true);
    
    assert(work_stealing_queue_size(queue) == 3);
    
    // Pop items (LIFO)
    int* popped = (int*)work_stealing_queue_pop(queue);
    assert(popped != NULL);
    assert(*popped == 3);
    
    popped = (int*)work_stealing_queue_pop(queue);
    assert(popped != NULL);
    assert(*popped == 2);
    
    assert(work_stealing_queue_size(queue) == 1);
    
    work_stealing_queue_destroy(queue);
    
    return 1;
}

int test_work_stealing_queue_steal() {
    WorkStealingQueue* queue = work_stealing_queue_create(10);
    assert(queue != NULL);
    
    // Push items
    int item1 = 1, item2 = 2, item3 = 3;
    work_stealing_queue_push(queue, &item1);
    work_stealing_queue_push(queue, &item2);
    work_stealing_queue_push(queue, &item3);
    
    // Steal from head (FIFO)
    int* stolen = (int*)work_stealing_queue_steal(queue);
    assert(stolen != NULL);
    assert(*stolen == 1);
    
    stolen = (int*)work_stealing_queue_steal(queue);
    assert(stolen != NULL);
    assert(*stolen == 2);
    
    assert(work_stealing_queue_size(queue) == 1);
    
    work_stealing_queue_destroy(queue);
    
    return 1;
}

int test_work_stealing_queue_empty() {
    WorkStealingQueue* queue = work_stealing_queue_create(10);
    assert(queue != NULL);
    
    assert(work_stealing_queue_is_empty(queue) == true);
    
    int item = 1;
    work_stealing_queue_push(queue, &item);
    
    assert(work_stealing_queue_is_empty(queue) == false);
    
    work_stealing_queue_pop(queue);
    
    assert(work_stealing_queue_is_empty(queue) == true);
    
    work_stealing_queue_destroy(queue);
    
    return 1;
}

int test_work_stealing_queue_full() {
    WorkStealingQueue* queue = work_stealing_queue_create(3);
    assert(queue != NULL);
    
    int items[] = {1, 2, 3, 4};
    
    // Fill queue
    assert(work_stealing_queue_push(queue, &items[0]) == true);
    assert(work_stealing_queue_push(queue, &items[1]) == true);
    assert(work_stealing_queue_push(queue, &items[2]) == true);
    
    // Try to push when full
    assert(work_stealing_queue_push(queue, &items[3]) == false);
    
    work_stealing_queue_destroy(queue);
    
    return 1;
}

// ============================================================================
// STATISTICS TESTS
// ============================================================================

int test_statistics_init() {
    CacheAwareStats stats;
    cache_aware_stats_init(&stats);
    
    assert(stats.total_distributions == 0);
    assert(stats.work_steals == 0);
    assert(stats.failed_steals == 0);
    
    return 1;
}

int test_statistics_update() {
    CacheAwareStats stats;
    cache_aware_stats_init(&stats);
    
    CpuTopology topology = detect_cpu_topology();
    uint64_t work_sizes[] = {1000, 2000};
    
    CacheAwareDistribution dist = create_cache_aware_distribution(
        2, work_sizes, &topology
    );
    
    cache_aware_stats_update(&stats, &dist);
    
    assert(stats.total_distributions == 1);
    
    cache_aware_distribution_free(&dist);
    
    return 1;
}

int test_statistics_record_steal() {
    CacheAwareStats stats;
    cache_aware_stats_init(&stats);
    
    cache_aware_stats_record_steal(&stats, true);
    assert(stats.work_steals == 1);
    assert(stats.failed_steals == 0);
    
    cache_aware_stats_record_steal(&stats, false);
    assert(stats.work_steals == 1);
    assert(stats.failed_steals == 1);
    
    return 1;
}

// ============================================================================
// UTILITY TESTS
// ============================================================================

int test_cache_affinity() {
    // Work fits in cache
    double affinity = calculate_cache_affinity(1000, 10000);
    assert(affinity == 1.0);
    
    // Work partially fits
    affinity = calculate_cache_affinity(15000, 10000);
    assert(affinity == 0.5);
    
    // Work doesn't fit
    affinity = calculate_cache_affinity(50000, 10000);
    assert(affinity == 0.1);
    
    return 1;
}

int test_get_optimal_cpu() {
    CpuTopology topology = detect_cpu_topology();
    
    uint32_t cpu = get_optimal_cpu(1000, &topology);
    assert(cpu < topology.num_cpus);
    
    return 1;
}

int test_distribution_validation() {
    CpuTopology topology = detect_cpu_topology();
    
    uint64_t work_sizes[] = {1000, 2000, 3000};
    CacheAwareDistribution dist = create_cache_aware_distribution(
        3, work_sizes, &topology
    );
    
    assert(cache_aware_distribution_validate(&dist) == true);
    
    cache_aware_distribution_free(&dist);
    
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("=== Cache-Aware Work Distribution Unit Tests ===\n\n");
    
    // Topology detection tests
    TEST(detect_cpu_topology);
    TEST(get_num_numa_nodes);
    TEST(get_cache_size);
    TEST(is_numa_available);
    
    // Cache-aware distribution tests
    TEST(create_cache_aware_distribution);
    TEST(optimize_for_cache_locality);
    TEST(optimize_for_numa);
    TEST(balance_distribution);
    
    // Work stealing tests
    TEST(work_stealing_queue_create_destroy);
    TEST(work_stealing_queue_push_pop);
    TEST(work_stealing_queue_steal);
    TEST(work_stealing_queue_empty);
    TEST(work_stealing_queue_full);
    
    // Statistics tests
    TEST(statistics_init);
    TEST(statistics_update);
    TEST(statistics_record_steal);
    
    // Utility tests
    TEST(cache_affinity);
    TEST(get_optimal_cpu);
    TEST(distribution_validation);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}