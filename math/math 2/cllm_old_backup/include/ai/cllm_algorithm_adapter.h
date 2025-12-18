/**
 * @file cllm_algorithm_adapter.h
 * @brief CLLM Algorithm Library Adapter
 * 
 * This adapter bridges the CLLM-specific threading implementation with the
 * generic algorithm library's hierarchical threading system.
 * 
 * Design Philosophy:
 * - Delegate generic threading operations to algorithm library
 * - Preserve CLLM-specific features (gradients, training, weights)
 * - Maintain backward compatibility with existing CLLM code
 * - Zero performance overhead for delegated operations
 * 
 * Mapping:
 * - CLLMLatticeHierarchy → HierarchicalThread (generic threading)
 * - CLLM-specific data remains in CLLMLatticeHierarchy
 * - Adapter provides translation layer between the two
 */

#ifndef CLLM_ALGORITHM_ADAPTER_H
#define CLLM_ALGORITHM_ADAPTER_H

#include "cllm_lattice_hierarchy.h"
#include "../../../algorithms/include/hierarchical_threading.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ADAPTER STRUCTURE
// ============================================================================

/**
 * CLLM Algorithm Adapter
 * 
 * Connects CLLMLatticeHierarchy with HierarchicalThread.
 * Maintains both structures and provides translation between them.
 */
typedef struct {
    // CLLM-specific structure (contains ML-specific data)
    CLLMLatticeHierarchy* cllm_hierarchy;
    
    // Generic threading structure (from algorithm library)
    HierarchicalThread* generic_thread;
    
    // Thread pool (shared across all adapters)
    HierarchicalThreadPool* thread_pool;
    
    // Adapter metadata
    uint32_t adapter_id;
    bool initialized;
    
} CLLMAlgorithmAdapter;

// ============================================================================
// ADAPTER POOL
// ============================================================================

/**
 * CLLM Adapter Pool
 * 
 * Manages a collection of adapters for all CLLM spheres.
 */
typedef struct {
    // Adapters array
    CLLMAlgorithmAdapter** adapters;
    uint32_t num_adapters;
    uint32_t max_adapters;
    
    // Shared thread pool
    HierarchicalThreadPool* thread_pool;
    
    // Configuration
    uint32_t symmetry_fold;      // 12 for dodecahedron
    uint32_t num_dimensions;     // 13 for CLLM
    
    // Initialization state
    bool initialized;
    
} CLLMAdapterPool;

// ============================================================================
// ADAPTER POOL OPERATIONS
// ============================================================================

/**
 * Create CLLM adapter pool
 * 
 * Creates a pool of adapters for CLLM spheres.
 * 
 * @param num_spheres Number of CLLM spheres
 * @param symmetry_fold Symmetry fold (12 for dodecahedron)
 * @param num_dimensions Number of dimensions (13 for CLLM)
 * @return Allocated adapter pool, or NULL on error
 */
CLLMAdapterPool* cllm_adapter_pool_create(
    uint32_t num_spheres,
    uint32_t symmetry_fold,
    uint32_t num_dimensions
);

/**
 * Free CLLM adapter pool
 * 
 * @param pool Pool to free
 */
void cllm_adapter_pool_free(CLLMAdapterPool* pool);

/**
 * Start adapter pool
 * 
 * Starts all threads in the pool.
 * 
 * @param pool Pool to start
 * @return 0 on success, -1 on error
 */
int cllm_adapter_pool_start(CLLMAdapterPool* pool);

/**
 * Stop adapter pool
 * 
 * Stops all threads in the pool.
 * 
 * @param pool Pool to stop
 * @return 0 on success, -1 on error
 */
int cllm_adapter_pool_stop(CLLMAdapterPool* pool);

// ============================================================================
// ADAPTER OPERATIONS
// ============================================================================

/**
 * Create CLLM adapter
 * 
 * Creates an adapter that connects a CLLMLatticeHierarchy with a
 * HierarchicalThread.
 * 
 * @param cllm_hierarchy CLLM hierarchy structure
 * @param pool Adapter pool
 * @return Allocated adapter, or NULL on error
 */
CLLMAlgorithmAdapter* cllm_adapter_create(
    CLLMLatticeHierarchy* cllm_hierarchy,
    CLLMAdapterPool* pool
);

/**
 * Free CLLM adapter
 * 
 * @param adapter Adapter to free
 */
void cllm_adapter_free(CLLMAlgorithmAdapter* adapter);

/**
 * Initialize adapter
 * 
 * Initializes the adapter and creates the generic thread.
 * 
 * @param adapter Adapter to initialize
 * @return 0 on success, -1 on error
 */
int cllm_adapter_initialize(CLLMAlgorithmAdapter* adapter);

// ============================================================================
// HIERARCHY OPERATIONS (Delegated to Algorithm Library)
// ============================================================================

