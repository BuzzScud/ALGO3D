/*
 * test_simple_init.c - Simple test to find where initialization hangs
 */

#include "prime_rainbow.h"
#include "cllm_utils.h"
#include <stdio.h>
#include <time.h>

int main(void) {
    printf("Test 1: Starting...\n");
    fflush(stdout);
    
    printf("Test 2: About to create model config...\n");
    fflush(stdout);
    
    CLLMConfig config = {
        .vocab_size = 100,  // Very small for testing
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 256,
        .max_seq_len = 128,
        .dropout = 0.1f
    };
    
    printf("Test 3: Config created, about to call cllm_create_model...\n");
    fflush(stdout);
    
    CLLMModel* model = cllm_create_model(&config);
    
    printf("Test 4: Model creation returned\n");
    fflush(stdout);
    
    if (model) {
        printf("Test 5: Model created successfully\n");
        cllm_free_model(model);
    } else {
        printf("Test 5: Model creation failed\n");
    }
    
    return 0;
}