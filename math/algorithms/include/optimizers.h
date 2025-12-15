/**
 * @file optimizers.h
 * @brief Mathematical optimization algorithms
 * 
 * This library provides pure mathematical implementations of optimization algorithms
 * used in numerical optimization, machine learning, and scientific computing.
 * 
 * These are MATHEMATICAL ALGORITHMS based on:
 * - Gradient descent
 * - Momentum methods
 * - Adaptive learning rates
 * - Second-order methods
 * 
 * They can be used for ANY optimization problem, not just neural networks.
 */

#ifndef OPTIMIZERS_H
#define OPTIMIZERS_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Optimizer types
 */
typedef enum {
    OPTIMIZER_SGD,           /**< Stochastic Gradient Descent */
    OPTIMIZER_MOMENTUM,      /**< SGD with momentum */
    OPTIMIZER_NESTEROV,      /**< Nesterov accelerated gradient */
    OPTIMIZER_ADAGRAD,       /**< Adaptive Gradient Algorithm */
    OPTIMIZER_RMSPROP,       /**< Root Mean Square Propagation */
    OPTIMIZER_ADAM,          /**< Adaptive Moment Estimation */
    OPTIMIZER_ADAMW,         /**< Adam with decoupled weight decay */
    OPTIMIZER_NADAM          /**< Nesterov-accelerated Adam */
} OptimizerType;

/**
 * @brief Learning rate schedule types
 */
typedef enum {
    LR_SCHEDULE_CONSTANT,    /**< Constant learning rate */
    LR_SCHEDULE_STEP,        /**< Step decay */
    LR_SCHEDULE_EXPONENTIAL, /**< Exponential decay */
    LR_SCHEDULE_COSINE,      /**< Cosine annealing */
    LR_SCHEDULE_WARMUP       /**< Linear warmup then decay */
} LRScheduleType;

/**
 * @brief Optimizer configuration
 */
typedef struct {
    OptimizerType type;          /**< Optimizer type */
    double learning_rate;        /**< Initial learning rate */
    double momentum;             /**< Momentum coefficient (0.0-1.0) */
    double beta1;                /**< First moment decay (Adam) */
    double beta2;                /**< Second moment decay (Adam) */
    double epsilon;              /**< Small constant for numerical stability */
    double weight_decay;         /**< L2 regularization coefficient */
    bool amsgrad;                /**< Use AMSGrad variant (Adam) */
    bool nesterov;               /**< Use Nesterov momentum */
} OptimizerConfig;

/**
 * @brief Learning rate schedule configuration
 */
typedef struct {
    LRScheduleType type;         /**< Schedule type */
    double initial_lr;           /**< Initial learning rate */
    double final_lr;             /**< Final learning rate */
    size_t warmup_steps;         /**< Number of warmup steps */
    size_t decay_steps;          /**< Steps between decay */
    double decay_rate;           /**< Decay rate */
    size_t total_steps;          /**< Total training steps */
} LRScheduleConfig;

/**
 * @brief Optimizer state (internal state for adaptive methods)
 */
typedef struct {
    OptimizerType type;          /**< Optimizer type */
    size_t num_parameters;       /**< Number of parameters */
    size_t step;                 /**< Current step number */
    
    /* Momentum state */
    double* velocity;            /**< Velocity (momentum) */
    
    /* Adam state */
    double* first_moment;        /**< First moment estimate (mean) */
    double* second_moment;       /**< Second moment estimate (variance) */
    double* max_second_moment;   /**< Max second moment (AMSGrad) */
    
    /* RMSprop state */
    double* squared_gradient;    /**< Running average of squared gradients */
    
    /* Configuration */
    OptimizerConfig config;      /**< Optimizer configuration */
} OptimizerState;

/* ============================================================================
 * Optimizer Creation and Management
 * ============================================================================ */

/**
 * @brief Create optimizer configuration with default values
 * @param type Optimizer type
 * @return Optimizer configuration
 */
OptimizerConfig optimizer_config_create(OptimizerType type);

/**
 * @brief Create optimizer state
 * @param config Optimizer configuration
 * @param num_parameters Number of parameters to optimize
 * @return Optimizer state (caller must free with optimizer_state_free)
 */
OptimizerState* optimizer_state_create(
    const OptimizerConfig* config,
    size_t num_parameters
);

/**
 * @brief Free optimizer state
 * @param state Optimizer state to free
 */
void optimizer_state_free(OptimizerState* state);

/**
 * @brief Reset optimizer state (clear momentum, moments, etc.)
 * @param state Optimizer state to reset
 */
void optimizer_state_reset(OptimizerState* state);

/* ============================================================================
 * Optimization Step Functions
 * ============================================================================ */

/**
 * @brief Perform one optimization step (update parameters)
 * @param state Optimizer state
 * @param parameters Parameters to update (modified in place)
 * @param gradients Gradients of loss w.r.t. parameters
 * @param num_parameters Number of parameters
 * @return Success status
 */
bool optimizer_step(
    OptimizerState* state,
    double* parameters,
    const double* gradients,
    size_t num_parameters
);

