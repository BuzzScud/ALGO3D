/**
 * Repository Training Test
 * Tests the complete training pipeline with real repository data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include "cllm_batch.h"
#include "cllm_vocab_builder.h"

#define TRAINING_FILE "training_data/repo_code.txt"
#define MODEL_OUTPUT "models/repo_trained_model.cllm"

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

int main() {
    printf("========================================\n");
    printf("CLLM Repository Training Test\n");
    printf("========================================\n\n");
    
    double start_time = get_time_ms();
    
    // Create model
    printf("Creating model...\n");
    CLLMConfig config_model = {
        .vocab_size = 10000,
        .embedding_dim = 256,
        .num_layers = 4,
        .num_heads = 8,
        .ff_dim = 1024,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config_model);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // Build vocabulary
    printf("Building vocabulary from %s...\n", TRAINING_FILE);
    if (cllm_build_vocabulary_from_file(model, TRAINING_FILE) != 0) {
        fprintf(stderr, "Failed to build vocabulary\n");
        cllm_free(model);
        return 1;
    }
    printf("✓ Vocabulary built: %lu tokens\n\n", model->header.vocab_size);
    
    // Create training config
    printf("Configuring training...\n");
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 8,  // Smaller for testing
        .num_epochs = 3,  // Fewer epochs for testing
        .max_steps = 1000,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 50,
        .save_every = 1,
        .eval_interval = 50,
        .sequence_length = 64,  // Shorter sequences for testing
        .gradient_accumulation_steps = 2,
        .use_mixed_precision = 0,  // Disable for stability
    };
    strcpy(config.optimizer, "adam");
    strcpy(config.lr_scheduler, "linear");
    
    printf("  Epochs: %d\n", config.num_epochs);
    printf("  Batch size: %d\n", config.batch_size);
    printf("  Sequence length: %d\n", config.sequence_length);
    printf("  Learning rate: %.4f\n", config.learning_rate);
    printf("✓ Configuration ready\n\n");
    
    // Initialize training
    printf("Initializing training system...\n");
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        cllm_free(model);
        return 1;
    }
    printf("✓ Training system initialized\n\n");
    
    // Load training data
    printf("Loading training data...\n");
    if (cllm_load_training_data(training, TRAINING_FILE) != 0) {
        fprintf(stderr, "Failed to load training data\n");
        cllm_training_free(training);
        cllm_free(model);
        return 1;
    }
    printf("✓ Training data loaded: %zu tokens\n\n", training->num_tokens);
    
    // Create batch iterator
    printf("Creating batch iterator...\n");
    CLLMBatchIterator* iterator = cllm_batch_iterator_create(
        training->tokens,
        training->num_tokens,
        config.batch_size,
        config.sequence_length,
        1,  // shuffle
        0   // drop_last
    );
    if (!iterator) {
        fprintf(stderr, "Failed to create batch iterator\n");
        cllm_training_free(training);
        cllm_free(model);
        return 1;
    }
    printf("✓ Batch iterator created\n\n");
    
    // Create threaded training system
    printf("Creating kissing spheres threading system...\n");
    ThreadedTrainingSystem* threaded = threaded_training_create(training, iterator, 0);
    if (!threaded) {
        fprintf(stderr, "Failed to create threaded training system\n");
        cllm_batch_iterator_free(iterator);
        cllm_training_free(training);
        cllm_free(model);
        return 1;
    }
    printf("✓ Threaded system created with 12 worker spheres\n\n");
    
    // Training loop
    printf("========================================\n");
    printf("Starting Training\n");
    printf("========================================\n\n");
    
    float best_loss = 1e9;
    
    for (int epoch = 0; epoch < config.num_epochs; epoch++) {
        printf("Epoch %d/%d\n", epoch + 1, config.num_epochs);
        printf("----------------------------------------\n");
        
        double epoch_start = get_time_ms();
        
        // Train one epoch
        float epoch_loss = threaded_train_epoch_lockfree(threaded, epoch);
        
        double epoch_time = get_time_ms() - epoch_start;
        
        // Get statistics
        float grad_norm = threaded_training_get_gradient_norm(threaded);
        
        printf("  Loss: %.4f\n", epoch_loss);
        printf("  Gradient Norm: %.4f\n", grad_norm);
        printf("  Time: %.2f seconds\n", epoch_time / 1000.0);
        
        // Print per-sphere stats
        printf("  Sphere Activity:\n");
        for (int i = 0; i < 12; i++) {
            int batches = 0;
            float avg_loss = 0.0f;
            if (threaded_training_get_sphere_stats(threaded, i, &batches, &avg_loss) == 0) {
                if (batches > 0) {
                    printf("    Sphere %2d: %4d batches, loss: %.4f\n", i, batches, avg_loss);
                }
            }
        }
        
        if (epoch_loss < best_loss) {
            best_loss = epoch_loss;
            printf("  ✓ New best loss!\n");
        }
        
        printf("\n");
    }
    
    double total_time = get_time_ms() - start_time;
    
    // Save model
    printf("========================================\n");
    printf("Saving Model\n");
    printf("========================================\n\n");
    
    system("mkdir -p models");
    if (cllm_write_model(model, MODEL_OUTPUT) == 0) {
        printf("✓ Model saved: %s\n\n", MODEL_OUTPUT);
    } else {
        fprintf(stderr, "✗ Failed to save model\n\n");
    }
    
    // Print summary
    printf("========================================\n");
    printf("Training Complete!\n");
    printf("========================================\n\n");
    printf("  Total time: %.2f seconds (%.2f minutes)\n", 
           total_time / 1000.0, total_time / 60000.0);
    printf("  Best loss: %.4f\n", best_loss);
    printf("  Model: %s\n", MODEL_OUTPUT);
    printf("\n");
    
    // Cleanup
    threaded_training_free(threaded);
    cllm_batch_iterator_free(iterator);
    cllm_training_free(training);
    cllm_free(model);
    
    printf("✓ Test complete\n\n");
    
    return 0;
}