/**
 * Add neighbor to CLLM sphere
 * 
 * Delegates to hierarchical_thread_add_neighbor.
 * 
 * @param adapter Adapter for this sphere
 * @param neighbor_adapter Adapter for neighbor sphere
 * @param relationship Relationship type
 * @return 0 on success, -1 on error
 */
int cllm_adapter_add_neighbor(
    CLLMAlgorithmAdapter* adapter,
    CLLMAlgorithmAdapter* neighbor_adapter,
    ThreadRelationType relationship
);

/**
 * Get neighbor by sphere ID
 * 
 * Delegates to hierarchical_thread_get_neighbor.
 * 
 * @param adapter Adapter for this sphere
 * @param neighbor_sphere_id Neighbor sphere ID
 * @return Neighbor adapter, or NULL if not found
 */
CLLMAlgorithmAdapter* cllm_adapter_get_neighbor(
    CLLMAlgorithmAdapter* adapter,
    uint32_t neighbor_sphere_id
);

/**
 * Find nearest neighbors
 * 
 * Delegates to hierarchical_thread_find_nearest_neighbors.
 * 
 * @param adapter Adapter for this sphere
 * @param k Number of neighbors to find
 * @param out_neighbor_ids Output buffer for neighbor sphere IDs
 * @return Number of neighbors found
 */
uint32_t cllm_adapter_find_nearest_neighbors(
    CLLMAlgorithmAdapter* adapter,
    uint32_t k,
    uint32_t* out_neighbor_ids
);

// ============================================================================
// STATE OPERATIONS (Delegated to Algorithm Library)
// ============================================================================

/**
 * Change sphere state
 * 
 * Maps CLLM HierarchyState to generic StateType and delegates to
 * hierarchical_thread_change_state.
 * 
 * @param adapter Adapter for this sphere
 * @param cllm_state CLLM-specific state
 * @return Transition result
 */
TransitionResult cllm_adapter_change_state(
    CLLMAlgorithmAdapter* adapter,
    HierarchyState cllm_state
);

/**
 * Get sphere state
 * 
 * Gets generic state and maps back to CLLM HierarchyState.
 * 
 * @param adapter Adapter for this sphere
 * @return CLLM-specific state
 */
HierarchyState cllm_adapter_get_state(CLLMAlgorithmAdapter* adapter);

// ============================================================================
// MESSAGE OPERATIONS (Delegated to Algorithm Library)
// ============================================================================

/**
 * Send message to sphere
 * 
 * Delegates to hierarchical_thread_send_message.
 * 
 * @param sender_adapter Sender adapter
 * @param receiver_sphere_id Receiver sphere ID
 * @param type Message type
 * @param priority Message priority
 * @param data Message data
 * @param data_size Data size in bytes
 * @return 0 on success, -1 on error
 */
int cllm_adapter_send_message(
    CLLMAlgorithmAdapter* sender_adapter,
    uint32_t receiver_sphere_id,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size
);

/**
 * Receive message
 * 
 * Delegates to hierarchical_thread_receive_message.
 * 
 * @param adapter Adapter for this sphere
 * @param timeout_ms Timeout in milliseconds
 * @return Received message, or NULL if none available
 */
Message* cllm_adapter_receive_message(
    CLLMAlgorithmAdapter* adapter,
    int timeout_ms
);

/**
 * Broadcast message to all spheres
 * 
 * Delegates to hierarchical_thread_broadcast_message.
 * 
 * @param sender_adapter Sender adapter
 * @param type Message type
 * @param priority Message priority
 * @param data Message data
 * @param data_size Data size in bytes
 * @return Number of messages sent, or -1 on error
 */
int cllm_adapter_broadcast_message(
    CLLMAlgorithmAdapter* sender_adapter,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size
);

// ============================================================================
// MEMORY OPERATIONS (Delegated to Algorithm Library)
// ============================================================================

/**
 * Allocate local memory
 * 
 * Delegates to hierarchical_thread_alloc_local.
 * 
 * @param adapter Adapter for this sphere
 * @param size Size in bytes
 * @return Allocated memory, or NULL on error
 */
void* cllm_adapter_alloc_local(CLLMAlgorithmAdapter* adapter, size_t size);

/**
 * Access parent shared memory
 * 
 * Delegates to hierarchical_thread_alloc_parent_shared.
 * 
 * @param adapter Adapter for this sphere
 * @param size Size in bytes
 * @param mode Access mode
 * @return Shared memory region, or NULL on error
 */
SharedMemoryEnhanced* cllm_adapter_access_parent_shared(
    CLLMAlgorithmAdapter* adapter,
    size_t size,
    SharedMemoryAccessMode mode
);

