#ifndef CLLM_OPTIMIZER_H
#define CLLM_OPTIMIZER_H

/**
 * CLLM Optimizer System
 * 
 * Implements various optimization algorithms for training the Crystalline
 * Lattice Language Model. Supports SGD, Adam, AdamW with learning rate
 * scheduling, momentum, and weight decay.
 * 
 * Key Features:
 * - Multiple optimizer types (SGD, Adam, AdamW)
 * - Learning rate scheduling (warmup, linear decay, cosine decay, step decay)
 * - Momentum and adaptive learning rates
 * - Weight decay (L2 regularization)
 * - Gradient clipping (by value and by norm)
 * - Integration with hierarchical backpropagation
 * - Thread-safe operations
 * 
 * Mathematical Foundation:
 * - All operations use crystalline library (prime_* functions)
 * - No external math dependencies
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// OPTIMIZER TYPES
// ============================================================================

/**
 * Optimizer Type
 */
typedef enum {
    OPTIMIZER_SGD,              // Stochastic Gradient Descent
    OPTIMIZER_SGD_MOMENTUM,     // SGD with momentum
    OPTIMIZER_SGD_NESTEROV,     // SGD with Nesterov momentum
    OPTIMIZER_ADAM,             // Adam optimizer
    OPTIMIZER_ADAMW,            // AdamW (Adam with decoupled weight decay)
    OPTIMIZER_RMSPROP,          // RMSProp optimizer
    OPTIMIZER_ADAGRAD           // Adagrad optimizer
} OptimizerType;

/**
 * Learning Rate Scheduler Type
 */
typedef enum {
    LR_SCHEDULER_NONE,          // No scheduling (constant LR)
    LR_SCHEDULER_WARMUP,        // Linear warmup only
    LR_SCHEDULER_LINEAR,        // Linear decay
    LR_SCHEDULER_COSINE,        // Cosine annealing
    LR_SCHEDULER_STEP,          // Step decay
    LR_SCHEDULER_EXPONENTIAL,   // Exponential decay
    LR_SCHEDULER_POLYNOMIAL,    // Polynomial decay
    LR_SCHEDULER_CYCLIC         // Cyclic learning rate
} LRSchedulerType;

// ============================================================================
// OPTIMIZER CONFIGURATION
// ============================================================================

/**
 * Optimizer Configuration
 */
typedef struct {
    // Optimizer type
    OptimizerType type;
    
    // Learning rate
    float learning_rate;        // Initial learning rate
    float min_lr;               // Minimum learning rate (for scheduling)
    float max_lr;               // Maximum learning rate (for cyclic)
    
    // SGD parameters
    float momentum;             // Momentum coefficient (default: 0.9)
    bool use_nesterov;          // Use Nesterov momentum (default: false)
    
    // Adam/AdamW parameters
    float beta1;                // First moment decay (default: 0.9)
    float beta2;                // Second moment decay (default: 0.999)
    float epsilon;              // Small constant for numerical stability (default: 1e-8)
    bool amsgrad;               // Use AMSGrad variant (default: false)
    
    // Weight decay
    float weight_decay;         // Weight decay coefficient (L2 regularization)
    bool decoupled_weight_decay; // Use decoupled weight decay (AdamW style)
    
    // Gradient clipping
    bool use_gradient_clipping; // Enable gradient clipping
    float clip_value;           // Clip by value (element-wise)
    float clip_norm;            // Clip by global norm
    
    // Learning rate scheduling
    LRSchedulerType scheduler;
    int warmup_steps;           // Number of warmup steps
    int total_steps;            // Total training steps (for scheduling)
    float decay_rate;           // Decay rate (for exponential/step)
    int decay_steps;            // Steps between decay (for step scheduler)
    int cycle_steps;            // Steps per cycle (for cyclic)
    
} OptimizerConfig;

/**
 * Optimizer State
 * 
 * Maintains optimizer-specific state (momentum, variance, etc.)
 */
typedef struct {
    // Configuration
    OptimizerConfig config;
    
    // Current state
    int step;                   // Current optimization step
    float current_lr;           // Current learning rate
    
    // Parameter and gradient buffers
    float* parameters;          // Pointer to model parameters
    float* gradients;           // Pointer to gradients
    size_t num_parameters;      // Number of parameters
    
    // Optimizer state buffers
    float* momentum_buffer;     // Momentum buffer (SGD, Adam)
    float* variance_buffer;     // Variance buffer (Adam, RMSProp)
    float* max_variance_buffer; // Max variance buffer (AMSGrad)
    
    // Statistics
    float gradient_norm;        // Current gradient norm
    float parameter_norm;       // Current parameter norm
    uint64_t updates_applied;   // Total updates applied
    
    // Thread safety
    pthread_mutex_t mutex;
    
} OptimizerState;

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

