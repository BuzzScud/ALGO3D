/**
 * Work Distribution Timing Integration - Implementation
 * 
 * Integrates 432 Hz cymatic timing with work distribution system.
 */

#include "ai/cllm_work_distribution_timing.h"
#include "ai/cllm_work_distribution.h"
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

/**
 * Update timing statistics
 */
static void update_timing_stats(
    TimedWorkDistributionContext* ctx,
    uint64_t wait_time_ns,
    uint64_t compute_time_ns
) {
    if (!ctx) return;
    
    ctx->total_distributions++;
    ctx->total_wait_time_ns += wait_time_ns;
    ctx->total_compute_time_ns += compute_time_ns;
    
    // Update min/max wait time
    if (ctx->total_distributions == 1 || wait_time_ns < ctx->min_wait_time_ns) {
        ctx->min_wait_time_ns = wait_time_ns;
    }
    if (wait_time_ns > ctx->max_wait_time_ns) {
        ctx->max_wait_time_ns = wait_time_ns;
    }
    
    // Update min/max compute time
    if (ctx->total_distributions == 1 || compute_time_ns < ctx->min_compute_time_ns) {
        ctx->min_compute_time_ns = compute_time_ns;
    }
    if (compute_time_ns > ctx->max_compute_time_ns) {
        ctx->max_compute_time_ns = compute_time_ns;
    }
}

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

TimedWorkDistributionContext* timed_work_distribution_create(
    int num_threads,
    bool adaptive
) {
    if (num_threads <= 0) {
        fprintf(stderr, "ERROR: Invalid number of threads: %d\n", num_threads);
        return NULL;
    }
    
    TimedWorkDistributionContext* ctx = (TimedWorkDistributionContext*)calloc(
        1, sizeof(TimedWorkDistributionContext)
    );
    if (!ctx) {
        fprintf(stderr, "ERROR: Failed to allocate timing context\n");
        return NULL;
    }
    
    // Create 432 Hz barrier for work distribution timing
    ctx->barrier = cymatic_barrier_create(FREQ_432_HZ, num_threads, adaptive);
    if (!ctx->barrier) {
        fprintf(stderr, "ERROR: Failed to create cymatic barrier\n");
        free(ctx);
        return NULL;
    }
    
    // Allocate work distribution statistics
    ctx->stats = (WorkDistributionStats*)calloc(1, sizeof(WorkDistributionStats));
    if (!ctx->stats) {
        fprintf(stderr, "ERROR: Failed to allocate statistics\n");
        cymatic_barrier_destroy(ctx->barrier);
        free(ctx);
        return NULL;
    }
    
    // Initialize configuration
    ctx->timing_enabled = true;
    ctx->adaptive_timing = adaptive;
    ctx->num_threads = num_threads;
    
    // Initialize statistics
    ctx->total_distributions = 0;
    ctx->total_wait_time_ns = 0;
    ctx->total_compute_time_ns = 0;
    ctx->min_wait_time_ns = UINT64_MAX;
    ctx->max_wait_time_ns = 0;
    ctx->min_compute_time_ns = UINT64_MAX;
    ctx->max_compute_time_ns = 0;
    
    return ctx;
}

void timed_work_distribution_destroy(TimedWorkDistributionContext* ctx) {
    if (!ctx) return;
    
    if (ctx->barrier) {
        cymatic_barrier_destroy(ctx->barrier);
    }
    
    if (ctx->stats) {
        free(ctx->stats);
    }
    
    free(ctx);
}

// ============================================================================
// TIMED DISTRIBUTION FUNCTIONS
// ============================================================================

