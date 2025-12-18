/**
 * @file abacus88d_threading.h
 * @brief Integration layer between Abacus88D and Hierarchical Threading
 * 
 * This module connects the 88D geometric computation system with the
 * hierarchical threading framework, enabling:
 * - Work distribution across 88 dimensions
 * - Thread-safe operations on geometric space
 * - Load balancing across layers and dimensions
 * - Performance monitoring and optimization
 */

#ifndef ALGORITHMS_ABACUS88D_THREADING_H
#define ALGORITHMS_ABACUS88D_THREADING_H

#include "abacus88d.h"
#include "geometric_space_ops.h"
#include "hierarchical_threading.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONSTANTS
 * ============================================================================
 */

#define ABACUS88D_THREADS_PER_LAYER 11  // One thread per dimension
#define ABACUS88D_TOTAL_THREADS 88      // 8 layers × 11 dimensions

/* ============================================================================
 * WORK ITEM TYPES
 * ============================================================================
 */

/**
 * @brief Type of work to be performed on Abacus88D
 */
typedef enum {
    ABACUS88D_WORK_BOUNDARY_DETECT,     // Detect boundaries
    ABACUS88D_WORK_TRANSFORM,           // Apply transformation
    ABACUS88D_WORK_HANDOFF,             // Execute handoff
    ABACUS88D_WORK_COMPUTE,             // General computation
    ABACUS88D_WORK_SYNC                 // Synchronization point
} Abacus88DWorkType;

/**
 * @brief Work item for Abacus88D operations
 */
typedef struct {
    Abacus88DWorkType type;             // Type of work
    uint8_t layer;                      // Target layer
    uint8_t dimension;                  // Target dimension
    uint32_t priority;                  // Work priority
    void* data;                         // Work-specific data
    void* result;                       // Result storage
} Abacus88DWorkItem;

/* ============================================================================
 * THREAD POOL STRUCTURE
 * ============================================================================
 */

/**
 * @brief Thread pool for Abacus88D operations
 * 
 * Manages a pool of threads organized by layers and dimensions,
 * with work distribution and load balancing.
 */
typedef struct {
    Abacus88D* abacus;                  // Shared Abacus88D instance
    HierarchicalThreadPool* thread_pool; // Underlying thread pool
    
    // Thread organization
    HierarchicalThread* layer_threads[ABACUS88D_NUM_LAYERS]; // One thread per layer
    uint32_t num_threads;               // Total number of threads
    
    // Work distribution
    WorkPool* work_pools[ABACUS88D_NUM_LAYERS]; // Work pool per layer
    uint64_t work_submitted;            // Total work submitted
    uint64_t work_completed;            // Total work completed
    uint64_t work_stolen;               // Work stolen between threads
    
    // Performance monitoring
    uint64_t boundary_detections;       // Total boundary detections
    uint64_t transformations;           // Total transformations
    uint64_t handoffs;                  // Total handoffs
    double avg_work_time;               // Average work completion time
    
    // Synchronization
    pthread_mutex_t pool_mutex;         // Pool-wide mutex
    pthread_cond_t work_available;      // Work available condition
    volatile bool running;              // Pool is running
} Abacus88DThreadPool;

/* ============================================================================
 * THREAD POOL MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Create Abacus88D thread pool
 * 
 * Creates a thread pool for parallel operations on Abacus88D space.
 * 
 * @param abacus The Abacus88D instance to operate on
 * @param num_threads Number of worker threads (0 = auto-detect)
 * @return Thread pool or NULL on error
 */
Abacus88DThreadPool* abacus88d_thread_pool_create(
    Abacus88D* abacus,
    uint32_t num_threads
);

/**
 * @brief Free Abacus88D thread pool
 * 
 * Stops all threads and frees resources.
 * 
 * @param pool Thread pool to free
 */
void abacus88d_thread_pool_free(Abacus88DThreadPool* pool);

/**
 * @brief Start thread pool
 * 
 * Starts all worker threads.
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int abacus88d_thread_pool_start(Abacus88DThreadPool* pool);

/**
 * @brief Stop thread pool
 * 
 * Stops all worker threads gracefully.
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int abacus88d_thread_pool_stop(Abacus88DThreadPool* pool);

/**
 * @brief Wait for all work to complete
 * 
 * Blocks until all submitted work is completed.
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int abacus88d_thread_pool_wait(Abacus88DThreadPool* pool);

/* ============================================================================
 * WORK SUBMISSION
 * ============================================================================
 */

