/**
 * Performance Comparison: Clock-Based vs Lattice Formula Initialization
 * 
 * Compares the speed of:
 * 1. Clock-based direct geometry (OPTIMIZED)
 * 2. Full L(n,d,k,λ) formula calculation (SLOW)
 * 3. Random initialization (BASELINE)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cllm.h"
#include "cllm_utils.h"
#include "ai/cllm_lattice_embeddings.h"
#include "ai/cllm_clock_embeddings.h"

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

int main(void) {
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  SPEED TEST: Clock vs Lattice vs Random                 ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    // Test configuration
    uint32_t vocab_size = 1000;
    uint32_t embedding_dim = 128;
    
    printf("Configuration:\n");
    printf("  Vocabulary size: %u\n", vocab_size);
    printf("  Embedding dimension: %u\n", embedding_dim);
    printf("  Total values: %u\n\n", vocab_size * embedding_dim);

    // Create three identical models
    CLLMConfig config = {
        .vocab_size = vocab_size,
        .embedding_dim = embedding_dim,
        .num_layers = 4,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    printf("Creating models...\n");
    CLLMModel* model_random = cllm_create_model(&config);
    CLLMModel* model_clock = cllm_create_model(&config);
    CLLMModel* model_lattice = cllm_create_model(&config);
    
    if (!model_random || !model_clock || !model_lattice) {
        fprintf(stderr, "✗ Failed to create models\n");
        return 1;
    }
    
    // Initialize tokens for all models
    for (int m = 0; m < 3; m++) {
        CLLMModel* model = (m == 0) ? model_random : (m == 1) ? model_clock : model_lattice;
        for (uint32_t i = 0; i < model->vocab_size; i++) {
            model->tokens[i].symmetry_group = i % 12;
            model->tokens[i].prime_encoding = 2 + (i % 100);
        }
    }
    
    printf("✓ Models created and tokens initialized\n\n");

    // Test 1: Random initialization (BASELINE)
    printf("Test 1: Random Initialization (BASELINE)\n");
    double start = get_time_ms();
    for (uint32_t i = 0; i < vocab_size * embedding_dim; i++) {
        model_random->embeddings.embeddings[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
    }
    double random_time = get_time_ms() - start;
    printf("  Time: %.2f ms\n", random_time);
    printf("  Speed: 1.00x (baseline)\n\n");

    // Test 2: Clock-based initialization (OPTIMIZED)
    printf("Test 2: Clock-Based Initialization (OPTIMIZED)\n");
    start = get_time_ms();
    cllm_embeddings_init_clock(model_clock);
    double clock_time = get_time_ms() - start;
    printf("  Time: %.2f ms\n", clock_time);
    printf("  Speed: %.2fx vs random\n", random_time / clock_time);
    printf("  Status: %s\n\n", clock_time < random_time * 10 ? "✓ FAST" : "⚠ SLOW");

    // Test 3: Full lattice formula (SLOW - may take a while)
    printf("Test 3: Full Lattice Formula (SLOW)\n");
    printf("  Warning: This may take 10-30 seconds...\n");
    start = get_time_ms();
    cllm_embeddings_init_lattice(model_lattice);
    double lattice_time = get_time_ms() - start;
    printf("  Time: %.2f ms (%.2f seconds)\n", lattice_time, lattice_time / 1000.0);
    printf("  Speed: %.2fx vs random\n", random_time / lattice_time);
    printf("  Speed: %.2fx vs clock\n\n", clock_time / lattice_time);

    // Summary
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  PERFORMANCE SUMMARY                                     ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  Random:  %.2f ms (baseline)                        ║\n", random_time);
    printf("║  Clock:   %.2f ms (%.1fx)                           ║\n", clock_time, random_time / clock_time);
    printf("║  Lattice: %.2f ms (%.1fx)                          ║\n", lattice_time, random_time / lattice_time);
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  Clock speedup vs Lattice: %.0fx                    ║\n", lattice_time / clock_time);
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    // Verify quality - check that embeddings are reasonable
    printf("Quality Check:\n");
    
    // Check clock embeddings
    int clock_in_range = 1;
    for (uint32_t i = 0; i < vocab_size * embedding_dim; i++) {
        float val = model_clock->embeddings.embeddings[i];
        if (val < -1.0f || val > 1.0f) {
            clock_in_range = 0;
            break;
        }
    }
    printf("  Clock embeddings in [-1,1]: %s\n", clock_in_range ? "✓ YES" : "✗ NO");
    
    // Check lattice embeddings
    int lattice_in_range = 1;
    for (uint32_t i = 0; i < vocab_size * embedding_dim; i++) {
        float val = model_lattice->embeddings.embeddings[i];
        if (val < -1.0f || val > 1.0f) {
            lattice_in_range = 0;
            break;
        }
    }
    printf("  Lattice embeddings in [-1,1]: %s\n\n", lattice_in_range ? "✓ YES" : "✗ NO");

    // Cleanup
    cllm_free_model(model_random);
    cllm_free_model(model_clock);
    cllm_free_model(model_lattice);

    printf("✓ Speed test complete!\n\n");
    printf("RECOMMENDATION: Use clock-based initialization for production.\n");
    printf("  - Maintains geometric structure\n");
    printf("  - %.0fx faster than full formula\n", lattice_time / clock_time);
    printf("  - Still uses crystalline math (RULE 3A compliant)\n\n");

    return 0;
}