/**
 * @file generic_model.h
 * @brief Generic Model Interface for Worker Threads
 * 
 * This interface allows worker threads to process work items
 * without knowing the specific model implementation (CLLM, PlatonicModel, etc.).
 * 
 * This maintains clean architecture:
 * - Algorithms library (lower layer) defines the interface
 * - CLLM library (upper layer) implements the interface
 * - No circular dependencies
 * 
 * Architecture:
 *   Math → Algorithms (generic interface) → CLLM (implements interface)
 */

#ifndef ALGORITHMS_GENERIC_MODEL_H
#define ALGORITHMS_GENERIC_MODEL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generic Model Interface
 * 
 * This structure provides a generic interface for models to work
 * with the threading system. Models implement function pointers
 * to provide their specific behavior.
 */
typedef struct GenericModel {
    // Opaque model data (specific to implementation)
    void* model_data;
    
    // Model dimensions
    uint32_t embedding_dim;      // Embedding dimension
    uint32_t num_layers;         // Number of layers
    uint32_t vocab_size;         // Vocabulary size
    
    // Forward pass function
    // Parameters: model_data, thread, layer_idx, input, output
    // Returns: 0 on success, -1 on error
    int (*forward_layer)(void* model_data, void* thread, 
                        uint8_t layer_idx,
                        const double* input, double* output);
    
    // Backward pass function (optional, can be NULL)
    // Parameters: model_data, thread, layer_idx, grad_output, grad_input
    // Returns: 0 on success, -1 on error
    int (*backward_layer)(void* model_data, void* thread,
                         uint8_t layer_idx,
                         const double* grad_output, double* grad_input);
    
    // Get input for token (optional, can be NULL)
    // Parameters: model_data, token_id
    // Returns: pointer to input data, or NULL on error
    void* (*get_input)(void* model_data, uint32_t token_id);
    
    // Set output for token (optional, can be NULL)
    // Parameters: model_data, token_id, output
    void (*set_output)(void* model_data, uint32_t token_id, const void* output);
    
    // Cleanup function (optional, can be NULL)
    // Called when the generic interface is freed
    // Should NOT free the underlying model_data (that's managed separately)
    void (*cleanup)(void* model_data);
    
} GenericModel;

/**
 * Create a generic model interface
 * 
 * @param model_data Opaque pointer to model-specific data
 * @param embedding_dim Embedding dimension
 * @param num_layers Number of layers
 * @param vocab_size Vocabulary size
 * @return Pointer to GenericModel, or NULL on error
 */
GenericModel* generic_model_create(void* model_data,
                                   uint32_t embedding_dim,
                                   uint32_t num_layers,
                                   uint32_t vocab_size);

/**
 * Free a generic model interface
 * 
 * This frees the GenericModel structure itself, but does NOT
 * free the underlying model_data. The model_data is managed
 * by the code that created it.
 * 
 * If a cleanup function is set, it will be called before freeing.
 * 
 * @param model Generic model to free
 */
void generic_model_free(GenericModel* model);

/**
 * Validate a generic model interface
 * 
 * Checks that required fields are set and valid.
 * 
 * @param model Generic model to validate
 * @return true if valid, false otherwise
 */
bool generic_model_validate(const GenericModel* model);

#ifdef __cplusplus
}
#endif

#endif // ALGORITHMS_GENERIC_MODEL_H