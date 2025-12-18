/**
 * Diagnose Inference Issues
 * 
 * Checks model state and inference pipeline for issues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_inference.h"
#include "cllm_model_manager.h"

void check_model_weights(CLLMModel* model) {
    printf("\n=== Checking Model Weights ===\n");
    
    // Check embeddings
    if (model->embeddings.embeddings) {
        float sum = 0.0f;
        float max = 0.0f;
        for (uint32_t i = 0; i < 100 && i < model->vocab_size * model->embedding_dim; i++) {
            float val = model->embeddings.embeddings[i];
            sum += val * val;
            if (val > max || val < -max) max = (val > 0) ? val : -val;
        }
        printf("Embeddings (first 100): sum_sq=%.6f, max_abs=%.6f\n", sum, max);
        
        if (sum < 0.001f) {
            printf("⚠️  WARNING: Embeddings appear to be near zero (untrained?)\n");
        }
    }
    
    // Check attention weights
    if (model->attention_layers && model->num_layers > 0) {
        AttentionLayer* attn = &model->attention_layers[0];
        if (attn->query_lattice) {
            float sum = 0.0f;
            for (uint32_t i = 0; i < 100 && i < attn->head_dim * attn->head_dim; i++) {
                sum += attn->query_lattice[i] * attn->query_lattice[i];
            }
            printf("Attention weights (first 100): sum_sq=%.6f\n", sum);
            
            if (sum < 0.001f) {
                printf("⚠️  WARNING: Attention weights appear to be near zero (untrained?)\n");
            }
        }
    }
    
    // Check feedforward weights
    if (model->ff_layers && model->num_layers > 0) {
        FeedForwardLayer* ff = &model->ff_layers[0];
        if (ff->w1_lattice) {
            float sum = 0.0f;
            for (uint32_t i = 0; i < 100 && i < ff->input_dim * ff->hidden_dim; i++) {
                sum += ff->w1_lattice[i] * ff->w1_lattice[i];
            }
            printf("FF weights (first 100): sum_sq=%.6f\n", sum);
            
            if (sum < 0.001f) {
                printf("⚠️  WARNING: FF weights appear to be near zero (untrained?)\n");
            }
        }
    }
}

void test_forward_pass(CLLMModel* model) {
    printf("\n=== Testing Forward Pass ===\n");
    
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        printf("❌ Failed to initialize inference\n");
        return;
    }
    
    // Test with simple tokens
    uint32_t tokens[] = {0, 1, 2};
    
    printf("Running forward pass with tokens: [0, 1, 2]\n");
    cllm_forward(inference, tokens, 3);
    
    // Check logits
    if (inference->logits) {
        printf("\nLogits for first 10 tokens:\n");
        for (int i = 0; i < 10 && i < (int)model->vocab_size; i++) {
            printf("  token_%d: %.6f\n", i, inference->logits[i]);
        }
        
        // Find max logit
        float max_logit = inference->logits[0];
        uint32_t max_idx = 0;
        for (uint32_t i = 1; i < model->vocab_size; i++) {
            if (inference->logits[i] > max_logit) {
                max_logit = inference->logits[i];
                max_idx = i;
            }
        }
        
        printf("\nMax logit: token_%u = %.6f\n", max_idx, max_logit);
        
        // Check if all logits are the same (indicates untrained model)
        bool all_same = true;
        for (uint32_t i = 1; i < model->vocab_size; i++) {
            if (inference->logits[i] != inference->logits[0]) {
                all_same = false;
                break;
            }
        }
        
        if (all_same) {
            printf("⚠️  WARNING: All logits are identical - model is untrained!\n");
        }
    }
    
    cllm_inference_cleanup(inference);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <model_name>\n", argv[0]);
        printf("\nDiagnoses inference issues by checking:\n");
        printf("  - Model weight initialization\n");
        printf("  - Forward pass computation\n");
        printf("  - Logits distribution\n");
        return 1;
    }
    
    const char* model_name = argv[1];
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║           Inference Diagnostic Tool                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("\nAcquiring model '%s' from model manager...\n", model_name);
    CLLMModel* model = model_manager_acquire_read(model_name);
    
    if (!model) {
        printf("❌ Failed to acquire model '%s'\n", model_name);
        printf("   Make sure the model exists and has been created.\n");
        return 1;
    }
    
    printf("✓ Model acquired\n");
    printf("\nModel Configuration:\n");
    printf("  Vocabulary size:  %lu\n", (unsigned long)model->vocab_size);
    printf("  Embedding dim:    %lu\n", (unsigned long)model->embedding_dim);
    printf("  Num layers:       %u\n", model->num_layers);
    printf("  Num heads:        %u\n", model->attention_layers[0].num_heads);
    
    // Run diagnostics
    check_model_weights(model);
    test_forward_pass(model);
    
    // Release model
    model_manager_release_read(model_name);
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  Diagnosis Complete                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}