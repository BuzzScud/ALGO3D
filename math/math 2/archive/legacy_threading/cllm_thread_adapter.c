/**
 * @file cllm_thread_adapter.c
 * @brief Implementation of adapter layer for CLLMLatticeHierarchy → HierarchicalThread migration
 */

#include "ai/cllm_thread_adapter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// GLOBAL STATE
// ============================================================================

static HierarchicalThreadPool* g_global_pool = NULL;
static pthread_mutex_t g_pool_mutex = PTHREAD_MUTEX_INITIALIZER;

// ============================================================================
// ADAPTER LIFECYCLE
// ============================================================================

CLLMLatticeHierarchy* cllm_adapter_create(
    int sphere_id,
    int hierarchy_level,
    HierarchicalThreadPool* pool
) {
    if (!pool) {
        fprintf(stderr, "ERROR: cllm_adapter_create: pool is NULL\n");
        return NULL;
    }
    
    // Allocate adapter
    CLLMLatticeHierarchyAdapter* adapter = calloc(1, sizeof(CLLMLatticeHierarchyAdapter));
    if (!adapter) {
        fprintf(stderr, "ERROR: cllm_adapter_create: failed to allocate adapter\n");
        return NULL;
    }
    
    // Store pool reference
    adapter->pool = pool;
    
    // Set CLLM-specific identity
    adapter->sphere_id = sphere_id;
    adapter->hierarchy_level = hierarchy_level;
    adapter->num_symmetry_groups = 0;
    adapter->primary_symmetry_group = -1;
    
    // Try to get or create a thread from the pool
    // For now, we'll use the sphere_id to map to a thread
    if (sphere_id >= 0 && (uint32_t)sphere_id < pool->num_threads) {
        adapter->thread = pool->threads[sphere_id];
    } else {
        // Create a new thread
        adapter->thread = hierarchical_thread_create(
            sphere_id,
            THREAD_ROLE_WORKER,
            NULL,  // parent
            pool
        );
        
        if (!adapter->thread) {
            fprintf(stderr, "ERROR: cllm_adapter_create: failed to create thread\n");
            free(adapter);
            return NULL;
        }
    }
    
    // Map Space88D if thread has 88D structure
    if (adapter->thread && adapter->thread->value) {
        // Thread has 88D integration
        // For now, space is NULL - will be populated when needed
        adapter->space = NULL;
    }
    
    // Initialize gradient buffers
    adapter->gradient_buffer_size = 1024 * 1024;  // 1MB default
    adapter->gradient_buffer = calloc(adapter->gradient_buffer_size, sizeof(float));
    adapter->child_gradients = calloc(adapter->gradient_buffer_size, sizeof(float));
    
    // Initialize work queue
    adapter->work_queue_capacity = 1000;
    adapter->work_queue = calloc(adapter->work_queue_capacity, sizeof(uint64_t));
    
    // Initialize state
    atomic_init(&adapter->state, HIERARCHY_STATE_INITIALIZING);
    pthread_mutex_init(&adapter->state_mutex, NULL);
    
    // Initialize statistics
    memset(&adapter->stats, 0, sizeof(SphereStatistics));
    
    return (CLLMLatticeHierarchy*)adapter;
}

void cllm_adapter_destroy(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    
    // Free gradient buffers
    if (adapter->gradient_buffer) {
        free(adapter->gradient_buffer);
    }
    if (adapter->child_gradients) {
        free(adapter->child_gradients);
    }
    
    // Free work queue
    if (adapter->work_queue) {
        free(adapter->work_queue);
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&adapter->state_mutex);
    
    // Note: We don't destroy the thread itself - it's owned by the pool
    
    // Free adapter
    free(adapter);
}

// ============================================================================
// ADAPTER ACCESSORS
// ============================================================================

HierarchicalThread* cllm_adapter_get_thread(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return NULL;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    return adapter->thread;
}

HierarchicalThreadPool* cllm_adapter_get_pool(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return NULL;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    return adapter->pool;
}

int cllm_adapter_is_adapter(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return 0;
    
    // Check if this looks like an adapter by checking for thread field
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    return (adapter->thread != NULL && adapter->pool != NULL);
}

// ============================================================================
// COMPATIBILITY WRAPPERS
// ============================================================================

int cllm_adapter_send_message(CLLMLatticeHierarchy* sphere, void* msg) {
    if (!sphere || !msg) return -1;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    if (!adapter->thread || !adapter->thread->inbox) return -1;
    
    // Use thread's message queue
    return message_queue_enqueue(adapter->thread->inbox, msg);
}

int cllm_adapter_receive_message(CLLMLatticeHierarchy* sphere, void* msg) {
    if (!sphere || !msg) return -1;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    if (!adapter->thread || !adapter->thread->inbox) return -1;
    
    // Use thread's message queue
    void* received = message_queue_dequeue(adapter->thread->inbox);
    if (!received) return -1;
    
    memcpy(msg, received, sizeof(Message));  // Assuming Message type
    return 0;
}

