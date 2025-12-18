/**
 * @file cllm_entropy_allocation.h
 * @brief Entropy-based thread allocation for crystalline CLLM
 * 
 * This module implements intelligent thread allocation based on entropy
 * distribution across dimensions. Higher entropy dimensions receive more
 * threads to handle increased computational complexity.
 */

#ifndef CLLM_ENTROPY_ALLOCATION_H
#define CLLM_ENTROPY_ALLOCATION_H

#include "ai/cllm_entropy_integration.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of dimensions for allocation
 */
#define MAX_ALLOCATION_DIMENSIONS 12

/**
 * @brief Thread allocation for a single dimension
 */
typedef struct {
    uint32_t dimension;              /**< Dimension index (0-11) */
    double entropy;                  /**< Entropy value for this dimension */
    double entropy_fraction;         /**< Fraction of total entropy */
    int allocated_threads;           /**< Number of threads allocated */
    int adjusted_threads;            /**< Threads after 12-fold adjustment */
    bool is_active;                  /**< Whether this dimension is active */
} DimensionAllocation;

/**
 * @brief Complete thread allocation plan
 */
typedef struct {
    DimensionAllocation dimensions[MAX_ALLOCATION_DIMENSIONS];  /**< Per-dimension allocations */
    int total_available_threads;     /**< Total threads available */
    int total_allocated_threads;     /**< Total threads allocated */
    int total_adjusted_threads;      /**< Total after 12-fold adjustment */
    double total_entropy;            /**< Total system entropy */
    uint32_t active_dimensions;      /**< Number of active dimensions */
    bool enforce_12fold;             /**< Enforce 12-fold symmetry */
} ThreadAllocationPlan;

/**
 * @brief Allocation strategy options
 */
typedef enum {
    ALLOCATION_PROPORTIONAL,         /**< Proportional to entropy */
    ALLOCATION_THRESHOLD,            /**< Only above entropy threshold */
    ALLOCATION_BALANCED,             /**< Balance between proportional and equal */
    ALLOCATION_ADAPTIVE              /**< Adaptive based on workload */
} AllocationStrategy;

/**
 * @brief Allocation configuration
 */
typedef struct {
    AllocationStrategy strategy;     /**< Allocation strategy */
    double entropy_threshold;        /**< Minimum entropy for allocation */
    bool enforce_12fold;             /**< Enforce 12-fold symmetry */
    int min_threads_per_dimension;   /**< Minimum threads per active dimension */
    int max_threads_per_dimension;   /**< Maximum threads per dimension */
    double balance_factor;           /**< Balance factor for BALANCED strategy (0-1) */
} AllocationConfig;

/**
 * @brief Initialize allocation configuration with defaults
 * 
 * @param config Pointer to configuration to initialize
 */
void allocation_config_init_default(AllocationConfig* config);

/**
 * @brief Calculate thread allocation based on entropy distribution
 * 
 * Allocates threads proportionally to entropy across dimensions.
 * Higher entropy dimensions receive more threads.
 * 
 * @param ctx Entropy integration context
 * @param available_threads Total threads available for allocation
 * @param config Allocation configuration
 * @param plan Output allocation plan
 * @return true on success, false on failure
 */
bool calculate_thread_allocation(
    EntropyIntegrationContext* ctx,
    int available_threads,
    const AllocationConfig* config,
    ThreadAllocationPlan* plan
);

/**
 * @brief Calculate allocation for specific dimension
 * 
 * @param entropy Entropy value for dimension
 * @param total_entropy Total system entropy
 * @param available_threads Total threads available
 * @param config Allocation configuration
 * @return Number of threads to allocate
 */
int calculate_dimension_threads(
    double entropy,
    double total_entropy,
    int available_threads,
    const AllocationConfig* config
);

/**
 * @brief Adjust thread count to enforce 12-fold symmetry
 * 
 * Rounds thread count to nearest multiple of 12.
 * 
 * @param threads Original thread count
 * @return Adjusted thread count (multiple of 12)
 */
int adjust_to_12fold_symmetry(int threads);

/**
 * @brief Validate thread allocation plan
 * 
 * Ensures allocation plan is valid and consistent.
 * 
 * @param plan Allocation plan to validate
 * @return true if valid, false otherwise
 */
bool validate_allocation_plan(const ThreadAllocationPlan* plan);

/**
 * @brief Apply allocation plan to training system
 * 
 * Updates thread allocation in the training system based on plan.
 * 
 * @param plan Allocation plan to apply
 * @return true on success, false on failure
 */
bool apply_allocation_plan(const ThreadAllocationPlan* plan);

/**
 * @brief Get allocation for specific dimension
 * 
 * @param plan Allocation plan
 * @param dimension Dimension index (0-11)
 * @return Pointer to dimension allocation (NULL if invalid)
 */
const DimensionAllocation* get_dimension_allocation(
    const ThreadAllocationPlan* plan,
    uint32_t dimension
);

/**
 * @brief Calculate allocation efficiency
 * 
 * Returns ratio of allocated threads to available threads.
 * 
 * @param plan Allocation plan
 * @return Efficiency ratio (0.0 to 1.0)
 */
double calculate_allocation_efficiency(const ThreadAllocationPlan* plan);

/**
 * @brief Calculate allocation balance
 * 
 * Returns measure of how balanced the allocation is across dimensions.
 * Lower values indicate more balanced allocation.
 * 
 * @param plan Allocation plan
 * @return Balance metric (0.0 = perfectly balanced)
 */
double calculate_allocation_balance(const ThreadAllocationPlan* plan);

/**
 * @brief Rebalance allocation plan
 * 
 * Adjusts allocation to improve balance while respecting entropy.
 * 
 * @param plan Allocation plan to rebalance (modified in place)
 * @param target_balance Target balance metric
 * @return true if rebalancing successful
 */
bool rebalance_allocation(ThreadAllocationPlan* plan, double target_balance);

/**
 * @brief Compare two allocation plans
 * 
 * Returns difference metric between two plans.
 * 
 * @param plan1 First allocation plan
 * @param plan2 Second allocation plan
 * @return Difference metric (0.0 = identical)
 */
double compare_allocation_plans(
    const ThreadAllocationPlan* plan1,
    const ThreadAllocationPlan* plan2
);

/**
 * @brief Print allocation plan
 * 
 * @param plan Allocation plan to print
 */
void print_allocation_plan(const ThreadAllocationPlan* plan);

/**
 * @brief Print allocation statistics
 * 
 * @param plan Allocation plan
 */
void print_allocation_statistics(const ThreadAllocationPlan* plan);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_ENTROPY_ALLOCATION_H */
