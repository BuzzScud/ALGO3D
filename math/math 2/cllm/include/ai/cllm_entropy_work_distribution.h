/**
 * @file cllm_entropy_work_distribution.h
 * @brief Entropy-based work distribution for crystalline CLLM
 * 
 * This module extends the Plimpton 322 work distribution with entropy-based
 * weighting. Work is distributed proportionally to both Plimpton ratios AND
 * entropy across dimensions.
 */

#ifndef CLLM_ENTROPY_WORK_DISTRIBUTION_H
#define CLLM_ENTROPY_WORK_DISTRIBUTION_H

#include "ai/cllm_work_distribution.h"
#include "ai/cllm_entropy_allocation.h"
#include "ai/cllm_entropy_integration.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum batch size for work distribution
 */
#define MAX_BATCH_SIZE 10000

/**
 * @brief Work assignment for a single thread
 */
typedef struct {
    uint64_t thread_id;              /**< Thread ID */
    uint32_t dimension;              /**< Dimension (0-11) */
    size_t start_index;              /**< Start index in batch */
    size_t end_index;                /**< End index in batch */
    size_t work_size;                /**< Amount of work (end - start) */
    double entropy_weight;           /**< Entropy weight for this dimension */
    double plimpton_ratio;           /**< Plimpton ratio (if applicable) */
    double combined_weight;          /**< Combined entropy × Plimpton weight */
} WorkAssignment;

/**
 * @brief Complete work distribution plan
 */
typedef struct {
    WorkAssignment* assignments;     /**< Array of work assignments */
    size_t num_assignments;          /**< Number of assignments */
    size_t total_work_size;          /**< Total work to distribute */
    size_t distributed_work;         /**< Work actually distributed */
    double total_entropy;            /**< Total system entropy */
    bool use_plimpton_ratios;        /**< Use Plimpton ratios */
    bool enforce_12fold;             /**< Enforce 12-fold symmetry */
} WorkDistributionPlan;

/**
 * @brief Work distribution strategy
 */
typedef enum {
    WORK_DIST_ENTROPY_ONLY,          /**< Pure entropy-based */
    WORK_DIST_PLIMPTON_ONLY,         /**< Pure Plimpton-based */
    WORK_DIST_COMBINED,              /**< Entropy × Plimpton */
    WORK_DIST_ADAPTIVE               /**< Adaptive based on workload */
} WorkDistributionStrategy;

/**
 * @brief Work distribution configuration
 */
typedef struct {
    WorkDistributionStrategy strategy;  /**< Distribution strategy */
    double entropy_weight;           /**< Weight for entropy (0-1) */
    double plimpton_weight;          /**< Weight for Plimpton (0-1) */
    bool enforce_12fold;             /**< Enforce 12-fold symmetry */
    size_t min_work_per_thread;      /**< Minimum work per thread */
    size_t max_work_per_thread;      /**< Maximum work per thread */
} WorkDistributionConfig;

/**
 * @brief Initialize work distribution configuration
 * 
 * @param config Configuration to initialize
 */
void work_distribution_config_init(WorkDistributionConfig* config);

/**
 * @brief Create work distribution plan
 * 
 * @param total_work Total work size to distribute
 * @param max_assignments Maximum number of assignments
 * @return Allocated work distribution plan (must be freed)
 */
WorkDistributionPlan* work_distribution_plan_create(
    size_t total_work,
    size_t max_assignments
);

/**
 * @brief Destroy work distribution plan
 * 
 * @param plan Plan to destroy
 */
void work_distribution_plan_destroy(WorkDistributionPlan* plan);

/**
 * @brief Calculate entropy-based work distribution
 * 
 * Distributes work across threads based on entropy allocation plan.
 * 
 * @param entropy_ctx Entropy integration context
 * @param allocation_plan Thread allocation plan
 * @param total_work Total work size
 * @param config Distribution configuration
 * @param plan Output work distribution plan
 * @return true on success, false on failure
 */
bool calculate_entropy_work_distribution(
    EntropyIntegrationContext* entropy_ctx,
    const ThreadAllocationPlan* allocation_plan,
    size_t total_work,
    const WorkDistributionConfig* config,
    WorkDistributionPlan* plan
);

/**
 * @brief Calculate work for specific dimension
 * 
 * @param dimension Dimension index (0-11)
 * @param entropy Entropy for dimension
 * @param total_entropy Total system entropy
 * @param total_work Total work to distribute
 * @return Work size for dimension
 */
size_t calculate_dimension_work(
    uint32_t dimension,
    double entropy,
    double total_entropy,
    size_t total_work
);

/**
 * @brief Distribute work with Plimpton ratios
 * 
 * Combines entropy-based allocation with Plimpton 322 ratios
 * for parent-child work distribution.
 * 
 * @param parent_work Parent's work amount
 * @param parent_id Parent thread ID
 * @param child_id Child thread ID
 * @param parent_entropy Parent dimension entropy
 * @param child_entropy Child dimension entropy
 * @return Work distribution with combined weights
 */
WorkDistribution calculate_combined_work_distribution(
    size_t parent_work,
    uint64_t parent_id,
    uint64_t child_id,
    double parent_entropy,
    double child_entropy
);

/**
 * @brief Assign work to threads based on plan
 * 
 * @param plan Work distribution plan
 * @param thread_ids Array of thread IDs
 * @param num_threads Number of threads
 * @return true on success
 */
bool assign_work_to_threads(
    WorkDistributionPlan* plan,
    const uint64_t* thread_ids,
    size_t num_threads
);

/**
 * @brief Validate work distribution plan
 * 
 * @param plan Work distribution plan
 * @return true if valid
 */
bool validate_work_distribution_plan(const WorkDistributionPlan* plan);

/**
 * @brief Get work assignment for thread
 * 
 * @param plan Work distribution plan
 * @param thread_id Thread ID
 * @return Pointer to work assignment (NULL if not found)
 */
const WorkAssignment* get_thread_work_assignment(
    const WorkDistributionPlan* plan,
    uint64_t thread_id
);

/**
 * @brief Calculate work distribution efficiency
 * 
 * Returns ratio of distributed work to total work.
 * 
 * @param plan Work distribution plan
 * @return Efficiency (0.0 to 1.0)
 */
double calculate_work_distribution_efficiency(const WorkDistributionPlan* plan);

/**
 * @brief Calculate work distribution balance
 * 
 * Returns coefficient of variation of work sizes.
 * 
 * @param plan Work distribution plan
 * @return Balance metric (0.0 = perfectly balanced)
 */
double calculate_work_distribution_balance(const WorkDistributionPlan* plan);

/**
 * @brief Print work distribution plan
 * 
 * @param plan Work distribution plan
 */
void print_work_distribution_plan(const WorkDistributionPlan* plan);

/**
 * @brief Print work distribution statistics
 * 
 * @param plan Work distribution plan
 */
void print_work_distribution_statistics(const WorkDistributionPlan* plan);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_ENTROPY_WORK_DISTRIBUTION_H */