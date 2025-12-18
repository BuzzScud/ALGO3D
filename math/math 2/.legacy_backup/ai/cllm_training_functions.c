/**
 * @file cllm_training.c
 * @brief Training Functions for Geometric CLLMModel
 * 
 * Implements training functionality that integrates with cllm_training_threaded.c
 * Includes: precomputation, initialization, loss computation, optimizer steps
 */

#include "cllm.h"
#include "math/constants.h"
#include "cllm_training.h"
#include "cllm_batch.h"
#include "ai/cllm_training_system.h"
#include "math/constants.h"
#include "cllm_inference_transformer.h"
#include "math/constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/transcendental.h"
#include "math/constants.h"

// ============================================================================
// EMBEDDING PRECOMPUTATION
// ============================================================================

/**
 * Precompute all embeddings
 * 
 * For the new geometric model, embeddings are already initialized during
 * model creation with clock lattice positions. This function ensures they
 * are properly set up and applies harmonic modulation if enabled.
 */
void cllm_precompute_all_embeddings(CLLMModel* model) {
    if (!model) return;
    
    printf("Pre-computing embeddings for %u tokens...\n", model->vocab_size);
    fflush(stdout);
    
    // Embeddings are already initialized in cllm_create_model
    // Just verify and apply harmonic modulation if enabled
    for (uint32_t token_id = 0; token_id < model->vocab_size; token_id++) {
        // Apply harmonic modulation if enabled
        if (model->harmonic.enabled && model->harmonic.fourier_coefficients) {
            double* embedding = &model->embeddings[token_id * model->embedding_dim];
            
            // Apply cymatic frequency modulation
            for (uint32_t dim = 0; dim < model->embedding_dim; dim++) {
                double freq_idx = (double)dim / model->embedding_dim;
                double freq = model->harmonic.primary_frequency * (1.0 + freq_idx);
                double modulation = math_cos(2.0 * MATH_PI * freq * token_id / model->vocab_size);
                embedding[dim] *= (1.0 + 0.1 * modulation);  // 10% modulation
            }
        }
        
        // Progress indicator
        if ((token_id + 1) % 1000 == 0) {
            printf("\r  Computed %u/%u embeddings...", token_id + 1, model->vocab_size);
            fflush(stdout);
        }
    }
    
    printf("\r✓ Pre-computed all %u embeddings\n", model->vocab_size);
    fflush(stdout);
}

// ============================================================================
// TRAINING INITIALIZATION
// ============================================================================

/**
 * Initialize training state
 */
CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config) {
    if (!model || !config) return NULL;
    
    CLLMTraining* training = calloc(1, sizeof(CLLMTraining));
    if (!training) return NULL;
    
    training->model = model;
    training->config = *config;
    training->current_epoch = 0;
    training->current_step = 0;
    training->best_loss = 1e9;
    training->accumulation_step = 0;
    
    // Store initial learning rate
    training->config.initial_learning_rate = config->learning_rate;
    
    // Allocate logits buffer
    size_t max_tokens = config->batch_size * config->sequence_length;
    training->logits = calloc(max_tokens * model->vocab_size, sizeof(double));
    if (!training->logits) {
        free(training);
        return NULL;
    }
    
    // Allocate gradient buffer
    training->gradient_buffer = calloc(max_tokens * model->embedding_dim, sizeof(double));
    if (!training->gradient_buffer) {
        free(training->logits);
        free(training);
        return NULL;
    }
    
    // CRITICAL FIX: Allocate gradients buffer for optimizer
    // This buffer stores gradients for vocabulary embeddings
    // So it needs vocab_size * embedding_dim, not max_tokens * embedding_dim
    training->gradients = calloc(model->vocab_size * model->embedding_dim, sizeof(double));
    if (!training->gradients) {
        free(training->gradient_buffer);
        free(training->logits);
        free(training);
        return NULL;
    }
    
    // Initialize optimizer buffers (Adam)
    if (model->optimizer.type == OPTIMIZER_ADAM) {
        // Calculate total parameters
        size_t total_params = 0;
        
        // Embeddings
        total_params += model->vocab_size * model->embedding_dim;
        
        // Positional encoding
        total_params += model->max_seq_len * model->embedding_dim;
        
        // Layers
        for (uint32_t i = 0; i < model->num_layers; i++) {
            // Attention
            total_params += 4 * model->embedding_dim * model->embedding_dim;
            // FFN
            total_params += model->embedding_dim * model->hidden_dim;
            total_params += model->hidden_dim * model->embedding_dim;
            total_params += model->hidden_dim + model->embedding_dim;
            // Layer norm
            total_params += 4 * model->embedding_dim;
        }
        
        // Output
        total_params += model->embedding_dim * model->vocab_size;
        total_params += model->vocab_size;
        
        // Allocate momentum buffers
        model->optimizer.m = calloc(total_params, sizeof(double));
        model->optimizer.v = calloc(total_params, sizeof(double));
        model->optimizer.t = 0;
        
        if (!model->optimizer.m || !model->optimizer.v) {
            fprintf(stderr, "Warning: Failed to allocate optimizer buffers\n");
        }
    }
    
    printf("✓ Training initialized\n");
    printf("  Batch size: %d\n", config->batch_size);
    printf("  Sequence length: %d\n", config->sequence_length);
    printf("  Learning rate: %.6f\n", config->learning_rate);
    printf("  Optimizer: %s\n", model->optimizer.type == OPTIMIZER_ADAM ? "Adam" : 
                                 model->optimizer.type == OPTIMIZER_SGD ? "SGD" : "RMSProp");
    
    return training;
}

