#include "ai/cllm_control_process.h"
#include "ai/cllm_lattice_hierarchy.h"
#include "ai/cllm_sphere_stats.h"
#include "ai/cllm_sphere_message.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static double get_current_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void* health_monitor_thread_func(void* arg);
static bool collect_statistics_recursive(CLLMLatticeHierarchy* sphere, 
                                         SphereStatistics* stats);
static CLLMLatticeHierarchy* find_sphere_recursive(CLLMLatticeHierarchy* sphere, 
                                                   uint32_t sphere_id);
static uint32_t count_spheres_recursive(const CLLMLatticeHierarchy* sphere);

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

ControlProcess* control_process_create(const SystemConfiguration* config) {
    if (!config) {
        fprintf(stderr, "Error: NULL configuration\n");
        return NULL;
    }
    
    ControlProcess* cp = (ControlProcess*)calloc(1, sizeof(ControlProcess));
    if (!cp) {
        fprintf(stderr, "Error: Failed to allocate control process\n");
        return NULL;
    }
    
    // Initialize state
    cp->state = CONTROL_STATE_INITIALIZING;
    pthread_mutex_init(&cp->state_mutex, NULL);
    pthread_cond_init(&cp->state_cond, NULL);
    
    // Copy configuration
    memcpy(&cp->config, config, sizeof(SystemConfiguration));
    
    // Initialize hierarchy
    cp->root_sphere = NULL;
    cp->total_sphere_count = 0;
    pthread_mutex_init(&cp->hierarchy_mutex, NULL);
    
    // Initialize epoch state
    memset(&cp->epoch_state, 0, sizeof(EpochState));
    pthread_mutex_init(&cp->epoch_mutex, NULL);
    
    // Initialize statistics
    cllm_sphere_stats_init(&cp->system_stats, 0, 0);
    pthread_mutex_init(&cp->stats_mutex, NULL);
    
    // Initialize health monitoring
    memset(&cp->health, 0, sizeof(SystemHealth));
    cp->health_monitor_running = false;
    
    // Initialize synchronization
    pthread_mutex_init(&cp->sync_mutex, NULL);
    
    // Initialize checkpoint path
    snprintf(cp->checkpoint_path, sizeof(cp->checkpoint_path), 
             "./checkpoints");
    cp->checkpoint_version = 0;
    
    return cp;
}

void control_process_free(ControlProcess* cp) {
    if (!cp) return;
    
    // Stop health monitoring
    if (cp->health_monitor_running) {
        cp->health_monitor_running = false;
        pthread_join(cp->health_monitor_thread, NULL);
    }
    
    // Free root sphere (recursively frees entire hierarchy)
    if (cp->root_sphere) {
        lattice_hierarchy_free(cp->root_sphere);
    }
    
    // Destroy synchronization primitives
    pthread_mutex_destroy(&cp->state_mutex);
    pthread_cond_destroy(&cp->state_cond);
    pthread_mutex_destroy(&cp->hierarchy_mutex);
    pthread_mutex_destroy(&cp->epoch_mutex);
    pthread_mutex_destroy(&cp->stats_mutex);
    pthread_mutex_destroy(&cp->sync_mutex);
    
    // Destroy epoch barrier if initialized
    if (cp->epoch_state.epoch_in_progress) {
        pthread_barrier_destroy(&cp->epoch_state.epoch_barrier);
    }
    
    free(cp);
}

bool control_process_start(ControlProcess* cp) {
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->state_mutex);
    
    if (cp->state != CONTROL_STATE_INITIALIZING && 
        cp->state != CONTROL_STATE_STOPPED) {
        pthread_mutex_unlock(&cp->state_mutex);
        fprintf(stderr, "Error: Cannot start from state %d\n", cp->state);
        return false;
    }
    
    // Create root sphere
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    if (!cp->root_sphere) {
        int symmetry_groups[1] = {0};
        cp->root_sphere = lattice_hierarchy_create(
            1,                              // sphere_id
            0,                              // hierarchy_level
            symmetry_groups,                // symmetry_groups
            1,                              // num_symmetry_groups
            0,                              // physical_thread_id
            NULL                            // parent
        );
        
        if (!cp->root_sphere) {
            pthread_mutex_unlock(&cp->hierarchy_mutex);
            pthread_mutex_unlock(&cp->state_mutex);
            fprintf(stderr, "Error: Failed to create root sphere\n");
            return false;
        }
        
        cp->total_sphere_count = 1;
    }
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    // Initialize global barrier
    pthread_barrier_init(&cp->global_barrier, NULL, 
                        cp->config.initial_sphere_count + 1);
    
    // Start health monitoring
    cp->health_monitor_running = true;
    pthread_create(&cp->health_monitor_thread, NULL, 
                  health_monitor_thread_func, cp);
    
    // Update state
    cp->state = CONTROL_STATE_RUNNING;
    pthread_cond_broadcast(&cp->state_cond);
    pthread_mutex_unlock(&cp->state_mutex);
    
    return true;
}

