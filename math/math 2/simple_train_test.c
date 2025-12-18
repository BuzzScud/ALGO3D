#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_vocabulary.h"
#include "include/cllm_training.h"
#include "include/cllm_data_loader.h"

int main() {
    printf("=== Simple Training Test ===\n\n");
    
    // Create a tiny model for fast testing
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 200);
    config.enable_blind_recovery = false;
    config.enable_harmonic_integration = false;
    config.enable_ntt_attention = false;
    config.enable_kissing_spheres = false;
    config.learning_rate = 0.01;
    config.num_threads = 1; // Single-threaded for simplicity
    
    printf("1. Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("   ✓ Model created (embedding_dim=%u, layers=%u)\n\n", 
           model->embedding_dim, model->num_layers);
    
    // Create data loader
    printf("2. Creating data loader...\n");
    CLLMDataLoader* loader = cllm_data_loader_create(200, 64);
    if (!loader) {
        fprintf(stderr, "Failed to create data loader\n");
        cllm_free_model(model);
        return 1;
    }
    printf("   ✓ Data loader created\n\n");
    
    // Load training file
    printf("3. Loading training data...\n");
    if (cllm_data_loader_load_file(loader, "training_data.txt") != 0) {
        fprintf(stderr, "Failed to load training data\n");
        cllm_data_loader_free(loader);
        cllm_free_model(model);
        return 1;
    }
    printf("   ✓ Training data loaded\n\n");
    
    // Build vocabulary
    printf("4. Building vocabulary...\n");
    CLLMVocabulary* vocab = cllm_data_loader_build_vocab(loader);
    if (!vocab) {
        fprintf(stderr, "Failed to build vocabulary\n");
        cllm_data_loader_free(loader);
        cllm_free_model(model);
        return 1;
    }
    printf("   ✓ Vocabulary built: %u tokens\n", vocab->size);
    
    // Attach vocabulary to model
    cllm_set_vocabulary(model, vocab);
    printf("   ✓ Vocabulary attached to model\n\n");
    
    // Print data loader stats
    cllm_data_loader_print_stats(loader);
    printf("\n");
    
    // Create training config
    printf("5. Setting up training...\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.01,
        .batch_size = 4,
        .num_epochs = 5,
        .max_steps = 100,
        .weight_decay = 0.01,
        .gradient_clip = 1.0,
        .warmup_steps = 0,
        .save_interval = 50,
        .eval_interval = 10,
        .sequence_length = 32,
        .gradient_accumulation_steps = 1,
        .use_mixed_precision = 0
    };
    strcpy(train_config.optimizer, "adam");
    strcpy(train_config.lr_scheduler, "none");
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to create training context\n");
        cllm_data_loader_free(loader);
        cllm_free_model(model);
        return 1;
    }
    printf("   ✓ Training context created\n\n");
    
    // Create dataset
    printf("6. Creating token dataset...\n");
    extern CLLMTokenDataset* cllm_data_loader_create_dataset(CLLMDataLoader* loader);
    CLLMTokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        fprintf(stderr, "Failed to create dataset\n");
        cllm_training_free(training);
        cllm_data_loader_free(loader);
        cllm_free_model(model);
        return 1;
    }
    printf("   ✓ Dataset created\n\n");
    
    // Set training tokens
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    
    printf("7. Training for %d epochs...\n", train_config.num_epochs);
    double initial_loss = model->metrics.current_loss;
    
    for (int epoch = 0; epoch < train_config.num_epochs; epoch++) {
        printf("\n   Epoch %d/%d:\n", epoch + 1, train_config.num_epochs);
        
        int result = cllm_train(training);
        if (result < 0) {
            fprintf(stderr, "   ✗ Training failed\n");
            break;
        }
        
        printf("     Loss: %.6f\n", model->metrics.current_loss);
        printf("     Tokens: %lu\n", model->metrics.tokens_processed);
    }
    
    double final_loss = model->metrics.current_loss;
    printf("\n=== Training Results ===\n");
    printf("Initial loss: %.6f\n", initial_loss);
    printf("Final loss: %.6f\n", final_loss);
    printf("Change: %.6f\n", final_loss - initial_loss);
    
    if (final_loss < initial_loss) {
        printf("✅ Loss decreased - Training is working!\n");
    } else if (final_loss == initial_loss) {
        printf("⚠️  Loss unchanged - Training may not be updating weights\n");
    } else {
        printf("❌ Loss increased - Training has issues\n");
    }
    
    // Save trained model with vocabulary
    printf("\n8. Saving trained model...\n");
    cllm_write_model(model, "trained_model.cllm");
    
    // Cleanup
    printf("\n9. Cleaning up...\n");
    cllm_training_free(training);
    cllm_data_loader_free(loader);
    cllm_free_model(model);
    printf("   ✓ Cleanup complete\n");
    
    return 0;
}