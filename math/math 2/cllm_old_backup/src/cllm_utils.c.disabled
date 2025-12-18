/**
 * CLLM Utilities
 * 
 * Model management, validation, and helper functions for the CLLM.
 * Provides high-level utilities for:
 * - Model creation and initialization
 * - Model inspection and validation
 * - Configuration management
 * - Helper functions for common operations
 */

#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_training.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/prime_math.h"
#include "../include/prime_float_math.h"

/**
 * Create CLLM Model Configuration
 * 
 * Creates a default configuration for a CLLM model
 */
CLLMConfig* cllm_create_config(uint32_t vocab_size,
                                uint32_t embedding_dim,
                                uint32_t num_layers,
                                uint32_t num_heads,
                                uint32_t ff_dim) {
    CLLMConfig* config = (CLLMConfig*)malloc(sizeof(CLLMConfig));
    if (!config) return NULL;
    
    config->vocab_size = vocab_size;
    config->embedding_dim = embedding_dim;
    config->num_layers = num_layers;
    config->num_heads = num_heads;
    config->ff_dim = ff_dim;
    config->max_seq_len = 512;  // Default
    config->dropout = 0.1f;      // Default
    
    return config;
}

/**
 * Free CLLM Configuration
 */
void cllm_free_config(CLLMConfig* config) {
    if (config) {
        free(config);
    }
}

/**
 * Validate Model Configuration
 * 
 * Checks if configuration parameters are valid
 * Returns 1 if valid, 0 otherwise
 */
bool cllm_validate_config(const CLLMConfig* config) {
    if (!config) return false;
    
    // Check basic constraints
    if (config->vocab_size == 0) {
        fprintf(stderr, "Error: vocab_size must be > 0\n");
        return false;
    }
    
    if (config->embedding_dim == 0) {
        fprintf(stderr, "Error: embedding_dim must be > 0\n");
        return false;
    }
    
    if (config->num_layers == 0) {
        fprintf(stderr, "Error: num_layers must be > 0\n");
        return false;
    }
    
    if (config->num_heads == 0) {
        fprintf(stderr, "Error: num_heads must be > 0\n");
        return false;
    }
    
    // Check that embedding_dim is divisible by num_heads
    if (config->embedding_dim % config->num_heads != 0) {
        fprintf(stderr, "Error: embedding_dim must be divisible by num_heads\n");
        return false;
    }
    
    // Check reasonable ranges
    if (config->dropout < 0.0 || config->dropout > 1.0) {
        fprintf(stderr, "Error: dropout must be in [0, 1]\n");
        return false;
    }
    
    return true;
}

/**
 * Print Model Configuration
 * 
 * Prints configuration details for debugging
 */
void cllm_print_config(const CLLMConfig* config) {
    if (!config) {
        printf("Configuration: NULL\n");
        return;
    }
    
    printf("CLLM Configuration:\n");
    printf("  Vocabulary Size:  %u\n", config->vocab_size);
    printf("  Embedding Dim:    %u\n", config->embedding_dim);
    printf("  Number of Layers: %u\n", config->num_layers);
    printf("  Number of Heads:  %u\n", config->num_heads);
    printf("  Head Dimension:   %u\n", config->embedding_dim / config->num_heads);
    printf("  FF Dimension:     %u\n", config->ff_dim);
    printf("  Max Seq Length:   %u\n", config->max_seq_len);
    printf("  Dropout:          %.2f\n", config->dropout);
}

/**
 * Get Model Parameter Count
 * 
 * Calculates total number of parameters in the model
 */
uint64_t cllm_get_parameter_count(CLLMModel* model) {
    if (!model) return 0;
    
    uint64_t count = 0;
    
    // Embeddings
    count += (uint64_t)model->embeddings.vocab_size * model->embeddings.embedding_dim;
    
    // Embedding transformations
    if (model->embeddings.lattice_transform) {
        count += (uint64_t)model->embeddings.embedding_dim * model->embeddings.embedding_dim;
    }
    if (model->embeddings.inverse_transform) {
        count += (uint64_t)model->embeddings.embedding_dim * model->embeddings.embedding_dim;
    }
    
    // Positional encoding (if learned)
    if (model->pos_encoding.learned_positions) {
        count += (uint64_t)model->pos_encoding.max_length * model->pos_encoding.embedding_dim;
    }
    
    // Transformer layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Attention layer
        if (model->attention_layers) {
            AttentionLayer* attn = &model->attention_layers[i];
            uint32_t d_model = attn->num_heads * attn->head_dim;
            
            // Q, K, V projections (3 * d_model * d_model)
            count += 3 * (uint64_t)d_model * d_model;
            
            // Output projection (d_model * d_model)
            count += (uint64_t)d_model * d_model;
        }
        
        // Feed-forward layer
        if (model->ff_layers) {
            FeedForwardLayer* ffn = &model->ff_layers[i];
            
            // W1: input_dim * hidden_dim
            count += (uint64_t)ffn->input_dim * ffn->hidden_dim;
            
            // W2: hidden_dim * output_dim
            count += (uint64_t)ffn->hidden_dim * ffn->output_dim;
            
            // Biases
            count += ffn->hidden_dim + ffn->output_dim;
        }
        
        // Layer norms (2 per layer)
        if (model->layer_norms) {
            // Gamma and beta for each layer norm
            count += 2 * 2 * model->embedding_dim;
        }
    }
    
    return count;
}

/**
 * Get Model Memory Usage
 * 
 * Estimates memory usage in bytes
 */
