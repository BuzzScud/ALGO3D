/**
 * @file cllm_algorithm_adapter.c
 * @brief CLLM Algorithm Library Adapter Implementation
 */

#include "ai/cllm_algorithm_adapter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// STATE MAPPING
// ============================================================================

StateType cllm_state_to_generic(HierarchyState cllm_state) {
    switch (cllm_state) {
        case HIERARCHY_STATE_INITIALIZING:
            return STATE_INITIALIZING;
        case HIERARCHY_STATE_READY:
            return STATE_READY;
        case HIERARCHY_STATE_PROCESSING:
        case HIERARCHY_STATE_CONTROLLING:
            return STATE_RUNNING;
        case HIERARCHY_STATE_WAITING:
            return STATE_WAITING;
        case HIERARCHY_STATE_ACCUMULATING:
        case HIERARCHY_STATE_UPDATING:
            return STATE_WORKING;
        case HIERARCHY_STATE_IDLE:
            return STATE_IDLE;
        case HIERARCHY_STATE_TERMINATING:
            return STATE_STOPPING;
        case HIERARCHY_STATE_TERMINATED:
            return STATE_STOPPED;
        default:
            return STATE_ERROR;
    }
}

HierarchyState generic_state_to_cllm(StateType generic_state) {
    switch (generic_state) {
        case STATE_UNINITIALIZED:
        case STATE_INITIALIZING:
            return HIERARCHY_STATE_INITIALIZING;
        case STATE_INITIALIZED:
        case STATE_READY:
            return HIERARCHY_STATE_READY;
        case STATE_RUNNING:
            return HIERARCHY_STATE_PROCESSING;
        case STATE_WAITING:
            return HIERARCHY_STATE_WAITING;
        case STATE_WORKING:
            return HIERARCHY_STATE_PROCESSING;
        case STATE_IDLE:
            return HIERARCHY_STATE_IDLE;
        case STATE_STOPPING:
            return HIERARCHY_STATE_TERMINATING;
        case STATE_STOPPED:
            return HIERARCHY_STATE_TERMINATED;
        case STATE_ERROR:
        default:
            return HIERARCHY_STATE_IDLE;
    }
}

// ============================================================================
// ADAPTER POOL OPERATIONS
// ============================================================================

CLLMAdapterPool* cllm_adapter_pool_create(
    uint32_t num_spheres,
    uint32_t symmetry_fold,
    uint32_t num_dimensions
) {
    if (num_spheres == 0) {
        return NULL;
    }
    
    CLLMAdapterPool* pool = calloc(1, sizeof(CLLMAdapterPool));
    if (!pool) {
        return NULL;
    }
    
    pool->num_adapters = 0;
    pool->max_adapters = num_spheres;
    pool->symmetry_fold = symmetry_fold;
    pool->num_dimensions = num_dimensions;
    pool->initialized = false;
    
    // Allocate adapters array
    pool->adapters = calloc(num_spheres, sizeof(CLLMAlgorithmAdapter*));
    if (!pool->adapters) {
        free(pool);
        return NULL;
    }
    
    // Create thread pool
    pool->thread_pool = hierarchical_thread_pool_create(
        num_spheres,
        symmetry_fold,
        num_dimensions,
        false  // not NUMA-aware for now
    );
    
    if (!pool->thread_pool) {
        free(pool->adapters);
        free(pool);
        return NULL;
    }
    
    pool->initialized = true;
    return pool;
}

void cllm_adapter_pool_free(CLLMAdapterPool* pool) {
    if (!pool) {
        return;
    }
    
    // Free all adapters
    for (uint32_t i = 0; i < pool->num_adapters; i++) {
        if (pool->adapters[i]) {
            cllm_adapter_free(pool->adapters[i]);
        }
    }
    
    // Free thread pool
    if (pool->thread_pool) {
        hierarchical_thread_pool_free(pool->thread_pool);
    }
    
    // Free adapters array
    free(pool->adapters);
    
    free(pool);
}

