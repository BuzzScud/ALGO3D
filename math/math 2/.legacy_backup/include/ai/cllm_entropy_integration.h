/**
 * @file cllm_entropy_integration.h
 * @brief Integration of entropy calculation with CLLM training system
 * 
 * This module integrates the lattice entropy Γ(n,d) with the CLLM model
 * to enable entropy-based optimization, monitoring, and work distribution.
 */

#ifndef CLLM_ENTROPY_INTEGRATION_H
#define CLLM_ENTROPY_INTEGRATION_H

#include "ai/cllm_lattice_entropy.h"
#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of dimensions to track
 */
#define MAX_TRACKED_DIMENSIONS 12

/**
 * @brief Entropy statistics for a single dimension
 */
typedef struct {
    uint32_t dimension;              /**< Dimension index (0-11) */
    double current_entropy;          /**< Current entropy value */
    double min_entropy;              /**< Minimum entropy seen */
    double max_entropy;              /**< Maximum entropy seen */
    double avg_entropy;              /**< Average entropy */
    uint64_t sample_count;           /**< Number of samples */
    double entropy_variance;         /**< Variance of entropy */
} DimensionEntropyStats;

/**
 * @brief Model-wide entropy statistics
 */
typedef struct {
    DimensionEntropyStats dimensions[MAX_TRACKED_DIMENSIONS];  /**< Per-dimension stats */
    double total_entropy;            /**< Total system entropy */
    double normalized_entropy;       /**< Normalized total entropy [0,1] */
    uint64_t update_count;           /**< Number of updates */
    double entropy_trend;            /**< Entropy change rate */
    double last_total_entropy;       /**< Previous total entropy */
} ModelEntropyStats;

/**
 * @brief Entropy integration context
 */
typedef struct {
    EntropyContext entropy_ctx;      /**< Base entropy context */
    ModelEntropyStats stats;         /**< Model-wide statistics */
    CLLMModel* model;                /**< Associated model */
    bool monitoring_enabled;         /**< Enable entropy monitoring */
    uint64_t monitoring_interval;    /**< Monitoring interval (steps) */
    uint64_t last_monitoring_step;   /**< Last monitoring step */
} EntropyIntegrationContext;

/**
 * @brief Initialize entropy integration context
 * 
 * @param ctx Pointer to entropy integration context
 * @param model Pointer to CLLM model
 * @return true on success, false on failure
 */
bool entropy_integration_init(EntropyIntegrationContext* ctx, CLLMModel* model);

/**
 * @brief Destroy entropy integration context
 * 
 * @param ctx Pointer to entropy integration context
 */
void entropy_integration_destroy(EntropyIntegrationContext* ctx);

/**
 * @brief Calculate entropy for model at current state
 * 
 * Calculates entropy across all dimensions based on current model state.
 * 
 * @param ctx Entropy integration context
 * @param sequence_length Current sequence length
 * @return Total model entropy
 */
double calculate_model_entropy(EntropyIntegrationContext* ctx, uint64_t sequence_length);

/**
 * @brief Calculate entropy for specific dimension
 * 
 * @param ctx Entropy integration context
 * @param dimension Dimension index (0-11)
 * @param sequence_length Current sequence length
 * @return Dimension entropy
 */
double calculate_dimension_entropy(
    EntropyIntegrationContext* ctx,
    uint32_t dimension,
    uint64_t sequence_length
);

/**
 * @brief Update entropy statistics
 * 
 * Updates running statistics for entropy monitoring.
 * 
 * @param ctx Entropy integration context
 * @param sequence_length Current sequence length
 */
void update_entropy_statistics(EntropyIntegrationContext* ctx, uint64_t sequence_length);

/**
 * @brief Get entropy statistics for dimension
 * 
 * @param ctx Entropy integration context
 * @param dimension Dimension index (0-11)
 * @return Pointer to dimension statistics (NULL if invalid)
 */
const DimensionEntropyStats* get_dimension_stats(
    const EntropyIntegrationContext* ctx,
    uint32_t dimension
);

/**
 * @brief Get model-wide entropy statistics
 * 
 * @param ctx Entropy integration context
 * @return Pointer to model statistics
 */
const ModelEntropyStats* get_model_entropy_stats(const EntropyIntegrationContext* ctx);

/**
 * @brief Enable entropy monitoring
 * 
 * @param ctx Entropy integration context
 * @param interval Monitoring interval in training steps
 */
void enable_entropy_monitoring(EntropyIntegrationContext* ctx, uint64_t interval);

/**
 * @brief Disable entropy monitoring
 * 
 * @param ctx Entropy integration context
 */
void disable_entropy_monitoring(EntropyIntegrationContext* ctx);

/**
 * @brief Check if monitoring should occur at current step
 * 
 * @param ctx Entropy integration context
 * @param current_step Current training step
 * @return true if monitoring should occur
 */
bool should_monitor_entropy(EntropyIntegrationContext* ctx, uint64_t current_step);

/**
 * @brief Perform entropy monitoring at current step
 * 
 * @param ctx Entropy integration context
 * @param current_step Current training step
 * @param sequence_length Current sequence length
 */
void monitor_entropy_step(
    EntropyIntegrationContext* ctx,
    uint64_t current_step,
    uint64_t sequence_length
);

/**
 * @brief Calculate entropy trend
 * 
 * Returns the rate of change of entropy (positive = increasing, negative = decreasing).
 * 
 * @param ctx Entropy integration context
 * @return Entropy trend value
 */
double calculate_entropy_trend(const EntropyIntegrationContext* ctx);

/**
 * @brief Get entropy for lattice point
 * 
 * Calculates entropy for a specific lattice point in the model.
 * 
 * @param ctx Entropy integration context
 * @param point_index Index of lattice point
 * @return Lattice point entropy
 */
double get_lattice_point_entropy(
    EntropyIntegrationContext* ctx,
    uint64_t point_index
);

/**
 * @brief Calculate entropy distribution across dimensions
 * 
 * Returns array of entropy values for all dimensions.
 * 
 * @param ctx Entropy integration context
 * @param sequence_length Current sequence length
 * @param entropies Output array (must have space for 12 doubles)
 */
void calculate_entropy_distribution(
    EntropyIntegrationContext* ctx,
    uint64_t sequence_length,
    double* entropies
);

/**
 * @brief Reset entropy statistics
 * 
 * @param ctx Entropy integration context
 */
void reset_entropy_statistics(EntropyIntegrationContext* ctx);

/**
 * @brief Print entropy statistics
 * 
 * @param ctx Entropy integration context
 */
void print_entropy_statistics(const EntropyIntegrationContext* ctx);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_ENTROPY_INTEGRATION_H */