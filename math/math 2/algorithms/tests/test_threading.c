/**
 * Threading Algorithms Test Suite
 * 
 * Tests for generic thread allocation and workload distribution.
 */

#include "threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== TEST: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ PASSED: %s\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ FAILED: %s\n", #name); \
        tests_failed++; \
    }

// ============================================================================
// TEST CASES
// ============================================================================

bool test_detect_cpu_cores(void) {
    int num_cores = detect_num_cpu_cores();
    printf("Detected CPU cores: %d\n", num_cores);
    
    if (num_cores < 1) {
        printf("ERROR: Failed to detect CPU cores\n");
        return false;
    }
    
    return true;
}

bool test_optimal_thread_count(void) {
    int optimal_4 = get_optimal_thread_count(4);
    int optimal_12 = get_optimal_thread_count(12);
    int optimal_24 = get_optimal_thread_count(24);
    
    printf("Optimal for 4 work groups: %d\n", optimal_4);
    printf("Optimal for 12 work groups: %d\n", optimal_12);
    printf("Optimal for 24 work groups: %d\n", optimal_24);
    
    // Should return at least 1
    if (optimal_4 < 1 || optimal_12 < 1 || optimal_24 < 1) {
        printf("ERROR: Invalid optimal thread count\n");
        return false;
    }
    
    return true;
}

bool test_default_workload_estimator(void) {
    uint64_t workload = default_workload_estimator(0, NULL);
    printf("Default workload: %lu\n", (unsigned long)workload);
    
    if (workload == 0) {
        printf("ERROR: Default workload should be non-zero\n");
        return false;
    }
    
    return true;
}

bool test_prime_workload_estimator(void) {
    uint64_t range[2] = {2, 1000000};
    uint64_t workload = prime_workload_estimator(0, range);
    printf("Prime workload [2, 1000000]: %lu\n", (unsigned long)workload);
    
    if (workload == 0) {
        printf("ERROR: Prime workload should be non-zero\n");
        return false;
    }
    
    return true;
}