int cllm_adapter_pool_start(CLLMAdapterPool* pool) {
    if (!pool || !pool->initialized) {
        return -1;
    }
    
    return hierarchical_thread_pool_start(pool->thread_pool);
}

int cllm_adapter_pool_stop(CLLMAdapterPool* pool) {
    if (!pool || !pool->initialized) {
        return -1;
    }
    
    return hierarchical_thread_pool_stop(pool->thread_pool);
}

// ============================================================================
// ADAPTER OPERATIONS
// ============================================================================

CLLMAlgorithmAdapter* cllm_adapter_create(
    CLLMLatticeHierarchy* cllm_hierarchy,
    CLLMAdapterPool* pool
) {
    if (!cllm_hierarchy || !pool) {
        return NULL;
    }
    
    CLLMAlgorithmAdapter* adapter = calloc(1, sizeof(CLLMAlgorithmAdapter));
    if (!adapter) {
        return NULL;
    }
    
    adapter->cllm_hierarchy = cllm_hierarchy;
    adapter->thread_pool = pool->thread_pool;
    adapter->adapter_id = pool->num_adapters;
    adapter->initialized = false;
    
    // Add to pool
    if (pool->num_adapters < pool->max_adapters) {
        pool->adapters[pool->num_adapters++] = adapter;
    }
    
    return adapter;
}

void cllm_adapter_free(CLLMAlgorithmAdapter* adapter) {
    if (!adapter) {
        return;
    }
    
    // Free generic thread (if created)
    if (adapter->generic_thread) {
        hierarchical_thread_free(adapter->generic_thread);
    }
    
    // Note: We don't free cllm_hierarchy as it's owned by CLLM
    
    free(adapter);
}

int cllm_adapter_initialize(CLLMAlgorithmAdapter* adapter) {
    if (!adapter || !adapter->cllm_hierarchy || !adapter->thread_pool) {
        return -1;
    }
    
    // Determine thread role based on hierarchy level
    ThreadRole role;
    if (adapter->cllm_hierarchy->hierarchy_level == 0) {
        role = THREAD_ROLE_CONTROL;
    } else if (adapter->cllm_hierarchy->num_children > 0) {
        role = THREAD_ROLE_MANAGER;
    } else {
        role = THREAD_ROLE_WORKER;
    }
    
    // Find parent adapter if exists
    HierarchicalThread* parent_thread = NULL;
    if (adapter->cllm_hierarchy->parent) {
        // Find parent adapter in pool
        CLLMAdapterPool* pool = (CLLMAdapterPool*)adapter->thread_pool;
        for (uint32_t i = 0; i < pool->num_adapters; i++) {
            if (pool->adapters[i]->cllm_hierarchy == adapter->cllm_hierarchy->parent) {
                parent_thread = pool->adapters[i]->generic_thread;
                break;
            }
        }
    }
    
    // Create generic thread
    adapter->generic_thread = hierarchical_thread_create(
        adapter->cllm_hierarchy->sphere_id,
        role,
        parent_thread,
        adapter->thread_pool
    );
    
    if (!adapter->generic_thread) {
        return -1;
    }
    
    adapter->initialized = true;
    return 0;
}

// ============================================================================
// HIERARCHY OPERATIONS
// ============================================================================

int cllm_adapter_add_neighbor(
    CLLMAlgorithmAdapter* adapter,
    CLLMAlgorithmAdapter* neighbor_adapter,
    ThreadRelationType relationship
) {
    if (!adapter || !neighbor_adapter || !adapter->generic_thread || !neighbor_adapter->generic_thread) {
        return -1;
    }
    
    // Calculate distance between spheres
    double distance = hierarchical_thread_distance(
        adapter->generic_thread,
        neighbor_adapter->generic_thread
    );
    
    return hierarchical_thread_add_neighbor(
        adapter->generic_thread,
        neighbor_adapter->cllm_hierarchy->sphere_id,
        relationship,
        distance,
        adapter->thread_pool
    );
}

