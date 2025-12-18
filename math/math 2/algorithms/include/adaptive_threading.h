/**
 * @file adaptive_threading.h
 * @brief Adaptive Threading for 88D System - Efficient on Limited Cores
 * 
 * CRITICAL OPTIMIZATION
 * 
 * This module provides adaptive threading that:
 * - Maintains 96 logical threads (88D structure)
 * - Creates only N physical threads (N = available cores)
 * - Uses work stealing for load balancing
 * - Reduces memory from 9 GB to 100-800 MB
 * - Eliminates context switching overhead
 * - Scales efficiently from 1 to 96+ cores
 * 
 * Key Concepts:
 * - Logical threads: 96 (data structures, work queues)
 * - Physical threads: N (OS threads, actual execution)
 * - Work stealing: Physical threads steal work from logical queues
 * - Shared memory: Physical threads share memory pools
 */

#ifndef ADAPTIVE_THREADING_H
#define ADAPTIVE_THREADING_H

#include "hierarchical_threading.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// SYSTEM DETECTION
// ============================================================================

/**
 * Get number of available CPU cores
 * 
 * Uses sysconf(_SC_NPROCESSORS_ONLN) on Unix systems.
 * Falls back to conservative default if detection fails.
 * 
 * @return Number of available cores
 */
uint32_t adaptive_get_available_cores(void);

/**
 * Get available system memory in MB
 * 
 * @return Available memory in megabytes
 */
uint64_t adaptive_get_available_memory_mb(void);

/**
 * Get recommended thread count based on system resources
 * 
 * Takes into account:
 * - Available CPU cores
 * - Available memory
 * - Workload type
 * 
 * @param workload_type Type of workload (0=CPU, 1=Memory, 2=Balanced)
 * @return Recommended number of physical threads
 */
uint32_t adaptive_get_recommended_threads(int workload_type);

// ============================================================================
// ADAPTIVE THREAD POOL
// ============================================================================

/**
 * Create 88D thread pool with adaptive threading
 * 
 * This function creates:
 * - 96 logical threads (full 88D structure)
 * - N physical threads (N = max_physical_threads or auto-detected)
 * - Work stealing scheduler
 * - Shared memory pools
 * 
 * Memory usage: ~100 MB per physical thread
 * 
 * @param base Tetration base (typically 2)
 * @param max_physical_threads Maximum physical threads (0 = auto-detect)
 * @return Thread pool or NULL on error
 */
HierarchicalThreadPool* hierarchical_thread_pool_create_88d_adaptive(
    uint32_t base,
    uint32_t max_physical_threads
);

/**
 * Get adaptive threading statistics
 * 
 * @param pool Thread pool
 * @param out_logical_threads Output: number of logical threads
 * @param out_physical_threads Output: number of physical threads
 * @param out_work_stolen Output: total work items stolen
 * @param out_memory_mb Output: memory usage in MB
 * @return 0 on success, -1 on error
 */
int adaptive_get_statistics(
    HierarchicalThreadPool* pool,
    uint32_t* out_logical_threads,
    uint32_t* out_physical_threads,
    uint64_t* out_work_stolen,
    uint64_t* out_memory_mb
);

/**
 * Print adaptive threading statistics
 * 
 * @param pool Thread pool
 */
void adaptive_print_statistics(HierarchicalThreadPool* pool);

/**
 * Start physical threads for adaptive pool
 * 
 * This function starts the physical OS threads that will steal work
 * from the logical thread queues. Should be called after pool creation.
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int adaptive_start_physical_threads(HierarchicalThreadPool* pool);

/**
 * Stop physical threads for adaptive pool
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int adaptive_stop_physical_threads(HierarchicalThreadPool* pool);

// ============================================================================
// CONFIGURATION
// ============================================================================

/**
 * Adaptive threading configuration
 */
typedef struct {
    uint32_t max_physical_threads;   // 0 = auto-detect
    uint64_t memory_limit_mb;        // 0 = no limit
    bool enable_work_stealing;       // Default: true
    bool enable_shared_memory;       // Default: true
    bool enable_numa_awareness;      // Default: true
    uint32_t memory_pool_size_mb;    // Per physical thread (default: 100)
} AdaptiveThreadingConfig;

/**
 * Get default adaptive threading configuration
 * 
 * @return Default configuration
 */
AdaptiveThreadingConfig adaptive_get_default_config(void);

/**
 * Create 88D thread pool with custom configuration
 * 
 * @param base Tetration base
 * @param config Configuration
 * @return Thread pool or NULL on error
 */
HierarchicalThreadPool* hierarchical_thread_pool_create_88d_configured(
    uint32_t base,
    const AdaptiveThreadingConfig* config
);

#ifdef __cplusplus
}
#endif

#endif /* ADAPTIVE_THREADING_H */