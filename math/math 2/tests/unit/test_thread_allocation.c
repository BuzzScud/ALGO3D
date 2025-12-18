/**
 * Unit Tests for Thread Allocation System
 * 
 * Tests the mapping of N physical cores to 12 logical symmetry groups.
 */

#include "cllm_thread_allocation.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            tests_passed++; \
        } else { \
            tests_failed++; \
            fprintf(stderr, "FAIL: %s\n", message); \
        } \
    } while(0)

/**
 * Test: Detect CPU cores
 */
void test_detect_cpu_cores() {
    printf("Test: Detect CPU cores\n");
    
    int num_cores = detect_num_cpu_cores();
    TEST_ASSERT(num_cores > 0, "Should detect at least 1 CPU core");
    
    printf("  Detected %d CPU cores\n", num_cores);
}

/**
 * Test: Create allocation with 1 core
 */
void test_allocation_1_core() {
    printf("\nTest: Allocation with 1 core\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(1);
    TEST_ASSERT(strategy != NULL, "Should create strategy");
    TEST_ASSERT(strategy->num_physical_cores == 1, "Should have 1 physical core");
    TEST_ASSERT(strategy->num_symmetry_groups == 12, "Should have 12 symmetry groups");
    TEST_ASSERT(strategy->strategy == STRATEGY_ROUND_ROBIN, "Should use round-robin");
    
    // Thread 0 should handle all 12 groups
    TEST_ASSERT(strategy->mappings[0].num_symmetry_groups == 12, 
                "Thread 0 should handle all 12 groups");
    
    // Validate
    TEST_ASSERT(validate_thread_allocation(strategy), "Should be valid");
    TEST_ASSERT(strategy->all_groups_covered, "All groups should be covered");
    
    print_thread_allocation(strategy);
    
    free_thread_allocation(strategy);
}

/**
 * Test: Create allocation with 2 cores
 */
void test_allocation_2_cores() {
    printf("\nTest: Allocation with 2 cores\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(2);
    TEST_ASSERT(strategy != NULL, "Should create strategy");
    TEST_ASSERT(strategy->num_physical_cores == 2, "Should have 2 physical cores");
    
    // Thread 0 should handle 6 groups, Thread 1 should handle 6 groups
    TEST_ASSERT(strategy->mappings[0].num_symmetry_groups == 6, 
                "Thread 0 should handle 6 groups");
    TEST_ASSERT(strategy->mappings[1].num_symmetry_groups == 6, 
                "Thread 1 should handle 6 groups");
    
    // Validate
    TEST_ASSERT(validate_thread_allocation(strategy), "Should be valid");
    
    print_thread_allocation(strategy);
    
    free_thread_allocation(strategy);
}

/**
 * Test: Create allocation with 4 cores
 */
void test_allocation_4_cores() {
    printf("\nTest: Allocation with 4 cores\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(4);
    TEST_ASSERT(strategy != NULL, "Should create strategy");
    TEST_ASSERT(strategy->num_physical_cores == 4, "Should have 4 physical cores");
    
    // Each thread should handle 3 groups
    for (int t = 0; t < 4; t++) {
        TEST_ASSERT(strategy->mappings[t].num_symmetry_groups == 3, 
                    "Each thread should handle 3 groups");
    }
    
    // Validate
    TEST_ASSERT(validate_thread_allocation(strategy), "Should be valid");
    
    print_thread_allocation(strategy);
    
    free_thread_allocation(strategy);
}

/**
 * Test: Create allocation with 6 cores
 */
void test_allocation_6_cores() {
    printf("\nTest: Allocation with 6 cores\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(6);
    TEST_ASSERT(strategy != NULL, "Should create strategy");
    TEST_ASSERT(strategy->num_physical_cores == 6, "Should have 6 physical cores");
    
    // Each thread should handle 2 groups
    for (int t = 0; t < 6; t++) {
        TEST_ASSERT(strategy->mappings[t].num_symmetry_groups == 2, 
                    "Each thread should handle 2 groups");
    }
    
    // Validate
    TEST_ASSERT(validate_thread_allocation(strategy), "Should be valid");
    
    print_thread_allocation(strategy);
    
    free_thread_allocation(strategy);
}

/**
 * Test: Create allocation with 12 cores (ideal case)
 */
void test_allocation_12_cores() {
    printf("\nTest: Allocation with 12 cores (IDEAL)\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(12);
    TEST_ASSERT(strategy != NULL, "Should create strategy");
    TEST_ASSERT(strategy->num_physical_cores == 12, "Should have 12 physical cores");
    TEST_ASSERT(strategy->strategy == STRATEGY_ONE_TO_ONE, "Should use one-to-one");
    
    // Each thread should handle exactly 1 group
    for (int t = 0; t < 12; t++) {
        TEST_ASSERT(strategy->mappings[t].num_symmetry_groups == 1, 
                    "Each thread should handle 1 group");
        TEST_ASSERT(strategy->mappings[t].symmetry_groups[0] == t, 
                    "Thread should handle its corresponding group");
    }
    
    // Validate
    TEST_ASSERT(validate_thread_allocation(strategy), "Should be valid");
    TEST_ASSERT(strategy->load_balance_factor > 0.9, "Should be well balanced");
    
    print_thread_allocation(strategy);
    
    free_thread_allocation(strategy);
}

/**
 * Test: Create allocation with 24 cores (more than needed)
 */
void test_allocation_24_cores() {
    printf("\nTest: Allocation with 24 cores (EXCESS)\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(24);
    TEST_ASSERT(strategy != NULL, "Should create strategy");
    TEST_ASSERT(strategy->num_physical_cores == 24, "Should have 24 physical cores");
    TEST_ASSERT(strategy->strategy == STRATEGY_ONE_TO_ONE, "Should use one-to-one");
    
    // First 12 threads should handle 1 group each
    for (int t = 0; t < 12; t++) {
        TEST_ASSERT(strategy->mappings[t].num_symmetry_groups == 1, 
                    "First 12 threads should handle 1 group");
    }
    
    // Remaining threads should have no groups
    for (int t = 12; t < 24; t++) {
        TEST_ASSERT(strategy->mappings[t].num_symmetry_groups == 0, 
                    "Extra threads should have no groups");
    }
    
    // Validate
    TEST_ASSERT(validate_thread_allocation(strategy), "Should be valid");
    
    print_thread_allocation(strategy);
    
    free_thread_allocation(strategy);
}

/**
 * Test: Get symmetry groups for thread
 */
void test_get_symmetry_groups() {
    printf("\nTest: Get symmetry groups for thread\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(4);
    
    for (int t = 0; t < 4; t++) {
        int* groups;
        int count;
        int result = get_symmetry_groups_for_thread(strategy, t, &groups, &count);
        
        TEST_ASSERT(result == 0, "Should succeed");
        TEST_ASSERT(count == 3, "Should have 3 groups");
        TEST_ASSERT(groups != NULL, "Should have groups array");
        
        printf("  Thread %d: groups [", t);
        for (int i = 0; i < count; i++) {
            printf("%d", groups[i]);
            if (i < count - 1) printf(", ");
        }
        printf("]\n");
    }
    
    free_thread_allocation(strategy);
}

/**
 * Test: Get thread for symmetry group
 */
void test_get_thread_for_group() {
    printf("\nTest: Get thread for symmetry group\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(4);
    
    for (int g = 0; g < 12; g++) {
        int thread = get_thread_for_symmetry_group(strategy, g);
        TEST_ASSERT(thread >= 0 && thread < 4, "Should return valid thread");
        
        printf("  Symmetry group %d -> Thread %d\n", g, thread);
    }
    
    free_thread_allocation(strategy);
}

/**
 * Test: Validate all groups covered
 */
void test_validate_coverage() {
    printf("\nTest: Validate all groups covered\n");
    
    int test_cases[] = {1, 2, 3, 4, 5, 6, 8, 12, 16, 24};
    int num_cases = sizeof(test_cases) / sizeof(int);
    
    for (int i = 0; i < num_cases; i++) {
        int num_cores = test_cases[i];
        ThreadAllocationStrategy* strategy = create_thread_allocation(num_cores);
        
        bool valid = validate_thread_allocation(strategy);
        TEST_ASSERT(valid, "All allocations should be valid");
        
        // Check that all 12 groups are covered
        bool covered[12] = {false};
        for (int t = 0; t < strategy->num_physical_cores; t++) {
            for (int j = 0; j < strategy->mappings[t].num_symmetry_groups; j++) {
                int group = strategy->mappings[t].symmetry_groups[j];
                covered[group] = true;
            }
        }
        
        for (int g = 0; g < 12; g++) {
            TEST_ASSERT(covered[g], "All groups should be covered");
        }
        
        printf("  %d cores: VALID (all 12 groups covered)\n", num_cores);
        
        free_thread_allocation(strategy);
    }
}

/**
 * Test: Workload estimation
 */
void test_workload_estimation() {
    printf("\nTest: Workload estimation\n");
    
    // Test workload estimation for different ranges
    uint64_t workload = estimate_symmetry_group_workload(0, 2, 1000);
    TEST_ASSERT(workload > 0, "Should estimate non-zero workload");
    printf("  Primes ≡ 0 (mod 12) in [2, 1000]: ~%lu\n", workload);
    
    workload = estimate_symmetry_group_workload(1, 2, 1000);
    printf("  Primes ≡ 1 (mod 12) in [2, 1000]: ~%lu\n", workload);
    
    workload = estimate_symmetry_group_workload(5, 2, 1000);
    printf("  Primes ≡ 5 (mod 12) in [2, 1000]: ~%lu\n", workload);
}

/**
 * Test: Optimal thread count
 */
void test_optimal_thread_count() {
    printf("\nTest: Optimal thread count\n");
    
    int optimal = get_optimal_thread_count();
    TEST_ASSERT(optimal > 0, "Should return positive value");
    TEST_ASSERT(optimal <= 12, "Should not exceed 12 (mathematical limit)");
    
    printf("  Optimal thread count: %d\n", optimal);
}

/**
 * Main test runner
 */
int main() {
    printf("=== THREAD ALLOCATION UNIT TESTS ===\n\n");
    
    test_detect_cpu_cores();
    test_allocation_1_core();
    test_allocation_2_cores();
    test_allocation_4_cores();
    test_allocation_6_cores();
    test_allocation_12_cores();
    test_allocation_24_cores();
    test_get_symmetry_groups();
    test_get_thread_for_group();
    test_validate_coverage();
    test_workload_estimation();
    test_optimal_thread_count();
    
    printf("\n=== TEST RESULTS ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("\n✗ SOME TESTS FAILED\n");
        return 1;
    }
}