/**
 * Access child shared memory
 * 
 * Delegates to hierarchical_thread_alloc_child_shared.
 * 
 * @param adapter Adapter for this sphere
 * @param child_sphere_id Child sphere ID
 * @param size Size in bytes
 * @param mode Access mode
 * @return Shared memory region, or NULL on error
 */
SharedMemoryEnhanced* cllm_adapter_access_child_shared(
    CLLMAlgorithmAdapter* adapter,
    uint32_t child_sphere_id,
    size_t size,
    SharedMemoryAccessMode mode
);

/**
 * Access neighbor boundary
 * 
 * Delegates to hierarchical_thread_access_boundary.
 * 
 * @param adapter Adapter for this sphere
 * @param neighbor_sphere_id Neighbor sphere ID
 * @param mode Access mode
 * @return Boundary memory, or NULL on error
 */
void* cllm_adapter_access_boundary(
    CLLMAlgorithmAdapter* adapter,
    uint32_t neighbor_sphere_id,
    SharedMemoryAccessMode mode
);

// ============================================================================
// CLLM-SPECIFIC OPERATIONS (Not Delegated)
// ============================================================================

/**
 * Allocate gradient buffer
 * 
 * CLLM-specific operation for gradient accumulation.
 * Not delegated to algorithm library.
 * 
 * @param adapter Adapter for this sphere
 * @param size Size of gradient buffer
 * @return 0 on success, -1 on error
 */
int cllm_adapter_alloc_gradient_buffer(
    CLLMAlgorithmAdapter* adapter,
    size_t size
);

/**
 * Accumulate gradients from child
 * 
 * CLLM-specific operation for gradient accumulation.
 * Not delegated to algorithm library.
 * 
 * @param adapter Adapter for this sphere
 * @param child_adapter Child adapter
 * @return 0 on success, -1 on error
 */
int cllm_adapter_accumulate_child_gradients(
    CLLMAlgorithmAdapter* adapter,
    CLLMAlgorithmAdapter* child_adapter
);

/**
 * Update weights
 * 
 * CLLM-specific operation for weight updates.
 * Not delegated to algorithm library.
 * 
 * @param adapter Adapter for this sphere
 * @param learning_rate Learning rate
 * @return 0 on success, -1 on error
 */
int cllm_adapter_update_weights(
    CLLMAlgorithmAdapter* adapter,
    float learning_rate
);

/**
 * Forward pass
 * 
 * CLLM-specific operation for forward propagation.
 * Not delegated to algorithm library.
 * 
 * @param adapter Adapter for this sphere
 * @param input Input data
 * @param output Output buffer
 * @return 0 on success, -1 on error
 */
int cllm_adapter_forward_pass(
    CLLMAlgorithmAdapter* adapter,
    const float* input,
    float* output
);

/**
 * Backward pass
 * 
 * CLLM-specific operation for backward propagation.
 * Not delegated to algorithm library.
 * 
 * @param adapter Adapter for this sphere
 * @param grad_output Gradient of output
 * @param grad_input Gradient of input (output)
 * @return 0 on success, -1 on error
 */
int cllm_adapter_backward_pass(
    CLLMAlgorithmAdapter* adapter,
    const float* grad_output,
    float* grad_input
);

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

/**
 * Adapter statistics
 */
typedef struct {
    uint32_t adapter_id;
    uint32_t sphere_id;
    
    // Generic thread statistics (from algorithm library)
    HierarchicalThreadStats thread_stats;
    
    // CLLM-specific statistics
    uint64_t primes_generated;
    uint64_t gradients_accumulated;
    uint64_t weights_updated;
    uint64_t forward_passes;
    uint64_t backward_passes;
    
} CLLMAdapterStats;

/**
 * Get adapter statistics
 * 
 * @param adapter Adapter to query
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int cllm_adapter_get_stats(
    CLLMAlgorithmAdapter* adapter,
    CLLMAdapterStats* stats
);

/**
 * Print adapter statistics
 * 
 * @param adapter Adapter to print stats for
 */
void cllm_adapter_print_stats(CLLMAlgorithmAdapter* adapter);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get adapter by sphere ID
 * 
 * @param pool Adapter pool
 * @param sphere_id Sphere ID to find
 * @return Adapter, or NULL if not found
 */
CLLMAlgorithmAdapter* cllm_adapter_pool_get_adapter(
    CLLMAdapterPool* pool,
    uint32_t sphere_id
);

/**
 * Map CLLM state to generic state
 * 
 * @param cllm_state CLLM-specific state
 * @return Generic state type
 */
StateType cllm_state_to_generic(HierarchyState cllm_state);

/**
 * Map generic state to CLLM state
 * 
 * @param generic_state Generic state type
 * @return CLLM-specific state
 */
HierarchyState generic_state_to_cllm(StateType generic_state);

#ifdef __cplusplus
}
#endif

#endif // CLLM_ALGORITHM_ADAPTER_H