#ifndef CLLM_TRAINING_H
#define CLLM_TRAINING_H

/*
 * cllm_training.h - Auto-generated header file
 * Source: cllm_training.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include "math/types.h"

/* Local includes */
#include "cllm.h"

/* Algorithm layer includes (WIRED) */
#include "loss_functions.h"
#include "optimizers.h"
#include "backprop.h"

/* Type definitions */

/*
 * CLLM Training Configuration
 */
typedef struct {
    double learning_rate;         // Learning rate
    int batch_size;              // Batch size
    int num_epochs;              // Number of training epochs
    int max_steps;               // Maximum training steps
    double weight_decay;          // Weight decay (L2 regularization)
    double gradient_clip;         // Gradient clipping threshold
    int warmup_steps;            // Learning rate warmup steps
    int save_interval;           // Save checkpoint every N steps
    int save_every;              // Alternative save interval
    int eval_interval;           // Evaluate every N steps
    int sequence_length;         // Sequence length for training
    char optimizer[32];          // Optimizer name (e.g., "adam", "sgd")
    
    // Learning rate scheduling
    char lr_scheduler[32];       // Scheduler type: "none", "linear", "cosine", "step"
    double lr_decay_factor;       // Decay factor for step scheduler (default: 0.1)
    int lr_decay_steps;          // Steps between decay for step scheduler (default: 1000)
    double min_lr;                // Minimum learning rate (default: 1e-6)
    double initial_learning_rate; // Original learning rate (preserved for scheduling)
    
    // Gradient accumulation
    int gradient_accumulation_steps;  // Number of steps to accumulate gradients (default: 1)
    
    // Mixed precision training
    int use_mixed_precision;          // Enable FP16/FP32 mixed precision (default: 0)
    double loss_scale;                 // Loss scaling factor for FP16 (default: 1024.0)
    double loss_scale_growth;          // Growth factor for dynamic loss scaling (default: 2.0)
    double loss_scale_backoff;         // Backoff factor for dynamic loss scaling (default: 0.5)
    int loss_scale_window;            // Steps before increasing loss scale (default: 2000)
    
} CLLMTrainingConfig;

/*
 * CLLM Training State
 */
typedef struct {
    CLLMModel* model;            // Pointer to the model
    CLLMTrainingConfig config;   // Training configuration
    
    // Training data
    uint32_t* tokens;            // Training tokens
    size_t num_tokens;           // Number of tokens
    
    // Training state
    int current_epoch;           // Current epoch
    int current_step;            // Current training step
    double best_loss;             // Best validation loss
    double current_loss;          // Current training loss
    time_t start_time;           // Training start time
    
    // Gradient accumulation state
    int accumulation_step;       // Current accumulation step (0 to gradient_accumulation_steps-1)
    
    // Mixed precision training state
    double* master_weights;       // FP32 master copy of weights (for mixed precision)
    uint16_t* fp16_activations;  // FP16 activation buffer
    uint16_t* fp16_gradients;    // FP16 gradient buffer
    double current_loss_scale;    // Current dynamic loss scale
    int loss_scale_steps;        // Steps since last loss scale increase
    
    // Batch management
    int total_batches;           // Total number of batches
    int current_batch_offset;    // Current batch offset in tokens
    
    // ========================================================================
    // 88D THREAD-CENTRIC TRAINING (MANDATORY)
    // ========================================================================
    
    // Current Batch Distribution
    // Each batch is distributed across threads based on token assignments
    struct {
        uint32_t* token_ids;           // Tokens in this batch [batch_size * seq_len]
        uint32_t* target_ids;          // Target tokens [batch_size * seq_len]
        uint8_t* assigned_layers;      // Which layer processes each token
        uint8_t* assigned_dimensions;  // Which dimension processes each token
        uint32_t batch_size;           // Current batch size
        uint32_t seq_len;              // Current sequence length
    } current_batch;
    
    // Thread Statistics (per-thread tracking)
    struct {
        uint64_t tokens_processed;     // Total tokens processed by this thread
        uint64_t gradients_computed;   // Total gradients computed
        double avg_loss;               // Average loss for this thread
        uint64_t forward_time_ns;      // Time spent in forward pass
        uint64_t backward_time_ns;     // Time spent in backward pass
    } *thread_stats;                   // [96 threads]
    
    // LEGACY REMOVED: All buffers now thread-local
    // DELETED: double* gradients;
    // DELETED: double* optimizer_state;
    // DELETED: struct { ... }* attention_grads;
    // DELETED: struct { ... }* ff_grads;
    // DELETED: struct { ... }* ln_grads;
    // DELETED: double* backward_embeddings;
    // DELETED: double* backward_grad_output;
    // DELETED: double* backward_layer_input;
    // DELETED: double* backward_layer_grad;
    // DELETED: double* backward_temp_grad;
    // DELETED: double* cached_input_embeddings;
    // DELETED: double* cached_target_embeddings;
    // DELETED: double* input_embeddings;
    // DELETED: double** layer_inputs;
    // DELETED: double** attention_outputs;
    // DELETED: double** ff_outputs;
    // DELETED: double** layer_outputs;
    // DELETED: double** ff_hidden;
    // DELETED: double* final_hidden;
    // DELETED: double* logits;
    // DELETED: struct { ... }* attention_cache;
    // All activations/gradients now stored in thread-local buffers
    
    // Algorithm layer integration (WIRED)
    // LossConfig loss_config;  // TODO: Define LossConfig type or remove          // Loss function configuration from algorithms layer
    OptimizerState* optimizer_state_alg;  // Optimizer state from algorithms layer
    GradientBuffer* gradient_buffer; // Gradient buffer from algorithms layer
} CLLMTraining;

