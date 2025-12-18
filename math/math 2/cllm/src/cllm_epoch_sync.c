/**
 * Epoch Synchronization with Schumann Resonance - Implementation
 * 
 * Implements epoch-level timing using 7.83 Hz Schumann resonance.
 */

#include "ai/cllm_epoch_sync.h"
#include "ai/cllm_cymatic_sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Get current time in nanoseconds
 */
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

EpochSyncContext* epoch_sync_create(
    int num_threads,
    uint64_t samples_per_epoch,
    bool adaptive
) {
    if (num_threads <= 0 || samples_per_epoch == 0) {
        fprintf(stderr, "ERROR: Invalid parameters (threads=%d, samples=%lu)\n",
                num_threads, samples_per_epoch);
        return NULL;
    }
    
    EpochSyncContext* ctx = (EpochSyncContext*)calloc(1, sizeof(EpochSyncContext));
    if (!ctx) {
        fprintf(stderr, "ERROR: Failed to allocate epoch sync context\n");
        return NULL;
    }
    
    // Create Schumann resonance barrier (7.83 Hz)
    ctx->epoch_barrier = cymatic_barrier_create(FREQ_SCHUMANN, num_threads, adaptive);
    if (!ctx->epoch_barrier) {
        fprintf(stderr, "ERROR: Failed to create Schumann resonance barrier\n");
        free(ctx);
        return NULL;
    }
    
    // Initialize configuration
    ctx->samples_per_epoch = samples_per_epoch;
    ctx->sync_enabled = true;
    ctx->adaptive_sync = adaptive;
    ctx->num_threads = num_threads;
    
    // Initialize counters
    ctx->current_epoch = 0;
    ctx->total_epochs = 0;
    ctx->samples_processed = 0;
    ctx->gradients_accumulated = 0;
    ctx->total_gradients = 0;
    
    // Initialize timing
    ctx->epoch_start_time_ns = 0;
    ctx->total_epoch_time_ns = 0;
    ctx->min_epoch_time_ns = UINT64_MAX;
    ctx->max_epoch_time_ns = 0;
    ctx->total_sync_time_ns = 0;
    
    // Initialize statistics
    ctx->sync_count = 0;
    ctx->skipped_syncs = 0;
    
    return ctx;
}

void epoch_sync_destroy(EpochSyncContext* ctx) {
    if (!ctx) return;
    
    if (ctx->epoch_barrier) {
        cymatic_barrier_destroy(ctx->epoch_barrier);
    }
    
    free(ctx);
}

// ============================================================================
// EPOCH MANAGEMENT
// ============================================================================

bool epoch_sync_start_epoch(EpochSyncContext* ctx) {
    if (!ctx) {
        return false;
    }
    
    // Record start time
    ctx->epoch_start_time_ns = get_time_ns();
    
    // Reset epoch counters
    ctx->samples_processed = 0;
    ctx->gradients_accumulated = 0;
    
    // Increment epoch number
    ctx->current_epoch++;
    
    return true;
}

bool epoch_sync_end_epoch(
    EpochSyncContext* ctx,
    EpochTimingResult* result
) {
    if (!ctx || !result) {
        return false;
    }
    
    // Initialize result
    memset(result, 0, sizeof(EpochTimingResult));
    result->epoch_number = ctx->current_epoch;
    result->samples_processed = ctx->samples_processed;
    result->gradients_accumulated = ctx->gradients_accumulated;
    
    uint64_t sync_start = get_time_ns();
    
    // Synchronize at Schumann resonance if enabled
    if (ctx->sync_enabled) {
        int sync_result = cymatic_barrier_wait(ctx->epoch_barrier);
        if (sync_result < 0) {
            fprintf(stderr, "ERROR: Epoch barrier wait failed\n");
            return false;
        }
        result->synced = true;
        ctx->sync_count++;
        
        if (sync_result == 1) {
            // Non-blocking returned immediately (adaptive mode)
            ctx->skipped_syncs++;
        }
    } else {
        result->synced = false;
    }
    
    uint64_t sync_end = get_time_ns();
    uint64_t epoch_end = sync_end;
    
    // Calculate timing
    result->sync_time_ns = sync_end - sync_start;
    result->epoch_time_ns = epoch_end - ctx->epoch_start_time_ns;
    result->compute_time_ns = result->epoch_time_ns - result->sync_time_ns;
    
    if (result->epoch_time_ns > 0) {
        result->sync_overhead_percent = 
            (double)result->sync_time_ns / result->epoch_time_ns * 100.0;
    }
    
    // Update statistics
    ctx->total_epochs++;
    ctx->total_gradients += ctx->gradients_accumulated;
    ctx->total_epoch_time_ns += result->epoch_time_ns;
    ctx->total_sync_time_ns += result->sync_time_ns;
    
    // Update min/max
    if (result->epoch_time_ns < ctx->min_epoch_time_ns) {
        ctx->min_epoch_time_ns = result->epoch_time_ns;
    }
    if (result->epoch_time_ns > ctx->max_epoch_time_ns) {
        ctx->max_epoch_time_ns = result->epoch_time_ns;
    }
    
    return true;
}

