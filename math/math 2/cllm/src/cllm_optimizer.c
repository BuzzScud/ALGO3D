/**
 * @file cllm_optimizer.c
 * @brief High-Level Optimizer Wrapper for Training
 * 
 * This is the SIMPLE, HIGH-LEVEL API for optimizer operations.
 * Implements Adam optimizer and gradient operations for training loops.
 * 
 * RELATIONSHIP TO OTHER FILES:
 * - This file: Simple optimizer wrapper for training
 * - infrastructure/cllm_optimizer.c: Low-level optimizer infrastructure,
 *   advanced algorithms, distributed optimization
 * 
 * USE THIS WHEN:
 * - Simple training loop optimization
 * - Basic Adam optimizer usage
 * - Standard gradient updates
 * 
 * USE infrastructure/cllm_optimizer.c WHEN:
 * - Advanced optimization algorithms
 * - Distributed optimization across threads
 * - Complex gradient accumulation strategies
 * - Custom optimizer state management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai/cllm.h"
#include "ai/cllm_training.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

/**
 * Apply gradient clipping by global norm
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @param max_norm Maximum gradient norm
 */
void cllm_apply_gradient_clipping(double* gradients, size_t size, double max_norm) {
    if (!gradients || size == 0 || max_norm <= 0.0) return;
    
    // Compute global norm
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    norm = math_sqrt(norm);
    
    // Clip if necessary
    if (norm > max_norm) {
        double scale = max_norm / norm;
        for (size_t i = 0; i < size; i++) {
            gradients[i] *= scale;
        }
    }
}

/**
 * Apply gradient clipping by value
 * 
 * @param gradients Gradient array [size]
               if (layer == 0 && i % 1000 == 0) {
                   printf("  Updating attention layer %d: %zu/%zu\n", layer, i, weight_size);
                   fflush(stdout);
               }
 * @param size Number of parameters
 * @param clip_value Maximum absolute value
 */
void cllm_clip_gradients_by_value(double* gradients, size_t size, double clip_value) {
    if (!gradients || size == 0 || clip_value <= 0.0) return;
    
    for (size_t i = 0; i < size; i++) {
        if (gradients[i] > clip_value) {
            gradients[i] = clip_value;
        } else if (gradients[i] < -clip_value) {
            gradients[i] = -clip_value;
        }
    }
}

/**
 * Adam update for a single parameter array
 * 
 * @param weights Weight array to update
 * @param gradients Gradient array
 * @param m First moment array
 * @param v Second moment array
 * @param size Number of parameters
 * @param learning_rate Learning rate
 * @param beta1 First moment decay
 * @param beta2 Second moment decay
               if (layer == 0 && i % 5000 == 0) {
                   printf("  Updating feedforward layer %d W1: %zu/%zu\n", layer, i, w1_size);
                   fflush(stdout);
               }
 * @param epsilon Small constant for numerical stability
 * @param bias_correction1 Bias correction for first moment
 * @param bias_correction2 Bias correction for second moment
 */
// Legacy float-based Adam update - unused (kept for reference)
__attribute__((unused))
static void adam_update_params(double* weights, double* gradients, double* m, double* v,
                               size_t size, double learning_rate, double beta1, double beta2,
                               double epsilon, double bias_correction1, double bias_correction2) {
    if (!weights || !gradients || !m || !v) return;
    
    for (size_t i = 0; i < size; i++) {
        // Update biased first moment estimate
        m[i] = beta1 * m[i] + (1.0 - beta1) * gradients[i];
        
        // Update biased second raw moment estimate
        v[i] = beta2 * v[i] + (1.0 - beta2) * gradients[i] * gradients[i];
        
        // Compute bias-corrected first moment estimate
        double m_hat = m[i] / bias_correction1;
        
        // Compute bias-corrected second raw moment estimate
        double v_hat = v[i] / bias_correction2;
        
        // Update parameters
        weights[i] -= learning_rate * m_hat / (math_sqrt(v_hat) + epsilon);
    }
}

