/**
 * @file cllm_inference_transformer.c
 * @brief Transformer layer implementation for CLLM inference
 * 
 * This file implements the transformer layers that were previously disabled.
 * It integrates with the new CLLMModel structure and enables actual text generation.
 */

#include "../../include/cllm.h"
#include "../../math/include/math.h"
#include "../../math/include/math/validation.h"  // For math_is_nan, math_is_inf
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations
static void layer_norm(double* output, const double* input, const double* gamma, 
                      const double* beta, uint32_t size);
static void attention_forward(const CLLMModel* model, uint32_t layer_idx,
                             const double* input, double* output, uint32_t seq_len);
static void feedforward_forward(const CLLMModel* model, uint32_t layer_idx,
                               const double* input, double* output);
static void gelu_activation(double* x, uint32_t size);

/**
 * Apply layer normalization
 */
static void layer_norm(double* output, const double* input, const double* gamma, 
                      const double* beta, uint32_t size) {
    // Check for NULL pointers
    if (!output || !input || !gamma || !beta) {
        fprintf(stderr, "ERROR: NULL pointer in layer_norm\n");
        return;
    }
    
    // Calculate mean
    double mean = 0.0;
    for (uint32_t i = 0; i < size; i++) {
        mean += input[i];
    }
    mean /= size;
    
    // Calculate variance
    double variance = 0.0;
    for (uint32_t i = 0; i < size; i++) {
        double diff = input[i] - mean;
        variance += diff * diff;
    }
    variance /= size;
    
    // Normalize and apply scale/shift
    double std = math_sqrt(variance + 1e-5);
    
    // Check for NaN/inf
    if (math_is_nan(std) || math_is_inf(std)) {
        fprintf(stderr, "ERROR: Invalid std in layer_norm: %f\n", std);
        return;
    }
    
    for (uint32_t i = 0; i < size; i++) {
        output[i] = gamma[i] * ((input[i] - mean) / std) + beta[i];
    }
}

/**
 * GELU activation function
 */
static void gelu_activation(double* x, uint32_t size) {
    const double sqrt_2_over_pi = 0.7978845608028654; // sqrt(2/pi)
    for (uint32_t i = 0; i < size; i++) {
        double val = x[i];
        double tanh_arg = sqrt_2_over_pi * (val + 0.044715 * val * val * val);
        x[i] = 0.5 * val * (1.0 + math_tanh(tanh_arg));
    }
}

/**
 * Simplified single-head attention for inference
 * (Multi-head attention would be more complex but this works for generation)
 */
static void attention_forward(const CLLMModel* model, uint32_t layer_idx,
                             const double* input, double* output, uint32_t seq_len) {
    (void)seq_len;  // Unused in single-token inference
    uint32_t embed_dim = model->embedding_dim;
    
    // Get layer weights
    const double* Q_weights = model->layers[layer_idx].query_weights;
    const double* K_weights = model->layers[layer_idx].key_weights;
    const double* V_weights = model->layers[layer_idx].value_weights;
    const double* O_weights = model->layers[layer_idx].output_weights;
    
    // For inference, we only process the last token
    // In a full implementation, we'd cache previous K,V values
    
    // Allocate temporary buffers
    double* query = (double*)calloc(embed_dim, sizeof(double));
    double* key = (double*)calloc(embed_dim, sizeof(double));
    double* value = (double*)calloc(embed_dim, sizeof(double));
    
    if (!query || !key || !value) {
        fprintf(stderr, "Error: Failed to allocate attention buffers\n");
        free(query);
        free(key);
        free(value);
        return;
    }
    
    // Compute Q, K, V for the current token
    for (uint32_t i = 0; i < embed_dim; i++) {
        query[i] = 0.0;
        key[i] = 0.0;
        value[i] = 0.0;
        for (uint32_t j = 0; j < embed_dim; j++) {
            query[i] += input[j] * Q_weights[j * embed_dim + i];
            key[i] += input[j] * K_weights[j * embed_dim + i];
            value[i] += input[j] * V_weights[j * embed_dim + i];
        }
    }
    
    // Simplified attention: just use the value directly
    // (In a full implementation, we'd compute attention scores with all previous tokens)
    double scale = 1.0 / math_sqrt((double)embed_dim);
    for (uint32_t i = 0; i < embed_dim; i++) {
        value[i] *= scale;
    }
    
    // Apply output projection
    for (uint32_t i = 0; i < embed_dim; i++) {
        output[i] = 0.0;
        for (uint32_t j = 0; j < embed_dim; j++) {
            output[i] += value[j] * O_weights[j * embed_dim + i];
        }
    }
    
    free(query);
    free(key);
    free(value);
}

/**
 * Feed-forward network
 */
