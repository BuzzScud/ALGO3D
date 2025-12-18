/**
 * @file cllm_88d_integration.h
 * @brief CLLM Integration with 88D Unified Threading System
 * 
 * CLEAN INTEGRATION - NO LEGACY SUPPORT
 * 
 * This file provides CLLM-specific helpers for integrating with the
 * algorithms library's unified 88D threading system. It does NOT wrap
 * or adapt the threading system - it provides convenience functions
 * for CLLM to use the threading system directly.
 * 
 * Design Philosophy:
 * - Direct use of HierarchicalThreadPool from algorithms library
 * - Automatic geometry-to-thread mapping based on Platonic solids
 * - Work distribution helpers for ML operations
 * - No adapters, no wrappers, no legacy support
 * 
 * Key Functions:
 * - Initialize 88D threading for a CLLM model
 * - Map Platonic solid geometry to thread topology
 * - Distribute ML workloads across 88D threads
 * - Synchronize threads for training/inference
 * - Cleanup threading resources
 */

#ifndef CLLM_88D_INTEGRATION_H
#define CLLM_88D_INTEGRATION_H

#include "cllm.h"
#include "hierarchical_threading.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// WORK TYPES FOR ML OPERATIONS
// ============================================================================

/**
 * Types of work that can be distributed across 88D threads
 */
typedef enum {
    CLLM_WORK_EMBEDDING_LOOKUP,      // Parallel embedding lookup
    CLLM_WORK_ATTENTION_QKV,         // Compute Q, K, V matrices
    CLLM_WORK_ATTENTION_SCORES,      // Compute attention scores
    CLLM_WORK_ATTENTION_OUTPUT,      // Apply attention and output projection
    CLLM_WORK_FFN_FORWARD,           // Feed-forward network forward pass
    CLLM_WORK_FFN_BACKWARD,          // Feed-forward network backward pass
    CLLM_WORK_LAYER_NORM,            // Layer normalization
    CLLM_WORK_GRADIENT_ACCUMULATION, // Accumulate gradients across threads
    CLLM_WORK_WEIGHT_UPDATE,         // Update weights with optimizer
    CLLM_WORK_LOSS_COMPUTATION,      // Compute loss
    CLLM_WORK_CUSTOM                 // Custom work type
} CLLMWorkType;

/**
 * Work item for ML operations
 */
typedef struct {
    CLLMWorkType type;               // Type of work
    void* data;                      // Work-specific data
    uint32_t start_idx;              // Start index (e.g., token, head, layer)
    uint32_t end_idx;                // End index
    uint32_t batch_idx;              // Batch index (if applicable)
    uint32_t layer_idx;              // Layer index (if applicable)
} CLLMWorkItem;

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

/**
 * Initialize 88D threading for a CLLM model
 * 
 * This function:
 * 1. Creates a HierarchicalThreadPool with 96 threads (8 layers × 12 threads)
 * 2. Maps the model's Platonic solid geometry to thread topology
 * 3. Sets up work queues and work stealing pools
 * 4. Initializes shared memory regions for thread communication
 * 
 * @param model CLLM model to initialize threading for
 * @return true on success, false on failure
 */
bool cllm_initialize_88d_threading(CLLMModel* model);

/**
 * Cleanup 88D threading for a CLLM model
 * 
 * This function:
 * 1. Stops all threads gracefully
 * 2. Frees work queues and work stealing pools
 * 3. Releases shared memory regions
 * 4. Destroys the thread pool
 * 5. Frees geometry mappings
 * 
 * @param model CLLM model to cleanup threading for
 */
void cllm_cleanup_88d_threading(CLLMModel* model);

// ============================================================================
// GEOMETRY MAPPING
// ============================================================================

/**
 * Map Platonic solid geometry to 88D thread topology
 * 
 * This function creates mappings between the model's geometric structure
 * and the 88D thread pool:
 * - Vertices → Threads (for embedding operations)
 * - Edges → Shared boundaries (for communication)
 * - Faces → Layers (for hierarchical operations)
 * - Tokens → Threads (for parallel processing)
 * 
 * The mapping is deterministic and based on the Platonic solid's symmetry.
 * 
 * @param model CLLM model with initialized geometry
 * @return true on success, false on failure
 */
bool cllm_map_geometry_to_threads(CLLMModel* model);

/**
 * Get thread ID for a specific vertex
 * 
 * @param model CLLM model
 * @param vertex_idx Vertex index
 * @return Thread ID (0-95) or UINT32_MAX on error
 */
uint32_t cllm_get_thread_for_vertex(const CLLMModel* model, uint32_t vertex_idx);

/**
 * Get thread ID for a specific token
 * 
 * @param model CLLM model
 * @param token_id Token ID
 * @return Thread ID (0-95) or UINT32_MAX on error
 */
uint32_t cllm_get_thread_for_token(const CLLMModel* model, uint32_t token_id);

