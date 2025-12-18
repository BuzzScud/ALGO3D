/**
 * @file cllm_workload_detector.c
 * @brief Implementation of workload detection system
 */

#include "ai/cllm_workload_detector.h"
// Migrated to NEW math library - no prime_float_math needed
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Small epsilon for floating point comparisons
 */
#define DETECTOR_EPSILON 1e-6

/**
 * @brief Initialize workload detector context
 */
bool workload_detector_init(WorkloadDetectorContext *ctx, bool verbose) {
    if (!ctx) {
        return false;
    }
    
    memset(ctx, 0, sizeof(WorkloadDetectorContext));
    ctx->verbose = verbose;
    ctx->last_spawn_time = 0.0;
    ctx->last_despawn_time = 0.0;
    
    return true;
}

/**
 * @brief Destroy workload detector context
 */
void workload_detector_destroy(WorkloadDetectorContext *ctx) {
    if (!ctx) {
        return;
    }
    
    // Nothing to free currently, but keep for future extensions
    memset(ctx, 0, sizeof(WorkloadDetectorContext));
}

/**
 * @brief Get current timestamp in seconds
 */
double get_current_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/**
 * @brief Get number of available CPU cores
 */
int get_available_cores(void) {
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (cores <= 0) {
        return 1; // Fallback to 1 core
    }
    return (int)cores;
}

/**
 * @brief Get current CPU utilization
 */
double get_cpu_utilization(void) {
    // Simplified implementation - in production, would read /proc/stat
    // For now, return a conservative estimate
    return 0.5; // Assume 50% utilization
}

/**
 * @brief Collect current workload metrics
 */
void collect_workload_metrics(size_t pending_batches,
                              size_t active_workers,
                              size_t completed_batches,
                              double total_time,
                              uint32_t current_depth,
                              WorkloadMetrics *metrics) {
    if (!metrics) {
        return;
    }
    
    metrics->pending_batches = pending_batches;
    metrics->active_workers = active_workers;
    metrics->completed_batches = completed_batches;
    metrics->total_processing_time = total_time;
    metrics->current_depth = current_depth;
    
    // Calculate average batch time
    if (completed_batches > 0 && total_time > DETECTOR_EPSILON) {
        metrics->avg_batch_time = total_time / (double)completed_batches;
    } else {
        metrics->avg_batch_time = 0.0;
    }
    
    // Get CPU utilization
    metrics->cpu_utilization = get_cpu_utilization();
}

/**
 * @brief Check if at maximum hierarchy depth
 */
bool at_max_hierarchy_depth(uint32_t current_depth) {
    return current_depth >= MAX_HIERARCHY_DEPTH;
}

/**
 * @brief Check if enough time has passed since last spawn (hysteresis)
 */
bool spawn_hysteresis_satisfied(const WorkloadDetectorContext *ctx, double current_time) {
    if (!ctx) {
        return false;
    }
    
    double time_since_spawn = current_time - ctx->last_spawn_time;
    return time_since_spawn >= SPAWN_HYSTERESIS_SECONDS;
}

/**
 * @brief Check if enough time has passed since last despawn (hysteresis)
 */
bool despawn_hysteresis_satisfied(const WorkloadDetectorContext *ctx, double current_time) {
    if (!ctx) {
        return false;
    }
    
    double time_since_despawn = current_time - ctx->last_despawn_time;
    return time_since_despawn >= SPAWN_HYSTERESIS_SECONDS;
}

/**
 * @brief Calculate optimal number of threads for workload
 */
int calculate_optimal_threads(const WorkloadMetrics *metrics, int available_cores) {
    if (!metrics) {
        return 1;
    }
    
    // Calculate based on pending batches and available cores
    // Each thread should handle at least MIN_BATCHES_PER_THREAD batches
    
    if (metrics->pending_batches < MIN_BATCHES_PER_THREAD) {
        return 1; // Not enough work for multiple threads
    }
    
    // Calculate threads needed based on workload
    int threads_needed = (int)(metrics->pending_batches / MIN_BATCHES_PER_THREAD);
    
    // Cap at available cores
    if (threads_needed > available_cores) {
        threads_needed = available_cores;
    }
    
    // Ensure at least 1 thread
    if (threads_needed < 1) {
        threads_needed = 1;
    }
    
    return threads_needed;
}