bool timed_calculate_work_distribution(
    TimedWorkDistributionContext* ctx,
    uint64_t parent_id,
    uint64_t child_id,
    TimedDistributionResult* result
) {
    if (!ctx || !result) {
        return false;
    }
    
    // Initialize result
    memset(result, 0, sizeof(TimedDistributionResult));
    
    uint64_t start_time = get_time_ns();
    uint64_t wait_start = start_time;
    
    // Synchronize at 432 Hz barrier if timing enabled
    if (ctx->timing_enabled) {
        int wait_result = cymatic_barrier_wait(ctx->barrier);
        if (wait_result < 0) {
            fprintf(stderr, "ERROR: Barrier wait failed\n");
            return false;
        }
        result->timed_out = (wait_result == 1);  // Non-blocking returned immediately
    }
    
    uint64_t compute_start = get_time_ns();
    result->wait_time_ns = compute_start - wait_start;
    
    // Calculate work distribution
    result->distribution = calculate_work_distribution(parent_id, child_id);
    
    uint64_t end_time = get_time_ns();
    result->compute_time_ns = end_time - compute_start;
    result->total_time_ns = end_time - start_time;
    
    // Update statistics
    update_timing_stats(ctx, result->wait_time_ns, result->compute_time_ns);
    
    // Update work distribution statistics
    if (ctx->stats) {
        ctx->stats->total_calculations++;
        if (result->distribution.is_valid) {
            ctx->stats->valid_distributions++;
        } else {
            ctx->stats->invalid_distributions++;
        }
    }
    
    return result->distribution.is_valid;
}

bool timed_calculate_multi_child_distribution(
    TimedWorkDistributionContext* ctx,
    uint64_t parent_id,
    const uint64_t* child_ids,
    size_t num_children,
    TimedMultiChildResult* result
) {
    if (!ctx || !child_ids || !result || num_children == 0) {
        return false;
    }
    
    // Initialize result
    memset(result, 0, sizeof(TimedMultiChildResult));
    
    uint64_t start_time = get_time_ns();
    uint64_t wait_start = start_time;
    
    // Synchronize at 432 Hz barrier if timing enabled
    if (ctx->timing_enabled) {
        int wait_result = cymatic_barrier_wait(ctx->barrier);
        if (wait_result < 0) {
            fprintf(stderr, "ERROR: Barrier wait failed\n");
            return false;
        }
        result->timed_out = (wait_result == 1);
    }
    
    uint64_t compute_start = get_time_ns();
    result->wait_time_ns = compute_start - wait_start;
    
    // Calculate multi-child distribution
    result->distribution = calculate_multi_child_distribution(
        parent_id, child_ids, num_children
    );
    
    uint64_t end_time = get_time_ns();
    result->compute_time_ns = end_time - compute_start;
    result->total_time_ns = end_time - start_time;
    
    // Update statistics
    update_timing_stats(ctx, result->wait_time_ns, result->compute_time_ns);
    
    // Update work distribution statistics
    if (ctx->stats) {
        ctx->stats->total_calculations++;
        if (result->distribution.is_valid) {
            ctx->stats->valid_distributions++;
        } else {
            ctx->stats->invalid_distributions++;
        }
    }
    
    return result->distribution.is_valid;
}

// ============================================================================
// TIMING CONTROL
// ============================================================================

void timed_work_distribution_set_timing_enabled(
    TimedWorkDistributionContext* ctx,
    bool enabled
) {
    if (ctx) {
        ctx->timing_enabled = enabled;
    }
}

void timed_work_distribution_set_adaptive(
    TimedWorkDistributionContext* ctx,
    bool adaptive
) {
    if (ctx) {
        ctx->adaptive_timing = adaptive;
        // Note: Adaptive mode is set at barrier creation time
        // To change it, we would need to recreate the barrier
    }
}

bool timed_work_distribution_set_frequency(
    TimedWorkDistributionContext* ctx,
    CymaticFrequencyType frequency_type
) {
    if (!ctx || !ctx->barrier) {
        return false;
    }
    
    return cymatic_barrier_set_frequency(ctx->barrier, frequency_type) == 0;
}

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

void timed_work_distribution_get_stats(
    const TimedWorkDistributionContext* ctx,
    TimingStatistics* stats
) {
    if (!ctx || !stats) {
        return;
    }
    
    memset(stats, 0, sizeof(TimingStatistics));
    
    stats->total_distributions = ctx->total_distributions;
    stats->total_wait_time_ns = ctx->total_wait_time_ns;
    stats->total_compute_time_ns = ctx->total_compute_time_ns;
    stats->min_wait_time_ns = ctx->min_wait_time_ns;
    stats->max_wait_time_ns = ctx->max_wait_time_ns;
    stats->min_compute_time_ns = ctx->min_compute_time_ns;
    stats->max_compute_time_ns = ctx->max_compute_time_ns;
    
    if (ctx->total_distributions > 0) {
        stats->avg_wait_time_ns = ctx->total_wait_time_ns / ctx->total_distributions;
        stats->avg_compute_time_ns = ctx->total_compute_time_ns / ctx->total_distributions;
        
        uint64_t total_time = ctx->total_wait_time_ns + ctx->total_compute_time_ns;
        if (total_time > 0) {
            stats->wait_overhead_percent = (double)ctx->total_wait_time_ns / total_time * 100.0;
            stats->compute_overhead_percent = (double)ctx->total_compute_time_ns / total_time * 100.0;
        }
    }
}

