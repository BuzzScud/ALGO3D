/**
 * @file optimizers.c
 * @brief Implementation of optimization algorithms
 * 
 * PHASE 1 INTEGRATION: Migrated to NEW math library
 * - Replaced math_sqrt with math_sqrt
 * - Replaced math_pow with math_pow
 */

#include "optimizers.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>

// Using MATH_PI from math library

/* ============================================================================
 * Configuration
 * ============================================================================ */

OptimizerConfig optimizer_config_create(OptimizerType type) {
    OptimizerConfig config = {
        .type = type,
        .learning_rate = 0.001,
        .momentum = 0.9,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
        .weight_decay = 0.0,
        .amsgrad = false,
        .nesterov = false
    };
    
    // Adjust defaults based on optimizer type
    switch (type) {
        case OPTIMIZER_SGD:
            config.learning_rate = 0.01;
            break;
        case OPTIMIZER_MOMENTUM:
        case OPTIMIZER_NESTEROV:
            config.learning_rate = 0.01;
            config.momentum = 0.9;
            break;
        case OPTIMIZER_ADAM:
        case OPTIMIZER_ADAMW:
        case OPTIMIZER_NADAM:
            config.learning_rate = 0.001;
            config.beta1 = 0.9;
            config.beta2 = 0.999;
            break;
        case OPTIMIZER_RMSPROP:
            config.learning_rate = 0.001;
            config.beta2 = 0.99;
            break;
        case OPTIMIZER_ADAGRAD:
            config.learning_rate = 0.01;
            break;
    }
    
    return config;
}

/* ============================================================================
 * State Management
 * ============================================================================ */

OptimizerState* optimizer_state_create(
    const OptimizerConfig* config,
    size_t num_parameters
) {
    if (!config || num_parameters == 0) return NULL;
    
    OptimizerState* state = (OptimizerState*)calloc(1, sizeof(OptimizerState));
    if (!state) return NULL;
    
    state->type = config->type;
    state->num_parameters = num_parameters;
    state->step = 0;
    state->config = *config;
    
    // Allocate state arrays based on optimizer type
    switch (config->type) {
        case OPTIMIZER_MOMENTUM:
        case OPTIMIZER_NESTEROV:
            state->velocity = (double*)calloc(num_parameters, sizeof(double));
            if (!state->velocity) {
                optimizer_state_free(state);
                return NULL;
            }
            break;
            
        case OPTIMIZER_ADAM:
        case OPTIMIZER_ADAMW:
        case OPTIMIZER_NADAM:
            state->first_moment = (double*)calloc(num_parameters, sizeof(double));
            state->second_moment = (double*)calloc(num_parameters, sizeof(double));
            if (!state->first_moment || !state->second_moment) {
                optimizer_state_free(state);
                return NULL;
            }
            if (config->amsgrad) {
                state->max_second_moment = (double*)calloc(num_parameters, sizeof(double));
                if (!state->max_second_moment) {
                    optimizer_state_free(state);
                    return NULL;
                }
            }
            break;
            
        case OPTIMIZER_RMSPROP:
            state->squared_gradient = (double*)calloc(num_parameters, sizeof(double));
            if (!state->squared_gradient) {
                optimizer_state_free(state);
                return NULL;
            }
            break;
            
        case OPTIMIZER_ADAGRAD:
            state->squared_gradient = (double*)calloc(num_parameters, sizeof(double));
            if (!state->squared_gradient) {
                optimizer_state_free(state);
                return NULL;
            }
            break;
            
        case OPTIMIZER_SGD:
            // No additional state needed
            break;
    }
    
    return state;
}

void optimizer_state_free(OptimizerState* state) {
    if (!state) return;
    
    if (state->velocity) free(state->velocity);
    if (state->first_moment) free(state->first_moment);
    if (state->second_moment) free(state->second_moment);
    if (state->max_second_moment) free(state->max_second_moment);
    if (state->squared_gradient) free(state->squared_gradient);
    
    free(state);
}

