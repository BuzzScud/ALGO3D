/**
 * Unit Tests for Epoch Synchronization
 * 
 * Tests the Schumann resonance (7.83 Hz) epoch synchronization system.
 */

#include "ai/cllm_epoch_sync.h"
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
    EpochSyncContext* ctx = epoch_sync_create(4, 1000, false);
    assert(ctx != NULL);
    assert(ctx->epoch_barrier != NULL);
    assert(ctx->samples_per_epoch == 1000);
    assert(ctx->sync_enabled == true);
    assert(ctx->adaptive_sync == false);
    assert(ctx->num_threads == 4);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_create_adaptive(void) {
    EpochSyncContext* ctx = epoch_sync_create(4, 1000, true);
    assert(ctx != NULL);
    assert(ctx->adaptive_sync == true);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_create_invalid(void) {
    EpochSyncContext* ctx = epoch_sync_create(0, 1000, false);
    assert(ctx == NULL);
    
    ctx = epoch_sync_create(4, 0, false);
    assert(ctx == NULL);
    
    return 1;
}

// ============================================================================
// EPOCH MANAGEMENT TESTS
// ============================================================================

int test_start_epoch(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    bool success = epoch_sync_start_epoch(ctx);
    assert(success == true);
    assert(ctx->current_epoch == 1);
    assert(ctx->samples_processed == 0);
    assert(ctx->gradients_accumulated == 0);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_record_samples(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    epoch_sync_record_samples(ctx, 100);
    assert(ctx->samples_processed == 100);
    
    epoch_sync_record_samples(ctx, 200);
    assert(ctx->samples_processed == 300);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_record_gradients(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    epoch_sync_record_gradients(ctx, 50);
    assert(ctx->gradients_accumulated == 50);
    
    epoch_sync_record_gradients(ctx, 75);
    assert(ctx->gradients_accumulated == 125);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_epoch_complete(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    assert(epoch_sync_is_epoch_complete(ctx) == false);
    
    epoch_sync_record_samples(ctx, 500);
    assert(epoch_sync_is_epoch_complete(ctx) == false);
    
    epoch_sync_record_samples(ctx, 500);
    assert(epoch_sync_is_epoch_complete(ctx) == true);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_end_epoch(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    epoch_sync_record_samples(ctx, 1000);
    epoch_sync_record_gradients(ctx, 100);
    
    EpochTimingResult result;
    bool success = epoch_sync_end_epoch(ctx, &result);
    
    assert(success == true);
    assert(result.epoch_number == 1);
    assert(result.samples_processed == 1000);
    assert(result.gradients_accumulated == 100);
    assert(result.epoch_time_ns > 0);
    assert(result.synced == true);
    
    assert(ctx->total_epochs == 1);
    assert(ctx->total_gradients == 100);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_multiple_epochs(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    for (int i = 0; i < 5; i++) {
        epoch_sync_start_epoch(ctx);
        epoch_sync_record_samples(ctx, 1000);
        epoch_sync_record_gradients(ctx, 100);
        
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
        
        assert(result.epoch_number == (uint64_t)(i + 1));
    }
    
    assert(ctx->total_epochs == 5);
    assert(ctx->total_gradients == 500);
    
    epoch_sync_destroy(ctx);
    return 1;
}

// ============================================================================
// SYNCHRONIZATION CONTROL TESTS
// ============================================================================

int test_disable_sync(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_set_enabled(ctx, false);
    assert(ctx->sync_enabled == false);
    
    epoch_sync_start_epoch(ctx);
    epoch_sync_record_samples(ctx, 1000);
    
    EpochTimingResult result;
    epoch_sync_end_epoch(ctx, &result);
    
    assert(result.synced == false);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_adaptive_mode(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_set_adaptive(ctx, true);
    assert(ctx->adaptive_sync == true);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_change_frequency(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    bool success = epoch_sync_set_frequency(ctx, FREQ_432_HZ);
    assert(success == true);
    
    epoch_sync_destroy(ctx);
    return 1;
}

// ============================================================================
// STATISTICS TESTS
// ============================================================================

int test_statistics(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    // Run a few epochs
    for (int i = 0; i < 3; i++) {
        epoch_sync_start_epoch(ctx);
        epoch_sync_record_samples(ctx, 1000);
        epoch_sync_record_gradients(ctx, 100);
        
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
    }
    
    EpochStatistics stats;
    epoch_sync_get_statistics(ctx, &stats);
    
    assert(stats.total_epochs == 3);
    assert(stats.total_samples == 3000);
    assert(stats.total_gradients == 300);
    assert(stats.avg_epoch_time_ns > 0);
    assert(stats.min_epoch_time_ns <= stats.max_epoch_time_ns);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_reset_statistics(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    epoch_sync_record_samples(ctx, 1000);
    EpochTimingResult result;
    epoch_sync_end_epoch(ctx, &result);
    
    assert(ctx->total_epochs == 1);
    
    epoch_sync_reset_statistics(ctx);
    assert(ctx->total_epochs == 0);
    assert(ctx->total_gradients == 0);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_print_statistics(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    for (int i = 0; i < 2; i++) {
        epoch_sync_start_epoch(ctx);
        epoch_sync_record_samples(ctx, 1000);
        epoch_sync_record_gradients(ctx, 100);
        
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
    }
    
    printf("\n");
    epoch_sync_print_statistics(ctx);
    
    epoch_sync_destroy(ctx);
    return 1;
}

// ============================================================================
// UTILITY TESTS
// ============================================================================

int test_overhead_calculation(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    epoch_sync_record_samples(ctx, 1000);
    EpochTimingResult result;
    epoch_sync_end_epoch(ctx, &result);
    
    double overhead = epoch_sync_calculate_overhead(ctx);
    assert(overhead >= 0.0);
    assert(overhead <= 100.0);
    
    printf("  Overhead: %.2f%%\n", overhead);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_excessive_overhead(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    epoch_sync_start_epoch(ctx);
    epoch_sync_record_samples(ctx, 1000);
    EpochTimingResult result;
    epoch_sync_end_epoch(ctx, &result);
    
    bool excessive = epoch_sync_is_overhead_excessive(ctx);
    printf("  Overhead excessive: %s\n", excessive ? "yes" : "no");
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_throughput(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    for (int i = 0; i < 3; i++) {
        epoch_sync_start_epoch(ctx);
        epoch_sync_record_samples(ctx, 1000);
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
    }
    
    double throughput = epoch_sync_get_throughput(ctx);
    double epoch_rate = epoch_sync_get_epoch_rate(ctx);
    
    assert(throughput > 0.0);
    assert(epoch_rate > 0.0);
    
    printf("  Throughput: %.2f samples/sec\n", throughput);
    printf("  Epoch rate: %.2f epochs/sec\n", epoch_rate);
    
    epoch_sync_destroy(ctx);
    return 1;
}

int test_get_epoch_numbers(void) {
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    assert(ctx != NULL);
    
    assert(epoch_sync_get_current_epoch(ctx) == 0);
    assert(epoch_sync_get_total_epochs(ctx) == 0);
    
    epoch_sync_start_epoch(ctx);
    assert(epoch_sync_get_current_epoch(ctx) == 1);
    
    EpochTimingResult result;
    epoch_sync_end_epoch(ctx, &result);
    assert(epoch_sync_get_total_epochs(ctx) == 1);
    
    epoch_sync_destroy(ctx);
    return 1;
}

// ============================================================================
// MULTI-THREADED TESTS
// ============================================================================

typedef struct {
    EpochSyncContext* ctx;
    int thread_id;
    int num_epochs;
} ThreadTestData;

void* thread_epoch_func(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->num_epochs; i++) {
        // Simulate sample processing
        epoch_sync_record_samples(data->ctx, 250);
        
        // Simulate gradient accumulation
        epoch_sync_record_gradients(data->ctx, 25);
        
        // Check if epoch complete
        if (epoch_sync_is_epoch_complete(data->ctx)) {
            EpochTimingResult result;
            epoch_sync_end_epoch(data->ctx, &result);
            
            // Start new epoch
            epoch_sync_start_epoch(data->ctx);
        }
    }
    
    return NULL;
}

int test_multi_threaded(void) {
    const int num_threads = 4;
    const int epochs_per_thread = 5;
    
    EpochSyncContext* ctx = epoch_sync_create(num_threads, 1000, false);
    assert(ctx != NULL);
    
    // Start first epoch
    epoch_sync_start_epoch(ctx);
    
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].ctx = ctx;
        thread_data[i].thread_id = i;
        thread_data[i].num_epochs = epochs_per_thread;
        
        int rc = pthread_create(&threads[i], NULL, thread_epoch_func, &thread_data[i]);
        assert(rc == 0);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n");
    epoch_sync_print_statistics(ctx);
    
    epoch_sync_destroy(ctx);
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("=== Epoch Synchronization Tests ===\n\n");
    
    // Basic tests
    TEST(create_destroy);
    TEST(create_adaptive);
    TEST(create_invalid);
    
    // Epoch management tests
    TEST(start_epoch);
    TEST(record_samples);
    TEST(record_gradients);
    TEST(epoch_complete);
    TEST(end_epoch);
    TEST(multiple_epochs);
    
    // Synchronization control tests
    TEST(disable_sync);
    TEST(adaptive_mode);
    TEST(change_frequency);
    
    // Statistics tests
    TEST(statistics);
    TEST(reset_statistics);
    TEST(print_statistics);
    
    // Utility tests
    TEST(overhead_calculation);
    TEST(excessive_overhead);
    TEST(throughput);
    TEST(get_epoch_numbers);
    
    // Multi-threaded tests
    TEST(multi_threaded);
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}