CLLMAlgorithmAdapter* cllm_adapter_get_neighbor(
    CLLMAlgorithmAdapter* adapter,
    uint32_t neighbor_sphere_id
) {
    if (!adapter || !adapter->generic_thread) {
        return NULL;
    }
    
    ThreadNeighbor* neighbor = hierarchical_thread_get_neighbor(
        adapter->generic_thread,
        neighbor_sphere_id
    );
    
    if (!neighbor) {
        return NULL;
    }
    
    // Find adapter in pool
    // Note: This requires access to the pool, which we don't have directly
    // In a real implementation, we'd need to pass the pool or store it
    return NULL;
}

uint32_t cllm_adapter_find_nearest_neighbors(
    CLLMAlgorithmAdapter* adapter,
    uint32_t k,
    uint32_t* out_neighbor_ids
) {
    if (!adapter || !adapter->generic_thread || !out_neighbor_ids) {
        return 0;
    }
    
    return hierarchical_thread_find_nearest_neighbors(
        adapter->generic_thread,
        adapter->thread_pool,
        k,
        out_neighbor_ids
    );
}

// ============================================================================
// STATE OPERATIONS
// ============================================================================

TransitionResult cllm_adapter_change_state(
    CLLMAlgorithmAdapter* adapter,
    HierarchyState cllm_state
) {
    if (!adapter || !adapter->generic_thread) {
        return TRANSITION_ERROR;
    }
    
    // Map CLLM state to generic state
    StateType generic_state = cllm_state_to_generic(cllm_state);
    
    // Delegate to generic thread
    TransitionResult result = hierarchical_thread_change_state(
        adapter->generic_thread,
        generic_state
    );
    
    // Update CLLM state if successful
    if (result == TRANSITION_SUCCESS) {
        atomic_store(&adapter->cllm_hierarchy->state, cllm_state);
    }
    
    return result;
}

HierarchyState cllm_adapter_get_state(CLLMAlgorithmAdapter* adapter) {
    if (!adapter || !adapter->generic_thread) {
        return HIERARCHY_STATE_IDLE;
    }
    
    // Get generic state
    StateType generic_state = hierarchical_thread_get_state(adapter->generic_thread);
    
    // Map back to CLLM state
    return generic_state_to_cllm(generic_state);
}

// ============================================================================
// MESSAGE OPERATIONS
// ============================================================================

int cllm_adapter_send_message(
    CLLMAlgorithmAdapter* sender_adapter,
    uint32_t receiver_sphere_id,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size
) {
    if (!sender_adapter || !sender_adapter->generic_thread) {
        return -1;
    }
    
    int result = hierarchical_thread_send_message(
        sender_adapter->generic_thread,
        receiver_sphere_id,
        type,
        priority,
        data,
        data_size,
        sender_adapter->thread_pool
    );
    
    if (result == 0) {
        atomic_fetch_add(&sender_adapter->cllm_hierarchy->messages_sent, 1);
    }
    
    return result;
}

Message* cllm_adapter_receive_message(
    CLLMAlgorithmAdapter* adapter,
    int timeout_ms
) {
    if (!adapter || !adapter->generic_thread) {
        return NULL;
    }
    
    Message* msg = hierarchical_thread_receive_message(
        adapter->generic_thread,
        timeout_ms
    );
    
    if (msg) {
        atomic_fetch_add(&adapter->cllm_hierarchy->messages_received, 1);
    }
    
    return msg;
}

int cllm_adapter_broadcast_message(
    CLLMAlgorithmAdapter* sender_adapter,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size
) {
    if (!sender_adapter || !sender_adapter->generic_thread) {
        return -1;
    }
    
    int sent = hierarchical_thread_broadcast_message(
        sender_adapter->generic_thread,
        type,
        priority,
        data,
        data_size,
        sender_adapter->thread_pool
    );
    
    if (sent > 0) {
        atomic_fetch_add(&sender_adapter->cllm_hierarchy->messages_sent, sent);
    }
    
    return sent;
}

// ============================================================================
// MEMORY OPERATIONS
// ============================================================================

