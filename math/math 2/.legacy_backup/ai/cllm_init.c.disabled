/**
 * CLLM Weight Initialization
 * 
 * Implements various weight initialization strategies for neural networks:
 * - Xavier/Glorot initialization (for sigmoid/tanh activations)
 * - He initialization (for ReLU/GELU activations)
 * - Orthogonal initialization (for recurrent connections)
 * - Zero/Uniform/Normal initialization
 * - Layer-specific initialization
 * 
 * Mathematical foundations:
 * - Xavier: Var(W) = 2/(n_in + n_out)
 * - He: Var(W) = 2/n_in
 * - Orthogonal: W^T * W = I
 */

// Include math functions
#include "../include/prime_float_math.h"

#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_training.h"
#include <stdlib.h>
#include <stdio.h>
#include "../include/prime_float_math.h"
#include <time.h>

// Random number generation utilities
static int rng_initialized = 0;

static void ensure_rng_initialized(void) {
    if (!rng_initialized) {
        srand((unsigned int)time(NULL));
        rng_initialized = 1;
    }
}

// Generate uniform random number in [0, 1]
static double uniform_random(void) {
    return (double)rand() / (double)RAND_MAX;
}

// Generate uniform random number in [a, b]
static double uniform_random_range(double a, double b) {
    return a + (b - a) * uniform_random();
}

// Box-Muller transform for normal distribution
static double normal_random(double mean, double stddev) {
    static int has_spare = 0;
    static double spare;
    
    if (has_spare) {
        has_spare = 0;
        return mean + stddev * spare;
    }
    
    has_spare = 1;
    double u, v, s;
    do {
        u = uniform_random() * 2.0 - 1.0;
        v = uniform_random() * 2.0 - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);
    
    spare = v * s;
    return mean + stddev * u * s;
}

/**
 * Xavier/Glorot Uniform Initialization
 * 
 * Samples weights from uniform distribution:
 * 
 * Best for: sigmoid, tanh activations
 */
void cllm_init_xavier_uniform(double* weights, int n_in, int n_out) {
    if (!weights) return;
    
    ensure_rng_initialized();
    
    int total = n_in * n_out;
    double limit = prime_sqrt(6.0 / (double)(n_in + n_out));
    
    for (int i = 0; i < total; i++) {
        weights[i] = uniform_random_range(-limit, limit);
    }
}

/**
 * Xavier/Glorot Normal Initialization
 * 
 * Samples weights from normal distribution:
 * 
 * Best for: sigmoid, tanh activations
 */
void cllm_init_xavier_normal(double* weights, int n_in, int n_out) {
    if (!weights) return;
    
    ensure_rng_initialized();
    
    int total = n_in * n_out;
    double stddev = prime_sqrt(2.0 / (double)(n_in + n_out));
    
    for (int i = 0; i < total; i++) {
        weights[i] = normal_random(0.0, stddev);
    }
}

/**
 * He Uniform Initialization
 * 
 * Samples weights from uniform distribution:
 * 
 * Best for: ReLU, GELU, Leaky ReLU activations
 */
void cllm_init_he_uniform(double* weights, int n_in, int n_out) {
    if (!weights) return;
    
    ensure_rng_initialized();
    
    int total = n_in * n_out;
    double limit = prime_sqrt(6.0 / (double)n_in);
    
    for (int i = 0; i < total; i++) {
        weights[i] = uniform_random_range(-limit, limit);
    }
}

/**
 * He Normal Initialization
 * 
 * Samples weights from normal distribution:
 * 
 * Best for: ReLU, GELU, Leaky ReLU activations
 * This is the recommended initialization for transformers
 */
void cllm_init_he_normal(double* weights, int n_in, int n_out) {
    if (!weights) return;
    
    ensure_rng_initialized();
    
    int total = n_in * n_out;
    double stddev = prime_sqrt(2.0 / (double)n_in);
    
    for (int i = 0; i < total; i++) {
        weights[i] = normal_random(0.0, stddev);
    }
}

/**
 * Orthogonal Initialization
 * 
 * Initializes weight matrix to be orthogonal using QR decomposition
 * Useful for recurrent connections and deep networks
 * 
 * Algorithm:
 * 1. Sample random matrix from N(0,1)
 * 2. Perform QR decomposition
 * 3. Use Q as the weight matrix
 */
