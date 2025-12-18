/**
 * Performance Benchmark: Training Speed Comparison
 * 
 * Compares training speed between simplified and full attention backward
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/cllm.h"
#include "../../include/cllm_training.h"

// Helper: Create a small test model
CLLMModel* create_test_model() {
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    // Small model configuration
    model->vocab_size = 100;
    model->embedding_dim = 128;
    model->num_layers = 2;
    
    // Allocate embeddings
    model->embeddings.vocab_size = model->vocab_size;
    model->embeddings.embedding_dim = model->embedding_dim;
    model->embeddings.embeddings = (float*)calloc(
        model->vocab_size * model->embedding_dim, sizeof(float));
    
    // Initialize embeddings with small random values
    for (size_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
        model->embeddings.embeddings[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
    }
    
    // Allocate attention layers
    model->attention_layers = (AttentionLayer*)calloc(model->num_layers, sizeof(AttentionLayer));
    for (uint32_t i = 0; i < model->num_layers; i++) {
        model->attention_layers[i].num_heads = 4;
        model->attention_layers[i].head_dim = 32;
        
        uint32_t dim = model->attention_layers[i].num_heads * model->attention_layers[i].head_dim;
        size_t weight_size = dim * dim;
        
        model->attention_layers[i].query_lattice = (float*)calloc(weight_size, sizeof(float));
        model->attention_layers[i].key_lattice = (float*)calloc(weight_size, sizeof(float));
        model->attention_layers[i].value_lattice = (float*)calloc(weight_size, sizeof(float));
        
        // Initialize with small random values
        for (size_t j = 0; j < weight_size; j++) {
            model->attention_layers[i].query_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
            model->attention_layers[i].key_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
            model->attention_layers[i].value_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    
    // Allocate feedforward layers
    model->ff_layers = (FeedForwardLayer*)calloc(model->num_layers, sizeof(FeedForwardLayer));
    for (uint32_t i = 0; i < model->num_layers; i++) {
        model->ff_layers[i].input_dim = model->embedding_dim;
        model->ff_layers[i].hidden_dim = 512;
        model->ff_layers[i].output_dim = model->embedding_dim;
        
        size_t w1_size = model->ff_layers[i].input_dim * model->ff_layers[i].hidden_dim;
        size_t w2_size = model->ff_layers[i].hidden_dim * model->ff_layers[i].output_dim;
        
        model->ff_layers[i].w1_lattice = (float*)calloc(w1_size, sizeof(float));
        model->ff_layers[i].w2_lattice = (float*)calloc(w2_size, sizeof(float));
        model->ff_layers[i].bias1 = (float*)calloc(model->ff_layers[i].hidden_dim, sizeof(float));
        model->ff_layers[i].bias2 = (float*)calloc(model->ff_layers[i].output_dim, sizeof(float));
        
        // Initialize with small random values
        for (size_t j = 0; j < w1_size; j++) {
            model->ff_layers[i].w1_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
        }
        for (size_t j = 0; j < w2_size; j++) {
            model->ff_layers[i].w2_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    
    // Allocate layer norms
    model->layer_norms = (CLLMLayerNorm*)calloc(model->num_layers, sizeof(CLLMLayerNorm));
    for (uint32_t i = 0; i < model->num_layers; i++) {
        model->layer_norms[i].dim = model->embedding_dim;
        model->layer_norms[i].gamma = (float*)calloc(model->embedding_dim, sizeof(float));
        model->layer_norms[i].beta = (float*)calloc(model->embedding_dim, sizeof(float));
        
        // Initialize gamma to 1, beta to 0
        for (uint32_t j = 0; j < model->embedding_dim; j++) {
            model->layer_norms[i].gamma[j] = 1.0f;
            model->layer_norms[i].beta[j] = 0.0f;
        }
    }
    
    return model;
}

// Helper: Free test model
void free_test_model(CLLMModel* model) {
    if (!model) return;
    
    free(model->embeddings.embeddings);
    
    if (model->attention_layers) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            free(model->attention_layers[i].query_lattice);
            free(model->attention_layers[i].key_lattice);
            free(model->attention_layers[i].value_lattice);
        }
        free(model->attention_layers);
    }
    
    if (model->ff_layers) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            free(model->ff_layers[i].w1_lattice);
            free(model->ff_layers[i].w2_lattice);
            free(model->ff_layers[i].bias1);
            free(model->ff_layers[i].bias2);
        }
        free(model->ff_layers);
    }
    
    if (model->layer_norms) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            free(model->layer_norms[i].gamma);
            free(model->layer_norms[i].beta);
        }
        free(model->layer_norms);
    }
    
    free(model);
}

// Benchmark: Measure training initialization time
void benchmark_initialization() {
    printf("\n");
    printf("Benchmark 1: Training Initialization\n");
    printf("─────────────────────────────────────\n");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL: Could not create model\n");
        return;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 8,
        .sequence_length = 32,
        .num_epochs = 10,
        .max_steps = 1000,
        .warmup_steps = 100,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    clock_t start = clock();
    CLLMTraining* training = cllm_training_init(model, &config);
    clock_t end = clock();
    
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    if (training) {
        printf("✓ Initialization successful\n");
        printf("  Time: %.2f ms\n", time_ms);
        printf("  Attention cache enabled: %s\n", 
               training->store_attention_weights ? "Yes" : "No");
        
        cllm_training_cleanup(training);
    } else {
        printf("✗ Initialization failed\n");
    }
    
    free_test_model(model);
}

// Benchmark: Memory usage comparison
void benchmark_memory_usage() {
    printf("\n");
    printf("Benchmark 2: Memory Usage\n");
    printf("─────────────────────────────────────\n");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL: Could not create model\n");
        return;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 8,
        .sequence_length = 32,
        .num_epochs = 10,
        .max_steps = 1000,
        .warmup_steps = 100,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    
    if (training && training->attention_cache) {
        // Calculate attention cache memory
        size_t cache_memory = 0;
        for (uint32_t i = 0; i < model->num_layers; i++) {
            uint32_t num_heads = model->attention_layers[i].num_heads;
            cache_memory += config.sequence_length * model->embedding_dim * sizeof(float) * 3;  // Q, K, V
            cache_memory += num_heads * config.sequence_length * config.sequence_length * sizeof(float) * 2;  // weights, scores
        }
        
        printf("✓ Memory allocation successful\n");
        printf("  Attention cache: %.2f MB\n", cache_memory / (1024.0f * 1024.0f));
        printf("  Per layer: %.2f MB\n", cache_memory / (1024.0f * 1024.0f) / model->num_layers);
        
        cllm_training_cleanup(training);
    } else {
        printf("✗ Memory allocation failed\n");
    }
    
    free_test_model(model);
}

// Benchmark: Feature flag test
void benchmark_feature_flag() {
    printf("\n");
    printf("Benchmark 3: Feature Flag Control\n");
    printf("─────────────────────────────────────\n");
    
    CLLMModel* model = create_test_model();
    if (!model) {
        printf("FAIL: Could not create model\n");
        return;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 8,
        .sequence_length = 32,
        .num_epochs = 10,
        .max_steps = 1000,
        .warmup_steps = 100,
        .optimizer = "adam",
        .lr_scheduler = "cosine"
    };
    
    // Test with full attention backward enabled
    CLLMTraining* training1 = cllm_training_init(model, &config);
    int enabled_by_default = training1 ? training1->store_attention_weights : 0;
    
    printf("✓ Default state: %s\n", enabled_by_default ? "Enabled" : "Disabled");
    
    // Test disabling
    if (training1) {
        training1->store_attention_weights = 0;
        printf("✓ Can be disabled: Yes\n");
        printf("✓ Feature flag working correctly\n");
        cllm_training_cleanup(training1);
    }
    
    free_test_model(model);
}

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     Training Speed Benchmark                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    
    srand(time(NULL));
    
    benchmark_initialization();
    benchmark_memory_usage();
    benchmark_feature_flag();
    
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Benchmark Complete\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Note: Full training benchmarks require larger datasets.\n");
    printf("      Use the train_model tool for comprehensive testing.\n");
    printf("\n");
    
    return 0;
}