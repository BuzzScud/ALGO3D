/**
 * @file cllm_generic_interface.c
 * @brief CLLM Implementation of Generic Model Interface
 */

#include "ai/cllm_generic_interface.h"
#include "ai/cllm_transformer_layer.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Forward layer implementation for CLLM
 * 
 * This is called by worker threads through the generic interface.
 * It casts the opaque model_data back to CLLMModel and calls
 * the CLLM-specific transformer layer function.
 */
static int cllm_generic_forward_layer(void* model_data, void* thread,
                                     uint8_t layer_idx,
                                     const double* input, double* output) {
    if (!model_data || !thread || !input || !output) {
        return -1;
    }
    
    // Cast opaque pointers back to specific types
    CLLMModel* model = (CLLMModel*)model_data;
    HierarchicalThread* t = (HierarchicalThread*)thread;
    
    // Call CLLM-specific transformer layer function
    return cllm_transformer_layer_forward(model, t, layer_idx, input, output);
}

/**
 * Backward layer implementation for CLLM (optional)
 * 
 * TODO: Implement when backward pass is needed
 */
static int cllm_generic_backward_layer(void* model_data, void* thread,
                                      uint8_t layer_idx,
                                      const double* grad_output, 
                                      double* grad_input) {
    // Not implemented yet
    (void)model_data;
    (void)thread;
    (void)layer_idx;
    (void)grad_output;
    (void)grad_input;
    return -1;
}

/**
 * Get input for token (optional)
 * 
 * TODO: Implement if needed
 */
static void* cllm_generic_get_input(void* model_data, uint32_t token_id) {
    // Not implemented yet
    (void)model_data;
    (void)token_id;
    return NULL;
}

/**
 * Set output for token (optional)
 * 
 * TODO: Implement if needed
 */
static void cllm_generic_set_output(void* model_data, uint32_t token_id, 
                                   const void* output) {
    // Not implemented yet
    (void)model_data;
    (void)token_id;
    (void)output;
}

/**
 * Cleanup function (optional)
 * 
 * This is called when the generic interface is freed.
 * We don't free the CLLMModel here because it's managed separately.
 */
static void cllm_generic_cleanup(void* model_data) {
    // Don't free the CLLMModel - it's managed by the caller
    (void)model_data;
}

/**
 * Create a generic model interface for a CLLM model
 */
GenericModel* cllm_create_generic_interface(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "Error: NULL CLLMModel in cllm_create_generic_interface\n");
        return NULL;
    }
    
    // Verify model has required components
    if (!model->threads) {
        fprintf(stderr, "Error: CLLMModel has no thread pool\n");
        return NULL;
    }
    
    // Create generic interface
    GenericModel* generic = generic_model_create(
        model,                    // model_data (opaque pointer to CLLMModel)
        model->embedding_dim,     // embedding_dim
        model->num_layers,        // num_layers
        model->vocab_size         // vocab_size
    );
    
    if (!generic) {
        fprintf(stderr, "Error: Failed to create generic model interface\n");
        return NULL;
    }
    
    // Set function pointers
    generic->forward_layer = cllm_generic_forward_layer;
    generic->backward_layer = cllm_generic_backward_layer;  // Optional, not implemented yet
    generic->get_input = cllm_generic_get_input;            // Optional, not implemented yet
    generic->set_output = cllm_generic_set_output;          // Optional, not implemented yet
    generic->cleanup = cllm_generic_cleanup;                // Optional
    
    return generic;
}

/**
 * Free a generic interface created for CLLM
 */
void cllm_free_generic_interface(GenericModel* generic) {
    if (generic) {
        generic_model_free(generic);
    }
}