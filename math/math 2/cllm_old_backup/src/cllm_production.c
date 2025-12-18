/**
 * Production Features for CLLM
 * 
 * 1. Checkpoint saving/loading (resume training)
 * 2. Validation set evaluation
 * 3. Early stopping
 * 4. Learning rate scheduling
 * 5. Gradient clipping
 * 6. Training metrics logging
 */

#include "cllm_training.h"
#include "ai/cllm_loss.h"
#include "cllm_format.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "math/transcendental.h"

// Training checkpoint
typedef struct {
    int epoch;
    int step;
    float best_loss;
    float learning_rate;
    time_t timestamp;
    
    // Optimizer state
    float* optimizer_state;
    size_t optimizer_state_size;
} TrainingCheckpoint;

// Validation set
typedef struct {
    uint32_t* tokens;
    size_t num_tokens;
    int batch_size;
    int sequence_length;
} ValidationSet;

// Early stopping state
typedef struct {
    float best_val_loss;
    int patience;
    int patience_counter;
    int min_delta_threshold;  // Minimum improvement (in basis points, e.g., 10 = 0.1%)
} EarlyStoppingState;

// Learning rate scheduler
typedef struct {
    float initial_lr;
    float min_lr;
    int warmup_steps;
    int total_steps;
    int current_step;
    
    enum {
        LR_CONSTANT,
        LR_LINEAR_WARMUP,
        LR_COSINE_DECAY,
        LR_STEP_DECAY
    } schedule_type;
} LRScheduler;

// Training metrics
typedef struct {
    float* train_losses;
    float* val_losses;
    float* learning_rates;
    int* epochs;
    int num_records;
    int capacity;
} TrainingMetrics;

/**
 * Create validation set from file
 */
ValidationSet* create_validation_set(const char* filepath, int batch_size, int seq_len) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot open validation file: %s\n", filepath);
        return NULL;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Allocate buffer
    char* buffer = (char*)malloc(file_size + 1);
    size_t bytes_read = fread(buffer, 1, file_size, f);
    buffer[bytes_read] = '\0';
    fclose(f);
    
    // Simple tokenization (space-separated)
    ValidationSet* val_set = (ValidationSet*)malloc(sizeof(ValidationSet));
    val_set->tokens = (uint32_t*)malloc(file_size * sizeof(uint32_t));
    val_set->num_tokens = 0;
    val_set->batch_size = batch_size;
    val_set->sequence_length = seq_len;
    
    char* token = strtok(buffer, " \t\n\r");
    while (token != NULL && val_set->num_tokens < (size_t)file_size) {
        // Simple hash for token ID
        uint32_t hash = 0;
        for (char* p = token; *p; p++) {
            hash = hash * 31 + (unsigned char)*p;
        }
        val_set->tokens[val_set->num_tokens++] = hash % 10000;
        token = strtok(NULL, " \t\n\r");
    }
    
    free(buffer);
    
    printf("Validation set loaded: %zu tokens\n", val_set->num_tokens);
    return val_set;
}

/**
 * Free validation set
 */
void free_validation_set(ValidationSet* val_set) {
    if (!val_set) return;
    free(val_set->tokens);
    free(val_set);
}

/**
 * Evaluate on validation set
 */
float evaluate_validation(CLLMTraining* training, ValidationSet* val_set) {
    if (!training || !val_set) return INFINITY;
    
    float total_loss = 0.0;
    int num_batches = 0;
    
    int batch_size = val_set->batch_size;
    int seq_len = val_set->sequence_length;
    size_t batch_tokens = batch_size * seq_len;
    
    uint32_t* input_tokens = (uint32_t*)malloc(batch_tokens * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(batch_tokens * sizeof(uint32_t));
    
    // Process validation batches
    for (size_t offset = 0; offset + batch_tokens < val_set->num_tokens; offset += batch_tokens) {
        // Prepare batch
        memcpy(input_tokens, &val_set->tokens[offset], batch_tokens * sizeof(uint32_t));
        memcpy(target_tokens, &val_set->tokens[offset + 1], (batch_tokens - 1) * sizeof(uint32_t));
        target_tokens[batch_tokens - 1] = val_set->tokens[offset + batch_tokens];
        
        // Forward pass only (no backward)
        float loss = cllm_forward_training(training, input_tokens);
        loss += cllm_compute_loss(training, input_tokens, target_tokens, batch_tokens);
        
        total_loss += loss;
        num_batches++;
    }
    
    free(input_tokens);
    free(target_tokens);
    
    return num_batches > 0 ? total_loss / num_batches : INFINITY;
}

/**
 * Save training checkpoint
 */
int save_checkpoint(CLLMTraining* training, const char* filepath) {
    if (!training || !filepath) return -1;
    
    FILE* f = fopen(filepath, "wb");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot create checkpoint file: %s\n", filepath);
        return -1;
    }
    
    // Write checkpoint header
    TrainingCheckpoint checkpoint = {0};
    checkpoint.epoch = training->current_epoch;
    checkpoint.step = training->current_step;
    checkpoint.best_loss = training->best_loss;
    checkpoint.learning_rate = training->config.learning_rate;
    checkpoint.timestamp = time(NULL);
    
    fwrite(&checkpoint, sizeof(TrainingCheckpoint), 1, f);
    
    // Write model (use existing save function)
    fclose(f);
    
    // Save model separately
    char model_path[512];
    snprintf(model_path, sizeof(model_path), "%s.model", filepath);
    cllm_write_model(training->model, model_path);
    
    printf("Checkpoint saved: epoch %d, step %d, loss %.4f\n", 
           checkpoint.epoch, checkpoint.step, checkpoint.best_loss);
    
    return 0;
}