void cllm_init_orthogonal(double* weights, int n_in, int n_out, double gain) {
    if (!weights) return;
    
    ensure_rng_initialized();
    
    int rows = n_out;
    int cols = n_in;
    int total = rows * cols;
    
    // Step 1: Initialize with random normal values
    for (int i = 0; i < total; i++) {
        weights[i] = normal_random(0.0, 1.0);
    }
    
    // Step 2: Gram-Schmidt orthogonalization (simplified QR)
    // For each column, orthogonalize against previous columns
    for (int j = 0; j < cols; j++) {
        // Orthogonalize column j against columns 0..j-1
        for (int k = 0; k < j; k++) {
            // Compute dot product
            double dot = 0.0;
            for (int i = 0; i < rows; i++) {
                dot += weights[i * cols + j] * weights[i * cols + k];
            }
            
            // Subtract projection
            for (int i = 0; i < rows; i++) {
                weights[i * cols + j] -= dot * weights[i * cols + k];
            }
        }
        
        // Normalize column j
        double norm = 0.0;
        for (int i = 0; i < rows; i++) {
            norm += weights[i * cols + j] * weights[i * cols + j];
        }
        
        if (norm > 1e-10) {
            for (int i = 0; i < rows; i++) {
                weights[i * cols + j] /= norm;
            }
        }
    }
    
    // Step 3: Apply gain
    if (gain != 1.0) {
        for (int i = 0; i < total; i++) {
            weights[i] *= gain;
        }
    }
}

/**
 * Zero Initialization
 * 
 * Initializes all weights to zero
 * Typically used for biases
 */
void cllm_init_zeros(double* weights, int size) {
    if (!weights) return;
    
    for (int i = 0; i < size; i++) {
        weights[i] = 0.0;
    }
}

/**
 * Constant Initialization
 * 
 * Initializes all weights to a constant value
 */
void cllm_init_constant(double* weights, int size, double value) {
    if (!weights) return;
    
    for (int i = 0; i < size; i++) {
        weights[i] = value;
    }
}

/**
 * Uniform Initialization
 * 
 * Samples weights from uniform distribution U[a, b]
 */
void cllm_init_uniform(double* weights, int size, double a, double b) {
    if (!weights) return;
    
    ensure_rng_initialized();
    
    for (int i = 0; i < size; i++) {
        weights[i] = uniform_random_range(a, b);
    }
}

/**
 * Normal Initialization
 * 
 * Samples weights from normal distribution N(mean, stddev^2)
 */
void cllm_init_normal(double* weights, int size, double mean, double stddev) {
    if (!weights) return;
    
    ensure_rng_initialized();
    
    for (int i = 0; i < size; i++) {
        weights[i] = normal_random(mean, stddev);
    }
}

/**
 * Initialize Embedding Layer
 * 
 * Uses normal initialization with small standard deviation
 */
void cllm_init_embedding_layer(Embeddings* embed) {
    if (!embed || !embed->embeddings) return;
    
    // NOTE: This function is now deprecated in favor of lattice-based initialization
    // See cllm_embeddings_init_lattice() in cllm_lattice_embeddings.c
    // 
    // Keeping this for backward compatibility, but it should not be used
    // for new models. Use cllm_init_embeddings_with_lattice() instead.
    
    // Use small standard deviation for embeddings
    double stddev = 0.02;
    
    int total = embed->vocab_size * embed->embedding_dim;
    
    // Convert float array to double for initialization
    double* temp = (double*)malloc(total * sizeof(double));
    if (!temp) return;
    
    cllm_init_normal(temp, total, 0.0, stddev);
    
    // Copy back to float array
    for (int i = 0; i < total; i++) {
        embed->embeddings[i] = (float)temp[i];
    }
    
    free(temp);
}

/**
 * Initialize Attention Layer
 * 
 * Uses Xavier initialization for Q, K, V projections
 */
void cllm_init_attention_layer(AttentionLayer* attn) {
    if (!attn) return;
    
    int num_heads = attn->num_heads;
    int head_dim = attn->head_dim;
    int d_model = num_heads * head_dim;
    
    // Initialize query lattice
    if (attn->query_lattice) {
        cllm_init_xavier_uniform(attn->query_lattice, d_model, d_model);
    }
    
    // Initialize key lattice
    if (attn->key_lattice) {
        cllm_init_xavier_uniform(attn->key_lattice, d_model, d_model);
    }
    
    // Initialize value lattice
    if (attn->value_lattice) {
        cllm_init_xavier_uniform(attn->value_lattice, d_model, d_model);
    }
}

/**
 * Initialize Feed-Forward Layer
 * 
 * Uses He initialization for weights (GELU activation)
 * Initializes biases to zero
 */
