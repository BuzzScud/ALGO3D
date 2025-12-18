#ifndef CLLM_KISSING_SPHERES_THREADING_H
#define CLLM_KISSING_SPHERES_THREADING_H

#include "ai/cllm_lattice_hierarchy.h"

/**
 * CLLM Kissing Spheres Threading System
 * 
 * Implements the complete threading system based on the kissing spheres design:
 * - 1 central sphere (root)
 * - 12 kissing spheres (one per symmetry group 0-11)
 * - Each sphere can have up to 12 children
 * - Hierarchical message passing
 * - Work stealing between siblings
 * - Gradient accumulation up the hierarchy
 */

// ============================================================================
// SYSTEM STRUCTURE
// ============================================================================

typedef struct {
    int num_levels;                      // Number of hierarchy levels
    int spheres_per_level[4];            // Spheres at each level
    int total_spheres;                   // Total number of spheres
    CLLMLatticeHierarchy** all_spheres;  // Array of all spheres
    CLLMLatticeHierarchy* root;          // Root sphere
    pthread_t* threads;                  // Thread handles
    int num_threads;                     // Number of threads
} ThreadSystem;

// ============================================================================
// SYSTEM MANAGEMENT
// ============================================================================

/**
 * Create kissing spheres system (fixed levels)
 * 
 * @param num_levels Number of hierarchy levels (1-4)
 *                   Level 0: 1 root sphere
 *                   Level 1: 12 kissing spheres
 *                   Level 2: 144 spheres (12 per level-1 sphere)
 *                   Level 3: 1728 spheres (12 per level-2 sphere)
 * @return New system, or NULL on error
 */
ThreadSystem* threads_create(int num_levels);

/**
 * Create dynamic kissing spheres system (optimal for CPU count)
 * 
 * Creates a hierarchical system optimized for the given number of CPUs.
 * Automatically calculates optimal depth and sphere distribution.
 * 
 * Strategy:
 * - Level 0: Always 1 (root control)
 * - Level 1: Up to 12 (kissing spheres)
 * - Level 2+: Distribute remaining CPUs evenly
 * 
 * @param num_cpus Number of CPU cores to utilize
 * @return New system, or NULL on error
 */
ThreadSystem* threads_create_dynamic(int num_cpus);

/**
 * Free kissing spheres system
 * 
 * @param system System to free
 */
void threads_free(ThreadSystem* system);

/**
 * Start all sphere threads
 * 
 * @param system Kissing spheres system
 * @return 0 on success, -1 on error
 */
int threads_start(ThreadSystem* system);

/**
 * Stop all sphere threads
 * 
 * @param system Kissing spheres system
 * @return 0 on success, -1 on error
 */
int threads_stop(ThreadSystem* system);

/**
 * Distribute work to spheres
 * 
 * @param system Kissing spheres system
 * @param work_items Array of work item IDs (e.g., batch indices)
 * @param num_items Number of work items
 * @return 0 on success, -1 on error
 */
int threads_distribute_work(ThreadSystem* system, 
                                    uint64_t* work_items, 
                                    int num_items);

/**
 * Print system statistics
 * 
 * @param system Kissing spheres system
 */
void threads_print_stats(ThreadSystem* system);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Get time in nanoseconds
 */
static inline uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * Get number of CPU cores
 */
int get_num_cpu_cores(void);

#endif // CLLM_KISSING_SPHERES_THREADING_H
// Worker thread function (used by dynamic spawning)
void* lattice_sphere_worker_thread(void* arg);