/**
 * Adam optimizer step - updates all model parameters
 * 
 * Adam: Adaptive Moment Estimation
 * m_t = β₁ * m_{t-1} + (1 - β₁) * g_t
 * v_t = β₂ * v_{t-1} + (1 - β₂) * g_t²
 * m̂_t = m_t / (1 - β₁^t)
 * v̂_t = v_t / (1 - β₂^t)
 * θ_t = θ_{t-1} - α * m̂_t / (√v̂_t + ε)
 * 
 * @param training Training state
 * @param learning_rate Learning rate (α)
 */
void cllm_adam_step(CLLMTraining* training, double learning_rate) {
    if (!training || !training->model) return;
    
    CLLMModel* model = training->model;
    
    // Adam hyperparameters
    double beta1 = 0.9;
    double beta2 = 0.999;
    double epsilon = 1e-8f;
    
    // Update step count
    int t = training->current_step + 1;
    
    // Bias correction terms
    double bias_correction1 = 1.0 - math_pow(beta1, (float)t);
    double bias_correction2 = 1.0 - math_pow(beta2, (float)t);
    
    // TODO: Reimplement for 88D architecture
    // Gradients are now in thread-local storage
    fprintf(stderr, "WARNING: cllm_adam_step() not yet implemented for 88D architecture\n");
    
    (void)training;
    (void)learning_rate;
    (void)beta1;
    (void)beta2;
    (void)epsilon;
    (void)t;
    (void)bias_correction1;
    (void)bias_correction2;
    (void)model;
}

/**
 * SGD with momentum optimizer step
 * 
 * @param training Training state
 * @param learning_rate Learning rate
 * @param momentum Momentum coefficient (typically 0.9)
 */
void cllm_sgd_momentum_step(CLLMTraining* training, double learning_rate, double momentum) {
    // LEGACY FLOAT CODE - DISABLED
    // Use BigFixed optimizer functions instead
    (void)training;
    (void)learning_rate;
    (void)momentum;
    return;
}

/**
 * Update learning rate with warmup and decay
 * 
 * @param training Training state
 */
void cllm_update_learning_rate(CLLMTraining* training) {
    if (!training) return;
    
    int step = training->current_step;
    int warmup_steps = training->config.warmup_steps;
    double base_lr = training->config.initial_learning_rate;  // Use preserved initial LR
    double min_lr = training->config.min_lr > 0 ? training->config.min_lr : 1e-6f;
    
    double lr;
    
    // Linear warmup phase (applies to all schedulers)
    if (step < warmup_steps && warmup_steps > 0) {
        // Warmup from min_lr to base_lr over warmup_steps
        // At step 0: lr = min_lr + small amount
        // At step warmup_steps-1: lr = base_lr
        double warmup_progress = (float)(step + 1) / (float)warmup_steps;
        lr = min_lr + (base_lr - min_lr) * warmup_progress;
        
        // DEBUG
        // printf("[WARMUP] step=%d, progress=%.3f, lr=%.6f\n", step, warmup_progress, lr);
        
        training->config.learning_rate = lr;
        return;
    }
    
    // Determine scheduler type
    const char* scheduler = training->config.lr_scheduler;
    if (scheduler[0] == '\0' || strcmp(scheduler, "none") == 0) {
        // No scheduling - keep base learning rate
        lr = base_lr;
    } else if (strcmp(scheduler, "cosine") == 0) {
        // Cosine decay after warmup
        int decay_steps = training->config.max_steps - warmup_steps;
        int steps_since_warmup = step - warmup_steps;
        
        if (decay_steps > 0) {
            double progress = (float)steps_since_warmup / (float)decay_steps;
            if (progress > 1.0) progress = 1.0;
            lr = min_lr + (base_lr - min_lr) * 0.5 * (1.0 + math_cos(3.14159265 * progress));
        } else {
            lr = base_lr;
        }
    } else if (strcmp(scheduler, "linear") == 0) {
        // Linear decay after warmup
        int decay_steps = training->config.max_steps - warmup_steps;
        int steps_since_warmup = step - warmup_steps;
        
        if (decay_steps > 0) {
            double progress = (float)steps_since_warmup / (float)decay_steps;
            if (progress > 1.0) progress = 1.0;
            lr = base_lr - (base_lr - min_lr) * progress;
        } else {
            lr = base_lr;
        }
    } else if (strcmp(scheduler, "step") == 0) {
        // Step decay - reduce by factor every N steps
        int decay_steps = training->config.lr_decay_steps > 0 ? training->config.lr_decay_steps : 1000;
        double decay_factor = training->config.lr_decay_factor > 0 ? training->config.lr_decay_factor : 0.1;
        
        int steps_since_warmup = step - warmup_steps;
        int num_decays = steps_since_warmup / decay_steps;
        
        lr = base_lr;
        for (int i = 0; i < num_decays; i++) {
            lr *= decay_factor;
            if (lr < min_lr) {
                lr = min_lr;
                break;
            }
        }
    } else {
        // Unknown scheduler - default to cosine
        int decay_steps = training->config.max_steps - warmup_steps;
        int steps_since_warmup = step - warmup_steps;
        
        if (decay_steps > 0) {
            double progress = (float)steps_since_warmup / (float)decay_steps;
            if (progress > 1.0) progress = 1.0;
            lr = min_lr + (base_lr - min_lr) * 0.5 * (1.0 + math_cos(3.14159265 * progress));
        } else {
            lr = base_lr;
        }
    }
    
    // Ensure minimum learning rate
    if (lr < min_lr) {
        lr = min_lr;
    }
    
    training->config.learning_rate = lr;
}

