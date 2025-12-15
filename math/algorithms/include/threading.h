#ifndef ALGORITHMS_THREADING_H
#define ALGORITHMS_THREADING_H

/**
 * Threading Algorithms Library
 * 
 * Generic thread allocation and workload distribution algorithms.
 * Provides strategies for mapping N physical threads to M logical work groups.
 * 
 * Key Features:
 * - Cross-platform CPU core detection
 * - Multiple allocation strategies (round-robin, balanced, priority-based)
 * - Workload estimation and load balancing
 * - Thread affinity management
 * - Dynamic rebalancing support
 * 
 * Mathematical Foundation:
 * - Uses prime number theorem for workload estimation
 * - All math operations use crystalline library (prime_* functions)
 * - No external math library dependencies
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// THREAD ALLOCATION STRATEGIES
// ============================================================================

/**
 * Thread Allocation Strategy Types
 */
typedef enum {
    THREAD_STRATEGY_ONE_TO_ONE,           // N >= M: one thread per work group
    THREAD_STRATEGY_ROUND_ROBIN,          // N < M: round-robin assignment
    THREAD_STRATEGY_BALANCED,             // N < M: load-balanced assignment
    THREAD_STRATEGY_PRIORITY_BASED,       // N < M: priority-based assignment
    THREAD_STRATEGY_DYNAMIC               // Dynamic load balancing
} ThreadStrategyType;

/**
 * Work Group Mapping
 * 
 * Maps a physical thread to one or more logical work groups.
 * When N < M, each physical thread handles multiple work groups.
 */
typedef struct {
    int physical_thread_id;        // 0 to (num_threads - 1)
    int* work_groups;              // Array of work group IDs
    int num_work_groups;           // How many groups this thread handles
    
    // Load balancing information
    uint64_t estimated_work_units; // Estimated work units across all groups
    double expected_workload;      // Expected computational load (0.0 to 1.0)
    double actual_workload;        // Actual measured workload (updated dynamically)
    
    // CPU affinity
    int preferred_cpu;             // Preferred CPU core for this thread
    bool affinity_set;             // Whether CPU affinity has been set
    
} WorkGroupMapping;

/**
 * Thread Allocation Strategy
 * 
 * Manages the mapping of N physical threads to M logical work groups.
 * Ensures all M groups are covered while adapting to hardware constraints.
 */
typedef struct {
    // Configuration
    int num_physical_threads;      // Actual hardware threads available
    int num_work_groups;           // Number of logical work groups
    
    // Mapping
    WorkGroupMapping* mappings;    // Array of num_physical_threads mappings
    
    // Strategy
    ThreadStrategyType strategy;
    
    // Validation
    bool all_groups_covered;       // True if all M groups are assigned
    int* group_to_thread_map;      // Map from work group to physical thread
    
    // Statistics
    double load_balance_factor;    // Measure of load balance (1.0 = perfect)
    double max_workload;           // Maximum workload on any thread
    double min_workload;           // Minimum workload on any thread
    double total_workload;         // Total workload across all threads
    
    // Dynamic rebalancing
    uint64_t rebalance_count;      // Number of times rebalanced
    uint64_t work_units_completed; // Total work units completed
    
} ThreadAllocationStrategy;

// ============================================================================
// WORKLOAD ESTIMATION
// ============================================================================

/**
 * Workload Estimator Function Type
 * 
 * User-provided function to estimate workload for a specific work group.
 * 
 * @param work_group Work group ID
 * @param user_data User-provided context data
 * @return Estimated workload (in arbitrary work units)
 */
typedef uint64_t (*WorkloadEstimator)(int work_group, void* user_data);

/**
 * Default workload estimator (uniform distribution)
 */
uint64_t default_workload_estimator(int work_group, void* user_data);

/**
 * Prime-based workload estimator
 * 
 * Estimates workload based on prime distribution using prime number theorem.
 * Useful for algorithms that process primes in different modulo classes.
 * 
 * @param work_group Work group ID (interpreted as modulo class)
 * @param user_data Pointer to range [start, end] as uint64_t[2]
 * @return Estimated number of primes in the work group
 */
uint64_t prime_workload_estimator(int work_group, void* user_data);

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

/**
 * Detect number of CPU cores
 * 
 * Cross-platform detection of available CPU cores.
 * 
 * @return Number of CPU cores, or -1 on error
 */
int detect_num_cpu_cores(void);

/**
 * Get optimal thread count
 * 
 * Returns the optimal number of threads for the current hardware.
 * This is min(num_physical_cores, num_work_groups) for best performance.
 * 
 * @param num_work_groups Number of logical work groups
 * @return Optimal thread count
 */
int get_optimal_thread_count(int num_work_groups);

