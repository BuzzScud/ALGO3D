#include "ai/cllm_lattice_hierarchy.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// HIERARCHY CREATION & DESTRUCTION
// ============================================================================

CLLMLatticeHierarchy* lattice_hierarchy_create(
    int sphere_id,
    int hierarchy_level,
    const int* symmetry_groups,
    int num_symmetry_groups,
    int physical_thread_id,
    CLLMLatticeHierarchy* parent
) {
    if (num_symmetry_groups <= 0 || num_symmetry_groups > 12) {
        fprintf(stderr, "ERROR: Invalid number of symmetry groups: %d\n", 
                num_symmetry_groups);
        return NULL;
    }
    
    if (!symmetry_groups) {
        fprintf(stderr, "ERROR: symmetry_groups is NULL\n");
        return NULL;
    }
    
    // Allocate sphere
    CLLMLatticeHierarchy* sphere = calloc(1, sizeof(CLLMLatticeHierarchy));
    if (!sphere) {
        fprintf(stderr, "ERROR: Failed to allocate sphere\n");
        return NULL;
    }
    
    // Set identity
    sphere->sphere_id = sphere_id;
    sphere->hierarchy_level = hierarchy_level;
    sphere->physical_thread_id = physical_thread_id;
    
    // Set symmetry groups
    sphere->num_symmetry_groups = num_symmetry_groups;
    for (int i = 0; i < num_symmetry_groups; i++) {
        if (symmetry_groups[i] < 0 || symmetry_groups[i] >= 12) {
            fprintf(stderr, "ERROR: Invalid symmetry group: %d\n", symmetry_groups[i]);
            free(sphere);
            return NULL;
        }
        sphere->symmetry_groups[i] = symmetry_groups[i];
    }
    sphere->primary_symmetry_group = symmetry_groups[0];
    
    // Set parent
    sphere->parent = parent;
    
    // Initialize children and siblings
    sphere->num_children = 0;
    sphere->num_siblings = 0;
    
    // Initialize state
    atomic_init(&sphere->state, HIERARCHY_STATE_INITIALIZING);
    atomic_init(&sphere->thread_running, 0);
    
    // Initialize mutexes
    pthread_mutex_init(&sphere->state_mutex, NULL);
    pthread_mutex_init(&sphere->children_mutex, NULL);
    pthread_mutex_init(&sphere->gradient_mutex, NULL);
    
    // Initialize condition variables
    pthread_cond_init(&sphere->state_changed, NULL);
    pthread_cond_init(&sphere->work_available, NULL);
    
    // Create message queues
    sphere->inbox = message_queue_create(10000, false);  // 10k message capacity
    sphere->outbox = message_queue_create(10000, false);
    
    if (!sphere->inbox || !sphere->outbox) {
        fprintf(stderr, "ERROR: Failed to create message queues\n");
        lattice_hierarchy_free(sphere);
        return NULL;
    }
    
    // Initialize message counters
    atomic_init(&sphere->messages_sent, 0);
    atomic_init(&sphere->messages_received, 0);
    
    // Initialize work queue
    sphere->work_queue_capacity = 1000;
    sphere->work_queue = calloc(sphere->work_queue_capacity, sizeof(uint64_t));
    if (!sphere->work_queue) {
        fprintf(stderr, "ERROR: Failed to allocate work queue\n");
        lattice_hierarchy_free(sphere);
        return NULL;
    }
    
    atomic_init(&sphere->work_queue_head, 0);
    atomic_init(&sphere->work_queue_tail, 0);
    atomic_init(&sphere->work_queue_size, 0);
    
    // Initialize work stealing
    atomic_init(&sphere->work_stealing_enabled, 1);
    atomic_init(&sphere->work_stolen_from, 0);
    atomic_init(&sphere->work_stolen_to, 0);
    
    // Initialize gradient buffers (will be allocated when needed)
    sphere->gradient_buffer = NULL;
    sphere->gradient_buffer_size = 0;
    atomic_init(&sphere->gradient_ready, 0);
    
    sphere->child_gradients = NULL;
    atomic_init(&sphere->children_gradients_ready, 0);
    
    // Initialize statistics
    cllm_sphere_stats_init(&sphere->stats, sphere->primary_symmetry_group, hierarchy_level);
    
    // Initialize 88D computational space
    // Use base-60 (Babylonian) with precision of 10 digits
    sphere->space = space88d_create(60, 10);
    if (!sphere->space) {
        fprintf(stderr, "ERROR: Failed to create 88D space\n");
        lattice_hierarchy_free(sphere);
        return NULL;
    }
    
    // Initialize boundary awareness
    atomic_init(&sphere->near_144000_boundary, 0);
    atomic_init(&sphere->boundary_crossings, 0);
    atomic_init(&sphere->twin_prime_hits, 0);
    
    // Set default configuration
    sphere->batch_size = 32;
    sphere->enable_work_stealing = 1;
    sphere->enable_recursive_spawning = 0;
    sphere->max_hierarchy_depth = 3;
    
    // Set timestamps
    sphere->creation_time_ns = get_time_ns();
    sphere->start_time_ns = 0;
    sphere->total_processing_time_ns = 0;
    
    // Set debug name
    snprintf(sphere->debug_name, sizeof(sphere->debug_name),
             "Sphere-%d-L%d-G%d-T%d",
             sphere_id, hierarchy_level, sphere->primary_symmetry_group, physical_thread_id);
    sphere->user_data = NULL;  // Initialize to NULL
    
    // Set state to READY
    atomic_store(&sphere->state, HIERARCHY_STATE_READY);
    
    return sphere;
}