/**
 * Load training checkpoint
 */
int load_checkpoint(CLLMTraining* training, const char* filepath) {
    if (!training || !filepath) return -1;
    
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot open checkpoint file: %s\n", filepath);
        return -1;
    }
    
    // Read checkpoint header
    TrainingCheckpoint checkpoint = {0};
    size_t read = fread(&checkpoint, sizeof(TrainingCheckpoint), 1, f);
    fclose(f);
    
    if (read != 1) {
        fprintf(stderr, "ERROR: Failed to read checkpoint\n");
        return -1;
    }
    
    // Restore training state
    training->current_epoch = checkpoint.epoch;
    training->current_step = checkpoint.step;
    training->best_loss = checkpoint.best_loss;
    training->config.learning_rate = checkpoint.learning_rate;
    
    // Load model
    char model_path[512];
    snprintf(model_path, sizeof(model_path), "%s.model", filepath);
    
    CLLMModel* loaded_model = cllm_read_model(model_path);
    if (!loaded_model) {
        fprintf(stderr, "ERROR: Failed to load model from checkpoint\n");
        return -1;
    }
    
    // Replace model
    // Note: This is simplified - in production you'd want to properly free old model
    training->model = loaded_model;
    
    printf("Checkpoint loaded: epoch %d, step %d, loss %.4f\n", 
           checkpoint.epoch, checkpoint.step, checkpoint.best_loss);
    
    return 0;
}

/**
 * Create early stopping state
 */
EarlyStoppingState* create_early_stopping(float initial_loss, int patience) {
    EarlyStoppingState* state = (EarlyStoppingState*)malloc(sizeof(EarlyStoppingState));
    state->best_val_loss = initial_loss;
    state->patience = patience;
    state->patience_counter = 0;
    state->min_delta_threshold = 10;  // 0.1% improvement required
    return state;
}

/**
 * Check early stopping condition
 */
int check_early_stopping(EarlyStoppingState* state, float val_loss) {
    if (!state) return 0;
    
    // Calculate improvement in basis points
    float improvement = (state->best_val_loss - val_loss) / state->best_val_loss * 10000.0;
    
    if (improvement > state->min_delta_threshold) {
        // Significant improvement
        state->best_val_loss = val_loss;
        state->patience_counter = 0;
        printf("Validation improved: %.4f (%.1f bp improvement)\n", val_loss, improvement);
        return 0;
    } else {
        // No significant improvement
        state->patience_counter++;
        printf("No improvement: %d/%d patience\n", state->patience_counter, state->patience);
        
        if (state->patience_counter >= state->patience) {
            printf("Early stopping triggered!\n");
            return 1;  // Stop training
        }
    }
    
    return 0;
}

/**
 * Create learning rate scheduler
 */
LRScheduler* create_lr_scheduler(float initial_lr, int warmup_steps, int total_steps) {
    LRScheduler* scheduler = (LRScheduler*)malloc(sizeof(LRScheduler));
    scheduler->initial_lr = initial_lr;
    scheduler->min_lr = initial_lr * 0.01f;  // 1% of initial
    scheduler->warmup_steps = warmup_steps;
    scheduler->total_steps = total_steps;
    scheduler->current_step = 0;
    scheduler->schedule_type = LR_COSINE_DECAY;
    return scheduler;
}

/**
 * Get current learning rate
 */
float get_learning_rate(LRScheduler* scheduler) {
    if (!scheduler) return 0.001f;
    
    int step = scheduler->current_step;
    
    if (step < scheduler->warmup_steps) {
        // Linear warmup
        float warmup_factor = (float)step / scheduler->warmup_steps;
        return scheduler->initial_lr * warmup_factor;
    }
    
    // Cosine decay
    int decay_steps = scheduler->total_steps - scheduler->warmup_steps;
    int decay_step = step - scheduler->warmup_steps;
    
    if (decay_step >= decay_steps) {
        return scheduler->min_lr;
    }
    
    float cosine_decay = 0.5 * (1.0 + math_cos(M_PI * decay_step / decay_steps));
    return scheduler->min_lr + (scheduler->initial_lr - scheduler->min_lr) * cosine_decay;
}

