/**
 * CLLM Repository Training Program
 * Trains the CLLM model on the entire repository codebase
 * Uses the kissing spheres threading architecture
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
#define CHECKPOINT_DIR "checkpoints"

// Get current time in milliseconds
double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

// Print training statistics
void print_stats(int epoch, int batch, float loss, float grad_norm, 
                 SphereStats* stats, double elapsed_ms) {
    printf("\n[Epoch %d, Batch %d] Loss: %.4f | Grad Norm: %.4f | Time: %.2f ms\n",
           epoch, batch, loss, grad_norm, elapsed_ms);
    
    if (stats) {
        printf("  Sphere Activity:\n");
        int active_count = 0;
        for (int i = 0; i < 12; i++) {
            if (stats->batches_processed[i] > 0) {
                active_count++;
                printf("    Sphere %2d: %4d batches, avg loss: %.4f\n",
                       i, stats->batches_processed[i], stats->avg_loss[i]);
            }
        }
        printf("  Active Spheres: %d/12\n", active_count);
        printf("  Total Batches: %d\n", stats->total_batches);
    }
}

// Save checkpoint
int save_checkpoint(CLLMModel* model, int epoch, float loss) {
    char checkpoint_path[512];
    snprintf(checkpoint_path, sizeof(checkpoint_path), 
             "%s/checkpoint_epoch_%d_loss_%.4f.cllm", CHECKPOINT_DIR, epoch, loss);
    
    printf("\n💾 Saving checkpoint: %s\n", checkpoint_path);
    
    if (cllm_write_model(model, checkpoint_path) == 0) {
        printf("✓ Checkpoint saved successfully\n");
        return 0;
    } else {
        printf("✗ Failed to save checkpoint\n");
        return -1;
    }
}

// Test code generation
void test_generation(CLLMModel* model, const char* prompt) {
    printf("\n🧪 Testing code generation...\n");
    printf("Prompt: &quot;%s&quot;\n", prompt);
    printf("Generated: ");
    
    // Simple greedy generation
    int max_tokens = 50;
    int* context = (int*)malloc(sizeof(int) * 256);
    int context_len = 0;
    
    // Tokenize prompt (simplified - just use character codes)
    for (int i = 0; prompt[i] != '\0' && i < 256; i++) {
        context[context_len++] = (int)prompt[i];
    }
    
    // Generate tokens
    for (int i = 0; i < max_tokens; i++) {
        // Get logits for next token
        float* logits = (float*)malloc(sizeof(float) * model->vocab_size);
        
        // Forward pass (simplified)
        // In real implementation, this would call cllm_forward
        
        // For now, just print a placeholder
        if (i == 0) {
            printf("[Generation would appear here with trained model]");
        }
        
        free(logits);
        break; // Simplified for now
    }
    
    printf("\n");
    free(context);
}

int main(int argc, char** argv) {
    printf("=" * 70);
    printf("\nCLLM Repository Training - Kissing Spheres Architecture\n");
    printf("=" * 70);
    printf("\n\n");
    
    double start_time = get_time_ms();
    
    // Create directories
    system("mkdir -p models");
    system("mkdir -p checkpoints");
    
    // Step 1: Create model
    printf("📦 Creating CLLM model...\n");
    CLLMConfig config = {
        .vocab_size = 50000,
        .d_model = 256,
        .n_heads = 8,
        .n_layers = 6,
        .d_ff = 1024,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "✗ Failed to create model\n");
        return 1;
    }
    printf("✓ Model created: %d params\n", 
           config.vocab_size * config.d_model + 
           config.n_layers * (config.d_model * config.d_model * 4));
    
    // Step 2: Build vocabulary
    printf("\n📚 Building vocabulary from training data...\n");
    if (cllm_build_vocabulary_from_file(model, TRAINING_FILE) != 0) {
        fprintf(stderr, "✗ Failed to build vocabulary\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Vocabulary built: %d tokens\n", model->vocab_size);
    
    // Step 3: Create training configuration
    printf("\n⚙️  Configuring training...\n");
    CLLMTrainingConfig train_config = {
        .num_epochs = 10,
        .batch_size = 32,
        .sequence_length = 128,
        .learning_rate = 0.001f,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 100,
        .save_every = 2,
        .eval_interval = 100,
        .max_steps = 10000,
        
        // Performance optimizations
        .gradient_accumulation_steps = 4,
        .use_mixed_precision = 1,
        .loss_scale = 1024.0f,
        .loss_scale_growth = 2.0f,
        .loss_scale_backoff = 0.5f,
        .loss_scale_window = 2000
    };
    strcpy(train_config.optimizer, "adam");
    
    printf("  Epochs: %d\n", train_config.num_epochs);
    printf("  Batch size: %d\n", train_config.batch_size);
    printf("  Sequence length: %d\n", train_config.sequence_length);
    printf("  Learning rate: %.4f\n", train_config.learning_rate);
    printf("  Gradient accumulation: %d steps\n", train_config.gradient_accumulation_steps);
    printf("  Mixed precision: %s\n", train_config.use_mixed_precision ? "enabled" : "disabled");
    
    // Step 4: Initialize training
    printf("\n🎯 Initializing training system...\n");
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "✗ Failed to initialize training\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Training system initialized\n");
    
    // Step 5: Load training data
    printf("\n📖 Loading training data...\n");
    if (cllm_load_training_data(training, TRAINING_FILE) != 0) {
        fprintf(stderr, "✗ Failed to load training data\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Training data loaded\n");
    
    // Step 6: Create threaded training system
    printf("\n🔮 Creating kissing spheres threading system...\n");
    ThreadedTrainingSystem* threaded_system = threaded_training_create(training, 0);
    if (!threaded_system) {
        fprintf(stderr, "✗ Failed to create threaded training system\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Threaded system created with 12 worker spheres\n");
    
    // Step 7: Train the model
    printf("\n🚀 Starting training...\n");
    printf("=" * 70);
    printf("\n\n");
    
    float best_loss = 1e9;
    int total_batches = 0;
    
    for (int epoch = 0; epoch < train_config.num_epochs; epoch++) {
        printf("\n📊 Epoch %d/%d\n", epoch + 1, train_config.num_epochs);
        printf("-" * 70);
        printf("\n");
        
        double epoch_start = get_time_ms();
        
        // Train one epoch with threading
        float epoch_loss = threaded_train_epoch_lockfree(threaded_system, epoch);
        
        double epoch_time = get_time_ms() - epoch_start;
        
        // Get sphere statistics
        SphereStats stats;
        threaded_training_get_sphere_stats(threaded_system, &stats);
        
        float grad_norm = threaded_training_get_gradient_norm(threaded_system);
        
        // Print statistics
        print_stats(epoch + 1, total_batches, epoch_loss, grad_norm, &stats, epoch_time);
        
        total_batches += stats.total_batches;
        
        // Save checkpoint if best loss
        if (epoch_loss < best_loss) {
            best_loss = epoch_loss;
            save_checkpoint(model, epoch + 1, epoch_loss);
        }
        
        // Save periodic checkpoint
        if ((epoch + 1) % train_config.save_every == 0) {
            save_checkpoint(model, epoch + 1, epoch_loss);
        }
    }
    
    double total_time = get_time_ms() - start_time;
    
    // Step 8: Save final model
    printf("\n\n💾 Saving final trained model...\n");
    if (cllm_write_model(model, MODEL_OUTPUT) == 0) {
        printf("✓ Model saved: %s\n", MODEL_OUTPUT);
    } else {
        fprintf(stderr, "✗ Failed to save model\n");
    }
    
    // Step 9: Test generation
    printf("\n");
    test_generation(model, "int main() {");
    test_generation(model, "// Calculate prime");
    test_generation(model, "void cllm_");
    
    // Step 10: Print final statistics
    printf("\n\n");
    printf("=" * 70);
    printf("\n📈 Training Complete!\n");
    printf("=" * 70);
    printf("\n");
    printf("  Total time: %.2f seconds (%.2f minutes)\n", 
           total_time / 1000.0, total_time / 60000.0);
    printf("  Total batches: %d\n", total_batches);
    printf("  Final loss: %.4f\n", best_loss);
    printf("  Best loss: %.4f\n", best_loss);
    printf("  Model saved: %s\n", MODEL_OUTPUT);
    printf("\n");
    
    // Cleanup
    threaded_training_destroy(threaded_system);
    cllm_training_free(training);
    cllm_free_model(model);
    
    printf("✓ Cleanup complete\n\n");
    
    return 0;
}