bool control_process_stop(ControlProcess* cp) {
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->state_mutex);
    
    if (cp->state == CONTROL_STATE_STOPPED || 
        cp->state == CONTROL_STATE_STOPPING) {
        pthread_mutex_unlock(&cp->state_mutex);
        return true;
    }
    
    // Update state
    cp->state = CONTROL_STATE_STOPPING;
    pthread_cond_broadcast(&cp->state_cond);
    pthread_mutex_unlock(&cp->state_mutex);
    
    // Stop health monitoring
    if (cp->health_monitor_running) {
        cp->health_monitor_running = false;
        pthread_join(cp->health_monitor_thread, NULL);
    }
    
    // Send termination messages to all spheres
    pthread_mutex_lock(&cp->hierarchy_mutex);
    if (cp->root_sphere) {
        SphereMessage* msg = sphere_message_create(MSG_CHILD_TERMINATE, 
                                                   MSG_PRIORITY_CRITICAL, 0, 0);
        if (msg) {
            lattice_hierarchy_send_message(cp->root_sphere, cp->root_sphere, msg);
            sphere_message_free(msg);
        }
    }
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    // Wait for all spheres to terminate
    usleep(100000); // 100ms grace period
    
    // Update state
    pthread_mutex_lock(&cp->state_mutex);
    cp->state = CONTROL_STATE_STOPPED;
    pthread_cond_broadcast(&cp->state_cond);
    pthread_mutex_unlock(&cp->state_mutex);
    
    return true;
}

bool control_process_pause(ControlProcess* cp) {
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->state_mutex);
    
    if (cp->state != CONTROL_STATE_RUNNING) {
        pthread_mutex_unlock(&cp->state_mutex);
        return false;
    }
    
    cp->state = CONTROL_STATE_PAUSED;
    pthread_cond_broadcast(&cp->state_cond);
    pthread_mutex_unlock(&cp->state_mutex);
    
    return true;
}

bool control_process_resume(ControlProcess* cp) {
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->state_mutex);
    
    if (cp->state != CONTROL_STATE_PAUSED) {
        pthread_mutex_unlock(&cp->state_mutex);
        return false;
    }
    
    cp->state = CONTROL_STATE_RUNNING;
    pthread_cond_broadcast(&cp->state_cond);
    pthread_mutex_unlock(&cp->state_mutex);
    
    return true;
}

// ============================================================================
// EPOCH MANAGEMENT
// ============================================================================

bool control_process_start_epoch(ControlProcess* cp, uint32_t total_batches) {
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->epoch_mutex);
    
    if (cp->epoch_state.epoch_in_progress) {
        pthread_mutex_unlock(&cp->epoch_mutex);
        fprintf(stderr, "Error: Epoch already in progress\n");
        return false;
    }
    
    // Initialize epoch state
    cp->epoch_state.current_epoch++;
    cp->epoch_state.total_batches = total_batches;
    cp->epoch_state.completed_batches = 0;
    cp->epoch_state.epoch_start_time = get_current_time();
    cp->epoch_state.total_loss = 0.0;
    cp->epoch_state.average_loss = 0.0;
    cp->epoch_state.primes_processed = 0;
    cp->epoch_state.gradients_computed = 0;
    cp->epoch_state.weights_updated = 0;
    cp->epoch_state.epoch_in_progress = true;
    
    // Initialize epoch barrier
    pthread_barrier_init(&cp->epoch_state.epoch_barrier, NULL, 
                        cp->total_sphere_count + 1);
    
    pthread_mutex_unlock(&cp->epoch_mutex);
    
    // Broadcast epoch start to all spheres
    pthread_mutex_lock(&cp->hierarchy_mutex);
    if (cp->root_sphere) {
        SphereMessage* msg = sphere_message_create(MSG_EPOCH_START, 
                                                   MSG_PRIORITY_HIGH, 0, 0);
        if (msg) {
            sphere_message_set_epoch(msg, cp->epoch_state.current_epoch, total_batches, 
                                    cp->config.learning_rate);
            lattice_hierarchy_send_message(cp->root_sphere, cp->root_sphere, msg);
            sphere_message_free(msg);
        }
    }
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return true;
}