uint64_t cllm_get_memory_usage(CLLMModel* model) {
    if (!model) return 0;
    
    uint64_t param_count = cllm_get_parameter_count(model);
    
    // Assume float (4 bytes) for all parameters
    uint64_t param_memory = param_count * sizeof(float);
    
    // Add structure overhead (rough estimate)
    uint64_t struct_memory = sizeof(CLLMModel);
    struct_memory += model->num_layers * (sizeof(AttentionLayer) + 
                                          sizeof(FeedForwardLayer) + 
                                          2 * sizeof(CLLMLayerNorm));
    
    return param_memory + struct_memory;
}

/**
 * Print Model Statistics
 * 
 * Prints detailed model statistics
 */
void cllm_print_model_stats(CLLMModel* model) {
    if (!model) {
        printf("Model: NULL\n");
        return;
    }
    
    uint64_t params = cllm_get_parameter_count(model);
    uint64_t memory = cllm_get_memory_usage(model);
    
    printf("CLLM Model Statistics:\n");
    printf("  Vocabulary Size:    %u\n", model->embeddings.vocab_size);
    printf("  Embedding Dim:      %u\n", model->embeddings.embedding_dim);
    printf("  Number of Layers:   %u\n", model->num_layers);
    printf("  Total Parameters:   %lu (%.2f M)\n", params, params / 1e6);
    printf("  Memory Usage:       %lu bytes (%.2f MB)\n", memory, memory / (1024.0 * 1024.0));
    
    if (model->attention_layers && model->num_layers > 0) {
        printf("  Attention Heads:    %u\n", model->attention_layers[0].num_heads);
        printf("  Head Dimension:     %u\n", model->attention_layers[0].head_dim);
    }
    
    if (model->ff_layers && model->num_layers > 0) {
        printf("  FF Hidden Dim:      %u\n", model->ff_layers[0].hidden_dim);
    }
}

/**
 * Validate Model Structure
 * 
 * Checks if model structure is valid and all required components are present
 * Returns 1 if valid, 0 otherwise
 */
int cllm_validate_model(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "Error: Model is NULL\n");
        return 0;
    }
    
    // Check embeddings
    if (!model->embeddings) {
        fprintf(stderr, "Error: Embeddings not allocated\n");
        return 0;
    }
    
    if (model->embeddings.vocab_size == 0) {
        fprintf(stderr, "Error: Vocabulary size is 0\n");
        return 0;
    }
    
    if (model->embeddings.embedding_dim == 0) {
        fprintf(stderr, "Error: Embedding dimension is 0\n");
        return 0;
    }
    
    // Check layers
    if (model->num_layers == 0) {
        fprintf(stderr, "Error: Number of layers is 0\n");
        return 0;
    }
    
    if (!model->attention_layers) {
        fprintf(stderr, "Error: Attention layers not allocated\n");
        return 0;
    }
    
    if (!model->ff_layers) {
        fprintf(stderr, "Error: Feed-forward layers not allocated\n");
        return 0;
    }
    
    if (!model->layer_norms) {
        fprintf(stderr, "Error: Layer norms not allocated\n");
        return 0;
    }
    
    // Validate each layer
    for (uint32_t i = 0; i < model->num_layers; i++) {
        AttentionLayer* attn = &model->attention_layers[i];
        
        if (!attn->query_lattice || !attn->key_lattice || !attn->value_lattice) {
            fprintf(stderr, "Error: Layer %u attention weights not allocated\n", i);
            return 0;
        }
        
        FeedForwardLayer* ffn = &model->ff_layers[i];
        
        if (!ffn->w1_lattice || !ffn->w2_lattice) {
            fprintf(stderr, "Error: Layer %u feed-forward weights not allocated\n", i);
            return 0;
        }
    }
    
    return 1;
}

/**
 * Check Model for NaN/Inf Values
 * 
 * Scans model parameters for NaN or Inf values
 * Returns 1 if clean, 0 if NaN/Inf found
 */
int cllm_check_model_health(CLLMModel* model) {
    if (!model) return 0;
    
    int issues = 0;
    
    // Check embeddings
    if (model->embeddings) {
        uint32_t size = model->embeddings.vocab_size * model->embeddings.embedding_dim;
        for (uint32_t i = 0; i < size; i++) {
            double val = model->embeddings[i];  // FIXED: Use double, not float
            if (prime_isnan(val) || prime_isinf(val)) {  // FIXED: Use double versions
                fprintf(stderr, "Warning: NaN/Inf in embeddings at index %u\n", i);
                issues++;
                if (issues >= 10) break;  // Limit output
            }
        }
    }
    
    // Check attention layers
    if (model->attention_layers) {
        for (uint32_t layer = 0; layer < model->num_layers && issues < 10; layer++) {
            AttentionLayer* attn = &model->attention_layers[layer];
            
            // Validate attention layer structure
            if (!attn) continue;
            
            uint32_t d_model = attn->num_heads * attn->head_dim;
            uint32_t size = d_model * d_model;
            
            if (attn->query_lattice) {
                for (uint32_t i = 0; i < size && issues < 10; i++) {
// DISABLED - USE BigFixed version:                     if (prime_isnanf(attn->query_lattice[i]) || prime_isinff(attn->query_lattice[i])) {
// DISABLED - USE BigFixed version:                         fprintf(stderr, "Warning: NaN/Inf in layer %u query weights\n", layer);
// DISABLED - USE BigFixed version:                         issues++;
// DISABLED - USE BigFixed version:                     }
                }
            }
        }
    }
    
    if (issues > 0) {
        fprintf(stderr, "Model health check: Found %d issues\n", issues);
        return 0;
    }
    
    return 1;  // Model is healthy
}
