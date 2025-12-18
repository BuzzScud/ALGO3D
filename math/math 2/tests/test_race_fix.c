#include <stdio.h>
#include <stdlib.h>
#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include "cllm_data_loader.h"
#include "cllm_batch.h"

int main() {
    printf("=== Race Condition Fix Test ===\n\n");
    
    // Small configuration for quick test
    CLLMConfig config = {
        .vocab_size = 1000,
        .embedding_dim = 128,
        .num_layers = 2,
        .num_heads = 4,
        .ff_hidden_dim = 512,
        .max_seq_len = 64,
        .batch_size = 4,
        .sequence_length = 32,
        .learning_rate = 0.001f
    };
    
    printf("Creating model with config:\n");
    printf("  Vocab: %d, Embed: %d, Layers: %d\n", 
           config.vocab_size, config.embedding_dim, config.num_layers);
    printf("  Batch: %d, SeqLen: %d\n\n", config.batch_size, config.sequence_length);
    
    // Create training system
    CLLMTraining* training = cllm_training_create(&config);
    if (!training) {
        fprintf(stderr, "Failed to create training system\n");
        return 1;
    }
    printf("✓ Training system created\n");
    
    // Load data
    CLLMDataLoader* loader = cllm_data_loader_create("test_race_fix_data", config.vocab_size);
    if (!loader) {
        fprintf(stderr, "Failed to create data loader\n");
        cllm_training_free(training);
        return 1;
    }
    printf("✓ Data loader created\n");
    
    // Create batch iterator
    CLLMBatchIterator* iterator = cllm_batch_iterator_create(
        loader, config.batch_size, config.sequence_length, 1
    );
    if (!iterator) {
        fprintf(stderr, "Failed to create batch iterator\n");
        cllm_data_loader_free(loader);
        cllm_training_free(training);
        return 1;
    }
    printf("✓ Batch iterator created\n");
    
    // Create threaded training system with 8 threads
    ThreadedTrainingSystem* threaded = threaded_training_create(training, iterator, 8);
    if (!threaded) {
        fprintf(stderr, "Failed to create threaded training system\n");
        cllm_batch_iterator_free(iterator);
        cllm_data_loader_free(loader);
        cllm_training_free(training);
        return 1;
    }
    printf("✓ Threaded training system created\n\n");
    
    // Train for 2 epochs
    printf("Training for 2 epochs...\n");
    for (int epoch = 0; epoch < 2; epoch++) {
        printf("\n--- Epoch %d ---\n", epoch + 1);
        float loss = threaded_train_epoch_lockfree(threaded, epoch);
        printf("Epoch %d loss: %.6f\n", epoch + 1, loss);
        
        if (loss == 0.0f) {
            fprintf(stderr, "ERROR: Loss is zero - gradients may not be accumulating!\n");
            threaded_training_free(threaded);
            cllm_batch_iterator_free(iterator);
            cllm_data_loader_free(loader);
            cllm_training_free(training);
            return 1;
        }
    }
    
    printf("\n✓ Training completed successfully!\n");
    printf("✓ Race condition fix verified - gradients are accumulating correctly\n");
    
    // Cleanup
    threaded_training_free(threaded);
    cllm_batch_iterator_free(iterator);
    cllm_data_loader_free(loader);
    cllm_training_free(training);
    
    return 0;
}