bool test_create_one_to_one(void) {
    // 12 threads, 12 work groups -> one-to-one
    ThreadAllocationStrategy* strategy = thread_allocation_create(12, 12, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    printf("Strategy type: %d (expected ONE_TO_ONE = 0)\n", strategy->strategy);
    printf("All groups covered: %s\n", strategy->all_groups_covered ? "YES" : "NO");
    printf("Load balance: %.3f\n", strategy->load_balance_factor);
    
    if (strategy->strategy != THREAD_STRATEGY_ONE_TO_ONE) {
        printf("ERROR: Expected ONE_TO_ONE strategy\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    if (!strategy->all_groups_covered) {
        printf("ERROR: Not all groups covered\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    // Verify each thread has exactly 1 work group
    for (int t = 0; t < 12; t++) {
        if (strategy->mappings[t].num_work_groups != 1) {
            printf("ERROR: Thread %d has %d work groups (expected 1)\n", 
                   t, strategy->mappings[t].num_work_groups);
            thread_allocation_free(strategy);
            return false;
        }
    }
    
    thread_allocation_free(strategy);
    return true;
}

bool test_create_round_robin(void) {
    // 4 threads, 12 work groups -> round-robin
    ThreadAllocationStrategy* strategy = thread_allocation_create_with_strategy(
        4, 12, THREAD_STRATEGY_ROUND_ROBIN, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    printf("Strategy type: %d (expected ROUND_ROBIN = 1)\n", strategy->strategy);
    printf("All groups covered: %s\n", strategy->all_groups_covered ? "YES" : "NO");
    printf("Load balance: %.3f\n", strategy->load_balance_factor);
    
    if (strategy->strategy != THREAD_STRATEGY_ROUND_ROBIN) {
        printf("ERROR: Expected ROUND_ROBIN strategy\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    if (!strategy->all_groups_covered) {
        printf("ERROR: Not all groups covered\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    // Verify distribution
    int total_groups = 0;
    for (int t = 0; t < 4; t++) {
        printf("Thread %d: %d work groups\n", t, strategy->mappings[t].num_work_groups);
        total_groups += strategy->mappings[t].num_work_groups;
    }
    
    if (total_groups != 12) {
        printf("ERROR: Total groups = %d (expected 12)\n", total_groups);
        thread_allocation_free(strategy);
        return false;
    }
    
    thread_allocation_free(strategy);
    return true;
}

bool test_create_balanced(void) {
    // 4 threads, 12 work groups -> balanced
    uint64_t range[2] = {2, 1000000};
    ThreadAllocationStrategy* strategy = thread_allocation_create_with_strategy(
        4, 12, THREAD_STRATEGY_BALANCED, prime_workload_estimator, range);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    printf("Strategy type: %d (expected BALANCED = 2)\n", strategy->strategy);
    printf("All groups covered: %s\n", strategy->all_groups_covered ? "YES" : "NO");
    printf("Load balance: %.3f\n", strategy->load_balance_factor);
    
    if (strategy->strategy != THREAD_STRATEGY_BALANCED) {
        printf("ERROR: Expected BALANCED strategy\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    if (!strategy->all_groups_covered) {
        printf("ERROR: Not all groups covered\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    // Print workload distribution
    for (int t = 0; t < 4; t++) {
        printf("Thread %d: %d work groups, %.0f work units\n", 
               t, strategy->mappings[t].num_work_groups,
               strategy->mappings[t].expected_workload);
    }
    
    thread_allocation_free(strategy);
    return true;
}

bool test_query_functions(void) {
    ThreadAllocationStrategy* strategy = thread_allocation_create(4, 12, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    // Test get_work_groups
    int* groups;
    int count;
    if (thread_allocation_get_work_groups(strategy, 0, &groups, &count) != 0) {
        printf("ERROR: Failed to get work groups\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    printf("Thread 0 has %d work groups: [", count);
    for (int i = 0; i < count; i++) {
        printf("%d", groups[i]);
        if (i < count - 1) printf(", ");
    }
    printf("]\n");
    
    // Test get_thread_for_group
    for (int g = 0; g < 12; g++) {
        int thread = thread_allocation_get_thread_for_group(strategy, g);
        if (thread < 0 || thread >= 4) {
            printf("ERROR: Invalid thread %d for group %d\n", thread, g);
            thread_allocation_free(strategy);
            return false;
        }
    }
    
    // Test get_workload
    double workload = thread_allocation_get_workload(strategy, 0);
    printf("Thread 0 workload: %.0f\n", workload);
    
    thread_allocation_free(strategy);
    return true;
}

bool test_validation(void) {
    ThreadAllocationStrategy* strategy = thread_allocation_create(4, 12, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    bool valid = thread_allocation_validate(strategy);
    printf("Validation result: %s\n", valid ? "VALID" : "INVALID");
    
    if (!valid) {
        printf("ERROR: Strategy should be valid\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    thread_allocation_free(strategy);
    return true;
}

bool test_load_balance_calculation(void) {
    ThreadAllocationStrategy* strategy = thread_allocation_create(4, 12, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    double balance = thread_allocation_calculate_balance(strategy);
    printf("Load balance factor: %.3f\n", balance);
    
    if (balance < 0.0 || balance > 1.0) {
        printf("ERROR: Load balance factor should be between 0.0 and 1.0\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    thread_allocation_free(strategy);
    return true;
}

bool test_update_workloads(void) {
    ThreadAllocationStrategy* strategy = thread_allocation_create(4, 12, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    double actual_workloads[4] = {100.0, 150.0, 120.0, 130.0};
    
    if (thread_allocation_update_workloads(strategy, actual_workloads) != 0) {
        printf("ERROR: Failed to update workloads\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    // Verify workloads were updated
    for (int t = 0; t < 4; t++) {
        if (strategy->mappings[t].actual_workload != actual_workloads[t]) {
            printf("ERROR: Workload not updated for thread %d\n", t);
            thread_allocation_free(strategy);
            return false;
        }
    }
    
    printf("Workloads updated successfully\n");
    
    thread_allocation_free(strategy);
    return true;
}

bool test_rebalance(void) {
    ThreadAllocationStrategy* strategy = thread_allocation_create(4, 12, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    uint64_t initial_count = strategy->rebalance_count;
    
    if (thread_allocation_rebalance(strategy) != 0) {
        printf("ERROR: Failed to rebalance\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    if (strategy->rebalance_count != initial_count + 1) {
        printf("ERROR: Rebalance count not incremented\n");
        thread_allocation_free(strategy);
        return false;
    }
    
    printf("Rebalance count: %lu\n", (unsigned long)strategy->rebalance_count);
    
    thread_allocation_free(strategy);
    return true;
}

bool test_print_strategy(void) {
    ThreadAllocationStrategy* strategy = thread_allocation_create(4, 12, NULL, NULL);
    
    if (!strategy) {
        printf("ERROR: Failed to create strategy\n");
        return false;
    }
    
    thread_allocation_print(strategy);
    
    thread_allocation_free(strategy);
    return true;
}

bool test_edge_cases(void) {
    // Test with 1 thread, 12 work groups
    ThreadAllocationStrategy* strategy1 = thread_allocation_create(1, 12, NULL, NULL);
    if (!strategy1) {
        printf("ERROR: Failed to create strategy with 1 thread\n");
        return false;
    }
    
    if (strategy1->mappings[0].num_work_groups != 12) {
        printf("ERROR: Single thread should have all 12 work groups\n");
        thread_allocation_free(strategy1);
        return false;
    }
    
    printf("1 thread, 12 groups: OK\n");
    thread_allocation_free(strategy1);
    
    // Test with 24 threads, 12 work groups
    ThreadAllocationStrategy* strategy2 = thread_allocation_create(24, 12, NULL, NULL);
    if (!strategy2) {
        printf("ERROR: Failed to create strategy with 24 threads\n");
        return false;
    }
    
    // First 12 threads should have 1 work group each
    for (int t = 0; t < 12; t++) {
        if (strategy2->mappings[t].num_work_groups != 1) {
            printf("ERROR: Thread %d should have 1 work group\n", t);
            thread_allocation_free(strategy2);
            return false;
        }
    }
    
    // Remaining threads should have 0 work groups
    for (int t = 12; t < 24; t++) {
        if (strategy2->mappings[t].num_work_groups != 0) {
            printf("ERROR: Thread %d should have 0 work groups\n", t);
            thread_allocation_free(strategy2);
            return false;
        }
    }
    
    printf("24 threads, 12 groups: OK\n");
    thread_allocation_free(strategy2);
    
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         THREADING ALGORITHMS TEST SUITE                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    TEST(detect_cpu_cores);
    TEST(optimal_thread_count);
    TEST(default_workload_estimator);
    TEST(prime_workload_estimator);
    TEST(create_one_to_one);
    TEST(create_round_robin);
    TEST(create_balanced);
    TEST(query_functions);
    TEST(validation);
    TEST(load_balance_calculation);
    TEST(update_workloads);
    TEST(rebalance);
    TEST(print_strategy);
    TEST(edge_cases);
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                        ║\n", tests_passed + tests_failed);
    printf("║  Passed:       %3d                                        ║\n", tests_passed);
    printf("║  Failed:       %3d                                        ║\n", tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return (tests_failed == 0) ? 0 : 1;
}
