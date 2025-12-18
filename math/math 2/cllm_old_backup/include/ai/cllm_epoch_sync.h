/**
 * Epoch Synchronization with Schumann Resonance
 * 
 * Implements epoch-level timing synchronization using Schumann resonance (7.83 Hz)
 * to provide natural timing coordination for gradient accumulation and epoch boundaries.
 * 
 * Key Features:
 * - 7.83 Hz (Schumann resonance) epoch synchronization
 * - Integration with gradient accumulation
 * - Epoch boundary detection and timing
 * - Performance metrics and statistics
 * - Multi-threaded epoch coordination
 * 
 * Phase 5, Day 17 Afternoon Implementation
 */

#ifndef CLLM_EPOCH_SYNC_H
#define CLLM_EPOCH_SYNC_H

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
 * Epoch Synchronization Context
 * 
 * Manages epoch-level timing synchronization using Schumann resonance.
 */
typedef struct {
    CymaticBarrier* epoch_barrier;     // 7.83 Hz Schumann resonance barrier
    
    // Epoch tracking
    uint64_t current_epoch;            // Current epoch number
    uint64_t total_epochs;             // Total epochs completed
    uint64_t samples_per_epoch;        // Samples per epoch
    uint64_t samples_processed;        // Samples processed in current epoch
    
    // Timing measurements
    uint64_t epoch_start_time_ns;     // Epoch start time
    uint64_t total_epoch_time_ns;     // Total time across all epochs
    uint64_t min_epoch_time_ns;       // Fastest epoch
    uint64_t max_epoch_time_ns;       // Slowest epoch
    uint64_t total_sync_time_ns;      // Total synchronization time
    
    // Gradient accumulation tracking
    uint64_t gradients_accumulated;    // Gradients accumulated this epoch
    uint64_t total_gradients;          // Total gradients across all epochs
    
    // Configuration
    bool sync_enabled;                 // Enable/disable synchronization
    bool adaptive_sync;                // Adaptive synchronization mode
    int num_threads;                   // Number of threads
    
    // Statistics
    uint64_t sync_count;               // Number of synchronizations
    uint64_t skipped_syncs;            // Skipped synchronizations (adaptive)
} EpochSyncContext;

/**
 * Epoch Timing Result
 * 
 * Contains timing information for a completed epoch.
 */
typedef struct {
    uint64_t epoch_number;             // Epoch number
    uint64_t samples_processed;        // Samples processed
    uint64_t gradients_accumulated;    // Gradients accumulated
    uint64_t epoch_time_ns;            // Total epoch time
    uint64_t sync_time_ns;             // Synchronization time
    uint64_t compute_time_ns;          // Computation time
    double sync_overhead_percent;      // Sync overhead percentage
    bool synced;                       // Whether epoch was synchronized
} EpochTimingResult;

/**
 * Epoch Statistics
 * 
 * Aggregate statistics across all epochs.
 */
typedef struct {
    uint64_t total_epochs;
    uint64_t total_samples;
    uint64_t total_gradients;
    uint64_t total_time_ns;
    uint64_t total_sync_time_ns;
    uint64_t avg_epoch_time_ns;
    uint64_t avg_sync_time_ns;
    uint64_t min_epoch_time_ns;
    uint64_t max_epoch_time_ns;
    double avg_sync_overhead_percent;
    double samples_per_second;
    double epochs_per_second;
} EpochStatistics;

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

/**
 * Create epoch synchronization context
 * 
 * @param num_threads Number of threads participating in epochs
 * @param samples_per_epoch Number of samples per epoch
 * @param adaptive Enable adaptive synchronization
 * @return Context pointer or NULL on failure
 */
EpochSyncContext* epoch_sync_create(
    int num_threads,
    uint64_t samples_per_epoch,
    bool adaptive
);

/**
 * Destroy epoch synchronization context
 */
void epoch_sync_destroy(EpochSyncContext* ctx);

// ============================================================================
// EPOCH MANAGEMENT
// ============================================================================

/**
 * Start a new epoch
 * 
 * Initializes timing and resets counters for a new epoch.
 * 
 * @param ctx Epoch sync context
 * @return true on success, false on failure
 */
bool epoch_sync_start_epoch(EpochSyncContext* ctx);

/**
 * End current epoch
 * 
 * Synchronizes threads at Schumann resonance, accumulates gradients,
 * and records timing information.
 * 
 * @param ctx Epoch sync context
 * @param result Output: epoch timing result
 * @return true on success, false on failure
 */
bool epoch_sync_end_epoch(
    EpochSyncContext* ctx,
    EpochTimingResult* result
);

/**
 * Record sample processing
 * 
 * Updates sample counter for current epoch.
 * 
 * @param ctx Epoch sync context
 * @param num_samples Number of samples processed
 */
void epoch_sync_record_samples(
    EpochSyncContext* ctx,
    uint64_t num_samples
);

/**
 * Record gradient accumulation
 * 
 * Updates gradient counter for current epoch.
 * 
 * @param ctx Epoch sync context
 * @param num_gradients Number of gradients accumulated
 */
void epoch_sync_record_gradients(
    EpochSyncContext* ctx,
    uint64_t num_gradients
);

/**
 * Check if epoch is complete
 * 
 * Returns true if enough samples have been processed for this epoch.
 */
bool epoch_sync_is_epoch_complete(const EpochSyncContext* ctx);

// ============================================================================
// SYNCHRONIZATION CONTROL
// ============================================================================

/**
 * Enable/disable epoch synchronization
 */
void epoch_sync_set_enabled(EpochSyncContext* ctx, bool enabled);

/**
 * Enable/disable adaptive synchronization
 */
void epoch_sync_set_adaptive(EpochSyncContext* ctx, bool adaptive);

/**
 * Change synchronization frequency
 * 
 * @param ctx Epoch sync context
 * @param frequency_type New frequency type
 * @return true on success, false on failure
 */
bool epoch_sync_set_frequency(
    EpochSyncContext* ctx,
    CymaticFrequencyType frequency_type
);

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

/**
 * Get epoch statistics
 * 
 * @param ctx Epoch sync context
 * @param stats Output: epoch statistics
 */
void epoch_sync_get_statistics(
    const EpochSyncContext* ctx,
    EpochStatistics* stats
);

/**
 * Print epoch statistics
 */
void epoch_sync_print_statistics(const EpochSyncContext* ctx);

/**
 * Reset epoch statistics
 */
void epoch_sync_reset_statistics(EpochSyncContext* ctx);

/**
 * Get current epoch number
 */
uint64_t epoch_sync_get_current_epoch(const EpochSyncContext* ctx);

/**
 * Get total epochs completed
 */
uint64_t epoch_sync_get_total_epochs(const EpochSyncContext* ctx);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Calculate synchronization overhead percentage
 */
double epoch_sync_calculate_overhead(const EpochSyncContext* ctx);

/**
 * Check if synchronization overhead is excessive
 * 
 * Returns true if sync time exceeds threshold (e.g., 10% of total time).
 */
bool epoch_sync_is_overhead_excessive(const EpochSyncContext* ctx);

/**
 * Get samples per second throughput
 */
double epoch_sync_get_throughput(const EpochSyncContext* ctx);

/**
 * Get epochs per second rate
 */
double epoch_sync_get_epoch_rate(const EpochSyncContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // CLLM_EPOCH_SYNC_H