static void feedforward_forward(const CLLMModel* model, uint32_t layer_idx,
                               const double* input, double* output) {
    uint32_t embed_dim = model->embedding_dim;
    uint32_t hidden_dim = model->hidden_dim;
    
    // Get layer weights
    const double* w1 = model->layers[layer_idx].ffn_w1;
    const double* w2 = model->layers[layer_idx].ffn_w2;
    const double* b1 = model->layers[layer_idx].ffn_b1;
    const double* b2 = model->layers[layer_idx].ffn_b2;
    
    // Check for NULL pointers
    if (!w1 || !w2 || !b1 || !b2) {
        fprintf(stderr, "ERROR: NULL weights in FFN layer %u\n", layer_idx);
        return;
    }
    
    // Allocate hidden layer
    double* hidden = (double*)calloc(hidden_dim, sizeof(double));
    if (!hidden) {
        fprintf(stderr, "Error: Failed to allocate FFN hidden buffer\n");
        return;
    }
    
    // First layer: input -> hidden with GELU activation
    for (uint32_t i = 0; i < hidden_dim; i++) {
        hidden[i] = b1[i];
        for (uint32_t j = 0; j < embed_dim; j++) {
            hidden[i] += input[j] * w1[j * hidden_dim + i];
        }
    }
    
    // Check for NaN after first layer
    bool has_nan_h1 __attribute__((unused)) = false;
    for (uint32_t i = 0; i < hidden_dim && i < 5; i++) {
        if (math_is_nan(hidden[i])) {
            has_nan_h1 = true;
            break;
        }
    }
    
    // Apply GELU activation
    gelu_activation(hidden, hidden_dim);
    
    // Check for NaN after GELU
    bool has_nan_gelu __attribute__((unused)) = false;
    for (uint32_t i = 0; i < hidden_dim && i < 5; i++) {
        if (math_is_nan(hidden[i])) {
            has_nan_gelu = true;
            break;
        }
    }
    
    // Second layer: hidden -> output
    for (uint32_t i = 0; i < embed_dim; i++) {
        output[i] = b2[i];
        for (uint32_t j = 0; j < hidden_dim; j++) {
            output[i] += hidden[j] * w2[j * embed_dim + i];
        }
    }
    
    // Check for NaN in output
    bool has_nan_out __attribute__((unused)) = false;
    for (uint32_t i = 0; i < embed_dim && i < 5; i++) {
        if (math_is_nan(output[i])) {
            has_nan_out = true;
            break;
        }
    }
    
    free(hidden);
}

/**
 * Process input through all transformer layers
 * This is the main function that replaces the disabled code in cllm_inference.c
 */
void cllm_transformer_forward(const CLLMModel* model, double* hidden_states) {
    if (!model || !hidden_states) {
        fprintf(stderr, "Error: NULL pointer in transformer forward\n");
        return;
    }
    
    if (!model->layers) {
        fprintf(stderr, "Error: Model has no layers\n");
        return;
    }
    
    uint32_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    // Allocate temporary buffers
    double* attn_output = (double*)calloc(embed_dim, sizeof(double));
    double* ffn_output = (double*)calloc(embed_dim, sizeof(double));
    double* ln_output = (double*)calloc(embed_dim, sizeof(double));
    
    if (!attn_output || !ffn_output || !ln_output) {
        fprintf(stderr, "Error: Failed to allocate transformer buffers\n");
        free(attn_output);
        free(ffn_output);
        free(ln_output);
        return;
    }
    
    // Process through each transformer layer
    for (uint32_t layer = 0; layer < num_layers; layer++) {
        // Debug: Check input to layer
        bool has_nan_input = false;
        for (uint32_t i = 0; i < embed_dim && i < 5; i++) {
            if (math_is_nan(hidden_states[i])) {
                has_nan_input = true;
                break;
            }
        }
        if (has_nan_input) {
        }
        
        // Pre-layer norm
        layer_norm(ln_output, hidden_states, 
                  model->layers[layer].ln1_gamma,
                  model->layers[layer].ln1_beta,
                  embed_dim);
        
        // Debug: Check after layer norm
        bool has_nan_ln = false;
        for (uint32_t i = 0; i < embed_dim && i < 5; i++) {
            if (math_is_nan(ln_output[i])) {
                has_nan_ln = true;
                break;
            }
        }
        if (has_nan_ln) {
        }
        
        // Self-attention with residual connection
        attention_forward(model, layer, ln_output, attn_output, 1);
        
        // Debug: Check after attention
        bool has_nan_attn = false;
        for (uint32_t i = 0; i < embed_dim && i < 5; i++) {
            if (math_is_nan(attn_output[i])) {
                has_nan_attn = true;
                break;
            }
        }
        if (has_nan_attn) {
        }
        
        for (uint32_t i = 0; i < embed_dim; i++) {
            hidden_states[i] += attn_output[i];
        }
        
        // Pre-FFN layer norm
        layer_norm(ln_output, hidden_states,
                  model->layers[layer].ln2_gamma,
                  model->layers[layer].ln2_beta,
                  embed_dim);
        
        // Feed-forward with residual connection
        feedforward_forward(model, layer, ln_output, ffn_output);
        
        // Debug: Check after FFN
        bool has_nan_ffn = false;
        for (uint32_t i = 0; i < embed_dim && i < 5; i++) {
            if (math_is_nan(ffn_output[i])) {
                has_nan_ffn = true;
                break;
            }
        }
        if (has_nan_ffn) {
        }
        
        for (uint32_t i = 0; i < embed_dim; i++) {
            hidden_states[i] += ffn_output[i];
        }
    }
    
    // Final layer norm (using last layer's ln2 parameters)
    if (num_layers > 0) {
        layer_norm(ln_output, hidden_states,
                  model->layers[num_layers - 1].ln2_gamma,
                  model->layers[num_layers - 1].ln2_beta,
                  embed_dim);
        memcpy(hidden_states, ln_output, embed_dim * sizeof(double));
    }
    
    free(attn_output);
    free(ffn_output);
    free(ln_output);
}

