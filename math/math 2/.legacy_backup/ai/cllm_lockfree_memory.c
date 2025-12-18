/**
 * Lock-Free Memory Access Patterns Implementation
 * 
 * Phase 3, Day 12
 */

#include "ai/cllm_lockfree_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// ATOMIC DOUBLE OPERATIONS
// ============================================================================

/**
 * Atomic compare-and-swap for double
 * 
 * Uses type punning to perform CAS on double values.
 */
static inline bool atomic_cas_double(_Atomic double* ptr, double* expected, double desired) {
    // Type pun double to uint64_t for atomic operations
    union {
        double d;
        uint64_t u;
    } exp_union, des_union;
    
    exp_union.d = *expected;
    des_union.d = desired;
    
    _Atomic uint64_t* atomic_ptr = (_Atomic uint64_t*)ptr;
    bool success = atomic_compare_exchange_weak(atomic_ptr, &exp_union.u, des_union.u);
    
    if (!success) {
        *expected = exp_union.d;
    }
    
    return success;
}

/**
 * Atomic load for double
 */
static inline double atomic_load_double(const _Atomic double* ptr) {
    union {
        double d;
        uint64_t u;
    } result;
    
    const _Atomic uint64_t* atomic_ptr = (const _Atomic uint64_t*)ptr;
    result.u = atomic_load(atomic_ptr);
    
    return result.d;
}

/**
 * Atomic store for double
 */
static inline void atomic_store_double(_Atomic double* ptr, double value) {
    union {
        double d;
        uint64_t u;
    } val_union;
    
    val_union.d = value;
    
    _Atomic uint64_t* atomic_ptr = (_Atomic uint64_t*)ptr;
    atomic_store(atomic_ptr, val_union.u);
}

// ============================================================================
// LIFECYCLE
// ============================================================================

LockFreeGradientAccumulator* lockfree_gradient_accumulator_create(size_t size) {
    if (size == 0) {
        fprintf(stderr, "ERROR: Invalid size 0\n");
        return NULL;
    }
    
    LockFreeGradientAccumulator* accumulator = calloc(1, sizeof(LockFreeGradientAccumulator));
    if (!accumulator) {
        fprintf(stderr, "ERROR: Failed to allocate gradient accumulator\n");
        return NULL;
    }
    
    // Allocate gradient array
    accumulator->gradients = calloc(size, sizeof(_Atomic double));
    if (!accumulator->gradients) {
        fprintf(stderr, "ERROR: Failed to allocate gradients\n");
        free(accumulator);
        return NULL;
    }
    
    accumulator->size = size;
    
    // Initialize statistics
    atomic_init(&accumulator->stats.successful_cas, 0);
    atomic_init(&accumulator->stats.failed_cas, 0);
    atomic_init(&accumulator->stats.total_operations, 0);
    atomic_init(&accumulator->stats.fallback_locks, 0);
    
    // Initialize fallback lock
    pthread_mutex_init(&accumulator->fallback_lock, NULL);
    
    return accumulator;
}

void lockfree_gradient_accumulator_destroy(LockFreeGradientAccumulator* accumulator) {
    if (!accumulator) return;
    
    if (accumulator->gradients) {
        free(accumulator->gradients);
    }
    
    pthread_mutex_destroy(&accumulator->fallback_lock);
    free(accumulator);
}

LockFreeWeightUpdater* lockfree_weight_updater_create(size_t size) {
    if (size == 0) {
        fprintf(stderr, "ERROR: Invalid size 0\n");
        return NULL;
    }
    
    LockFreeWeightUpdater* updater = calloc(1, sizeof(LockFreeWeightUpdater));
    if (!updater) {
        fprintf(stderr, "ERROR: Failed to allocate weight updater\n");
        return NULL;
    }
    
    // Allocate weight array
    updater->weights = calloc(size, sizeof(_Atomic double));
    if (!updater->weights) {
        fprintf(stderr, "ERROR: Failed to allocate weights\n");
        free(updater);
        return NULL;
    }
    
    updater->size = size;
    
    // Initialize statistics
    atomic_init(&updater->stats.successful_cas, 0);
    atomic_init(&updater->stats.failed_cas, 0);
    atomic_init(&updater->stats.total_operations, 0);
    atomic_init(&updater->stats.fallback_locks, 0);
    
    // Initialize fallback lock
    pthread_mutex_init(&updater->fallback_lock, NULL);
    
    return updater;
}

