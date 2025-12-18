/**
 * Test Hierarchical Training System
 * 
 * Simple test to verify the hierarchical training system works correctly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm_training.h"
#include "cllm_utils.h"
#include "cllm_data_loader.h"
#include "cllm_batch.h"
#include "cllm_tokenizer.h"
#include "ai/cllm_hierarchical_training.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <data_dir>\n", argv[0]);
        return 1;
    }
    
    const char* data_dir = argv[1];
    
    printf("=== Hierarchical Training System Test ===\n\n");
    
    // Step 1: Create tokenizer and load data
    printf("Step 1: Loading data from %s\n", data_dir);
    
    CLLMTokenizer* tokenizer = cllm_tokenizer_create(1000);  // vocab_size
    if (!tokenizer) {
        fprintf(stderr, "ERROR: Failed to create tokenizer\n");
        return 1;
    }
    
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        fprintf(stderr, "ERROR: Failed to create data loader\n");
        cllm_tokenizer_free(tokenizer);
        return 1;
    }
    
    if (cllm_data_loader_load_directory(loader, data_dir) != 0) {
        fprintf(stderr, "ERROR: Failed to load data\n");
        cllm_data_loader_free(loader);
        cllm_tokenizer_free(tokenizer);
        return 1;
    }
    
    printf("✓ Loaded data\n\n");
    
    // Step 2: Create model
    printf("Step 2: Creating model\n");
    CLLMConfig config = {
        .vocab_size = 1000,
        .embedding_dim = 128,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_len = 64,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "ERROR: Failed to create model\n");
        cllm_data_loader_free(loader);
        return 1;
    }
    
    // Set symmetry order in header
    model->header.symmetry_order = 12;
    model->header.num_heads = config.num_heads;
    
    printf("✓ Model created (vocab=%d, embed=%d, layers=%d)\n\n",
           config.vocab_size, config.embedding_dim, config.num_layers);
    
    // Step 3: Create training configuration
    printf("Step 3: Creating training configuration\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 8,
        .sequence_length = 64,
        .num_epochs = 1,
        .max_steps = 100,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 10,
        .save_interval = 1000,
        .eval_interval = 100
    };
    strcpy(train_config.optimizer, "adam");
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "ERROR: Failed to create training\n");
        cllm_free_model(model);
        cllm_data_loader_free(loader);
        return 1;
    }
    
    printf("✓ Training configuration created\n\n");
    
    // Step 4: Create batch iterator
    printf("Step 4: Creating batch iterator\n");
    
    // Get tokens from loader
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset || dataset->num_tokens == 0) {
        fprintf(stderr, "ERROR: Failed to create dataset\n");
        cllm_training_free(training);
        cllm_free_model(model);
        cllm_data_loader_free(loader);
        return 1;
    }
    
    uint32_t* tokens = dataset->tokens;
    size_t num_tokens = dataset->num_tokens;
    
    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
        tokens,
        num_tokens,
        train_config.batch_size,
        train_config.sequence_length,
        0,  // shuffle
        1   // drop_last
    );
    
    if (!batch_iterator) {
        fprintf(stderr, "ERROR: Failed to create batch iterator\n");
        cllm_token_dataset_free(dataset);
        cllm_training_free(training);
        cllm_free_model(model);
        cllm_data_loader_free(loader);
        return 1;
    }
    
    size_t num_batches = cllm_batch_iterator_num_batches(batch_iterator);
    printf("✓ Batch iterator created (%zu batches)\n\n", num_batches);
    
    // Step 5: Create hierarchical training system
    printf("Step 5: Creating hierarchical training system\n");
    int num_threads = 16;  // Small test with 16 threads
    
    HierarchicalTrainingSystem* hier_system = hierarchical_training_create(
        training,
        num_threads,
        batch_iterator
    );
    
    if (!hier_system) {
        fprintf(stderr, "ERROR: Failed to create hierarchical training system\n");
        cllm_batch_iterator_free(batch_iterator);
        cllm_training_free(training);
        cllm_free_model(model);
        cllm_data_loader_free(loader);
        return 1;
    }
    
    printf("✓ Hierarchical training system created\n\n");
    
    // Step 6: Train one epoch
    printf("Step 6: Training one epoch\n");
    float loss = hierarchical_train_epoch(hier_system);
    
    printf("✓ Epoch complete - Loss: %.4f\n\n", loss);
    
    // Step 7: Cleanup
    printf("Step 7: Cleanup\n");
    hierarchical_training_free(hier_system);
    cllm_batch_iterator_free(batch_iterator);
    cllm_token_dataset_free(dataset);
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_data_loader_free(loader);
    cllm_tokenizer_free(tokenizer);
    
    printf("✓ Cleanup complete\n\n");
    printf("=== Test Complete ===\n");
    
    return 0;
}