void* cllm_adapter_alloc_local(CLLMAlgorithmAdapter* adapter, size_t size) {
    if (!adapter || !adapter->generic_thread) {
        return NULL;
    }
    
    return hierarchical_thread_alloc_local(adapter->generic_thread, size);
}

SharedMemoryEnhanced* cllm_adapter_access_parent_shared(
    CLLMAlgorithmAdapter* adapter,
    size_t size,
    SharedMemoryAccessMode mode
) {
    if (!adapter || !adapter->generic_thread) {
        return NULL;
    }
    
    return hierarchical_thread_alloc_parent_shared(
        adapter->generic_thread,
        size,
        mode
    );
}

SharedMemoryEnhanced* cllm_adapter_access_child_shared(
    CLLMAlgorithmAdapter* adapter,
    uint32_t child_sphere_id,
    size_t size,
    SharedMemoryAccessMode mode
) {
    if (!adapter || !adapter->generic_thread) {
        return NULL;
    }
    
    return hierarchical_thread_alloc_child_shared(
        adapter->generic_thread,
        child_sphere_id,
        size,
        mode
    );
}

void* cllm_adapter_access_boundary(
    CLLMAlgorithmAdapter* adapter,
    uint32_t neighbor_sphere_id,
    SharedMemoryAccessMode mode
) {
    if (!adapter || !adapter->generic_thread) {
        return NULL;
    }
    
    return hierarchical_thread_access_boundary(
        adapter->generic_thread,
        neighbor_sphere_id,
        mode
    );
}

// ============================================================================
// CLLM-SPECIFIC OPERATIONS
// ============================================================================

int cllm_adapter_alloc_gradient_buffer(
    CLLMAlgorithmAdapter* adapter,
    size_t size
) {
    if (!adapter || !adapter->cllm_hierarchy) {
        return -1;
    }
    
    CLLMLatticeHierarchy* hierarchy = adapter->cllm_hierarchy;
    
    // Allocate gradient buffer
    hierarchy->gradient_buffer = calloc(size, sizeof(float));
    if (!hierarchy->gradient_buffer) {
        return -1;
    }
    
    hierarchy->gradient_buffer_size = size;
    atomic_store(&hierarchy->gradient_ready, 0);
    
    // Allocate child gradients buffer
    hierarchy->child_gradients = calloc(size, sizeof(float));
    if (!hierarchy->child_gradients) {
        free(hierarchy->gradient_buffer);
        hierarchy->gradient_buffer = NULL;
        return -1;
    }
    
    atomic_store(&hierarchy->children_gradients_ready, 0);
    
    return 0;
}

int cllm_adapter_accumulate_child_gradients(
    CLLMAlgorithmAdapter* adapter,
    CLLMAlgorithmAdapter* child_adapter
) {
    if (!adapter || !child_adapter || 
        !adapter->cllm_hierarchy || !child_adapter->cllm_hierarchy) {
        return -1;
    }
    
    CLLMLatticeHierarchy* parent = adapter->cllm_hierarchy;
    CLLMLatticeHierarchy* child = child_adapter->cllm_hierarchy;
    
    // Check if child gradients are ready
    if (atomic_load(&child->gradient_ready) == 0) {
        return -1;
    }
    
    // Accumulate gradients
    pthread_mutex_lock(&parent->gradient_mutex);
    
    for (size_t i = 0; i < parent->gradient_buffer_size && i < child->gradient_buffer_size; i++) {
        parent->child_gradients[i] += child->gradient_buffer[i];
    }
    
    atomic_fetch_add(&parent->children_gradients_ready, 1);
    
    pthread_mutex_unlock(&parent->gradient_mutex);
    
    return 0;
}

int cllm_adapter_update_weights(
    CLLMAlgorithmAdapter* adapter,
    float learning_rate
) {
    if (!adapter || !adapter->cllm_hierarchy) {
        return -1;
    }
    
    CLLMLatticeHierarchy* hierarchy = adapter->cllm_hierarchy;
    
    // This is a placeholder - actual weight update would depend on CLLM's
    // weight storage structure
    (void)learning_rate;
    
    // Change state to UPDATING
    cllm_adapter_change_state(adapter, HIERARCHY_STATE_UPDATING);
    
    // Actual weight update logic would go here
    
    // Change state back to READY
    cllm_adapter_change_state(adapter, HIERARCHY_STATE_READY);
    
    return 0;
}

