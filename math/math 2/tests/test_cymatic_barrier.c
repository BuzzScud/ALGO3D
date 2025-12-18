/**
 * Unit tests for cymatic barrier synchronization
 */

#include "ai/cllm_cymatic_sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing %s...\n", #name); \
    if (test_##name()) { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED\n"); \
        tests_failed++; \
    }

/**
 * Test: Create and destroy barrier
 */
int test_create_destroy(void) {
    CymaticBarrier* barrier = cymatic_barrier_create(4, CYMATIC_FREQ_432_HZ, false);
    if (!barrier) return 0;
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: Create with custom frequency
 */
int test_create_custom(void) {
    CymaticBarrier* barrier = cymatic_barrier_create_custom(4, 100.0, false);
    if (!barrier) return 0;
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: Invalid parameters
 */
int test_invalid_params(void) {
    // Invalid num_threads
    CymaticBarrier* barrier1 = cymatic_barrier_create(0, CYMATIC_FREQ_432_HZ, false);
    if (barrier1) {
        cymatic_barrier_destroy(barrier1);
        return 0;
    }
    
    // Invalid frequency
    CymaticBarrier* barrier2 = cymatic_barrier_create_custom(4, -10.0, false);
    if (barrier2) {
        cymatic_barrier_destroy(barrier2);
        return 0;
    }
    
    return 1;
}

/**
 * Test: Timespec utilities
 */
int test_timespec_utils(void) {
    // Test ns_to_timespec
    struct timespec ts1 = ns_to_timespec(1500000000ULL);  // 1.5 seconds
    if (ts1.tv_sec != 1 || ts1.tv_nsec != 500000000) return 0;
    
    // Test timespec_add_ns
    struct timespec ts2 = {.tv_sec = 1, .tv_nsec = 500000000};
    struct timespec ts3 = timespec_add_ns(&ts2, 1000000000ULL);  // Add 1 second
    if (ts3.tv_sec != 2 || ts3.tv_nsec != 500000000) return 0;
    
    // Test timespec_diff_ns
    struct timespec start = {.tv_sec = 1, .tv_nsec = 0};
    struct timespec end = {.tv_sec = 2, .tv_nsec = 500000000};
    uint64_t diff = timespec_diff_ns(&end, &start);
    if (diff != 1500000000ULL) return 0;
    
    return 1;
}

/**
 * Test: Get statistics
 */
int test_get_stats(void) {
    CymaticBarrier* barrier = cymatic_barrier_create(1, CYMATIC_FREQ_432_HZ, false);
    if (!barrier) return 0;
    
    CymaticBarrierStats stats;
    int result = cymatic_barrier_get_stats(barrier, &stats);
    
    if (result != 0) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    // Check initial stats
    if (stats.sync_count != 0) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: Reset statistics
 */
int test_reset_stats(void) {
    CymaticBarrier* barrier = cymatic_barrier_create(1, CYMATIC_FREQ_432_HZ, false);
    if (!barrier) return 0;
    
    // Wait once to generate stats
    cymatic_barrier_wait(barrier);
    
    // Reset stats
    cymatic_barrier_reset_stats(barrier);
    
    // Check stats are reset
    CymaticBarrierStats stats;
    cymatic_barrier_get_stats(barrier, &stats);
    
    if (stats.sync_count != 0) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: Set frequency
 */
int test_set_frequency(void) {
    CymaticBarrier* barrier = cymatic_barrier_create(1, CYMATIC_FREQ_432_HZ, false);
    if (!barrier) return 0;
    
    // Change to 528 Hz
    int result = cymatic_barrier_set_frequency(barrier, CYMATIC_FREQ_528_HZ);
    if (result != 0) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    // Verify frequency changed
    if (barrier->frequency_ns != NS_528_HZ) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: Set custom frequency
 */
int test_set_custom_frequency(void) {
    CymaticBarrier* barrier = cymatic_barrier_create(1, CYMATIC_FREQ_432_HZ, false);
    if (!barrier) return 0;
    
    // Change to 100 Hz
    int result = cymatic_barrier_set_custom_frequency(barrier, 100.0);
    if (result != 0) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    // Verify frequency changed (approximately)
    uint64_t expected_ns = cymatic_frequency_to_ns(100.0);
    if (barrier->frequency_ns != expected_ns) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: Single thread wait
 */
int test_single_thread_wait(void) {
    CymaticBarrier* barrier = cymatic_barrier_create(1, CYMATIC_FREQ_GAMMA, false);
    if (!barrier) return 0;
    
    // Wait once
    int result = cymatic_barrier_wait(barrier);
    if (result != 0) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    // Check stats
    CymaticBarrierStats stats;
    cymatic_barrier_get_stats(barrier, &stats);
    
    if (stats.sync_count != 1) {
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Thread function for multi-thread test
 */
typedef struct {
    CymaticBarrier* barrier;
    int thread_id;
    int num_syncs;
    int success;
} ThreadData;

static void* thread_func(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < data->num_syncs; i++) {
        if (cymatic_barrier_wait(data->barrier) != 0) {
            data->success = 0;
            return NULL;
        }
    }
    
    data->success = 1;
    return NULL;
}

/**
 * Test: Multi-thread synchronization
 */
int test_multi_thread_sync(void) {
    const int num_threads = 4;
    const int num_syncs = 3;
    
    // Use a faster frequency for testing (100 Hz = 10 ms period)
    CymaticBarrier* barrier = cymatic_barrier_create_custom(num_threads, 100.0, false);
    if (!barrier) return 0;
    
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].barrier = barrier;
        thread_data[i].thread_id = i;
        thread_data[i].num_syncs = num_syncs;
        thread_data[i].success = 0;
        
        if (pthread_create(&threads[i], NULL, thread_func, &thread_data[i]) != 0) {
            cymatic_barrier_destroy(barrier);
            return 0;
        }
    }
    
    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Check all threads succeeded
    for (int i = 0; i < num_threads; i++) {
        if (!thread_data[i].success) {
            cymatic_barrier_destroy(barrier);
            return 0;
        }
    }
    
    // Check stats
    CymaticBarrierStats stats;
    cymatic_barrier_get_stats(barrier, &stats);
    
    // Debug output
    if (stats.sync_count != (uint64_t)num_syncs) {
        printf("    Expected %d syncs, got %lu\n", num_syncs, stats.sync_count);
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: Adaptive mode
 */
int test_adaptive_mode(void) {
    // Create adaptive barrier with fast frequency
    CymaticBarrier* barrier = cymatic_barrier_create_custom(1, 1000.0, true);  // 1000 Hz = 1 ms
    if (!barrier) return 0;
    
    // Wait once
    cymatic_barrier_wait(barrier);
    
    // Sleep longer than period
    usleep(5000);  // 5 ms
    
    // Wait again - should not wait full period in adaptive mode
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    cymatic_barrier_wait(barrier);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    uint64_t wait_time = timespec_diff_ns(&end, &start);
    
    // Should be much less than 1 ms since we're already past the period
    if (wait_time > 2000000) {  // 2 ms threshold
        cymatic_barrier_destroy(barrier);
        return 0;
    }
    
    cymatic_barrier_destroy(barrier);
    return 1;
}

/**
 * Test: NULL handling
 */
int test_null_handling(void) {
    // All functions should handle NULL gracefully
    cymatic_barrier_destroy(NULL);
    
    if (cymatic_barrier_wait(NULL) == 0) return 0;
    if (cymatic_barrier_try_wait(NULL) == 0) return 0;
    
    CymaticBarrierStats stats;
    if (cymatic_barrier_get_stats(NULL, &stats) == 0) return 0;
    
    cymatic_barrier_reset_stats(NULL);
    cymatic_barrier_print_stats(NULL);
    
    if (cymatic_barrier_set_frequency(NULL, CYMATIC_FREQ_432_HZ) == 0) return 0;
    if (cymatic_barrier_set_custom_frequency(NULL, 100.0) == 0) return 0;
    
    return 1;
}

int main(void) {
    printf("=== Cymatic Barrier Unit Tests ===\n\n");
    
    TEST(create_destroy);
    TEST(create_custom);
    TEST(invalid_params);
    TEST(timespec_utils);
    TEST(get_stats);
    TEST(reset_stats);
    TEST(set_frequency);
    TEST(set_custom_frequency);
    TEST(single_thread_wait);
    TEST(multi_thread_sync);
    TEST(adaptive_mode);
    TEST(null_handling);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}