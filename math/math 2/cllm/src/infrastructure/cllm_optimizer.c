/**
 * @file infrastructure/cllm_optimizer.c
 * @brief Low-Level Optimizer Infrastructure
 * 
 * This is the LOW-LEVEL INFRASTRUCTURE for optimization.
 * Implements various optimization algorithms with advanced features.
 * All mathematical operations use crystalline library (prime_* functions).
 * 
 * RELATIONSHIP TO OTHER FILES:
 * - This file: Low-level optimizer infrastructure (advanced algorithms)
 * - src/ai/cllm_optimizer.c: High-level simple wrapper for training
 * 
 * USE THIS WHEN:
 * - Advanced optimization algorithms (Adam, RMSprop, etc.)
 * - Distributed optimization across threads
 * - Complex gradient accumulation
 * - Custom optimizer state management
 * - Performance-critical optimization
 * 
 * USE src/ai/cllm_optimizer.c WHEN:
 * - Simple training loop optimization
 * - Basic Adam optimizer
 * - Don't need advanced features
 */

#include "ai/cllm_optimizer.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static float safe_sqrt(float x) {
    return math_sqrt(x > 0.0f ? x : 0.0f);
}

static float safe_pow(float base, float exp) {
    return math_pow(base, exp);
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

OptimizerState* optimizer_create(
    const OptimizerConfig* config,
    float* parameters,
    size_t num_parameters
) {
    if (!config || !parameters || num_parameters == 0) {
        fprintf(stderr, "Error: Invalid optimizer configuration\n");
        return NULL;
    }
    
    // Validate configuration
    if (!optimizer_validate_config(config)) {
        fprintf(stderr, "Error: Invalid optimizer configuration\n");
        return NULL;
    }
    
    // Allocate state
    OptimizerState* state = (OptimizerState*)calloc(1, sizeof(OptimizerState));
    if (!state) {
        fprintf(stderr, "Error: Failed to allocate optimizer state\n");
        return NULL;
    }
    
    // Copy configuration
    state->config = *config;
    
    // Set parameters
    state->parameters = parameters;
    state->num_parameters = num_parameters;
    
    // Initialize state
    state->step = 0;
    state->current_lr = config->learning_rate;
    state->gradient_norm = 0.0f;
    state->parameter_norm = 0.0f;
    state->updates_applied = 0;
    
    // Allocate gradient buffer
    state->gradients = (float*)calloc(num_parameters, sizeof(float));
    if (!state->gradients) {
        fprintf(stderr, "Error: Failed to allocate gradient buffer\n");
        free(state);
        return NULL;
    }
    
    // Allocate optimizer-specific buffers
    bool needs_momentum = (config->type == OPTIMIZER_SGD_MOMENTUM ||
                          config->type == OPTIMIZER_SGD_NESTEROV ||
                          config->type == OPTIMIZER_ADAM ||
                          config->type == OPTIMIZER_ADAMW);
    
    bool needs_variance = (config->type == OPTIMIZER_ADAM ||
                          config->type == OPTIMIZER_ADAMW ||
                          config->type == OPTIMIZER_RMSPROP ||
                          config->type == OPTIMIZER_ADAGRAD);
    
    if (needs_momentum) {
        state->momentum_buffer = (float*)calloc(num_parameters, sizeof(float));
        if (!state->momentum_buffer) {
            fprintf(stderr, "Error: Failed to allocate momentum buffer\n");
            free(state->gradients);
            free(state);
            return NULL;
        }
    }
    
    if (needs_variance) {
        state->variance_buffer = (float*)calloc(num_parameters, sizeof(float));
        if (!state->variance_buffer) {
            fprintf(stderr, "Error: Failed to allocate variance buffer\n");
            free(state->momentum_buffer);
            free(state->gradients);
            free(state);
            return NULL;
        }
    }
    
    if (config->amsgrad) {
        state->max_variance_buffer = (float*)calloc(num_parameters, sizeof(float));
        if (!state->max_variance_buffer) {
            fprintf(stderr, "Error: Failed to allocate max variance buffer\n");
            free(state->variance_buffer);
            free(state->momentum_buffer);
            free(state->gradients);
            free(state);
            return NULL;
        }
    }
    
    // Initialize mutex
    pthread_mutex_init(&state->mutex, NULL);
    
    return state;
}

void optimizer_free(OptimizerState* state) {
    if (!state) return;
    
    pthread_mutex_destroy(&state->mutex);
    
    if (state->gradients) free(state->gradients);
    if (state->momentum_buffer) free(state->momentum_buffer);
    if (state->variance_buffer) free(state->variance_buffer);
    if (state->max_variance_buffer) free(state->max_variance_buffer);
    
    free(state);
}

void optimizer_reset(OptimizerState* state) {
    if (!state) return;
    
    pthread_mutex_lock(&state->mutex);
    
    state->step = 0;
    state->current_lr = state->config.learning_rate;
    state->gradient_norm = 0.0f;
    state->parameter_norm = 0.0f;
    state->updates_applied = 0;
    
    // Zero buffers
    if (state->gradients) {
        memset(state->gradients, 0, state->num_parameters * sizeof(float));
    }
    if (state->momentum_buffer) {
        memset(state->momentum_buffer, 0, state->num_parameters * sizeof(float));
    }
    if (state->variance_buffer) {
        memset(state->variance_buffer, 0, state->num_parameters * sizeof(float));
    }
    if (state->max_variance_buffer) {
        memset(state->max_variance_buffer, 0, state->num_parameters * sizeof(float));
    }
    
    pthread_mutex_unlock(&state->mutex);
}

// ============================================================================
// OPTIMIZATION STEP
// ============================================================================

int optimizer_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients) {
        return -1;
    }
    
    pthread_mutex_lock(&state->mutex);
    
    // Copy gradients
    memcpy(state->gradients, gradients, state->num_parameters * sizeof(float));
    
    // Update learning rate BEFORE the step (so it's used in this step)
    state->current_lr = optimizer_update_learning_rate(state);
    
    // Apply gradient clipping if enabled
    if (state->config.use_gradient_clipping) {
        if (state->config.clip_value > 0.0f) {
            optimizer_clip_gradients_by_value(
                state->gradients,
            state->num_parameters,
                state->config.clip_value
            );
        }
        if (state->config.clip_norm > 0.0f) {
            state->gradient_norm = optimizer_clip_gradients_by_norm(
                state->gradients,
                state->num_parameters,
                state->config.clip_norm
            );
        }
    } else {
        state->gradient_norm = optimizer_compute_gradient_norm(
            state->gradients,
            state->num_parameters
        );
    }
    
    // Perform optimizer-specific step
    int result = 0;
    switch (state->config.type) {
        case OPTIMIZER_SGD:
            result = optimizer_sgd_step(state, state->gradients);
            break;
        case OPTIMIZER_SGD_MOMENTUM:
            result = optimizer_sgd_momentum_step(state, state->gradients);
            break;
        case OPTIMIZER_SGD_NESTEROV:
            result = optimizer_sgd_nesterov_step(state, state->gradients);
            break;
        case OPTIMIZER_ADAM:
            result = optimizer_adam_step(state, state->gradients);
            break;
        case OPTIMIZER_ADAMW:
            result = optimizer_adamw_step(state, state->gradients);
            break;
        case OPTIMIZER_RMSPROP:
            result = optimizer_rmsprop_step(state, state->gradients);
            break;
        case OPTIMIZER_ADAGRAD:
            result = optimizer_adagrad_step(state, state->gradients);
            break;
        default:
            fprintf(stderr, "Error: Unknown optimizer type\n");
            result = -1;
    }
    
    if (result == 0) {
        state->step++;
        state->updates_applied++;
        state->parameter_norm = optimizer_compute_parameter_norm(
            state->parameters,
            state->num_parameters
        );
    }
    
    pthread_mutex_unlock(&state->mutex);
    
    return result;
}