void lattice_hierarchy_free(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return;
    
    // Free message queues
    if (sphere->inbox) {
        message_queue_free(sphere->inbox);
    }
    if (sphere->outbox) {
        message_queue_free(sphere->outbox);
    }
    
    // Free work queue
    if (sphere->work_queue) {
        free(sphere->work_queue);
    }
    
    // Free gradient buffers
    if (sphere->gradient_buffer) {
        free(sphere->gradient_buffer);
    }
    if (sphere->child_gradients) {
        free(sphere->child_gradients);
    }
    
    // Free 88D computational space
    if (sphere->space) {
        space88d_free(sphere->space);
    }
    
    // Free shared memory regions
    if (sphere->parent_weights) {
        shared_memory_free(sphere->parent_weights);
    }
    if (sphere->parent_lattice) {
        shared_memory_free(sphere->parent_lattice);
    }
    if (sphere->shared_weights) {
        shared_memory_free(sphere->shared_weights);
    }
    if (sphere->shared_lattice) {
        shared_memory_free(sphere->shared_lattice);
    }
    
    // Free position and partition
    // Free position
    if (sphere->position) {
        free_sphere_position(sphere->position);
    }
    
    // Free partition
    if (sphere->partition) {
        free_lattice_partition(sphere->partition);
    }
    
    // Note: 88D space is already freed above
    // Old abacus field has been replaced with Space88D
    
    // Destroy mutexes and condition variables
    pthread_mutex_destroy(&sphere->state_mutex);
    pthread_mutex_destroy(&sphere->children_mutex);
    pthread_mutex_destroy(&sphere->gradient_mutex);
    pthread_cond_destroy(&sphere->state_changed);
    pthread_cond_destroy(&sphere->work_available);
    
    // Free sphere
    free(sphere);
}

// ============================================================================
// HIERARCHY RELATIONSHIPS
// ============================================================================

int lattice_hierarchy_add_child(CLLMLatticeHierarchy* parent,
                                CLLMLatticeHierarchy* child) {
    if (!parent || !child) return 0;
    
    pthread_mutex_lock(&parent->children_mutex);
    
    // Check if we have room for more children
    if (parent->num_children >= 12) {
        pthread_mutex_unlock(&parent->children_mutex);
        fprintf(stderr, "ERROR: Parent already has 12 children\n");
        return 0;
    }
    
    // Check if child already exists
    for (int i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == child) {
            pthread_mutex_unlock(&parent->children_mutex);
            return 1;  // Already added
        }
    }
    
    // Add child
    parent->children[parent->num_children] = child;
    parent->num_children++;
    
    // Set parent reference in child
    child->parent = parent;
    
    pthread_mutex_unlock(&parent->children_mutex);
    
    return 1;
}

int lattice_hierarchy_remove_child(CLLMLatticeHierarchy* parent,
                                   CLLMLatticeHierarchy* child) {
    if (!parent || !child) return 0;
    
    pthread_mutex_lock(&parent->children_mutex);
    
    // Find and remove child
    int found = 0;
    for (int i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < parent->num_children - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children[parent->num_children - 1] = NULL;
            parent->num_children--;
            found = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&parent->children_mutex);
    
    if (found) {
        child->parent = NULL;
    }
    
    return found;
}

