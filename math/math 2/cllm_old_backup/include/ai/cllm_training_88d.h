/**
 * @file cllm_training_88d.h
 * @brief Parallel Training Operations using 88D Threading System
 * 
 * This header defines the interface for parallel training operations that
 * leverage the 88D threading system for efficient distributed computation.
 */

#ifndef CLLM_TRAINING_88D_H
#define CLLM_TRAINING_88D_H

#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TRAINING STATISTICS
// ============================================================================

/**
 * Training statistics structure
 */
typedef struct {
    uint32_t num_threads;           // Number of threads used
    uint32_t num_layers;            // Number of layers
    uint64_t work_items_processed;  // Total work items processed
    double total_sync_time_ms;      // Total synchronization time
    double total_compute_time_ms;   // Total computation time
} CLLMTrainingStats;

// ============================================================================
// FORWARD PASS OPERATIONS
// ============================================================================

/**
 * Parallel forward pass across 88D threads
 * 
 * Distributes forward pass computation across all available threads in the
 * 88D threading system. Work is distributed geometrically based on the
 * Platonic solid structure.
 * 
 * @param model CLLM model
 * @param input Input tensor [batch_size × hidden_size]
 * @param output Output tensor [batch_size × hidden_size]
 * @param batch_size Batch size
 * @return 0 on success, -1 on error
 */
int cllm_training_forward_pass_parallel(
    CLLMModel* model,
    const float* input,
    float* output,
    uint32_t batch_size
);

// ============================================================================
// BACKWARD PASS OPERATIONS
// ============================================================================

/**
 * Parallel backward pass across 88D threads
 * 
 * Distributes backward pass computation across all available threads in the
 * 88D threading system. Gradients are accumulated thread-safely.
 * 
 * @param model CLLM model
 * @param grad_output Gradient of loss w.r.t. output [batch_size × hidden_size]
 * @param grad_input Gradient of loss w.r.t. input [batch_size × hidden_size]
 * @param grad_weights Gradient of loss w.r.t. weights (can be NULL)
 * @param batch_size Batch size
 * @return 0 on success, -1 on error
 */
int cllm_training_backward_pass_parallel(
    CLLMModel* model,
    const float* grad_output,
    float* grad_input,
    float* grad_weights,
    uint32_t batch_size
);

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================

/**
 * Accumulate gradients across threads
 * 
 * Thread-safe gradient accumulation from multiple threads into a global
 * gradient buffer.
 * 
 * @param model CLLM model
 * @param global_gradients Global gradient buffer
 * @param thread_gradients Thread-local gradient buffer
 * @param num_params Number of parameters
 * @return 0 on success, -1 on error
 */
int cllm_training_accumulate_gradients(
    CLLMModel* model,
    float* global_gradients,
    const float* thread_gradients,
    uint32_t num_params
);

/**
 * Zero out gradients
 * 
 * @param model CLLM model
 * @param gradients Gradient buffer to zero
 * @param num_params Number of parameters
 * @return 0 on success, -1 on error
 */
int cllm_training_zero_gradients(
    CLLMModel* model,
    float* gradients,
    uint32_t num_params
);

/**
 * Apply gradient clipping
 * 
 * Clips gradients to a maximum norm to prevent exploding gradients.
 * 
 * @param model CLLM model
 * @param gradients Gradient buffer
 * @param num_params Number of parameters
 * @param max_norm Maximum gradient norm
 * @return 0 on success, -1 on error
 */
int cllm_training_clip_gradients(
    CLLMModel* model,
    float* gradients,
    uint32_t num_params,
    float max_norm
);

// ============================================================================
// LAYER-WISE OPERATIONS
// ============================================================================

/**
 * Synchronize threads at layer boundary
 * 
 * Uses the 88D layer barrier to synchronize all threads at a layer boundary.
 * This ensures all threads complete their work before proceeding to the next layer.
 * 
 * @param model CLLM model
 * @param layer_id Layer ID (0-7 for 8 layers)
 * @return 0 on success, -1 on error
 */
int cllm_training_sync_layer(
    CLLMModel* model,
    uint32_t layer_id
);

/**
 * Distribute layer computation across threads
 * 
 * Computes a single layer in parallel across all threads, with automatic
 * synchronization at layer boundaries.
 * 
 * @param model CLLM model
 * @param layer_id Layer ID
 * @param input Input tensor
 * @param output Output tensor
 * @param batch_size Batch size
 * @return 0 on success, -1 on error
 */
int cllm_training_compute_layer_parallel(
    CLLMModel* model,
    uint32_t layer_id,
    const float* input,
    float* output,
    uint32_t batch_size
);

// ============================================================================
// TRAINING LOOP HELPERS
// ============================================================================

/**
 * Single training step with 88D parallelism
 * 
 * Performs a complete forward pass, loss computation, and backward pass
 * using the 88D threading system.
 * 
 * @param model CLLM model
 * @param input Input tensor [batch_size × hidden_size]
 * @param target Target tensor [batch_size × hidden_size]
 * @param loss Output: computed loss
 * @param batch_size Batch size
 * @return 0 on success, -1 on error
 */
int cllm_training_step_parallel(
    CLLMModel* model,
    const float* input,
    const float* target,
    float* loss,
    uint32_t batch_size
);

/**
 * Get training statistics
 * 
 * @param model CLLM model
 * @param stats Output: training statistics
 * @return 0 on success, -1 on error
 */
int cllm_training_get_stats(
    CLLMModel* model,
    CLLMTrainingStats* stats
);

/**
 * Print training statistics
 * 
 * @param stats Training statistics to print
 */
void cllm_training_print_stats(const CLLMTrainingStats* stats);

#ifdef __cplusplus
}
#endif

#endif // CLLM_TRAINING_88D_H