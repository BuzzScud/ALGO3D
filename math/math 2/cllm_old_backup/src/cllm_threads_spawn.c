/**
 * CLLM Dynamic Thread Spawning
 * 
 * Implements dynamic child thread spawning and termination for the
 * kissing spheres threading system.
 * 
 * Key Features:
 * - Spawn children on-demand based on workload
 * - Terminate idle children to free resources
 * - Maintain 12-fold symmetry
 * - CPU availability monitoring
 */

#include "ai/cllm_lattice_hierarchy.h"
#include "cllm_threads.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysinfo.h>

// Forward declaration of worker thread function
extern void* lattice_sphere_worker_thread(void* arg);

/**
 * Get current CPU load average
 * 
 * Returns the 1-minute load average
 */
static float get_cpu_load() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        return 0.0;
    }
    
    // Load average is scaled by 65536
    return (float)info.loads[0] / 65536.0f;
}

/**
 * Get number of available CPUs
 */
static int get_available_cpus() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

/**
 * Check if a sphere can spawn children
 * 
 * Criteria:
 * - Sphere has fewer than 12 children
 * - Work queue size is above threshold
 * - CPU load is below threshold
 * - System has available CPUs
 */
int sphere_can_spawn_children(CLLMLatticeHierarchy* sphere, int work_threshold) {
    if (!sphere) return 0;
    
    // Already has maximum children (12-fold symmetry)
    if (sphere->num_children >= 12) {
        return 0;
    }
    
    // Not enough work to justify spawning
    size_t queue_size = atomic_load(&sphere->work_queue_size);
    if (queue_size < (size_t)work_threshold) {
        return 0;
    }
    
    // CPU load too high
    float cpu_load = get_cpu_load();
    int num_cpus = get_available_cpus();
    if (cpu_load > (float)num_cpus * 0.8f) {  // 80% threshold
        return 0;
    }
    
    return 1;
}

/**
 * Spawn a single child thread for a sphere
 * 
 * Creates a new child sphere and starts its worker thread.
 * Maintains 12-fold symmetry by assigning appropriate symmetry group.
 * 
 * @param parent Parent sphere
 * @param sphere_id Unique ID for the new sphere
 * @param physical_thread_id Physical thread ID
 * @return Pointer to new child sphere, or NULL on failure
 */
CLLMLatticeHierarchy* sphere_spawn_child(CLLMLatticeHierarchy* parent, 
                                         int sphere_id,
                                         int physical_thread_id) {
    if (!parent) return NULL;
    
    // Determine symmetry group (cycle through 0-11)
    int symmetry_group = parent->num_children % 12;
    int groups[1] = {symmetry_group};
    
    // Create child sphere
    CLLMLatticeHierarchy* child = lattice_hierarchy_create(
        sphere_id,
        parent->hierarchy_level + 1,
        groups,
        1,
        physical_thread_id,
        parent
    );
    
    if (!child) {
        fprintf(stderr, "ERROR: Failed to create child sphere\n");
        return NULL;
    }
    
    // Add to parent's children array
    if (lattice_hierarchy_add_child(parent, child) != 1) {
        fprintf(stderr, "ERROR: Failed to add child to parent\n");
        lattice_hierarchy_free(child);
        return NULL;
    }
    
    // Start worker thread
    if (pthread_create(&child->thread, NULL, lattice_sphere_worker_thread, child) != 0) {
        fprintf(stderr, "ERROR: Failed to create worker thread\n");
        // Note: Child is already added to parent, need to handle cleanup carefully
        return NULL;
    }
    
    printf("[SPAWN] Parent %s spawned child %s (symmetry group %d)\n",
           parent->debug_name, child->debug_name, symmetry_group);
    
    return child;
}

/**
 * Terminate a child thread
 * 
 * Signals the child to terminate, waits for completion, and frees resources.
 * 
 * @param parent Parent sphere
 * @param child Child sphere to terminate
 * @return 0 on success, -1 on failure
 */
