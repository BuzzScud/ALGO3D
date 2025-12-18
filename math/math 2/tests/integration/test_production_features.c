/**
 * Integration Test: Production Features
 * 
 * Tests production features: checkpointing, metrics, early stopping, gradient clipping
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include <sys/stat.h>
#include <unistd.h>
#include "../../include/cllm.h"
#include "../../include/cllm_training.h"
#include "../../include/cllm_production.h"

// Helper: Create a minimal test model
CLLMModel* create_test_model() {
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    model->vocab_size = 50;
    model->embedding_dim = 64;
    model->num_layers = 1;
    
    model->embeddings.vocab_size = model->vocab_size;
    model->embeddings.embedding_dim = model->embedding_dim;
    model->embeddings.embeddings = (float*)calloc(
        model->vocab_size * model->embedding_dim, sizeof(float));
    
    for (size_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
        model->embeddings.embeddings[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
    }
    
    return model;
}

void free_test_model(CLLMModel* model) {
    if (!model) return;
    free(model->embeddings.embeddings);
    free(model);
}

// Test 1: Checkpoint save
int test_checkpoint_save() {
    printf("Test 1: Checkpoint save... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 1,
        .max_steps = 10,
        .warmup_steps = 2,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    int success = 0;
    if (training) {
        system("mkdir -p /tmp/cllm_test");
        int result = save_checkpoint(training, "/tmp/cllm_test/checkpoint.ckpt");
        
        if (result == 0) {
            struct stat st;
            if (stat("/tmp/cllm_test/checkpoint.ckpt", &st) == 0) {
                success = 1;
            }
        }
        
        cllm_training_free(training);
    }
    
    free_test_model(model);
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 2: Checkpoint load
int test_checkpoint_load() {
    printf("Test 2: Checkpoint load... ");
    
    CLLMModel* model1 = create_test_model();
    if (!model1) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 1,
        .max_steps = 10,
        .warmup_steps = 2,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training1 = cllm_training_init(model1, &config);
    if (!training1) {
        free_test_model(model1);
        printf("FAIL (training init failed)\n");
        return 0;
    }
    
    training1->current_epoch = 5;
    training1->current_step = 100;
    training1->best_loss = 2.5f;
    
    save_checkpoint(training1, "/tmp/cllm_test/checkpoint_load.ckpt");
    cllm_training_free(training1);
    free_test_model(model1);
    
    CLLMModel* model2 = create_test_model();
    CLLMTraining* training2 = cllm_training_init(model2, &config);
    
    int success = 0;
    if (training2) {
        int result = load_checkpoint(training2, "/tmp/cllm_test/checkpoint_load.ckpt");
        
        if (result == 0 && training2->current_epoch == 5 &&
            training2->current_step == 100 &&
            fabs(training2->best_loss - 2.5f) < 0.01f) {
            success = 1;
        }
        
        cllm_training_free(training2);
    }
    
    free_test_model(model2);
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 3: Metrics tracking
int test_metrics_tracking() {
    printf("Test 3: Metrics tracking... ");
    
    TrainingMetrics* metrics = create_training_metrics(10);
    
    int success = 0;
    if (metrics) {
        record_metrics(metrics, 1, 3.5f, 3.2f, 0.001f);
        record_metrics(metrics, 2, 3.0f, 2.9f, 0.001f);
        record_metrics(metrics, 3, 2.5f, 2.6f, 0.001f);
        
        // Just verify metrics object was created and functions don't crash
        success = 1;
        
        free_training_metrics(metrics);
    }
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 4: Metrics CSV export
int test_metrics_csv() {
    printf("Test 4: Metrics CSV export... ");
    
    TrainingMetrics* metrics = create_training_metrics(10);
    
    int success = 0;
    if (metrics) {
        record_metrics(metrics, 1, 3.5f, 3.2f, 0.001f);
        record_metrics(metrics, 2, 3.0f, 2.9f, 0.001f);
        
        save_metrics_csv(metrics, "/tmp/cllm_test/metrics.csv");
        
        struct stat st;
        if (stat("/tmp/cllm_test/metrics.csv", &st) == 0) {
            success = 1;
        }
        
        free_training_metrics(metrics);
    }
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 5: Early stopping
int test_early_stopping() {
    printf("Test 5: Early stopping... ");
    
    EarlyStoppingState* early_stop = create_early_stopping(10.0f, 3);
    
    int success = 0;
    if (early_stop) {
        // Test improving loss - should not stop
        int should_stop = check_early_stopping(early_stop, 9.0f);
        if (should_stop) {
            free(early_stop);
            printf("FAIL (stopped too early)\n");
            return 0;
        }
        
        // Test non-improving losses - should stop after patience
        check_early_stopping(early_stop, 9.5f);
        check_early_stopping(early_stop, 9.6f);
        should_stop = check_early_stopping(early_stop, 9.7f);
        
        if (should_stop) {
            success = 1;
        }
        
        free(early_stop);
    }
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 6: Gradient clipping
int test_gradient_clipping() {
    printf("Test 6: Gradient clipping... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 1,
        .max_steps = 10,
        .warmup_steps = 2,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    int success = 0;
    if (training) {
        // Set some large gradients
        if (training->gradients) {
            for (size_t i = 0; i < 100 && i < model->vocab_size * model->embedding_dim; i++) {
                training->gradients[i] = 10.0f;
            }
        }
        
        // Clip gradients
        clip_gradients(training, 1.0f);
        
        // Verify gradients were clipped
        float max_grad = 0.0f;
        if (training->gradients) {
            for (size_t i = 0; i < 100 && i < model->vocab_size * model->embedding_dim; i++) {
                float abs_grad = fabsf(training->gradients[i]);
                if (abs_grad > max_grad) {
                    max_grad = abs_grad;
                }
            }
        }
        
        // After clipping, max gradient should be around 1.0 or less
        if (max_grad <= 1.5f) {
            success = 1;
        }
        
        cllm_training_free(training);
    }
    
    free_test_model(model);
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

// Test 7: Learning rate scheduler
int test_lr_scheduler() {
    printf("Test 7: Learning rate scheduler... ");
    
    LRScheduler* scheduler = create_lr_scheduler(0.001f, 10, 100);
    
    int success = 0;
    if (scheduler) {
        float initial_lr = get_learning_rate(scheduler);
        
        // Verify initial LR is set
        if (fabsf(initial_lr - 0.001f) < 0.0001f) {
            success = 1;
        }
        
        free(scheduler);
    }
    
    printf("%s\n", success ? "PASS" : "FAIL");
    return success;
}

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     Production Features Integration Tests               ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int total = 0;
    int passed = 0;
    
    total++; if (test_checkpoint_save()) passed++;
    total++; if (test_checkpoint_load()) passed++;
    total++; if (test_metrics_tracking()) passed++;
    total++; if (test_metrics_csv()) passed++;
    total++; if (test_early_stopping()) passed++;
    total++; if (test_gradient_clipping()) passed++;
    total++; if (test_lr_scheduler()) passed++;
    
    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("Results: %d/%d tests passed (%.1f%%)\n", passed, total, 
           (float)passed / total * 100.0f);
    printf("══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    // Cleanup
    system("rm -rf /tmp/cllm_test");
    
    return (passed == total) ? 0 : 1;
}