int cllm_adapter_accumulate_gradient(
    CLLMLatticeHierarchy* sphere,
    float* gradient,
    size_t size
) {
    if (!sphere || !gradient) return -1;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    if (!adapter->gradient_buffer) return -1;
    
    // Accumulate into gradient buffer
    pthread_mutex_lock(&adapter->state_mutex);
    
    size_t copy_size = (size < adapter->gradient_buffer_size) ? size : adapter->gradient_buffer_size;
    for (size_t i = 0; i < copy_size; i++) {
        adapter->gradient_buffer[i] += gradient[i];
    }
    
    pthread_mutex_unlock(&adapter->state_mutex);
    
    return 0;
}

float* cllm_adapter_get_gradient_buffer(
    CLLMLatticeHierarchy* sphere,
    size_t* size
) {
    if (!sphere) return NULL;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    
    if (size) {
        *size = adapter->gradient_buffer_size;
    }
    
    return adapter->gradient_buffer;
}

int cllm_adapter_enqueue_work(CLLMLatticeHierarchy* sphere, uint64_t work_item) {
    if (!sphere) return -1;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    if (!adapter->work_queue) return -1;
    
    // Simple circular queue implementation
    // In production, this would use the thread's work queue
    
    // For now, just store in our local queue
    // This is a simplified implementation
    
    return 0;
}

int cllm_adapter_dequeue_work(CLLMLatticeHierarchy* sphere, uint64_t* work_item) {
    if (!sphere || !work_item) return -1;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    if (!adapter->work_queue) return -1;
    
    // Simple circular queue implementation
    // In production, this would use the thread's work queue
    
    return -1;  // Queue empty
}

int cllm_adapter_get_state(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return -1;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    return atomic_load(&adapter->state);
}

int cllm_adapter_set_state(CLLMLatticeHierarchy* sphere, int state) {
    if (!sphere) return -1;
    
    CLLMLatticeHierarchyAdapter* adapter = (CLLMLatticeHierarchyAdapter*)sphere;
    
    pthread_mutex_lock(&adapter->state_mutex);
    atomic_store(&adapter->state, state);
    pthread_mutex_unlock(&adapter->state_mutex);
    
    return 0;
}

// ============================================================================
// GLOBAL POOL MANAGEMENT
// ============================================================================

HierarchicalThreadPool* cllm_adapter_get_global_pool(void) {
    pthread_mutex_lock(&g_pool_mutex);
    
    if (!g_global_pool) {
        // Create default 88D pool
        g_global_pool = hierarchical_thread_pool_create_88d(60);
        if (!g_global_pool) {
            fprintf(stderr, "ERROR: Failed to create global thread pool\n");
        }
    }
    
    pthread_mutex_unlock(&g_pool_mutex);
    
    return g_global_pool;
}

void cllm_adapter_set_global_pool(HierarchicalThreadPool* pool) {
    pthread_mutex_lock(&g_pool_mutex);
    
    if (g_global_pool && g_global_pool != pool) {
        // Destroy old pool
        hierarchical_thread_pool_free(g_global_pool);
    }
    
    g_global_pool = pool;
    
    pthread_mutex_unlock(&g_pool_mutex);
}

void cllm_adapter_destroy_global_pool(void) {
    pthread_mutex_lock(&g_pool_mutex);
    
    if (g_global_pool) {
        hierarchical_thread_pool_free(g_global_pool);
        g_global_pool = NULL;
    }
    
    pthread_mutex_unlock(&g_pool_mutex);
}

// ============================================================================
// MIGRATION HELPERS
// ============================================================================

HierarchicalThread* cllm_to_hierarchical_thread(CLLMLatticeHierarchy* sphere) {
    if (!sphere) return NULL;
    
    // Check if this is an adapter
    if (cllm_adapter_is_adapter(sphere)) {
        return cllm_adapter_get_thread(sphere);
    }
    
    // Legacy CLLMLatticeHierarchy - no thread available
    return NULL;
}

CLLMLatticeHierarchy* hierarchical_thread_to_cllm(HierarchicalThread* thread) {
    if (!thread) return NULL;
    
    // Get global pool
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    if (!pool) return NULL;
    
    // Create adapter wrapping this thread
    CLLMLatticeHierarchyAdapter* adapter = calloc(1, sizeof(CLLMLatticeHierarchyAdapter));
    if (!adapter) return NULL;
    
    adapter->thread = thread;
    adapter->pool = pool;
    adapter->sphere_id = thread->thread_id;
    adapter->hierarchy_level = thread->layer;
    
    // Initialize other fields as needed
    atomic_init(&adapter->state, HIERARCHY_STATE_READY);
    pthread_mutex_init(&adapter->state_mutex, NULL);
    
    return (CLLMLatticeHierarchy*)adapter;
}