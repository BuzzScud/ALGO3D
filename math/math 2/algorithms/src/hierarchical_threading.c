/**
 * @file hierarchical_threading.c
 * @brief Unified Hierarchical Threading System Implementation
 */

#define _USE_MATH_DEFINES
#include "hierarchical_threading.h"
#include "generic_model.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

/**
 * Get current time in nanoseconds
 */
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * Calculate Euclidean distance between two positions
 */
static double calculate_distance(const double* pos1, const double* pos2, uint32_t dims) {
    double sum = 0.0;
    for (uint32_t i = 0; i < dims; i++) {
        double diff = pos1[i] - pos2[i];
        sum += diff * diff;
    }
    return math_sqrt(sum);
}

// ============================================================================
// THREAD POOL OPERATIONS
// ============================================================================

HierarchicalThreadPool* hierarchical_thread_pool_create(
    uint32_t num_threads,
    uint32_t symmetry_fold,
    uint32_t num_dimensions,
    bool numa_aware
) {
    if (num_threads == 0 || symmetry_fold == 0 || num_dimensions == 0) {
        return NULL;
    }
    
    HierarchicalThreadPool* pool = calloc(1, sizeof(HierarchicalThreadPool));
    if (!pool) {
        return NULL;
    }
    
    // Initialize basic fields
    pool->num_threads = 0;
    pool->max_threads = num_threads;
    pool->symmetry_fold = symmetry_fold;
    pool->num_dimensions = num_dimensions;
    pool->numa_aware = numa_aware;
    pool->initialized = false;
    pool->running = false;
    
    // Allocate thread array
    pool->threads = calloc(num_threads, sizeof(HierarchicalThread*));
    if (!pool->threads) {
        free(pool);
        return NULL;
    }
    
    // Create global hierarchical memory
    pool->global_memory = hierarchical_memory_create(
        num_threads * 1024 * 1024,  // 1MB per thread
        symmetry_fold,              // initial segments
        num_dimensions,             // initial dimensions
        0,                          // owner_id (root)
        0                           // hierarchy_level (root)
    );
    if (!pool->global_memory) {
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    // Create work distributor
    pool->work_distributor = work_distributor_create(num_threads, 1000);  // 1000 work items in pool
    if (!pool->work_distributor) {
        hierarchical_memory_destroy(pool->global_memory);
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    // Create state manager
    pool->state_manager = state_manager_create(num_threads, 100);  // 100 max states
    if (!pool->state_manager) {
        work_distributor_destroy(pool->work_distributor);
        hierarchical_memory_destroy(pool->global_memory);
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&pool->pool_mutex, NULL) != 0) {
        state_manager_destroy(pool->state_manager);
        work_distributor_destroy(pool->work_distributor);
        hierarchical_memory_destroy(pool->global_memory);
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    pool->initialized = true;
    return pool;
}

void hierarchical_thread_pool_free(HierarchicalThreadPool* pool) {
    fprintf(stderr, "DEBUG: hierarchical_thread_pool_free ENTERED, pool=%p\n", (void*)pool);
    fflush(stderr);
    
    if (!pool) {
        fprintf(stderr, "DEBUG: pool is NULL, returning\n");
        fflush(stderr);
        return;
    }
    
    fprintf(stderr, "DEBUG: pool->running=%d\n", pool->running);
    fflush(stderr);
    
    // Stop pool if running
    if (pool->running) {
        fprintf(stderr, "DEBUG: Calling hierarchical_thread_pool_stop...\n");
        fflush(stderr);
        hierarchical_thread_pool_stop(pool);
        fprintf(stderr, "DEBUG: hierarchical_thread_pool_stop returned\n");
        fflush(stderr);
    }
    
    // Free all threads
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        if (pool->threads[i]) {
            hierarchical_thread_free(pool->threads[i]);
        }
    }
    
    // Free shared components
    if (pool->state_manager) {
        state_manager_destroy(pool->state_manager);
    }
    if (pool->work_distributor) {
        work_distributor_destroy(pool->work_distributor);
    }
    if (pool->global_memory) {
        hierarchical_memory_destroy(pool->global_memory);
    }
    
    // Free thread array
    free(pool->threads);
    
    // Destroy mutex
    pthread_mutex_destroy(&pool->pool_mutex);
    
    free(pool);
}

int hierarchical_thread_pool_start(HierarchicalThreadPool* pool) {
    if (!pool || !pool->initialized) {
        return -1;
    }
    
    pthread_mutex_lock(&pool->pool_mutex);
    
    if (pool->running) {
        pthread_mutex_unlock(&pool->pool_mutex);
        return 0;  // Already running
    }
    
    // Start all threads
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        HierarchicalThread* thread = pool->threads[i];
        if (thread && !thread->running) {
            // Change state to READY
            hierarchical_thread_change_state(thread, STATE_READY);
            thread->running = true;
        }
    }
    
    pool->running = true;
    pthread_mutex_unlock(&pool->pool_mutex);
    
    return 0;
}

int hierarchical_thread_pool_stop(HierarchicalThreadPool* pool) {
    if (!pool || !pool->initialized) {
        return -1;
    }
    
    pthread_mutex_lock(&pool->pool_mutex);
    
    if (!pool->running) {
        pthread_mutex_unlock(&pool->pool_mutex);
        return 0;  // Already stopped
    }
    
    // Signal all threads to stop
    int threads_stopped = 0;
    fprintf(stderr, "DEBUG: Stopping %u threads...\n", pool->num_threads);
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        HierarchicalThread* thread = pool->threads[i];
        if (thread) {
            fprintf(stderr, "DEBUG: Thread %u - running=%d\n", i, thread->running);
            if (thread->running) {
                // Lock thread's control mutex before modifying state
                pthread_mutex_lock(&thread->control_mutex);
                
                thread->should_stop = true;
                thread->running = false;  // Set running to false so worker loop exits
                
                // Wake up thread if it's waiting
                pthread_cond_signal(&thread->control_cond);
                
                pthread_mutex_unlock(&thread->control_mutex);
                
                hierarchical_thread_change_state(thread, STATE_STOPPING);
                threads_stopped++;
                fprintf(stderr, "DEBUG: Signaled thread %u to stop\n", i);
            }
        }
    }
    fprintf(stderr, "DEBUG: Signaled %d threads to stop\n", threads_stopped);
    
    pthread_mutex_unlock(&pool->pool_mutex);
    
    // Wait for all threads to stop
    return hierarchical_thread_pool_wait(pool);
}

int hierarchical_thread_pool_wait(HierarchicalThreadPool* pool) {
    if (!pool || !pool->initialized) {
        return -1;
    }
    
    fprintf(stderr, "DEBUG: Waiting for %u threads to join...\n", pool->num_threads);
    
    // Join all threads (don't check running flag - threads may have already exited)
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        HierarchicalThread* thread = pool->threads[i];
        if (thread) {
            fprintf(stderr, "DEBUG: Joining thread %u...\n", i);
            hierarchical_thread_join(thread);
            fprintf(stderr, "DEBUG: Thread %u joined\n", i);
        }
    }
    
    pthread_mutex_lock(&pool->pool_mutex);
    pool->running = false;
    pthread_mutex_unlock(&pool->pool_mutex);
    
    fprintf(stderr, "DEBUG: All threads joined successfully\n");
    
    return 0;
}

// ============================================================================
// THREAD OPERATIONS
// ============================================================================

