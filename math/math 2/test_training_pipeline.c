#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_vocabulary.h"

// External training function
extern int cllm_train_epoch(CLLMModel* model, const char* training_file, int batch_size);

int main() {
    printf("=== Training Pipeline Test ===\n\n");
    
    // Create a small model for testing
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 200);
    config.enable_blind_recovery = false;
    config.enable_harmonic_integration = false;
    config.enable_ntt_attention = false;
    config.enable_kissing_spheres = false;
    config.learning_rate = 0.01;
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("✓ Model created (embedding_dim=%u, layers=%u)\n\n", 
           model->embedding_dim, model->num_layers);
    
    // Build vocabulary from training data
    printf("Building vocabulary from training_data.txt...\n");
    CLLMVocabulary* vocab = cllm_vocab_create(500);
    
    // Read training file and build vocabulary
    FILE* f = fopen("training_data.txt", "r");
    if (!f) {
        fprintf(stderr, "Failed to open training_data.txt\n");
        cllm_free_model(model);
        return 1;
    }
    
    char line[1024];
    int total_words = 0;
    while (fgets(line, sizeof(line), f)) {
        // Simple word tokenization
        char* token = strtok(line, " \t\n.,;:!?");
        while (token) {
            // Convert to lowercase
            for (char* p = token; *p; p++) {
                if (*p >= 'A' && *p <= 'Z') *p = *p + 32;
            }
            cllm_vocab_add_token(vocab, token);
            total_words++;
            token = strtok(NULL, " \t\n.,;:!?");
        }
    }
    fclose(f);
    
    printf("✓ Vocabulary built: %u unique tokens from %d words\n", vocab->size, total_words);
    
    // Attach vocabulary to model
    cllm_set_vocabulary(model, vocab);
    printf("✓ Vocabulary attached to model\n\n");
    
    // Train for a few epochs
    printf("Training for 5 epochs...\n");
    double initial_loss = model->metrics.current_loss;
    
    for (int epoch = 0; epoch < 5; epoch++) {
        printf("\nEpoch %d:\n", epoch + 1);
        
        int result = cllm_train_epoch(model, "training_data.txt", 4);
        if (result < 0) {
            fprintf(stderr, "  ✗ Training failed\n");
            break;
        }
        
        printf("  Loss: %.6f\n", model->metrics.current_loss);
        printf("  Tokens processed: %lu\n", model->metrics.tokens_processed);
        
        // Check if loss is decreasing
        if (epoch > 0 && model->metrics.current_loss < initial_loss) {
            printf("  ✓ Loss is decreasing!\n");
        }
    }
    
    double final_loss = model->metrics.current_loss;
    printf("\n=== Training Summary ===\n");
    printf("Initial loss: %.6f\n", initial_loss);
    printf("Final loss: %.6f\n", final_loss);
    
    if (final_loss < initial_loss) {
        printf("✅ Loss decreased by %.6f - Training is working!\n", initial_loss - final_loss);
    } else {
        printf("⚠️  Loss did not decrease - Training may have issues\n");
    }
    
    // Save trained model
    printf("\nSaving trained model...\n");
    cllm_write_model(model, "trained_model.cllm");
    
    // Cleanup
    cllm_free_model(model);
    
    return 0;
}