/* Loss computation functions */
// OBJECTIVE 2B: Only crystalline loss remains - standard loss functions removed
// PRECISION FIX: Changed from double to double for consistency
double cllm_compute_loss(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens, int num_tokens);
double cllm_compute_accuracy(double* logits, uint32_t* targets, int batch_size, int vocab_size);
double cllm_compute_top_k_accuracy(double* logits, uint32_t* targets, int batch_size, int vocab_size, int k);

/* Optimizer functions */
void cllm_apply_gradient_clipping(double* gradients, size_t size, double max_norm);
void cllm_clip_gradients_by_value(double* gradients, size_t size, double clip_value);
/* Training initialization and cleanup */
CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config);
void cllm_training_free(CLLMTraining* training);
void cllm_training_cleanup(CLLMTraining* training);

void cllm_adam_step(CLLMTraining* training, double learning_rate);
void cllm_sgd_momentum_step(CLLMTraining* training, double learning_rate, double momentum);
void cllm_update_learning_rate(CLLMTraining* training);
double cllm_get_learning_rate(CLLMTraining* training);
double cllm_get_learning_rate_step_decay(CLLMTraining* training, int* decay_steps, int num_decay_steps, double decay_factor);
double cllm_get_learning_rate_exponential(CLLMTraining* training, double decay_rate);
double cllm_get_learning_rate_polynomial(CLLMTraining* training, double power);
void cllm_print_lr_schedule(CLLMTraining* training);
int cllm_adam_init(CLLMTraining* training);
void cllm_apply_weight_decay(double* weights, size_t size, double weight_decay, double learning_rate);
void cllm_zero_gradients(double* gradients, size_t size);
double cllm_compute_gradient_norm(double* gradients, size_t size);
void cllm_accumulate_gradients(double* accumulated_grads, double* current_grads, size_t size);
void cllm_scale_gradients(double* gradients, size_t size, double scale);
int cllm_check_gradients_valid(double* gradients, size_t size);
void cllm_update_ema_weights(double* ema_weights, double* current_weights, size_t size, double decay);

/* Backward pass functions */
void cllm_layer_norm_backward(CLLMLayerNorm* ln, double* input, double* grad_output, double* grad_input, double* grad_gamma, double* grad_beta);
// Note: FeedForwardLayer, AttentionLayer, Embeddings are legacy types - using CLLMModel directly now
// void cllm_feedforward_backward(FeedForwardLayer* layer, double* input, double* hidden, double* grad_output, double* grad_input, double* grad_w1, double* grad_w2, double* grad_b1, double* grad_b2);
// void cllm_attention_backward(AttentionLayer* layer, double* input, double* grad_output, double* grad_input,
//                             double* grad_query_weights, double* grad_key_weights, double* grad_value_weights,
//                             int seq_len);
// void cllm_embedding_backward(Embeddings* embeddings, uint32_t* token_ids, double* grad_output, double* grad_embeddings, int batch_size);
void cllm_transformer_layer_backward(CLLMTraining* training, int layer_idx, double* input, double* grad_output, double* grad_input, int seq_len);
void cllm_backward(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens, int num_tokens);

/* Function declarations */
int cllm_load_training_data(CLLMTraining* training, const char* filename);
int cllm_get_batch(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens);
void cllm_optimizer_step(CLLMTraining* training);
void cllm_optimizer_step_adam(CLLMTraining* training);  // Adam optimizer with gradient accumulation
// PRECISION FIX: Changed from double to double for consistency
double cllm_train_epoch(CLLMTraining* training);
int cllm_train_legacy(CLLMTraining* training);
int cllm_save_checkpoint(CLLMTraining* training, const char* filename);
int cllm_load_checkpoint(CLLMTraining* training, const char* filename);
void cllm_training_cleanup(CLLMTraining* training);

/* Training step functions (used by multi-threading and production features) */
// PRECISION FIX: Changed from double to double for consistency
double cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens);
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, double* gradient_buffer);

/**
 * Complete training step with forward and backward passes (DAY 11)
 * 
 * This is the main training function that coordinates:
 * 1. Forward pass (all tokens in parallel)
 * 2. Loss computation
 * 3. Backward pass (all tokens in parallel)
 * 4. Optimizer application (all threads in parallel)
 * 
 * @param training Training context
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @return Loss value, or -1.0 on error
 */
double cllm_train_step_threaded(
    CLLMTraining* training,
    const uint32_t* input_tokens,
    const uint32_t* target_tokens,
    uint32_t num_tokens
);

// Forward declare for 88D integration
struct ThreadLocalTrainingContext;

/**
 * Note: cllm_forward_training() and cllm_backward_training() are now
 * the unified 88D implementations (no _threaded suffix needed).
 * See cllm_training_functions.c for implementation.
 */

#endif /* CLLM_TRAINING_H */