/**
 * Cache-Aware Work Distribution
 * 
 * Implements NUMA-aware and cache-optimized work distribution for
 * the crystalline CLLM system. Optimizes work placement based on:
 * - NUMA node topology
 * - Cache hierarchy (L1, L2, L3)
 * - Memory bandwidth
 * - Work stealing for load balancing
 * 
 * Phase 4, Day 14 Implementation
 */

#ifndef CLLM_CACHE_AWARE_DISTRIBUTION_H
#define CLLM_CACHE_AWARE_DISTRIBUTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define MAX_NUMA_NODES 8
#define MAX_CACHE_LEVELS 3
#define CACHE_LINE_SIZE 64

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * NUMA Node Information
 */
typedef struct {
    uint32_t node_id;
    uint32_t num_cpus;
    uint64_t total_memory;
    uint64_t free_memory;
    bool is_available;
} NumaNodeInfo;

/**
 * Cache Level Information
 */
typedef struct {
    uint32_t level;              // 1, 2, or 3
    uint64_t size;               // Cache size in bytes
    uint32_t line_size;          // Cache line size
    uint32_t associativity;      // Cache associativity
    bool is_shared;              // Shared between cores
} CacheLevelInfo;

/**
 * CPU Topology Information
 */
typedef struct {
    uint32_t num_cpus;
    uint32_t num_numa_nodes;
    NumaNodeInfo numa_nodes[MAX_NUMA_NODES];
    CacheLevelInfo cache_levels[MAX_CACHE_LEVELS];
    bool numa_available;
} CpuTopology;

/**
 * Work Placement
 * 
 * Describes where work should be placed for optimal cache/NUMA performance.
 */
typedef struct {
    uint32_t cpu_id;             // CPU to run on
    uint32_t numa_node;          // NUMA node
    uint64_t work_size;          // Size of work in bytes
    bool pin_to_cpu;             // Whether to pin thread to CPU
} WorkPlacement;

/**
 * Cache-Aware Distribution
 * 
 * Distributes work with cache and NUMA awareness.
 */
typedef struct {
    WorkPlacement* placements;   // Array of work placements
    size_t num_placements;       // Number of placements
    uint64_t total_work_size;    // Total work size
    bool is_valid;               // Distribution is valid
} CacheAwareDistribution;

/**
 * Work Stealing Queue
 * 
 * Lock-free queue for work stealing between threads.
 */
typedef struct {
    void** items;                // Work items
    size_t capacity;             // Queue capacity
    _Atomic size_t head;         // Head index (consumer)
    _Atomic size_t tail;         // Tail index (producer)
    _Atomic size_t size;         // Current size
} WorkStealingQueue;

/**
 * Cache-Aware Statistics
 */
typedef struct {
    uint64_t total_distributions;
    uint64_t numa_aware_distributions;
    uint64_t cache_optimized_distributions;
    uint64_t work_steals;
    uint64_t failed_steals;
    double avg_work_size;
    double cache_hit_rate;
} CacheAwareStats;

// ============================================================================
// TOPOLOGY DETECTION
// ============================================================================

/**
 * Detect CPU topology
 * 
 * Detects NUMA nodes, cache hierarchy, and CPU count.
 * 
 * @return CPU topology information
 */
CpuTopology detect_cpu_topology(void);

/**
 * Get number of NUMA nodes (cache-aware version)
 * 
 * @return Number of NUMA nodes, or 1 if NUMA not available
 */
uint32_t cache_aware_get_num_numa_nodes(void);

/**
 * Get CPU count for NUMA node
 * 
 * @param node_id NUMA node ID
 * @return Number of CPUs on node
 */
uint32_t cache_aware_get_numa_node_cpu_count(uint32_t node_id);

/**
 * Get cache size for level
 * 
 * @param level Cache level (1, 2, or 3)
 * @return Cache size in bytes
 */
uint64_t cache_aware_get_cache_size(uint32_t level);

/**
 * Check if NUMA is available
 * 
 * @return true if NUMA is available, false otherwise
 */
bool cache_aware_is_numa_available(void);

// ============================================================================
// CACHE-AWARE DISTRIBUTION
// ============================================================================

/**
 * Create cache-aware distribution
 * 
 * Creates work distribution optimized for cache and NUMA.
 * 
 * @param num_workers Number of worker threads
 * @param work_sizes Array of work sizes for each worker
 * @param topology CPU topology information
 * @return Cache-aware distribution
 */
CacheAwareDistribution create_cache_aware_distribution(
    size_t num_workers,
    const uint64_t* work_sizes,
    const CpuTopology* topology
);

