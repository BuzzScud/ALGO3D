/**
 * Thread Allocation System - Implementation
 * 
 * Maps N physical CPU cores to 12 logical symmetry groups while
 * preserving the mathematical integrity of the 12-fold symmetry.
 */

#include "ai/cllm_thread_allocation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "math/transcendental.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/**
 * Detect number of CPU cores
 */
int detect_num_cpu_cores(void) {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) {
        return -1;
    }
    return (int)nprocs;
#endif
}

/**
 * Get optimal thread count
 */
int get_optimal_thread_count(void) {
    int num_cores = detect_num_cpu_cores();
    if (num_cores < 1) {
        return 1;  // Fallback to single thread
    }
    
    // Optimal is min(num_cores, 12) for best performance
    return (num_cores < NUM_SYMMETRY_GROUPS) ? num_cores : NUM_SYMMETRY_GROUPS;
}

/**
 * Estimate workload for symmetry group using prime number theorem
 * 
 * π(x) ≈ x / ln(x)
 * Primes in [a, b] ≈ π(b) - π(a)
 * Primes ≡ k (mod 12) ≈ (π(b) - π(a)) / φ(12) where φ(12) = 4
 * 
 * Note: This is an approximation. Actual distribution varies.
 */
uint64_t estimate_symmetry_group_workload(int symmetry_group,
                                          uint64_t range_start,
                                          uint64_t range_end) {
    (void)symmetry_group; // Reserved for future symmetry-specific workload estimation
    if (range_start >= range_end) {
        return 0;
    }
    
    // Prime number theorem approximation
    double pi_end = (double)range_end / math_log((double)range_end);
    double pi_start = (range_start > 1) ? (double)range_start / math_log((double)range_start) : 0;
    
    double total_primes = pi_end - pi_start;
    
    // Distribute among 12 symmetry groups
    // Not all groups have equal density (e.g., even numbers excluded)
    // But for estimation, we use uniform distribution
    double primes_per_group = total_primes / NUM_SYMMETRY_GROUPS;
    
    return (uint64_t)primes_per_group;
}

/**
 * Create round-robin allocation
 * 
 * Distributes 12 symmetry groups across N threads in round-robin fashion.
 * Example for N=4:
 *   Thread 0: groups [0, 4, 8]
 *   Thread 1: groups [1, 5, 9]
 *   Thread 2: groups [2, 6, 10]
 *   Thread 3: groups [3, 7, 11]
 */
static void create_round_robin_allocation(ThreadAllocationStrategy* strategy) {
    int N = strategy->num_physical_cores;
    
    for (int t = 0; t < N; t++) {
        SymmetryGroupMapping* mapping = &strategy->mappings[t];
        mapping->physical_thread_id = t;
        mapping->preferred_cpu = t;
        
        // Calculate how many groups this thread handles
        int groups_per_thread = NUM_SYMMETRY_GROUPS / N;
        int extra_groups = NUM_SYMMETRY_GROUPS % N;
        int num_groups = groups_per_thread + (t < extra_groups ? 1 : 0);
        
        mapping->num_symmetry_groups = num_groups;
        mapping->symmetry_groups = (int*)malloc(num_groups * sizeof(int));
        
        // Assign symmetry groups round-robin
        int group_idx = 0;
        for (int g = t; g < NUM_SYMMETRY_GROUPS; g += N) {
            mapping->symmetry_groups[group_idx++] = g;
            strategy->group_to_thread_map[g] = t;
        }
        
        // Estimate workload (using default range for now)
        mapping->primes_in_groups = 0;
        for (int i = 0; i < num_groups; i++) {
            mapping->primes_in_groups += estimate_symmetry_group_workload(
                mapping->symmetry_groups[i], 2, 1000000);
        }
        mapping->expected_workload = (double)mapping->primes_in_groups;
    }
}

/**
 * Create one-to-one allocation
 * 
 * When N >= 12, assign one thread per symmetry group.
 * Extra threads (if N > 12) can be used for other purposes or left idle.
 */
