/**
 * @file cllm_workload_detector.h
 * @brief Workload detection system for dynamic thread spawning
 * 
 * Monitors system workload and makes intelligent decisions about when to
 * spawn or despawn child threads in the recursive hierarchy.
 * 
 * Key Features:
 * - Real-time workload metrics collection
 * - Intelligent spawn/despawn decisions
 * - Hysteresis to prevent thrashing
 * - Core availability detection
 * - Hierarchy depth management
 */

#ifndef CLLM_WORKLOAD_DETECTOR_H
#define CLLM_WORKLOAD_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum hierarchy depth (prevents infinite recursion)
 */
#define MAX_HIERARCHY_DEPTH 5

/**
 * @brief Minimum batches per thread to justify spawning
 */
#define MIN_BATCHES_PER_THREAD 10

/**
 * @brief Hysteresis threshold to prevent thrashing (in seconds)
 */
#define SPAWN_HYSTERESIS_SECONDS 5.0

/**
 * @brief Workload metrics structure
 */
typedef struct {
    size_t pending_batches;      /**< Number of batches waiting to be processed */
    size_t active_workers;       /**< Number of currently active worker threads */
    double avg_batch_time;       /**< Average time to process one batch (seconds) */
    double cpu_utilization;      /**< CPU utilization (0.0 to 1.0) */
    size_t completed_batches;    /**< Total batches completed */
    double total_processing_time; /**< Total time spent processing (seconds) */
    uint32_t current_depth;      /**< Current depth in hierarchy */
} WorkloadMetrics;

/**
 * @brief Spawn decision enumeration
 */
typedef enum {
    SPAWN_DECISION_NO,           /**< Don't change current state */
    SPAWN_DECISION_YES,          /**< Spawn 12 children (become control thread) */
    SPAWN_DECISION_DESPAWN       /**< Despawn children (become worker again) */
} SpawnDecision;

/**
 * @brief Workload detector context
 */
typedef struct {
    double last_spawn_time;      /**< Timestamp of last spawn decision */
    double last_despawn_time;    /**< Timestamp of last despawn decision */
    size_t spawn_count;          /**< Total number of spawns */
    size_t despawn_count;        /**< Total number of despawns */
    bool verbose;                /**< Enable verbose logging */
} WorkloadDetectorContext;

/**
 * @brief Initialize workload detector context
 * 
 * @param ctx Context to initialize
 * @param verbose Enable verbose logging
 * @return true on success, false on failure
 */
bool workload_detector_init(WorkloadDetectorContext *ctx, bool verbose);

/**
 * @brief Destroy workload detector context
 * 
 * @param ctx Context to destroy
 */
void workload_detector_destroy(WorkloadDetectorContext *ctx);

/**
 * @brief Collect current workload metrics
 * 
 * Gathers real-time metrics about the current workload including pending
 * batches, active workers, CPU utilization, and processing times.
 * 
 * @param pending_batches Number of batches waiting
 * @param active_workers Number of active worker threads
 * @param completed_batches Total batches completed
 * @param total_time Total processing time
 * @param current_depth Current depth in hierarchy
 * @param metrics Output: collected metrics
 */
void collect_workload_metrics(size_t pending_batches,
                              size_t active_workers,
                              size_t completed_batches,
                              double total_time,
                              uint32_t current_depth,
                              WorkloadMetrics *metrics);

/**
 * @brief Decide whether to spawn children
 * 
 * Makes an intelligent decision about spawning based on:
 * - Current workload (pending batches)
 * - Available CPU cores
 * - Current hierarchy depth
 * - Recent spawn/despawn history (hysteresis)
 * - CPU utilization
 * 
 * @param ctx Detector context
 * @param metrics Current workload metrics
 * @param current_children Number of children currently spawned
 * @param available_cores Number of available CPU cores
 * @return Spawn decision (YES, NO, or DESPAWN)
 */
SpawnDecision should_spawn_children(WorkloadDetectorContext *ctx,
                                   const WorkloadMetrics *metrics,
                                   int current_children,
                                   int available_cores);

/**
 * @brief Get number of available CPU cores
 * 
 * Detects the number of CPU cores available on the system.
 * 
 * @return Number of available cores
 */
int get_available_cores(void);

/**
 * @brief Get current CPU utilization
 * 
 * Measures current CPU utilization as a value between 0.0 and 1.0.
 * 
 * @return CPU utilization (0.0 = idle, 1.0 = fully utilized)
 */
double get_cpu_utilization(void);

/**
 * @brief Calculate optimal number of threads for workload
 * 
 * Determines the optimal number of threads based on workload and
 * available resources.
 * 
 * @param metrics Current workload metrics
 * @param available_cores Number of available cores
 * @return Optimal number of threads
 */
int calculate_optimal_threads(const WorkloadMetrics *metrics, int available_cores);

/**
 * @brief Check if at maximum hierarchy depth
 * 
 * @param current_depth Current depth in hierarchy
 * @return true if at maximum depth, false otherwise
 */
bool at_max_hierarchy_depth(uint32_t current_depth);

/**
 * @brief Get workload detector statistics
 * 
 * @param ctx Detector context
 * @param spawn_count Output: total spawns
 * @param despawn_count Output: total despawns
 */
void workload_detector_get_stats(const WorkloadDetectorContext *ctx,
                                size_t *spawn_count,
                                size_t *despawn_count);

/**
 * @brief Reset workload detector statistics
 * 
 * @param ctx Detector context
 */
void workload_detector_reset_stats(WorkloadDetectorContext *ctx);

/**
 * @brief Get current timestamp in seconds
 * 
 * @return Current time in seconds since epoch
 */
double get_current_time_seconds(void);

/**
 * @brief Check if enough time has passed since last spawn (hysteresis)
 * 
 * @param ctx Detector context
 * @param current_time Current timestamp
 * @return true if enough time has passed, false otherwise
 */
bool spawn_hysteresis_satisfied(const WorkloadDetectorContext *ctx, double current_time);

/**
 * @brief Check if enough time has passed since last despawn (hysteresis)
 * 
 * @param ctx Detector context
 * @param current_time Current timestamp
 * @return true if enough time has passed, false otherwise
 */
bool despawn_hysteresis_satisfied(const WorkloadDetectorContext *ctx, double current_time);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_WORKLOAD_DETECTOR_H */