/**
 * Create optimizer state
 * 
 * @param config Optimizer configuration
 * @param parameters Pointer to model parameters
 * @param num_parameters Number of parameters
 * @return Allocated optimizer state, or NULL on error
 */
OptimizerState* optimizer_create(
    const OptimizerConfig* config,
    float* parameters,
    size_t num_parameters
);

/**
 * Free optimizer state
 * 
 * @param state Optimizer state to free
 */
void optimizer_free(OptimizerState* state);

/**
 * Reset optimizer state
 * 
 * Resets momentum, variance, and step counter.
 * 
 * @param state Optimizer state
 */
void optimizer_reset(OptimizerState* state);

// ============================================================================
// OPTIMIZATION STEP
// ============================================================================

/**
 * Perform optimization step
 * 
 * Updates parameters based on gradients using the configured optimizer.
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_step(OptimizerState* state, const float* gradients);

/**
 * Zero gradients
 * 
 * Sets all gradients to zero.
 * 
 * @param state Optimizer state
 */
void optimizer_zero_grad(OptimizerState* state);

// ============================================================================
// SPECIFIC OPTIMIZERS
// ============================================================================

/**
 * SGD step
 * 
 * w = w - lr * (g + weight_decay * w)
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_sgd_step(OptimizerState* state, const float* gradients);

/**
 * SGD with momentum step
 * 
 * v = momentum * v + g
 * w = w - lr * (v + weight_decay * w)
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_sgd_momentum_step(OptimizerState* state, const float* gradients);

/**
 * SGD with Nesterov momentum step
 * 
 * v = momentum * v + g
 * w = w - lr * (momentum * v + g + weight_decay * w)
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_sgd_nesterov_step(OptimizerState* state, const float* gradients);

/**
 * Adam step
 * 
 * m = beta1 * m + (1 - beta1) * g
 * v = beta2 * v + (1 - beta2) * g^2
 * m_hat = m / (1 - beta1^t)
 * v_hat = v / (1 - beta2^t)
 * w = w - lr * m_hat / (sqrt(v_hat) + epsilon)
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_adam_step(OptimizerState* state, const float* gradients);

/**
 * AdamW step
 * 
 * Same as Adam but with decoupled weight decay:
 * w = w - lr * (m_hat / (sqrt(v_hat) + epsilon) + weight_decay * w)
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_adamw_step(OptimizerState* state, const float* gradients);

/**
 * RMSProp step
 * 
 * v = beta2 * v + (1 - beta2) * g^2
 * w = w - lr * g / (sqrt(v) + epsilon)
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_rmsprop_step(OptimizerState* state, const float* gradients);

/**
 * Adagrad step
 * 
 * v = v + g^2
 * w = w - lr * g / (sqrt(v) + epsilon)
 * 
 * @param state Optimizer state
 * @param gradients Gradient buffer
 * @return 0 on success, -1 on error
 */
int optimizer_adagrad_step(OptimizerState* state, const float* gradients);

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================

/**
 * Clip gradients by value
 * 
 * Clips each gradient element to [-clip_value, clip_value].
 * 
 * @param gradients Gradient buffer
 * @param num_parameters Number of parameters
 * @param clip_value Clipping threshold
 */
void optimizer_clip_gradients_by_value(
    float* gradients,
    size_t num_parameters,
    float clip_value
);

/**
 * Clip gradients by global norm
 * 
 * Scales gradients if their global norm exceeds max_norm.
 * 
 * @param gradients Gradient buffer
 * @param num_parameters Number of parameters
 * @param max_norm Maximum gradient norm
 * @return Gradient norm before clipping
 */
float optimizer_clip_gradients_by_norm(
    float* gradients,
    size_t num_parameters,
    float max_norm
);

/**
 * Compute gradient norm
 * 
 * @param gradients Gradient buffer
 * @param num_parameters Number of parameters
 * @return L2 norm of gradients
 */
float optimizer_compute_gradient_norm(
    const float* gradients,
    size_t num_parameters
);

/**
 * Compute parameter norm
 * 
 * @param parameters Parameter buffer
 * @param num_parameters Number of parameters
 * @return L2 norm of parameters
 */
