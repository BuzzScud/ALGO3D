#ifndef CLLM_CACHE_OPTIMIZATION_H
#define CLLM_CACHE_OPTIMIZATION_H

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Cache Optimization System
 * 
 * Uses angular position θ to optimize cache placement and CPU affinity.
 * Threads with similar θ values share cache lines for better locality.
 */

// Number of cache lines to track (typical L3 cache has ~1000s of lines)
#define NUM_CACHE_LINES 1024

// NUMA node information
#define MAX_NUMA_NODES 8

/**
 * Cache placement information
 */
typedef struct {
    uint32_t cache_line;           // Preferred cache line (0-1023)
    uint32_t numa_node;            // NUMA node (0-7)
    int cpu_core;                  // Preferred CPU core
    double theta;                  // Angular position θ
    uint64_t cache_hits;           // Cache hit count
    uint64_t cache_misses;         // Cache miss count
} CachePlacement;

/**
 * CPU affinity mask
 */
typedef struct {
    cpu_set_t cpu_set;             // CPU set for affinity
    int num_cpus;                  // Number of CPUs in set
    int preferred_cpu;             // Preferred CPU core
} CPUAffinityMask;

// ============================================================================
// CACHE MAPPING
// ============================================================================

/**
 * Map angular position θ to cache line
 * 
 * Threads with similar θ values get nearby cache lines.
 * 
 * @param theta Angular position (radians, 0 to 2π)
 * @return Cache line number (0 to NUM_CACHE_LINES-1)
 */
uint32_t map_theta_to_cache_line(double theta);

/**
 * Map angular position θ to NUMA node
 * 
 * Distributes threads across NUMA nodes based on θ.
 * 
 * @param theta Angular position (radians)
 * @param num_numa_nodes Number of NUMA nodes available
 * @return NUMA node number (0 to num_numa_nodes-1)
 */
uint32_t map_theta_to_numa_node(double theta, int num_numa_nodes);

/**
 * Calculate cache proximity between two threads
 * 
 * Returns a value indicating how close two threads are in cache space.
 * Lower values mean closer proximity (better cache sharing).
 * 
 * @param theta1 First thread's angular position
 * @param theta2 Second thread's angular position
 * @return Proximity value (0.0 = same cache line, 1.0 = opposite)
 */
double calculate_cache_proximity(double theta1, double theta2);

// ============================================================================
// CPU AFFINITY
// ============================================================================

/**
 * Create CPU affinity mask for thread
 * 
 * @param cache_line Preferred cache line
 * @param numa_node NUMA node
 * @return CPU affinity mask
 */
CPUAffinityMask create_cpu_affinity_mask(uint32_t cache_line, uint32_t numa_node);

/**
 * Set CPU affinity for thread
 * 
 * @param thread Thread handle
 * @param mask CPU affinity mask
 * @return 0 on success, -1 on error
 */
int set_thread_cpu_affinity(pthread_t thread, const CPUAffinityMask* mask);

/**
 * Get number of available CPU cores
 * 
 * @return Number of CPU cores
 */
int get_num_cpu_cores(void);

/**
 * Get number of NUMA nodes
 * 
 * @return Number of NUMA nodes (1 if NUMA not available)
 */
int get_num_numa_nodes(void);

// ============================================================================
// CACHE PLACEMENT
// ============================================================================

/**
 * Calculate optimal cache placement for thread
 * 
 * Uses angular position θ to determine cache line, NUMA node, and CPU core.
 * 
 * @param theta Angular position (radians)
 * @param thread_id Thread ID
 * @return Cache placement information
 */
CachePlacement calculate_cache_placement(double theta, int thread_id);

/**
 * Position thread for optimal cache usage
 * 
 * Sets CPU affinity and tracks cache placement.
 * 
 * @param thread Thread handle
 * @param placement Cache placement information
 * @return 0 on success, -1 on error
 */
int position_thread_for_cache(pthread_t thread, const CachePlacement* placement);

/**
 * Update cache statistics
 * 
 * @param placement Cache placement to update
 * @param hit 1 if cache hit, 0 if cache miss
 */
void update_cache_stats(CachePlacement* placement, int hit);

/**
 * Get cache hit rate
 * 
 * @param placement Cache placement
 * @return Hit rate (0.0 to 1.0)
 */
double get_cache_hit_rate(const CachePlacement* placement);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Print cache placement information
 * 
 * @param placement Cache placement
 */
void print_cache_placement(const CachePlacement* placement);

/**
 * Validate cache placement
 * 
 * @param placement Cache placement
 * @return 1 if valid, 0 if invalid
 */
int validate_cache_placement(const CachePlacement* placement);

#ifdef __cplusplus
}
#endif

#endif // CLLM_CACHE_OPTIMIZATION_H