int lattice_hierarchy_add_sibling(CLLMLatticeHierarchy* sphere,
                                  CLLMLatticeHierarchy* sibling) {
    if (!sphere || !sibling) return 0;
    
    // Check if we have room for more siblings
    if (sphere->num_siblings >= 11) {
        fprintf(stderr, "ERROR: Sphere already has 11 siblings\n");
        return 0;
    }
    
    // Check if sibling already exists
    for (int i = 0; i < sphere->num_siblings; i++) {
        if (sphere->siblings[i] == sibling) {
            return 1;  // Already added
        }
    }
    
    // Add sibling
    sphere->siblings[sphere->num_siblings] = sibling;
    sphere->num_siblings++;
    
    return 1;
}

void lattice_hierarchy_discover_siblings(CLLMLatticeHierarchy** spheres,
                                        int num_spheres) {
    if (!spheres || num_spheres <= 0) return;
    
    // Link all spheres at the same level as siblings
    for (int i = 0; i < num_spheres; i++) {
        for (int j = 0; j < num_spheres; j++) {
            if (i != j) {
                lattice_hierarchy_add_sibling(spheres[i], spheres[j]);
            }
        }
    }
}

CLLMLatticeHierarchy* lattice_hierarchy_get_child_for_group(
    CLLMLatticeHierarchy* parent,
    int symmetry_group
) {
    if (!parent || symmetry_group < 0 || symmetry_group >= 12) {
        return NULL;
    }
    
    pthread_mutex_lock(&parent->children_mutex);
    
    // Find child with matching symmetry group
    CLLMLatticeHierarchy* result = NULL;
    for (int i = 0; i < parent->num_children; i++) {
        CLLMLatticeHierarchy* child = parent->children[i];
        for (int j = 0; j < child->num_symmetry_groups; j++) {
            if (child->symmetry_groups[j] == symmetry_group) {
                result = child;
                break;
            }
        }
        if (result) break;
    }
    
    pthread_mutex_unlock(&parent->children_mutex);
    
    return result;
}

CLLMLatticeHierarchy* lattice_hierarchy_get_sibling_for_group(
    CLLMLatticeHierarchy* sphere,
    int symmetry_group
) {
    if (!sphere || symmetry_group < 0 || symmetry_group >= 12) {
        return NULL;
    }
    
    // Find sibling with matching symmetry group
    for (int i = 0; i < sphere->num_siblings; i++) {
        CLLMLatticeHierarchy* sibling = sphere->siblings[i];
        for (int j = 0; j < sibling->num_symmetry_groups; j++) {
            if (sibling->symmetry_groups[j] == symmetry_group) {
                return sibling;
            }
        }
    }
    
    return NULL;
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

HierarchyState lattice_hierarchy_get_state(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) return HIERARCHY_STATE_TERMINATED;
    return (HierarchyState)atomic_load(&sphere->state);
}

void lattice_hierarchy_set_state(CLLMLatticeHierarchy* sphere,
                                HierarchyState new_state) {
    if (!sphere) return;
    
    pthread_mutex_lock(&sphere->state_mutex);
    
    HierarchyState old_state = (HierarchyState)atomic_load(&sphere->state);
    atomic_store(&sphere->state, new_state);
    
    // Signal state change
    pthread_cond_broadcast(&sphere->state_changed);
    
    pthread_mutex_unlock(&sphere->state_mutex);
    
    // Log state transition for debugging
    if (old_state != new_state) {
        // Uncomment for debugging:
        // printf("[%s] State: %s -> %s\n", sphere->debug_name,
        //        lattice_hierarchy_state_name(old_state),
        //        lattice_hierarchy_state_name(new_state));
    }
}

int lattice_hierarchy_wait_for_state(CLLMLatticeHierarchy* sphere,
                                    HierarchyState target_state,
                                    int timeout_ms) {
    if (!sphere) return 0;
    
    pthread_mutex_lock(&sphere->state_mutex);
    
    if (timeout_ms == 0) {
        // No timeout - wait indefinitely
        while ((HierarchyState)atomic_load(&sphere->state) != target_state) {
            pthread_cond_wait(&sphere->state_changed, &sphere->state_mutex);
        }
        pthread_mutex_unlock(&sphere->state_mutex);
        return 1;
    } else {
        // Wait with timeout
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        int result = 1;
        while ((HierarchyState)atomic_load(&sphere->state) != target_state) {
            int rc = pthread_cond_timedwait(&sphere->state_changed, 
                                           &sphere->state_mutex, &ts);
            if (rc == ETIMEDOUT) {
                result = 0;
                break;
            }
        }
        
        pthread_mutex_unlock(&sphere->state_mutex);
        return result;
    }
}