HierarchicalThread* hierarchical_thread_create(
    uint32_t thread_id,
    ThreadRole role,
    HierarchicalThread* parent,
    HierarchicalThreadPool* pool
) {
    if (!pool) {
        return NULL;
    }
    
    HierarchicalThread* thread = calloc(1, sizeof(HierarchicalThread));
    if (!thread) {
        return NULL;
    }
    
    // Initialize identity
    thread->thread_id = thread_id;
    thread->role = role;
    thread->parent = parent;
    thread->running = false;
    thread->should_stop = false;
    
    // Allocate children array
    thread->max_children = HIERARCHICAL_THREAD_MAX_CHILDREN;
    thread->children = calloc(thread->max_children, sizeof(HierarchicalThread*));
    if (!thread->children) {
        free(thread);
        return NULL;
    }
    thread->num_children = 0;
    
    // Initialize neighbors
    thread->num_neighbors = 0;
    memset(thread->neighbors, 0, sizeof(thread->neighbors));
    
    // Allocate geometric position
    thread->num_dimensions = pool->num_dimensions;
    thread->position = calloc(thread->num_dimensions, sizeof(double));
    if (!thread->position) {
        free(thread->children);
        free(thread);
        return NULL;
    }
    
    // Calculate position based on thread ID and symmetry
    thread->symmetry_group = thread_id % pool->symmetry_fold;
    for (uint32_t i = 0; i < thread->num_dimensions; i++) {
        // Simple geometric distribution
        double angle = 2.0 * M_PI * thread->symmetry_group / pool->symmetry_fold;
        thread->position[i] = math_cos(angle + i * M_PI / thread->num_dimensions);
    }
    
    // Create hierarchical memory segment
    thread->memory_segment_id = thread_id;
    thread->memory = pool->global_memory;  // Share global memory
    
    // Initialize message queues (simplified - would use MessageSystem in real version)
    thread->inbox = NULL;
    thread->outbox = NULL;
    
    // Create state machine
    thread->state_machine = state_machine_create(
        pool->state_manager,
        thread_id,
        STATE_UNINITIALIZED,
        100,  // max transitions
        10    // max callbacks
    );
    if (!thread->state_machine) {
        free(thread->position);
        free(thread->children);
        free(thread);
        return NULL;
    }
    thread->current_state = STATE_UNINITIALIZED;
    
    // Create work pool (simplified - would access from WorkDistributor in real version)
    thread->work_pool = NULL;
    
    // Create local shared memory
    thread->local_shared = shared_memory_enhanced_create(
        4096,  // 4KB initial size
        SHARED_LOCKED_WRITE,
        thread_id
    );
    
    // Initialize mutexes
    pthread_mutex_init(&thread->control_mutex, NULL);
    pthread_cond_init(&thread->control_cond, NULL);
    
    // Initialize statistics
    thread->start_time = get_time_ns();
    thread->messages_sent = 0;
    thread->messages_received = 0;
    thread->work_completed = 0;
    thread->work_stolen = 0;
    thread->context_switches = 0;
    
    // Add to parent's children if parent exists
    if (parent) {
        if (parent->num_children < parent->max_children) {
            parent->children[parent->num_children++] = thread;
            
            // Create shared memory with parent
            thread->parent_shared = shared_memory_enhanced_create(
                4096,
                SHARED_COPY_ON_WRITE,
                thread_id * 1000 + parent->thread_id
            );
        }
    }
    
    // Change state to INITIALIZED
    hierarchical_thread_change_state(thread, STATE_INITIALIZED);
    
    return thread;
}

void hierarchical_thread_free(HierarchicalThread* thread) {
    if (!thread) {
        return;
    }
    
    // Stop thread if running
    if (thread->running) {
        hierarchical_thread_stop(thread);
        hierarchical_thread_join(thread);
    }
    
    // Free message queues (simplified - would use MessageSystem in real version)
    // Nothing to free in simplified version
    
    // Free shared memory
    if (thread->local_shared) {
        shared_memory_enhanced_free(thread->local_shared);
    }
    if (thread->parent_shared) {
        shared_memory_enhanced_free(thread->parent_shared);
    }
    if (thread->child_shared) {
        for (uint32_t i = 0; i < thread->num_children; i++) {
            if (thread->child_shared[i]) {
                shared_memory_enhanced_free(thread->child_shared[i]);
            }
        }
        free(thread->child_shared);
    }
    
    // Free neighbor boundaries
    for (uint32_t i = 0; i < thread->num_neighbors; i++) {
        if (thread->neighbors[i].boundary) {
            shared_memory_enhanced_free(thread->neighbors[i].boundary);
        }
    }
    
    // Free position
    free(thread->position);
    
    // Free children array
    free(thread->children);
    
    // Clean up work queue
    pthread_mutex_lock(&thread->work_queue_mutex);
    TrainingWorkItem* item = thread->work_queue_head;
    while (item) {
        TrainingWorkItem* next = item->next;
        free(item);
        item = next;
    }
    pthread_mutex_unlock(&thread->work_queue_mutex);
    
    // Destroy mutexes
    pthread_mutex_destroy(&thread->control_mutex);
    pthread_cond_destroy(&thread->control_cond);
    pthread_mutex_destroy(&thread->work_queue_mutex);
    
    free(thread);
}

/**
 * Thread work function wrapper
 */
typedef struct {
    HierarchicalThread* thread;
    void* (*work_fn)(void*);
    void* work_data;
} ThreadWorkContext;

static void* thread_work_wrapper(void* arg) {
    ThreadWorkContext* ctx = (ThreadWorkContext*)arg;
    HierarchicalThread* thread = ctx->thread;
    void* (*work_fn)(void*) = ctx->work_fn;
    void* work_data = ctx->work_data;
    
    free(ctx);
    
    // Change state to RUNNING
    hierarchical_thread_change_state(thread, STATE_RUNNING);
    
    // Execute work function
    void* result = NULL;
    if (work_fn) {
        result = work_fn(work_data);
    }
    
    // Change state to STOPPED
    hierarchical_thread_change_state(thread, STATE_STOPPED);
    
    return result;
}

int hierarchical_thread_start(
    HierarchicalThread* thread,
    void* (*work_fn)(void*),
    void* work_data
) {
    if (!thread || thread->running) {
        return -1;
    }
    
    // Create work context
    ThreadWorkContext* ctx = malloc(sizeof(ThreadWorkContext));
    if (!ctx) {
        return -1;
    }
    ctx->thread = thread;
    ctx->work_fn = work_fn;
    ctx->work_data = work_data;
    
    // Create pthread
    int result = pthread_create(&thread->pthread, NULL, thread_work_wrapper, ctx);
    if (result != 0) {
        free(ctx);
        return -1;
    }
    
    thread->running = true;
    thread->start_time = get_time_ns();
    
    return 0;
}

int hierarchical_thread_stop(HierarchicalThread* thread) {
    if (!thread || !thread->running) {
        return -1;
    }
    
    pthread_mutex_lock(&thread->control_mutex);
    thread->should_stop = true;
    pthread_cond_signal(&thread->control_cond);
    pthread_mutex_unlock(&thread->control_mutex);
    
    return 0;
}

int hierarchical_thread_join(HierarchicalThread* thread) {
    if (!thread || !thread->running) {
        return -1;
    }
    
    void* result;
    int ret = pthread_join(thread->pthread, &result);
    if (ret == 0) {
        thread->running = false;
        thread->total_runtime = get_time_ns() - thread->start_time;
    }
    
    return ret;
}

// ============================================================================
// NEIGHBOR OPERATIONS
// ============================================================================

int hierarchical_thread_add_neighbor(
    HierarchicalThread* thread,
    uint32_t neighbor_id,
    ThreadRelationType relationship,
    double distance,
    HierarchicalThreadPool* pool
) {
    if (!thread || !pool || thread->num_neighbors >= HIERARCHICAL_THREAD_MAX_NEIGHBORS) {
        return -1;
    }
    
    // Check if neighbor already exists
    for (uint32_t i = 0; i < thread->num_neighbors; i++) {
        if (thread->neighbors[i].thread_id == neighbor_id) {
            return 0;  // Already exists
        }
    }
    
    // Create shared boundary memory
    SharedMemoryEnhanced* boundary = shared_memory_enhanced_create(
        4096,  // 4KB boundary
        SHARED_COPY_ON_WRITE,
        thread->thread_id * 10000 + neighbor_id
    );
    if (!boundary) {
        return -1;
    }
    
    // Add neighbor
    ThreadNeighbor* neighbor = &thread->neighbors[thread->num_neighbors++];
    neighbor->thread_id = neighbor_id;
    neighbor->relationship = relationship;
    neighbor->distance = distance;
    neighbor->boundary = boundary;
    
    return 0;
}

ThreadNeighbor* hierarchical_thread_get_neighbor(
    HierarchicalThread* thread,
    uint32_t neighbor_id
) {
    if (!thread) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < thread->num_neighbors; i++) {
        if (thread->neighbors[i].thread_id == neighbor_id) {
            return &thread->neighbors[i];
        }
    }
    
    return NULL;
}

uint32_t hierarchical_thread_get_neighbors_by_type(
    HierarchicalThread* thread,
    ThreadRelationType relationship,
    ThreadNeighbor* out_neighbors,
    uint32_t max_neighbors
) {
    if (!thread || !out_neighbors) {
        return 0;
    }
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < thread->num_neighbors && count < max_neighbors; i++) {
        if (thread->neighbors[i].relationship == relationship) {
            out_neighbors[count++] = thread->neighbors[i];
        }
    }
    
    return count;
}

SharedMemoryEnhanced* hierarchical_thread_get_boundary(
    HierarchicalThread* thread,
    uint32_t neighbor_id
) {
    ThreadNeighbor* neighbor = hierarchical_thread_get_neighbor(thread, neighbor_id);
    return neighbor ? neighbor->boundary : NULL;
}

// ============================================================================
// MESSAGE OPERATIONS
// ============================================================================

int hierarchical_thread_send_message(
    HierarchicalThread* sender,
    uint32_t receiver_id,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size,
    HierarchicalThreadPool* pool
) {
    if (!sender || !pool) {
        return -1;
    }
    
    // Simplified implementation - would need MessageSystem in real version
    (void)receiver_id;
    (void)type;
    (void)priority;
    (void)data;
    (void)data_size;
    
    sender->messages_sent++;
    pool->total_messages++;
    
    return 0;
}

Message* hierarchical_thread_receive_message(
    HierarchicalThread* thread,
    int timeout_ms
) {
    if (!thread) {
        return NULL;
    }
    
    // Simplified implementation - would need MessageSystem in real version
    (void)timeout_ms;
    
    return NULL;
}