int sphere_terminate_child(CLLMLatticeHierarchy* parent, CLLMLatticeHierarchy* child) {
    if (!parent || !child) return -1;
    
    printf("[TERMINATE] Parent %s terminating child %s\n",
           parent->debug_name, child->debug_name);
    
    // Signal child to terminate
    lattice_hierarchy_set_state(child, HIERARCHY_STATE_TERMINATING);
    
    // Wait for thread to finish
    pthread_join(child->thread, NULL);
    
    // Remove from parent's children array
    pthread_mutex_lock(&parent->children_mutex);
    for (int i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < parent->num_children - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->num_children--;
            break;
        }
    }
    pthread_mutex_unlock(&parent->children_mutex);
    
    // Free child resources
    lattice_hierarchy_free(child);
    
    return 0;
}

/**
 * Check if children should be spawned
 * 
 * Called periodically by control threads to decide if they should
 * spawn additional children based on workload.
 * 
 * @param sphere Control thread sphere
 * @param work_threshold Minimum work queue size to trigger spawning
 * @return Number of children to spawn (0 if none)
 */
int sphere_check_spawn_children(CLLMLatticeHierarchy* sphere, int work_threshold) {
    if (!sphere) return 0;
    
    // Only control threads can spawn
    if (sphere->num_children == 0) return 0;
    
    // Check if we can spawn
    if (!sphere_can_spawn_children(sphere, work_threshold)) {
        return 0;
    }
    
    // Calculate how many children to spawn
    // Spawn in groups to maintain symmetry
    size_t queue_size = atomic_load(&sphere->work_queue_size);
    int available_slots = 12 - sphere->num_children;
    
    // Spawn based on work queue size
    // For every 10 work items, spawn 1 child (up to available slots)
    int desired_children = (int)(queue_size / 10);
    if (desired_children > available_slots) {
        desired_children = available_slots;
    }
    
    // Spawn in groups of 1, 3, 6, or 12 to maintain symmetry
    if (desired_children >= 12) return 12;
    if (desired_children >= 6) return 6;
    if (desired_children >= 3) return 3;
    if (desired_children >= 1) return 1;
    
    return 0;
}

/**
 * Check if children should be terminated
 * 
 * Called periodically by control threads to decide if they should
 * terminate idle children to free resources.
 * 
 * @param sphere Control thread sphere
 * @param idle_threshold Maximum idle time before termination (seconds)
 * @return Number of children to terminate (0 if none)
 */
int sphere_check_terminate_children(CLLMLatticeHierarchy* sphere, int idle_threshold) {
    if (!sphere) return 0;
    
    // Need at least one child to terminate
    if (sphere->num_children == 0) return 0;
    
    // Check CPU load - don't terminate if system is busy
    float cpu_load = get_cpu_load();
    int num_cpus = get_available_cpus();
    if (cpu_load > (float)num_cpus * 0.5f) {  // 50% threshold
        return 0;
    }
    
    // Count idle children
    int idle_count = 0;
    for (int i = 0; i < sphere->num_children; i++) {
        CLLMLatticeHierarchy* child = sphere->children[i];
        
        // Check if child is idle
        int state = atomic_load(&child->state);
        size_t queue_size = atomic_load(&child->work_queue_size);
        
        if (state == HIERARCHY_STATE_IDLE && queue_size == 0) {
            idle_count++;
        }
    }
    
    // Terminate idle children if we have enough of them
    // Keep at least 1 child to maintain control thread status
    int can_terminate = idle_count;
    if (sphere->num_children - can_terminate < 1) {
        can_terminate = sphere->num_children - 1;
    }
    
    // Only terminate if more than half are idle
    if (idle_count > sphere->num_children / 2) {
        return can_terminate;
    }
    
    (void)idle_threshold;  // Unused for now, could implement time-based logic
    return 0;
}
