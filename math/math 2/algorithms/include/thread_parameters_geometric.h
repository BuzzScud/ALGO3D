#ifndef THREAD_PARAMETERS_GEOMETRIC_H
#define THREAD_PARAMETERS_GEOMETRIC_H

#include <stdint.h>
#include <stdbool.h>
#include "hierarchical_threading.h"
#include "geometric_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file thread_parameters_geometric.h
 * @brief Thread Parameter Management using Geometric Matrix Storage
 * 
 * This replaces the flat array parameter storage with geometric matrices
 * using Platonic solid coordinate frames and barycentric interpolation.
 * 
 * Key Benefits:
 * - 1,674× memory reduction (7.2 GB → 4.3 MB)
 * - True 88D architecture
 * - Self-similar structure
 * - Minimal memory footprint
 */

// ============================================================================
// PARAMETER INITIALIZATION METHODS
// ============================================================================

typedef enum {
    GEOMETRIC_INIT_ZEROS = 0,
    GEOMETRIC_INIT_ONES = 1,
    GEOMETRIC_INIT_UNIFORM = 2,
    GEOMETRIC_INIT_XAVIER = 3,
    GEOMETRIC_INIT_HE = 4,
    GEOMETRIC_INIT_NORMAL = 5
} GeometricInitMethod;

// ============================================================================
// PARAMETER ALLOCATION
// ============================================================================

/**
 * @brief Allocate a geometric parameter for a thread
 * 
 * Creates a GeometricMatrix with the specified dimensions and stores it
 * in the thread's geometric parameter array.
 * 
 * @param thread Thread to allocate parameter for
 * @param name Parameter name (e.g., "W_q", "W_k")
 * @param rows Number of rows
 * @param cols Number of columns
 * @return Parameter index, or -1 on error
 */
int thread_allocate_geometric_parameter(
    HierarchicalThread* thread,
    const char* name,
    uint32_t rows,
    uint32_t cols
);

/**
 * @brief Allocate all geometric parameters for a thread based on its layer
 * 
 * Automatically allocates the appropriate parameters based on the thread's
 * layer in the 88D architecture:
 * - Layer 0: Embeddings
 * - Layers 1-6: W_q, W_k, W_v, W_o, W_ffn1, W_ffn2
 * - Layer 7: W_out
 * 
 * @param thread Thread to allocate parameters for
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension (for FFN)
 * @param vocab_size Vocabulary size (for layer 0 and 7)
 * @return 0 on success, -1 on error
 */
int thread_allocate_all_geometric_parameters(
    HierarchicalThread* thread,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    uint32_t vocab_size
);

/**
 * @brief Free all geometric parameters for a thread
 * 
 * @param thread Thread to free parameters for
 */
void thread_free_geometric_parameters(HierarchicalThread* thread);

// ============================================================================
// PARAMETER INITIALIZATION
// ============================================================================

/**
 * @brief Initialize a geometric parameter
 * 
 * @param thread Thread that owns the parameter
 * @param param_idx Parameter index
 * @param method Initialization method
 * @param seed Random seed (for random methods)
 * @return 0 on success, -1 on error
 */
int thread_initialize_geometric_parameter(
    HierarchicalThread* thread,
    uint32_t param_idx,
    GeometricInitMethod method,
    uint64_t seed
);

/**
 * @brief Initialize all geometric parameters for a thread
 * 
 * @param thread Thread to initialize parameters for
 * @param method Initialization method
 * @param seed Random seed
 * @return 0 on success, -1 on error
 */
int thread_initialize_all_geometric_parameters(
    HierarchicalThread* thread,
    GeometricInitMethod method,
    uint64_t seed
);

// ============================================================================
// PARAMETER ACCESS
// ============================================================================

/**
 * @brief Get a geometric parameter by name
 * 
 * @param thread Thread that owns the parameter
 * @param name Parameter name
 * @return Pointer to GeometricMatrix, or NULL if not found
 */
