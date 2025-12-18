/**
 * Work Distribution Timing Integration
 * 
 * Integrates cymatic frequency barriers (432 Hz) with work distribution
 * to provide natural timing synchronization during batch distribution.
 * 
 * Key Features:
 * - 432 Hz timing for batch distribution operations
 * - Timing measurements for distribution overhead
 * - Statistics tracking for performance analysis
 * - Integration with existing work distribution system
 * 
 * Phase 5, Day 17 Implementation
 */

#ifndef CLLM_WORK_DISTRIBUTION_TIMING_H
#define CLLM_WORK_DISTRIBUTION_TIMING_H

#include "ai/cllm_work_distribution.h"
#include "ai/cllm_cymatic_sync.h"
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Timed Work Distribution Context
 * 
 * Wraps work distribution with cymatic timing synchronization.
 */
typedef struct {
    CymaticBarrier* barrier;           // 432 Hz timing barrier
    WorkDistributionStats* stats;      // Work distribution statistics
    
    // Timing measurements
    uint64_t total_distributions;      // Total distributions performed
    uint64_t total_wait_time_ns;       // Total time spent waiting
    uint64_t total_compute_time_ns;    // Total time computing distributions
    uint64_t min_wait_time_ns;         // Minimum wait time
    uint64_t max_wait_time_ns;         // Maximum wait time
    uint64_t min_compute_time_ns;      // Minimum compute time
    uint64_t max_compute_time_ns;      // Maximum compute time
    
    // Configuration
    bool timing_enabled;               // Enable/disable timing
    bool adaptive_timing;              // Use adaptive timing mode
    int num_threads;                   // Number of threads using this context
} TimedWorkDistributionContext;

/**
 * Timed Distribution Result
 * 
 * Contains both distribution results and timing information.
 */
typedef struct {
    WorkDistribution distribution;     // Work distribution ratios
    uint64_t wait_time_ns;            // Time spent waiting at barrier
    uint64_t compute_time_ns;         // Time spent computing distribution
    uint64_t total_time_ns;           // Total time (wait + compute)
    bool timed_out;                   // Whether timing was skipped (adaptive mode)
} TimedDistributionResult;

/**
 * Multi-Child Timed Distribution Result
 */
typedef struct {
    MultiChildDistribution distribution;  // Multi-child distribution
    uint64_t wait_time_ns;               // Time spent waiting at barrier
    uint64_t compute_time_ns;            // Time spent computing distribution
    uint64_t total_time_ns;              // Total time (wait + compute)
    bool timed_out;                      // Whether timing was skipped
} TimedMultiChildResult;

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

/**
 * Create timed work distribution context
 * 
 * @param num_threads Number of threads that will use this context
 * @param adaptive Enable adaptive timing (skip if too fast)
 * @return Context pointer or NULL on failure
 */
TimedWorkDistributionContext* timed_work_distribution_create(
    int num_threads,
    bool adaptive
);

/**
 * Destroy timed work distribution context
 */
void timed_work_distribution_destroy(TimedWorkDistributionContext* ctx);

// ============================================================================
// TIMED DISTRIBUTION FUNCTIONS
// ============================================================================

/**
 * Calculate work distribution with 432 Hz timing
 * 
 * Synchronizes threads at 432 Hz before computing distribution.
 * 
 * @param ctx Timing context
 * @param parent_id Parent thread ID (p)
 * @param child_id Child thread ID (q)
 * @param result Output: timed distribution result
 * @return true on success, false on failure
 */
bool timed_calculate_work_distribution(
    TimedWorkDistributionContext* ctx,
    uint64_t parent_id,
    uint64_t child_id,
    TimedDistributionResult* result
);

/**
 * Calculate multi-child distribution with 432 Hz timing
 * 
 * @param ctx Timing context
 * @param parent_id Parent thread ID
 * @param child_ids Array of child thread IDs
 * @param num_children Number of children
 * @param result Output: timed multi-child result
 * @return true on success, false on failure
 */
bool timed_calculate_multi_child_distribution(
    TimedWorkDistributionContext* ctx,
    uint64_t parent_id,
    const uint64_t* child_ids,
    size_t num_children,
    TimedMultiChildResult* result
);

// ============================================================================
// TIMING CONTROL
// ============================================================================

/**
 * Enable/disable timing synchronization
 * 
 * When disabled, distributions are computed without barrier synchronization.
 */
void timed_work_distribution_set_timing_enabled(
    TimedWorkDistributionContext* ctx,
    bool enabled
);

/**
 * Enable/disable adaptive timing
 * 
 * Adaptive mode skips synchronization if past the timing period.
 */
void timed_work_distribution_set_adaptive(
    TimedWorkDistributionContext* ctx,
    bool adaptive
);

/**
 * Change timing frequency
 * 
 * @param ctx Timing context
 * @param frequency_type New frequency type (e.g., FREQ_432_HZ)
 * @return true on success, false on failure
 */
bool timed_work_distribution_set_frequency(
    TimedWorkDistributionContext* ctx,
    CymaticFrequencyType frequency_type
);

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

/**
 * Get timing statistics
 * 
 * Returns statistics about timing overhead and performance.
 */
typedef struct {
    uint64_t total_distributions;
    uint64_t total_wait_time_ns;
    uint64_t total_compute_time_ns;
    uint64_t avg_wait_time_ns;
    uint64_t avg_compute_time_ns;
    uint64_t min_wait_time_ns;
    uint64_t max_wait_time_ns;
    uint64_t min_compute_time_ns;
    uint64_t max_compute_time_ns;
    double wait_overhead_percent;      // Wait time as % of total
    double compute_overhead_percent;   // Compute time as % of total
} TimingStatistics;

/**
 * Get timing statistics from context
 */
void timed_work_distribution_get_stats(
    const TimedWorkDistributionContext* ctx,
    TimingStatistics* stats
);

/**
 * Print timing statistics
 */
void timed_work_distribution_print_stats(
    const TimedWorkDistributionContext* ctx
);

/**
 * Reset timing statistics
 */
void timed_work_distribution_reset_stats(
    TimedWorkDistributionContext* ctx
);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Calculate timing overhead percentage
 * 
 * Returns the percentage of time spent in timing synchronization
 * vs. actual computation.
 */
double calculate_timing_overhead(const TimedWorkDistributionContext* ctx);

/**
 * Check if timing is causing performance issues
 * 
 * Returns true if wait time exceeds a threshold (e.g., 50% of total time).
 */
bool is_timing_overhead_excessive(const TimedWorkDistributionContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // CLLM_WORK_DISTRIBUTION_TIMING_H