// ============================================================================
// TRAINING CLEANUP
// ============================================================================

/**
 * Free training state
 */
void cllm_training_free(CLLMTraining* training) {
    if (!training) return;
    
    free(training->logits);
    free(training->gradient_buffer);
    free(training->gradients);  // CRITICAL FIX: Free gradients buffer
    free(training);
    
    printf("✓ Training freed\n");
}

// ============================================================================
// TRAINING LOOP
// ============================================================================

/**
 * Train model
 * 
 * This is a wrapper that calls cllm_training_threaded functions
 */
int cllm_train(CLLMTraining* training) {
    if (!training) return -1;
    
    printf("Training model (using cllm_training_threaded)\n");
    printf("Note: This is a wrapper - actual training uses threaded implementation\n");
    
    // TODO: Call cllm_training_threaded functions
    // For now, this is a placeholder that returns success
    
    return 0;
}

// ============================================================================
// FORWARD PASS
// ============================================================================

/**
 * Forward pass for training
 * 
 * Computes logits for input tokens using the model
 */
double cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens) {
    if (!training || !input_tokens) return 0.0;
    
    CLLMModel* model = training->model;
    int num_tokens = training->config.batch_size * training->config.sequence_length;
    uint32_t embed_dim = model->embedding_dim;
    
    // Allocate buffer for hidden states
    double* hidden_states = (double*)calloc(embed_dim, sizeof(double));
    if (!hidden_states) {
        fprintf(stderr, "Failed to allocate hidden states buffer\n");
        return 0.0;
    }
    
    // CRITICAL FIX: Process each token through the full transformer pipeline
    for (int i = 0; i < num_tokens; i++) {
        uint32_t token = input_tokens[i];
        if (token >= model->vocab_size) continue;
        
        // Step 1: Get embedding
        double* embedding = &model->embeddings[token * embed_dim];
        memcpy(hidden_states, embedding, embed_dim * sizeof(double));
        
        // Step 2: CRITICAL - Process through transformer layers
        // This is what was missing - we must use the transformer!
        cllm_transformer_forward(model, hidden_states);
        
        // Step 3: Project to vocabulary (output layer)
        double* logits = &training->logits[i * model->vocab_size];
        
        for (uint32_t v = 0; v < model->vocab_size; v++) {
            double sum = model->output_bias[v];
            for (uint32_t d = 0; d < embed_dim; d++) {
                sum += hidden_states[d] * model->output_weights[d * model->vocab_size + v];
            }
            logits[v] = sum;
        }
    }
    
    free(hidden_states);
    return 0.0;  // Loss computed separately
}

// ============================================================================
// LOSS COMPUTATION
// ============================================================================

/**
 * Compute GCD of two numbers (for GCD-based similarity)
 */
static uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Compute GCD-based similarity between tokens
 */
static double gcd_similarity(uint32_t token1, uint32_t token2) {
    if (token1 == 0 || token2 == 0) return 0.0;
    
    uint32_t shared = gcd(token1 + 1, token2 + 1);  // +1 to avoid zero
    uint32_t max_val = (token1 > token2) ? token1 : token2;
    
    return (double)shared / (double)(max_val + 1);
}

/**
 * Compute loss with GCD-based similarity
 */
