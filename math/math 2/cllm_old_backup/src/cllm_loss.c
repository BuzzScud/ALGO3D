/*
 * CLLM Loss Computation - Utility Functions
 * 
 * NOTE: The main loss function is cllm_compute_loss() in cllm_training.c
 * This file contains only utility functions for metrics and evaluation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "math/transcendental.h"

/**
 * Compute softmax in-place
 * 
 * @param logits Input/output logits [size]
 * @param size Array size
 */
static void softmax_inplace(double* logits, int size) {
    if (!logits || size <= 0) return;
    
    // Find max for numerical stability
    double max_logit = logits[0];
    for (int i = 1; i < size; i++) {
        if (logits[i] > max_logit) {
            max_logit = logits[i];
        }
    }
    
    // Compute exp and sum
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        logits[i] = math_exp(logits[i] - max_logit);
        sum += logits[i];
    }
    
    // Normalize
    if (sum > 1e-8f) {
        for (int i = 0; i < size; i++) {
            logits[i] /= sum;
        }
    }
}

/**
 * Compute loss gradient for backpropagation
 * 
 * Gradient = P(predicted) - 1[target]
 * where 1[target] is one-hot vector
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param grad_output Output gradient [vocab_size]
 * @param vocab_size Vocabulary size
 */
void cllm_compute_loss_gradient(double* logits, uint32_t target, 
                                double* grad_output, int vocab_size) {
    if (!logits || !grad_output || target >= (uint32_t)vocab_size || vocab_size <= 0) {
        return;
    }
    
    // Compute softmax probabilities
    memcpy(grad_output, logits, vocab_size * sizeof(float));
    softmax_inplace(grad_output, vocab_size);
    
    // Subtract 1 from target position: grad = P - 1[target]
    grad_output[target] -= 1.0;
}

/**
 * Compute perplexity from loss
 * 
 * Perplexity = exp(loss)
 * 
 * @param loss Loss value
 * @return Perplexity value
 */
double cllm_compute_perplexity(double loss) {
    return math_exp(loss);
}

/**
 * Compute accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @return Accuracy (0 to 1)
 */
double cllm_compute_accuracy(double* logits, uint32_t* targets,
                           int batch_size, int vocab_size) {
    if (!logits || !targets || batch_size <= 0 || vocab_size <= 0) {
        return 0.0;
    }
    
    int correct = 0;
    
    for (int i = 0; i < batch_size; i++) {
        double* batch_logits = &logits[i * vocab_size];
        
        // Find argmax
        int pred = 0;
        double max_logit = batch_logits[0];
        for (int j = 1; j < vocab_size; j++) {
            if (batch_logits[j] > max_logit) {
                max_logit = batch_logits[j];
                pred = j;
            }
        }
        
        if (pred == (int)targets[i]) {
            correct++;
        }
    }
    
    return (float)correct / (float)batch_size;
}

/**
 * Compute top-k accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @param k Top-k value
 * @return Top-k accuracy (0 to 1)
 */
double cllm_compute_top_k_accuracy(double* logits, uint32_t* targets,
                                  int batch_size, int vocab_size, int k) {
    if (!logits || !targets || batch_size <= 0 || vocab_size <= 0 || k <= 0) {
        return 0.0;
    }
    
    int correct = 0;
    
    for (int i = 0; i < batch_size; i++) {
        double* batch_logits = &logits[i * vocab_size];
        uint32_t target = targets[i];
        
        // Find top-k predictions
        int* top_k_indices = (int*)malloc(k * sizeof(int));
        if (!top_k_indices) continue;
        
        // Simple selection of top-k
        for (int j = 0; j < k && j < vocab_size; j++) {
            int max_idx = 0;
            double max_val = -1e9f;
            
            for (int m = 0; m < vocab_size; m++) {
                // Check if already selected
                int already_selected = 0;
                for (int n = 0; n < j; n++) {
                    if (top_k_indices[n] == m) {
                        already_selected = 1;
                        break;
                    }
                }
                
                if (!already_selected && batch_logits[m] > max_val) {
                    max_val = batch_logits[m];
                    max_idx = m;
                }
            }
            
            top_k_indices[j] = max_idx;
        }
        
        // Check if target is in top-k
        for (int j = 0; j < k; j++) {
            if (top_k_indices[j] == (int)target) {
                correct++;
                break;
            }
        }
        
        free(top_k_indices);
    }
    
    return (float)correct / (float)batch_size;
}
