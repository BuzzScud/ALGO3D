/**
 * Performance Benchmark for Cymatic Timing System
 * 
 * Benchmarks the complete cymatic timing integration:
 * - Work distribution timing (432 Hz)
 * - Epoch synchronization (7.83 Hz)
 * - Combined overhead analysis
 * - Comparison with baseline (no timing)
 * 
 * Phase 5, Day 18 Morning
 */

#include "ai/cllm_work_distribution_timing.h"
#include "ai/cllm_epoch_sync.h"
#include "ai/cllm_work_distribution.h"
#include "ai/cllm_cymatic_sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// ============================================================================
// TIMING UTILITIES
// ============================================================================

static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static double ns_to_ms(uint64_t ns) {
    return ns / 1e6;
}

static double ns_to_sec(uint64_t ns) {
    return ns / 1e9;
}

// ============================================================================
// BENCHMARK 1: WORK DISTRIBUTION TIMING OVERHEAD
// ============================================================================

void benchmark_work_distribution_timing(void) {
    printf("\n=== Benchmark 1: Work Distribution Timing (432 Hz) ===\n");
    
    const int num_threads = 4;
    const int iterations = 10000;
    
    // Baseline: No timing
    printf("\nBaseline (no timing):\n");
    uint64_t baseline_start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        WorkDistribution dist = calculate_work_distribution(5, 3);
        (void)dist; // Prevent optimization
    }
    uint64_t baseline_time = get_time_ns() - baseline_start;
    printf("  Total time: %.3f ms\n", ns_to_ms(baseline_time));
    printf("  Per operation: %.3f ns\n", (double)baseline_time / iterations);
    printf("  Operations/sec: %.2f M\n", iterations / ns_to_sec(baseline_time) / 1e6);
    
    // With timing (non-adaptive)
    printf("\nWith 432 Hz timing (non-adaptive):\n");
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(num_threads, false);
    
    uint64_t timed_start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
    }
    uint64_t timed_time = get_time_ns() - timed_start;
    
    printf("  Total time: %.3f ms\n", ns_to_ms(timed_time));
    printf("  Per operation: %.3f ns\n", (double)timed_time / iterations);
    printf("  Operations/sec: %.2f M\n", iterations / ns_to_sec(timed_time) / 1e6);
    
    double overhead_percent = ((double)timed_time - baseline_time) / baseline_time * 100.0;
    printf("  Overhead: %.2f%%\n", overhead_percent);
    
    timed_work_distribution_destroy(ctx);
    
    // With timing (adaptive)
    printf("\nWith 432 Hz timing (adaptive):\n");
    ctx = timed_work_distribution_create(num_threads, true);
    
    timed_start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
    }
    timed_time = get_time_ns() - timed_start;
    
    printf("  Total time: %.3f ms\n", ns_to_ms(timed_time));
    printf("  Per operation: %.3f ns\n", (double)timed_time / iterations);
    printf("  Operations/sec: %.2f M\n", iterations / ns_to_sec(timed_time) / 1e6);
    
    overhead_percent = ((double)timed_time - baseline_time) / baseline_time * 100.0;
    printf("  Overhead: %.2f%%\n", overhead_percent);
    
    timed_work_distribution_print_stats(ctx);
    timed_work_distribution_destroy(ctx);
}

// ============================================================================
// BENCHMARK 2: EPOCH SYNCHRONIZATION OVERHEAD
// ============================================================================

void benchmark_epoch_synchronization(void) {
    printf("\n=== Benchmark 2: Epoch Synchronization (7.83 Hz) ===\n");
    
    const int num_threads = 1;
    const int num_epochs = 100;
    const uint64_t samples_per_epoch = 1000;
    
    // Baseline: No synchronization
    printf("\nBaseline (no synchronization):\n");
    uint64_t baseline_start = get_time_ns();
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        // Simulate epoch processing
        for (uint64_t i = 0; i < samples_per_epoch; i++) {
            // Minimal work
            volatile int x = i * 2;
            (void)x;
        }
    }
    uint64_t baseline_time = get_time_ns() - baseline_start;
    printf("  Total time: %.3f ms\n", ns_to_ms(baseline_time));
    printf("  Per epoch: %.3f ms\n", ns_to_ms(baseline_time) / num_epochs);
    printf("  Epochs/sec: %.2f\n", num_epochs / ns_to_sec(baseline_time));
    
    // With synchronization (non-adaptive)
    printf("\nWith 7.83 Hz synchronization (non-adaptive):\n");
    EpochSyncContext* ctx = epoch_sync_create(num_threads, samples_per_epoch, false);
    
    uint64_t synced_start = get_time_ns();
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        epoch_sync_start_epoch(ctx);
        
        for (uint64_t i = 0; i < samples_per_epoch; i++) {
            epoch_sync_record_samples(ctx, 1);
            // Minimal work
            volatile int x = i * 2;
            (void)x;
        }
        
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
    }
    uint64_t synced_time = get_time_ns() - synced_start;
    
    printf("  Total time: %.3f ms\n", ns_to_ms(synced_time));
    printf("  Per epoch: %.3f ms\n", ns_to_ms(synced_time) / num_epochs);
    printf("  Epochs/sec: %.2f\n", num_epochs / ns_to_sec(synced_time));
    
    double overhead_percent = ((double)synced_time - baseline_time) / baseline_time * 100.0;
    printf("  Overhead: %.2f%%\n", overhead_percent);
    
    epoch_sync_print_statistics(ctx);
    epoch_sync_destroy(ctx);
    
    // With synchronization (adaptive)
    printf("\nWith 7.83 Hz synchronization (adaptive):\n");
    ctx = epoch_sync_create(num_threads, samples_per_epoch, true);
    
    synced_start = get_time_ns();
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        epoch_sync_start_epoch(ctx);
        
        for (uint64_t i = 0; i < samples_per_epoch; i++) {
            epoch_sync_record_samples(ctx, 1);
            volatile int x = i * 2;
            (void)x;
        }
        
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
    }
    synced_time = get_time_ns() - synced_start;
    
    printf("  Total time: %.3f ms\n", ns_to_ms(synced_time));
    printf("  Per epoch: %.3f ms\n", ns_to_ms(synced_time) / num_epochs);
    printf("  Epochs/sec: %.2f\n", num_epochs / ns_to_sec(synced_time));
    
    overhead_percent = ((double)synced_time - baseline_time) / baseline_time * 100.0;
    printf("  Overhead: %.2f%%\n", overhead_percent);
    
    epoch_sync_print_statistics(ctx);
    epoch_sync_destroy(ctx);
}

