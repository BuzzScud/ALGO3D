/**
 * Integration Test: Learning Rate Scheduling
 * 
 * Tests that learning rate scheduling works correctly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include "../../include/cllm.h"
#include "../../include/cllm_training.h"

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
    
    // Minimal initialization for other components
    model->attention_layers = (AttentionLayer*)calloc(1, sizeof(AttentionLayer));
    model->ff_layers = (FeedForwardLayer*)calloc(1, sizeof(FeedForwardLayer));
    model->layer_norms = (CLLMLayerNorm*)calloc(1, sizeof(CLLMLayerNorm));
    
    return model;
}

void free_test_model(CLLMModel* model) {
    if (!model) return;
    free(model->embeddings.embeddings);
    free(model->attention_layers);
    free(model->ff_layers);
    free(model->layer_norms);
    free(model);
}

// Test 1: Warmup phase
int test_warmup_phase() {
    printf("Test 1: Warmup phase... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 100,
        .warmup_steps = 10,
        .min_lr = 0.0f,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    int success = 0;
    if (training) {
        // Simulate warmup steps
        float prev_lr = 0.0f;
        int increasing = 1;
        
        for (int step = 0; step < config.warmup_steps && increasing; step++) {
            training->current_step = step;
            cllm_update_learning_rate(training);
            float current_lr = training->config.learning_rate;  // Access directly
            
            if (step > 0 && current_lr <= prev_lr) {
                increasing = 0;
            }
            prev_lr = current_lr;
        }
        
        success = increasing;
        cllm_training_cleanup(training);
    }
    
    free_test_model(model);
    
    if (success) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (LR not increasing during warmup)\n");
        return 0;
    }
}

// Test 2: Decay phase
int test_decay_phase() {
    printf("Test 2: Decay phase... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 100,
        .warmup_steps = 10,
        .min_lr = 0.0001f,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    int success = 0;
    if (training) {
        // Skip warmup, test decay
        training->current_step = config.warmup_steps;
        cllm_update_learning_rate(training);
        float lr_after_warmup = training->config.learning_rate;
        
        // Advance several steps
        training->current_step = config.warmup_steps + 20;
        cllm_update_learning_rate(training);
        float lr_after_decay = training->config.learning_rate;
        
        // LR should decrease during decay
        success = (lr_after_decay < lr_after_warmup);
        
        cllm_training_cleanup(training);
    }
    
    free_test_model(model);
    
    if (success) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (LR not decreasing during decay)\n");
        return 0;
    }
}

// Test 3: Different scheduler types
int test_scheduler_types() {
    printf("Test 3: Different scheduler types... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    int all_work = 1;
    
    // Test cosine scheduler
    {
        CLLMTrainingConfig config = {
            .learning_rate = 0.001f,
            .batch_size = 2,
            .sequence_length = 8,
            .num_epochs = 1,
            .max_steps = 100,
            .warmup_steps = 10,
            .min_lr = 0.0001f,
            .optimizer = "adam"
        };
        strncpy(config.lr_scheduler, "cosine", sizeof(config.lr_scheduler) - 1);
        
        CLLMTraining* training = cllm_training_init(model, &config);
        
        if (training) {
            training->current_step = 0;
            cllm_update_learning_rate(training);
            float lr = training->config.learning_rate;
            
            // Check that we get a valid learning rate
            if (lr <= 0.0f || isnan(lr) || isinf(lr)) {
                all_work = 0;
            }
            
            cllm_training_cleanup(training);
        } else {
            all_work = 0;
        }
    }
    
    free_test_model(model);
    
    if (all_work) {
        printf("PASS (cosine scheduler tested)\n");
        return 1;
    } else {
        printf("FAIL (scheduler test failed)\n");
        return 0;
    }
}

// Test 4: Learning rate bounds
int test_lr_bounds() {
    printf("Test 4: Learning rate bounds... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 100,
        .warmup_steps = 10,
        .min_lr = 0.0001f,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    int success = 1;
    if (training) {
        // Test various steps
        for (int step = 0; step < config.max_steps; step += 10) {
            training->current_step = step;
            cllm_update_learning_rate(training);
            float lr = training->config.learning_rate;
            
            // Check bounds
            if (lr < config.min_lr || lr > config.learning_rate) {
                success = 0;
                break;
            }
            
            // Check for NaN/Inf
            if (isnan(lr) || isinf(lr)) {
                success = 0;
                break;
            }
        }
        
        cllm_training_cleanup(training);
    } else {
        success = 0;
    }
    
    free_test_model(model);
    
    if (success) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (LR out of bounds or invalid)\n");
        return 0;
    }
}

// Test 5: Scheduler integration with training
int test_scheduler_integration() {
    printf("Test 5: Scheduler integration with training... ");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL (model creation failed)\n");
        return 0;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 50,
        .warmup_steps = 5,
        .min_lr = 0.0001f,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    int success = 0;
    if (training) {
        // Simulate training steps
        float lrs[10];
        for (int i = 0; i < 10; i++) {
            training->current_step = i * 5;
            cllm_update_learning_rate(training);
            lrs[i] = training->config.learning_rate;
        }
        
        // Check that learning rates change over time
        int changes = 0;
        for (int i = 1; i < 10; i++) {
            if (fabs(lrs[i] - lrs[i-1]) > 1e-6) {
                changes++;
            }
        }
        
        // Should have multiple changes
        success = (changes >= 5);
        
        cllm_training_cleanup(training);
    }
    
    free_test_model(model);
    
    if (success) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (LR not changing properly)\n");
        return 0;
    }
}

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     Learning Rate Scheduling Integration Tests          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int passed = 0;
    int total = 5;
    
    passed += test_warmup_phase();
    passed += test_decay_phase();
    passed += test_scheduler_types();
    passed += test_lr_bounds();
    passed += test_scheduler_integration();
    
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Results: %d/%d tests passed (%.1f%%)\n", passed, total, 
           (float)passed / total * 100.0f);
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}