float optimizer_compute_parameter_norm(
    const float* parameters,
    size_t num_parameters
);

// ============================================================================
// LEARNING RATE SCHEDULING
// ============================================================================

/**
 * Update learning rate
 * 
 * Updates the learning rate based on the configured scheduler.
 * 
 * @param state Optimizer state
 * @return New learning rate
 */
float optimizer_update_learning_rate(OptimizerState* state);

/**
 * Get current learning rate
 * 
 * @param state Optimizer state
 * @return Current learning rate
 */
float optimizer_get_learning_rate(const OptimizerState* state);

/**
 * Set learning rate
 * 
 * Manually set the learning rate.
 * 
 * @param state Optimizer state
 * @param learning_rate New learning rate
 */
void optimizer_set_learning_rate(OptimizerState* state, float learning_rate);

/**
 * Compute warmup learning rate
 * 
 * Linear warmup from 0 to initial_lr over warmup_steps.
 * 
 * @param step Current step
 * @param warmup_steps Total warmup steps
 * @param initial_lr Initial learning rate
 * @return Warmup learning rate
 */
float optimizer_warmup_lr(int step, int warmup_steps, float initial_lr);

/**
 * Compute linear decay learning rate
 * 
 * Linear decay from initial_lr to min_lr over total_steps.
 * 
 * @param step Current step
 * @param total_steps Total training steps
 * @param initial_lr Initial learning rate
 * @param min_lr Minimum learning rate
 * @return Decayed learning rate
 */
float optimizer_linear_decay_lr(
    int step,
    int total_steps,
    float initial_lr,
    float min_lr
);

/**
 * Compute cosine annealing learning rate
 * 
 * Cosine annealing from initial_lr to min_lr over total_steps.
 * 
 * @param step Current step
 * @param total_steps Total training steps
 * @param initial_lr Initial learning rate
 * @param min_lr Minimum learning rate
 * @return Annealed learning rate
 */
float optimizer_cosine_annealing_lr(
    int step,
    int total_steps,
    float initial_lr,
    float min_lr
);

/**
 * Compute step decay learning rate
 * 
 * Multiply learning rate by decay_rate every decay_steps.
 * 
 * @param step Current step
 * @param decay_steps Steps between decay
 * @param initial_lr Initial learning rate
 * @param decay_rate Decay multiplier
 * @param min_lr Minimum learning rate
 * @return Decayed learning rate
 */
float optimizer_step_decay_lr(
    int step,
    int decay_steps,
    float initial_lr,
    float decay_rate,
    float min_lr
);

/**
 * Compute exponential decay learning rate
 * 
 * lr = initial_lr * decay_rate^(step / decay_steps)
 * 
 * @param step Current step
 * @param decay_steps Decay time constant
 * @param initial_lr Initial learning rate
 * @param decay_rate Decay rate
 * @param min_lr Minimum learning rate
 * @return Decayed learning rate
 */
float optimizer_exponential_decay_lr(
    int step,
    int decay_steps,
    float initial_lr,
    float decay_rate,
    float min_lr
);

/**
 * Compute cyclic learning rate
 * 
 * Cycles between min_lr and max_lr over cycle_steps.
 * 
 * @param step Current step
 * @param cycle_steps Steps per cycle
 * @param min_lr Minimum learning rate
 * @param max_lr Maximum learning rate
 * @return Cyclic learning rate
 */
float optimizer_cyclic_lr(
    int step,
    int cycle_steps,
    float min_lr,
    float max_lr
);

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

/**
 * Get optimizer statistics
 * 
 * @param state Optimizer state
 * @param out_gradient_norm Output: gradient norm
 * @param out_parameter_norm Output: parameter norm
 * @param out_learning_rate Output: current learning rate
 * @param out_step Output: current step
 */
void optimizer_get_statistics(
    const OptimizerState* state,
    float* out_gradient_norm,
    float* out_parameter_norm,
    float* out_learning_rate,
    int* out_step
);

/**
 * Print optimizer state
 * 
 * Prints optimizer configuration and current state for debugging.
 * 
 * @param state Optimizer state
 */
void optimizer_print_state(const OptimizerState* state);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Create default optimizer configuration
 * 
 * @param type Optimizer type
 * @return Default configuration for the specified optimizer
 */
OptimizerConfig optimizer_default_config(OptimizerType type);

/**
 * Validate optimizer configuration
 * 
 * @param config Configuration to validate
 * @return true if valid, false otherwise
 */
bool optimizer_validate_config(const OptimizerConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_OPTIMIZER_H */