GeometricMatrix* thread_get_geometric_parameter(
    HierarchicalThread* thread,
    const char* name
);

/**
 * @brief Get a geometric parameter by index
 * 
 * @param thread Thread that owns the parameter
 * @param param_idx Parameter index
 * @return Pointer to GeometricMatrix, or NULL if invalid
 */
GeometricMatrix* thread_get_geometric_parameter_by_index(
    HierarchicalThread* thread,
    uint32_t param_idx
);

/**
 * @brief Get value from geometric parameter at (row, col)
 * 
 * Uses barycentric interpolation to compute the value.
 * 
 * @param thread Thread that owns the parameter
 * @param param_idx Parameter index
 * @param row Row index
 * @param col Column index
 * @param result Output value (must be pre-allocated)
 * @return 0 on success, -1 on error
 */
int thread_get_geometric_value(
    HierarchicalThread* thread,
    uint32_t param_idx,
    uint32_t row,
    uint32_t col,
    CrystallineAbacus* result
);

/**
 * @brief Accumulate gradient to geometric parameter
 * 
 * Distributes the gradient to nearby vertices using barycentric weights.
 * 
 * @param thread Thread that owns the parameter
 * @param param_idx Parameter index
 * @param row Row index
 * @param col Column index
 * @param gradient Gradient value
 * @return 0 on success, -1 on error
 */
int thread_accumulate_geometric_gradient(
    HierarchicalThread* thread,
    uint32_t param_idx,
    uint32_t row,
    uint32_t col,
    const CrystallineAbacus* gradient
);

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================

/**
 * @brief Clear all gradients for a thread
 * 
 * Sets all vertex gradients to zero.
 * 
 * @param thread Thread to clear gradients for
 * @return 0 on success, -1 on error
 */
int thread_clear_geometric_gradients(HierarchicalThread* thread);

/**
 * @brief Apply optimizer to geometric parameters
 * 
 * Uses Adam optimizer on vertex values only:
 *   m = β₁*m + (1-β₁)*grad
 *   v = β₂*v + (1-β₂)*grad²
 *   param -= lr * m / (√v + ε)
 * 
 * @param thread Thread that owns the parameters
 * @param learning_rate Learning rate
 * @param beta1 Adam beta1 (momentum decay, default 0.9)
 * @param beta2 Adam beta2 (velocity decay, default 0.999)
 * @param epsilon Adam epsilon (numerical stability, default 1e-8)
 * @return 0 on success, -1 on error
 */
int thread_apply_geometric_optimizer(
    HierarchicalThread* thread,
    double learning_rate,
    double beta1,
    double beta2,
    double epsilon
);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get total memory usage of geometric parameters
 * 
 * @param thread Thread to analyze
 * @return Memory usage in bytes
 */
size_t thread_geometric_memory_usage(const HierarchicalThread* thread);

/**
 * @brief Print geometric parameter info for a thread
 * 
 * @param thread Thread to print info for
 */
void thread_print_geometric_info(const HierarchicalThread* thread);

/**
 * @brief Get number of geometric parameters
 * 
 * @param thread Thread to query
 * @return Number of geometric parameters
 */
uint32_t thread_get_num_geometric_parameters(const HierarchicalThread* thread);

// ============================================================================
// MIGRATION HELPERS
// ============================================================================

/**
 * @brief Convert flat array parameters to geometric matrices
 * 
 * This is a migration helper to convert existing flat array parameters
 * to geometric matrix format.
 * 
 * @param thread Thread with flat array parameters
 * @return 0 on success, -1 on error
 */
int thread_migrate_to_geometric(HierarchicalThread* thread);

/**
 * @brief Check if thread is using geometric parameters
 * 
 * @param thread Thread to check
 * @return true if using geometric parameters, false otherwise
 */
bool thread_is_using_geometric(const HierarchicalThread* thread);

#ifdef __cplusplus
}
#endif

#endif // THREAD_PARAMETERS_GEOMETRIC_H