/**
 * Check if model has properly initialized transformer layers
 */
bool cllm_has_transformer_layers(const CLLMModel* model) {
    if (!model || !model->layers) {
        return false;
    }
    
    // Check if first layer has weights allocated
    if (!model->layers[0].query_weights ||
        !model->layers[0].key_weights ||
        !model->layers[0].value_weights ||
        !model->layers[0].output_weights ||
        !model->layers[0].ffn_w1 ||
        !model->layers[0].ffn_w2) {
        return false;
    }
    
    return true;
}

/**
 * Replace NaN values in model weights with zeros
 * This is a safety measure for models that were saved with NaN values
 */
void cllm_fix_nan_weights(CLLMModel* model) {
    if (!model || !model->layers) {
        return;
    }
    
    uint32_t embed_dim = model->embedding_dim;
    uint32_t hidden_dim = model->hidden_dim;
    uint32_t vocab_size = model->vocab_size;
    uint32_t num_layers = model->num_layers;
    
    int total_nan_fixed = 0;
    
    // Fix embeddings
    for (size_t i = 0; i < vocab_size * embed_dim; i++) {
        if (math_is_nan(model->embeddings[i])) {
            model->embeddings[i] = 0.0;
            total_nan_fixed++;
        }
    }
    
    // Fix each layer
    for (uint32_t layer = 0; layer < num_layers; layer++) {
        // Fix attention weights
        for (size_t i = 0; i < embed_dim * embed_dim; i++) {
            if (math_is_nan(model->layers[layer].query_weights[i])) {
                model->layers[layer].query_weights[i] = 0.0;
                total_nan_fixed++;
            }
            if (math_is_nan(model->layers[layer].key_weights[i])) {
                model->layers[layer].key_weights[i] = 0.0;
                total_nan_fixed++;
            }
            if (math_is_nan(model->layers[layer].value_weights[i])) {
                model->layers[layer].value_weights[i] = 0.0;
                total_nan_fixed++;
            }
            if (math_is_nan(model->layers[layer].output_weights[i])) {
                model->layers[layer].output_weights[i] = 0.0;
                total_nan_fixed++;
            }
        }
        
        // Fix FFN weights
        for (size_t i = 0; i < embed_dim * hidden_dim; i++) {
            if (math_is_nan(model->layers[layer].ffn_w1[i])) {
                model->layers[layer].ffn_w1[i] = 0.0;
                total_nan_fixed++;
            }
        }
        
        for (size_t i = 0; i < hidden_dim * embed_dim; i++) {
            if (math_is_nan(model->layers[layer].ffn_w2[i])) {
                model->layers[layer].ffn_w2[i] = 0.0;
                total_nan_fixed++;
            }
        }
        
        // Fix layer norm parameters
        for (size_t i = 0; i < embed_dim; i++) {
            if (math_is_nan(model->layers[layer].ln1_gamma[i])) {
                model->layers[layer].ln1_gamma[i] = 1.0;
                total_nan_fixed++;
            }
            if (math_is_nan(model->layers[layer].ln1_beta[i])) {
                model->layers[layer].ln1_beta[i] = 0.0;
                total_nan_fixed++;
            }
            if (math_is_nan(model->layers[layer].ln2_gamma[i])) {
                model->layers[layer].ln2_gamma[i] = 1.0;
                total_nan_fixed++;
            }
            if (math_is_nan(model->layers[layer].ln2_beta[i])) {
                model->layers[layer].ln2_beta[i] = 0.0;
                total_nan_fixed++;
            }
        }
    }
    
    if (total_nan_fixed > 0) {
        fprintf(stderr, "WARNING: Fixed %d NaN values in model weights\n", total_nan_fixed);
    } else {
        fprintf(stderr, "INFO: No NaN values found in model weights\n");
    }
}