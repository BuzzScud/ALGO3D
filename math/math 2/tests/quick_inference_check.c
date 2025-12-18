/**
 * Quick Inference Check
 * 
 * Directly loads a model and checks inference without model manager
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_inference.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <model_file>\n", argv[0]);
        return 1;
    }
    
    printf("Loading model: %s\n", argv[1]);
    
    // Load model directly
    CLLMModel* model = cllm_load_model(argv[1]);
    if (!model) {
        printf("❌ Failed to load model\n");
        return 1;
    }
    
    printf("✓ Model loaded\n");
    printf("  Vocab size: %u\n", model->vocab_size);
    printf("  Embedding dim: %u\n", model->embeddings.embedding_dim);
    printf("  Num layers: %u\n", model->num_layers);
    
    // Check embeddings
    printf("\nChecking embeddings...\n");
    if (model->embeddings.embeddings) {
        float sum = 0.0f;
        float max_val = 0.0f;
        float min_val = 0.0f;
        
        for (uint32_t i = 0; i < 100 && i < model->vocab_size * model->embeddings.embedding_dim; i++) {
            float val = model->embeddings.embeddings[i];
            sum += val * val;
            if (val > max_val) max_val = val;
            if (val < min_val) min_val = val;
        }
        
        printf("  First 100 values:\n");
        printf("    Sum of squares: %.6f\n", sum);
        printf("    Max value: %.6f\n", max_val);
        printf("    Min value: %.6f\n", min_val);
        
        if (sum < 0.001f) {
            printf("  ⚠️  WARNING: Embeddings appear to be near zero!\n");
        }
        
        // Print first 10 embedding values
        printf("\n  First 10 embedding values:\n");
        for (int i = 0; i < 10 && i < (int)(model->vocab_size * model->embeddings.embedding_dim); i++) {
            printf("    [%d] = %.6f\n", i, model->embeddings.embeddings[i]);
        }
    }
    
    // Initialize inference
    printf("\nInitializing inference...\n");
    CLLMInference* inf = cllm_inference_init(model);
    if (!inf) {
        printf("❌ Failed to initialize inference\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("✓ Inference initialized\n");
    
    // Test with simple tokens
    printf("\nTesting forward pass with tokens [0, 1, 2]...\n");
    uint32_t tokens[] = {0, 1, 2};
    
    cllm_forward(inf, tokens, 3);
    
    // Check logits
    printf("\nLogits for first 10 tokens:\n");
    for (int i = 0; i < 10 && i < (int)model->vocab_size; i++) {
        printf("  token_%d: %.6f\n", i, inf->logits[i]);
    }
    
    // Find max logit
    float max_logit = inf->logits[0];
    uint32_t max_idx = 0;
    for (uint32_t i = 1; i < model->vocab_size; i++) {
        if (inf->logits[i] > max_logit) {
            max_logit = inf->logits[i];
            max_idx = i;
        }
    }
    
    printf("\nMax logit: token_%u = %.6f\n", max_idx, max_logit);
    
    // Check if all logits are similar (untrained model indicator)
    float logit_variance = 0.0f;
    float logit_mean = 0.0f;
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        logit_mean += inf->logits[i];
    }
    logit_mean /= model->vocab_size;
    
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        float diff = inf->logits[i] - logit_mean;
        logit_variance += diff * diff;
    }
    logit_variance /= model->vocab_size;
    
    printf("\nLogit statistics:\n");
    printf("  Mean: %.6f\n", logit_mean);
    printf("  Variance: %.6f\n", logit_variance);
    
    if (logit_variance < 0.01f) {
        printf("\n⚠️  WARNING: Logits have very low variance!\n");
        printf("   This indicates the model is UNTRAINED or has a bug.\n");
        printf("   All tokens have similar probabilities.\n");
    }
    
    // Test generation
    printf("\nTesting generation with prompt 'test'...\n");
    char output[1024];
    int generated = cllm_generate(inf, "test", output, sizeof(output));
    
    printf("Generated %d tokens:\n", generated);
    printf("Output: %s\n", output);
    
    // Cleanup
    cllm_inference_free(inf);
    cllm_free_model(model);
    
    return 0;
}