/**
 * Apply weight decay (L2 regularization)
 * 
 * @param weights Weight array [size]
 * @param size Number of parameters
 * @param weight_decay Weight decay coefficient
 * @param learning_rate Learning rate
 */
void cllm_apply_weight_decay(double* weights, size_t size, 
                            double weight_decay, double learning_rate) {
    if (!weights || size == 0 || weight_decay <= 0.0) return;
    
    for (size_t i = 0; i < size; i++) {
        weights[i] *= (1.0 - learning_rate * weight_decay);
    }
}

/**
 * Zero gradients
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 */
void cllm_zero_gradients(double* gradients, size_t size) {
    if (!gradients || size == 0) return;
    memset(gradients, 0, size * sizeof(float));
}

/**
 * Compute gradient norm
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @return L2 norm of gradients
 */
double cllm_compute_gradient_norm(double* gradients, size_t size) {
    if (!gradients || size == 0) return 0.0;
    
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    
    return math_sqrt(norm);
}

/**
 * Apply gradient accumulation
 * Adds current gradients to accumulated gradients
 * 
 * @param accumulated_grads Accumulated gradients [size]
 * @param current_grads Current gradients [size]
 * @param size Number of parameters
 */
void cllm_accumulate_gradients(double* accumulated_grads, double* current_grads, size_t size) {
    if (!accumulated_grads || !current_grads || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        accumulated_grads[i] += current_grads[i];
    }
}

/**
 * Scale gradients (for gradient accumulation)
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @param scale Scale factor
 */
void cllm_scale_gradients(double* gradients, size_t size, double scale) {
    if (!gradients || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        gradients[i] *= scale;
    }
}

/**
 * Check for NaN or Inf in gradients
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @return 1 if NaN/Inf found, 0 otherwise
 */
int cllm_check_gradients_valid(double* gradients, size_t size) {
    if (!gradients || size == 0) return 0;
    
    for (size_t i = 0; i < size; i++) {
        // Check for NaN
        if (gradients[i] != gradients[i]) {
            return 1;
        }
        
        // Check for Inf
        if (gradients[i] > 1e38f || gradients[i] < -1e38f) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Exponential moving average for model parameters
 * Used for model averaging (e.g., EMA of weights)
 * 
 * @param ema_weights EMA weights [size]
 * @param current_weights Current weights [size]
 * @param size Number of parameters
 * @param decay Decay rate (typically 0.999)
 */
void cllm_update_ema_weights(double* ema_weights, double* current_weights,
                            size_t size, double decay) {
    if (!ema_weights || !current_weights || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        ema_weights[i] = decay * ema_weights[i] + (1.0 - decay) * current_weights[i];
    }
}