void optimizer_zero_grad(OptimizerState* state) {
    if (!state || !state->gradients) return;
    
    memset(state->gradients, 0, state->num_parameters * sizeof(float));
}

// ============================================================================
// SPECIFIC OPTIMIZERS
// ============================================================================

int optimizer_sgd_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients) return -1;
    
    float lr = state->current_lr;
    float wd = state->config.weight_decay;
    
    for (size_t i = 0; i < state->num_parameters; i++) {
        float grad = gradients[i];
        
        // Add weight decay
        if (wd > 0.0f) {
            grad += wd * state->parameters[i];
        }
        
        // Update parameter
        state->parameters[i] -= lr * grad;
    }
    
    return 0;
}

int optimizer_sgd_momentum_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients || !state->momentum_buffer) return -1;
    
    float lr = state->current_lr;
    float momentum = state->config.momentum;
    float wd = state->config.weight_decay;
    
    for (size_t i = 0; i < state->num_parameters; i++) {
        float grad = gradients[i];
        
        // Add weight decay
        if (wd > 0.0f) {
            grad += wd * state->parameters[i];
        }
        
        // Update momentum: v = momentum * v + grad
        state->momentum_buffer[i] = momentum * state->momentum_buffer[i] + grad;
        
        // Update parameter: w = w - lr * v
        state->parameters[i] -= lr * state->momentum_buffer[i];
    }
    
    return 0;
}