// ============================================================================
// BENCHMARK 3: MULTI-THREADED WORK DISTRIBUTION
// ============================================================================

typedef struct {
    TimedWorkDistributionContext* ctx;
    int thread_id;
    int iterations;
    uint64_t total_time_ns;
} WorkDistThreadData;

void* work_dist_thread_func(void* arg) {
    WorkDistThreadData* data = (WorkDistThreadData*)arg;
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < data->iterations; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(data->ctx, 5, 3, &result);
    }
    
    data->total_time_ns = get_time_ns() - start;
    
    return NULL;
}

void benchmark_multi_threaded_work_distribution(void) {
    printf("\n=== Benchmark 3: Multi-Threaded Work Distribution ===\n");
    
    const int num_threads = 4;
    const int iterations_per_thread = 1000;
    
    printf("\nConfiguration:\n");
    printf("  Threads: %d\n", num_threads);
    printf("  Iterations per thread: %d\n", iterations_per_thread);
    printf("  Total operations: %d\n", num_threads * iterations_per_thread);
    
    TimedWorkDistributionContext* ctx = timed_work_distribution_create(num_threads, true);
    
    pthread_t threads[num_threads];
    WorkDistThreadData thread_data[num_threads];
    
    uint64_t start = get_time_ns();
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].ctx = ctx;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations_per_thread;
        thread_data[i].total_time_ns = 0;
        
        pthread_create(&threads[i], NULL, work_dist_thread_func, &thread_data[i]);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    uint64_t total_time = get_time_ns() - start;
    
    printf("\nResults:\n");
    printf("  Total time: %.3f ms\n", ns_to_ms(total_time));
    printf("  Operations/sec: %.2f M\n", 
           (num_threads * iterations_per_thread) / ns_to_sec(total_time) / 1e6);
    
    printf("\nPer-thread times:\n");
    for (int i = 0; i < num_threads; i++) {
        printf("  Thread %d: %.3f ms\n", i, ns_to_ms(thread_data[i].total_time_ns));
    }
    
    timed_work_distribution_print_stats(ctx);
    timed_work_distribution_destroy(ctx);
}

// ============================================================================
// BENCHMARK 4: MULTI-THREADED EPOCH SYNCHRONIZATION
// ============================================================================

typedef struct {
    EpochSyncContext* ctx;
    int thread_id;
    int num_epochs;
    uint64_t samples_per_thread;
    uint64_t total_time_ns;
} EpochThreadData;

void* epoch_thread_func(void* arg) {
    EpochThreadData* data = (EpochThreadData*)arg;
    
    uint64_t start = get_time_ns();
    
    for (int epoch = 0; epoch < data->num_epochs; epoch++) {
        // Process samples
        for (uint64_t i = 0; i < data->samples_per_thread; i++) {
            epoch_sync_record_samples(data->ctx, 1);
            epoch_sync_record_gradients(data->ctx, 1);
        }
        
        // Check if epoch complete and synchronize
        if (epoch_sync_is_epoch_complete(data->ctx)) {
            EpochTimingResult result;
            epoch_sync_end_epoch(data->ctx, &result);
            epoch_sync_start_epoch(data->ctx);
        }
    }
    
    data->total_time_ns = get_time_ns() - start;
    
    return NULL;
}