int hierarchical_thread_broadcast_message(
    HierarchicalThread* sender,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size,
    HierarchicalThreadPool* pool
) {
    if (!sender || !pool) {
        return -1;
    }
    
    // Simplified implementation - would need MessageSystem in real version
    (void)type;
    (void)priority;
    (void)data;
    (void)data_size;
    
    int sent = pool->num_threads - 1;  // All except sender
    sender->messages_sent += sent;
    pool->total_messages += sent;
    
    return sent;
}

// ============================================================================
// STATE OPERATIONS
// ============================================================================

TransitionResult hierarchical_thread_change_state(
    HierarchicalThread* thread,
    StateType new_state
) {
    if (!thread || !thread->state_machine) {
        return TRANSITION_ERROR;
    }
    
    TransitionResult result = state_machine_transition(thread->state_machine, new_state);
    if (result == TRANSITION_SUCCESS) {
        thread->current_state = new_state;
    }
    
    return result;
}

StateType hierarchical_thread_get_state(HierarchicalThread* thread) {
    if (!thread) {
        return STATE_ERROR;
    }
    return thread->current_state;
}

int hierarchical_thread_register_state_callback(
    HierarchicalThread* thread,
    StateChangeCallback callback,
    void* user_data
) {
    if (!thread || !thread->state_machine || !callback) {
        return -1;
    }
    
    return state_machine_register_callback(thread->state_machine, callback, user_data) ? 0 : -1;
}

// ============================================================================
// WORK OPERATIONS
// ============================================================================

uint64_t hierarchical_thread_submit_work(
    HierarchicalThread* thread,
    void (*work_fn)(void*),
    void* data,
    size_t data_size,
    WorkPriority priority
) {
    (void)data; // Reserved for future use
    if (!thread || !thread->work_pool || !work_fn) {
        return 0;
    }
    
    // Use the work distributor API
    // Note: We need to get the distributor from somewhere
    // For now, return a placeholder
    (void)data_size;  // Unused
    (void)priority;   // Unused for now
    
    // This is a simplified version - in a real implementation,
    // we'd need access to the WorkDistributor
    return 0;
}

WorkItem* hierarchical_thread_get_work(HierarchicalThread* thread) {
    if (!thread || !thread->work_pool) {
        return NULL;
    }
    
    // This is a simplified version - in a real implementation,
    // we'd need access to the WorkDistributor
    // For now, return NULL
    return NULL;
}

int hierarchical_thread_complete_work(
    HierarchicalThread* thread,
    WorkItem* item,
    bool success
) {
    if (!thread || !item) {
        return -1;
    }
    
    item->end_time = get_time_ns();
    item->status = success ? WORK_STATUS_COMPLETED : WORK_STATUS_FAILED;
    
    thread->work_completed++;
    
    // Note: In a real implementation, we'd call work_complete or work_fail
    // from the WorkDistributor API
    
    return 0;
}

// ============================================================================
// MEMORY OPERATIONS
// ============================================================================

void* hierarchical_thread_alloc_local(HierarchicalThread* thread, size_t size) {
    if (!thread || !thread->memory) {
        return NULL;
    }
    
    // Get the segment and allocate from it
    HierarchicalSegment* seg = hierarchical_memory_get_segment(thread->memory, thread->memory_segment_id);
    if (!seg || !seg->data) {
        return NULL;
    }
    
    // Simple allocation - just return the segment data
    // In a real implementation, we'd need proper memory management
    (void)size;  // Unused for now
    return seg->data;
}

SharedMemoryEnhanced* hierarchical_thread_alloc_parent_shared(
    HierarchicalThread* thread,
    size_t size,
    SharedMemoryAccessMode mode
) {
    if (!thread || !thread->parent) {
        return NULL;
    }
    
    if (!thread->parent_shared) {
        thread->parent_shared = shared_memory_enhanced_create(
            size,
            mode,
            thread->thread_id * 1000 + thread->parent->thread_id
        );
    }
    
    return thread->parent_shared;
}

SharedMemoryEnhanced* hierarchical_thread_alloc_child_shared(
    HierarchicalThread* thread,
    uint32_t child_id,
    size_t size,
    SharedMemoryAccessMode mode
) {
    if (!thread) {
        return NULL;
    }
    
    // Find child
    HierarchicalThread* child = NULL;
    for (uint32_t i = 0; i < thread->num_children; i++) {
        if (thread->children[i]->thread_id == child_id) {
            child = thread->children[i];
            break;
        }
    }
    
    if (!child) {
        return NULL;
    }
    
    // Allocate child shared array if needed
    if (!thread->child_shared) {
        thread->child_shared = calloc(thread->max_children, sizeof(SharedMemoryEnhanced*));
        if (!thread->child_shared) {
            return NULL;
        }
    }
    
    // Find child index
    uint32_t child_idx = 0;
    for (uint32_t i = 0; i < thread->num_children; i++) {
        if (thread->children[i] == child) {
            child_idx = i;
            break;
        }
    }
    
    if (!thread->child_shared[child_idx]) {
        thread->child_shared[child_idx] = shared_memory_enhanced_create(
            size,
            mode,
            thread->thread_id * 1000 + child_id
        );
    }
    
    return thread->child_shared[child_idx];
}

void* hierarchical_thread_access_boundary(
    HierarchicalThread* thread,
    uint32_t neighbor_id,
    SharedMemoryAccessMode mode
) {
    if (!thread) {
        return NULL;
    }
    
    SharedMemoryEnhanced* boundary = hierarchical_thread_get_boundary(thread, neighbor_id);
    if (!boundary) {
        return NULL;
    }
    
    // Use appropriate access method based on mode
    if (mode == SHARED_READ_ONLY) {
        return (void*)shared_memory_read(&boundary->base);
    } else {
        return shared_memory_write(&boundary->base);
    }
}

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

int hierarchical_thread_get_stats(
    HierarchicalThread* thread,
    HierarchicalThreadStats* stats
) {
    if (!thread || !stats) {
        return -1;
    }
    
    memset(stats, 0, sizeof(HierarchicalThreadStats));
    
    stats->thread_id = thread->thread_id;
    stats->role = thread->role;
    stats->current_state = thread->current_state;
    stats->num_children = thread->num_children;
    stats->num_neighbors = thread->num_neighbors;
    stats->messages_sent = thread->messages_sent;
    stats->messages_received = thread->messages_received;
    stats->messages_pending = 0;  // Simplified - would use MessageSystem in real version
    stats->work_completed = thread->work_completed;
    stats->work_stolen = thread->work_stolen;
    stats->work_pending = 0;  // Simplified - would need WorkDistributor access
    stats->total_runtime = thread->running ? 
        (get_time_ns() - thread->start_time) : thread->total_runtime;
    
    // Calculate memory usage
    if (thread->local_shared) {
        stats->local_memory_used = thread->local_shared->base.size;
    }
    if (thread->parent_shared) {
        stats->shared_memory_used += thread->parent_shared->base.size;
    }
    for (uint32_t i = 0; i < thread->num_neighbors; i++) {
        if (thread->neighbors[i].boundary) {
            stats->boundary_memory_used += thread->neighbors[i].boundary->base.size;
        }
    }
    
    return 0;
}

int hierarchical_thread_pool_get_stats(
    HierarchicalThreadPool* pool,
    HierarchicalThreadPoolStats* stats
) {
    if (!pool || !stats) {
        return -1;
    }
    
    memset(stats, 0, sizeof(HierarchicalThreadPoolStats));
    
    stats->num_threads = pool->num_threads;
    stats->num_levels = pool->num_levels;
    stats->symmetry_fold = pool->symmetry_fold;
    stats->total_messages = pool->total_messages;
    stats->total_work_items = pool->total_work_items;
    stats->total_state_changes = pool->total_state_changes;
    
    // Aggregate thread statistics
    uint64_t total_work = 0;
    uint64_t min_work = UINT64_MAX;
    uint64_t max_work = 0;
    
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        HierarchicalThread* thread = pool->threads[i];
        if (thread) {
            HierarchicalThreadStats thread_stats;
            hierarchical_thread_get_stats(thread, &thread_stats);
            
            stats->total_memory_used += thread_stats.local_memory_used;
            stats->total_shared_memory += thread_stats.shared_memory_used;
            stats->total_boundary_memory += thread_stats.boundary_memory_used;
            
            total_work += thread_stats.work_completed;
            if (thread_stats.work_completed < min_work) min_work = thread_stats.work_completed;
            if (thread_stats.work_completed > max_work) max_work = thread_stats.work_completed;
        }
    }
    
    // Calculate load balance factor (1.0 = perfect balance)
    if (max_work > 0) {
        stats->load_balance_factor = (double)min_work / (double)max_work;
    } else {
        stats->load_balance_factor = 1.0;
    }
    
    return 0;
}

