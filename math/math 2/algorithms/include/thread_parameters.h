/**
 * @file thread_parameters.h
 * @brief Thread-Local Parameter Storage API
 * 
 * PHASE 2: Thread-Centric Parameter Management
 * 
 * This module provides the API for managing parameters in thread-local storage.
 * Each thread owns its parameters in CrystallineAbacus format, eliminating the
 * need for global parameter arrays.
 * 
 * Key Features:
 * - Thread-local parameter storage (no global arrays)
 * - CrystallineAbacus-based computation
 * - Gradient accumulation per thread
 * - Optimizer state per thread (momentum, velocity)
 * - Named parameter access (e.g., "W_q", "embedding")
 * 
 * Architecture:
 * - Layer 0 threads: Store embeddings
 * - Layer 1-6 threads: Store attention weights, FFN weights
 * - Layer 7 threads: Store output weights
 * - All threads: Store gradients, momentum, velocity
 */

#ifndef THREAD_PARAMETERS_H
#define THREAD_PARAMETERS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/abacus.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
struct HierarchicalThread;
struct CLLMModel;

// ============================================================================
// PARAMETER METADATA
// ============================================================================

/**
 * Parameter metadata for a single parameter in a thread
 */
typedef struct {
    char name[64];              // Parameter name (e.g., "W_q", "embedding")
    uint32_t* shape;            // Parameter shape [num_dims]
    uint32_t num_dims;          // Number of dimensions
    size_t total_elements;      // Total number of elements
    bool requires_grad;         // Whether this parameter needs gradients
    bool is_initialized;        // Whether parameter is initialized
} ThreadParameterMetadata;

/**
 * Parameter initialization method
 */
typedef enum {
    PARAM_INIT_ZEROS,       // All zeros
    PARAM_INIT_ONES,        // All ones
    PARAM_INIT_RANDOM,      // Random uniform [-0.1, 0.1]
    PARAM_INIT_XAVIER,      // Xavier/Glorot initialization
    PARAM_INIT_HE,          // He initialization
    PARAM_INIT_NORMAL,      // Normal distribution N(0, 0.02)
} ParameterInitMethod;

// ============================================================================
// PARAMETER ALLOCATION
// ============================================================================

/**
 * Allocate a parameter in a thread's storage
 * 
 * Creates a new CrystallineAbacus parameter with the specified shape.
 * Also allocates corresponding gradient, momentum, and velocity storage.
 * 
 * @param thread Thread to allocate parameter in
 * @param name Parameter name (e.g., "W_q", "embedding")
 * @param shape Parameter shape (e.g., [embedding_dim, embedding_dim])
 * @param num_dims Number of dimensions
 * @return 0 on success, -1 on error
 */
int thread_allocate_parameter(
    struct HierarchicalThread* thread,
    const char* name,
    const uint32_t* shape,
    uint32_t num_dims
);

/**
 * Allocate all parameters for a thread based on its role
 * 
 * For Layer 0 (Embeddings):
 *   - embeddings: [num_tokens_assigned, embedding_dim]
 * 
 * For Layer 1-6 (Transformer):
 *   - W_q: [embedding_dim, embedding_dim]
 *   - W_k: [embedding_dim, embedding_dim]
 *   - W_v: [embedding_dim, embedding_dim]
 *   - W_o: [embedding_dim, embedding_dim]
 *   - W_ffn1: [embedding_dim, hidden_dim]
 *   - W_ffn2: [hidden_dim, embedding_dim]
 *   - gamma: [embedding_dim] (layer norm)
 *   - beta: [embedding_dim] (layer norm)
 * 
 * For Layer 7 (Output):
 *   - W_out: [embedding_dim, vocab_size / 12]
 * 
 * @param thread Thread to allocate parameters for
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension
 * @param vocab_size Vocabulary size
 * @param num_tokens_assigned Number of tokens assigned to this thread (Layer 0 only)
 * @return 0 on success, -1 on error
 */
int thread_allocate_all_parameters(
    struct HierarchicalThread* thread,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    uint32_t vocab_size,
    uint32_t num_tokens_assigned
);

// ============================================================================
// PARAMETER INITIALIZATION
// ============================================================================

/**
 * Initialize a parameter with a specific method
 * 
 * @param thread Thread containing the parameter
 * @param name Parameter name
 * @param method Initialization method
 * @param seed Random seed (use thread->thread_id for determinism)
 * @return 0 on success, -1 on error
 */
int thread_initialize_parameter(
    struct HierarchicalThread* thread,
    const char* name,
    ParameterInitMethod method,
    uint64_t seed
);

/**
 * Initialize all parameters in a thread
 * 
 * @param thread Thread to initialize
 * @param method Initialization method
 * @return 0 on success, -1 on error
 */
