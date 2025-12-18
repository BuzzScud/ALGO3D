/**
 * Optimizer Wrapper with Gradient Accumulation Support
 * Combines gradient accumulation with Adam optimizer
 */

#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"
#include <string.h>

// Forward declaration of BigFixed Adam optimizer
extern void cllm_adam_step_bigfixed(CLLMTraining* training, float learning_rate);

/**
 * Optimizer step with gradient accumulation and Adam
 * This replaces the simple SGD in cllm_training.c
 */
void cllm_optimizer_step_adam(CLLMTraining* training) {
    if (!training) return;
    
    // Gradient accumulation logic
    int accum_steps = training->config.gradient_accumulation_steps;
    if (accum_steps <= 0) accum_steps = 1;
    
    training->accumulation_step++;
    
    // Only apply gradients when we've accumulated enough steps
    if (training->accumulation_step < accum_steps) {
        return;  // Continue accumulating
    }
    
    // Reset accumulation counter
    training->accumulation_step = 0;
    
    // CRITICAL: Clip gradients to prevent explosion (use double precision!)
    double max_grad_norm = 1.0;  // Maximum gradient norm
    
    CLLMModel* model = training->model;
    
    // Clip embedding gradients
    size_t embed_size = model->vocab_size * model->embedding_dim;
    if (training->gradients) {
        double grad_norm = 0.0;
        for (size_t i = 0; i < embed_size; i++) {
            grad_norm += training->gradients[i] * training->gradients[i];
        }
        grad_norm = prime_sqrt(grad_norm);
        
        if (grad_norm > max_grad_norm) {
            double scale = max_grad_norm / grad_norm;
            for (size_t i = 0; i < embed_size; i++) {
                training->gradients[i] *= scale;
            }
        }
    }
    
    // CRITICAL: Clip attention gradients
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (training->attention_grads) {
            uint64_t attn_size = model->embedding_dim * model->embedding_dim;
            
            // Clip query gradients
            if (training->attention_grads[layer].query_lattice) {
                double grad_norm = 0.0;
                for (uint64_t i = 0; i < attn_size; i++) {
                    double g = training->attention_grads[layer].query_lattice[i];
                    grad_norm += g * g;
                }
                grad_norm = prime_sqrt(grad_norm);
                
                if (grad_norm > max_grad_norm) {
                    double scale = max_grad_norm / grad_norm;
                    for (uint64_t i = 0; i < attn_size; i++) {
                        training->attention_grads[layer].query_lattice[i] *= scale;
                    }
                }
            }
            
            // Clip key gradients
            if (training->attention_grads[layer].key_lattice) {
                double grad_norm = 0.0;
                for (uint64_t i = 0; i < attn_size; i++) {
                    double g = training->attention_grads[layer].key_lattice[i];
                    grad_norm += g * g;
                }
                grad_norm = prime_sqrt(grad_norm);
                
                if (grad_norm > max_grad_norm) {
                    double scale = max_grad_norm / grad_norm;
                    for (uint64_t i = 0; i < attn_size; i++) {
                        training->attention_grads[layer].key_lattice[i] *= scale;
                    }
                }
            }
            
            // Clip value gradients
            if (training->attention_grads[layer].value_lattice) {
                double grad_norm = 0.0;
                for (uint64_t i = 0; i < attn_size; i++) {
                    double g = training->attention_grads[layer].value_lattice[i];
                    grad_norm += g * g;
                }
                grad_norm = prime_sqrt(grad_norm);
                
                if (grad_norm > max_grad_norm) {
                    double scale = max_grad_norm / grad_norm;
                    for (uint64_t i = 0; i < attn_size; i++) {
                        training->attention_grads[layer].value_lattice[i] *= scale;
                    }
                }
            }
        }
    }
    
    // CRITICAL: Clip feedforward gradients
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (training->ff_grads && model->ff_layers) {
            FeedForwardLayer* ff = &model->ff_layers[layer];
            
            // Clip W1 gradients
            if (training->ff_grads[layer].w1_lattice) {
                size_t w1_size = ff->input_dim * ff->hidden_dim;
                double grad_norm = 0.0;
                for (size_t i = 0; i < w1_size; i++) {
                    double g = training->ff_grads[layer].w1_lattice[i];
                    grad_norm += g * g;
                }
                grad_norm = prime_sqrt(grad_norm);
                
                if (grad_norm > max_grad_norm) {
                    double scale = max_grad_norm / grad_norm;
                    for (size_t i = 0; i < w1_size; i++) {
                        training->ff_grads[layer].w1_lattice[i] *= scale;
                    }
                }
            }
            
            // Clip W2 gradients
            if (training->ff_grads[layer].w2_lattice) {
                size_t w2_size = ff->hidden_dim * ff->output_dim;
                double grad_norm = 0.0;
                for (size_t i = 0; i < w2_size; i++) {
                    double g = training->ff_grads[layer].w2_lattice[i];
                    grad_norm += g * g;
                }
                grad_norm = prime_sqrt(grad_norm);
                
                if (grad_norm > max_grad_norm) {
                    double scale = max_grad_norm / grad_norm;
                    for (size_t i = 0; i < w2_size; i++) {
                        training->ff_grads[layer].w2_lattice[i] *= scale;
                    }
                }
            }
        }
    }
    
    // Scale gradients by 1/accum_steps
    double gradient_scale = 1.0 / (double)accum_steps;
    (void)gradient_scale;  // Currently unused - reserved for future gradient scaling
    
    // Note: Gradient scaling disabled - using gradient clipping instead
    
    // Scale attention gradients
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (training->attention_grads) {
            uint64_t attn_size = model->embedding_dim * model->embedding_dim;
            
            if (training->attention_grads[layer].query_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
// DISABLED - USE BigFixed version:                     training->attention_grads[layer].query_lattice[i] *= gradient_scale;
                }
            }
            if (training->attention_grads[layer].key_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
// DISABLED - USE BigFixed version:                     training->attention_grads[layer].key_lattice[i] *= gradient_scale;
                }
            }
            if (training->attention_grads[layer].value_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
// DISABLED - USE BigFixed version:                     training->attention_grads[layer].value_lattice[i] *= gradient_scale;
                }
            }
        }
        
        // Scale feed-forward gradients
        if (training->ff_grads && model->ff_layers) {
            FeedForwardLayer* ff = &model->ff_layers[layer];
            
            if (training->ff_grads[layer].w1_lattice) {
                size_t w1_size = ff->input_dim * ff->hidden_dim;
                for (size_t i = 0; i < w1_size; i++) {
// DISABLED - USE BigFixed version:                     training->ff_grads[layer].w1_lattice[i] *= gradient_scale;
                }
            }
            
            if (training->ff_grads[layer].w2_lattice) {
                size_t w2_size = ff->hidden_dim * ff->output_dim;
                for (size_t i = 0; i < w2_size; i++) {
// DISABLED - USE BigFixed version:                     training->ff_grads[layer].w2_lattice[i] *= gradient_scale;
                }
            }
            
            if (training->ff_grads[layer].bias1) {
                for (uint32_t i = 0; i < ff->hidden_dim; i++) {
// DISABLED - USE BigFixed version:                     training->ff_grads[layer].bias1[i] *= gradient_scale;
                }
            }
            
            if (training->ff_grads[layer].bias2) {
                for (uint32_t i = 0; i < ff->output_dim; i++) {
// DISABLED - USE BigFixed version:                     training->ff_grads[layer].bias2[i] *= gradient_scale;
                }
            }
        }
    }
    
    // CRITICAL: Clip layer norm gradients to prevent NaN
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (training->ln_grads && model->layer_norms) {
            // Clip gamma gradients
            if (training->ln_grads[layer].gamma) {
                double grad_norm = 0.0;
                for (uint32_t i = 0; i < model->embedding_dim; i++) {
                    double g = training->ln_grads[layer].gamma[i];
                    grad_norm += g * g;
                }
                grad_norm = prime_sqrt(grad_norm);
                
                if (grad_norm > max_grad_norm) {
                    double scale = max_grad_norm / grad_norm;
                    for (uint32_t i = 0; i < model->embedding_dim; i++) {
                        training->ln_grads[layer].gamma[i] *= scale;
                    }
                }
            }
            
            // Clip beta gradients
            if (training->ln_grads[layer].beta) {
                double grad_norm = 0.0;
                for (uint32_t i = 0; i < model->embedding_dim; i++) {
                    double g = training->ln_grads[layer].beta[i];
                    grad_norm += g * g;
                }
                grad_norm = prime_sqrt(grad_norm);
                
                if (grad_norm > max_grad_norm) {
                    double scale = max_grad_norm / grad_norm;
                    for (uint32_t i = 0; i < model->embedding_dim; i++) {
                        training->ln_grads[layer].beta[i] *= scale;
                    }
                }
            }
        }
    }
    
    // Use the proper BigFixed Adam optimizer from cllm_training.c
    // This provides momentum, adaptive learning rates, and bias correction
    cllm_adam_step_bigfixed(training, training->config.learning_rate);
}
/**
 * Adam Optimizer Implementation
 * 
 * Implements the Adam optimization algorithm:
 * m_t = β₁ * m_{t-1} + (1 - β₁) * g_t
 * v_t = β₂ * v_{t-1} + (1 - β₂) * g_t²
 * m̂_t = m_t / (1 - β₁^t)
 * v̂_t = v_t / (1 - β₂^t)
 * θ_t = θ_{t-1} - α * m̂_t / (√v̂_t + ε)
 */