/**
 * @brief Submit boundary detection work
 * 
 * Submits work to detect boundaries for a value.
 * 
 * @param pool Thread pool
 * @param layer Target layer
 * @param dimension Target dimension
 * @param value Value to analyze
 * @param boundary Output boundary information
 * @return Work ID or 0 on error
 */
uint64_t abacus88d_submit_boundary_detection(
    Abacus88DThreadPool* pool,
    uint8_t layer,
    uint8_t dimension,
    const CrystallineAbacus* value,
    BoundaryInfo* boundary
);

/**
 * @brief Submit transformation work
 * 
 * Submits work to apply a transformation.
 * 
 * @param pool Thread pool
 * @param transform Transformation to apply
 * @param input Input value
 * @param output Output value
 * @return Work ID or 0 on error
 */
uint64_t abacus88d_submit_transformation(
    Abacus88DThreadPool* pool,
    const TransformMatrix* transform,
    const CrystallineAbacus* input,
    CrystallineAbacus* output
);

/**
 * @brief Submit handoff work
 * 
 * Submits work to execute a handoff operation.
 * 
 * @param pool Thread pool
 * @param context Handoff context
 * @param value Value to transfer
 * @return Work ID or 0 on error
 */
uint64_t abacus88d_submit_handoff(
    Abacus88DThreadPool* pool,
    const HandoffContext* context,
    const CrystallineAbacus* value
);

/**
 * @brief Submit batch work
 * 
 * Submits multiple work items as a batch.
 * 
 * @param pool Thread pool
 * @param items Array of work items
 * @param count Number of items
 * @return Number of items submitted
 */
uint32_t abacus88d_submit_batch(
    Abacus88DThreadPool* pool,
    const Abacus88DWorkItem* items,
    uint32_t count
);

/* ============================================================================
 * WORK STEALING
 * ============================================================================
 */

/**
 * @brief Enable work stealing
 * 
 * Enables work stealing between threads for load balancing.
 * 
 * @param pool Thread pool
 * @param enabled True to enable, false to disable
 * @return 0 on success, -1 on error
 */
int abacus88d_set_work_stealing(
    Abacus88DThreadPool* pool,
    bool enabled
);

/**
 * @brief Get work stealing statistics
 * 
 * Returns statistics about work stealing activity.
 * 
 * @param pool Thread pool
 * @param stolen_count Output: number of work items stolen
 * @param steal_attempts Output: number of steal attempts
 * @return 0 on success, -1 on error
 */
int abacus88d_get_stealing_stats(
    Abacus88DThreadPool* pool,
    uint64_t* stolen_count,
    uint64_t* steal_attempts
);

/* ============================================================================
 * LOAD BALANCING
 * ============================================================================
 */

/**
 * @brief Balance load across threads
 * 
 * Redistributes work to balance load across all threads.
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int abacus88d_balance_load(Abacus88DThreadPool* pool);

/**
 * @brief Get load statistics
 * 
 * Returns load statistics for each thread.
 * 
 * @param pool Thread pool
 * @param loads Output: array of load values per thread
 * @param count Size of loads array
 * @return 0 on success, -1 on error
 */
int abacus88d_get_load_stats(
    Abacus88DThreadPool* pool,
    uint64_t* loads,
    uint32_t count
);

/* ============================================================================
 * PERFORMANCE MONITORING
 * ============================================================================
 */

/**
 * @brief Performance statistics
 */
typedef struct {
    uint64_t work_submitted;            // Total work submitted
    uint64_t work_completed;            // Total work completed
    uint64_t work_stolen;               // Work stolen
    uint64_t boundary_detections;       // Boundary detections
    uint64_t transformations;           // Transformations
    uint64_t handoffs;                  // Handoffs
    double avg_work_time;               // Average work time (seconds)
    double throughput;                  // Work items per second
    uint32_t active_threads;            // Currently active threads
} Abacus88DPerformanceStats;

/**
 * @brief Get performance statistics
 * 
 * Returns current performance statistics.
 * 
 * @param pool Thread pool
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int abacus88d_get_performance_stats(
    Abacus88DThreadPool* pool,
    Abacus88DPerformanceStats* stats
);

/**
 * @brief Print performance statistics
 * 
 * Prints performance statistics to stdout.
 * 
 * @param pool Thread pool
 */
void abacus88d_print_performance_stats(Abacus88DThreadPool* pool);

/**
 * @brief Reset performance statistics
 * 
 * Resets all performance counters to zero.
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int abacus88d_reset_performance_stats(Abacus88DThreadPool* pool);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_ABACUS88D_THREADING_H */