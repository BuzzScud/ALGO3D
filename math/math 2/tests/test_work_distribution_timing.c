/**
 * Unit Tests for Work Distribution Timing Integration
 * 
 * Tests the integration of 432 Hz cymatic timing with work distribution.
 */

#include "ai/cllm_work_distribution_timing.h"
#include "ai/cllm_work_distribution.h"
#include "ai/cllm_cymatic_sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { \
        printf("Testing %s...\n", #name); \
        if (test_##name()) { \
            printf("  ✓ PASSED\n"); \
            tests_passed++; \
        } else { \
            printf("  ✗ FAILED\n"); \
            tests_failed++; \
        } \
    } while(0)

// ============================================================================
// BASIC TESTS
// ============================================================================

int test_create_destroy(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(4, false);
    assert(ctx != NULL);
    assert(ctx->barrier != NULL);
    assert(ctx->stats != NULL);
    assert(ctx->timing_enabled == true);
    assert(ctx->adaptive_timing == false);
    assert(ctx->num_threads == 4);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_create_adaptive(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(4, true);
    assert(ctx != NULL);
    assert(ctx->adaptive_timing == true);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_create_invalid_threads(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(0, false);
    assert(ctx == NULL);
    
    ctx = timed_work_distribution_create(-1, false);
    assert(ctx == NULL);
    
    return 1;
}

// ============================================================================
// SINGLE DISTRIBUTION TESTS
// ============================================================================

int test_single_distribution(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    TimedDistributionResult result;
    bool success = timed_calculate_work_distribution(ctx, 5, 3, &result);
    
    assert(success == true);
    assert(result.distribution.is_valid == true);
    assert(result.distribution.parent_id == 5);
    assert(result.distribution.child_id == 3);
    assert(result.total_time_ns > 0);
    assert(result.compute_time_ns > 0);
    
    // Check statistics updated
    assert(ctx->total_distributions == 1);
    assert(ctx->total_compute_time_ns > 0);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_invalid_distribution(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Both odd - violates Plimpton constraints
    TimedDistributionResult result;
    bool success = timed_calculate_work_distribution(ctx, 5, 7, &result);
    
    assert(success == false);
    assert(result.distribution.is_valid == false);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_multiple_distributions(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Perform multiple distributions
    for (int i = 0; i < 10; i++) {
        TimedDistributionResult result;
        bool success = timed_calculate_work_distribution(ctx, 5, 3, &result);
        assert(success == true);
    }
    
    assert(ctx->total_distributions == 10);
    assert(ctx->min_compute_time_ns <= ctx->max_compute_time_ns);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// MULTI-CHILD DISTRIBUTION TESTS
// ============================================================================

int test_multi_child_distribution(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    uint64_t child_ids[] = {3, 4, 6, 8};
    TimedMultiChildResult result;
    
    bool success = timed_calculate_multi_child_distribution(
        ctx, 5, child_ids, 4, &result
    );
    
    assert(success == true);
    assert(result.distribution.is_valid == true);
    assert(result.distribution.num_children == 4);
    assert(result.total_time_ns > 0);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_multi_child_invalid(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    TimedMultiChildResult result;
    
    // NULL child_ids
    bool success = timed_calculate_multi_child_distribution(
        ctx, 5, NULL, 4, &result
    );
    assert(success == false);
    
    // Zero children
    uint64_t child_ids[] = {3, 4};
    success = timed_calculate_multi_child_distribution(
        ctx, 5, child_ids, 0, &result
    );
    assert(success == false);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// TIMING CONTROL TESTS
// ============================================================================

int test_disable_timing(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Disable timing
    timed_work_distribution_set_timing_enabled(ctx, false);
    assert(ctx->timing_enabled == false);
    
    TimedDistributionResult result;
    bool success = timed_calculate_work_distribution(ctx, 5, 3, &result);
    assert(success == true);
    
    // Wait time should be minimal when timing disabled
    assert(result.wait_time_ns < 1000000);  // Less than 1ms
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_adaptive_mode(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Enable adaptive mode
    timed_work_distribution_set_adaptive(ctx, true);
    assert(ctx->adaptive_timing == true);
    
    TimedDistributionResult result;
    bool success = timed_calculate_work_distribution(ctx, 5, 3, &result);
    assert(success == true);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_change_frequency(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Change to 528 Hz
    bool success = timed_work_distribution_set_frequency(ctx, FREQ_528_HZ);
    assert(success == true);
    
    TimedDistributionResult result;
    success = timed_calculate_work_distribution(ctx, 5, 3, &result);
    assert(success == true);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// STATISTICS TESTS
// ============================================================================

int test_statistics(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Perform some distributions
    for (int i = 0; i < 5; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
    }
    
    TimingStatistics stats;
    timed_work_distribution_get_stats(ctx, &stats);
    
    assert(stats.total_distributions == 5);
    assert(stats.avg_wait_time_ns > 0);
    assert(stats.avg_compute_time_ns > 0);
    assert(stats.min_wait_time_ns <= stats.max_wait_time_ns);
    assert(stats.min_compute_time_ns <= stats.max_compute_time_ns);
    assert(stats.wait_overhead_percent >= 0.0);
    assert(stats.wait_overhead_percent <= 100.0);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_reset_statistics(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Perform some distributions
    TimedDistributionResult result;
    timed_calculate_work_distribution(ctx, 5, 3, &result);
    assert(ctx->total_distributions == 1);
    
    // Reset statistics
    timed_work_distribution_reset_stats(ctx);
    assert(ctx->total_distributions == 0);
    assert(ctx->total_wait_time_ns == 0);
    assert(ctx->total_compute_time_ns == 0);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_print_statistics(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Perform some distributions
    for (int i = 0; i < 3; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
    }
    
    printf("\n");
    timed_work_distribution_print_stats(ctx);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// UTILITY TESTS
// ============================================================================

int test_timing_overhead(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Perform distributions
    for (int i = 0; i < 5; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
    }
    
    double overhead = calculate_timing_overhead(ctx);
    assert(overhead >= 0.0);
    assert(overhead <= 100.0);
    
    printf("  Timing overhead: %.2f%%\n", overhead);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

int test_excessive_overhead(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, false);
    assert(ctx != NULL);
    
    // Perform distributions
    TimedDistributionResult result;
    timed_calculate_work_distribution(ctx, 5, 3, &result);
    
    bool excessive = is_timing_overhead_excessive(ctx);
    // Should not be excessive for normal operations
    printf("  Overhead excessive: %s\n", excessive ? "yes" : "no");
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// MULTI-THREADED TESTS
// ============================================================================

typedef struct {
    TimedWorkDistributionContext* ctx;
    int thread_id;
    int num_iterations;
} ThreadTestData;

void* thread_test_func(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->num_iterations; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(data->ctx, 5, 3, &result);
    }
    
    return NULL;
}

int test_multi_threaded(void) {
    const int num_threads = 4;
    const int iterations = 10;
    
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(num_threads, false);
    assert(ctx != NULL);
    
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].ctx = ctx;
        thread_data[i].thread_id = i;
        thread_data[i].num_iterations = iterations;
        
        int rc = pthread_create(&threads[i], NULL, thread_test_func, &thread_data[i]);
        assert(rc == 0);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Check statistics
    assert(ctx->total_distributions == num_threads * iterations);
    
    printf("\n");
    timed_work_distribution_print_stats(ctx);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("=== Work Distribution Timing Tests ===\n\n");
    
    // Basic tests
    TEST(create_destroy);
    TEST(create_adaptive);
    TEST(create_invalid_threads);
    
    // Single distribution tests
    TEST(single_distribution);
    TEST(invalid_distribution);
    TEST(multiple_distributions);
    
    // Multi-child tests
    TEST(multi_child_distribution);
    TEST(multi_child_invalid);
    
    // Timing control tests
    TEST(disable_timing);
    TEST(adaptive_mode);
    TEST(change_frequency);
    
    // Statistics tests
    TEST(statistics);
    TEST(reset_statistics);
    TEST(print_statistics);
    
    // Utility tests
    TEST(timing_overhead);
    TEST(excessive_overhead);
    
    // Multi-threaded tests
    TEST(multi_threaded);
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}