/**
 * @brief Decide whether to spawn children
 */
SpawnDecision should_spawn_children(WorkloadDetectorContext *ctx,
                                   const WorkloadMetrics *metrics,
                                   int current_children,
                                   int available_cores) {
    if (!ctx || !metrics) {
        return SPAWN_DECISION_NO;
    }
    
    double current_time = get_current_time_seconds();
    
    // Check if at maximum hierarchy depth
    if (at_max_hierarchy_depth(metrics->current_depth)) {
        if (ctx->verbose) {
            printf("[Workload] At max hierarchy depth %u, cannot spawn\n", 
                   metrics->current_depth);
        }
        return SPAWN_DECISION_NO;
    }
    
    // Decision logic for spawning
    if (current_children == 0) {
        // Currently a worker thread, consider becoming control thread
        
        // Check if we have enough pending work
        if (metrics->pending_batches < MIN_BATCHES_PER_THREAD * 12) {
            if (ctx->verbose) {
                printf("[Workload] Not enough pending batches (%zu) to spawn 12 children\n",
                       metrics->pending_batches);
            }
            return SPAWN_DECISION_NO;
        }
        
        // Check if we have available cores (need at least 12 for children)
        if (available_cores < 12) {
            if (ctx->verbose) {
                printf("[Workload] Not enough cores (%d) to spawn 12 children\n",
                       available_cores);
            }
            return SPAWN_DECISION_NO;
        }
        
        // Check hysteresis
        if (!spawn_hysteresis_satisfied(ctx, current_time)) {
            if (ctx->verbose) {
                printf("[Workload] Spawn hysteresis not satisfied\n");
            }
            return SPAWN_DECISION_NO;
        }
        
        // All conditions met - spawn!
        ctx->last_spawn_time = current_time;
        ctx->spawn_count++;
        
        if (ctx->verbose) {
            printf("[Workload] SPAWN DECISION: YES (pending=%zu, cores=%d, depth=%u)\n",
                   metrics->pending_batches, available_cores, metrics->current_depth);
        }
        
        return SPAWN_DECISION_YES;
        
    } else {
        // Currently a control thread with children, consider despawning
        
        // Check if workload has dropped significantly
        if (metrics->pending_batches < MIN_BATCHES_PER_THREAD) {
            // Check hysteresis
            if (!despawn_hysteresis_satisfied(ctx, current_time)) {
                if (ctx->verbose) {
                    printf("[Workload] Despawn hysteresis not satisfied\n");
                }
                return SPAWN_DECISION_NO;
            }
            
            // Workload too low - despawn children
            ctx->last_despawn_time = current_time;
            ctx->despawn_count++;
            
            if (ctx->verbose) {
                printf("[Workload] DESPAWN DECISION: YES (pending=%zu, threshold=%d)\n",
                       metrics->pending_batches, MIN_BATCHES_PER_THREAD);
            }
            
            return SPAWN_DECISION_DESPAWN;
        }
        
        // Workload still high enough - keep children
        return SPAWN_DECISION_NO;
    }
}

/**
 * @brief Get workload detector statistics
 */
void workload_detector_get_stats(const WorkloadDetectorContext *ctx,
                                size_t *spawn_count,
                                size_t *despawn_count) {
    if (!ctx) {
        if (spawn_count) *spawn_count = 0;
        if (despawn_count) *despawn_count = 0;
        return;
    }
    
    if (spawn_count) {
        *spawn_count = ctx->spawn_count;
    }
    
    if (despawn_count) {
        *despawn_count = ctx->despawn_count;
    }
}

/**
 * @brief Reset workload detector statistics
 */
void workload_detector_reset_stats(WorkloadDetectorContext *ctx) {
    if (!ctx) {
        return;
    }
    
    ctx->spawn_count = 0;
    ctx->despawn_count = 0;
    ctx->last_spawn_time = 0.0;
    ctx->last_despawn_time = 0.0;
}