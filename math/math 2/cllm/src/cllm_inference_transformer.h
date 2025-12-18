/**
 * @file cllm_inference_transformer.h
 * @brief Transformer layer implementation for CLLM inference
 */

#ifndef CLLM_INFERENCE_TRANSFORMER_H
#define CLLM_INFERENCE_TRANSFORMER_H

#include "ai/cllm.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Process input through all transformer layers
 * 
 * @param model The CLLM model
 * @param hidden_states Input/output hidden states [embedding_dim]
 */
void cllm_transformer_forward(const CLLMModel* model, double* hidden_states);

/**
 * Check if model has properly initialized transformer layers
 * 
 * @param model The CLLM model
 * @return true if transformer layers are initialized, false otherwise
 */
bool cllm_has_transformer_layers(const CLLMModel* model);

/**
 * Replace NaN values in model weights with zeros
 * This is a safety measure for models that were saved with NaN values
 * 
 * @param model The CLLM model
 */
void cllm_fix_nan_weights(CLLMModel* model);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_INFERENCE_TRANSFORMER_H */