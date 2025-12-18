/**
 * Cache Optimization Implementation
 * 
 * Uses angular position θ to optimize cache placement and CPU affinity.
 */

#define _GNU_SOURCE  // For CPU_SET macros
#include "ai/cllm_cache_optimization.h"
#include "math/types.h"  // For MATH_PI (NEW math library)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include "math/transcendental.h"

// ============================================================================
// CACHE MAPPING
// ============================================================================

/**
 * Map angular position θ to cache line
 * 
 * Uses θ to distribute threads across cache lines.
 * Threads with similar θ get nearby cache lines.
 */
uint32_t map_theta_to_cache_line(double theta) {
    // Normalize theta to [0, 2π]
    double normalized_theta = theta;
    while (normalized_theta < 0.0) {
        normalized_theta += 2.0 * MATH_PI;
    }
    while (normalized_theta >= 2.0 * MATH_PI) {
        normalized_theta -= 2.0 * MATH_PI;
    }
    
    // Map to cache line [0, NUM_CACHE_LINES-1]
    // Use linear mapping for simplicity
    uint32_t cache_line = (uint32_t)((normalized_theta / (2.0 * MATH_PI)) * NUM_CACHE_LINES);
    
    // Ensure within bounds
    if (cache_line >= NUM_CACHE_LINES) {
        cache_line = NUM_CACHE_LINES - 1;
    }
    
    return cache_line;
}

/**
 * Map angular position θ to NUMA node
 * 
 * Distributes threads evenly across NUMA nodes based on θ.
 */
uint32_t map_theta_to_numa_node(double theta, int num_numa_nodes) {
    if (num_numa_nodes <= 1) {
        return 0;
    }
    
    // Normalize theta to [0, 2π]
    double normalized_theta = theta;
    while (normalized_theta < 0.0) {
        normalized_theta += 2.0 * MATH_PI;
    }
    while (normalized_theta >= 2.0 * MATH_PI) {
        normalized_theta -= 2.0 * MATH_PI;
    }
    
    // Map to NUMA node
    uint32_t numa_node = (uint32_t)((normalized_theta / (2.0 * MATH_PI)) * num_numa_nodes);
    
    // Ensure within bounds
    if (numa_node >= (uint32_t)num_numa_nodes) {
        numa_node = num_numa_nodes - 1;
    }
    
    return numa_node;
}

/**
 * Calculate cache proximity between two threads
 * 
 * Returns angular distance normalized to [0, 1].
 */
double calculate_cache_proximity(double theta1, double theta2) {
    // Calculate angular distance
    double diff = math_abs(theta1 - theta2);
    
    // Normalize to [0, π] (shortest angular distance)
    if (diff > MATH_PI) {
        diff = 2.0 * MATH_PI - diff;
    }
    
    // Normalize to [0, 1]
    double proximity = diff / MATH_PI;
    
    return proximity;
}

// ============================================================================
// CPU AFFINITY
// ============================================================================

/**
 * Get number of available CPU cores
 * 
 * Note: This function is defined in cllm_threads.c
 * We declare it here for use in this module.
 */
extern int get_num_cpu_cores(void);

/**
 * Get number of NUMA nodes
 * 
 * Note: This is a simplified implementation.
 * For production, use libnuma for proper NUMA detection.
 */
int get_num_numa_nodes(void) {
    // Simple heuristic: assume 1 NUMA node per 8 cores
    int num_cores = get_num_cpu_cores();
    int num_nodes = (num_cores + 7) / 8;
    
    if (num_nodes > MAX_NUMA_NODES) {
        num_nodes = MAX_NUMA_NODES;
    }
    if (num_nodes < 1) {
        num_nodes = 1;
    }
    
    return num_nodes;
}

/**
 * Create CPU affinity mask for thread
 */
CPUAffinityMask create_cpu_affinity_mask(uint32_t cache_line, uint32_t numa_node) {
    CPUAffinityMask mask;
    CPU_ZERO(&mask.cpu_set);
    
    int num_cores = get_num_cpu_cores();
    int num_numa_nodes = get_num_numa_nodes();
    
    // Calculate cores per NUMA node
    int cores_per_node = num_cores / num_numa_nodes;
    if (cores_per_node < 1) {
        cores_per_node = 1;
    }
    
    // Calculate preferred CPU based on cache line and NUMA node
    int node_start = numa_node * cores_per_node;
    int node_end = node_start + cores_per_node;
    if (node_end > num_cores) {
        node_end = num_cores;
    }
    
    // Map cache line to CPU within NUMA node
    int cpu_offset = cache_line % cores_per_node;
    mask.preferred_cpu = node_start + cpu_offset;
    
    // Ensure within bounds
    if (mask.preferred_cpu >= num_cores) {
        mask.preferred_cpu = num_cores - 1;
    }
    
    // Set affinity to preferred CPU and nearby CPUs
    // This allows some flexibility while maintaining locality
    for (int cpu = node_start; cpu < node_end; cpu++) {
        CPU_SET(cpu, &mask.cpu_set);
    }
    
    mask.num_cpus = node_end - node_start;
    
    return mask;
}