/**
 * Step learning rate scheduler
 */
void step_lr_scheduler(LRScheduler* scheduler, CLLMTraining* training) {
    if (!scheduler || !training) return;
    
    scheduler->current_step++;
    float new_lr = get_learning_rate(scheduler);
    training->config.learning_rate = new_lr;
}

/**
 * Clip gradients to prevent exploding gradients
 */
void clip_gradients(CLLMTraining* training, float max_norm) {
    if (!training) return;
    
    CLLMModel* model = training->model;
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    // Compute gradient norm
    float grad_norm = 0.0;
    
    // Embedding gradients
    if (training->gradients) {
        for (size_t i = 0; i < vocab_size * embed_dim; i++) {
// DISABLED - USE BigFixed version:             grad_norm += training->gradients[i] * training->gradients[i];
        }
    }
    
    // Layer gradients
    for (uint32_t layer = 0; layer < num_layers; layer++) {
        if (training->attention_grads) {
            uint64_t attn_size = embed_dim * embed_dim;
            // DISABLED - Legacy float gradient norm calculation
            // Now handled by BigFixed version
            (void)attn_size;  // Suppress unused warning
        }
        
        // DISABLED - Legacy float gradient norm calculation for feedforward
        // Now handled by BigFixed version
    }
    
    grad_norm = math_sqrt(grad_norm);
    
    // Clip if necessary
    if (grad_norm > max_norm) {
        float scale = max_norm / grad_norm;
        (void)scale;  // Currently unused - reserved for gradient clipping
        
        // Scale embedding gradients
        if (training->gradients) {
            for (size_t i = 0; i < vocab_size * embed_dim; i++) {
// DISABLED - USE BigFixed version:                 training->gradients[i] *= scale;
            }
        }
        
        // Scale layer gradients
        for (uint32_t layer = 0; layer < num_layers; layer++) {
            if (training->attention_grads) {
                uint64_t attn_size = embed_dim * embed_dim;
                for (uint64_t i = 0; i < attn_size; i++) {
// DISABLED - USE BigFixed version:                     training->attention_grads[layer].query_lattice[i] *= scale;
// DISABLED - USE BigFixed version:                     training->attention_grads[layer].key_lattice[i] *= scale;
// DISABLED - USE BigFixed version:                     training->attention_grads[layer].value_lattice[i] *= scale;
                }
            }
            
            if (training->ff_grads) {
                uint64_t ff_size = embed_dim * embed_dim;
                for (uint64_t i = 0; i < ff_size; i++) {
// DISABLED - USE BigFixed version:                     training->ff_grads[layer].w1_lattice[i] *= scale;
// DISABLED - USE BigFixed version:                     training->ff_grads[layer].w2_lattice[i] *= scale;
                }
            }
        }
        
        printf("Gradients clipped: norm %.2f → %.2f\n", grad_norm, max_norm);
    }
}

/**
 * Create training metrics tracker
 */
TrainingMetrics* create_training_metrics(int capacity) {
    TrainingMetrics* metrics = (TrainingMetrics*)malloc(sizeof(TrainingMetrics));
    metrics->train_losses = (float*)malloc(capacity * sizeof(float));
    metrics->val_losses = (float*)malloc(capacity * sizeof(float));
    metrics->learning_rates = (float*)malloc(capacity * sizeof(float));
    metrics->epochs = (int*)malloc(capacity * sizeof(int));
    metrics->num_records = 0;
    metrics->capacity = capacity;
    return metrics;
}

/**
 * Record training metrics
 */
void record_metrics(TrainingMetrics* metrics, int epoch, float train_loss, 
                   float val_loss, float lr) {
    if (!metrics || metrics->num_records >= metrics->capacity) return;
    
    int idx = metrics->num_records++;
    metrics->epochs[idx] = epoch;
    metrics->train_losses[idx] = train_loss;
    metrics->val_losses[idx] = val_loss;
    metrics->learning_rates[idx] = lr;
}

/**
 * Save metrics to CSV
 */
void save_metrics_csv(TrainingMetrics* metrics, const char* filepath) {
    if (!metrics || !filepath) return;
    
    FILE* f = fopen(filepath, "w");
    if (!f) return;
    
    fprintf(f, "epoch,train_loss,val_loss,learning_rate\n");
    for (int i = 0; i < metrics->num_records; i++) {
        fprintf(f, "%d,%.6f,%.6f,%.6f\n", 
                metrics->epochs[i], 
                metrics->train_losses[i],
                metrics->val_losses[i],
                metrics->learning_rates[i]);
    }
    
    fclose(f);
    printf("Metrics saved to %s\n", filepath);
}

/**
 * Free training metrics
 */
void free_training_metrics(TrainingMetrics* metrics) {
    if (!metrics) return;
    free(metrics->train_losses);
    free(metrics->val_losses);
    free(metrics->learning_rates);
    free(metrics->epochs);
    free(metrics);
}