void cllm_init_feedforward_layer(FeedForwardLayer* ffn) {
    if (!ffn) return;
    
    int input_dim = ffn->input_dim;
    int hidden_dim = ffn->hidden_dim;
    int output_dim = ffn->output_dim;
    
    // Initialize first layer (input_dim -> hidden_dim)
    if (ffn->w1_lattice) {
        cllm_init_he_normal(ffn->w1_lattice, input_dim, hidden_dim);
    }
    
    if (ffn->bias1) {
        for (int i = 0; i < hidden_dim; i++) {
            ffn->bias1[i] = 0.0;
        }
    }
    
    // Initialize second layer (hidden_dim -> output_dim)
    if (ffn->w2_lattice) {
        cllm_init_he_normal(ffn->w2_lattice, hidden_dim, output_dim);
    }
    
    if (ffn->bias2) {
        for (int i = 0; i < output_dim; i++) {
            ffn->bias2[i] = 0.0;
        }
    }
}

/**
 * Initialize Layer Normalization
 * 
 * Initializes gamma (scale) to 1.0 and beta (shift) to 0.0
 */
void cllm_init_layernorm(LayerNorm* ln) {
    if (!ln) return;
    
    int size = ln->size;
    
    // Initialize scale to 1.0
    if (ln->gamma) {
        for (int i = 0; i < size; i++) {
            ln->gamma[i] = 1.0;
        }
    }
    
    // Initialize shift to 0.0
    if (ln->beta) {
        for (int i = 0; i < size; i++) {
            ln->beta[i] = 0.0;
        }
    }
}

/**
 * Initialize CLLM Layer Normalization
 * 
 * Initializes gamma (scale) to 1.0 and beta (shift) to 0.0
 */
void cllm_init_cllm_layernorm(CLLMLayerNorm* ln) {
    if (!ln) return;
    
    int dim = ln->dim;
    
    // Initialize scale to 1.0
    if (ln->gamma) {
        for (int i = 0; i < dim; i++) {
            ln->gamma[i] = 1.0;
        }
    }
    
    // Initialize shift to 0.0
    if (ln->beta) {
        for (int i = 0; i < dim; i++) {
            ln->beta[i] = 0.0;
        }
    }
}

/**
 * Initialize Complete CLLM Model with Lattice Formula
 * 
 * Uses crystalline lattice formula L(n,d,k,λ) for embeddings.
 * This is the CORRECT initialization method for crystalline models.
 */
void cllm_init_model_lattice(CLLMModel* model) {
    if (!model) return;
    
    printf("\n=== Initializing CLLM Model with Lattice Formula ===\n");
    
    // Initialize embedding layer with lattice formula
    cllm_init_embeddings_with_lattice(model);
    
    // Initialize transformer layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Initialize attention
        if (model->attention_layers) {
            cllm_init_attention_layer(&model->attention_layers[i]);
        }
        
        // Initialize feed-forward
        if (model->ff_layers) {
            cllm_init_feedforward_layer(&model->ff_layers[i]);
        }
        
        // Initialize layer norms
        if (model->layer_norms) {
            cllm_init_layernorm((LayerNorm*)&model->layer_norms[i]);
        }
    }
    
    printf("✓ Model initialization complete\n\n");
}

/**
 * Initialize Complete CLLM Model (Legacy)
 * 
 * Initializes all layers in the model with appropriate strategies.
 * NOTE: This uses random initialization. For crystalline models,
 * use cllm_init_model_lattice() instead.
 */
void cllm_init_model(CLLMModel* model) {
    if (!model) return;
    
    // Initialize embedding layer (random - deprecated)
    cllm_init_embedding_layer(&model->embeddings);
    
    // Initialize transformer layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Initialize attention
        if (model->attention_layers) {
            cllm_init_attention_layer(&model->attention_layers[i]);
        }
        
        // Initialize feed-forward
        if (model->ff_layers) {
            cllm_init_feedforward_layer(&model->ff_layers[i]);
        }
        
        // Initialize layer norms (CLLMLayerNorm type)
        if (model->layer_norms) {
            cllm_init_cllm_layernorm(&model->layer_norms[i * 2]);     // ln1
            cllm_init_cllm_layernorm(&model->layer_norms[i * 2 + 1]); // ln2
        }
    }
}

/**
 * Initialize Model with Seed
 * 
 * Allows custom initialization with specific random seed
 */
void cllm_init_model_with_seed(CLLMModel* model, unsigned int seed) {
    if (!model) return;
    
    // Set random seed
    srand(seed);
    rng_initialized = 1;
    
    // Initialize model with default strategy
    cllm_init_model(model);
}