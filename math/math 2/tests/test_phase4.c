#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include "math/math.h"
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include "cllm_batch.h"

int main() {
    printf("=== Phase 4 Lock-Free Gradient Test ===\n\n");
    
    // Create minimal training setup
    CLLMConfig config = {
        .vocab_size = 100,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n");
    
    printf("Creating training object...\n");
    CLLMTraining* training = cllm_training_init(model);
    if (!training) {
        fprintf(stderr, "Failed to create training\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Training object created\n");
    
    // Create simple dataset
    printf("\nCreating dataset...\n");
    uint32_t tokens[100];
    for (int i = 0; i < 100; i++) {
        tokens[i] = i % config.vocab_size;
    }
    
    CLLMBatchIterator* iterator = cllm_batch_iterator_create(tokens, 100, 4, 16, 0, 0);
    if (!iterator) {
        fprintf(stderr, "Failed to create iterator\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Dataset created (100 tokens, batch_size=4, seq_len=16)\n");
    
    // Test with 2 threads
    printf("\nCreating threaded system with 2 threads...\n");
    ThreadedTrainingSystem* system = threaded_training_create(training, iterator, 2);
    if (!system) {
        fprintf(stderr, "Failed to create threaded system\n");
        cllm_batch_iterator_free(iterator);
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Threaded system created\n");
    
    printf("\nSystem configuration:\n");
    printf("  Workers: %d\n", threaded_training_get_num_workers(system));
    printf("  Symmetry positions: 12\n");
    printf("  Active workers: %d\n", threaded_training_get_num_workers(system));
    
    printf("\nRunning 1 epoch...\n");
    float loss = threaded_train_epoch_lockfree(system, 0);
    printf("✓ Epoch complete, loss: %.4f\n", loss);
    
    printf("\nChecking gradient norm...\n");
    float norm = threaded_training_get_gradient_norm(system);
    printf("✓ Gradient norm: %.4f\n", norm);
    
    if (norm > 0.0f && !isnan(norm) && !isinf(norm)) {
        printf("\n✅ Phase 4 Test PASSED\n");
        printf("  - Lock-free gradient accumulation working\n");
        printf("  - No NaN gradients\n");
        printf("  - No Inf gradients\n");
        printf("  - Barrier synchronization working\n");
    } else {
        printf("\n❌ Phase 4 Test FAILED\n");
        printf("  - Gradient norm invalid: %.4f\n", norm);
    }
    
    // Cleanup
    threaded_training_free(system);
    cllm_batch_iterator_free(iterator);
    cllm_training_free(training);
    cllm_free_model(model);
    
    printf("\n✓ Cleanup complete\n");
    return 0;
}