int optimizer_sgd_nesterov_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients || !state->momentum_buffer) return -1;
    
    float lr = state->current_lr;
    float momentum = state->config.momentum;
    float wd = state->config.weight_decay;
    
    for (size_t i = 0; i < state->num_parameters; i++) {
        float grad = gradients[i];
        
        // Add weight decay
        if (wd > 0.0f) {
            grad += wd * state->parameters[i];
        }
        
        // Update momentum: v = momentum * v + grad
        state->momentum_buffer[i] = momentum * state->momentum_buffer[i] + grad;
        
        // Nesterov update: w = w - lr * (momentum * v + grad)
        state->parameters[i] -= lr * (momentum * state->momentum_buffer[i] + grad);
    }
    
    return 0;
}

int optimizer_adam_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients || !state->momentum_buffer || !state->variance_buffer) {
        return -1;
    }
    
    float lr = state->current_lr;
    float beta1 = state->config.beta1;
    float beta2 = state->config.beta2;
    float epsilon = state->config.epsilon;
    float wd = state->config.weight_decay;
    int t = state->step + 1;
    
    // Bias correction terms
    float bias_correction1 = 1.0f - safe_pow(beta1, (float)t);
    float bias_correction2 = 1.0f - safe_pow(beta2, (float)t);
    
    for (size_t i = 0; i < state->num_parameters; i++) {
        float grad = gradients[i];
        
        // Add weight decay (coupled)
        if (wd > 0.0f && !state->config.decoupled_weight_decay) {
            grad += wd * state->parameters[i];
        }
        
        // Update biased first moment: m = beta1 * m + (1 - beta1) * g
        state->momentum_buffer[i] = beta1 * state->momentum_buffer[i] + 
                                   (1.0f - beta1) * grad;
        
        // Update biased second moment: v = beta2 * v + (1 - beta2) * g^2
        state->variance_buffer[i] = beta2 * state->variance_buffer[i] + 
                                   (1.0f - beta2) * grad * grad;
        
        // Compute bias-corrected moments
        float m_hat = state->momentum_buffer[i] / bias_correction1;
        float v_hat = state->variance_buffer[i] / bias_correction2;
        
        // AMSGrad variant
        if (state->config.amsgrad && state->max_variance_buffer) {
            if (v_hat > state->max_variance_buffer[i]) {
                state->max_variance_buffer[i] = v_hat;
            }
            v_hat = state->max_variance_buffer[i];
        }
        
        // Update parameter: w = w - lr * m_hat / (math_sqrt(v_hat) + epsilon)
        state->parameters[i] -= lr * m_hat / (safe_sqrt(v_hat) + epsilon);
    }
    
    return 0;
}

int optimizer_adamw_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients || !state->momentum_buffer || !state->variance_buffer) {
        return -1;
    }
    
    float lr = state->current_lr;
    float beta1 = state->config.beta1;
    float beta2 = state->config.beta2;
    float epsilon = state->config.epsilon;
    float wd = state->config.weight_decay;
    int t = state->step + 1;
    
    // Bias correction terms
    float bias_correction1 = 1.0f - safe_pow(beta1, (float)t);
    float bias_correction2 = 1.0f - safe_pow(beta2, (float)t);
    
    for (size_t i = 0; i < state->num_parameters; i++) {
        float grad = gradients[i];
        
        // Update biased first moment: m = beta1 * m + (1 - beta1) * g
        state->momentum_buffer[i] = beta1 * state->momentum_buffer[i] + 
                                   (1.0f - beta1) * grad;
        
        // Update biased second moment: v = beta2 * v + (1 - beta2) * g^2
        state->variance_buffer[i] = beta2 * state->variance_buffer[i] + 
                                   (1.0f - beta2) * grad * grad;
        
        // Compute bias-corrected moments
        float m_hat = state->momentum_buffer[i] / bias_correction1;
        float v_hat = state->variance_buffer[i] / bias_correction2;
        
        // AMSGrad variant
        if (state->config.amsgrad && state->max_variance_buffer) {
            if (v_hat > state->max_variance_buffer[i]) {
                state->max_variance_buffer[i] = v_hat;
            }
            v_hat = state->max_variance_buffer[i];
        }
        
        // AdamW: Decoupled weight decay
        // w = w - lr * (m_hat / (math_sqrt(v_hat) + epsilon) + wd * w)
        float update = m_hat / (safe_sqrt(v_hat) + epsilon);
        if (wd > 0.0f) {
            update += wd * state->parameters[i];
        }
        
        state->parameters[i] -= lr * update;
    }
    
    return 0;
}