/**
 * Optimize distribution for cache locality
 * 
 * Adjusts work placement to maximize cache locality.
 * 
 * @param distribution Distribution to optimize
 * @param topology CPU topology
 * @return true if optimized, false otherwise
 */
bool optimize_for_cache_locality(
    CacheAwareDistribution* distribution,
    const CpuTopology* topology
);

/**
 * Optimize distribution for NUMA
 * 
 * Adjusts work placement to minimize NUMA remote access.
 * 
 * @param distribution Distribution to optimize
 * @param topology CPU topology
 * @return true if optimized, false otherwise
 */
bool optimize_for_numa(
    CacheAwareDistribution* distribution,
    const CpuTopology* topology
);

/**
 * Balance distribution
 * 
 * Balances work across CPUs for even load.
 * 
 * @param distribution Distribution to balance
 * @return true if balanced, false otherwise
 */
bool balance_distribution(CacheAwareDistribution* distribution);

// ============================================================================
// WORK STEALING
// ============================================================================

/**
 * Create work stealing queue
 * 
 * @param capacity Queue capacity
 * @return Allocated queue, or NULL on error
 */
WorkStealingQueue* work_stealing_queue_create(size_t capacity);

/**
 * Destroy work stealing queue
 * 
 * @param queue Queue to destroy
 */
void work_stealing_queue_destroy(WorkStealingQueue* queue);

/**
 * Push work item (producer side)
 * 
 * @param queue Work stealing queue
 * @param item Work item to push
 * @return true if pushed, false if queue full
 */
bool work_stealing_queue_push(WorkStealingQueue* queue, void* item);

/**
 * Pop work item (consumer side)
 * 
 * @param queue Work stealing queue
 * @return Work item, or NULL if queue empty
 */
void* work_stealing_queue_pop(WorkStealingQueue* queue);

/**
 * Steal work item (thief side)
 * 
 * Steals from the head of the queue (opposite end from pop).
 * 
 * @param queue Work stealing queue
 * @return Stolen work item, or NULL if queue empty
 */
void* work_stealing_queue_steal(WorkStealingQueue* queue);

/**
 * Get queue size
 * 
 * @param queue Work stealing queue
 * @return Current queue size
 */
size_t work_stealing_queue_size(const WorkStealingQueue* queue);

/**
 * Check if queue is empty
 * 
 * @param queue Work stealing queue
 * @return true if empty, false otherwise
 */
bool work_stealing_queue_is_empty(const WorkStealingQueue* queue);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Initialize cache-aware statistics
 * 
 * @param stats Statistics structure
 */
void cache_aware_stats_init(CacheAwareStats* stats);

/**
 * Update statistics
 * 
 * @param stats Statistics structure
 * @param distribution Cache-aware distribution
 */
void cache_aware_stats_update(
    CacheAwareStats* stats,
    const CacheAwareDistribution* distribution
);

/**
 * Record work steal
 * 
 * @param stats Statistics structure
 * @param success Whether steal was successful
 */
void cache_aware_stats_record_steal(
    CacheAwareStats* stats,
    bool success
);

/**
 * Print statistics
 * 
 * @param stats Statistics structure
 */
void cache_aware_stats_print(const CacheAwareStats* stats);

/**
 * Reset statistics
 * 
 * @param stats Statistics structure
 */
void cache_aware_stats_reset(CacheAwareStats* stats);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Free cache-aware distribution
 * 
 * @param distribution Distribution to free
 */
void cache_aware_distribution_free(CacheAwareDistribution* distribution);

/**
 * Validate cache-aware distribution
 * 
 * @param distribution Distribution to validate
 * @return true if valid, false otherwise
 */
bool cache_aware_distribution_validate(const CacheAwareDistribution* distribution);

/**
 * Print cache-aware distribution
 * 
 * @param distribution Distribution to print
 */
void cache_aware_distribution_print(const CacheAwareDistribution* distribution);

/**
 * Print CPU topology
 * 
 * @param topology Topology to print
 */
void cpu_topology_print(const CpuTopology* topology);

/**
 * Get optimal CPU for work
 * 
 * Selects the best CPU for a given work size based on cache and NUMA.
 * 
 * @param work_size Size of work in bytes
 * @param topology CPU topology
 * @return Optimal CPU ID
 */
uint32_t get_optimal_cpu(uint64_t work_size, const CpuTopology* topology);

/**
 * Calculate cache affinity
 * 
 * Calculates how well work fits in cache.
 * 
 * @param work_size Size of work in bytes
 * @param cache_size Cache size in bytes
 * @return Affinity score (0.0 to 1.0)
 */
double calculate_cache_affinity(uint64_t work_size, uint64_t cache_size);

#ifdef __cplusplus
}
#endif

#endif // CLLM_CACHE_AWARE_DISTRIBUTION_H