const char* lattice_hierarchy_state_name(HierarchyState state) {
    switch (state) {
        case HIERARCHY_STATE_INITIALIZING: return "INITIALIZING";
        case HIERARCHY_STATE_READY: return "READY";
        case HIERARCHY_STATE_PROCESSING: return "PROCESSING";
        case HIERARCHY_STATE_WAITING: return "WAITING";
        case HIERARCHY_STATE_ACCUMULATING: return "ACCUMULATING";
        case HIERARCHY_STATE_UPDATING: return "UPDATING";
        case HIERARCHY_STATE_IDLE: return "IDLE";
        case HIERARCHY_STATE_TERMINATING: return "TERMINATING";
        case HIERARCHY_STATE_TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

SyncBarrier* sync_barrier_create(int num_spheres) {
    if (num_spheres <= 0) return NULL;
    
    SyncBarrier* barrier = calloc(1, sizeof(SyncBarrier));
    if (!barrier) return NULL;
    
    // Initialize POSIX barrier
    if (pthread_barrier_init(&barrier->barrier, NULL, num_spheres) != 0) {
        free(barrier);
        return NULL;
    }
    
    atomic_init(&barrier->arrived, 0);
    atomic_init(&barrier->required, num_spheres);
    atomic_init(&barrier->generation, 0);
    
    return barrier;
}

void sync_barrier_free(SyncBarrier* barrier) {
    if (!barrier) return;
    
    pthread_barrier_destroy(&barrier->barrier);
    free(barrier);
}

int sync_barrier_wait(SyncBarrier* barrier) {
    if (!barrier) return 0;
    
    // Wait at POSIX barrier
    int rc = pthread_barrier_wait(&barrier->barrier);
    
    // PTHREAD_BARRIER_SERIAL_THREAD is returned to exactly one thread
    if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
        // This thread is the last one - increment generation
        atomic_fetch_add(&barrier->generation, 1);
        return 1;
    } else if (rc == 0) {
        // Normal return for other threads
        return 1;
    } else {
        // Error
        return 0;
    }
}

void sync_barrier_reset(SyncBarrier* barrier) {
    if (!barrier) return;
    
    atomic_store(&barrier->arrived, 0);
    atomic_fetch_add(&barrier->generation, 1);
}

// Continue in next part...
// ============================================================================
// MESSAGE PASSING
// ============================================================================

int lattice_hierarchy_send_message(CLLMLatticeHierarchy* sender,
                                   CLLMLatticeHierarchy* receiver,
                                   SphereMessage* message) {
    if (!sender || !receiver || !message) return 0;
    
    // Set sender ID
    message->sender_id = sender->sphere_id;
    message->receiver_id = receiver->sphere_id;
    
    // Enqueue to receiver's inbox
    if (!message_queue_enqueue(receiver->inbox, message)) {
        fprintf(stderr, "ERROR: Failed to enqueue message to receiver %d\n",
                receiver->sphere_id);
        return 0;
    }
    
    // Track statistics
    atomic_fetch_add(&sender->messages_sent, 1);
    atomic_fetch_add(&receiver->messages_received, 1);
    
    // Also add to sender's outbox for debugging
    SphereMessage* copy = sphere_message_clone(message);
    if (copy) {
        message_queue_enqueue(sender->outbox, copy);
    }
    
    return 1;
}

int lattice_hierarchy_broadcast_to_siblings(CLLMLatticeHierarchy* sender,
                                           SphereMessage* message) {
    if (!sender || !message) return 0;
    
    int sent = 0;
    
    for (int i = 0; i < sender->num_siblings; i++) {
        CLLMLatticeHierarchy* sibling = sender->siblings[i];
        if (sibling) {
            SphereMessage* copy = sphere_message_clone(message);
            if (copy && lattice_hierarchy_send_message(sender, sibling, copy)) {
                sent++;
            }
        }
    }
    
    return sent;
}

int lattice_hierarchy_broadcast_to_children(CLLMLatticeHierarchy* parent,
                                           SphereMessage* message) {
    if (!parent || !message) return 0;
    
    pthread_mutex_lock(&parent->children_mutex);
    
    int sent = 0;
    
    for (int i = 0; i < parent->num_children; i++) {
        CLLMLatticeHierarchy* child = parent->children[i];
        if (child) {
            SphereMessage* copy = sphere_message_clone(message);
            if (copy && lattice_hierarchy_send_message(parent, child, copy)) {
                sent++;
            }
        }
    }
    
    pthread_mutex_unlock(&parent->children_mutex);
    
    return sent;
}

int lattice_hierarchy_process_messages(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    
    int processed = 0;
    
    // Process up to 100 messages at a time
    for (int i = 0; i < 100; i++) {
        SphereMessage* message = message_queue_dequeue(sphere->inbox);
        if (!message) break;
        
        // Process message based on type
        switch (message->type) {
            case MSG_TYPE_WORK_REQUEST:
                // Handle work request
                // TODO: Implement work stealing response
                break;
                
            case MSG_TYPE_WORK_ASSIGN:
                // Handle work offer
                // TODO: Implement work acceptance
                break;
                
            case MSG_CLLM_GRADIENT_READY:
                // Child has gradients ready
                atomic_fetch_add(&sphere->children_gradients_ready, 1);
                break;
                
            case MSG_CLLM_WEIGHTS_UPDATED:
                // Parent has updated weights
                // TODO: Implement weight synchronization
                break;
                
            case MSG_CLLM_BOUNDARY_CROSSING:
                // Boundary crossing notification
                atomic_fetch_add(&sphere->boundary_crossings, 1);
                break;
                
            case MSG_CLLM_TWIN_PRIME_HIT:
                // Twin prime hit notification
                atomic_fetch_add(&sphere->twin_prime_hits, 1);
                break;
                
            case MSG_CLLM_EPOCH_START:
                // Start new epoch
                lattice_hierarchy_set_state(sphere, HIERARCHY_STATE_PROCESSING);
                break;
                
            case MSG_CLLM_EPOCH_COMPLETE:
                // Epoch complete
                lattice_hierarchy_set_state(sphere, HIERARCHY_STATE_WAITING);
                break;
                
            case MSG_TYPE_SHUTDOWN:
                // Shutdown request
                lattice_hierarchy_set_state(sphere, HIERARCHY_STATE_TERMINATING);
                break;
                
            default:
                // Unknown message type
                break;
        }
        
        // Mark as processed
        sphere_message_mark_processed(message);
        sphere_message_free(message);
        
        processed++;
    }
    
    return processed;
}

// ============================================================================
// WORK MANAGEMENT
// ============================================================================

int lattice_hierarchy_add_work(CLLMLatticeHierarchy* sphere,
                              uint64_t work_item) {
    if (!sphere) return 0;
    
    size_t tail = atomic_load(&sphere->work_queue_tail);
    size_t size = atomic_load(&sphere->work_queue_size);
    
    // Check if queue is full
    if (size >= sphere->work_queue_capacity) {
        return 0;
    }
    
    // Add work item
    sphere->work_queue[tail] = work_item;
    
    // Update tail
    size_t new_tail = (tail + 1) % sphere->work_queue_capacity;
    atomic_store(&sphere->work_queue_tail, new_tail);
    atomic_fetch_add(&sphere->work_queue_size, 1);
    
    // Signal work available
    pthread_cond_signal(&sphere->work_available);
    
    return 1;
}

int lattice_hierarchy_get_work(CLLMLatticeHierarchy* sphere,
                              uint64_t* work_item) {
    if (!sphere || !work_item) return 0;
    
    size_t size = atomic_load(&sphere->work_queue_size);
    
    // Check if queue is empty
    if (size == 0) {
        return 0;
    }
    
    size_t head = atomic_load(&sphere->work_queue_head);
    
    // Get work item
    *work_item = sphere->work_queue[head];
    
    // Update head
    size_t new_head = (head + 1) % sphere->work_queue_capacity;
    atomic_store(&sphere->work_queue_head, new_head);
    atomic_fetch_sub(&sphere->work_queue_size, 1);
    
    return 1;
}

int lattice_hierarchy_steal_work(CLLMLatticeHierarchy* thief,
                                CLLMLatticeHierarchy* victim,
                                uint64_t* work_item) {
    if (!thief || !victim || !work_item) return 0;
    
    // Check if work stealing is enabled
    if (!atomic_load(&victim->work_stealing_enabled)) {
        return 0;
    }
    
    // Try to steal work from victim
    if (lattice_hierarchy_get_work(victim, work_item)) {
        // Successfully stole work
        atomic_fetch_add(&thief->work_stolen_to, 1);
        atomic_fetch_add(&victim->work_stolen_from, 1);
        
        // Update statistics
        cllm_sphere_stats_record_work_stealing(&thief->stats, 0, 1);
        cllm_sphere_stats_record_work_stealing(&victim->stats, 1, 1);
        
        return 1;
    }
    
    return 0;
}

size_t lattice_hierarchy_work_queue_size(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    return atomic_load(&sphere->work_queue_size);
}

// ============================================================================
// BOUNDARY AWARENESS
// ============================================================================

int lattice_hierarchy_is_near_boundary(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    return atomic_load(&sphere->near_144000_boundary);
}

void lattice_hierarchy_notify_boundary_crossing(CLLMLatticeHierarchy* sphere,
                                               uint64_t prime) {
    if (!sphere) return;
    
    // Update boundary status
    atomic_store(&sphere->near_144000_boundary, 1);
    atomic_fetch_add(&sphere->boundary_crossings, 1);
    
    // Update statistics
    cllm_sphere_stats_record_boundary_crossing(&sphere->stats, prime);
    
    // Notify parent
    if (sphere->parent) {
        SphereMessage* message = sphere_message_create(
            MSG_CLLM_BOUNDARY_CROSSING,
            MSG_PRIORITY_HIGH,
            sphere->sphere_id,
            sphere->parent->sphere_id
        );
        
        if (message) {
            sphere_message_set_boundary(message, prime, 
                                       sphere->primary_symmetry_group,
                                       0.0, 0);
            lattice_hierarchy_send_message(sphere, sphere->parent, message);
        }
    }
    
    // Notify siblings
    SphereMessage* broadcast = sphere_message_create(
        MSG_CLLM_BOUNDARY_CROSSING,
        MSG_PRIORITY_HIGH,
        sphere->sphere_id,
        -1  // Broadcast
    );
    
    if (broadcast) {
        sphere_message_set_boundary(broadcast, prime,
                                   sphere->primary_symmetry_group,
                                   0.0, 0);
        lattice_hierarchy_broadcast_to_siblings(sphere, broadcast);
        sphere_message_free(broadcast);
    }
}

void lattice_hierarchy_notify_twin_prime(CLLMLatticeHierarchy* sphere,
                                        uint64_t prime) {
    if (!sphere) return;
    
    // Update twin prime counter
    atomic_fetch_add(&sphere->twin_prime_hits, 1);
    
    // Notify parent
    if (sphere->parent) {
        SphereMessage* message = sphere_message_create(
            MSG_CLLM_TWIN_PRIME_HIT,
            MSG_PRIORITY_CRITICAL,
            sphere->sphere_id,
            sphere->parent->sphere_id
        );
        
        if (message) {
            sphere_message_set_boundary(message, prime,
                                       sphere->primary_symmetry_group,
                                       0.0, 1);
            lattice_hierarchy_send_message(sphere, sphere->parent, message);
        }
    }
    
    // Notify siblings
    SphereMessage* broadcast = sphere_message_create(
        MSG_CLLM_TWIN_PRIME_HIT,
        MSG_PRIORITY_CRITICAL,
        sphere->sphere_id,
        -1  // Broadcast
    );
    
    if (broadcast) {
        sphere_message_set_boundary(broadcast, prime,
                                   sphere->primary_symmetry_group,
                                   0.0, 1);
        lattice_hierarchy_broadcast_to_siblings(sphere, broadcast);
        sphere_message_free(broadcast);
    }
}

// ============================================================================
// UTILITIES
// ============================================================================

void lattice_hierarchy_print(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) {
        printf("NULL sphere\n");
        return;
    }
    
    printf("=== Lattice Hierarchy Sphere ===\n");
    printf("ID: %d (%s)\n", sphere->sphere_id, sphere->debug_name);
    printf("Level: %d\n", sphere->hierarchy_level);
    printf("State: %s\n", lattice_hierarchy_state_name(
        lattice_hierarchy_get_state(sphere)));
    
    printf("Symmetry Groups: ");
    for (int i = 0; i < sphere->num_symmetry_groups; i++) {
        printf("%d", sphere->symmetry_groups[i]);
        if (i < sphere->num_symmetry_groups - 1) printf(", ");
    }
    printf("\n");
    
    printf("Physical Thread: %d\n", sphere->physical_thread_id);
    printf("Parent: %s\n", sphere->parent ? sphere->parent->debug_name : "None");
    printf("Children: %d\n", sphere->num_children);
    printf("Siblings: %d\n", sphere->num_siblings);
    
    printf("Work Queue: %zu/%zu\n", 
           lattice_hierarchy_work_queue_size(sphere),
           sphere->work_queue_capacity);
    
    printf("Messages: Sent=%lu, Received=%lu\n",
           (unsigned long)atomic_load(&sphere->messages_sent),
           (unsigned long)atomic_load(&sphere->messages_received));
    
    if (lattice_hierarchy_is_near_boundary(sphere)) {
        printf("⚠ Near 144000 boundary\n");
    }
    
    printf("Boundary Crossings: %lu\n",
           (unsigned long)atomic_load(&sphere->boundary_crossings));
    printf("Twin Prime Hits: %lu\n",
           (unsigned long)atomic_load(&sphere->twin_prime_hits));
    
    printf("================================\n");
}

