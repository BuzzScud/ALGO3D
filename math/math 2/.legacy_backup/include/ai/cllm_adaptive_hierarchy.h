/**
 * @file cllm_adaptive_hierarchy.h
 * @brief Entropy-aware adaptive hierarchy depth management
 * 
 * This module implements dynamic hierarchy depth adjustment based on
 * entropy distribution across dimensions. It integrates with the existing
 * hierarchy depth management and entropy optimization systems to provide
 * intelligent, adaptive spawning decisions.
 */

#ifndef CLLM_ADAPTIVE_HIERARCHY_H
#define CLLM_ADAPTIVE_HIERARCHY_H

#include "ai/cllm_entropy_integration.h"
#include "ai/cllm_entropy_allocation.h"
#include "../../algorithms/include/hierarchical_threading.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum hierarchy depth (from existing system)
 */
#define ADAPTIVE_MAX_DEPTH 5

/**
 * @brief Entropy thresholds for depth adjustment
 */
#define ENTROPY_THRESHOLD_LOW 0.3
#define ENTROPY_THRESHOLD_MEDIUM 0.6
#define ENTROPY_THRESHOLD_HIGH 0.8

/**
 * @brief Depth adjustment reasons
 */
typedef enum {
    DEPTH_ADJUST_NONE,              /**< No adjustment needed */
    DEPTH_ADJUST_ENTROPY_HIGH,      /**< High entropy - increase depth */
    DEPTH_ADJUST_ENTROPY_LOW,       /**< Low entropy - decrease depth */
    DEPTH_ADJUST_WORKLOAD_HIGH,     /**< High workload - increase depth */
    DEPTH_ADJUST_WORKLOAD_LOW,      /**< Low workload - decrease depth */
    DEPTH_ADJUST_RESOURCE_LIMIT,    /**< Resource constraints - decrease depth */
    DEPTH_ADJUST_BALANCED           /**< Balanced adjustment */
} DepthAdjustmentReason;

/**
 * @brief Depth adjustment decision
 */
typedef struct {
    int current_depth;              /**< Current hierarchy depth */
    int recommended_depth;          /**< Recommended depth */
    int depth_change;               /**< Change in depth (+/- or 0) */
    DepthAdjustmentReason reason;   /**< Reason for adjustment */
    double entropy_score;           /**< Entropy score [0,1] */
    double workload_score;          /**< Workload score [0,1] */
    double resource_score;          /**< Resource availability score [0,1] */
    bool should_adjust;             /**< Whether to adjust depth */
} DepthAdjustmentDecision;

/**
 * @brief Adaptive hierarchy configuration
 */
typedef struct {
    double entropy_weight;          /**< Weight for entropy factor (0-1) */
    double workload_weight;         /**< Weight for workload factor (0-1) */
    double resource_weight;         /**< Weight for resource factor (0-1) */
    double adjustment_threshold;    /**< Threshold for triggering adjustment */
    uint64_t adjustment_interval;   /**< Minimum steps between adjustments */
    bool enable_aggressive_scaling; /**< Enable aggressive depth scaling */
    int min_depth;                  /**< Minimum allowed depth */
    int max_depth;                  /**< Maximum allowed depth */
} AdaptiveHierarchyConfig;

/**
 * @brief Adaptive hierarchy context
 */
typedef struct {
    AdaptiveHierarchyConfig config;         /**< Configuration */
    EntropyIntegrationContext* entropy_ctx; /**< Entropy integration context */
    int current_depth;                      /**< Current hierarchy depth */
    int recommended_depth;                  /**< Last recommended depth */
    uint64_t last_adjustment_step;          /**< Last adjustment step */
    uint64_t adjustment_count;              /**< Number of adjustments made */
    double last_entropy_score;              /**< Last entropy score */
    double last_workload_score;             /**< Last workload score */
    double last_resource_score;             /**< Last resource score */
    DepthAdjustmentDecision last_decision;  /**< Last adjustment decision */
} AdaptiveHierarchyContext;

/**
 * @brief Initialize adaptive hierarchy configuration with defaults
 * 
 * @param config Pointer to configuration to initialize
 */
void adaptive_hierarchy_config_init_default(AdaptiveHierarchyConfig* config);

/**
 * @brief Initialize adaptive hierarchy context
 * 
 * @param ctx Pointer to adaptive hierarchy context
 * @param entropy_ctx Pointer to entropy integration context
 * @param config Pointer to configuration (NULL for defaults)
 * @return true on success, false on failure
 */
bool adaptive_hierarchy_init(
    AdaptiveHierarchyContext* ctx,
    EntropyIntegrationContext* entropy_ctx,
    const AdaptiveHierarchyConfig* config
);

/**
 * @brief Destroy adaptive hierarchy context
 * 
 * @param ctx Pointer to adaptive hierarchy context
 */
void adaptive_hierarchy_destroy(AdaptiveHierarchyContext* ctx);