int optimizer_rmsprop_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients || !state->variance_buffer) return -1;
    
    float lr = state->current_lr;
    float beta2 = state->config.beta2;
    float epsilon = state->config.epsilon;
    float wd = state->config.weight_decay;
    
    for (size_t i = 0; i < state->num_parameters; i++) {
        float grad = gradients[i];
        
        // Add weight decay
        if (wd > 0.0f) {
            grad += wd * state->parameters[i];
        }
        
        // Update variance: v = beta2 * v + (1 - beta2) * g^2
        state->variance_buffer[i] = beta2 * state->variance_buffer[i] + 
                                   (1.0f - beta2) * grad * grad;
        
        // Update parameter: w = w - lr * g / (math_sqrt(v) + epsilon)
        state->parameters[i] -= lr * grad / (safe_sqrt(state->variance_buffer[i]) + epsilon);
    }
    
    return 0;
}

int optimizer_adagrad_step(OptimizerState* state, const float* gradients) {
    if (!state || !gradients || !state->variance_buffer) return -1;
    
    float lr = state->current_lr;
    float epsilon = state->config.epsilon;
    float wd = state->config.weight_decay;
    
    for (size_t i = 0; i < state->num_parameters; i++) {
        float grad = gradients[i];
        
        // Add weight decay
        if (wd > 0.0f) {
            grad += wd * state->parameters[i];
        }
        
        // Accumulate squared gradients: v = v + g^2
        state->variance_buffer[i] += grad * grad;
        
        // Update parameter: w = w - lr * g / (math_sqrt(v) + epsilon)
        state->parameters[i] -= lr * grad / (safe_sqrt(state->variance_buffer[i]) + epsilon);
    }
    
    return 0;
}

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================

void optimizer_clip_gradients_by_value(
    float* gradients,
    size_t num_parameters,
    float clip_value
) {
    if (!gradients || clip_value <= 0.0f) return;
    
    for (size_t i = 0; i < num_parameters; i++) {
        if (gradients[i] > clip_value) {
            gradients[i] = clip_value;
        } else if (gradients[i] < -clip_value) {
            gradients[i] = -clip_value;
        }
    }
}

float optimizer_clip_gradients_by_norm(
    float* gradients,
    size_t num_parameters,
    float max_norm
) {
    if (!gradients || max_norm <= 0.0f) return 0.0f;
    
    // Compute gradient norm
    float norm = optimizer_compute_gradient_norm(gradients, num_parameters);
    
    // Clip if necessary
    if (norm > max_norm) {
        float scale = max_norm / norm;
        for (size_t i = 0; i < num_parameters; i++) {
            gradients[i] *= scale;
        }
    }
    
    return norm;
}

float optimizer_compute_gradient_norm(
    const float* gradients,
    size_t num_parameters
) {
    if (!gradients) return 0.0f;
    
    float sum = 0.0f;
    for (size_t i = 0; i < num_parameters; i++) {
        sum += gradients[i] * gradients[i];
    }
    
    return safe_sqrt(sum);
}

float optimizer_compute_parameter_norm(
    const float* parameters,
    size_t num_parameters
) {
    if (!parameters) return 0.0f;
    
    float sum = 0.0f;
    for (size_t i = 0; i < num_parameters; i++) {
        sum += parameters[i] * parameters[i];
    }
    
    return safe_sqrt(sum);
}

// ============================================================================
// LEARNING RATE SCHEDULING
// ============================================================================