void lattice_hierarchy_print_detailed(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) {
        printf("NULL sphere\n");
        return;
    }
    
    lattice_hierarchy_print(sphere);
    
    printf("\n=== Detailed Information ===\n");
    
    // Print statistics
    cllm_sphere_stats_print(&sphere->stats, sphere->sphere_id);
    
    // Print message queue statistics
    printf("\nInbox Statistics:\n");
    message_queue_print_statistics(sphere->inbox);
    
    printf("\nOutbox Statistics:\n");
    message_queue_print_statistics(sphere->outbox);
}

void lattice_hierarchy_print_tree(const CLLMLatticeHierarchy* sphere,
                                 int indent) {
    if (!sphere) return;
    
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    // Print sphere info
    printf("├─ Sphere %d (L%d, G%d, T%d) [%s]\n",
           sphere->sphere_id,
           sphere->hierarchy_level,
           sphere->primary_symmetry_group,
           sphere->physical_thread_id,
           lattice_hierarchy_state_name(lattice_hierarchy_get_state(sphere)));
    
    // Print children recursively
    pthread_mutex_lock((pthread_mutex_t*)&sphere->children_mutex);
    for (int i = 0; i < sphere->num_children; i++) {
        lattice_hierarchy_print_tree(sphere->children[i], indent + 1);
    }
    pthread_mutex_unlock((pthread_mutex_t*)&sphere->children_mutex);
}