int cllm_adapter_forward_pass(
    CLLMAlgorithmAdapter* adapter,
    const float* input,
    float* output
) {
    if (!adapter || !adapter->cllm_hierarchy || !input || !output) {
        return -1;
    }
    
    // This is a placeholder - actual forward pass would depend on CLLM's
    // network architecture
    
    // Change state to PROCESSING
    cllm_adapter_change_state(adapter, HIERARCHY_STATE_PROCESSING);
    
    // Actual forward pass logic would go here
    
    // Change state back to READY
    cllm_adapter_change_state(adapter, HIERARCHY_STATE_READY);
    
    return 0;
}

int cllm_adapter_backward_pass(
    CLLMAlgorithmAdapter* adapter,
    const float* grad_output,
    float* grad_input
) {
    if (!adapter || !adapter->cllm_hierarchy || !grad_output || !grad_input) {
        return -1;
    }
    
    CLLMLatticeHierarchy* hierarchy = adapter->cllm_hierarchy;
    
    // Change state to ACCUMULATING
    cllm_adapter_change_state(adapter, HIERARCHY_STATE_ACCUMULATING);
    
    // Actual backward pass logic would go here
    
    // Mark gradients as ready
    atomic_store(&hierarchy->gradient_ready, 1);
    
    // Change state back to READY
    cllm_adapter_change_state(adapter, HIERARCHY_STATE_READY);
    
    return 0;
}

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

int cllm_adapter_get_stats(
    CLLMAlgorithmAdapter* adapter,
    CLLMAdapterStats* stats
) {
    if (!adapter || !stats || !adapter->generic_thread || !adapter->cllm_hierarchy) {
        return -1;
    }
    
    memset(stats, 0, sizeof(CLLMAdapterStats));
    
    stats->adapter_id = adapter->adapter_id;
    stats->sphere_id = adapter->cllm_hierarchy->sphere_id;
    
    // Get generic thread statistics
    hierarchical_thread_get_stats(adapter->generic_thread, &stats->thread_stats);
    
    // Get CLLM-specific statistics
    stats->primes_generated = adapter->cllm_hierarchy->primes_generated;
    stats->gradients_accumulated = 0;  // Would need to track this
    stats->weights_updated = 0;        // Would need to track this
    stats->forward_passes = 0;         // Would need to track this
    stats->backward_passes = 0;        // Would need to track this
    
    return 0;
}

void cllm_adapter_print_stats(CLLMAlgorithmAdapter* adapter) {
    if (!adapter) {
        return;
    }
    
    CLLMAdapterStats stats;
    if (cllm_adapter_get_stats(adapter, &stats) != 0) {
        return;
    }
    
    printf("CLLM Adapter %u (Sphere %u) Statistics:\n", 
           stats.adapter_id, stats.sphere_id);
    printf("  Generic Thread Stats:\n");
    printf("    Role: %d\n", stats.thread_stats.role);
    printf("    State: %d\n", stats.thread_stats.current_state);
    printf("    Messages: sent=%lu, received=%lu\n",
           stats.thread_stats.messages_sent, stats.thread_stats.messages_received);
    printf("    Work: completed=%lu, stolen=%lu\n",
           stats.thread_stats.work_completed, stats.thread_stats.work_stolen);
    printf("  CLLM-Specific Stats:\n");
    printf("    Primes generated: %lu\n", stats.primes_generated);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

CLLMAlgorithmAdapter* cllm_adapter_pool_get_adapter(
    CLLMAdapterPool* pool,
    uint32_t sphere_id
) {
    if (!pool) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < pool->num_adapters; i++) {
        if (pool->adapters[i]->cllm_hierarchy->sphere_id == (int)sphere_id) {
            return pool->adapters[i];
        }
    }
    
    return NULL;
}