float optimizer_update_learning_rate(OptimizerState* state) {
    if (!state) return 0.0f;
    
    // Use step + 1 because we calculate LR before incrementing step
    int step = state->step + 1;
    OptimizerConfig* config = &state->config;
    
    float lr = config->learning_rate;
    
    // Apply warmup if needed
    if (step < config->warmup_steps) {
        lr = optimizer_warmup_lr(step, config->warmup_steps, config->learning_rate);
    } else {
        // Apply scheduler after warmup
        int adjusted_step = step - config->warmup_steps;
        int adjusted_total = config->total_steps - config->warmup_steps;
        
        switch (config->scheduler) {
            case LR_SCHEDULER_NONE:
                lr = config->learning_rate;
                break;
            case LR_SCHEDULER_WARMUP:
                // Already handled above
                lr = config->learning_rate;
                break;
            case LR_SCHEDULER_LINEAR:
                lr = optimizer_linear_decay_lr(
                    adjusted_step, adjusted_total,
                    config->learning_rate, config->min_lr
                );
                break;
            case LR_SCHEDULER_COSINE:
                lr = optimizer_cosine_annealing_lr(
                    adjusted_step, adjusted_total,
                    config->learning_rate, config->min_lr
                );
                break;
            case LR_SCHEDULER_STEP:
                lr = optimizer_step_decay_lr(
                    step, config->decay_steps,
                    config->learning_rate, config->decay_rate, config->min_lr
                );
                break;
            case LR_SCHEDULER_EXPONENTIAL:
                lr = optimizer_exponential_decay_lr(
                    step, config->decay_steps,
                    config->learning_rate, config->decay_rate, config->min_lr
                );
                break;
            case LR_SCHEDULER_CYCLIC:
                lr = optimizer_cyclic_lr(
                    step, config->cycle_steps,
                    config->min_lr, config->max_lr
                );
                break;
            default:
                lr = config->learning_rate;
        }
    }
    
    return lr;
}

float optimizer_get_learning_rate(const OptimizerState* state) {
    return state ? state->current_lr : 0.0f;
}

void optimizer_set_learning_rate(OptimizerState* state, float learning_rate) {
    if (state) {
        state->current_lr = learning_rate;
    }
}

float optimizer_warmup_lr(int step, int warmup_steps, float initial_lr) {
    if (warmup_steps <= 0) return initial_lr;
    if (step >= warmup_steps) return initial_lr;
    
    return initial_lr * ((float)step / (float)warmup_steps);
}

float optimizer_linear_decay_lr(
    int step,
    int total_steps,
    float initial_lr,
    float min_lr
) {
    if (total_steps <= 0) return initial_lr;
    if (step >= total_steps) return min_lr;
    
    float progress = (float)step / (float)total_steps;
    return initial_lr - (initial_lr - min_lr) * progress;
}

float optimizer_cosine_annealing_lr(
    int step,
    int total_steps,
    float initial_lr,
    float min_lr
) {
    if (total_steps <= 0) return initial_lr;
    if (step >= total_steps) return min_lr;
    
    float progress = (float)step / (float)total_steps;
    float cosine_decay = 0.5f * (1.0f + math_cos(3.14159265359f * progress));
    
    return min_lr + (initial_lr - min_lr) * cosine_decay;
}

float optimizer_step_decay_lr(
    int step,
    int decay_steps,
    float initial_lr,
    float decay_rate,
    float min_lr
) {
    if (decay_steps <= 0) return initial_lr;
    
    int num_decays = step / decay_steps;
    float lr = initial_lr;
    for (int i = 0; i < num_decays; i++) {
        lr *= decay_rate;
    }
    
    return lr > min_lr ? lr : min_lr;
}

float optimizer_exponential_decay_lr(
    int step,
    int decay_steps,
    float initial_lr,
    float decay_rate,
    float min_lr
) {
    if (decay_steps <= 0) return initial_lr;
    
    float exponent = (float)step / (float)decay_steps;
    float lr = initial_lr * safe_pow(decay_rate, exponent);
    
    return lr > min_lr ? lr : min_lr;
}

float optimizer_cyclic_lr(
    int step,
    int cycle_steps,
    float min_lr,
    float max_lr
) {
    if (cycle_steps <= 0) return min_lr;
    
    int cycle_position = step % cycle_steps;
    float progress = (float)cycle_position / (float)cycle_steps;
    
    // Triangle wave
    float amplitude = max_lr - min_lr;
    if (progress < 0.5f) {
        return min_lr + amplitude * (2.0f * progress);
    } else {
        return max_lr - amplitude * (2.0f * (progress - 0.5f));
    }
}

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

void optimizer_get_statistics(
    const OptimizerState* state,
    float* out_gradient_norm,
    float* out_parameter_norm,
    float* out_learning_rate,
    int* out_step
) {
    if (!state) return;
    
    if (out_gradient_norm) *out_gradient_norm = state->gradient_norm;
    if (out_parameter_norm) *out_parameter_norm = state->parameter_norm;
    if (out_learning_rate) *out_learning_rate = state->current_lr;
    if (out_step) *out_step = state->step;
}