bool control_process_end_epoch(ControlProcess* cp) {
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->epoch_mutex);
    
    if (!cp->epoch_state.epoch_in_progress) {
        pthread_mutex_unlock(&cp->epoch_mutex);
        return false;
    }
    
    // Calculate epoch statistics
    double end_time = get_current_time();
    cp->epoch_state.epoch_duration = end_time - cp->epoch_state.epoch_start_time;
    
    if (cp->epoch_state.completed_batches > 0) {
        cp->epoch_state.average_loss = cp->epoch_state.total_loss / 
                                       cp->epoch_state.completed_batches;
    }
    
    // Broadcast epoch complete to all spheres
    pthread_mutex_lock(&cp->hierarchy_mutex);
    if (cp->root_sphere) {
        SphereMessage* msg = sphere_message_create(MSG_EPOCH_COMPLETE, 
                                                   MSG_PRIORITY_HIGH, 0, 0);
        if (msg) {
            sphere_message_set_epoch(msg, cp->epoch_state.current_epoch, 
                                    cp->epoch_state.completed_batches,
                                    cp->config.learning_rate);
            lattice_hierarchy_send_message(cp->root_sphere, cp->root_sphere, msg);
            sphere_message_free(msg);
        }
    }
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    // Destroy epoch barrier
    pthread_barrier_destroy(&cp->epoch_state.epoch_barrier);
    
    cp->epoch_state.epoch_in_progress = false;
    pthread_mutex_unlock(&cp->epoch_mutex);
    
    return true;
}

bool control_process_wait_epoch_complete(ControlProcess* cp, double timeout_seconds) {
    (void)timeout_seconds; // Reserved for future timeout implementation
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->epoch_mutex);
    
    if (!cp->epoch_state.epoch_in_progress) {
        pthread_mutex_unlock(&cp->epoch_mutex);
        return true;
    }
    
    pthread_mutex_unlock(&cp->epoch_mutex);
    
    // Wait at epoch barrier
    int result = pthread_barrier_wait(&cp->epoch_state.epoch_barrier);
    
    return (result == 0 || result == PTHREAD_BARRIER_SERIAL_THREAD);
}

bool control_process_get_epoch_stats(ControlProcess* cp, EpochState* epoch_state) {
    if (!cp || !epoch_state) return false;
    
    pthread_mutex_lock(&cp->epoch_mutex);
    memcpy(epoch_state, &cp->epoch_state, sizeof(EpochState));
    pthread_mutex_unlock(&cp->epoch_mutex);
    
    return true;
}

// ============================================================================
// SPHERE LIFECYCLE MANAGEMENT
// ============================================================================

uint32_t control_process_spawn_sphere(ControlProcess* cp, uint32_t parent_id, 
                                      uint32_t symmetry_group) {
    if (!cp) return 0;
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    // Find parent sphere
    CLLMLatticeHierarchy* parent = NULL;
    if (parent_id == 0) {
        parent = cp->root_sphere;
    } else {
        parent = find_sphere_recursive(cp->root_sphere, parent_id);
    }
    
    if (!parent) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        fprintf(stderr, "Error: Parent sphere %u not found\n", parent_id);
        return 0;
    }
    
    // Check if parent can have more children
    if (parent->num_children >= 12) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        fprintf(stderr, "Error: Parent sphere %u already has 12 children\n", parent_id);
        return 0;
    }
    
    // Create new sphere
    uint32_t new_sphere_id = cp->total_sphere_count + 1;
    uint32_t new_level = parent->hierarchy_level + 1;
    
    int symmetry_groups_arr[1] = {(int)symmetry_group};
    CLLMLatticeHierarchy* new_sphere = lattice_hierarchy_create(
        new_sphere_id,
        new_level,
        symmetry_groups_arr,
        1,
        new_sphere_id % cp->config.max_threads,
        parent
    );
    
    if (!new_sphere) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        fprintf(stderr, "Error: Failed to create sphere\n");
        return 0;
    }
    
    // Add to parent
    if (!lattice_hierarchy_add_child(parent, new_sphere)) {
        lattice_hierarchy_free(new_sphere);
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        fprintf(stderr, "Error: Failed to add child to parent\n");
        return 0;
    }
    
    cp->total_sphere_count++;
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return new_sphere_id;
}

