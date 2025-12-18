#ifndef CLLM_THREAD_ALLOCATION_H
#define CLLM_THREAD_ALLOCATION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CRITICAL: Mathematical Integrity
 * 
 * The Crystalline Lattice system REQUIRES 12-fold symmetry based on:
 * - 12 symmetry groups (0-11) for prime distribution (p mod 12)
 * - 12 hours on clock face
 * - 12 months in year
 * - 12 zodiac signs
 * - 12³ = 1728 (approaching 144000 = 3 × 12³ × 250/9)
 * 
 * This is MATHEMATICAL LAW, not implementation choice.
 * 
 * However, physical hardware may have fewer than 12 cores.
 * This module maps N physical cores to 12 logical symmetry groups
 * while preserving mathematical integrity.
 */

// Number of symmetry groups (IMMUTABLE - mathematical constant)
#define NUM_SYMMETRY_GROUPS 12

// Thread allocation strategies
typedef enum {
    STRATEGY_ONE_TO_ONE,           // N >= 12: one thread per symmetry group
    STRATEGY_ROUND_ROBIN,          // N < 12: round-robin assignment
    STRATEGY_GROUPED,              // N < 12: grouped assignment
    STRATEGY_PRIORITY_BASED,       // N < 12: priority-based assignment
    STRATEGY_DYNAMIC_LOAD_BALANCED // Dynamic load balancing
} ThreadMappingStrategy;

/**
 * Symmetry Group Mapping
 * 
 * Maps a physical thread to one or more symmetry groups (0-11).
 * When N < 12, each physical thread handles multiple symmetry groups.
 */
typedef struct {
    int physical_thread_id;        // 0 to (num_physical_cores - 1)
    int* symmetry_groups;          // Array of symmetry groups (0-11)
    int num_symmetry_groups;       // How many groups this thread handles
    
    // Load balancing information
    uint64_t primes_in_groups;     // Total primes across all groups
    double expected_workload;      // Expected computational load (0.0 to 1.0)
    
    // CPU affinity
    int preferred_cpu;             // Preferred CPU core for this thread
    
} SymmetryGroupMapping;

/**
 * Thread Allocation Strategy
 * 
 * Manages the mapping of N physical cores to 12 logical symmetry groups.
 * Ensures all 12 groups are covered while adapting to hardware constraints.
 */
typedef struct {
    // Hardware configuration
    int num_physical_cores;        // Actual hardware cores available
    int num_symmetry_groups;       // Always 12 (mathematical requirement)
    
    // Mapping
    SymmetryGroupMapping* mappings; // Array of num_physical_cores mappings
    
    // Strategy
    ThreadMappingStrategy strategy;
    
    // Validation
    bool all_groups_covered;       // True if all 12 groups are assigned
    int* group_to_thread_map;      // Map from symmetry group to physical thread
    
    // Statistics
    double load_balance_factor;    // Measure of load balance (1.0 = perfect)
    double max_workload;           // Maximum workload on any thread
    double min_workload;           // Minimum workload on any thread
    
} ThreadAllocationStrategy;

/**
 * Create thread allocation strategy
 * 
 * Creates a mapping of N physical cores to 12 symmetry groups.
 * Automatically selects the best strategy based on num_physical_cores.
 * 
 * @param num_physical_cores Number of physical CPU cores available
 * @return Allocated strategy, or NULL on error
 */
ThreadAllocationStrategy* create_thread_allocation(int num_physical_cores);

/**
 * Free thread allocation strategy
 * 
 * @param strategy Strategy to free
 */
void free_thread_allocation(ThreadAllocationStrategy* strategy);

/**
 * Get symmetry groups for a physical thread
 * 
 * @param strategy Allocation strategy
 * @param physical_thread_id Physical thread ID (0 to N-1)
 * @param out_groups Output array for symmetry groups
 * @param out_count Output count of symmetry groups
 * @return 0 on success, -1 on error
 */
int get_symmetry_groups_for_thread(const ThreadAllocationStrategy* strategy,
                                   int physical_thread_id,
                                   int** out_groups,
                                   int* out_count);

/**
 * Get physical thread for a symmetry group
 * 
 * @param strategy Allocation strategy
 * @param symmetry_group Symmetry group (0-11)
 * @return Physical thread ID, or -1 on error
 */
int get_thread_for_symmetry_group(const ThreadAllocationStrategy* strategy,
                                  int symmetry_group);

/**
 * Validate thread allocation
 * 
 * Verifies that:
 * - All 12 symmetry groups are covered
 * - No symmetry group is assigned to multiple threads
 * - Load is reasonably balanced
 * 
 * @param strategy Strategy to validate
 * @return true if valid, false otherwise
 */
bool validate_thread_allocation(const ThreadAllocationStrategy* strategy);

/**
 * Print thread allocation
 * 
 * Prints the thread allocation strategy for debugging.
 * 
 * @param strategy Strategy to print
 */
void print_thread_allocation(const ThreadAllocationStrategy* strategy);

/**
 * Estimate workload for symmetry group
 * 
 * Estimates the computational workload for a symmetry group based on
 * prime distribution in the range [range_start, range_end].
 * 
 * @param symmetry_group Symmetry group (0-11)
 * @param range_start Start of prime range
 * @param range_end End of prime range
 * @return Estimated workload (number of primes)
 */
uint64_t estimate_symmetry_group_workload(int symmetry_group,
                                          uint64_t range_start,
                                          uint64_t range_end);

/**
 * Rebalance thread allocation
 * 
 * Dynamically rebalances the thread allocation based on actual workload.
 * This can be called during training to adapt to changing conditions.
 * 
 * @param strategy Strategy to rebalance
 * @param actual_workloads Array of actual workloads per thread
 * @return 0 on success, -1 on error
 */
int rebalance_thread_allocation(ThreadAllocationStrategy* strategy,
                                const double* actual_workloads);

/**
 * Get optimal thread count
 * 
 * Returns the optimal number of threads for the current hardware.
 * This is min(num_physical_cores, 12) for best performance.
 * 
 * @return Optimal thread count
 */
int get_optimal_thread_count(void);

/**
 * Detect number of CPU cores
 * 
 * Detects the number of physical CPU cores available on the system.
 * 
 * @return Number of CPU cores, or -1 on error
 */
int detect_num_cpu_cores(void);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_THREAD_ALLOCATION_H */