void optimizer_print_state(const OptimizerState* state) {
    if (!state) return;
    
    printf("\n=== OPTIMIZER STATE ===\n");
    printf("Type: ");
    switch (state->config.type) {
        case OPTIMIZER_SGD: printf("SGD\n"); break;
        case OPTIMIZER_SGD_MOMENTUM: printf("SGD with Momentum\n"); break;
        case OPTIMIZER_SGD_NESTEROV: printf("SGD with Nesterov Momentum\n"); break;
        case OPTIMIZER_ADAM: printf("Adam\n"); break;
        case OPTIMIZER_ADAMW: printf("AdamW\n"); break;
        case OPTIMIZER_RMSPROP: printf("RMSProp\n"); break;
        case OPTIMIZER_ADAGRAD: printf("Adagrad\n"); break;
    }
    
    printf("Step: %d\n", state->step);
    printf("Learning Rate: %.6f\n", state->current_lr);
    printf("Gradient Norm: %.6f\n", state->gradient_norm);
    printf("Parameter Norm: %.6f\n", state->parameter_norm);
    printf("Updates Applied: %lu\n", (unsigned long)state->updates_applied);
    printf("======================\n\n");
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

OptimizerConfig optimizer_default_config(OptimizerType type) {
    OptimizerConfig config = {0};
    
    config.type = type;
    config.learning_rate = 0.001f;
    config.min_lr = 1e-6f;
    config.max_lr = 0.01f;
    
    // SGD defaults
    config.momentum = 0.9f;
    config.use_nesterov = false;
    
    // Adam defaults
    config.beta1 = 0.9f;
    config.beta2 = 0.999f;
    config.epsilon = 1e-8f;
    config.amsgrad = false;
    
    // Weight decay
    config.weight_decay = 0.0f;
    config.decoupled_weight_decay = (type == OPTIMIZER_ADAMW);
    
    // Gradient clipping
    config.use_gradient_clipping = false;
    config.clip_value = 0.0f;
    config.clip_norm = 0.0f;
    
    // Learning rate scheduling
    config.scheduler = LR_SCHEDULER_NONE;
    config.warmup_steps = 0;
    config.total_steps = 10000;
    config.decay_rate = 0.1f;
    config.decay_steps = 1000;
    config.cycle_steps = 1000;
    
    return config;
}

bool optimizer_validate_config(const OptimizerConfig* config) {
    if (!config) return false;
    
    // Check learning rates
    if (config->learning_rate <= 0.0f) {
        fprintf(stderr, "Error: learning_rate must be > 0\n");
        return false;
    }
    if (config->min_lr < 0.0f) {
        fprintf(stderr, "Error: min_lr must be >= 0\n");
        return false;
    }
    if (config->max_lr < config->min_lr) {
        fprintf(stderr, "Error: max_lr must be >= min_lr\n");
        return false;
    }
    
    // Check momentum
    if (config->momentum < 0.0f || config->momentum >= 1.0f) {
        fprintf(stderr, "Error: momentum must be in [0, 1)\n");
        return false;
    }
    
    // Check betas
    if (config->beta1 < 0.0f || config->beta1 >= 1.0f) {
        fprintf(stderr, "Error: beta1 must be in [0, 1)\n");
        return false;
    }
    if (config->beta2 < 0.0f || config->beta2 >= 1.0f) {
        fprintf(stderr, "Error: beta2 must be in [0, 1)\n");
        return false;
    }
    
    // Check epsilon
    if (config->epsilon <= 0.0f) {
        fprintf(stderr, "Error: epsilon must be > 0\n");
        return false;
    }
    
    // Check weight decay
    if (config->weight_decay < 0.0f) {
        fprintf(stderr, "Error: weight_decay must be >= 0\n");
        return false;
    }
    
    // Check gradient clipping
    if (config->use_gradient_clipping) {
        if (config->clip_value < 0.0f) {
            fprintf(stderr, "Error: clip_value must be >= 0\n");
            return false;
        }
        if (config->clip_norm < 0.0f) {
            fprintf(stderr, "Error: clip_norm must be >= 0\n");
            return false;
        }
    }
    
    // Check scheduling
    if (config->warmup_steps < 0) {
        fprintf(stderr, "Error: warmup_steps must be >= 0\n");
        return false;
    }
    if (config->total_steps <= 0) {
        fprintf(stderr, "Error: total_steps must be > 0\n");
        return false;
    }
    
    return true;
}