int thread_initialize_all_parameters(
    struct HierarchicalThread* thread,
    ParameterInitMethod method
);

// ============================================================================
// PARAMETER ACCESS
// ============================================================================

/**
 * Get a parameter from a thread
 * 
 * @param thread Thread containing the parameter
 * @param name Parameter name
 * @return Parameter CrystallineAbacus, or NULL if not found
 */
CrystallineAbacus* thread_get_parameter(
    struct HierarchicalThread* thread,
    const char* name
);

/**
 * Get parameter index by name
 * 
 * @param thread Thread to search
 * @param name Parameter name
 * @return Parameter index, or -1 if not found
 */
int thread_get_parameter_index(
    struct HierarchicalThread* thread,
    const char* name
);

/**
 * Get parameter metadata
 * 
 * @param thread Thread containing the parameter
 * @param name Parameter name
 * @return Parameter metadata, or NULL if not found
 */
ThreadParameterMetadata* thread_get_parameter_metadata(
    struct HierarchicalThread* thread,
    const char* name
);

// ============================================================================
// GRADIENT ACCESS
// ============================================================================

/**
 * Get a gradient from a thread
 * 
 * @param thread Thread containing the gradient
 * @param name Parameter name
 * @return Gradient CrystallineAbacus, or NULL if not found
 */
CrystallineAbacus* thread_get_gradient(
    struct HierarchicalThread* thread,
    const char* name
);

/**
 * Clear all gradients in a thread
 * 
 * Sets all gradients to zero.
 * 
 * @param thread Thread to clear gradients for
 * @return 0 on success, -1 on error
 */
int thread_clear_gradients(struct HierarchicalThread* thread);

/**
 * Accumulate gradient for a parameter
 * 
 * Adds the given gradient to the parameter's accumulated gradient.
 * 
 * @param thread Thread containing the parameter
 * @param name Parameter name
 * @param grad Gradient to accumulate
 * @return 0 on success, -1 on error
 */
int thread_accumulate_gradient(
    struct HierarchicalThread* thread,
    const char* name,
    const CrystallineAbacus* grad
);

// ============================================================================
// OPTIMIZER STATE ACCESS
// ============================================================================

/**
 * Get momentum (first moment) for a parameter
 * 
 * @param thread Thread containing the momentum
 * @param name Parameter name
 * @return Momentum CrystallineAbacus, or NULL if not found
 */
CrystallineAbacus* thread_get_momentum(
    struct HierarchicalThread* thread,
    const char* name
);

/**
 * Get velocity (second moment) for a parameter
 * 
 * @param thread Thread containing the velocity
 * @param name Parameter name
 * @return Velocity CrystallineAbacus, or NULL if not found
 */
CrystallineAbacus* thread_get_velocity(
    struct HierarchicalThread* thread,
    const char* name
);

/**
 * Clear optimizer state (momentum and velocity)
 * 
 * @param thread Thread to clear optimizer state for
 * @return 0 on success, -1 on error
 */
int thread_clear_optimizer_state(struct HierarchicalThread* thread);

// ============================================================================
// PARAMETER STATISTICS
// ============================================================================

/**
 * Parameter statistics
 */
typedef struct {
    uint32_t num_parameters;        // Number of parameters
    size_t total_memory_bytes;      // Total memory used
    double avg_param_value;         // Average parameter value
    double avg_grad_norm;           // Average gradient norm
    double avg_momentum_norm;       // Average momentum norm
    double avg_velocity_norm;       // Average velocity norm
    uint32_t num_initialized;       // Number of initialized parameters
} ThreadParameterStats;

/**
 * Get parameter statistics for a thread
 * 
 * @param thread Thread to query
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int thread_get_parameter_stats(
    struct HierarchicalThread* thread,
    ThreadParameterStats* stats
);

/**
 * Print parameter statistics for a thread
 * 
 * @param thread Thread to print stats for
 */
void thread_print_parameter_stats(struct HierarchicalThread* thread);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if thread has a parameter
 * 
 * @param thread Thread to check
 * @param name Parameter name
 * @return true if parameter exists, false otherwise
 */
bool thread_has_parameter(
    struct HierarchicalThread* thread,
    const char* name
);

/**
 * Get number of parameters in a thread
 * 
 * @param thread Thread to query
 * @return Number of parameters
 */
uint32_t thread_get_num_parameters(struct HierarchicalThread* thread);

/**
 * Get total memory used by parameters in a thread
 * 
 * @param thread Thread to query
 * @return Total memory in bytes
 */
size_t thread_get_parameter_memory(struct HierarchicalThread* thread);

#ifdef __cplusplus
}
#endif

#endif // THREAD_PARAMETERS_H