/**
 * @brief Calculate entropy-aware depth recommendation
 * 
 * Analyzes current entropy distribution and recommends optimal hierarchy depth.
 * 
 * @param ctx Adaptive hierarchy context
 * @param available_cores Number of available CPU cores
 * @param current_thread_count Current total number of threads
 * @param pending_batches Number of pending work batches
 * @return Recommended hierarchy depth
 */
int calculate_entropy_aware_depth(
    AdaptiveHierarchyContext* ctx,
    int available_cores,
    int current_thread_count,
    int pending_batches
);

/**
 * @brief Evaluate depth adjustment decision
 * 
 * Evaluates whether hierarchy depth should be adjusted based on entropy,
 * workload, and resource availability.
 * 
 * @param ctx Adaptive hierarchy context
 * @param available_cores Number of available CPU cores
 * @param current_thread_count Current total number of threads
 * @param pending_batches Number of pending work batches
 * @param decision Output parameter for adjustment decision
 * @return true if adjustment is recommended, false otherwise
 */
bool evaluate_depth_adjustment(
    AdaptiveHierarchyContext* ctx,
    int available_cores,
    int current_thread_count,
    int pending_batches,
    DepthAdjustmentDecision* decision
);

/**
 * @brief Calculate entropy score for depth decision
 * 
 * Analyzes entropy distribution to determine if deeper hierarchy is beneficial.
 * Higher entropy suggests more complex computation requiring deeper hierarchy.
 * 
 * @param ctx Adaptive hierarchy context
 * @return Entropy score [0,1]
 */
double calculate_entropy_score(const AdaptiveHierarchyContext* ctx);

/**
 * @brief Calculate workload score for depth decision
 * 
 * Analyzes pending workload to determine if deeper hierarchy is needed.
 * 
 * @param pending_batches Number of pending work batches
 * @param current_thread_count Current total number of threads
 * @return Workload score [0,1]
 */
double calculate_workload_score(int pending_batches, int current_thread_count);

/**
 * @brief Calculate resource availability score
 * 
 * Analyzes available resources (cores, threads) to determine spawning capacity.
 * 
 * @param available_cores Number of available CPU cores
 * @param current_thread_count Current total number of threads
 * @return Resource score [0,1]
 */
double calculate_resource_score(int available_cores, int current_thread_count);

/**
 * @brief Check if depth adjustment should be applied
 * 
 * Checks if enough time has passed since last adjustment and if the
 * adjustment threshold is met.
 * 
 * @param ctx Adaptive hierarchy context
 * @param current_step Current training step
 * @param adjustment_score Combined adjustment score
 * @return true if adjustment should be applied, false otherwise
 */
bool should_apply_depth_adjustment(
    const AdaptiveHierarchyContext* ctx,
    uint64_t current_step,
    double adjustment_score
);

/**
 * @brief Apply depth adjustment decision
 * 
 * Updates the context with the new depth recommendation.
 * 
 * @param ctx Adaptive hierarchy context
 * @param decision Depth adjustment decision to apply
 * @param current_step Current training step
 */
void apply_depth_adjustment(
    AdaptiveHierarchyContext* ctx,
    const DepthAdjustmentDecision* decision,
    uint64_t current_step
);

/**
 * @brief Get recommended children count with entropy awareness
 * 
 * Extends the existing get_recommended_children_count() with entropy awareness.
 * 
 * @param ctx Adaptive hierarchy context
 * @param current_depth Current hierarchy depth
 * @param available_cores Number of available CPU cores
 * @param pending_batches Number of pending work batches
 * @return Recommended number of children (0-12)
 */
int get_entropy_aware_children_count(
    const AdaptiveHierarchyContext* ctx,
    int current_depth,
    int available_cores,
    int pending_batches
);

/**
 * @brief Check if spawning is allowed with entropy awareness
 * 
 * Extends the existing can_spawn_at_depth() with entropy awareness.
 * 
 * @param ctx Adaptive hierarchy context
 * @param current_depth Current hierarchy depth
 * @param available_cores Number of available CPU cores
 * @param current_thread_count Current total number of threads
 * @return true if spawning is allowed, false otherwise
 */
bool can_spawn_with_entropy_awareness(
    const AdaptiveHierarchyContext* ctx,
    int current_depth,
    int available_cores,
    int current_thread_count
);

/**
 * @brief Get depth adjustment statistics
 * 
 * @param ctx Adaptive hierarchy context
 * @param current_depth Output parameter for current depth
 * @param recommended_depth Output parameter for recommended depth
 * @param adjustment_count Output parameter for adjustment count
 */
void get_depth_adjustment_stats(
    const AdaptiveHierarchyContext* ctx,
    int* current_depth,
    int* recommended_depth,
    uint64_t* adjustment_count
);

/**
 * @brief Print depth adjustment decision
 * 
 * @param decision Depth adjustment decision to print
 */
void print_depth_adjustment_decision(const DepthAdjustmentDecision* decision);

#ifdef __cplusplus
}
#endif

#endif // CLLM_ADAPTIVE_HIERARCHY_H