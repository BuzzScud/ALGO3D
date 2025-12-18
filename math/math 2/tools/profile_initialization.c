/*
 * profile_initialization.c - Profile application initialization to find real bottleneck
 * 
 * This tool measures time spent in each initialization phase to identify
 * where the actual slowness occurs.
 */

#include "cllm_inference.h"
#include "cllm_pure_crystalline.h"
#include "prime_rainbow.h"
#include <stdio.h>
#include <time.h>

#define MEASURE_START(name) \
    struct timespec start_##name, end_##name; \
    clock_gettime(CLOCK_MONOTONIC, &start_##name); \
    printf("Starting: %s...\n", #name);

#define MEASURE_END(name) \
    clock_gettime(CLOCK_MONOTONIC, &end_##name); \
    double elapsed_##name = (end_##name.tv_sec - start_##name.tv_sec) + \
                            (end_##name.tv_nsec - start_##name.tv_nsec) / 1e9; \
    printf("✓ %s: %.6f seconds (%.2f ms)\n", #name, elapsed_##name, elapsed_##name * 1000.0);

int main(void) {
    printf("=======================================================\n");
    printf("  Application Initialization Profiling\n");
    printf("=======================================================\n\n");
    
    struct timespec total_start, total_end;
    clock_gettime(CLOCK_MONOTONIC, &total_start);
    
    // Phase 1: Rainbow Table Initialization
    MEASURE_START(rainbow_table_init);
    rainbow_table_init();
    MEASURE_END(rainbow_table_init);
    
    // Phase 2: Load Important Primes
    MEASURE_START(load_important_primes);
    int important = rainbow_table_load_important_primes();
    MEASURE_END(load_important_primes);
    printf("  Loaded %d important primes\n\n", important);
    
    // Phase 3: Generate 10k Primes
    MEASURE_START(generate_10k_primes);
    rainbow_table_generate_primes(10000);
    MEASURE_END(generate_10k_primes);
    
    // Phase 4: Generate 100k Primes
    MEASURE_START(generate_100k_primes);
    rainbow_table_generate_primes(90000);  // Additional 90k
    MEASURE_END(generate_100k_primes);
    
    // Phase 5: Create Tiny Model
    MEASURE_START(create_tiny_model);
    CLLMConfig config = {
        .vocab_size = 10000,
        .embedding_dim = 512,
        .num_layers = 6,
        .num_heads = 8,
        .ff_dim = 2048,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    MEASURE_END(create_tiny_model);
    
    if (model) {
        printf("  Model created successfully\n");
        printf("  Vocab size: %lu\n", (unsigned long)model->vocab_size);
        printf("  Embedding dim: %lu\n", (unsigned long)model->embedding_dim);
        printf("  Num layers: %lu\n\n", (unsigned long)model->num_layers);
        
        // Phase 6: Initialize Embeddings
        MEASURE_START(init_embeddings);
        // Embeddings already initialized in create
        MEASURE_END(init_embeddings);
        
        // Phase 7: Initialize Kissing Spheres
        MEASURE_START(init_kissing_spheres);
        // Already initialized in create
        MEASURE_END(init_kissing_spheres);
        
        cllm_free_model(model);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &total_end);
    double total_elapsed = (total_end.tv_sec - total_start.tv_sec) + 
                          (total_end.tv_nsec - total_start.tv_nsec) / 1e9;
    
    printf("\n=======================================================\n");
    printf("  Total Initialization Time: %.6f seconds (%.2f ms)\n", 
           total_elapsed, total_elapsed * 1000.0);
    printf("=======================================================\n");
    
    return 0;
}