void optimizer_state_reset(OptimizerState* state) {
    if (!state) return;
    
    state->step = 0;
    
    if (state->velocity) {
        memset(state->velocity, 0, state->num_parameters * sizeof(double));
    }
    if (state->first_moment) {
        memset(state->first_moment, 0, state->num_parameters * sizeof(double));
    }
    if (state->second_moment) {
        memset(state->second_moment, 0, state->num_parameters * sizeof(double));
    }
    if (state->max_second_moment) {
        memset(state->max_second_moment, 0, state->num_parameters * sizeof(double));
    }
    if (state->squared_gradient) {
        memset(state->squared_gradient, 0, state->num_parameters * sizeof(double));
    }
}

/* ============================================================================
 * Optimization Step
 * ============================================================================ */

bool optimizer_step(
    OptimizerState* state,
    double* parameters,
    const double* gradients,
    size_t num_parameters
) {
    if (!state || !parameters || !gradients || num_parameters != state->num_parameters) {
        return false;
    }
    
    state->step++;
    
    switch (state->type) {
        case OPTIMIZER_SGD:
            optimizer_sgd_step(
                parameters, gradients, num_parameters,
                state->config.learning_rate, state->config.weight_decay
            );
            break;
            
        case OPTIMIZER_MOMENTUM:
        case OPTIMIZER_NESTEROV:
            optimizer_momentum_step(
                parameters, gradients, state->velocity, num_parameters,
                state->config.learning_rate, state->config.momentum,
                state->config.weight_decay, state->config.nesterov
            );
            break;
            
        case OPTIMIZER_ADAM:
        case OPTIMIZER_ADAMW:
        case OPTIMIZER_NADAM:
            optimizer_adam_step(
                parameters, gradients,
                state->first_moment, state->second_moment, state->max_second_moment,
                num_parameters, state->step,
                state->config.learning_rate,
                state->config.beta1, state->config.beta2,
                state->config.epsilon, state->config.weight_decay,
                state->config.amsgrad
            );
            break;
            
        case OPTIMIZER_RMSPROP:
            optimizer_rmsprop_step(
                parameters, gradients, state->squared_gradient,
                num_parameters, state->config.learning_rate,
                state->config.beta2, state->config.epsilon,
                state->config.weight_decay
            );
            break;
            
        case OPTIMIZER_ADAGRAD:
            // AdaGrad uses accumulated squared gradients
            for (size_t i = 0; i < num_parameters; i++) {
                state->squared_gradient[i] += gradients[i] * gradients[i];
                parameters[i] -= state->config.learning_rate * gradients[i] /
                                (math_sqrt(state->squared_gradient[i]) + state->config.epsilon);  // PHASE 1: NEW math library
            }
            if (state->config.weight_decay > 0.0) {
                optimizer_apply_weight_decay(parameters, num_parameters, 
                                            state->config.weight_decay);
            }
            break;
    }
    
    return true;
}

/* ============================================================================
 * Individual Optimizer Implementations
 * ============================================================================ */

void optimizer_sgd_step(
    double* parameters,
    const double* gradients,
    size_t num_parameters,
    double learning_rate,
    double weight_decay
) {
    if (!parameters || !gradients || num_parameters == 0) return;
    
    for (size_t i = 0; i < num_parameters; i++) {
        double grad = gradients[i];
        
        // Add weight decay (L2 regularization)
        if (weight_decay > 0.0) {
            grad += weight_decay * parameters[i];
        }
        
        // Update parameter
        parameters[i] -= learning_rate * grad;
    }
}

void optimizer_momentum_step(
    double* parameters,
    const double* gradients,
    double* velocity,
    size_t num_parameters,
    double learning_rate,
    double momentum,
    double weight_decay,
    bool nesterov
) {
    if (!parameters || !gradients || !velocity || num_parameters == 0) return;
    
    for (size_t i = 0; i < num_parameters; i++) {
        double grad = gradients[i];
        
        // Add weight decay
        if (weight_decay > 0.0) {
            grad += weight_decay * parameters[i];
        }
        
        // Update velocity
        velocity[i] = momentum * velocity[i] + grad;
        
        // Update parameter
        if (nesterov) {
            // Nesterov momentum: look ahead
            parameters[i] -= learning_rate * (grad + momentum * velocity[i]);
        } else {
            // Standard momentum
            parameters[i] -= learning_rate * velocity[i];
        }
    }
}