void hierarchical_thread_print_stats(HierarchicalThread* thread) {
    if (!thread) {
        return;
    }
    
    HierarchicalThreadStats stats;
    if (hierarchical_thread_get_stats(thread, &stats) != 0) {
        return;
    }
    
    printf("Thread %u Statistics:\n", stats.thread_id);
    printf("  Role: %d\n", stats.role);
    printf("  State: %d\n", stats.current_state);
    printf("  Children: %u\n", stats.num_children);
    printf("  Neighbors: %u\n", stats.num_neighbors);
    printf("  Messages: sent=%lu, received=%lu, pending=%lu\n",
           stats.messages_sent, stats.messages_received, stats.messages_pending);
    printf("  Work: completed=%lu, stolen=%lu, pending=%lu\n",
           stats.work_completed, stats.work_stolen, stats.work_pending);
    printf("  Memory: local=%zu, shared=%zu, boundary=%zu\n",
           stats.local_memory_used, stats.shared_memory_used, stats.boundary_memory_used);
    printf("  Runtime: %lu ns\n", stats.total_runtime);
}

void hierarchical_thread_pool_print_stats(HierarchicalThreadPool* pool) {
    if (!pool) {
        return;
    }
    
    HierarchicalThreadPoolStats stats;
    if (hierarchical_thread_pool_get_stats(pool, &stats) != 0) {
        return;
    }
    
    printf("Thread Pool Statistics:\n");
    printf("  Threads: %u\n", stats.num_threads);
    printf("  Levels: %u\n", stats.num_levels);
    printf("  Symmetry: %u-fold\n", stats.symmetry_fold);
    printf("  Messages: %lu\n", stats.total_messages);
    printf("  Work Items: %lu\n", stats.total_work_items);
    printf("  State Changes: %lu\n", stats.total_state_changes);
    printf("  Memory: total=%zu, shared=%zu, boundary=%zu\n",
           stats.total_memory_used, stats.total_shared_memory, stats.total_boundary_memory);
    printf("  Load Balance: %.2f\n", stats.load_balance_factor);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

HierarchicalThread* hierarchical_thread_pool_get_thread(
    HierarchicalThreadPool* pool,
    uint32_t thread_id
) {
    if (!pool) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        if (pool->threads[i] && pool->threads[i]->thread_id == thread_id) {
            return pool->threads[i];
        }
    }
    
    return NULL;
}

double hierarchical_thread_distance(
    HierarchicalThread* thread1,
    HierarchicalThread* thread2
) {
    if (!thread1 || !thread2 || thread1->num_dimensions != thread2->num_dimensions) {
        return -1.0;
    }
    
    return calculate_distance(thread1->position, thread2->position, thread1->num_dimensions);
}

uint32_t hierarchical_thread_find_nearest_neighbors(
    HierarchicalThread* thread,
    HierarchicalThreadPool* pool,
    uint32_t k,
    uint32_t* out_neighbors
) {
    if (!thread || !pool || !out_neighbors || k == 0) {
        return 0;
    }
    
    // Simple implementation: calculate distances to all threads and sort
    typedef struct {
        uint32_t thread_id;
        double distance;
    } DistanceEntry;
    
    DistanceEntry* distances = malloc(pool->num_threads * sizeof(DistanceEntry));
    if (!distances) {
        return 0;
    }
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        HierarchicalThread* other = pool->threads[i];
        if (other && other->thread_id != thread->thread_id) {
            distances[count].thread_id = other->thread_id;
            distances[count].distance = hierarchical_thread_distance(thread, other);
            count++;
        }
    }
    
    // Simple bubble sort (good enough for small k)
    for (uint32_t i = 0; i < count - 1; i++) {
        for (uint32_t j = 0; j < count - i - 1; j++) {
            if (distances[j].distance > distances[j + 1].distance) {
                DistanceEntry temp = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = temp;
            }
        }
    }
    
    // Copy k nearest neighbors
    uint32_t result = (k < count) ? k : count;
    for (uint32_t i = 0; i < result; i++) {
        out_neighbors[i] = distances[i].thread_id;
    }
    
    free(distances);
    return result;
}

// ============================================================================
// 88D-SPECIFIC FUNCTIONS
// ============================================================================

HierarchicalThreadPool* hierarchical_thread_pool_create_88d(uint32_t base) {
    // Create pool with 88D configuration
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(
        HIERARCHICAL_88D_TOTAL_THREADS,   // 96 threads
        HIERARCHICAL_88D_CLOCK_POSITIONS, // 12-fold symmetry
        HIERARCHICAL_88D_TOTAL_DIMENSIONS, // 88 dimensions
        true  // NUMA-aware
    );
    
    if (!pool) {
        return NULL;
    }
    
    // Enable 88D structure
    pool->use_88d_structure = true;
    pool->num_levels = HIERARCHICAL_88D_NUM_LAYERS;
    pool->threads_per_level = HIERARCHICAL_88D_THREADS_PER_LAYER;
    
    // Create clock lattice
    pool->clock_lattice = calloc(1, sizeof(ClockContext));
    if (!pool->clock_lattice) {
        hierarchical_thread_pool_free(pool);
        return NULL;
    }
    
    MathError err = clock_init(pool->clock_lattice);
    if (err != MATH_SUCCESS) {
        free(pool->clock_lattice);
        pool->clock_lattice = NULL;
        hierarchical_thread_pool_free(pool);
        return NULL;
    }
    
    // Create Platonic solid frames for each layer
    // TODO: Use solid_types to initialize layer frames when integrated with Abacus88D
    for (int i = 0; i < HIERARCHICAL_88D_NUM_LAYERS; i++) {
        // Get frame from abacus88d layer (they're already created)
        // For now, set to NULL - will be populated when integrated with Abacus88D
        pool->layer_frames[i] = NULL;
        
        // Initialize layer barrier
        pthread_barrier_init(&pool->layer_barriers[i], NULL, HIERARCHICAL_88D_THREADS_PER_LAYER);
    }
    
    // Initialize global barrier
    pthread_barrier_init(&pool->global_barrier, NULL, HIERARCHICAL_88D_TOTAL_THREADS);
    
    // Create and organize threads into 88D structure
    for (uint32_t i = 0; i < HIERARCHICAL_88D_TOTAL_THREADS; i++) {
        // Create thread if it doesn't exist
        if (!pool->threads[i]) {
            pool->threads[i] = hierarchical_thread_create(i, THREAD_ROLE_WORKER, NULL, pool);
            if (!pool->threads[i]) {
                // Failed to create thread, clean up and return
                hierarchical_thread_pool_free(pool);
                return NULL;
            }
            pool->num_threads++;
        }
        
        HierarchicalThread* thread = pool->threads[i];
        
        // Calculate 88D position from thread index
        uint8_t layer = i / HIERARCHICAL_88D_THREADS_PER_LAYER;
        uint8_t dim = i % HIERARCHICAL_88D_THREADS_PER_LAYER;
        
        // Set 88D position
        thread->layer = layer;
        thread->dimension = dim;  // Dimension matches array index (0-11)
        thread->clock_position = dim;  // Clock position (0-11)
        
        // Set role
        if (dim == 0) {
            thread->role = THREAD_ROLE_CONTROL;
        } else {
            thread->role = THREAD_ROLE_WORKER;
        }
        
        // Set geometric frame (will be NULL for now)
        thread->platonic_frame = pool->layer_frames[layer];
        
        // Create abacus values
        thread->value = abacus_new(base);
        thread->accumulator = abacus_new(base);
        thread->temp = abacus_new(base);
        
        // Initialize gradient lock
        pthread_mutex_init(&thread->gradient_lock, NULL);
        
        // Initialize work queue
        thread->work_queue_head = NULL;
        thread->work_queue_tail = NULL;
        thread->work_queue_size = 0;
        pthread_mutex_init(&thread->work_queue_mutex, NULL);
        thread->should_exit = false;
        thread->completion_barrier = NULL;  // Will be set by training system
        
        // Store in layer array
        if (layer < HIERARCHICAL_88D_NUM_LAYERS && dim < HIERARCHICAL_88D_THREADS_PER_LAYER) {
            pool->layers[layer][dim] = thread;
            if (dim == 0) {
                pool->control_threads[layer] = thread;
            }
        }
    }
    
    // Initialize 88D statistics
    atomic_init(&pool->total_boundary_crossings, 0);
    atomic_init(&pool->total_twin_primes, 0);
    
    // Start worker loops for all worker threads
    for (uint32_t i = 0; i < HIERARCHICAL_88D_TOTAL_THREADS; i++) {
        HierarchicalThread* thread = pool->threads[i];
        if (thread && thread->role == THREAD_ROLE_WORKER) {
            // Start worker loop
            thread->running = true;
            thread->batch_count = 0;
            thread->work_completed = 0;
            
            int result = pthread_create(&thread->pthread, NULL, 
                                       hierarchical_thread_worker, thread);
            if (result != 0) {
                fprintf(stderr, "Failed to start worker thread %u\n", i);
                // Continue with other threads
            }
        }
    }
    atomic_init(&pool->total_operations, 0);
    
    // Initialize group nesting
    pool->parent_group = NULL;
    pool->child_groups = NULL;
    pool->num_child_groups = 0;
    pool->max_child_groups = 0;
    
    // Mark pool as running since threads are started
    pool->running = true;
    
    fprintf(stderr, "DEBUG: 88D pool created with %u threads, pool->running=%d\n", 
            pool->num_threads, pool->running);
    fflush(stderr);
    
    return pool;
}

