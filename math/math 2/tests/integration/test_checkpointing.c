/**
 * Integration Test: Model Checkpointing
 * 
 * Tests checkpoint save/load/resume functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include <unistd.h>
#include "../../include/cllm.h"
#include "../../include/cllm_training.h"
#include "../../include/cllm_production.h"

// Helper: Create a minimal test model
CLLMModel* create_test_model() {
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    // Minimal model configuration
    model->vocab_size = 50;
    model->embedding_dim = 64;
    model->num_layers = 1;
    
    // Allocate embeddings
    model->embeddings.vocab_size = model->vocab_size;
    model->embeddings.embedding_dim = model->embedding_dim;
    model->embeddings.embeddings = (float*)calloc(
        model->vocab_size * model->embedding_dim, sizeof(float));
    
    // Initialize embeddings with known values
    for (size_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
        model->embeddings.embeddings[i] = (float)i * 0.01f;
    }
    
    return model;
}

// Helper: Free test model
void free_test_model(CLLMModel* model) {
    if (!model) return;
    free(model->embeddings.embeddings);
    free(model);
}

// Test 1: Save checkpoint
int test_save_checkpoint() {
    printf("Test 1: Save checkpoint... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 10,
        .max_steps = 100,
        .warmup_steps = 10,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        printf("FAIL (training init failed)\n");
        free_test_model(model);
        return 0;
    }
    
    // Set some training state
    training->current_epoch = 5;
    training->current_step = 50;
    training->current_loss = 2.5f;
    training->best_loss = 2.3f;
    
    // Save checkpoint
    const char* checkpoint_path = "/tmp/test_checkpoint.ckpt";
    int result = save_checkpoint(training, checkpoint_path);
    
    int success = (result == 0);
    
    // Check file exists
    if (success) {
        FILE* f = fopen(checkpoint_path, "rb");
        if (f) {
            fclose(f);
        } else {
            success = 0;
        }
    }
    
    cllm_training_free(training);
    free_test_model(model);
    
    // Cleanup
    unlink(checkpoint_path);
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 2: Load checkpoint
int test_load_checkpoint() {
    printf("Test 2: Load checkpoint... ");
    
    // First, create and save a checkpoint
    CLLMModel* model1 = create_test_model();
    if (!model1) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 10,
        .max_steps = 100,
        .warmup_steps = 10,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training1 = cllm_training_init(model1, &config);
    if (!training1) {
        printf("FAIL (training init failed)\n");
        free_test_model(model1);
        return 0;
    }
    
    // Set some training state
    training1->current_epoch = 5;
    training1->current_step = 50;
    training1->current_loss = 2.5f;
    training1->best_loss = 2.3f;
    
    // Save checkpoint
    const char* checkpoint_path = "/tmp/test_checkpoint_load.ckpt";
    save_checkpoint(training1, checkpoint_path);
    
    // Now create a new training instance and load
    CLLMModel* model2 = create_test_model();
    CLLMTraining* training2 = cllm_training_init(model2, &config);
    
    int result = load_checkpoint(training2, checkpoint_path);
    
    int success = (result == 0);
    
    // Verify loaded state matches saved state
    if (success) {
        if (training2->current_epoch != training1->current_epoch ||
            training2->current_step != training1->current_step ||
            fabsf(training2->current_loss - training1->current_loss) > 0.001f ||
            fabsf(training2->best_loss - training1->best_loss) > 0.001f) {
            success = 0;
        }
    }
    
    cllm_training_free(training1);
    cllm_training_free(training2);
    free_test_model(model1);
    free_test_model(model2);
    
    // Cleanup
    unlink(checkpoint_path);
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 3: Resume training from checkpoint
int test_resume_training() {
    printf("Test 3: Resume training from checkpoint... ");
    
    // Create and train for a few steps
    CLLMModel* model1 = create_test_model();
    if (!model1) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 10,
        .max_steps = 100,
        .warmup_steps = 10,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training1 = cllm_training_init(model1, &config);
    if (!training1) {
        printf("FAIL (training init failed)\n");
        free_test_model(model1);
        return 0;
    }
    
    // Simulate training progress
    training1->current_epoch = 3;
    training1->current_step = 30;
    training1->current_loss = 2.8f;
    training1->best_loss = 2.5f;
    
    // Save checkpoint
    const char* checkpoint_path = "/tmp/test_checkpoint_resume.ckpt";
    save_checkpoint(training1, checkpoint_path);
    
    int initial_epoch = training1->current_epoch;
    int initial_step = training1->current_step;
    
    // Create new training instance and resume
    CLLMModel* model2 = create_test_model();
    CLLMTraining* training2 = cllm_training_init(model2, &config);
    
    load_checkpoint(training2, checkpoint_path);
    
    // Verify we can continue from where we left off
    int success = (training2->current_epoch == initial_epoch &&
                   training2->current_step == initial_step);
    
    cllm_training_free(training1);
    cllm_training_free(training2);
    free_test_model(model1);
    free_test_model(model2);
    
    // Cleanup
    unlink(checkpoint_path);
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 4: Checkpoint with optimizer state
int test_checkpoint_optimizer_state() {
    printf("Test 4: Checkpoint with optimizer state... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 10,
        .max_steps = 100,
        .warmup_steps = 10,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        printf("FAIL (training init failed)\n");
        free_test_model(model);
        return 0;
    }
    
    // Allocate and initialize optimizer state
    size_t optimizer_state_size = model->vocab_size * model->embedding_dim * 2; // momentum + variance
    training->optimizer_state = (float*)calloc(optimizer_state_size, sizeof(float));
    
    // Set some optimizer state values
    for (size_t i = 0; i < optimizer_state_size; i++) {
        training->optimizer_state[i] = (float)i * 0.001f;
    }
    
    // Save checkpoint
    const char* checkpoint_path = "/tmp/test_checkpoint_optimizer.ckpt";
    int result = save_checkpoint(training, checkpoint_path);
    
    int success = (result == 0);
    
    cllm_training_free(training);
    free_test_model(model);
    
    // Cleanup
    unlink(checkpoint_path);
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 5: Multiple checkpoint save/load cycles
int test_multiple_checkpoints() {
    printf("Test 5: Multiple checkpoint save/load cycles... ");
    
    int success = 1;
    
    for (int cycle = 0; cycle < 3; cycle++) {
        CLLMModel* model = create_test_model();
        if (!model) {
            success = 0;
            break;
        }
        
        CLLMTrainingConfig config = {
            .learning_rate = 0.001f,
            .batch_size = 4,
            .sequence_length = 16,
            .num_epochs = 10,
            .max_steps = 100,
            .warmup_steps = 10,
            .optimizer = "adam",
            .lr_scheduler = "cosine"
        };
        
        CLLMTraining* training = cllm_training_init(model, &config);
        if (!training) {
            success = 0;
            free_test_model(model);
            break;
        }
        
        training->current_epoch = cycle + 1;
        training->current_step = (cycle + 1) * 10;
        
        char checkpoint_path[256];
        snprintf(checkpoint_path, sizeof(checkpoint_path), 
                "/tmp/test_checkpoint_cycle_%d.ckpt", cycle);
        
        int result = save_checkpoint(training, checkpoint_path);
        if (result != 0) {
            success = 0;
        }
        
        cllm_training_free(training);
        free_test_model(model);
        
        // Cleanup
        unlink(checkpoint_path);
        
        if (!success) break;
    }
    
    printf("%s (3 cycles)\n", success ? "PASS" : "FAIL");
    return success;
}

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     Model Checkpointing Integration Tests               ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int passed = 0;
    int total = 5;
    
    passed += test_save_checkpoint();
    passed += test_load_checkpoint();
    passed += test_resume_training();
    passed += test_checkpoint_optimizer_state();
    passed += test_multiple_checkpoints();
    
    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("Results: %d/%d tests passed (%.1f%%)\n", passed, total, 
           (float)passed / total * 100.0f);
    printf("══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}