/**
 * Get layer ID for a specific face
 * 
 * @param model CLLM model
 * @param face_idx Face index
 * @return Layer ID (0-7) or UINT32_MAX on error
 */
uint32_t cllm_get_layer_for_face(const CLLMModel* model, uint32_t face_idx);

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

/**
 * Distribute work across 88D threads
 * 
 * This function takes a work type and distributes it across the thread pool.
 * Work is automatically balanced based on thread load and geometry.
 * 
 * @param model CLLM model
 * @param work_type Type of work to distribute
 * @param work_data Work-specific data
 * @param num_items Number of work items
 * @return true on success, false on failure
 */
bool cllm_distribute_work_88d(CLLMModel* model, CLLMWorkType work_type, 
                              void* work_data, uint32_t num_items);

/**
 * Submit a single work item to the thread pool
 * 
 * @param model CLLM model
 * @param work_item Work item to submit
 * @return true on success, false on failure
 */
bool cllm_submit_work_item(CLLMModel* model, const CLLMWorkItem* work_item);

/**
 * Wait for all work to complete
 * 
 * This function blocks until all submitted work items have been processed.
 * 
 * @param model CLLM model
 */
void cllm_wait_for_work_completion(CLLMModel* model);

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

/**
 * Synchronize all threads at a barrier
 * 
 * This function ensures all threads reach a synchronization point before
 * any thread proceeds. Useful for coordinating between training steps.
 * 
 * @param model CLLM model
 */
void cllm_synchronize_threads(CLLMModel* model);

/**
 * Synchronize threads within a specific layer
 * 
 * @param model CLLM model
 * @param layer_idx Layer index (0-7)
 */
void cllm_synchronize_layer(CLLMModel* model, uint32_t layer_idx);

/**
 * Broadcast a message to all threads
 * 
 * @param model CLLM model
 * @param message_type Message type
 * @param data Message data
 * @param data_size Size of message data
 */
void cllm_broadcast_message(CLLMModel* model, uint32_t message_type, 
                            const void* data, size_t data_size);

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

/**
 * Get threading statistics
 * 
 * @param model CLLM model
 * @param stats Output statistics structure
 */
void cllm_get_threading_stats(const CLLMModel* model, HierarchicalThreadPoolStats* stats);

/**
 * Print threading statistics to stdout
 * 
 * @param model CLLM model
 */
void cllm_print_threading_stats(const CLLMModel* model);

/**
 * Reset threading statistics
 * 
 * @param model CLLM model
 */
void cllm_reset_threading_stats(CLLMModel* model);

// ============================================================================
// PARALLEL ML OPERATIONS (HIGH-LEVEL HELPERS)
// ============================================================================

/**
 * Parallel embedding lookup
 * 
 * Distributes embedding lookups across threads based on token-to-thread mapping.
 * 
 * @param model CLLM model
 * @param token_ids Token IDs to lookup [batch_size × seq_len]
 * @param batch_size Batch size
 * @param seq_len Sequence length
 * @param output Output embeddings [batch_size × seq_len × embedding_dim]
 */
void cllm_parallel_embedding_lookup(CLLMModel* model, const uint32_t* token_ids,
                                   uint32_t batch_size, uint32_t seq_len,
                                   double* output);

/**
 * Parallel multi-head attention
 * 
 * Distributes attention heads across threads (12 heads → 12 threads per layer).
 * 
 * @param model CLLM model
 * @param layer_idx Layer index
 * @param input Input tensor [batch_size × seq_len × embedding_dim]
 * @param batch_size Batch size
 * @param seq_len Sequence length
 * @param output Output tensor [batch_size × seq_len × embedding_dim]
 */
void cllm_parallel_attention(CLLMModel* model, uint32_t layer_idx,
                            const double* input, uint32_t batch_size,
                            uint32_t seq_len, double* output);

/**
 * Parallel feed-forward network
 * 
 * Distributes FFN computation across threads.
 * 
 * @param model CLLM model
 * @param layer_idx Layer index
 * @param input Input tensor [batch_size × seq_len × embedding_dim]
 * @param batch_size Batch size
 * @param seq_len Sequence length
 * @param output Output tensor [batch_size × seq_len × embedding_dim]
 */
void cllm_parallel_ffn(CLLMModel* model, uint32_t layer_idx,
                      const double* input, uint32_t batch_size,
                      uint32_t seq_len, double* output);

/**
 * Parallel gradient accumulation
 * 
 * Accumulates gradients from all threads into a single gradient buffer.
 * 
 * @param model CLLM model
 */
void cllm_parallel_gradient_accumulation(CLLMModel* model);

/**
 * Parallel weight update
 * 
 * Updates model weights in parallel using the optimizer.
 * 
 * @param model CLLM model
 */
void cllm_parallel_weight_update(CLLMModel* model);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_88D_INTEGRATION_H */