HierarchicalThread* hierarchical_thread_get(
    HierarchicalThreadPool* pool,
    uint8_t layer,
    uint8_t dimension
) {
    if (!pool || !pool->use_88d_structure) {
        return NULL;
    }
    
    if (layer >= HIERARCHICAL_88D_NUM_LAYERS) {
        return NULL;
    }
    
    if (dimension >= HIERARCHICAL_88D_THREADS_PER_LAYER) {
        return NULL;
    }
    
    return pool->layers[layer][dimension];
}

int hierarchical_thread_sync_layer(
    HierarchicalThreadPool* pool,
    uint8_t layer
) {
    if (!pool || !pool->use_88d_structure) {
        return -1;
    }
    
    if (layer >= HIERARCHICAL_88D_NUM_LAYERS) {
        return -1;
    }
    
    // Wait at layer barrier
    int result = pthread_barrier_wait(&pool->layer_barriers[layer]);
    
    // pthread_barrier_wait returns PTHREAD_BARRIER_SERIAL_THREAD for one thread
    // and 0 for all others, both are success
    return (result == 0 || result == PTHREAD_BARRIER_SERIAL_THREAD) ? 0 : -1;
}

int hierarchical_thread_sync_all(HierarchicalThreadPool* pool) {
    if (!pool || !pool->use_88d_structure) {
        return -1;
    }
    
    // Wait at global barrier
    int result = pthread_barrier_wait(&pool->global_barrier);
    
    return (result == 0 || result == PTHREAD_BARRIER_SERIAL_THREAD) ? 0 : -1;
}

int hierarchical_thread_notify_boundary_crossing(
    HierarchicalThread* thread,
    uint8_t from_layer,
    uint8_t to_layer
) {
    (void)from_layer; (void)to_layer; // Reserved for future layer-specific handling
    if (!thread) {
        return -1;
    }
    
    thread->boundary_crossed = true;
    thread->boundary_crossings++;
    
    // Update pool statistics if available
    // (Would need pool reference - can be added later)
    
    return 0;
}

int hierarchical_thread_notify_twin_prime(
    HierarchicalThread* thread,
    uint64_t prime1,
    uint64_t prime2
) {
    (void)prime1; (void)prime2; // Reserved for future prime-specific handling
    if (!thread) {
        return -1;
    }
    
    thread->twin_prime_detected = true;
    thread->twin_primes_found++;
    
    // Update pool statistics if available
    // (Would need pool reference - can be added later)
    
    return 0;
}

uint32_t hierarchical_thread_get_siblings(
    HierarchicalThread* thread,
    HierarchicalThread** out_siblings,
    uint32_t max_siblings
) {
    if (!thread || !out_siblings) {
        return 0;
    }
    
    // If siblings array is populated, use it
    if (thread->siblings && thread->num_siblings > 0) {
        uint32_t count = (thread->num_siblings < max_siblings) ? 
                         thread->num_siblings : max_siblings;
        
        for (uint32_t i = 0; i < count; i++) {
            out_siblings[i] = thread->siblings[i];
        }
        
        return count;
    }
    
    return 0;
}

int hierarchical_thread_pool_attach_group(
    HierarchicalThreadPool* parent,
    HierarchicalThreadPool* child
) {
    if (!parent || !child) {
        return -1;
    }
    
    pthread_mutex_lock(&parent->pool_mutex);
    
    // Allocate child groups array if needed
    if (!parent->child_groups) {
        parent->max_child_groups = 8;  // Start with 8
        parent->child_groups = calloc(parent->max_child_groups, 
                                     sizeof(HierarchicalThreadPool*));
        if (!parent->child_groups) {
            pthread_mutex_unlock(&parent->pool_mutex);
            return -1;
        }
    }
    
    // Expand if needed
    if (parent->num_child_groups >= parent->max_child_groups) {
        uint32_t new_max = parent->max_child_groups * 2;
        struct HierarchicalThreadPool** new_array = realloc(parent->child_groups,
                                                     new_max * sizeof(struct HierarchicalThreadPool*));
        if (!new_array) {
            pthread_mutex_unlock(&parent->pool_mutex);
            return -1;
        }
        parent->child_groups = new_array;
        parent->max_child_groups = new_max;
    }
    
    // Add child
    parent->child_groups[parent->num_child_groups] = (struct HierarchicalThreadPool*)child;
    parent->num_child_groups++;
    child->parent_group = (struct HierarchicalThreadPool *)parent;
    
    pthread_mutex_unlock(&parent->pool_mutex);
    
    return 0;
}