bool control_process_terminate_sphere(ControlProcess* cp, uint32_t sphere_id) {
    if (!cp || sphere_id == 0) return false;
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    // Find sphere
    CLLMLatticeHierarchy* sphere = find_sphere_recursive(cp->root_sphere, sphere_id);
    if (!sphere) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        return false;
    }
    
    // Cannot terminate root
    if (sphere == cp->root_sphere) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        return false;
    }
    
    // Send termination message
    SphereMessage* msg = sphere_message_create(MSG_CHILD_TERMINATE, 
                                               MSG_PRIORITY_CRITICAL, 0, sphere_id);
    if (msg) {
        lattice_hierarchy_send_message(sphere, sphere, msg);
        sphere_message_free(msg);
    }
    
    // Remove from parent's children
    CLLMLatticeHierarchy* parent = sphere->parent;
    if (parent) {
        for (uint32_t i = 0; i < (uint32_t)parent->num_children; i++) {
            if (parent->children[i] == sphere) {
                // Shift remaining children
                for (uint32_t j = i; j < (uint32_t)(parent->num_children - 1); j++) {
                    parent->children[j] = parent->children[j + 1];
                }
                parent->num_children--;
                break;
            }
        }
    }
    
    // Free sphere
    lattice_hierarchy_free(sphere);
    cp->total_sphere_count--;
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return true;
}

bool control_process_rebalance_hierarchy(ControlProcess* cp) {
    if (!cp) return false;
    
    // TODO: Implement intelligent rebalancing based on load metrics
    // For now, this is a placeholder
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    // Collect statistics from all spheres
    SphereStatistics stats;
    cllm_sphere_stats_init(&stats, 0, 0);
    collect_statistics_recursive(cp->root_sphere, &stats);
    
    // Initialize stats with default values
    cllm_sphere_stats_init(&stats, 0, 0);
    
    // Analyze load distribution
    // Identify overloaded and underloaded spheres
    // Migrate work or spawn/terminate spheres as needed
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return true;
}

bool control_process_check_sphere_health(ControlProcess* cp, uint32_t sphere_id) {
    if (!cp) return false;
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    CLLMLatticeHierarchy* sphere = find_sphere_recursive(cp->root_sphere, sphere_id);
    if (!sphere) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        return false;
    }
    
    // Check sphere state
    HierarchyState state = lattice_hierarchy_get_state(sphere);
    bool healthy = (state != HIERARCHY_STATE_TERMINATED);
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return healthy;
}

bool control_process_recover_sphere(ControlProcess* cp, uint32_t sphere_id) {
    (void)sphere_id; // Reserved for future sphere recovery implementation
    if (!cp) return false;
    
    // TODO: Implement sphere recovery logic
    // This would involve:
    // 1. Detecting the failure
    // 2. Saving the sphere's state
    // 3. Creating a new sphere
    // 4. Restoring the state
    // 5. Reconnecting to the hierarchy
    
    return false;
}

// ============================================================================
// SYSTEM-WIDE OPERATIONS
// ============================================================================

bool control_process_broadcast_weights(ControlProcess* cp, const double* weights, 
                                       size_t weight_count) {
    (void)weight_count; // Reserved for future weight validation
    if (!cp || !weights) return false;
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    if (!cp->root_sphere) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        return false;
    }
    
    // Create weight update message
    SphereMessage* msg = sphere_message_create(MSG_WEIGHTS_BROADCAST, 
                                               MSG_PRIORITY_HIGH, 0, 0);
    if (msg) {
        // Broadcast to all spheres
        lattice_hierarchy_send_message(cp->root_sphere, cp->root_sphere, msg);
        sphere_message_free(msg);
    }
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return true;
}