static void create_one_to_one_allocation(ThreadAllocationStrategy* strategy) {
    int N = strategy->num_physical_cores;
    
    // Assign first 12 threads to symmetry groups
    for (int t = 0; t < NUM_SYMMETRY_GROUPS; t++) {
        SymmetryGroupMapping* mapping = &strategy->mappings[t];
        mapping->physical_thread_id = t;
        mapping->preferred_cpu = t;
        mapping->num_symmetry_groups = 1;
        mapping->symmetry_groups = (int*)malloc(sizeof(int));
        mapping->symmetry_groups[0] = t;
        
        strategy->group_to_thread_map[t] = t;
        
        // Estimate workload
        mapping->primes_in_groups = estimate_symmetry_group_workload(t, 2, 1000000);
        mapping->expected_workload = (double)mapping->primes_in_groups;
    }
    
    // Extra threads (if N > 12) are not assigned symmetry groups
    for (int t = NUM_SYMMETRY_GROUPS; t < N; t++) {
        SymmetryGroupMapping* mapping = &strategy->mappings[t];
        mapping->physical_thread_id = t;
        mapping->preferred_cpu = t;
        mapping->num_symmetry_groups = 0;
        mapping->symmetry_groups = NULL;
        mapping->primes_in_groups = 0;
        mapping->expected_workload = 0.0;
    }
}

/**
 * Calculate load balance factor
 * 
 * Returns a value between 0.0 and 1.0 indicating load balance quality.
 * 1.0 = perfect balance, lower values = worse balance.
 */
static double calculate_load_balance_factor(ThreadAllocationStrategy* strategy) {
    if (strategy->num_physical_cores == 0) {
        return 0.0;
    }
    
    double max_load = 0.0;
    double min_load = 1e100;
    double total_load = 0.0;
    
    for (int t = 0; t < strategy->num_physical_cores; t++) {
        double load = strategy->mappings[t].expected_workload;
        if (load > max_load) max_load = load;
        if (load < min_load) min_load = load;
        total_load += load;
    }
    
    strategy->max_workload = max_load;
    strategy->min_workload = min_load;
    
    if (max_load == 0.0) {
        return 1.0;  // No work, perfect balance
    }
    
    // Balance factor = min_load / max_load
    // 1.0 = perfect balance, 0.0 = worst balance
    return min_load / max_load;
}

/**
 * Create thread allocation strategy
 */
ThreadAllocationStrategy* create_thread_allocation(int num_physical_cores) {
    if (num_physical_cores < 1) {
        fprintf(stderr, "Error: num_physical_cores must be >= 1\n");
        return NULL;
    }
    
    ThreadAllocationStrategy* strategy = 
        (ThreadAllocationStrategy*)malloc(sizeof(ThreadAllocationStrategy));
    
    if (!strategy) {
        fprintf(stderr, "Error: Failed to allocate ThreadAllocationStrategy\n");
        return NULL;
    }
    
    strategy->num_physical_cores = num_physical_cores;
    strategy->num_symmetry_groups = NUM_SYMMETRY_GROUPS;
    strategy->all_groups_covered = false;
    
    // Allocate mappings
    strategy->mappings = (SymmetryGroupMapping*)calloc(
        num_physical_cores, sizeof(SymmetryGroupMapping));
    
    if (!strategy->mappings) {
        fprintf(stderr, "Error: Failed to allocate mappings\n");
        free(strategy);
        return NULL;
    }
    
    // Allocate group-to-thread map
    strategy->group_to_thread_map = (int*)malloc(NUM_SYMMETRY_GROUPS * sizeof(int));
    if (!strategy->group_to_thread_map) {
        fprintf(stderr, "Error: Failed to allocate group_to_thread_map\n");
        free(strategy->mappings);
        free(strategy);
        return NULL;
    }
    
    // Initialize group-to-thread map to -1 (unassigned)
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        strategy->group_to_thread_map[i] = -1;
    }
    
    // Select and create allocation strategy
    if (num_physical_cores >= NUM_SYMMETRY_GROUPS) {
        strategy->strategy = STRATEGY_ONE_TO_ONE;
        create_one_to_one_allocation(strategy);
    } else {
        strategy->strategy = STRATEGY_ROUND_ROBIN;
        create_round_robin_allocation(strategy);
    }
    
    // Calculate load balance
    strategy->load_balance_factor = calculate_load_balance_factor(strategy);
    
    // Validate
    strategy->all_groups_covered = validate_thread_allocation(strategy);
    
    return strategy;
}

/**
 * Free thread allocation strategy
 */
void free_thread_allocation(ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return;
    }
    
    if (strategy->mappings) {
        for (int t = 0; t < strategy->num_physical_cores; t++) {
            if (strategy->mappings[t].symmetry_groups) {
                free(strategy->mappings[t].symmetry_groups);
            }
        }
        free(strategy->mappings);
    }
    
    if (strategy->group_to_thread_map) {
        free(strategy->group_to_thread_map);
    }
    
    free(strategy);
}

/**
 * Get symmetry groups for a physical thread
 */
