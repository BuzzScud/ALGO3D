#ifndef CLLM_PRODUCTION_H
#define CLLM_PRODUCTION_H

#include "cllm_training.h"

/**
 * Validation Set
 */
typedef struct ValidationSet ValidationSet;

/**
 * Early Stopping State
 */
typedef struct EarlyStoppingState EarlyStoppingState;

/**
 * Learning Rate Scheduler
 */
typedef struct LRScheduler LRScheduler;

/**
 * Training Metrics
 */
typedef struct TrainingMetrics TrainingMetrics;

// Validation Set Functions
ValidationSet* create_validation_set(const char* filepath, int batch_size, int seq_len);
void free_validation_set(ValidationSet* val_set);
float evaluate_validation(CLLMTraining* training, ValidationSet* val_set);

// Checkpoint Functions
int save_checkpoint(CLLMTraining* training, const char* filepath);
int load_checkpoint(CLLMTraining* training, const char* filepath);

// Early Stopping Functions
EarlyStoppingState* create_early_stopping(float initial_loss, int patience);
int check_early_stopping(EarlyStoppingState* state, float val_loss);

// Learning Rate Scheduler Functions
LRScheduler* create_lr_scheduler(float initial_lr, int warmup_steps, int total_steps);
float get_learning_rate(LRScheduler* scheduler);
void step_lr_scheduler(LRScheduler* scheduler, CLLMTraining* training);

// Gradient Clipping
void clip_gradients(CLLMTraining* training, float max_norm);

// Metrics Tracking
TrainingMetrics* create_training_metrics(int capacity);
void record_metrics(TrainingMetrics* metrics, int epoch, float train_loss, float val_loss, float lr);
void save_metrics_csv(TrainingMetrics* metrics, const char* filepath);
void free_training_metrics(TrainingMetrics* metrics);

#endif // CLLM_PRODUCTION_H