int lattice_hierarchy_validate(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    
    int valid = 1;
    
    // Check symmetry groups
    if (sphere->num_symmetry_groups <= 0 || sphere->num_symmetry_groups > 12) {
        fprintf(stderr, "ERROR: Invalid num_symmetry_groups: %d\n",
                sphere->num_symmetry_groups);
        valid = 0;
    }
    
    for (int i = 0; i < sphere->num_symmetry_groups; i++) {
        if (sphere->symmetry_groups[i] < 0 || sphere->symmetry_groups[i] >= 12) {
            fprintf(stderr, "ERROR: Invalid symmetry group: %d\n",
                    sphere->symmetry_groups[i]);
            valid = 0;
        }
    }
    
    // Check children count
    if (sphere->num_children < 0 || sphere->num_children > 12) {
        fprintf(stderr, "ERROR: Invalid num_children: %d\n",
                sphere->num_children);
        valid = 0;
    }
    
    // Check siblings count
    if (sphere->num_siblings < 0 || sphere->num_siblings > 11) {
        fprintf(stderr, "ERROR: Invalid num_siblings: %d\n",
                sphere->num_siblings);
        valid = 0;
    }
    
    // Check message queues
    if (!sphere->inbox || !sphere->outbox) {
        fprintf(stderr, "ERROR: Message queues not initialized\n");
        valid = 0;
    }
    
    // Check work queue
    if (!sphere->work_queue) {
        fprintf(stderr, "ERROR: Work queue not initialized\n");
        valid = 0;
    }
    
    return valid;
}