double cllm_compute_loss(CLLMTraining* training, uint32_t* input_tokens, 
                        uint32_t* target_tokens, int num_tokens) {
    if (!training || !target_tokens) return 0.0;
    
    (void)input_tokens;  // Unused in this implementation
    
    CLLMModel* model = training->model;
    double total_loss = 0.0;
    int count = 0;
    
    // Compute cross-entropy loss with GCD similarity bonus
    for (int i = 0; i < num_tokens; i++) {
        uint32_t target = target_tokens[i];
        if (target >= model->vocab_size) continue;
        
        double* logits = &training->logits[i * model->vocab_size];
        
        // Compute softmax (numerically stable)
        double max_logit = logits[0];
        for (uint32_t v = 1; v < model->vocab_size; v++) {
            if (logits[v] > max_logit) max_logit = logits[v];
        }
        
        double sum_exp = 0.0;
        for (uint32_t v = 0; v < model->vocab_size; v++) {
            sum_exp += math_exp(logits[v] - max_logit);
        }
        
        // Cross-entropy loss
        double log_prob = (logits[target] - max_logit) - math_log(sum_exp);
        double ce_loss = -log_prob;
        
        // Add GCD similarity bonus (encourages related tokens)
        double gcd_bonus = 0.0;
        if (i > 0) {
            uint32_t prev_target = target_tokens[i - 1];
            if (prev_target < model->vocab_size) {
                gcd_bonus = gcd_similarity(target, prev_target);
            }
        }
        
        // Combined loss (cross-entropy - small GCD bonus)
        total_loss += ce_loss - 0.01 * gcd_bonus;
        count++;
    }
    
    return count > 0 ? total_loss / count : 0.0;
}

// ============================================================================
// EMBEDDING COMPUTATION
// ============================================================================

/**
 * Compute embedding (lazy)
 * 
 * For geometric model, embeddings are pre-computed, so just copy
 */
void cllm_compute_embedding_lazy(CLLMModel* model, uint32_t token_id, double* output) {
    if (!model || !output || token_id >= model->vocab_size) return;
    
    memcpy(output, &model->embeddings[token_id * model->embedding_dim], 
           model->embedding_dim * sizeof(double));
}

// ============================================================================
// OPTIMIZER
// ============================================================================

/**
 * Adam optimizer step
 */
void cllm_optimizer_step(CLLMTraining* training) {
    // Wrapper that calls Adam optimizer
    cllm_optimizer_step_adam(training);
}

/**
 * Forward pass using thread-local context
 * 
 * Uses thread-local activation buffers to avoid race conditions.
 * Model weights are read-only (shared across threads).
 */
double cllm_forward_training_threaded(
    CLLMTraining* training,
    struct ThreadLocalTrainingContext* local_ctx,
    uint32_t* input_tokens
) {
    if (!training || !local_ctx || !input_tokens) return 0.0;
    
    CLLMModel* model = training->model;
    int batch_size = local_ctx->batch_size;
    int seq_len = local_ctx->seq_len;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Get embeddings (write to thread-local buffer)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = input_tokens[idx];
            if (token_id >= vocab_size) continue;
            
            double* embed_src = &model->embeddings[token_id * embed_dim];
            double* embed_dst = &local_ctx->input_embeddings[idx * embed_dim];
            memcpy(embed_dst, embed_src, embed_dim * sizeof(double));
        }
    }
    
    // Process through layers (all writes go to thread-local buffers)
    double* layer_input = local_ctx->input_embeddings;
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        memcpy(local_ctx->layer_inputs[layer], layer_input, 
               batch_size * seq_len * embed_dim * sizeof(double));
        
        // Simplified: copy input to output (identity mapping for now)
        // TODO: Implement full attention with thread-local caching
        memcpy(local_ctx->attention_outputs[layer], layer_input, 
               batch_size * seq_len * embed_dim * sizeof(double));
        
        // Simplified feedforward (identity for now)
        // TODO: Implement full feedforward
        memcpy(local_ctx->ff_outputs[layer], local_ctx->attention_outputs[layer],
               batch_size * seq_len * embed_dim * sizeof(double));
        
        // Layer output = attention output + feedforward (residual)
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                double* attn_out = &local_ctx->attention_outputs[layer][idx * embed_dim];
                double* ff_out = &local_ctx->ff_outputs[layer][idx * embed_dim];
                double* layer_out = &local_ctx->layer_outputs[layer][idx * embed_dim];
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    layer_out[d] = attn_out[d] + ff_out[d];
                }
            }
        }
        
        layer_input = local_ctx->layer_outputs[layer];
    }
    
    // Copy final hidden (to thread-local buffer)
    memcpy(local_ctx->final_hidden, layer_input, 
           batch_size * seq_len * embed_dim * sizeof(double));
    
    // Project to vocabulary (write to thread-local logits)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            double* hidden = &local_ctx->final_hidden[idx * embed_dim];
            double* logits = &local_ctx->logits[idx * vocab_size];
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                double* vocab_embed = &model->embeddings[v * embed_dim];
                double score = 0.0;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    score += hidden[d] * vocab_embed[d];
                }
                logits[v] = score;
            }
        }
    }
    
    return 0.0;  // Loss computation happens separately
}