void benchmark_multi_threaded_epoch_sync(void) {
    printf("\n=== Benchmark 4: Multi-Threaded Epoch Synchronization ===\n");
    
    const int num_threads = 4;
    const int num_epochs = 20;
    const uint64_t samples_per_epoch = 1000;
    const uint64_t samples_per_thread = samples_per_epoch / num_threads;
    
    printf("\nConfiguration:\n");
    printf("  Threads: %d\n", num_threads);
    printf("  Epochs: %d\n", num_epochs);
    printf("  Samples per epoch: %lu\n", samples_per_epoch);
    printf("  Samples per thread: %lu\n", samples_per_thread);
    
    EpochSyncContext* ctx = epoch_sync_create(num_threads, samples_per_epoch, true);
    epoch_sync_start_epoch(ctx);
    
    pthread_t threads[num_threads];
    EpochThreadData thread_data[num_threads];
    
    uint64_t start = get_time_ns();
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].ctx = ctx;
        thread_data[i].thread_id = i;
        thread_data[i].num_epochs = num_epochs;
        thread_data[i].samples_per_thread = samples_per_thread;
        thread_data[i].total_time_ns = 0;
        
        pthread_create(&threads[i], NULL, epoch_thread_func, &thread_data[i]);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    uint64_t total_time = get_time_ns() - start;
    
    printf("\nResults:\n");
    printf("  Total time: %.3f ms\n", ns_to_ms(total_time));
    printf("  Epochs/sec: %.2f\n", num_epochs / ns_to_sec(total_time));
    printf("  Samples/sec: %.2f K\n", 
           (num_epochs * samples_per_epoch) / ns_to_sec(total_time) / 1e3);
    
    printf("\nPer-thread times:\n");
    for (int i = 0; i < num_threads; i++) {
        printf("  Thread %d: %.3f ms\n", i, ns_to_ms(thread_data[i].total_time_ns));
    }
    
    epoch_sync_print_statistics(ctx);
    epoch_sync_destroy(ctx);
}

// ============================================================================
// BENCHMARK 5: COMBINED OVERHEAD ANALYSIS
// ============================================================================

void benchmark_combined_overhead(void) {
    printf("\n=== Benchmark 5: Combined Overhead Analysis ===\n");
    
    const int num_threads = 4;
    const int num_epochs = 10;
    const uint64_t samples_per_epoch = 1000;
    const int distributions_per_sample = 10;
    
    printf("\nConfiguration:\n");
    printf("  Threads: %d\n", num_threads);
    printf("  Epochs: %d\n", num_epochs);
    printf("  Samples per epoch: %lu\n", samples_per_epoch);
    printf("  Distributions per sample: %d\n", distributions_per_sample);
    
    // Baseline: No timing
    printf("\nBaseline (no timing):\n");
    uint64_t baseline_start = get_time_ns();
    
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        for (uint64_t sample = 0; sample < samples_per_epoch; sample++) {
            for (int dist = 0; dist < distributions_per_sample; dist++) {
                WorkDistribution d = calculate_work_distribution(5, 3);
                (void)d;
            }
        }
    }
    
    uint64_t baseline_time = get_time_ns() - baseline_start;
    printf("  Total time: %.3f ms\n", ns_to_ms(baseline_time));
    
    // With both timing systems
    printf("\nWith combined timing (432 Hz + 7.83 Hz):\n");
    TimedWorkDistributionContext* work_ctx = timed_work_distribution_create(num_threads, true);
    EpochSyncContext* epoch_ctx = epoch_sync_create(num_threads, samples_per_epoch, true);
    
    uint64_t combined_start = get_time_ns();
    
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        epoch_sync_start_epoch(epoch_ctx);
        
        for (uint64_t sample = 0; sample < samples_per_epoch; sample++) {
            epoch_sync_record_samples(epoch_ctx, 1);
            
            for (int dist = 0; dist < distributions_per_sample; dist++) {
                TimedDistributionResult result;
                timed_calculate_work_distribution(work_ctx, 5, 3, &result);
            }
        }
        
        EpochTimingResult result;
        epoch_sync_end_epoch(epoch_ctx, &result);
    }
    
    uint64_t combined_time = get_time_ns() - combined_start;
    printf("  Total time: %.3f ms\n", ns_to_ms(combined_time));
    
    double overhead_percent = ((double)combined_time - baseline_time) / baseline_time * 100.0;
    printf("  Combined overhead: %.2f%%\n", overhead_percent);
    
    printf("\nBreakdown:\n");
    double work_overhead = calculate_timing_overhead(work_ctx);
    double epoch_overhead = epoch_sync_calculate_overhead(epoch_ctx);
    printf("  Work distribution overhead: %.2f%%\n", work_overhead);
    printf("  Epoch sync overhead: %.2f%%\n", epoch_overhead);
    
    timed_work_distribution_destroy(work_ctx);
    epoch_sync_destroy(epoch_ctx);
}

// ============================================================================
// MAIN BENCHMARK RUNNER
// ============================================================================

int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Cymatic Timing System Performance Benchmark           ║\n");
    printf("║     Phase 5, Day 18 - Performance & Optimization          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    benchmark_work_distribution_timing();
    benchmark_epoch_synchronization();
    benchmark_multi_threaded_work_distribution();
    benchmark_multi_threaded_epoch_sync();
    benchmark_combined_overhead();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    Benchmark Complete                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}