void lockfree_weight_updater_destroy(LockFreeWeightUpdater* updater) {
    if (!updater) return;
    
    if (updater->weights) {
        free(updater->weights);
    }
    
    pthread_mutex_destroy(&updater->fallback_lock);
    free(updater);
}

// ============================================================================
// GRADIENT ACCUMULATION (LOCK-FREE)
// ============================================================================

int lockfree_accumulate_gradient(
    LockFreeGradientAccumulator* accumulator,
    size_t index,
    double local_gradient
) {
    if (!accumulator || index >= accumulator->size) {
        return -1;
    }
    
    atomic_fetch_add(&accumulator->stats.total_operations, 1);
    
    _Atomic double* target = &accumulator->gradients[index];
    
    // Try lock-free CAS
    int retries = 0;
    while (retries < LOCKFREE_MAX_RETRIES) {
        double old_value = atomic_load_double(target);
        double new_value = old_value + local_gradient;
        
        if (atomic_cas_double(target, &old_value, new_value)) {
            atomic_fetch_add(&accumulator->stats.successful_cas, 1);
            return 0;
        }
        
        atomic_fetch_add(&accumulator->stats.failed_cas, 1);
        retries++;
    }
    
    // Fallback to lock if CAS fails too many times
    atomic_fetch_add(&accumulator->stats.fallback_locks, 1);
    pthread_mutex_lock(&accumulator->fallback_lock);
    
    double old_value = atomic_load_double(target);
    double new_value = old_value + local_gradient;
    atomic_store_double(target, new_value);
    
    pthread_mutex_unlock(&accumulator->fallback_lock);
    
    return 0;
}

size_t lockfree_accumulate_gradients(
    LockFreeGradientAccumulator* accumulator,
    const double* local_gradients,
    size_t size
) {
    if (!accumulator || !local_gradients) {
        return 0;
    }
    
    size_t count = (size < accumulator->size) ? size : accumulator->size;
    
    for (size_t i = 0; i < count; i++) {
        lockfree_accumulate_gradient(accumulator, i, local_gradients[i]);
    }
    
    return count;
}

double lockfree_get_gradient(
    const LockFreeGradientAccumulator* accumulator,
    size_t index
) {
    if (!accumulator || index >= accumulator->size) {
        return 0.0;
    }
    
    return atomic_load_double(&accumulator->gradients[index]);
}

void lockfree_reset_gradients(LockFreeGradientAccumulator* accumulator) {
    if (!accumulator) return;
    
    for (size_t i = 0; i < accumulator->size; i++) {
        atomic_store_double(&accumulator->gradients[i], 0.0);
    }
}

// ============================================================================
// WEIGHT UPDATES (LOCK-FREE)
// ============================================================================

int lockfree_update_weight(
    LockFreeWeightUpdater* updater,
    size_t index,
    double delta
) {
    if (!updater || index >= updater->size) {
        return -1;
    }
    
    atomic_fetch_add(&updater->stats.total_operations, 1);
    
    _Atomic double* target = &updater->weights[index];
    
    // Try lock-free CAS
    int retries = 0;
    while (retries < LOCKFREE_MAX_RETRIES) {
        double old_value = atomic_load_double(target);
        double new_value = old_value + delta;
        
        if (atomic_cas_double(target, &old_value, new_value)) {
            atomic_fetch_add(&updater->stats.successful_cas, 1);
            return 0;
        }
        
        atomic_fetch_add(&updater->stats.failed_cas, 1);
        retries++;
    }
    
    // Fallback to lock
    atomic_fetch_add(&updater->stats.fallback_locks, 1);
    pthread_mutex_lock(&updater->fallback_lock);
    
    double old_value = atomic_load_double(target);
    double new_value = old_value + delta;
    atomic_store_double(target, new_value);
    
    pthread_mutex_unlock(&updater->fallback_lock);
    
    return 0;
}

size_t lockfree_update_weights(
    LockFreeWeightUpdater* updater,
    const double* deltas,
    size_t size
) {
    if (!updater || !deltas) {
        return 0;
    }
    
    size_t count = (size < updater->size) ? size : updater->size;
    
    for (size_t i = 0; i < count; i++) {
        lockfree_update_weight(updater, i, deltas[i]);
    }
    
    return count;
}

double lockfree_get_weight(
    const LockFreeWeightUpdater* updater,
    size_t index
) {
    if (!updater || index >= updater->size) {
        return 0.0;
    }
    
    return atomic_load_double(&updater->weights[index]);
}