void optimizer_adam_step(
    double* parameters,
    const double* gradients,
    double* first_moment,
    double* second_moment,
    double* max_second_moment,
    size_t num_parameters,
    size_t step,
    double learning_rate,
    double beta1,
    double beta2,
    double epsilon,
    double weight_decay,
    bool amsgrad
) {
    if (!parameters || !gradients || !first_moment || !second_moment || 
        num_parameters == 0 || step == 0) return;
    
    // Compute bias correction terms
    double bias_correction1 = 1.0 - math_pow(beta1, (double)step);  // PHASE 1: NEW math library
    double bias_correction2 = 1.0 - math_pow(beta2, (double)step);  // PHASE 1: NEW math library
    
    for (size_t i = 0; i < num_parameters; i++) {
        double grad = gradients[i];
        
        // Update biased first moment estimate
        first_moment[i] = beta1 * first_moment[i] + (1.0 - beta1) * grad;
        
        // Update biased second raw moment estimate
        second_moment[i] = beta2 * second_moment[i] + (1.0 - beta2) * grad * grad;
        
        // Compute bias-corrected moments
        double m_hat = first_moment[i] / bias_correction1;
        double v_hat = second_moment[i] / bias_correction2;
        
        // AMSGrad: use max of past second moments
        if (amsgrad && max_second_moment) {
            if (v_hat > max_second_moment[i]) {
                max_second_moment[i] = v_hat;
            }
            v_hat = max_second_moment[i];
        }
        
        // Update parameter
        parameters[i] -= learning_rate * m_hat / (math_sqrt(v_hat) + epsilon);  // PHASE 1: NEW math library
        
        // Apply weight decay (decoupled for AdamW)
        if (weight_decay > 0.0) {
            parameters[i] -= learning_rate * weight_decay * parameters[i];
        }
    }
}

void optimizer_rmsprop_step(
    double* parameters,
    const double* gradients,
    double* squared_gradient,
    size_t num_parameters,
    double learning_rate,
    double beta,
    double epsilon,
    double weight_decay
) {
    if (!parameters || !gradients || !squared_gradient || num_parameters == 0) return;
    
    for (size_t i = 0; i < num_parameters; i++) {
        double grad = gradients[i];
        
        // Update running average of squared gradients
        squared_gradient[i] = beta * squared_gradient[i] + (1.0 - beta) * grad * grad;
        
        // Update parameter
        parameters[i] -= learning_rate * grad / (math_sqrt(squared_gradient[i]) + epsilon);  // PHASE 1: NEW math library
        
        // Apply weight decay
        if (weight_decay > 0.0) {
            parameters[i] -= learning_rate * weight_decay * parameters[i];
        }
    }
}

/* ============================================================================
 * Learning Rate Scheduling
 * ============================================================================ */

LRScheduleConfig lr_schedule_config_create(LRScheduleType type, double initial_lr) {
    LRScheduleConfig config = {
        .type = type,
        .initial_lr = initial_lr,
        .final_lr = initial_lr * 0.01,
        .warmup_steps = 0,
        .decay_steps = 1000,
        .decay_rate = 0.96,
        .total_steps = 10000
    };
    return config;
}

double lr_schedule_get_lr(const LRScheduleConfig* config, size_t step) {
    if (!config) return 0.0;
    
    switch (config->type) {
        case LR_SCHEDULE_CONSTANT:
            return lr_schedule_constant(config->initial_lr, step);
            
        case LR_SCHEDULE_STEP:
            return lr_schedule_step_decay(
                config->initial_lr, step, 
                config->decay_steps, config->decay_rate
            );
            
        case LR_SCHEDULE_EXPONENTIAL:
            return lr_schedule_exponential_decay(
                config->initial_lr, step, config->decay_rate
            );
            
        case LR_SCHEDULE_COSINE:
            return lr_schedule_cosine_annealing(
                config->initial_lr, config->final_lr,
                step, config->total_steps
            );
            
        case LR_SCHEDULE_WARMUP:
            return lr_schedule_warmup(
                config->initial_lr, step,
                config->warmup_steps, config->total_steps
            );
            
        default:
            return config->initial_lr;
    }
}