int lattice_hierarchy_get_depth(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    
    int max_depth = 0;
    
    pthread_mutex_lock((pthread_mutex_t*)&sphere->children_mutex);
    
    for (int i = 0; i < sphere->num_children; i++) {
        int child_depth = lattice_hierarchy_get_depth(sphere->children[i]);
        if (child_depth > max_depth) {
            max_depth = child_depth;
        }
    }
    
    pthread_mutex_unlock((pthread_mutex_t*)&sphere->children_mutex);
    
    return max_depth + 1;
}

int lattice_hierarchy_count_spheres(const CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    
    int count = 1;  // Count this sphere
    
    pthread_mutex_lock((pthread_mutex_t*)&sphere->children_mutex);
    
    for (int i = 0; i < sphere->num_children; i++) {
        count += lattice_hierarchy_count_spheres(sphere->children[i]);
    }
    
    pthread_mutex_unlock((pthread_mutex_t*)&sphere->children_mutex);
    
    return count;
}

// ============================================================================
// DEPTH MANAGEMENT
// ============================================================================

int calculate_max_depth(int available_cores) {
    if (available_cores <= 0) {
        return 1;  // Minimum depth
    }
    
    // Calculate depth using logarithm base 12
    // max_depth = floor(log12(cores)) + 1
    // 
    // We use the change of base formula: log12(x) = math_log(x) / math_log(12)
    // Since we don't have math.h, we'll use a simple iterative approach
    
    // Calculate how many levels we can support
    // Level 0: 1 thread (root) - depth 1
    // Level 1: 12 threads - depth 2
    // Level 2: 144 threads - depth 3
    // Level 3: 1,728 threads - depth 4
    // Level 4: 20,736 threads - depth 5
    // Level 5: 248,832 threads (exceeds 144,000 limit)
    
    // We need at least 12 cores to spawn children (depth 2)
    if (available_cores < 12) {
        return 1;
    }
    
    // Check each depth level
    if (available_cores >= 20736 && 20736 <= 144000) {
        return 5;  // Can support depth 5 (20,736 threads at level 4)
    }
    if (available_cores >= 1728 && 1728 <= 144000) {
        return 4;  // Can support depth 4 (1,728 threads at level 3)
    }
    if (available_cores >= 144 && 144 <= 144000) {
        return 3;  // Can support depth 3 (144 threads at level 2)
    }
    if (available_cores >= 12 && 12 <= 144000) {
        return 2;  // Can support depth 2 (12 threads at level 1)
    }
    
    return 1;  // Only root thread
}

