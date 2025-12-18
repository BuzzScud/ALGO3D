/**
 * Lock-Free Memory Access Patterns
 * 
 * Provides lock-free operations for gradient accumulation and weight updates
 * using atomic compare-and-swap operations.
 * 
 * Phase 3, Day 12 Implementation
 */

#ifndef CLLM_LOCKFREE_MEMORY_H
#define CLLM_LOCKFREE_MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define LOCKFREE_MAX_RETRIES 1000  // Maximum CAS retries before fallback

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Lock-Free Memory Statistics
 */
typedef struct {
    atomic_uint_fast64_t successful_cas;    // Successful compare-and-swap operations
    atomic_uint_fast64_t failed_cas;        // Failed CAS operations (retries)
    atomic_uint_fast64_t total_operations;  // Total operations attempted
    atomic_uint_fast64_t fallback_locks;    // Times fallback to lock was used
} LockFreeStats;

/**
 * Lock-Free Gradient Accumulator
 * 
 * Accumulates gradients from multiple threads without locks.
 */
typedef struct {
    _Atomic double* gradients;              // Shared gradient array
    size_t size;                            // Number of gradients
    LockFreeStats stats;                    // Statistics
    
    // Fallback lock (used only when CAS fails too many times)
    pthread_mutex_t fallback_lock;
    
} LockFreeGradientAccumulator;

/**
 * Lock-Free Weight Updater
 * 
 * Updates weights from multiple threads without locks.
 */
typedef struct {
    _Atomic double* weights;                // Shared weight array
    size_t size;                            // Number of weights
    LockFreeStats stats;                    // Statistics
    
    // Fallback lock
    pthread_mutex_t fallback_lock;
    
} LockFreeWeightUpdater;

// ============================================================================
// LIFECYCLE
// ============================================================================

/**
 * Create lock-free gradient accumulator
 * 
 * @param size Number of gradients
 * @return Allocated accumulator, or NULL on error
 */
LockFreeGradientAccumulator* lockfree_gradient_accumulator_create(size_t size);

/**
 * Destroy lock-free gradient accumulator
 * 
 * @param accumulator Accumulator to destroy
 */
void lockfree_gradient_accumulator_destroy(LockFreeGradientAccumulator* accumulator);

/**
 * Create lock-free weight updater
 * 
 * @param size Number of weights
 * @return Allocated updater, or NULL on error
 */
LockFreeWeightUpdater* lockfree_weight_updater_create(size_t size);

/**
 * Destroy lock-free weight updater
 * 
 * @param updater Updater to destroy
 */
void lockfree_weight_updater_destroy(LockFreeWeightUpdater* updater);

// ============================================================================
// GRADIENT ACCUMULATION (LOCK-FREE)
// ============================================================================

/**
 * Accumulate gradient (lock-free)
 * 
 * Adds local_gradient to shared gradient using atomic compare-and-swap.
 * Falls back to lock if CAS fails too many times.
 * 
 * @param accumulator Gradient accumulator
 * @param index Gradient index
 * @param local_gradient Local gradient value to add
 * @return 0 on success, -1 on error
 */
int lockfree_accumulate_gradient(
    LockFreeGradientAccumulator* accumulator,
    size_t index,
    double local_gradient
);

/**
 * Accumulate gradient array (lock-free)
 * 
 * Adds entire local gradient array to shared gradients.
 * 
 * @param accumulator Gradient accumulator
 * @param local_gradients Local gradient array
 * @param size Number of gradients to accumulate
 * @return Number of gradients accumulated
 */
size_t lockfree_accumulate_gradients(
    LockFreeGradientAccumulator* accumulator,
    const double* local_gradients,
    size_t size
);

/**
 * Get gradient value (lock-free read)
 * 
 * @param accumulator Gradient accumulator
 * @param index Gradient index
 * @return Gradient value
 */
double lockfree_get_gradient(
    const LockFreeGradientAccumulator* accumulator,
    size_t index
);

/**
 * Reset gradients to zero (lock-free)
 * 
 * @param accumulator Gradient accumulator
 */