/**
 * @brief SGD optimization step
 * @param parameters Parameters to update
 * @param gradients Gradients
 * @param num_parameters Number of parameters
 * @param learning_rate Learning rate
 * @param weight_decay L2 regularization
 */
void optimizer_sgd_step(
    double* parameters,
    const double* gradients,
    size_t num_parameters,
    double learning_rate,
    double weight_decay
);

/**
 * @brief SGD with momentum step
 * @param parameters Parameters to update
 * @param gradients Gradients
 * @param velocity Velocity (momentum) state
 * @param num_parameters Number of parameters
 * @param learning_rate Learning rate
 * @param momentum Momentum coefficient
 * @param weight_decay L2 regularization
 * @param nesterov Use Nesterov momentum
 */
void optimizer_momentum_step(
    double* parameters,
    const double* gradients,
    double* velocity,
    size_t num_parameters,
    double learning_rate,
    double momentum,
    double weight_decay,
    bool nesterov
);

/**
 * @brief Adam optimization step
 * @param parameters Parameters to update
 * @param gradients Gradients
 * @param first_moment First moment estimate
 * @param second_moment Second moment estimate
 * @param max_second_moment Max second moment (AMSGrad)
 * @param num_parameters Number of parameters
 * @param step Current step number
 * @param learning_rate Learning rate
 * @param beta1 First moment decay
 * @param beta2 Second moment decay
 * @param epsilon Numerical stability constant
 * @param weight_decay L2 regularization
 * @param amsgrad Use AMSGrad variant
 */
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
);

/**
 * @brief RMSprop optimization step
 * @param parameters Parameters to update
 * @param gradients Gradients
 * @param squared_gradient Running average of squared gradients
 * @param num_parameters Number of parameters
 * @param learning_rate Learning rate
 * @param beta Decay rate
 * @param epsilon Numerical stability constant
 * @param weight_decay L2 regularization
 */
void optimizer_rmsprop_step(
    double* parameters,
    const double* gradients,
    double* squared_gradient,
    size_t num_parameters,
    double learning_rate,
    double beta,
    double epsilon,
    double weight_decay
);

/* ============================================================================
 * Learning Rate Scheduling
 * ============================================================================ */

/**
 * @brief Create learning rate schedule configuration
 * @param type Schedule type
 * @param initial_lr Initial learning rate
 * @return Schedule configuration
 */
LRScheduleConfig lr_schedule_config_create(
    LRScheduleType type,
    double initial_lr
);

/**
 * @brief Get learning rate for current step
 * @param config Schedule configuration
 * @param step Current step number
 * @return Learning rate for this step
 */
double lr_schedule_get_lr(
    const LRScheduleConfig* config,
    size_t step
);

/**
 * @brief Constant learning rate
 * @param initial_lr Learning rate
 * @param step Current step (unused)
 * @return Constant learning rate
 */
double lr_schedule_constant(double initial_lr, size_t step);

/**
 * @brief Step decay learning rate
 * @param initial_lr Initial learning rate
 * @param step Current step
 * @param decay_steps Steps between decay
 * @param decay_rate Decay rate
 * @return Decayed learning rate
 */
double lr_schedule_step_decay(
    double initial_lr,
    size_t step,
    size_t decay_steps,
    double decay_rate
);

/**
 * @brief Exponential decay learning rate
 * @param initial_lr Initial learning rate
 * @param step Current step
 * @param decay_rate Decay rate
 * @return Decayed learning rate
 */
double lr_schedule_exponential_decay(
    double initial_lr,
    size_t step,
    double decay_rate
);

/**
 * @brief Cosine annealing learning rate
 * @param initial_lr Initial learning rate
 * @param final_lr Final learning rate
 * @param step Current step
 * @param total_steps Total training steps
 * @return Learning rate
 */
double lr_schedule_cosine_annealing(
    double initial_lr,
    double final_lr,
    size_t step,
    size_t total_steps
);

/**
 * @brief Linear warmup then decay
 * @param initial_lr Initial learning rate
 * @param step Current step
 * @param warmup_steps Number of warmup steps
 * @param total_steps Total training steps
 * @return Learning rate
 */
double lr_schedule_warmup(
    double initial_lr,
    size_t step,
    size_t warmup_steps,
    size_t total_steps
);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Apply weight decay (L2 regularization)
 * @param parameters Parameters to regularize
 * @param num_parameters Number of parameters
 * @param weight_decay Regularization coefficient
 */
void optimizer_apply_weight_decay(
    double* parameters,
    size_t num_parameters,
    double weight_decay
);

/**
 * @brief Compute gradient norm
 * @param gradients Gradient values
 * @param num_parameters Number of parameters
 * @return L2 norm of gradients
 */
double optimizer_gradient_norm(
    const double* gradients,
    size_t num_parameters
);

/**
 * @brief Get optimizer name as string
 * @param type Optimizer type
 * @return Optimizer name
 */
const char* optimizer_type_name(OptimizerType type);

/**
 * @brief Get learning rate schedule name as string
 * @param type Schedule type
 * @return Schedule name
 */
const char* lr_schedule_type_name(LRScheduleType type);

#endif /* OPTIMIZERS_H */