void epoch_sync_record_samples(
    EpochSyncContext* ctx,
    uint64_t num_samples
) {
    if (ctx) {
        ctx->samples_processed += num_samples;
    }
}

void epoch_sync_record_gradients(
    EpochSyncContext* ctx,
    uint64_t num_gradients
) {
    if (ctx) {
        ctx->gradients_accumulated += num_gradients;
    }
}

bool epoch_sync_is_epoch_complete(const EpochSyncContext* ctx) {
    if (!ctx) {
        return false;
    }
    
    return ctx->samples_processed >= ctx->samples_per_epoch;
}

// ============================================================================
// SYNCHRONIZATION CONTROL
// ============================================================================

void epoch_sync_set_enabled(EpochSyncContext* ctx, bool enabled) {
    if (ctx) {
        ctx->sync_enabled = enabled;
    }
}

void epoch_sync_set_adaptive(EpochSyncContext* ctx, bool adaptive) {
    if (ctx) {
        ctx->adaptive_sync = adaptive;
        // Note: Adaptive mode is set at barrier creation time
        // To change it, we would need to recreate the barrier
    }
}

bool epoch_sync_set_frequency(
    EpochSyncContext* ctx,
    CymaticFrequencyType frequency_type
) {
    if (!ctx || !ctx->epoch_barrier) {
        return false;
    }
    
    return cymatic_barrier_set_frequency(ctx->epoch_barrier, frequency_type) == 0;
}

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

void epoch_sync_get_statistics(
    const EpochSyncContext* ctx,
    EpochStatistics* stats
) {
    if (!ctx || !stats) {
        return;
    }
    
    memset(stats, 0, sizeof(EpochStatistics));
    
    stats->total_epochs = ctx->total_epochs;
    stats->total_samples = ctx->total_epochs * ctx->samples_per_epoch;
    stats->total_gradients = ctx->total_gradients;
    stats->total_time_ns = ctx->total_epoch_time_ns;
    stats->total_sync_time_ns = ctx->total_sync_time_ns;
    stats->min_epoch_time_ns = ctx->min_epoch_time_ns;
    stats->max_epoch_time_ns = ctx->max_epoch_time_ns;
    
    if (ctx->total_epochs > 0) {
        stats->avg_epoch_time_ns = ctx->total_epoch_time_ns / ctx->total_epochs;
        stats->avg_sync_time_ns = ctx->total_sync_time_ns / ctx->total_epochs;
        
        if (ctx->total_epoch_time_ns > 0) {
            stats->avg_sync_overhead_percent = 
                (double)ctx->total_sync_time_ns / ctx->total_epoch_time_ns * 100.0;
        }
        
        // Calculate throughput
        double total_time_sec = ctx->total_epoch_time_ns / 1e9;
        if (total_time_sec > 0) {
            stats->samples_per_second = stats->total_samples / total_time_sec;
            stats->epochs_per_second = ctx->total_epochs / total_time_sec;
        }
    }
}

