/**
 * Simple Training Test Tool
 * 
 * Tests the complete training pipeline on tiny.txt dataset
 * Uses the actual kissing spheres training system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_training_threaded.h"
#include "../include/cllm_batch.h"
#include "../include/cllm_tokenizer.h"
#include "../include/cllm_vocab_builder.h"
#include "../include/cllm_data_loader.h"

void print_banner() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM Training Test - Tiny Dataset                 ║\n");
    printf("║         Testing Complete Pipeline                         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int main(int argc, char** argv) {
    print_banner();
    
    const char* data_file = "data/tiny.txt";
    const char* output_file = "models/test_tiny.cllm";
    
    // Step 1: Build vocabulary
    printf("Step 1: Building vocabulary from %s\n", data_file);
    CLLMVocabBuilder* vocab_builder = cllm_vocab_builder_create(1000);
    if (!vocab_builder) {
        fprintf(stderr, "Failed to create vocab builder\n");
        return 1;
    }
    
    if (cllm_vocab_builder_add_file(vocab_builder, data_file) != 0) {
        fprintf(stderr, "Failed to add file to vocab builder\n");
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    
    cllm_vocab_builder_finalize(vocab_builder);
    printf("  Vocabulary size: %d\n", vocab_builder->vocab_size);
    
    // Step 2: Create model
    printf("\nStep 2: Creating model\n");
    CLLMConfig config = {
        .vocab_size = vocab_builder->vocab_size,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 2,
        .seq_length = 16,
        .dropout_rate = 0.1,
        .ff_hidden_dim = 256
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    
    printf("  Model created successfully\n");
    printf("  Vocab size: %d\n", model->config.vocab_size);
    printf("  Embedding dim: %d\n", model->config.embedding_dim);
    printf("  Layers: %d\n", model->config.num_layers);
    
    // Step 3: Check embeddings
    printf("\nStep 3: Checking initial embeddings\n");
    double sum = 0.0;
    double sum_sq = 0.0;
    int count = 0;
    for (int i = 0; i < 100 && i < model->config.vocab_size * model->config.embedding_dim; i++) {
        double val = model->embeddings.embeddings[i];
        sum += val;
        sum_sq += val * val;
        count++;
    }
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double stddev = sqrt(variance);
    printf("  Embedding statistics (first 100 values):\n");
    printf("    Mean: %.6f\n", mean);
    printf("    StdDev: %.6f\n", stddev);
    
    if (mean == 0.0 && stddev == 0.0) {
        fprintf(stderr, "ERROR: Embeddings are all zero!\n");
        cllm_free_model(model);
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    
    // Step 4: Create tokenizer
    printf("\nStep 4: Creating tokenizer\n");
    CLLMTokenizer* tokenizer = cllm_tokenizer_create(vocab_builder);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer\n");
        cllm_free_model(model);
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    printf("  Tokenizer created\n");
    
    // Step 5: Load and tokenize data
    printf("\nStep 5: Loading and tokenizing data\n");
    FILE* f = fopen(data_file, "r");
    if (!f) {
        fprintf(stderr, "Failed to open data file\n");
        cllm_tokenizer_free(tokenizer);
        cllm_free_model(model);
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* text = malloc(file_size + 1);
    fread(text, 1, file_size, f);
    text[file_size] = '\0';
    fclose(f);
    
    printf("  Loaded %ld bytes\n", file_size);
    printf("  Text: "%s"\n", text);
    
    uint32_t* tokens = NULL;
    size_t num_tokens = 0;
    if (cllm_tokenizer_encode(tokenizer, text, &tokens, &num_tokens) != 0) {
        fprintf(stderr, "Failed to tokenize text\n");
        free(text);
        cllm_tokenizer_free(tokenizer);
        cllm_free_model(model);
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    
    printf("  Tokenized into %zu tokens\n", num_tokens);
    
    // Step 6: Create training config
    printf("\nStep 6: Setting up training\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 2,
        .max_steps = 10,
        .gradient_clip = 1.0f,
        .weight_decay = 0.01f,
        .warmup_steps = 0,
        .save_interval = 5,
        .checkpoint_dir = NULL
    };
    
    printf("  Learning rate: %.6f\n", train_config.learning_rate);
    printf("  Batch size: %d\n", train_config.batch_size);
    printf("  Sequence length: %d\n", train_config.sequence_length);
    printf("  Epochs: %d\n", train_config.num_epochs);
    
    // Step 7: Initialize training
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        free(tokens);
        free(text);
        cllm_tokenizer_free(tokenizer);
        cllm_free_model(model);
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    
    printf("  Training initialized\n");
    
    // Step 8: Create threaded training system
    printf("\nStep 8: Creating kissing spheres training system\n");
    ThreadedTrainingSystem* threaded_system = threaded_training_create(
        training,
        train_config.batch_size,
        train_config.sequence_length,
        0  // auto-detect threads
    );
    
    if (!threaded_system) {
        fprintf(stderr, "Failed to create threaded training system\n");
        cllm_training_free(training);
        free(tokens);
        free(text);
        cllm_tokenizer_free(tokenizer);
        cllm_free_model(model);
        cllm_vocab_builder_free(vocab_builder);
        return 1;
    }
    
    printf("  Threaded system created with %d threads\n", threaded_system->num_threads);
    
    // Step 9: Train for a few steps
    printf("\nStep 9: Training for %d steps\n", train_config.max_steps);
    
    for (int step = 0; step < train_config.max_steps; step++) {
        // Create a simple batch from tokens
        if (num_tokens < train_config.sequence_length + 1) {
            fprintf(stderr, "Not enough tokens for training\n");
            break;
        }
        
        CLLMBatch* batch = cllm_batch_create(train_config.batch_size, train_config.sequence_length);
        if (!batch) {
            fprintf(stderr, "Failed to create batch\n");
            break;
        }
        
        // Fill batch with tokens (simple sequential)
        for (int b = 0; b < train_config.batch_size; b++) {
            int start_idx = (step * train_config.batch_size + b) % (num_tokens - train_config.sequence_length - 1);
            for (int t = 0; t < train_config.sequence_length; t++) {
                batch->input_ids[b * train_config.sequence_length + t] = tokens[start_idx + t];
                batch->target_ids[b * train_config.sequence_length + t] = tokens[start_idx + t + 1];
            }
        }
        
        // Train on batch
        float loss = threaded_train_batch(threaded_system, batch);
        
        printf("  Step %d: loss = %.4f\n", step + 1, loss);
        
        cllm_batch_free(batch);
    }
    
    // Step 10: Save model
    printf("\nStep 10: Saving model to %s\n", output_file);
    if (cllm_save_model(model, output_file) != 0) {
        fprintf(stderr, "Failed to save model\n");
    } else {
        printf("  Model saved successfully\n");
    }
    
    // Step 11: Verify embeddings after training
    printf("\nStep 11: Checking final embeddings\n");
    sum = 0.0;
    sum_sq = 0.0;
    count = 0;
    for (int i = 0; i < 100 && i < model->config.vocab_size * model->config.embedding_dim; i++) {
        double val = model->embeddings.embeddings[i];
        sum += val;
        sum_sq += val * val;
        count++;
    }
    mean = sum / count;
    variance = (sum_sq / count) - (mean * mean);
    stddev = sqrt(variance);
    printf("  Final embedding statistics:\n");
    printf("    Mean: %.6f\n", mean);
    printf("    StdDev: %.6f\n", stddev);
    
    // Cleanup
    threaded_training_free(threaded_system);
    cllm_training_free(training);
    free(tokens);
    free(text);
    cllm_tokenizer_free(tokenizer);
    cllm_free_model(model);
    cllm_vocab_builder_free(vocab_builder);
    
    printf("\n=== Training test complete ===\n\n");
    return 0;
}