void timed_work_distribution_print_stats(
    const TimedWorkDistributionContext* ctx
) {
    if (!ctx) {
        return;
    }
    
    TimingStatistics stats;
    timed_work_distribution_get_stats(ctx, &stats);
    
    printf("\n=== Work Distribution Timing Statistics ===\n");
    printf("Total distributions: %lu\n", stats.total_distributions);
    printf("\nWait Time (432 Hz synchronization):\n");
    printf("  Total:   %lu ns\n", stats.total_wait_time_ns);
    printf("  Average: %lu ns\n", stats.avg_wait_time_ns);
    printf("  Min:     %lu ns\n", stats.min_wait_time_ns);
    printf("  Max:     %lu ns\n", stats.max_wait_time_ns);
    printf("  Overhead: %.2f%%\n", stats.wait_overhead_percent);
    
    printf("\nCompute Time (distribution calculation):\n");
    printf("  Total:   %lu ns\n", stats.total_compute_time_ns);
    printf("  Average: %lu ns\n", stats.avg_compute_time_ns);
    printf("  Min:     %lu ns\n", stats.min_compute_time_ns);
    printf("  Max:     %lu ns\n", stats.max_compute_time_ns);
    printf("  Overhead: %.2f%%\n", stats.compute_overhead_percent);
    
    // Print barrier statistics
    if (ctx->barrier) {
        printf("\n");
        cymatic_barrier_print_stats(ctx->barrier);
    }
    
    // Print work distribution statistics
    if (ctx->stats) {
        printf("\nWork Distribution Statistics:\n");
        printf("  Total calculations: %lu\n", ctx->stats->total_calculations);
        printf("  Valid distributions: %lu\n", ctx->stats->valid_distributions);
        printf("  Invalid distributions: %lu\n", ctx->stats->invalid_distributions);
        if (ctx->stats->total_calculations > 0) {
            double success_rate = (double)ctx->stats->valid_distributions / 
                                 ctx->stats->total_calculations * 100.0;
            printf("  Success rate: %.2f%%\n", success_rate);
        }
    }
    
    printf("==========================================\n\n");
}

void timed_work_distribution_reset_stats(TimedWorkDistributionContext* ctx) {
    if (!ctx) {
        return;
    }
    
    ctx->total_distributions = 0;
    ctx->total_wait_time_ns = 0;
    ctx->total_compute_time_ns = 0;
    ctx->min_wait_time_ns = UINT64_MAX;
    ctx->max_wait_time_ns = 0;
    ctx->min_compute_time_ns = UINT64_MAX;
    ctx->max_compute_time_ns = 0;
    
    if (ctx->stats) {
        memset(ctx->stats, 0, sizeof(WorkDistributionStats));
    }
    
    if (ctx->barrier) {
        cymatic_barrier_reset_stats(ctx->barrier);
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

double calculate_timing_overhead(const TimedWorkDistributionContext* ctx) {
    if (!ctx || ctx->total_distributions == 0) {
        return 0.0;
    }
    
    uint64_t total_time = ctx->total_wait_time_ns + ctx->total_compute_time_ns;
    if (total_time == 0) {
        return 0.0;
    }
    
    return (double)ctx->total_wait_time_ns / total_time * 100.0;
}

bool is_timing_overhead_excessive(const TimedWorkDistributionContext* ctx) {
    if (!ctx) {
        return false;
    }
    
    double overhead = calculate_timing_overhead(ctx);
    
    // Consider overhead excessive if wait time exceeds 50% of total time
    return overhead > 50.0;
}
