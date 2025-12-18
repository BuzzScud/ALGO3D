/**
 * @file generic_model.c
 * @brief Generic Model Interface Implementation
 */

#include "generic_model.h"
#include <stdlib.h>
#include <string.h>

/**
 * Create a generic model interface
 */
GenericModel* generic_model_create(void* model_data,
                                   uint32_t embedding_dim,
                                   uint32_t num_layers,
                                   uint32_t vocab_size) {
    if (!model_data) {
        return NULL;
    }
    
    GenericModel* model = (GenericModel*)calloc(1, sizeof(GenericModel));
    if (!model) {
        return NULL;
    }
    
    model->model_data = model_data;
    model->embedding_dim = embedding_dim;
    model->num_layers = num_layers;
    model->vocab_size = vocab_size;
    
    // Function pointers are NULL by default (calloc)
    // They will be set by the specific implementation
    
    return model;
}

/**
 * Free a generic model interface
 */
void generic_model_free(GenericModel* model) {
    if (!model) {
        return;
    }
    
    // Call cleanup function if provided
    if (model->cleanup && model->model_data) {
        model->cleanup(model->model_data);
    }
    
    // Free the GenericModel structure itself
    // NOTE: Does NOT free model_data - that's managed by the creator
    free(model);
}

/**
 * Validate a generic model interface
 */
bool generic_model_validate(const GenericModel* model) {
    if (!model) {
        return false;
    }
    
    // Check required fields
    if (!model->model_data) {
        return false;
    }
    
    if (model->embedding_dim == 0) {
        return false;
    }
    
    if (model->num_layers == 0) {
        return false;
    }
    
    if (model->vocab_size == 0) {
        return false;
    }
    
    // At least forward_layer must be set
    if (!model->forward_layer) {
        return false;
    }
    
    return true;
}