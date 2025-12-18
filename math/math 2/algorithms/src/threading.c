/**
 * Threading Algorithms Library - Implementation
 * 
 * Generic thread allocation and workload distribution algorithms.
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_log with math_log (3 calls)
 * Total: 3 function calls migrated to NEW math library
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE  // For CPU_SET, CPU_ZERO, pthread_setaffinity_np
#endif

#include "threading.h"
#include "math/transcendental.h"  // PHASE 1: NEW math library
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#ifdef __linux__
#include <sched.h>
#endif
#endif

// ============================================================================
// CPU DETECTION
// ============================================================================

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

int get_optimal_thread_count(int num_work_groups) {
    int num_cores = detect_num_cpu_cores();
    if (num_cores < 1) {
        return 1;  // Fallback to single thread
    }
    
    // Optimal is min(num_cores, num_work_groups) for best performance
    return (num_cores < num_work_groups) ? num_cores : num_work_groups;
}

// ============================================================================
// WORKLOAD ESTIMATION
// ============================================================================

uint64_t default_workload_estimator(int work_group, void* user_data) {
    (void)work_group;    // Unused
    (void)user_data;     // Unused
    return 1000;  // Default: uniform workload
}

uint64_t prime_workload_estimator(int work_group, void* user_data) {
    (void)work_group; // Reserved for future work group-specific estimation
    if (!user_data) {
        return 1000;  // Fallback to default
    }
    
    uint64_t* range = (uint64_t*)user_data;
    uint64_t range_start = range[0];
    uint64_t range_end = range[1];
    
    if (range_start >= range_end) {
        return 0;
    }
    
    // Prime number theorem approximation: π(x) ≈ x / ln(x)
    // Using crystalline library's math_log function
    double pi_end = (double)range_end / math_log((double)range_end);
    double pi_start = (range_start > 1) ? 
        (double)range_start / math_log((double)range_start) : 0;
    
    double total_primes = pi_end - pi_start;
    
    // Assume uniform distribution across work groups
    // (In reality, distribution varies by modulo class)
    // This is a simplification - actual distribution should be measured
    
    return (uint64_t)(total_primes / 12.0);  // Assuming 12 work groups typical
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static double calculate_load_balance_factor(ThreadAllocationStrategy* strategy) {
    if (strategy->num_physical_threads == 0) {
        return 0.0;
    }
    
    double max_load = 0.0;
    double min_load = 1e100;
    double total_load = 0.0;
    
    for (int t = 0; t < strategy->num_physical_threads; t++) {
        double load = strategy->mappings[t].expected_workload;
        if (load > max_load) max_load = load;
        if (load < min_load) min_load = load;
        total_load += load;
    }
    
    strategy->max_workload = max_load;
    strategy->min_workload = min_load;
    strategy->total_workload = total_load;
    
    if (max_load == 0.0) {
        return 1.0;  // No work, perfect balance
    }
    
    // Balance factor = min_load / max_load
    // 1.0 = perfect balance, 0.0 = worst balance
    return min_load / max_load;
}

// ============================================================================
// ALLOCATION STRATEGIES
// ============================================================================

static void create_one_to_one_allocation(
    ThreadAllocationStrategy* strategy,
    WorkloadEstimator estimator,
    void* estimator_data
) {
    int N = strategy->num_physical_threads;
    int M = strategy->num_work_groups;
    
    // Assign first M threads to work groups
    for (int t = 0; t < M && t < N; t++) {
        WorkGroupMapping* mapping = &strategy->mappings[t];
        mapping->physical_thread_id = t;
        mapping->preferred_cpu = t;
        mapping->num_work_groups = 1;
        mapping->work_groups = (int*)malloc(sizeof(int));
        mapping->work_groups[0] = t;
        
        strategy->group_to_thread_map[t] = t;
        
        // Estimate workload
        mapping->estimated_work_units = estimator(t, estimator_data);
        mapping->expected_workload = (double)mapping->estimated_work_units;
        mapping->actual_workload = 0.0;
        mapping->affinity_set = false;
    }
    
    // Extra threads (if N > M) are not assigned work groups
    for (int t = M; t < N; t++) {
        WorkGroupMapping* mapping = &strategy->mappings[t];
        mapping->physical_thread_id = t;
        mapping->preferred_cpu = t;
        mapping->num_work_groups = 0;
        mapping->work_groups = NULL;
        mapping->estimated_work_units = 0;
        mapping->expected_workload = 0.0;
        mapping->actual_workload = 0.0;
        mapping->affinity_set = false;
    }
}

static void create_round_robin_allocation(
    ThreadAllocationStrategy* strategy,
    WorkloadEstimator estimator,
    void* estimator_data
) {
    int N = strategy->num_physical_threads;
    int M = strategy->num_work_groups;
    
    for (int t = 0; t < N; t++) {
        WorkGroupMapping* mapping = &strategy->mappings[t];
        mapping->physical_thread_id = t;
        mapping->preferred_cpu = t;
        
        // Calculate how many groups this thread handles
        int groups_per_thread = M / N;
        int extra_groups = M % N;
        int num_groups = groups_per_thread + (t < extra_groups ? 1 : 0);
        
        mapping->num_work_groups = num_groups;
        mapping->work_groups = (int*)malloc(num_groups * sizeof(int));
        
        // Assign work groups round-robin
        int group_idx = 0;
        for (int g = t; g < M; g += N) {
            mapping->work_groups[group_idx++] = g;
            strategy->group_to_thread_map[g] = t;
        }
        
        // Estimate workload
        mapping->estimated_work_units = 0;
        for (int i = 0; i < num_groups; i++) {
            mapping->estimated_work_units += estimator(mapping->work_groups[i], estimator_data);
        }
        mapping->expected_workload = (double)mapping->estimated_work_units;
        mapping->actual_workload = 0.0;
        mapping->affinity_set = false;
    }
}

static void create_balanced_allocation(
    ThreadAllocationStrategy* strategy,
    WorkloadEstimator estimator,
    void* estimator_data
) {
    int N = strategy->num_physical_threads;
    int M = strategy->num_work_groups;
    
    // First, estimate workload for each work group
    uint64_t* group_workloads = (uint64_t*)malloc(M * sizeof(uint64_t));
    uint64_t total_work = 0;
    
    for (int g = 0; g < M; g++) {
        group_workloads[g] = estimator(g, estimator_data);
        total_work += group_workloads[g];
    }
    
    // Target workload per thread
    double target_per_thread = (double)total_work / N;
    (void)target_per_thread; // Reserved for future load balancing optimization
    
    // Initialize mappings
    for (int t = 0; t < N; t++) {
        WorkGroupMapping* mapping = &strategy->mappings[t];
        mapping->physical_thread_id = t;
        mapping->preferred_cpu = t;
        mapping->num_work_groups = 0;
        mapping->work_groups = (int*)malloc(M * sizeof(int));  // Max possible
        mapping->estimated_work_units = 0;
        mapping->expected_workload = 0.0;
        mapping->actual_workload = 0.0;
        mapping->affinity_set = false;
    }
    
    // Greedy assignment: assign each work group to thread with least load
    for (int g = 0; g < M; g++) {
        // Find thread with minimum current load
        int min_thread = 0;
        double min_load = strategy->mappings[0].expected_workload;
        
        for (int t = 1; t < N; t++) {
            if (strategy->mappings[t].expected_workload < min_load) {
                min_load = strategy->mappings[t].expected_workload;
                min_thread = t;
            }
        }
        
        // Assign work group g to min_thread
        WorkGroupMapping* mapping = &strategy->mappings[min_thread];
        mapping->work_groups[mapping->num_work_groups++] = g;
        mapping->estimated_work_units += group_workloads[g];
        mapping->expected_workload = (double)mapping->estimated_work_units;
        
        strategy->group_to_thread_map[g] = min_thread;
    }
    
    // Resize work_groups arrays to actual size
    for (int t = 0; t < N; t++) {
        WorkGroupMapping* mapping = &strategy->mappings[t];
        if (mapping->num_work_groups > 0) {
            int* resized = (int*)realloc(mapping->work_groups, 
                                        mapping->num_work_groups * sizeof(int));
            if (resized) {
                mapping->work_groups = resized;
            }
        } else {
            free(mapping->work_groups);
            mapping->work_groups = NULL;
        }
    }
    
    free(group_workloads);
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

ThreadAllocationStrategy* thread_allocation_create(
    int num_physical_threads,
    int num_work_groups,
    WorkloadEstimator estimator,
    void* estimator_data
) {
    if (num_physical_threads < 1) {
        fprintf(stderr, "Error: num_physical_threads must be >= 1\n");
        return NULL;
    }
    
    if (num_work_groups < 1) {
        fprintf(stderr, "Error: num_work_groups must be >= 1\n");
        return NULL;
    }
    
    // Use default estimator if none provided
    if (!estimator) {
        estimator = default_workload_estimator;
    }
    
    // Allocate strategy
    ThreadAllocationStrategy* strategy = (ThreadAllocationStrategy*)
        calloc(1, sizeof(ThreadAllocationStrategy));
    if (!strategy) {
        fprintf(stderr, "Error: Failed to allocate ThreadAllocationStrategy\n");
        return NULL;
    }
    
    strategy->num_physical_threads = num_physical_threads;
    strategy->num_work_groups = num_work_groups;
    
    // Allocate mappings
    strategy->mappings = (WorkGroupMapping*)
        calloc(num_physical_threads, sizeof(WorkGroupMapping));
    if (!strategy->mappings) {
        fprintf(stderr, "Error: Failed to allocate mappings\n");
        free(strategy);
        return NULL;
    }
    
    // Allocate group-to-thread map
    strategy->group_to_thread_map = (int*)malloc(num_work_groups * sizeof(int));
    if (!strategy->group_to_thread_map) {
        fprintf(stderr, "Error: Failed to allocate group_to_thread_map\n");
        free(strategy->mappings);
        free(strategy);
        return NULL;
    }
    
    // Initialize group-to-thread map to -1 (unassigned)
    for (int i = 0; i < num_work_groups; i++) {
        strategy->group_to_thread_map[i] = -1;
    }
    
    // Select and create allocation strategy
    if (num_physical_threads >= num_work_groups) {
        strategy->strategy = THREAD_STRATEGY_ONE_TO_ONE;
        create_one_to_one_allocation(strategy, estimator, estimator_data);
    } else {
        strategy->strategy = THREAD_STRATEGY_BALANCED;
        create_balanced_allocation(strategy, estimator, estimator_data);
    }
    
    // Calculate load balance
    strategy->load_balance_factor = calculate_load_balance_factor(strategy);
    
    // Validate
    strategy->all_groups_covered = thread_allocation_validate(strategy);
    
    strategy->rebalance_count = 0;
    strategy->work_units_completed = 0;
    
    return strategy;
}

ThreadAllocationStrategy* thread_allocation_create_with_strategy(
    int num_physical_threads,
    int num_work_groups,
    ThreadStrategyType strategy_type,
    WorkloadEstimator estimator,
    void* estimator_data
) {
    ThreadAllocationStrategy* strategy = thread_allocation_create(
        num_physical_threads, num_work_groups, estimator, estimator_data);
    
    if (!strategy) {
        return NULL;
    }
    
    // If requested strategy is different, recreate with that strategy
    if (strategy->strategy != strategy_type) {
        // Free existing mappings
        for (int t = 0; t < num_physical_threads; t++) {
            if (strategy->mappings[t].work_groups) {
                free(strategy->mappings[t].work_groups);
            }
        }
        
        // Reset group-to-thread map
        for (int i = 0; i < num_work_groups; i++) {
            strategy->group_to_thread_map[i] = -1;
        }
        
        // Use default estimator if none provided
        if (!estimator) {
            estimator = default_workload_estimator;
        }
        
        // Create with requested strategy
        strategy->strategy = strategy_type;
        switch (strategy_type) {
            case THREAD_STRATEGY_ONE_TO_ONE:
                create_one_to_one_allocation(strategy, estimator, estimator_data);
                break;
            case THREAD_STRATEGY_ROUND_ROBIN:
                create_round_robin_allocation(strategy, estimator, estimator_data);
                break;
            case THREAD_STRATEGY_BALANCED:
                create_balanced_allocation(strategy, estimator, estimator_data);
                break;
            default:
                // For other strategies, fall back to balanced
                create_balanced_allocation(strategy, estimator, estimator_data);
                break;
        }
        
        // Recalculate load balance
        strategy->load_balance_factor = calculate_load_balance_factor(strategy);
        strategy->all_groups_covered = thread_allocation_validate(strategy);
    }
    
    return strategy;
}

void thread_allocation_free(ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return;
    }
    
    if (strategy->mappings) {
        for (int t = 0; t < strategy->num_physical_threads; t++) {
            if (strategy->mappings[t].work_groups) {
                free(strategy->mappings[t].work_groups);
            }
        }
        free(strategy->mappings);
    }
    
    if (strategy->group_to_thread_map) {
        free(strategy->group_to_thread_map);
    }
    
    free(strategy);
}

// ============================================================================
// QUERY FUNCTIONS
// ============================================================================

int thread_allocation_get_work_groups(
    const ThreadAllocationStrategy* strategy,
    int physical_thread_id,
    int** out_groups,
    int* out_count
) {
    if (!strategy || !out_groups || !out_count) {
        return -1;
    }
    
    if (physical_thread_id < 0 || physical_thread_id >= strategy->num_physical_threads) {
        return -1;
    }
    
    const WorkGroupMapping* mapping = &strategy->mappings[physical_thread_id];
    *out_groups = mapping->work_groups;
    *out_count = mapping->num_work_groups;
    
    return 0;
}

int thread_allocation_get_thread_for_group(
    const ThreadAllocationStrategy* strategy,
    int work_group
) {
    if (!strategy) {
        return -1;
    }
    
    if (work_group < 0 || work_group >= strategy->num_work_groups) {
        return -1;
    }
    
    return strategy->group_to_thread_map[work_group];
}

double thread_allocation_get_workload(
    const ThreadAllocationStrategy* strategy,
    int physical_thread_id
) {
    if (!strategy) {
        return 0.0;
    }
    
    if (physical_thread_id < 0 || physical_thread_id >= strategy->num_physical_threads) {
        return 0.0;
    }
    
    return strategy->mappings[physical_thread_id].expected_workload;
}

// ============================================================================
// VALIDATION & STATISTICS
// ============================================================================

bool thread_allocation_validate(const ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return false;
    }
    
    // Check that all work groups are covered
    bool* covered = (bool*)calloc(strategy->num_work_groups, sizeof(bool));
    if (!covered) {
        return false;
    }
    
    for (int t = 0; t < strategy->num_physical_threads; t++) {
        const WorkGroupMapping* mapping = &strategy->mappings[t];
        
        for (int i = 0; i < mapping->num_work_groups; i++) {
            int group = mapping->work_groups[i];
            
            if (group < 0 || group >= strategy->num_work_groups) {
                fprintf(stderr, "Error: Invalid work group %d\n", group);
                free(covered);
                return false;
            }
            
            if (covered[group]) {
                fprintf(stderr, "Error: Work group %d assigned to multiple threads\n", group);
                free(covered);
                return false;
            }
            
            covered[group] = true;
        }
    }
    
    // Verify all groups are covered
    for (int g = 0; g < strategy->num_work_groups; g++) {
        if (!covered[g]) {
            fprintf(stderr, "Error: Work group %d not assigned to any thread\n", g);
            free(covered);
            return false;
        }
    }
    
    free(covered);
    return true;
}

double thread_allocation_calculate_balance(ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return 0.0;
    }
    
    return calculate_load_balance_factor(strategy);
}

void thread_allocation_print(const ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return;
    }
    
    printf("\n=== THREAD ALLOCATION STRATEGY ===\n");
    printf("Physical threads: %d\n", strategy->num_physical_threads);
    printf("Work groups: %d\n", strategy->num_work_groups);
    printf("Strategy: ");
    
    switch (strategy->strategy) {
        case THREAD_STRATEGY_ONE_TO_ONE:
            printf("ONE_TO_ONE (1 thread per group)\n");
            break;
        case THREAD_STRATEGY_ROUND_ROBIN:
            printf("ROUND_ROBIN (groups distributed across threads)\n");
            break;
        case THREAD_STRATEGY_BALANCED:
            printf("BALANCED (load-balanced distribution)\n");
            break;
        case THREAD_STRATEGY_PRIORITY_BASED:
            printf("PRIORITY_BASED\n");
            break;
        case THREAD_STRATEGY_DYNAMIC:
            printf("DYNAMIC\n");
            break;
    }
    
    printf("All groups covered: %s\n", strategy->all_groups_covered ? "YES" : "NO");
    printf("Load balance factor: %.3f (1.0 = perfect)\n", strategy->load_balance_factor);
    printf("Max workload: %.0f work units\n", strategy->max_workload);
    printf("Min workload: %.0f work units\n", strategy->min_workload);
    printf("Total workload: %.0f work units\n", strategy->total_workload);
    printf("Rebalance count: %lu\n", (unsigned long)strategy->rebalance_count);
    printf("\n");
    
    for (int t = 0; t < strategy->num_physical_threads; t++) {
        const WorkGroupMapping* mapping = &strategy->mappings[t];
        
        printf("Physical Thread %d (CPU %d):\n", t, mapping->preferred_cpu);
        printf("  Work groups: [");
        for (int i = 0; i < mapping->num_work_groups; i++) {
            printf("%d", mapping->work_groups[i]);
            if (i < mapping->num_work_groups - 1) {
                printf(", ");
            }
        }
        printf("]\n");
        printf("  Expected workload: %.0f work units\n", mapping->expected_workload);
        printf("  Actual workload: %.0f work units\n", mapping->actual_workload);
        printf("\n");
    }
    
    printf("=== END THREAD ALLOCATION ===\n\n");
}

// ============================================================================
// DYNAMIC REBALANCING
// ============================================================================

int thread_allocation_update_workloads(
    ThreadAllocationStrategy* strategy,
    const double* actual_workloads
) {
    if (!strategy || !actual_workloads) {
        return -1;
    }
    
    for (int t = 0; t < strategy->num_physical_threads; t++) {
        strategy->mappings[t].actual_workload = actual_workloads[t];
    }
    
    return 0;
}

int thread_allocation_rebalance(ThreadAllocationStrategy* strategy) {
    if (!strategy) {
        return -1;
    }
    
    // TODO: Implement dynamic rebalancing based on actual workloads
    // For now, just recalculate load balance factor
    strategy->load_balance_factor = calculate_load_balance_factor(strategy);
    strategy->rebalance_count++;
    
    return 0;
}

// ============================================================================
// CPU AFFINITY
// ============================================================================

int thread_allocation_set_affinity(
    ThreadAllocationStrategy* strategy,
    int physical_thread_id
) {
    if (!strategy) {
        return -1;
    }
    
    if (physical_thread_id < 0 || physical_thread_id >= strategy->num_physical_threads) {
        return -1;
    }
    
    WorkGroupMapping* mapping = &strategy->mappings[physical_thread_id];
    
#ifdef __linux__
    // Linux: use pthread_setaffinity_np
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(mapping->preferred_cpu, &cpuset);
    
    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (result == 0) {
        mapping->affinity_set = true;
        return 0;
    }
    return -1;
#elif defined(_WIN32)
    // Windows: use SetThreadAffinityMask
    DWORD_PTR mask = 1ULL << mapping->preferred_cpu;
    if (SetThreadAffinityMask(GetCurrentThread(), mask) != 0) {
        mapping->affinity_set = true;
        return 0;
    }
    return -1;
#else
    // Not supported on this platform
    return -1;
#endif
}
