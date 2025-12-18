/**
 * Integration Tests for Cymatic Timing System
 * 
 * Tests integration of cymatic timing with the complete training pipeline:
 * - Work distribution timing with batch processing
 * - Epoch synchronization with gradient accumulation
 * - Combined system integration
 * - Multi-threaded training coordination
 * 
 * Phase 5, Day 18 Afternoon
 */

#include "ai/cllm_work_distribution_timing.h"
#include "ai/cllm_epoch_sync.h"
#include "ai/cllm_work_distribution.h"
#include "ai/cllm_batch_splitting.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

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
// INTEGRATION TEST 1: WORK DISTRIBUTION WITH BATCH SPLITTING
// ============================================================================

int test_work_distribution_batch_integration(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, true);
    assert(ctx != NULL);
    
    // Calculate distribution
    TimedDistributionResult result;
    bool success = timed_calculate_work_distribution(ctx, 5, 3, &result);
    assert(success == true);
    
    // Verify distribution is valid
    assert(result.distribution.is_valid == true);
    assert(result.distribution.parent_keeps > 0.0);
    assert(result.distribution.child_gets > 0.0);
    
    // Verify timing measurements
    assert(result.total_time_ns > 0);
    assert(result.compute_time_ns > 0);
    
    // Verify ratios sum to ~1.0
    double sum = result.distribution.parent_keeps + result.distribution.child_gets;
    assert(sum > 0.99 && sum < 1.01);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 2: EPOCH SYNC WITH SAMPLE TRACKING
// ============================================================================