void epoch_sync_print_statistics(const EpochSyncContext* ctx) {
    if (!ctx) {
        return;
    }
    
    EpochStatistics stats;
    epoch_sync_get_statistics(ctx, &stats);
    
    printf("\n=== Epoch Synchronization Statistics ===\n");
    printf("Total epochs: %lu\n", stats.total_epochs);
    printf("Total samples: %lu\n", stats.total_samples);
    printf("Total gradients: %lu\n", stats.total_gradients);
    
    printf("\nEpoch Timing:\n");
    printf("  Total time:   %lu ns (%.3f sec)\n", 
           stats.total_time_ns, stats.total_time_ns / 1e9);
    printf("  Average:      %lu ns (%.3f ms)\n",
           stats.avg_epoch_time_ns, stats.avg_epoch_time_ns / 1e6);
    printf("  Min:          %lu ns (%.3f ms)\n",
           stats.min_epoch_time_ns, stats.min_epoch_time_ns / 1e6);
    printf("  Max:          %lu ns (%.3f ms)\n",
           stats.max_epoch_time_ns, stats.max_epoch_time_ns / 1e6);
    
    printf("\nSynchronization (Schumann 7.83 Hz):\n");
    printf("  Total sync time: %lu ns (%.3f sec)\n",
           stats.total_sync_time_ns, stats.total_sync_time_ns / 1e9);
    printf("  Average:         %lu ns (%.3f ms)\n",
           stats.avg_sync_time_ns, stats.avg_sync_time_ns / 1e6);
    printf("  Overhead:        %.2f%%\n", stats.avg_sync_overhead_percent);
    printf("  Sync count:      %lu\n", ctx->sync_count);
    printf("  Skipped syncs:   %lu\n", ctx->skipped_syncs);
    
    printf("\nThroughput:\n");
    printf("  Samples/sec: %.2f\n", stats.samples_per_second);
    printf("  Epochs/sec:  %.2f\n", stats.epochs_per_second);
    
    // Print barrier statistics
    if (ctx->epoch_barrier) {
        printf("\n");
        cymatic_barrier_print_stats(ctx->epoch_barrier);
    }
    
    printf("==========================================\n\n");
}

void epoch_sync_reset_statistics(EpochSyncContext* ctx) {
    if (!ctx) {
        return;
    }
    
    ctx->current_epoch = 0;
    ctx->total_epochs = 0;
    ctx->samples_processed = 0;
    ctx->gradients_accumulated = 0;
    ctx->total_gradients = 0;
    ctx->total_epoch_time_ns = 0;
    ctx->min_epoch_time_ns = UINT64_MAX;
    ctx->max_epoch_time_ns = 0;
    ctx->total_sync_time_ns = 0;
    ctx->sync_count = 0;
    ctx->skipped_syncs = 0;
    
    if (ctx->epoch_barrier) {
        cymatic_barrier_reset_stats(ctx->epoch_barrier);
    }
}

uint64_t epoch_sync_get_current_epoch(const EpochSyncContext* ctx) {
    return ctx ? ctx->current_epoch : 0;
}

uint64_t epoch_sync_get_total_epochs(const EpochSyncContext* ctx) {
    return ctx ? ctx->total_epochs : 0;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

double epoch_sync_calculate_overhead(const EpochSyncContext* ctx) {
    if (!ctx || ctx->total_epoch_time_ns == 0) {
        return 0.0;
    }
    
    return (double)ctx->total_sync_time_ns / ctx->total_epoch_time_ns * 100.0;
}

bool epoch_sync_is_overhead_excessive(const EpochSyncContext* ctx) {
    if (!ctx) {
        return false;
    }
    
    double overhead = epoch_sync_calculate_overhead(ctx);
    
    // Consider overhead excessive if sync time exceeds 10% of total time
    return overhead > 10.0;
}

double epoch_sync_get_throughput(const EpochSyncContext* ctx) {
    if (!ctx || ctx->total_epoch_time_ns == 0) {
        return 0.0;
    }
    
    double total_time_sec = ctx->total_epoch_time_ns / 1e9;
    uint64_t total_samples = ctx->total_epochs * ctx->samples_per_epoch;
    
    return total_samples / total_time_sec;
}

double epoch_sync_get_epoch_rate(const EpochSyncContext* ctx) {
    if (!ctx || ctx->total_epoch_time_ns == 0) {
        return 0.0;
    }
    
    double total_time_sec = ctx->total_epoch_time_ns / 1e9;
    
    return ctx->total_epochs / total_time_sec;
}