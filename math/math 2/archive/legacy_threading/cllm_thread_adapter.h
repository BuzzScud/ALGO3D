#ifndef CLLM_THREAD_ADAPTER_H
#define CLLM_THREAD_ADAPTER_H

/**
 * @file cllm_thread_adapter.h
 * @brief Adapter layer for gradual migration from CLLMLatticeHierarchy to HierarchicalThread
 * 
 * This adapter allows existing CLLM code to continue working while gradually
 * migrating to the new unified HierarchicalThread system.
 */

#include <stdint.h>
#include <pthread.h>
#include "../../algorithms/include/hierarchical_threading.h"
#include "cllm_lattice_hierarchy.h"

// ============================================================================
// ADAPTER STRUCTURE
// ============================================================================

/**
 * Adapter that wraps HierarchicalThread to provide CLLMLatticeHierarchy interface
 * 
 * This allows existing code to use the new threading system without modification.
 */
typedef struct CLLMLatticeHierarchyAdapter {
    // Core thread (from new system)
    HierarchicalThread* thread;
    HierarchicalThreadPool* pool;
    
    // CLLM-specific identity
    int sphere_id;
    int hierarchy_level;
    int symmetry_groups[12];
    int num_symmetry_groups;
    int primary_symmetry_group;
    
    // Legacy compatibility pointers
    Space88D* space;  // Points to thread's 88D space (if available)
    
    // Statistics (mapped from thread stats)
    SphereStatistics stats;
    
    // Gradient buffers (mapped from thread gradient buffer)
    float* gradient_buffer;
    size_t gradient_buffer_size;
    float* child_gradients;
    
    // Work queue (mapped from thread work queue)
    uint64_t* work_queue;
    size_t work_queue_capacity;
    
    // State tracking
    atomic_int state;
    pthread_mutex_t state_mutex;
    
} CLLMLatticeHierarchyAdapter;

// ============================================================================
// ADAPTER LIFECYCLE
// ============================================================================

/**
 * Create adapter wrapping a HierarchicalThread
 * 
 * @param sphere_id CLLM sphere ID
 * @param hierarchy_level Level in hierarchy (0 = root)
 * @param pool Thread pool to use
 * @return Adapter instance, or NULL on error
 */
CLLMLatticeHierarchy* cllm_adapter_create(
    int sphere_id,
    int hierarchy_level,
    HierarchicalThreadPool* pool
);

/**
 * Destroy adapter
 * 
 * @param adapter Adapter to destroy
 */
void cllm_adapter_destroy(CLLMLatticeHierarchy* adapter);

// ============================================================================
// ADAPTER ACCESSORS
// ============================================================================

/**
 * Get underlying HierarchicalThread from adapter
 * 
 * @param adapter Adapter instance
 * @return Underlying thread, or NULL if not an adapter
 */
HierarchicalThread* cllm_adapter_get_thread(CLLMLatticeHierarchy* adapter);

/**
 * Get thread pool from adapter
 * 
 * @param adapter Adapter instance
 * @return Thread pool, or NULL if not an adapter
 */
HierarchicalThreadPool* cllm_adapter_get_pool(CLLMLatticeHierarchy* adapter);

/**
 * Check if a CLLMLatticeHierarchy is an adapter
 * 
 * @param sphere Sphere to check
 * @return 1 if adapter, 0 otherwise
 */
int cllm_adapter_is_adapter(CLLMLatticeHierarchy* sphere);

// ============================================================================
// COMPATIBILITY WRAPPERS
// ============================================================================

/**
 * Send message (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @param msg Message to send
 * @return 0 on success, -1 on error
 */
int cllm_adapter_send_message(CLLMLatticeHierarchy* adapter, void* msg);

/**
 * Receive message (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @param msg Buffer for received message
 * @return 0 on success, -1 on error
 */
int cllm_adapter_receive_message(CLLMLatticeHierarchy* adapter, void* msg);

/**
 * Accumulate gradient (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @param gradient Gradient to accumulate
 * @param size Size of gradient buffer
 * @return 0 on success, -1 on error
 */
int cllm_adapter_accumulate_gradient(
    CLLMLatticeHierarchy* adapter,
    float* gradient,
    size_t size
);

/**
 * Get gradient buffer (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @param size Output: size of gradient buffer
 * @return Gradient buffer, or NULL on error
 */
float* cllm_adapter_get_gradient_buffer(
    CLLMLatticeHierarchy* adapter,
    size_t* size
);

/**
 * Enqueue work (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @param work_item Work item to enqueue
 * @return 0 on success, -1 on error
 */
int cllm_adapter_enqueue_work(CLLMLatticeHierarchy* adapter, uint64_t work_item);

/**
 * Dequeue work (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @param work_item Output: dequeued work item
 * @return 0 on success, -1 if queue empty
 */
int cllm_adapter_dequeue_work(CLLMLatticeHierarchy* adapter, uint64_t* work_item);

/**
 * Get state (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @return Current state
 */
int cllm_adapter_get_state(CLLMLatticeHierarchy* adapter);

/**
 * Set state (compatibility wrapper)
 * 
 * @param adapter Adapter instance
 * @param state New state
 * @return 0 on success, -1 on error
 */
int cllm_adapter_set_state(CLLMLatticeHierarchy* adapter, int state);

// ============================================================================
// GLOBAL POOL MANAGEMENT
// ============================================================================

/**
 * Get or create global thread pool for CLLM
 * 
 * @return Global thread pool
 */
HierarchicalThreadPool* cllm_adapter_get_global_pool(void);

/**
 * Set global thread pool for CLLM
 * 
 * @param pool Thread pool to use globally
 */
void cllm_adapter_set_global_pool(HierarchicalThreadPool* pool);

/**
 * Destroy global thread pool
 */
void cllm_adapter_destroy_global_pool(void);

// ============================================================================
// MIGRATION HELPERS
// ============================================================================

/**
 * Convert CLLMLatticeHierarchy to HierarchicalThread (for new code)
 * 
 * @param sphere CLLM sphere (may be adapter or legacy)
 * @return HierarchicalThread if available, NULL otherwise
 */
HierarchicalThread* cllm_to_hierarchical_thread(CLLMLatticeHierarchy* sphere);

/**
 * Convert HierarchicalThread to CLLMLatticeHierarchy (for legacy code)
 * 
 * @param thread HierarchicalThread
 * @return CLLMLatticeHierarchy adapter, or NULL on error
 */
CLLMLatticeHierarchy* hierarchical_thread_to_cllm(HierarchicalThread* thread);

#endif // CLLM_THREAD_ADAPTER_H