int lockfree_set_weight(
    LockFreeWeightUpdater* updater,
    size_t index,
    double value
) {
    if (!updater || index >= updater->size) {
        return -1;
    }
    
    atomic_store_double(&updater->weights[index], value);
    return 0;
}

// ============================================================================
// STATISTICS
// ============================================================================

void lockfree_get_stats(
    const LockFreeStats* stats,
    uint64_t* successful_cas,
    uint64_t* failed_cas,
    uint64_t* total_ops,
    uint64_t* fallback_locks
) {
    if (!stats) return;
    
    if (successful_cas) *successful_cas = atomic_load(&stats->successful_cas);
    if (failed_cas) *failed_cas = atomic_load(&stats->failed_cas);
    if (total_ops) *total_ops = atomic_load(&stats->total_operations);
    if (fallback_locks) *fallback_locks = atomic_load(&stats->fallback_locks);
}

void lockfree_print_stats(const LockFreeStats* stats, const char* name) {
    if (!stats) return;
    
    uint64_t successful = atomic_load(&stats->successful_cas);
    uint64_t failed = atomic_load(&stats->failed_cas);
    uint64_t total = atomic_load(&stats->total_operations);
    uint64_t fallbacks = atomic_load(&stats->fallback_locks);
    
    printf("%s Statistics:\n", name ? name : "Lock-Free");
    printf("  Total Operations:   %llu\n", (unsigned long long)total);
    printf("  Successful CAS:     %llu\n", (unsigned long long)successful);
    printf("  Failed CAS:         %llu\n", (unsigned long long)failed);
    printf("  Fallback Locks:     %llu\n", (unsigned long long)fallbacks);
    
    if (total > 0) {
        double success_rate = (double)successful / (double)total * 100.0;
        printf("  CAS Success Rate:   %.2f%%\n", success_rate);
    }
}

void lockfree_reset_stats(LockFreeStats* stats) {
    if (!stats) return;
    
    atomic_store(&stats->successful_cas, 0);
    atomic_store(&stats->failed_cas, 0);
    atomic_store(&stats->total_operations, 0);
    atomic_store(&stats->fallback_locks, 0);
}

void lockfree_gradient_accumulator_get_stats(
    const LockFreeGradientAccumulator* accumulator,
    uint64_t* successful_cas,
    uint64_t* failed_cas,
    uint64_t* total_ops,
    uint64_t* fallback_locks
) {
    if (!accumulator) return;
    lockfree_get_stats(&accumulator->stats, successful_cas, failed_cas, 
                      total_ops, fallback_locks);
}

void lockfree_weight_updater_get_stats(
    const LockFreeWeightUpdater* updater,
    uint64_t* successful_cas,
    uint64_t* failed_cas,
    uint64_t* total_ops,
    uint64_t* fallback_locks
) {
    if (!updater) return;
    lockfree_get_stats(&updater->stats, successful_cas, failed_cas,
                      total_ops, fallback_locks);
}

// ============================================================================
// UTILITIES
// ============================================================================

double lockfree_calculate_success_rate(const LockFreeStats* stats) {
    if (!stats) return 0.0;
    
    uint64_t total = atomic_load(&stats->total_operations);
    if (total == 0) return 0.0;
    
    uint64_t successful = atomic_load(&stats->successful_cas);
    return (double)successful / (double)total;
}

int lockfree_gradient_accumulator_validate(
    const LockFreeGradientAccumulator* accumulator
) {
    if (!accumulator) {
        fprintf(stderr, "ERROR: NULL accumulator\n");
        return 0;
    }
    
    if (!accumulator->gradients) {
        fprintf(stderr, "ERROR: NULL gradients array\n");
        return 0;
    }
    
    if (accumulator->size == 0) {
        fprintf(stderr, "ERROR: Zero size\n");
        return 0;
    }
    
    return 1;
}

int lockfree_weight_updater_validate(
    const LockFreeWeightUpdater* updater
) {
    if (!updater) {
        fprintf(stderr, "ERROR: NULL updater\n");
        return 0;
    }
    
    if (!updater->weights) {
        fprintf(stderr, "ERROR: NULL weights array\n");
        return 0;
    }
    
    if (updater->size == 0) {
        fprintf(stderr, "ERROR: Zero size\n");
        return 0;
    }
    
    return 1;
}