int hierarchical_thread_pool_detach_group(
    HierarchicalThreadPool* parent,
    HierarchicalThreadPool* child
) {
    if (!parent || !child || !parent->child_groups) {
        return -1;
    }
    
    pthread_mutex_lock(&parent->pool_mutex);
    
    // Find and remove child
    for (uint32_t i = 0; i < parent->num_child_groups; i++) {
        if (parent->child_groups[i] == (struct HierarchicalThreadPool *)child) {
            // Shift remaining children
            for (uint32_t j = i; j < parent->num_child_groups - 1; j++) {
                parent->child_groups[j] = parent->child_groups[j + 1];
            }
            parent->num_child_groups--;
            child->parent_group = NULL;
            
            pthread_mutex_unlock(&parent->pool_mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&parent->pool_mutex);
    return -1;  // Child not found
}

int hierarchical_thread_pool_get_88d_stats(
    HierarchicalThreadPool* pool,
    uint64_t* out_boundary_crossings,
    uint64_t* out_twin_primes,
    uint64_t* out_operations
) {
    if (!pool || !pool->use_88d_structure) {
        return -1;
    }
    
    if (out_boundary_crossings) {
        *out_boundary_crossings = atomic_load(&pool->total_boundary_crossings);
    }
    
    if (out_twin_primes) {
        *out_twin_primes = atomic_load(&pool->total_twin_primes);
    }
    
    if (out_operations) {
        *out_operations = atomic_load(&pool->total_operations);
    }
    
    return 0;
}
// ============================================================================
// 88D WORKER LOOP IMPLEMENTATION
// ============================================================================

/**
 * Process one work item in thread
 * This is where the actual computation happens
 */
/**
 * Process a single token through the model layer
 * This is called by worker threads to perform actual computation
 * 
 * Uses the generic model interface - no CLLM dependency!
 */
// ============================================================================
// WORK PROCESSORS
// ============================================================================

/**
 * Process forward pass work item
 */
static int worker_process_forward(HierarchicalThread* thread, TrainingWorkItem* work) {
    if (!thread || !work || !thread->model) {
        return -1;
    }
    
    // Get generic model interface
    GenericModel* model = thread->model;
    
    // Validate model
    if (!generic_model_validate(model)) {
        return -1;
    }
    
    // Check if this is a control thread or worker thread
    if (thread->dimension == 0) {
        // Control thread - coordinate layer processing
        return 0;
    }
    
    // Worker thread - process token through model layer
    uint32_t dim = model->embedding_dim;
    
    // Allocate output buffer
    double* output = (double*)malloc(dim * sizeof(double));
    if (!output) {
        return -1;
    }
    
    // Get input from activation buffer
    double* input = thread->activation_buffer;
    
    // Call through generic interface
    int result = model->forward_layer(
        model->model_data,  // Opaque model data
        thread,             // Thread context
        thread->layer,      // Layer index
        input,              // Input
        output              // Output
    );
    
    if (result != 0) {
        free(output);
        return -1;
    }
    
    // Copy output back to activation buffer for next layer
    memcpy(thread->activation_buffer, output, dim * sizeof(double));
    
    free(output);
    return 0;
}

/**
 * Process backward pass work item
 */
static int worker_process_backward(HierarchicalThread* thread, TrainingWorkItem* work) {
    if (!thread || !work || !thread->model) {
        return -1;
    }
    
    // Get generic model interface
    GenericModel* model = thread->model;
    
    // Validate model
    if (!generic_model_validate(model)) {
        return -1;
    }
    
    // Check if this is a control thread or worker thread
    if (thread->dimension == 0) {
        // Control thread - coordinate layer processing
        return 0;
    }
    
    // Worker thread - compute gradients
    uint32_t dim = model->embedding_dim;
    
    // Allocate gradient buffer
    double* grad_output = (double*)malloc(dim * sizeof(double));
    if (!grad_output) {
        return -1;
    }
    
    // Initialize gradient from loss
    // For now, use simple gradient (will be replaced with actual loss gradient)
    for (uint32_t i = 0; i < dim; i++) {
        grad_output[i] = thread->activation_buffer[i] - (double)work->target_id;
    }
    
    // Call through generic interface for backward pass
    int result = model->backward_layer(
        model->model_data,  // Opaque model data
        thread,             // Thread context
        thread->layer,      // Layer index
        grad_output,        // Gradient output
        grad_output         // Gradient input (in-place)
    );
    
    if (result != 0) {
        free(grad_output);
        return -1;
    }
    
    // Store gradients in thread's gradient buffer (cached_qkv used as gradient storage)
    if (thread->cached_qkv && thread->cached_qkv_size >= dim) {
        memcpy(thread->cached_qkv, grad_output, dim * sizeof(double));
    }
    
    free(grad_output);
    return 0;
}

// ============================================================================
// ATTENTION OPERATIONS (DAY 7)
// ============================================================================

/**
 * Compute Q, K, V for a token using thread's local parameters
 * 
 * This function computes the Query, Key, and Value matrices for attention
 * using the thread's local W_q, W_k, W_v parameters.
 * 
 * @param thread Thread that owns the computation
 * @param input Input embedding (double array)
 * @param embedding_dim Embedding dimension
 * @param Q Output Q (double array, pre-allocated)
 * @param K Output K (double array, pre-allocated)
 * @param V Output V (double array, pre-allocated)
 * @return 0 on success, -1 on error
 */
int worker_compute_qkv(
    HierarchicalThread* thread,
    const double* input,
    uint32_t embedding_dim,
    double* Q,
    double* K,
    double* V
) {
    if (!thread || !input || !Q || !K || !V) {
        fprintf(stderr, "ERROR: Invalid parameters for worker_compute_qkv\n");
        return -1;
    }
    
    // Import thread parameter function
    extern CrystallineAbacus* thread_get_parameter(
        HierarchicalThread* thread,
        const char* name,
        uint32_t index
    );
    
    // Get W_q, W_k, W_v from thread's parameters
    // For now, we'll use a simplified approach with double arrays
    // TODO: Full CrystallineAbacus matrix multiplication in Phase 4
    
    // Placeholder: Initialize Q, K, V to input (identity transformation)
    // This will be replaced with proper matrix multiplication
    for (uint32_t i = 0; i < embedding_dim; i++) {
        Q[i] = input[i];
        K[i] = input[i];
        V[i] = input[i];
    }
    
    return 0;
}

/**
 * Share K, V with neighbors via shared boundaries
 * 
 * @param thread Thread sharing its K, V
 * @param K K matrix to share (double array)
 * @param V V matrix to share (double array)
 * @param dim Dimension of K and V
 * @return 0 on success, -1 on error
 */
int worker_share_kv(
    HierarchicalThread* thread,
    const double* K,
    const double* V,
    uint32_t dim
) {
    if (!thread || !K || !V) {
        fprintf(stderr, "ERROR: Invalid parameters for worker_share_kv\n");
        return -1;
    }
    
    // Write K, V to all shared boundaries
    for (uint32_t i = 0; i < thread->num_neighbors; i++) {
        SharedMemoryEnhanced* boundary = thread->neighbors[i].boundary;
        if (!boundary) continue;
        
        // Get write access to shared memory
        void* shared_ptr = shared_memory_write(&boundary->base);
        if (!shared_ptr) continue;
        
        // Write K and V (concatenated)
        size_t kv_size = dim * 2 * sizeof(double);
        if (boundary->base.size < kv_size) {
            shared_memory_release_write(&boundary->base);
            continue;
        }
        
        double* shared_data = (double*)shared_ptr;
        memcpy(shared_data, K, dim * sizeof(double));
        memcpy(shared_data + dim, V, dim * sizeof(double));
        
        // Release write access
        shared_memory_release_write(&boundary->base);
    }
    
    return 0;
}

/**
 * Collect K, V from neighbors via shared boundaries
 * 
 * @param thread Thread collecting K, V
 * @param neighbor_K Array of K matrices from neighbors (pre-allocated)
 * @param neighbor_V Array of V matrices from neighbors (pre-allocated)
 * @param dim Dimension of K and V
 * @param max_neighbors Maximum number of neighbors
 * @return Number of neighbors collected, or -1 on error
 */
int worker_collect_neighbor_kv(
    HierarchicalThread* thread,
    double** neighbor_K,
    double** neighbor_V,
    uint32_t dim,
    uint32_t max_neighbors
) {
    if (!thread || !neighbor_K || !neighbor_V) {
        fprintf(stderr, "ERROR: Invalid parameters for worker_collect_neighbor_kv\n");
        return -1;
    }
    
    uint32_t collected = 0;
    
    // Read K, V from all shared boundaries
    for (uint32_t i = 0; i < thread->num_neighbors && collected < max_neighbors; i++) {
        SharedMemoryEnhanced* boundary = thread->neighbors[i].boundary;
        if (!boundary) continue;
        
        // Get read access to shared memory
        const void* shared_ptr = shared_memory_read(&boundary->base);
        if (!shared_ptr) continue;
        
        // Read K and V
        const double* shared_data = (const double*)shared_ptr;
        memcpy(neighbor_K[collected], shared_data, dim * sizeof(double));
        memcpy(neighbor_V[collected], shared_data + dim, dim * sizeof(double));
        
        // Release read access
        shared_memory_release_read(&boundary->base);
        
        collected++;
    }
    
    return (int)collected;
}

// ============================================================================
// OPTIMIZER OPERATIONS (DAY 10)
// ============================================================================

/**
 * Clear all gradients in thread
 * 
 * @param thread Thread to clear gradients for
 * @return 0 on success, -1 on error
 */
int worker_clear_gradients(HierarchicalThread* thread) {
    if (!thread) {
        fprintf(stderr, "ERROR: Invalid thread for worker_clear_gradients\n");
        return -1;
    }
    
    // Clear all gradient accumulators
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        if (thread->gradients[i]) {
            // TODO: Reset CrystallineAbacus to zero
            // For now, just acknowledge the clear
            // Will be implemented in Phase 4 optimization
        }
    }
    
    return 0;
}

/**
 * Apply optimizer to thread's parameters
 * 
 * Uses Adam optimizer:
 *   m = β1*m + (1-β1)*grad
 *   v = β2*v + (1-β2)*grad²
 *   param -= lr * m / (√v + ε)
 * 
 * @param thread Thread that owns the parameters
 * @param learning_rate Learning rate
 * @param beta1 Adam beta1 (momentum decay, default 0.9)
 * @param beta2 Adam beta2 (velocity decay, default 0.999)
 * @param epsilon Adam epsilon (numerical stability, default 1e-8)
 * @return 0 on success, -1 on error
 */
int worker_apply_optimizer(
    HierarchicalThread* thread,
    double learning_rate,
    double beta1,
    double beta2,
    double epsilon
) {
    if (!thread) {
        fprintf(stderr, "ERROR: Invalid thread for worker_apply_optimizer\n");
        return -1;
    }
    
    // For each parameter in this thread
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        // Get parameter, gradient, momentum, velocity
        CrystallineAbacus* param = thread->parameters[i];
        CrystallineAbacus* grad = thread->gradients[i];
        CrystallineAbacus* m = thread->momentum[i];
        CrystallineAbacus* v = thread->velocity[i];
        
        if (!param || !grad || !m || !v) continue;
        
        // Update momentum: m = β1*m + (1-β1)*grad
        // Simplified: m = grad (will upgrade in optimization phase)
        // TODO: Full Adam implementation with CrystallineAbacus
        
        // Update velocity: v = β2*v + (1-β2)*grad²
        // Simplified: v = grad² (will upgrade in optimization phase)
        // TODO: Full Adam implementation with CrystallineAbacus
        
        // Update parameter: param -= lr * m / (√v + ε)
        // Simplified: param -= lr * grad (will upgrade in optimization phase)
        // TODO: Full Adam implementation with CrystallineAbacus
        
        // For now, just acknowledge the update
        // The actual parameter update will be implemented in Phase 4 optimization
    }
    
    // Clear gradients after update
    worker_clear_gradients(thread);
    
    return 0;
}

// ============================================================================
// GRADIENT OPERATIONS (DAY 9)
// ============================================================================

/**
 * Compute gradients for FFN parameters
 * 
 * Computes:
 *   ∂L/∂W_ffn2 = grad_output × hidden^T
 *   ∂L/∂hidden = grad_output × W_ffn2^T
 *   ∂L/∂hidden (after ReLU) = ∂L/∂hidden * (hidden > 0)
 *   ∂L/∂W_ffn1 = grad_hidden × input^T
 *   ∂L/∂input = grad_hidden × W_ffn1^T
 * 
 * @param thread Thread that owns the computation
 * @param grad_output Gradient from next layer (double array)
 * @param input Input that was used in forward pass (double array)
 * @param hidden Hidden layer from forward pass (double array)
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension
 * @param grad_input Gradient to pass to previous layer (output)
 * @return 0 on success, -1 on error
 */