/**
 * Set CPU affinity for thread
 */
int set_thread_cpu_affinity(pthread_t thread, const CPUAffinityMask* mask) {
    if (!mask) {
        return -1;
    }
    
    int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &mask->cpu_set);
    if (result != 0) {
        fprintf(stderr, "Warning: Failed to set CPU affinity (error %d)\n", result);
        return -1;
    }
    
    return 0;
}

// ============================================================================
// CACHE PLACEMENT
// ============================================================================

/**
 * Calculate optimal cache placement for thread
 */
CachePlacement calculate_cache_placement(double theta, int thread_id) {
    CachePlacement placement;
    
    // Store theta
    placement.theta = theta;
    
    // Map to cache line
    placement.cache_line = map_theta_to_cache_line(theta);
    
    // Map to NUMA node
    int num_numa_nodes = get_num_numa_nodes();
    placement.numa_node = map_theta_to_numa_node(theta, num_numa_nodes);
    
    // Calculate preferred CPU core
    int num_cores = get_num_cpu_cores();
    int cores_per_node = num_cores / num_numa_nodes;
    if (cores_per_node < 1) {
        cores_per_node = 1;
    }
    
    int node_start = placement.numa_node * cores_per_node;
    int cpu_offset = (thread_id % cores_per_node);
    placement.cpu_core = node_start + cpu_offset;
    
    // Ensure within bounds
    if (placement.cpu_core >= num_cores) {
        placement.cpu_core = num_cores - 1;
    }
    
    // Initialize statistics
    placement.cache_hits = 0;
    placement.cache_misses = 0;
    
    return placement;
}

/**
 * Position thread for optimal cache usage
 */
int position_thread_for_cache(pthread_t thread, const CachePlacement* placement) {
    if (!placement) {
        return -1;
    }
    
    // Create CPU affinity mask
    CPUAffinityMask mask = create_cpu_affinity_mask(
        placement->cache_line,
        placement->numa_node
    );
    
    // Set CPU affinity
    int result = set_thread_cpu_affinity(thread, &mask);
    
    return result;
}

/**
 * Update cache statistics
 */
void update_cache_stats(CachePlacement* placement, int hit) {
    if (!placement) {
        return;
    }
    
    if (hit) {
        placement->cache_hits++;
    } else {
        placement->cache_misses++;
    }
}

/**
 * Get cache hit rate
 */
double get_cache_hit_rate(const CachePlacement* placement) {
    if (!placement) {
        return 0.0;
    }
    
    uint64_t total = placement->cache_hits + placement->cache_misses;
    if (total == 0) {
        return 0.0;
    }
    
    return (double)placement->cache_hits / (double)total;
}

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Print cache placement information
 */
void print_cache_placement(const CachePlacement* placement) {
    if (!placement) {
        printf("NULL cache placement\n");
        return;
    }
    
    printf("Cache Placement:\n");
    printf("  Theta: %.6f radians\n", placement->theta);
    printf("  Cache Line: %u / %d\n", placement->cache_line, NUM_CACHE_LINES);
    printf("  NUMA Node: %u\n", placement->numa_node);
    printf("  CPU Core: %d\n", placement->cpu_core);
    printf("  Cache Hits: %lu\n", placement->cache_hits);
    printf("  Cache Misses: %lu\n", placement->cache_misses);
    printf("  Hit Rate: %.2f%%\n", get_cache_hit_rate(placement) * 100.0);
}

/**
 * Validate cache placement
 */
int validate_cache_placement(const CachePlacement* placement) {
    if (!placement) {
        return 0;
    }
    
    // Check cache line bounds
    if (placement->cache_line >= NUM_CACHE_LINES) {
        return 0;
    }
    
    // Check NUMA node bounds
    if (placement->numa_node >= MAX_NUMA_NODES) {
        return 0;
    }
    
    // Check CPU core bounds
    int num_cores = get_num_cpu_cores();
    if (placement->cpu_core >= num_cores || placement->cpu_core < 0) {
        return 0;
    }
    
    // Check theta bounds
    if (placement->theta < 0.0 || placement->theta > 2.0 * MATH_PI) {
        return 0;
    }
    
    return 1;
}