void lockfree_reset_gradients(LockFreeGradientAccumulator* accumulator);

// ============================================================================
// WEIGHT UPDATES (LOCK-FREE)
// ============================================================================

/**
 * Update weight (lock-free)
 * 
 * Updates weight using atomic compare-and-swap.
 * 
 * @param updater Weight updater
 * @param index Weight index
 * @param delta Change to apply
 * @return 0 on success, -1 on error
 */
int lockfree_update_weight(
    LockFreeWeightUpdater* updater,
    size_t index,
    double delta
);

/**
 * Update weight array (lock-free)
 * 
 * Updates entire weight array.
 * 
 * @param updater Weight updater
 * @param deltas Array of changes to apply
 * @param size Number of weights to update
 * @return Number of weights updated
 */
size_t lockfree_update_weights(
    LockFreeWeightUpdater* updater,
    const double* deltas,
    size_t size
);

/**
 * Get weight value (lock-free read)
 * 
 * @param updater Weight updater
 * @param index Weight index
 * @return Weight value
 */
double lockfree_get_weight(
    const LockFreeWeightUpdater* updater,
    size_t index
);

/**
 * Set weight value (lock-free)
 * 
 * @param updater Weight updater
 * @param index Weight index
 * @param value New weight value
 * @return 0 on success, -1 on error
 */
int lockfree_set_weight(
    LockFreeWeightUpdater* updater,
    size_t index,
    double value
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get lock-free statistics
 * 
 * @param stats Statistics structure
 * @param successful_cas Output: successful CAS operations
 * @param failed_cas Output: failed CAS operations
 * @param total_ops Output: total operations
 * @param fallback_locks Output: fallback lock uses
 */
void lockfree_get_stats(
    const LockFreeStats* stats,
    uint64_t* successful_cas,
    uint64_t* failed_cas,
    uint64_t* total_ops,
    uint64_t* fallback_locks
);

/**
 * Print lock-free statistics
 * 
 * @param stats Statistics structure
 * @param name Name for the statistics (e.g., "Gradient Accumulator")
 */
void lockfree_print_stats(const LockFreeStats* stats, const char* name);

/**
 * Reset lock-free statistics
 * 
 * @param stats Statistics structure
 */
void lockfree_reset_stats(LockFreeStats* stats);

/**
 * Get gradient accumulator statistics
 * 
 * @param accumulator Gradient accumulator
 * @param successful_cas Output: successful CAS operations
 * @param failed_cas Output: failed CAS operations
 * @param total_ops Output: total operations
 * @param fallback_locks Output: fallback lock uses
 */
void lockfree_gradient_accumulator_get_stats(
    const LockFreeGradientAccumulator* accumulator,
    uint64_t* successful_cas,
    uint64_t* failed_cas,
    uint64_t* total_ops,
    uint64_t* fallback_locks
);

/**
 * Get weight updater statistics
 * 
 * @param updater Weight updater
 * @param successful_cas Output: successful CAS operations
 * @param failed_cas Output: failed CAS operations
 * @param total_ops Output: total operations
 * @param fallback_locks Output: fallback lock uses
 */
void lockfree_weight_updater_get_stats(
    const LockFreeWeightUpdater* updater,
    uint64_t* successful_cas,
    uint64_t* failed_cas,
    uint64_t* total_ops,
    uint64_t* fallback_locks
);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Calculate CAS success rate
 * 
 * @param stats Statistics structure
 * @return Success rate (0.0 to 1.0)
 */
double lockfree_calculate_success_rate(const LockFreeStats* stats);

/**
 * Validate lock-free accumulator
 * 
 * @param accumulator Gradient accumulator
 * @return 1 if valid, 0 if invalid
 */
int lockfree_gradient_accumulator_validate(
    const LockFreeGradientAccumulator* accumulator
);

/**
 * Validate lock-free updater
 * 
 * @param updater Weight updater
 * @return 1 if valid, 0 if invalid
 */
int lockfree_weight_updater_validate(
    const LockFreeWeightUpdater* updater
);

#ifdef __cplusplus
}
#endif

#endif // CLLM_LOCKFREE_MEMORY_H