int worker_compute_ffn_gradients(
    HierarchicalThread* thread,
    const double* grad_output,
    const double* input,
    const double* hidden,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    double* grad_input
) {
    if (!thread || !grad_output || !input || !hidden || !grad_input) {
        fprintf(stderr, "ERROR: Invalid parameters for worker_compute_ffn_gradients\n");
        return -1;
    }
    
    // Allocate temporary gradients
    double* grad_hidden = (double*)malloc(hidden_dim * sizeof(double));
    if (!grad_hidden) {
        fprintf(stderr, "ERROR: Failed to allocate grad_hidden\n");
        return -1;
    }
    
    // Compute ∂L/∂W_ffn2 = grad_output × hidden^T
    // Simplified: Just store grad_output (will be accumulated later)
    // TODO: Full matrix multiplication in Phase 4 optimization
    
    // Compute ∂L/∂hidden = grad_output × W_ffn2^T
    // Simplified: Copy grad_output to grad_hidden
    for (uint32_t i = 0; i < hidden_dim; i++) {
        grad_hidden[i] = (i < embedding_dim) ? grad_output[i] : 0.0;
    }
    
    // Apply ReLU gradient: grad_hidden *= (hidden > 0)
    for (uint32_t i = 0; i < hidden_dim; i++) {
        if (hidden[i] <= 0.0) {
            grad_hidden[i] = 0.0;
        }
    }
    
    // Compute ∂L/∂W_ffn1 = grad_hidden × input^T
    // Simplified: Just store grad_hidden (will be accumulated later)
    
    // Compute ∂L/∂input = grad_hidden × W_ffn1^T
    // Simplified: Copy grad_hidden to grad_input
    for (uint32_t i = 0; i < embedding_dim; i++) {
        grad_input[i] = (i < hidden_dim) ? grad_hidden[i] : 0.0;
    }
    
    free(grad_hidden);
    return 0;
}

/**
 * Compute gradients for attention parameters
 * 
 * @param thread Thread that owns the computation
 * @param grad_output Gradient from next layer
 * @param Q Query matrix from forward pass
 * @param K Key matrix from forward pass
 * @param V Value matrix from forward pass
 * @param input Input from forward pass
 * @param embedding_dim Embedding dimension
 * @param grad_input Gradient to pass to previous layer (output)
 * @return 0 on success, -1 on error
 */
int worker_compute_attention_gradients(
    HierarchicalThread* thread,
    const double* grad_output,
    const double* Q,
    const double* K,
    const double* V,
    const double* input,
    uint32_t embedding_dim,
    double* grad_input
) {
    if (!thread || !grad_output || !Q || !K || !V || !input || !grad_input) {
        fprintf(stderr, "ERROR: Invalid parameters for worker_compute_attention_gradients\n");
        return -1;
    }
    
    // Compute ∂L/∂W_v, ∂L/∂W_k, ∂L/∂W_q
    // Simplified: Just store grad_output (will be accumulated later)
    // TODO: Full attention gradient computation in Phase 4 optimization
    
    // Compute grad_input
    // Simplified: Copy grad_output
    memcpy(grad_input, grad_output, embedding_dim * sizeof(double));
    
    return 0;
}

/**
 * Compute gradients for embeddings
 * 
 * @param thread Thread that owns the embedding
 * @param token_id Token ID
 * @param grad_output Gradient from next layer
 * @param embedding_dim Embedding dimension
 * @return 0 on success, -1 on error
 */
int worker_compute_embedding_gradients(
    HierarchicalThread* thread,
    uint32_t token_id,
    const double* grad_output,
    uint32_t embedding_dim
) {
    if (!thread || !grad_output) {
        fprintf(stderr, "ERROR: Invalid parameters for worker_compute_embedding_gradients\n");
        return -1;
    }
    
    // Store gradient for this token's embedding
    // Simplified: Just acknowledge the gradient (will be accumulated later)
    // TODO: Full gradient accumulation in Phase 4 optimization
    
    // For now, just return success
    // The gradient will be used by the optimizer
    (void)token_id;  // Suppress unused warning
    (void)embedding_dim;  // Suppress unused warning
    
    return 0;
}

// ============================================================================
// FFN OPERATIONS (DAY 8)
// ============================================================================

/**
 * Compute FFN (Feed-Forward Network) using thread's local parameters
 * 
 * Computes:
 *   hidden = ReLU(input × W_ffn1)
 *   output = hidden × W_ffn2
 * 
 * @param thread Thread that owns the computation
 * @param input Input from attention (double array)
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension
 * @param output Output (double array, pre-allocated)
 * @return 0 on success, -1 on error
 */
int worker_compute_ffn(
    HierarchicalThread* thread,
    const double* input,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    double* output
) {
    if (!thread || !input || !output) {
        fprintf(stderr, "ERROR: Invalid parameters for worker_compute_ffn\n");
        return -1;
    }
    
    // Import thread parameter function
    extern CrystallineAbacus* thread_get_parameter(
        HierarchicalThread* thread,
        const char* name,
        uint32_t index
    );
    
    // Get W_ffn1, W_ffn2 from thread's parameters
    // W_ffn1: [embedding_dim, hidden_dim]
    // W_ffn2: [hidden_dim, embedding_dim]
    // For now, we'll use a simplified approach with double arrays
    // TODO: Full CrystallineAbacus matrix multiplication in Phase 4
    
    // Allocate temporary hidden layer
    double* hidden = (double*)malloc(hidden_dim * sizeof(double));
    if (!hidden) {
        fprintf(stderr, "ERROR: Failed to allocate hidden layer\n");
        return -1;
    }
    
    // Compute hidden = input × W_ffn1
    // Simplified: Copy input to hidden (identity transformation)
    // TODO: Full matrix multiplication in Phase 4
    for (uint32_t i = 0; i < hidden_dim; i++) {
        hidden[i] = (i < embedding_dim) ? input[i] : 0.0;
    }
    
    // Apply ReLU: hidden = max(0, hidden)
    for (uint32_t i = 0; i < hidden_dim; i++) {
        if (hidden[i] < 0.0) {
            hidden[i] = 0.0;
        }
    }
    
    // Compute output = hidden × W_ffn2
    // Simplified: Copy hidden to output (identity transformation)
    // TODO: Full matrix multiplication in Phase 4
    for (uint32_t i = 0; i < embedding_dim; i++) {
        output[i] = (i < hidden_dim) ? hidden[i] : 0.0;
    }
    
    free(hidden);
    return 0;
}

// ============================================================================
// EMBEDDING OPERATIONS (DAY 6)
// ============================================================================

/**
 * Get embedding for a token from thread's local storage
 * 
 * This function retrieves the embedding for a specific token from the thread
 * that owns it. The embedding is stored in the thread's parameter storage
 * as a CrystallineAbacus.
 * 
 * @param thread Thread that owns the token's embedding
 * @param token_id Token ID to get embedding for
 * @param output Output buffer (CrystallineAbacus) to store the embedding
 * @return 0 on success, -1 on error
 */
int worker_get_embedding(
    HierarchicalThread* thread,
    uint32_t token_id,
    CrystallineAbacus* output
) {
    if (!thread || !output) {
        fprintf(stderr, "ERROR: Invalid thread or output buffer\n");
        return -1;
    }
    
    // Import thread parameter function
    extern CrystallineAbacus* thread_get_parameter(
        HierarchicalThread* thread,
        const char* name,
        uint32_t index
    );
    
    // Get embedding parameter from thread's storage
    // For Layer 0 threads, embeddings are stored with the token_id as index
    CrystallineAbacus* embedding = thread_get_parameter(thread, "embeddings", token_id);
    if (!embedding) {
        fprintf(stderr, "ERROR: Failed to get embedding for token %u from thread [%d][%d]\n",
                token_id, thread->layer, thread->dimension);
        return -1;
    }
    
    // Import abacus copy function (returns new abacus)
    extern CrystallineAbacus* abacus_copy(const CrystallineAbacus* src);
    
    // Copy embedding to output
    // Note: abacus_copy creates a new abacus, so we need to copy the data manually
    // to the provided output buffer
    
    // Verify bases match
    if (output->base != embedding->base) {
        fprintf(stderr, "ERROR: Output abacus has different base than embedding\n");
        return -1;
    }
    
    // Copy dense beads if in dense mode
    if (!embedding->is_sparse && embedding->beads) {
        // Ensure output has enough capacity
        if (output->capacity < embedding->num_beads) {
            fprintf(stderr, "ERROR: Output abacus capacity too small\n");
            return -1;
        }
        
        // Copy beads
        for (size_t i = 0; i < embedding->num_beads; i++) {
            output->beads[i] = embedding->beads[i];
        }
        output->num_beads = embedding->num_beads;
    }
    
    // Copy sparse beads if in sparse mode
    if (embedding->is_sparse && embedding->sparse_beads) {
        // Ensure output has enough capacity
        if (output->sparse_capacity < embedding->num_nonzero) {
            fprintf(stderr, "ERROR: Output abacus sparse capacity too small\n");
            return -1;
        }
        
        // Copy sparse beads
        for (size_t i = 0; i < embedding->num_nonzero; i++) {
            output->sparse_beads[i] = embedding->sparse_beads[i];
        }
        output->num_nonzero = embedding->num_nonzero;
    }
    
    // Copy metadata
    output->is_sparse = embedding->is_sparse;
    output->negative = embedding->negative;
    output->min_exponent = embedding->min_exponent;
    output->max_exponent = embedding->max_exponent;
    
    return 0;
}

/**
 * DEPRECATED: Old token processor (kept for compatibility)
 */
