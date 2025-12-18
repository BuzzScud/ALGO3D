#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_utils.h"

int main() {
    printf("=== Forward Pass Debug Test ===\n\n");
    
    CLLMModel* model = cllm_create_small_model();
    if (!model) return 1;
    
    uint32_t* tokens = (uint32_t*)malloc(16 * sizeof(uint32_t));
    for (size_t i = 0; i < 16; i++) tokens[i] = (i % 4) + 1;
    
    CLLMTrainingConfig config = {
        .num_epochs = 1, .batch_size = 1, .sequence_length = 4,
        .learning_rate = 0.01, .gradient_accumulation_steps = 1,
        .save_every = 1000, .use_mixed_precision = false
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) return 1;
    training->tokens = tokens;
    training->num_tokens = 16;
    
    uint32_t batch[4] = {1, 2, 3, 4};
    uint32_t target[4] = {2, 3, 4, 1};
    
    printf("Step 0:\n");
    printf("  Embedding[1][0] before forward = %.10f\n", model->embeddings.embeddings[128]);
    cllm_forward_training(training, batch);
    printf("  Logits[0] = %.6f\n", training->logits[0]);
    printf("  Logits[1] = %.6f\n", training->logits[1]);
    float loss0 = cllm_compute_loss(training, batch, target, 4);
    printf("  Loss = %.6f\n", loss0);
    cllm_backward_training(training, target, NULL);
    printf("  Gradient[128] = %.10f\n", training->gradients[128]);
    cllm_optimizer_step(training);
    printf("  Embedding[1][0] after update = %.10f\n\n", model->embeddings.embeddings[128]);
    
    printf("Step 1:\n");
    printf("  Embedding[1][0] before forward = %.10f\n", model->embeddings.embeddings[128]);
    cllm_forward_training(training, batch);
    printf("  Logits[0] = %.6f\n", training->logits[0]);
    printf("  Logits[1] = %.6f\n", training->logits[1]);
    float loss1 = cllm_compute_loss(training, batch, target, 4);
    printf("  Loss = %.6f\n", loss1);
    printf("  Loss change = %.10f\n\n", loss1 - loss0);
    
    cllm_training_cleanup(training);
    free(tokens);
    cllm_free_model(model);
    
    return 0;
}