bool control_process_collect_gradients(ControlProcess* cp, double* gradients, 
                                       size_t gradient_count) {
    if (!cp || !gradients) return false;
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    if (!cp->root_sphere) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        return false;
    }
    
    // Initialize gradients to zero
    memset(gradients, 0, gradient_count * sizeof(double));
    
    // TODO: Implement gradient collection from all spheres
    // This would involve:
    // 1. Sending gradient request messages
    // 2. Collecting responses
    // 3. Aggregating gradients
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return true;
}

bool control_process_synchronize_all(ControlProcess* cp, double timeout_seconds) {
    (void)timeout_seconds; // Reserved for future timeout implementation
    if (!cp) return false;
    
    // Wait at global barrier
    int result = pthread_barrier_wait(&cp->global_barrier);
    
    return (result == 0 || result == PTHREAD_BARRIER_SERIAL_THREAD);
}

bool control_process_checkpoint(ControlProcess* cp, const char* checkpoint_name) {
    if (!cp || !checkpoint_name) return false;
    
    // TODO: Implement checkpoint saving
    // This would involve:
    // 1. Pausing training
    // 2. Collecting state from all spheres
    // 3. Saving to disk
    // 4. Resuming training
    
    cp->checkpoint_version++;
    
    return true;
}

bool control_process_restore(ControlProcess* cp, const char* checkpoint_name) {
    if (!cp || !checkpoint_name) return false;
    
    // TODO: Implement checkpoint restoration
    // This would involve:
    // 1. Loading checkpoint from disk
    // 2. Recreating sphere hierarchy
    // 3. Restoring state to all spheres
    
    return true;
}

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

bool control_process_get_system_stats(ControlProcess* cp, SphereStatistics* stats) {
    if (!cp || !stats) return false;
    
    pthread_mutex_lock(&cp->stats_mutex);
    
    // Initialize stats
    cllm_sphere_stats_init(stats, 0, 0);
    
    // Collect from all spheres
    pthread_mutex_lock(&cp->hierarchy_mutex);
    if (cp->root_sphere) {
        collect_statistics_recursive(cp->root_sphere, stats);
    }
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    pthread_mutex_unlock(&cp->stats_mutex);
    
    return true;
}

bool control_process_get_sphere_stats(ControlProcess* cp, uint32_t sphere_id, 
                                      SphereStatistics* stats) {
    if (!cp || !stats) return false;
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    
    CLLMLatticeHierarchy* sphere = find_sphere_recursive(cp->root_sphere, sphere_id);
    if (!sphere) {
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        return false;
    }
    
    memcpy(stats, &sphere->stats, sizeof(SphereStatistics));
    
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return true;
}

bool control_process_get_system_health(ControlProcess* cp, SystemHealth* health) {
    if (!cp || !health) return false;
    
    memcpy(health, &cp->health, sizeof(SystemHealth));
    
    return true;
}

void control_process_print_hierarchy(const ControlProcess* cp) {
    if (!cp) return;
    
    printf("\n=== Sphere Hierarchy ===\n");
    printf("Total Spheres: %u\n", cp->total_sphere_count);
    printf("State: %s\n", control_process_state_to_string(cp->state));
    
    if (cp->root_sphere) {
        printf("\nHierarchy Structure:\n");
        lattice_hierarchy_print_tree(cp->root_sphere, 0);
    }
    
    printf("\n");
}