int get_symmetry_groups_for_thread(const ThreadAllocationStrategy* strategy,
                                   int physical_thread_id,
                                   int** out_groups,
                                   int* out_count) {
    if (!strategy || !out_groups || !out_count) {
        return -1;
    }
    
    if (physical_thread_id < 0 || physical_thread_id >= strategy->num_physical_cores) {
        return -1;
    }
    
    const SymmetryGroupMapping* mapping = &strategy->mappings[physical_thread_id];
    *out_groups = mapping->symmetry_groups;
    *out_count = mapping->num_symmetry_groups;
    
    return 0;
}

/**
 * Get physical thread for a symmetry group
 */
int get_thread_for_symmetry_group(const ThreadAllocationStrategy* strategy,
                                  int symmetry_group) {
    if (!strategy) {
        return -1;
    }
    
    if (symmetry_group < 0 || symmetry_group >= NUM_SYMMETRY_GROUPS) {
        return -1;
    }
    
    return strategy->group_to_thread_map[symmetry_group];
}

/**
 * Validate thread allocation
 */
bool validate_thread_allocation(const ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return false;
    }
    
    // Check that all 12 symmetry groups are covered
    bool covered[NUM_SYMMETRY_GROUPS] = {false};
    
    for (int t = 0; t < strategy->num_physical_cores; t++) {
        const SymmetryGroupMapping* mapping = &strategy->mappings[t];
        
        for (int i = 0; i < mapping->num_symmetry_groups; i++) {
            int group = mapping->symmetry_groups[i];
            
            if (group < 0 || group >= NUM_SYMMETRY_GROUPS) {
                fprintf(stderr, "Error: Invalid symmetry group %d\n", group);
                return false;
            }
            
            if (covered[group]) {
                fprintf(stderr, "Error: Symmetry group %d assigned to multiple threads\n", group);
                return false;
            }
            
            covered[group] = true;
        }
    }
    
    // Verify all groups are covered
    for (int g = 0; g < NUM_SYMMETRY_GROUPS; g++) {
        if (!covered[g]) {
            fprintf(stderr, "Error: Symmetry group %d not assigned to any thread\n", g);
            return false;
        }
    }
    
    return true;
}

/**
 * Print thread allocation
 */
void print_thread_allocation(const ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return;
    }
    
    printf("\n=== THREAD ALLOCATION STRATEGY ===\n");
    printf("Physical cores: %d\n", strategy->num_physical_cores);
    printf("Symmetry groups: %d (IMMUTABLE)\n", strategy->num_symmetry_groups);
    printf("Strategy: ");
    
    switch (strategy->strategy) {
        case STRATEGY_ONE_TO_ONE:
            printf("ONE_TO_ONE (1 thread per group)\n");
            break;
        case STRATEGY_ROUND_ROBIN:
            printf("ROUND_ROBIN (groups distributed across threads)\n");
            break;
        case STRATEGY_GROUPED:
            printf("GROUPED\n");
            break;
        case STRATEGY_PRIORITY_BASED:
            printf("PRIORITY_BASED\n");
            break;
        case STRATEGY_DYNAMIC_LOAD_BALANCED:
            printf("DYNAMIC_LOAD_BALANCED\n");
            break;
    }
    
    printf("All groups covered: %s\n", strategy->all_groups_covered ? "YES" : "NO");
    printf("Load balance factor: %.3f (1.0 = perfect)\n", strategy->load_balance_factor);
    printf("Max workload: %.0f primes\n", strategy->max_workload);
    printf("Min workload: %.0f primes\n", strategy->min_workload);
    printf("\n");
    
    for (int t = 0; t < strategy->num_physical_cores; t++) {
        const SymmetryGroupMapping* mapping = &strategy->mappings[t];
        
        printf("Physical Thread %d (CPU %d):\n", t, mapping->preferred_cpu);
        printf("  Symmetry groups: [");
        for (int i = 0; i < mapping->num_symmetry_groups; i++) {
            printf("%d", mapping->symmetry_groups[i]);
            if (i < mapping->num_symmetry_groups - 1) {
                printf(", ");
            }
        }
        printf("]\n");
        printf("  Expected workload: %.0f primes\n", mapping->expected_workload);
        printf("\n");
    }
    
    printf("=== END THREAD ALLOCATION ===\n\n");
}

/**
 * Rebalance thread allocation
 * 
 * TODO: Implement dynamic rebalancing based on actual workloads
 */
int rebalance_thread_allocation(ThreadAllocationStrategy* strategy,
                                const double* actual_workloads) {
    if (!strategy || !actual_workloads) {
        return -1;
    }
    
    // TODO: Implement dynamic rebalancing
    // For now, just recalculate load balance factor
    strategy->load_balance_factor = calculate_load_balance_factor(strategy);
    
    return 0;
}