/**
 * Backward pass using thread-local context
 * 
 * Uses thread-local gradient buffers to avoid race conditions.
 * Gradients are accumulated to the provided gradient_buffer (lock-free segment).
 */
void cllm_backward_training_threaded(
    CLLMTraining* training,
    struct ThreadLocalTrainingContext* local_ctx,
    uint32_t* target_tokens,
    double* gradient_buffer
) {
    if (!training || !local_ctx || !target_tokens || !gradient_buffer) return;
    
    CLLMModel* model = training->model;
    int batch_size = local_ctx->batch_size;
    int seq_len = local_ctx->seq_len;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Use thread-local temporary buffers
    double* grad_logits = local_ctx->grad_logits;
    double* grad_hidden = local_ctx->grad_hidden;
    
    // Zero the buffers
    memset(grad_logits, 0, batch_size * seq_len * vocab_size * sizeof(double));
    memset(grad_hidden, 0, batch_size * seq_len * embed_dim * sizeof(double));
    
    // Gradient of cross-entropy w.r.t. logits (using thread-local logits)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            if (target >= vocab_size) continue;
            
            double* logits = &local_ctx->logits[idx * vocab_size];
            double* grad = &grad_logits[idx * vocab_size];
            
            // Softmax gradient: prob - target_one_hot
            double max_logit = logits[0];
            for (uint32_t v = 1; v < vocab_size; v++) {
                if (logits[v] > max_logit) max_logit = logits[v];
            }
            
            double sum_exp = 0.0;
            for (uint32_t v = 0; v < vocab_size; v++) {
                double x = logits[v] - max_logit;
                if (x > 50.0) x = 50.0;
                if (x < -50.0) x = -50.0;
                sum_exp += exp(x);
            }
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                double x = logits[v] - max_logit;
                if (x > 50.0) x = 50.0;
                if (x < -50.0) x = -50.0;
                double prob = exp(x) / sum_exp;
                grad[v] = prob - (v == target ? 1.0 : 0.0);
            }
        }
    }
    
    // Backprop through vocabulary projection (write to gradient_buffer)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            double* grad_logit = &grad_logits[idx * vocab_size];
            double* hidden = &local_ctx->final_hidden[idx * embed_dim];
            double* grad_h = &grad_hidden[idx * embed_dim];
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                double* vocab_embed = &model->embeddings[v * embed_dim];
                double grad_v = grad_logit[v];
                
                // Accumulate to gradient_buffer (lock-free segment)
                for (uint32_t d = 0; d < embed_dim; d++) {
                    gradient_buffer[v * embed_dim + d] += grad_v * hidden[d];
                    grad_h[d] += grad_v * vocab_embed[d];
                }
            }
        }
    }
    
    // TODO: Backprop through transformer layers
    // For now, we only backprop through the output projection
}

/**
 * Non-threaded backward pass (stub that calls threaded version)
 */
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, double* gradient_buffer) {
    // TODO: This should create a temporary ThreadLocalTrainingContext
    // For now, this is a stub
    (void)training;
    (void)target_tokens;
    (void)gradient_buffer;
}

void cllm_optimizer_step_adam(CLLMTraining* training) {
    if (!training) return;
    
    CLLMModel* model = training->model;
    
    // Update time step
    model->optimizer.t++;
    
    // Bias correction
    double beta1_t = math_pow(model->optimizer.beta1, model->optimizer.t);
    double beta2_t = math_pow(model->optimizer.beta2, model->optimizer.t);
    double lr_t = model->optimizer.learning_rate * math_sqrt(1.0 - beta2_t) / (1.0 - beta1_t);
    
    // Update parameters (simplified - just embeddings for now)
    // TODO: Update all parameters with proper gradients
    (void)lr_t;  // Suppress unused warning - will be used in full implementation
    
    printf("Adam optimizer step (simplified implementation)\n");
}