int test_epoch_sync_sample_integration(void) {
    const uint64_t samples_per_epoch = 1000;
    EpochSyncContext* ctx = epoch_sync_create(1, samples_per_epoch, true);
    assert(ctx != NULL);
    
    // Start epoch
    epoch_sync_start_epoch(ctx);
    assert(ctx->current_epoch == 1);
    
    // Process samples
    for (uint64_t i = 0; i < samples_per_epoch; i++) {
        epoch_sync_record_samples(ctx, 1);
    }
    
    // Verify epoch complete
    assert(epoch_sync_is_epoch_complete(ctx) == true);
    
    // End epoch
    EpochTimingResult result;
    bool success = epoch_sync_end_epoch(ctx, &result);
    assert(success == true);
    assert(result.samples_processed == samples_per_epoch);
    
    epoch_sync_destroy(ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 3: GRADIENT ACCUMULATION TRACKING
// ============================================================================

int test_gradient_accumulation_integration(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, true);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    
    // Simulate gradient accumulation
    for (int batch = 0; batch < 10; batch++) {
        epoch_sync_record_samples(ctx, 100);
        epoch_sync_record_gradients(ctx, 100);
    }
    
    // Verify tracking
    assert(ctx->samples_processed == 1000);
    assert(ctx->gradients_accumulated == 1000);
    
    // End epoch
    EpochTimingResult result;
    epoch_sync_end_epoch(ctx, &result);
    
    assert(result.gradients_accumulated == 1000);
    assert(ctx->total_gradients == 1000);
    
    epoch_sync_destroy(ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 4: COMBINED WORK DISTRIBUTION AND EPOCH SYNC
// ============================================================================

int test_combined_timing_integration(void) {
    const int num_threads = 1;
    const uint64_t samples_per_epoch = 1000;
    const int distributions_per_sample = 5;
    
    // Create both contexts
    TimedWorkDistributionContext* work_ctx = 
        timed_work_distribution_create(num_threads, true);
    EpochSyncContext* epoch_ctx = 
        epoch_sync_create(num_threads, samples_per_epoch, true);
    
    assert(work_ctx != NULL);
    assert(epoch_ctx != NULL);
    
    // Run epoch with work distributions
    epoch_sync_start_epoch(epoch_ctx);
    
    for (uint64_t sample = 0; sample < samples_per_epoch; sample++) {
        // Calculate work distributions
        for (int dist = 0; dist < distributions_per_sample; dist++) {
            TimedDistributionResult work_result;
            timed_calculate_work_distribution(work_ctx, 5, 3, &work_result);
            assert(work_result.distribution.is_valid == true);
        }
        
        // Record sample
        epoch_sync_record_samples(epoch_ctx, 1);
    }
    
    // End epoch
    EpochTimingResult epoch_result;
    epoch_sync_end_epoch(epoch_ctx, &epoch_result);
    
    assert(epoch_result.samples_processed == samples_per_epoch);
    
    // Verify both systems tracked operations
    assert(work_ctx->total_distributions == samples_per_epoch * distributions_per_sample);
    assert(epoch_ctx->total_epochs == 1);
    
    timed_work_distribution_destroy(work_ctx);
    epoch_sync_destroy(epoch_ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 5: MULTI-EPOCH TRAINING SIMULATION
// ============================================================================

int test_multi_epoch_training_simulation(void) {
    const int num_epochs = 5;
    const uint64_t samples_per_epoch = 1000;
    
    TimedWorkDistributionContext* work_ctx = 
        timed_work_distribution_create(1, true);
    EpochSyncContext* epoch_ctx = 
        epoch_sync_create(1, samples_per_epoch, true);
    
    assert(work_ctx != NULL);
    assert(epoch_ctx != NULL);
    
    // Training loop
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        epoch_sync_start_epoch(epoch_ctx);
        
        for (uint64_t sample = 0; sample < samples_per_epoch; sample++) {
            // Calculate work distribution
            TimedDistributionResult work_result;
            timed_calculate_work_distribution(work_ctx, 5, 3, &work_result);
            
            // Record progress
            epoch_sync_record_samples(epoch_ctx, 1);
            epoch_sync_record_gradients(epoch_ctx, 1);
        }
        
        // End epoch
        EpochTimingResult epoch_result;
        epoch_sync_end_epoch(epoch_ctx, &epoch_result);
        
        assert(epoch_result.epoch_number == (uint64_t)(epoch + 1));
        assert(epoch_result.samples_processed == samples_per_epoch);
    }
    
    // Verify final state
    assert(epoch_ctx->total_epochs == (uint64_t)num_epochs);
    assert(work_ctx->total_distributions == num_epochs * samples_per_epoch);
    
    timed_work_distribution_destroy(work_ctx);
    epoch_sync_destroy(epoch_ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 6: MULTI-THREADED TRAINING COORDINATION
// ============================================================================

typedef struct {
    TimedWorkDistributionContext* work_ctx;
    EpochSyncContext* epoch_ctx;
    int thread_id;
    uint64_t samples_per_thread;
} IntegrationThreadData;

void* integration_thread_func(void* arg) {
    IntegrationThreadData* data = (IntegrationThreadData*)arg;
    
    for (uint64_t i = 0; i < data->samples_per_thread; i++) {
        // Calculate work distribution
        TimedDistributionResult work_result;
        timed_calculate_work_distribution(data->work_ctx, 5, 3, &work_result);
        
        // Record progress
        epoch_sync_record_samples(data->epoch_ctx, 1);
        epoch_sync_record_gradients(data->epoch_ctx, 1);
        
        // Check if epoch complete
        if (epoch_sync_is_epoch_complete(data->epoch_ctx)) {
            break;
        }
    }
    
    return NULL;
}

int test_multi_threaded_integration(void) {
    const int num_threads = 4;
    const uint64_t samples_per_epoch = 1000;
    const uint64_t samples_per_thread = samples_per_epoch / num_threads;
    
    // Create contexts
    TimedWorkDistributionContext* work_ctx = 
        timed_work_distribution_create(num_threads, true);
    EpochSyncContext* epoch_ctx = 
        epoch_sync_create(num_threads, samples_per_epoch, true);
    
    assert(work_ctx != NULL);
    assert(epoch_ctx != NULL);
    
    // Start epoch
    epoch_sync_start_epoch(epoch_ctx);
    
    // Create threads
    pthread_t threads[num_threads];
    IntegrationThreadData thread_data[num_threads];
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].work_ctx = work_ctx;
        thread_data[i].epoch_ctx = epoch_ctx;
        thread_data[i].thread_id = i;
        thread_data[i].samples_per_thread = samples_per_thread;
        
        pthread_create(&threads[i], NULL, integration_thread_func, &thread_data[i]);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // End epoch
    EpochTimingResult epoch_result;
    epoch_sync_end_epoch(epoch_ctx, &epoch_result);
    
    assert(epoch_result.samples_processed >= samples_per_epoch);
    
    printf("\n");
    timed_work_distribution_print_stats(work_ctx);
    epoch_sync_print_statistics(epoch_ctx);
    
    timed_work_distribution_destroy(work_ctx);
    epoch_sync_destroy(epoch_ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 7: PERFORMANCE OVERHEAD VALIDATION
// ============================================================================

int test_overhead_validation(void) {
    const int iterations = 1000;
    
    TimedWorkDistributionContext* work_ctx = 
        timed_work_distribution_create(1, true);
    EpochSyncContext* epoch_ctx = 
        epoch_sync_create(1, 1000, true);
    
    assert(work_ctx != NULL);
    assert(epoch_ctx != NULL);
    
    // Run operations
    epoch_sync_start_epoch(epoch_ctx);
    
    for (int i = 0; i < iterations; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(work_ctx, 5, 3, &result);
        epoch_sync_record_samples(epoch_ctx, 1);
    }
    
    EpochTimingResult epoch_result;
    epoch_sync_end_epoch(epoch_ctx, &epoch_result);
    
    // Verify overhead is acceptable
    double work_overhead = calculate_timing_overhead(work_ctx);
    double epoch_overhead = epoch_sync_calculate_overhead(epoch_ctx);
    
    printf("  Work distribution overhead: %.2f%%\n", work_overhead);
    printf("  Epoch sync overhead: %.2f%%\n", epoch_overhead);
    
    // Overhead should be reasonable
    assert(work_overhead < 10.0);  // < 10%
    assert(epoch_overhead < 15.0);  // < 15%
    
    timed_work_distribution_destroy(work_ctx);
    epoch_sync_destroy(epoch_ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 8: STATISTICS CONSISTENCY
// ============================================================================

int test_statistics_consistency(void) {
    TimedWorkDistributionContext* work_ctx = 
        timed_work_distribution_create(1, true);
    EpochSyncContext* epoch_ctx = 
        epoch_sync_create(1, 100, true);
    
    assert(work_ctx != NULL);
    assert(epoch_ctx != NULL);
    
    // Run 3 epochs
    for (int epoch = 0; epoch < 3; epoch++) {
        epoch_sync_start_epoch(epoch_ctx);
        
        for (int i = 0; i < 100; i++) {
            TimedDistributionResult result;
            timed_calculate_work_distribution(work_ctx, 5, 3, &result);
            epoch_sync_record_samples(epoch_ctx, 1);
        }
        
        EpochTimingResult result;
        epoch_sync_end_epoch(epoch_ctx, &result);
    }
    
    // Get statistics
    TimingStatistics work_stats;
    timed_work_distribution_get_stats(work_ctx, &work_stats);
    
    EpochStatistics epoch_stats;
    epoch_sync_get_statistics(epoch_ctx, &epoch_stats);
    
    // Verify consistency
    assert(work_stats.total_distributions == 300);  // 3 epochs * 100 samples
    assert(epoch_stats.total_epochs == 3);
    assert(epoch_stats.total_samples == 300);
    
    timed_work_distribution_destroy(work_ctx);
    epoch_sync_destroy(epoch_ctx);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 9: ADAPTIVE MODE EFFECTIVENESS
// ============================================================================

int test_adaptive_mode_effectiveness(void) {
    const int iterations = 1000;
    
    // Non-adaptive
    TimedWorkDistributionContext* ctx_non_adaptive = 
        timed_work_distribution_create(1, false);
    
    for (int i = 0; i < iterations; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx_non_adaptive, 5, 3, &result);
    }
    
    double non_adaptive_overhead = calculate_timing_overhead(ctx_non_adaptive);
    
    // Adaptive
    TimedWorkDistributionContext* ctx_adaptive = 
        timed_work_distribution_create(1, true);
    
    for (int i = 0; i < iterations; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx_adaptive, 5, 3, &result);
    }
    
    double adaptive_overhead = calculate_timing_overhead(ctx_adaptive);
    
    printf("  Non-adaptive overhead: %.2f%%\n", non_adaptive_overhead);
    printf("  Adaptive overhead: %.2f%%\n", adaptive_overhead);
    printf("  Improvement: %.2f%%\n", non_adaptive_overhead - adaptive_overhead);
    
    // Adaptive should have lower or equal overhead
    assert(adaptive_overhead <= non_adaptive_overhead + 1.0);  // Allow 1% margin
    
    timed_work_distribution_destroy(ctx_non_adaptive);
    timed_work_distribution_destroy(ctx_adaptive);
    return 1;
}

// ============================================================================
// INTEGRATION TEST 10: FREQUENCY CHANGE DURING TRAINING
// ============================================================================

int test_frequency_change_integration(void) {
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(1, true);
    assert(ctx != NULL);
    
    // Start with 432 Hz
    for (int i = 0; i < 100; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
    }
    
    // Change to 528 Hz
    bool success = timed_work_distribution_set_frequency(ctx, FREQ_528_HZ);
    assert(success == true);
    
    // Continue with new frequency
    for (int i = 0; i < 100; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
    }
    
    // Verify operations completed
    assert(ctx->total_distributions == 200);
    
    timed_work_distribution_destroy(ctx);
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("=== Cymatic Timing Integration Tests ===\n\n");
    
    // Integration tests
    TEST(work_distribution_batch_integration);
    TEST(epoch_sync_sample_integration);
    TEST(gradient_accumulation_integration);
    TEST(combined_timing_integration);
    TEST(multi_epoch_training_simulation);
    TEST(multi_threaded_integration);
    TEST(overhead_validation);
    TEST(statistics_consistency);
    TEST(adaptive_mode_effectiveness);
    TEST(frequency_change_integration);
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}