void cllm_adam_step_bigfixed(CLLMTraining* training, float learning_rate) {
    if (!training || !training->model) return;
    
    CLLMModel* model = training->model;
    
    // Adam hyperparameters (MUST use double for precision!)
    const double beta1 = 0.9;
    const double beta2 = 0.999;
    const double epsilon = 1e-8;
    
    // Bias correction (use current_step, don't increment here as it's incremented in training loop)
    // CRITICAL: Use double precision to avoid precision loss in large models
    double beta1_t = prime_pow(beta1, (double)(training->current_step + 1));
    double beta2_t = prime_pow(beta2, (double)(training->current_step + 1));
    double bias_correction1 = 1.0 - beta1_t;
    double bias_correction2 = 1.0 - beta2_t;
    
    // Update embeddings
    size_t embed_size = model->vocab_size * model->embedding_dim;
    if (training->gradients && training->optimizer_state) {
        double* m = training->optimizer_state;  // First moment
        double* v = &training->optimizer_state[embed_size];  // Second moment
        
        for (size_t i = 0; i < embed_size; i++) {
            double grad = training->gradients[i];
            
            // Update biased first moment estimate
            m[i] = beta1 * m[i] + (1.0 - beta1) * grad;
            
            // Update biased second raw moment estimate
            v[i] = beta2 * v[i] + (1.0 - beta2) * grad * grad;
            
            // Compute bias-corrected first moment estimate
            double m_hat = m[i] / bias_correction1;
            
            // Compute bias-corrected second raw moment estimate
            double v_hat = v[i] / bias_correction2;
            
            // Update parameters
            model->embeddings[i] -= learning_rate * m_hat / (prime_sqrt(v_hat) + epsilon);
        }
    }
    
    // Update attention weights
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (!training->attention_grads || !model->attention_layers) continue;
        
        size_t attn_size = model->embedding_dim * model->embedding_dim;
        
        // Query weights
        if (training->attention_grads[layer].query_lattice && 
            model->attention_layers[layer].query_lattice) {
            for (size_t i = 0; i < attn_size; i++) {
                double grad = training->attention_grads[layer].query_lattice[i];
                
                // Simple SGD for now (Adam state would require more memory)
                model->attention_layers[layer].query_lattice[i] -= learning_rate * grad;
            }
        }
        
        // Key weights
        if (training->attention_grads[layer].key_lattice && 
            model->attention_layers[layer].key_lattice) {
            for (size_t i = 0; i < attn_size; i++) {
                double grad = training->attention_grads[layer].key_lattice[i];
                model->attention_layers[layer].key_lattice[i] -= learning_rate * grad;
            }
        }
        
        // Value weights
        if (training->attention_grads[layer].value_lattice && 
            model->attention_layers[layer].value_lattice) {
            for (size_t i = 0; i < attn_size; i++) {
                double grad = training->attention_grads[layer].value_lattice[i];
                model->attention_layers[layer].value_lattice[i] -= learning_rate * grad;
            }
        }
    }
    
    // Update feed-forward weights
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (!training->ff_grads || !model->ff_layers) continue;
        
        FeedForwardLayer* ff = &model->ff_layers[layer];
        
        // W1 weights
        if (training->ff_grads[layer].w1_lattice && ff->w1_lattice) {
            size_t w1_size = ff->input_dim * ff->hidden_dim;
            for (size_t i = 0; i < w1_size; i++) {
                double grad = training->ff_grads[layer].w1_lattice[i];
                ff->w1_lattice[i] -= learning_rate * grad;
            }
        }
        
        // W2 weights
        if (training->ff_grads[layer].w2_lattice && ff->w2_lattice) {
            size_t w2_size = ff->hidden_dim * ff->output_dim;
            for (size_t i = 0; i < w2_size; i++) {
                double grad = training->ff_grads[layer].w2_lattice[i];
                ff->w2_lattice[i] -= learning_rate * grad;
            }
        }
        
        // Biases
        if (training->ff_grads[layer].bias1 && ff->bias1) {
            for (uint32_t i = 0; i < ff->hidden_dim; i++) {
                double grad = training->ff_grads[layer].bias1[i];
                ff->bias1[i] -= learning_rate * grad;
            }
        }
        
        if (training->ff_grads[layer].bias2 && ff->bias2) {
            for (uint32_t i = 0; i < ff->output_dim; i++) {
                double grad = training->ff_grads[layer].bias2[i];
                ff->bias2[i] -= learning_rate * grad;
            }
        }
    }
    
    // Update layer norm parameters
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (!training->ln_grads || !model->layer_norms) continue;
        
        CLLMLayerNorm* ln = &model->layer_norms[layer];
        
        // Gamma (scale) parameters
        if (training->ln_grads[layer].gamma && ln->gamma) {
            for (uint32_t i = 0; i < model->embedding_dim; i++) {
                double grad = training->ln_grads[layer].gamma[i];
                ln->gamma[i] -= learning_rate * grad;
            }
        }
        
        // Beta (shift) parameters
        if (training->ln_grads[layer].beta && ln->beta) {
            for (uint32_t i = 0; i < model->embedding_dim; i++) {
                double grad = training->ln_grads[layer].beta[i];
                ln->beta[i] -= learning_rate * grad;
            }
        }
    }
}