int can_spawn_at_depth(int current_depth, int available_cores, int current_thread_count) {
    // Check if we're at maximum depth
    if (current_depth >= 5) {  // MAX_HIERARCHY_DEPTH = 5
        return 0;
    }
    
    // Check if we have enough cores for 12 children
    if (available_cores < 12) {
        return 0;
    }
    
    // Check if spawning 12 children would exceed the 144,000 thread limit
    if (current_thread_count + 12 > 144000) {
        return 0;
    }
    
    // Calculate the maximum depth we can support with available cores
    int max_depth = calculate_max_depth(available_cores);
    
    // Check if current depth is below the calculated maximum
    if (current_depth >= max_depth) {
        return 0;
    }
    
    // All checks passed - spawning is allowed
    return 1;
}

int get_recommended_children_count(int current_depth, int available_cores, int pending_batches) {
    // If we can't spawn at this depth, return 0
    if (!can_spawn_at_depth(current_depth, available_cores, 0)) {
        return 0;
    }
    
    // Calculate the ideal number of children based on workload
    // We want at least 10 batches per child thread (MIN_BATCHES_PER_THREAD)
    int ideal_children = pending_batches / 10;
    
    // Clamp to valid range [1, 12]
    if (ideal_children < 1) {
        ideal_children = 1;
    }
    if (ideal_children > 12) {
        ideal_children = 12;
    }
    
    // Adjust based on available cores
    // If we have fewer cores than ideal children, reduce the count
    if (available_cores < ideal_children) {
        ideal_children = available_cores;
    }
    
    // Ensure we still have at least 1 child if spawning is allowed
    if (ideal_children < 1) {
        ideal_children = 1;
    }
    
    // Ensure we don't exceed 12 (12-fold symmetry)
    if (ideal_children > 12) {
        ideal_children = 12;
    }
    
    return ideal_children;
}