double lr_schedule_constant(double initial_lr, size_t step) {
    (void)step; // Unused
    return initial_lr;
}

double lr_schedule_step_decay(
    double initial_lr,
    size_t step,
    size_t decay_steps,
    double decay_rate
) {
    if (decay_steps == 0) return initial_lr;
    
    size_t num_decays = step / decay_steps;
    return initial_lr * math_pow(decay_rate, (double)num_decays);  // PHASE 1: NEW math library
}

double lr_schedule_exponential_decay(
    double initial_lr,
    size_t step,
    double decay_rate
) {
    return initial_lr * math_exp(-decay_rate * (double)step);  // PHASE 1: NEW math library
}

double lr_schedule_cosine_annealing(
    double initial_lr,
    double final_lr,
    size_t step,
    size_t total_steps
) {
    if (total_steps == 0 || step >= total_steps) return final_lr;
    
    double progress = (double)step / (double)total_steps;
    double cosine = 0.5 * (1.0 + math_cos(MATH_PI * progress));  // PHASE 1: NEW math library
    
    return final_lr + (initial_lr - final_lr) * cosine;
}

double lr_schedule_warmup(
    double initial_lr,
    size_t step,
    size_t warmup_steps,
    size_t total_steps
) {
    if (step < warmup_steps) {
        // Linear warmup
        return initial_lr * ((double)step / (double)warmup_steps);
    } else {
        // Linear decay after warmup
        size_t decay_steps = total_steps - warmup_steps;
        if (decay_steps == 0) return initial_lr;
        
        size_t steps_after_warmup = step - warmup_steps;
        double decay_progress = (double)steps_after_warmup / (double)decay_steps;
        
        return initial_lr * (1.0 - decay_progress);
    }
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

void optimizer_apply_weight_decay(
    double* parameters,
    size_t num_parameters,
    double weight_decay
) {
    if (!parameters || num_parameters == 0 || weight_decay <= 0.0) return;
    
    for (size_t i = 0; i < num_parameters; i++) {
        parameters[i] *= (1.0 - weight_decay);
    }
}

double optimizer_gradient_norm(const double* gradients, size_t num_parameters) {
    if (!gradients || num_parameters == 0) return 0.0;
    
    double norm = 0.0;
    for (size_t i = 0; i < num_parameters; i++) {
        norm += gradients[i] * gradients[i];
    }
    return math_sqrt(norm);  // PHASE 1: NEW math library
}

const char* optimizer_type_name(OptimizerType type) {
    switch (type) {
        case OPTIMIZER_SGD: return "SGD";
        case OPTIMIZER_MOMENTUM: return "Momentum";
        case OPTIMIZER_NESTEROV: return "Nesterov";
        case OPTIMIZER_ADAGRAD: return "AdaGrad";
        case OPTIMIZER_RMSPROP: return "RMSprop";
        case OPTIMIZER_ADAM: return "Adam";
        case OPTIMIZER_ADAMW: return "AdamW";
        case OPTIMIZER_NADAM: return "NAdam";
        default: return "Unknown";
    }
}

const char* lr_schedule_type_name(LRScheduleType type) {
    switch (type) {
        case LR_SCHEDULE_CONSTANT: return "Constant";
        case LR_SCHEDULE_STEP: return "Step";
        case LR_SCHEDULE_EXPONENTIAL: return "Exponential";
        case LR_SCHEDULE_COSINE: return "Cosine";
        case LR_SCHEDULE_WARMUP: return "Warmup";
        default: return "Unknown";
    }
}