/**
 * Create thread allocation strategy
 * 
 * Creates a mapping of N physical threads to M work groups.
 * Automatically selects the best strategy based on N and M.
 * 
 * @param num_physical_threads Number of physical threads available
 * @param num_work_groups Number of logical work groups
 * @param estimator Workload estimator function (NULL for uniform)
 * @param estimator_data User data for estimator function
 * @return Allocated strategy, or NULL on error
 */
ThreadAllocationStrategy* thread_allocation_create(
    int num_physical_threads,
    int num_work_groups,
    WorkloadEstimator estimator,
    void* estimator_data
);

/**
 * Create thread allocation with specific strategy
 * 
 * Creates a mapping using a specific allocation strategy.
 * 
 * @param num_physical_threads Number of physical threads available
 * @param num_work_groups Number of logical work groups
 * @param strategy Specific strategy to use
 * @param estimator Workload estimator function (NULL for uniform)
 * @param estimator_data User data for estimator function
 * @return Allocated strategy, or NULL on error
 */
ThreadAllocationStrategy* thread_allocation_create_with_strategy(
    int num_physical_threads,
    int num_work_groups,
    ThreadStrategyType strategy,
    WorkloadEstimator estimator,
    void* estimator_data
);

/**
 * Free thread allocation strategy
 * 
 * @param strategy Strategy to free
 */
void thread_allocation_free(ThreadAllocationStrategy* strategy);

// ============================================================================
// QUERY FUNCTIONS
// ============================================================================

/**
 * Get work groups for a physical thread
 * 
 * @param strategy Allocation strategy
 * @param physical_thread_id Physical thread ID (0 to N-1)
 * @param out_groups Output array for work group IDs (do not free)
 * @param out_count Output count of work groups
 * @return 0 on success, -1 on error
 */
int thread_allocation_get_work_groups(
    const ThreadAllocationStrategy* strategy,
    int physical_thread_id,
    int** out_groups,
    int* out_count
);

/**
 * Get physical thread for a work group
 * 
 * @param strategy Allocation strategy
 * @param work_group Work group ID
 * @return Physical thread ID, or -1 on error
 */
int thread_allocation_get_thread_for_group(
    const ThreadAllocationStrategy* strategy,
    int work_group
);

/**
 * Get workload for a physical thread
 * 
 * @param strategy Allocation strategy
 * @param physical_thread_id Physical thread ID
 * @return Expected workload, or 0.0 on error
 */
double thread_allocation_get_workload(
    const ThreadAllocationStrategy* strategy,
    int physical_thread_id
);

// ============================================================================
// VALIDATION & STATISTICS
// ============================================================================

/**
 * Validate thread allocation
 * 
 * Verifies that:
 * - All M work groups are covered
 * - No work group is assigned to multiple threads
 * - Load is reasonably balanced
 * 
 * @param strategy Strategy to validate
 * @return true if valid, false otherwise
 */
bool thread_allocation_validate(const ThreadAllocationStrategy* strategy);

/**
 * Calculate load balance factor
 * 
 * Returns a value between 0.0 and 1.0 indicating load balance quality.
 * 1.0 = perfect balance, lower values = worse balance.
 * 
 * @param strategy Strategy to analyze
 * @return Load balance factor (0.0 to 1.0)
 */
double thread_allocation_calculate_balance(ThreadAllocationStrategy* strategy);

/**
 * Print thread allocation
 * 
 * Prints the thread allocation strategy for debugging.
 * 
 * @param strategy Strategy to print
 */
void thread_allocation_print(const ThreadAllocationStrategy* strategy);

// ============================================================================
// DYNAMIC REBALANCING
// ============================================================================

/**
 * Update actual workloads
 * 
 * Updates the actual measured workloads for each thread.
 * This information can be used for dynamic rebalancing.
 * 
 * @param strategy Strategy to update
 * @param actual_workloads Array of actual workloads per thread
 * @return 0 on success, -1 on error
 */
int thread_allocation_update_workloads(
    ThreadAllocationStrategy* strategy,
    const double* actual_workloads
);

/**
 * Rebalance thread allocation
 * 
 * Dynamically rebalances the thread allocation based on actual workload.
 * This can be called during execution to adapt to changing conditions.
 * 
 * @param strategy Strategy to rebalance
 * @return 0 on success, -1 on error
 */
int thread_allocation_rebalance(ThreadAllocationStrategy* strategy);

// ============================================================================
// CPU AFFINITY
// ============================================================================

/**
 * Set CPU affinity for thread
 * 
 * Sets CPU affinity for a physical thread based on the allocation strategy.
 * This is platform-specific and may not be supported on all systems.
 * 
 * @param strategy Allocation strategy
 * @param physical_thread_id Physical thread ID
 * @return 0 on success, -1 on error or not supported
 */
int thread_allocation_set_affinity(
    ThreadAllocationStrategy* strategy,
    int physical_thread_id
);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_THREADING_H */
