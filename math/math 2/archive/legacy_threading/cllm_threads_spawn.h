#ifndef CLLM_THREADS_SPAWN_H
#define CLLM_THREADS_SPAWN_H

#include "ai/cllm_lattice_hierarchy.h"

/**
 * CLLM Dynamic Thread Spawning
 * 
 * Functions for dynamically spawning and terminating child threads
 * based on workload and CPU availability.
 */

/**
 * Check if a sphere can spawn children
 * 
 * @param sphere Sphere to check
 * @param work_threshold Minimum work queue size to allow spawning
 * @return 1 if can spawn, 0 otherwise
 */
int sphere_can_spawn_children(CLLMLatticeHierarchy* sphere, int work_threshold);

/**
 * Spawn a single child thread
 * 
 * @param parent Parent sphere
 * @param sphere_id Unique ID for new sphere
 * @param physical_thread_id Physical thread ID
 * @return Pointer to new child, or NULL on failure
 */
CLLMLatticeHierarchy* sphere_spawn_child(CLLMLatticeHierarchy* parent,
                                         int sphere_id,
                                         int physical_thread_id);

/**
 * Terminate a child thread
 * 
 * @param parent Parent sphere
 * @param child Child to terminate
 * @return 0 on success, -1 on failure
 */
int sphere_terminate_child(CLLMLatticeHierarchy* parent, CLLMLatticeHierarchy* child);

/**
 * Check if children should be spawned
 * 
 * @param sphere Control thread sphere
 * @param work_threshold Minimum work queue size
 * @return Number of children to spawn
 */
int sphere_check_spawn_children(CLLMLatticeHierarchy* sphere, int work_threshold);

/**
 * Check if children should be terminated
 * 
 * @param sphere Control thread sphere
 * @param idle_threshold Maximum idle time (seconds)
 * @return Number of children to terminate
 */
int sphere_check_terminate_children(CLLMLatticeHierarchy* sphere, int idle_threshold);

#endif // CLLM_THREADS_SPAWN_H