void control_process_print_stats(const ControlProcess* cp) {
    if (!cp) return;
    
    printf("\n=== System Statistics ===\n");
    
    // Epoch stats
    printf("\nEpoch Information:\n");
    printf("  Current Epoch: %u\n", cp->epoch_state.current_epoch);
    printf("  Batches: %u / %u\n", cp->epoch_state.completed_batches, 
           cp->epoch_state.total_batches);
    printf("  Average Loss: %.6f\n", cp->epoch_state.average_loss);
    printf("  Duration: %.2f seconds\n", cp->epoch_state.epoch_duration);
    
    // Processing stats
    printf("\nProcessing Statistics:\n");
    printf("  Primes Processed: %lu\n", cp->epoch_state.primes_processed);
    printf("  Gradients Computed: %lu\n", cp->epoch_state.gradients_computed);
    printf("  Weights Updated: %lu\n", cp->epoch_state.weights_updated);
    
    // Health stats
    printf("\nSystem Health:\n");
    printf("  Active Spheres: %u\n", cp->health.active_spheres);
    printf("  Idle Spheres: %u\n", cp->health.idle_spheres);
    printf("  Failed Spheres: %u\n", cp->health.failed_spheres);
    printf("  CPU Utilization: %.1f%%\n", cp->health.cpu_utilization);
    printf("  Memory Utilization: %.1f%%\n", cp->health.memory_utilization);
    
    printf("\n");
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

ControlProcessState control_process_get_state(const ControlProcess* cp) {
    if (!cp) return CONTROL_STATE_STOPPED;
    return cp->state;
}

const char* control_process_state_to_string(ControlProcessState state) {
    switch (state) {
        case CONTROL_STATE_INITIALIZING: return "INITIALIZING";
        case CONTROL_STATE_RUNNING: return "RUNNING";
        case CONTROL_STATE_PAUSED: return "PAUSED";
        case CONTROL_STATE_STOPPING: return "STOPPING";
        case CONTROL_STATE_STOPPED: return "STOPPED";
        default: return "UNKNOWN";
    }
}

CLLMLatticeHierarchy* control_process_find_sphere(ControlProcess* cp, 
                                                  uint32_t sphere_id) {
    if (!cp || !cp->root_sphere) return NULL;
    
    pthread_mutex_lock(&cp->hierarchy_mutex);
    CLLMLatticeHierarchy* sphere = find_sphere_recursive(cp->root_sphere, sphere_id);
    pthread_mutex_unlock(&cp->hierarchy_mutex);
    
    return sphere;
}

uint32_t control_process_count_spheres(const ControlProcess* cp) {
    if (!cp) return 0;
    return cp->total_sphere_count;
}

bool control_process_validate(const ControlProcess* cp) {
    if (!cp) return false;
    
    // Check basic structure
    if (cp->total_sphere_count == 0 && cp->root_sphere != NULL) return false;
    if (cp->total_sphere_count > 0 && cp->root_sphere == NULL) return false;
    
    // Validate configuration
    if (cp->config.max_hierarchy_depth == 0) return false;
    if (cp->config.max_threads == 0) return false;
    
    // Count spheres and verify
    if (cp->root_sphere) {
        uint32_t counted = count_spheres_recursive(cp->root_sphere);
        if (counted != cp->total_sphere_count) return false;
    }
    
    return true;
}

// ============================================================================
// HELPER FUNCTION IMPLEMENTATIONS
// ============================================================================

static void* health_monitor_thread_func(void* arg) {
    ControlProcess* cp = (ControlProcess*)arg;
    
    while (cp->health_monitor_running) {
        // Update health metrics
        cp->health.last_health_check_time = get_current_time();
        
        pthread_mutex_lock(&cp->hierarchy_mutex);
        
        // Count sphere states
        cp->health.active_spheres = 0;
        cp->health.idle_spheres = 0;
        cp->health.failed_spheres = 0;
        
        // TODO: Traverse hierarchy and count sphere states
        
        pthread_mutex_unlock(&cp->hierarchy_mutex);
        
        // Sleep for health check interval
        usleep(cp->config.health_check_interval_ms * 1000);
    }
    
    return NULL;
}

static bool collect_statistics_recursive(CLLMLatticeHierarchy* sphere, 
                                         SphereStatistics* stats) {
    if (!sphere || !stats) return false;
    
    // Merge this sphere's statistics
    cllm_sphere_stats_merge(stats, &sphere->stats);
    
    // Recursively collect from children
    for (uint32_t i = 0; i < (uint32_t)sphere->num_children; i++) {
        collect_statistics_recursive(sphere->children[i], stats);
    }
    
    return true;
}

static CLLMLatticeHierarchy* find_sphere_recursive(CLLMLatticeHierarchy* sphere, 
                                                     uint32_t sphere_id) {
    if (!sphere) return NULL;
    
    if ((uint32_t)sphere->sphere_id == sphere_id) {
        return sphere;
    }
    
    // Search children
    for (uint32_t i = 0; i < (uint32_t)sphere->num_children; i++) {
        CLLMLatticeHierarchy* found = find_sphere_recursive(sphere->children[i], 
                                                              sphere_id);
        if (found) return found;
    }
    
    return NULL;
}

static uint32_t count_spheres_recursive(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    
    uint32_t count = 1; // Count this sphere
    
    // Count children
    for (uint32_t i = 0; i < (uint32_t)sphere->num_children; i++) {
        count += count_spheres_recursive(sphere->children[i]);
    }
    
    return count;
}
