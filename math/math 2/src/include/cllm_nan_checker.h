#ifndef CLLM_NAN_CHECKER_H
#define CLLM_NAN_CHECKER_H

#include <stdbool.h>
#include <stddef.h>
#include "cllm_training.h"

// Check array for NaN values
int check_array_for_nan_double(const double* array, size_t size, const char* name);
int check_array_for_nan_float(const float* array, size_t size, const char* name);

// Check specific components for NaN
bool check_embeddings_for_nan(CLLMTraining* training);
bool check_attention_outputs_for_nan(CLLMTraining* training, uint32_t layer);
bool check_feedforward_outputs_for_nan(CLLMTraining* training, uint32_t layer);
bool check_logits_for_nan(CLLMTraining* training);
bool check_gradients_for_nan(CLLMTraining* training);
bool check_weights_for_nan(CLLMModel* model);

// Comprehensive checks
bool check_forward_pass_for_nan(CLLMTraining* training);
bool check_backward_pass_for_nan(CLLMTraining* training);

#endif // CLLM_NAN_CHECKER_H