static int worker_process_token(HierarchicalThread* thread) {
    if (!thread || !thread->activation_buffer || !thread->model) {
        return -1;
    }
    
    // Get generic model interface
    GenericModel* model = thread->model;
    
    // Validate model
    if (!generic_model_validate(model)) {
        return -1;
    }
    
    // Get thread's layer
    uint8_t layer_idx = thread->layer;
    
    // Check if this is a control thread or worker thread
    if (thread->dimension == 0) {
        // Control thread - coordinate layer processing
        // For now, control threads don't do computation
        // They just coordinate the workers
        return 0;
    }
    
    // Worker thread - process token through model layer
    uint32_t dim = model->embedding_dim;
    
    // Allocate output buffer
    double* output = (double*)malloc(dim * sizeof(double));
    if (!output) {
        return -1;
    }
    
    // Get input from activation buffer
    // (This was set by the previous layer or initial embedding)
    double* input = thread->activation_buffer;
    
    // Call through generic interface (NO CLLM dependency!)
    // The model implementation provides the actual computation
    int result = model->forward_layer(
        model->model_data,  // Opaque model data
        thread,             // Thread context
        layer_idx,          // Layer index
        input,              // Input
        output              // Output
    );
    
    if (result != 0) {
        free(output);
        return -1;
    }
    
    // Copy output back to activation buffer for next layer
    memcpy(thread->activation_buffer, output, dim * sizeof(double));
    
    free(output);
    return 0;
}

// ============================================================================
// WORK QUEUE OPERATIONS
// ============================================================================

/**
 * Enqueue work item to thread's work queue
 */
int hierarchical_thread_enqueue_work(
    HierarchicalThread* thread,
    TrainingWorkType type,
    uint32_t token_id,
    uint32_t target_id
) {
    if (!thread) {
        return -1;
    }
    
    // Allocate work item
    TrainingWorkItem* item = (TrainingWorkItem*)malloc(sizeof(TrainingWorkItem));
    if (!item) {
        return -1;
    }
    
    item->type = type;
    item->token_id = token_id;
    item->target_id = target_id;
    item->next = NULL;
    
    // Lock queue
    pthread_mutex_lock(&thread->work_queue_mutex);
    
    // Add to tail
    if (thread->work_queue_tail) {
        thread->work_queue_tail->next = item;
        thread->work_queue_tail = item;
    } else {
        thread->work_queue_head = item;
        thread->work_queue_tail = item;
    }
    
    thread->work_queue_size++;
    
    pthread_mutex_unlock(&thread->work_queue_mutex);
    
    return 0;
}

/**
 * Dequeue work item from thread's work queue
 */
TrainingWorkItem* hierarchical_thread_dequeue_work(HierarchicalThread* thread) {
    if (!thread) {
        return NULL;
    }
    
    pthread_mutex_lock(&thread->work_queue_mutex);
    
    TrainingWorkItem* item = thread->work_queue_head;
    
    if (item) {
        thread->work_queue_head = item->next;
        if (!thread->work_queue_head) {
            thread->work_queue_tail = NULL;
        }
        thread->work_queue_size--;
    }
    
    pthread_mutex_unlock(&thread->work_queue_mutex);
    
    return item;
}

/**
 * Free work item
 */
void hierarchical_thread_free_work_item(TrainingWorkItem* item) {
    if (item) {
        free(item);
    }
}

/**
 * Get work queue size
 */
uint32_t hierarchical_thread_get_work_queue_size(HierarchicalThread* thread) {
    if (!thread) {
        return 0;
    }
    
    pthread_mutex_lock(&thread->work_queue_mutex);
    uint32_t size = thread->work_queue_size;
    pthread_mutex_unlock(&thread->work_queue_mutex);
    
    return size;
}

// ============================================================================
// WORKER LOOP
// ============================================================================

/**
 * Main worker loop for 88D threads
 * This runs continuously, processing work items as they arrive
 */
void* hierarchical_thread_worker(void* arg) {
    HierarchicalThread* thread = (HierarchicalThread*)arg;
    
    if (!thread) {
        return NULL;
    }
    
    // Change state to RUNNING
    hierarchical_thread_change_state(thread, STATE_RUNNING);
    
    // Main worker loop
    while (thread->running) {
        // Wait for work
        pthread_mutex_lock(&thread->control_mutex);
        while (thread->work_queue_size == 0 && thread->running) {
            pthread_cond_wait(&thread->control_cond, &thread->control_mutex);
        }
        pthread_mutex_unlock(&thread->control_mutex);
        
        if (!thread->running) break;
        
        // Process all work items in queue
        uint32_t processed = 0;
        TrainingWorkItem* work;
        
        while ((work = hierarchical_thread_dequeue_work(thread)) != NULL) {
            // Process based on work type
            int result = 0;
            
            switch (work->type) {
                case TRAINING_WORK_TYPE_FORWARD:
                    result = worker_process_forward(thread, work);
                    break;
                    
                case TRAINING_WORK_TYPE_BACKWARD:
                    result = worker_process_backward(thread, work);
                    break;
                    
                default:
                    fprintf(stderr, "WARNING: Unknown work type %d\n", work->type);
                    result = -1;
                    break;
            }
            
            // Free work item
            hierarchical_thread_free_work_item(work);
            
            if (result == 0) {
                processed++;
            }
        }
        
        // Mark work complete
        if (processed > 0) {
            __atomic_add_fetch(&thread->work_completed, processed, __ATOMIC_SEQ_CST);
        }
    }
    
    // Change state to STOPPED
    hierarchical_thread_change_state(thread, STATE_STOPPED);
    
    return NULL;
}

// ============================================================================
// INFERENCE HELPER FUNCTIONS
// ============================================================================

/**
 * Collect logits from Layer 7 threads
 * 
 * This function collects the output values from all Layer 7 worker threads
 * and concatenates them into a single logits array.
 * 
 * @param pool The 88D thread pool
 * @param logits Output array to store logits (must be pre-allocated)
 * @param vocab_size Total vocabulary size
 * @return 0 on success, -1 on error
 */
int collect_logits_from_layer7(HierarchicalThreadPool* pool, 
                                double* logits,
                                uint32_t vocab_size) {
    if (!pool || !logits) {
        fprintf(stderr, "ERROR: Invalid parameters for collect_logits_from_layer7\n");
        return -1;
    }
    
    // Layer 7 has 12 threads (dimensions 0-11)
    // Each thread outputs vocab_size/12 logits
    uint32_t logits_per_thread = vocab_size / 12;
    
    // Collect from each Layer 7 thread
    for (uint8_t dim = 0; dim < 12; dim++) {
        HierarchicalThread* thread = hierarchical_thread_get(pool, 7, dim);
        if (!thread || !thread->activation_buffer) {
            fprintf(stderr, "ERROR: Layer 7 thread %d is NULL or has no activation buffer\n", dim);
            return -1;
        }
        
        // Copy this thread's logits to the output array
        // Offset = dim * logits_per_thread
        uint32_t offset = dim * logits_per_thread;
        
        // Copy values from thread's activation buffer to logits
        for (uint32_t i = 0; i < logits_per_thread && (offset + i) < vocab_size; i++) {
            logits[offset + i] = thread->activation_buffer[i];
        }
    }
    
    return 0;
}

/**
 * Apply temperature scaling to logits
 * 
 * Divides all logits by the temperature parameter to control randomness.
 * Higher temperature = more random, lower temperature = more deterministic.
 * 
 * @param logits The logits array (modified in place)
 * @param vocab_size Size of vocabulary
 * @param temperature Temperature parameter (> 0)
 * @return 0 on success, -1 on error
 */
int apply_temperature_to_logits(double* logits,
                                 uint32_t vocab_size,
                                 double temperature) {
    if (!logits || temperature <= 0.0) {
        fprintf(stderr, "ERROR: Invalid parameters for apply_temperature_to_logits\n");
        return -1;
    }
    
    // Divide each logit by temperature
    for (uint32_t i = 0; i < vocab_size; i++) {
        logits[i] /= temperature;
    }
    
    return 0;
}

/**
 * Apply softmax to logits to get probabilities
 * 
 * Converts logits to probabilities using softmax:
 * P(i) = exp(logit[i]) / sum(exp(logit[j]) for all j)
 * 
 * @param logits The logits array (modified in place to probabilities)
 * @param vocab_size Size of vocabulary
 * @return 0 on success, -1 on error
 */
int apply_softmax_to_logits(double* logits, uint32_t vocab_size) {
    if (!logits) {
        fprintf(stderr, "ERROR: Invalid parameters for apply_softmax_to_logits\n");
        return -1;
    }
    
    // Find max logit for numerical stability
    double max_logit = logits[0];
    for (uint32_t i = 1; i < vocab_size; i++) {
        if (logits[i] > max_logit) {
            max_logit = logits[i];
        }
    }
    
    // Compute exp(logit - max_logit) and sum
    double sum = 0.0;
    for (uint32_t i = 0; i < vocab_size; i++) {
        double exp_value = exp(logits[i] - max_logit);
        logits[i] = exp_value;
        sum += exp_value;
    }
    
    // Normalize to get probabilities
    if (sum > 0.0) {
        for (uint32_t i = 0; i < vocab_size; i++) {
            logits[i] /= sum;
        }
    }
    
    return 0;
}
