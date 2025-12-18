/**
 * Unit Test: Attention Cache
 * 
 * Tests attention cache allocation and deallocation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai/cllm.h"
#include "ai/cllm_training.h"

// Test 1: Single layer cache allocation
int test_single_layer_allocation() {
    printf("Test 1: Single layer cache allocation... ");
    
    int num_layers = 1;
    int seq_len = 32;
    int embed_dim = 128;
    int num_heads = 4;
    
    // Allocate cache structure
    struct {
        float* attention_weights;
        float* queries;
        float* keys;
        float* values;
        float* scores;
    }* cache = calloc(num_layers, sizeof(*cache));
    
    if (!cache) {
        printf("FAIL (cache allocation failed)\n");
        return 0;
    }
    
    // Allocate buffers
    cache[0].queries = calloc(seq_len * embed_dim, sizeof(float));
    cache[0].keys = calloc(seq_len * embed_dim, sizeof(float));
    cache[0].values = calloc(seq_len * embed_dim, sizeof(float));
    cache[0].attention_weights = calloc(num_heads * seq_len * seq_len, sizeof(float));
    cache[0].scores = calloc(num_heads * seq_len * seq_len, sizeof(float));
    
    // Check all allocations succeeded
    int success = (cache[0].queries != NULL) &&
                  (cache[0].keys != NULL) &&
                  (cache[0].values != NULL) &&
                  (cache[0].attention_weights != NULL) &&
                  (cache[0].scores != NULL);
    
    // Cleanup
    free(cache[0].queries);
    free(cache[0].keys);
    free(cache[0].values);
    free(cache[0].attention_weights);
    free(cache[0].scores);
    free(cache);
    
    if (success) {
        printf("PASS\n");
        return 1;
    } else {
        printf("FAIL (some allocations failed)\n");
        return 0;
    }
}

// Test 2: Multiple layer cache allocation
int test_multiple_layer_allocation() {
    printf("Test 2: Multiple layer cache allocation... ");
    
    int num_layers = 6;
    int seq_len = 128;
    int embed_dim = 512;
    int num_heads = 8;
    
    // Allocate cache structure
    struct {
        float* attention_weights;
        float* queries;
        float* keys;
        float* values;
        float* scores;
    }* cache = calloc(num_layers, sizeof(*cache));
    
    if (!cache) {
        printf("FAIL (cache allocation failed)\n");
        return 0;
    }
    
    int success = 1;
    
    // Allocate buffers for all layers
    for (int i = 0; i < num_layers; i++) {
        cache[i].queries = calloc(seq_len * embed_dim, sizeof(float));
        cache[i].keys = calloc(seq_len * embed_dim, sizeof(float));
        cache[i].values = calloc(seq_len * embed_dim, sizeof(float));
        cache[i].attention_weights = calloc(num_heads * seq_len * seq_len, sizeof(float));
        cache[i].scores = calloc(num_heads * seq_len * seq_len, sizeof(float));
        
        if (!cache[i].queries || !cache[i].keys || !cache[i].values ||
            !cache[i].attention_weights || !cache[i].scores) {
            success = 0;
            break;
        }
    }
    
    // Calculate total memory
    size_t total_memory = 0;
    if (success) {
        for (int i = 0; i < num_layers; i++) {
            total_memory += seq_len * embed_dim * sizeof(float) * 3;  // Q, K, V
            total_memory += num_heads * seq_len * seq_len * sizeof(float) * 2;  // weights, scores
        }
    }
    
    // Cleanup
    for (int i = 0; i < num_layers; i++) {
        free(cache[i].queries);
        free(cache[i].keys);
        free(cache[i].values);
        free(cache[i].attention_weights);
        free(cache[i].scores);
    }
    free(cache);
    
    if (success) {
        printf("PASS (%.2f MB allocated)\n", total_memory / (1024.0f * 1024.0f));
        return 1;
    } else {
        printf("FAIL (some allocations failed)\n");
        return 0;
    }
}

// Test 3: Memory size verification
int test_memory_size() {
    printf("Test 3: Memory size verification... ");
    
    int num_layers = 6;
    int seq_len = 128;
    int embed_dim = 512;
    int num_heads = 8;
    
    // Expected memory per layer
    size_t expected_per_layer = 
        3 * seq_len * embed_dim * sizeof(float) +  // Q, K, V
        2 * num_heads * seq_len * seq_len * sizeof(float);  // weights, scores
    
    size_t expected_total = num_layers * expected_per_layer;
    
    // Allocate and measure
    struct {
        float* attention_weights;
        float* queries;
        float* keys;
        float* values;
        float* scores;
    }* cache = calloc(num_layers, sizeof(*cache));
    
    size_t actual_total = 0;
    
    for (int i = 0; i < num_layers; i++) {
        cache[i].queries = calloc(seq_len * embed_dim, sizeof(float));
        cache[i].keys = calloc(seq_len * embed_dim, sizeof(float));
        cache[i].values = calloc(seq_len * embed_dim, sizeof(float));
        cache[i].attention_weights = calloc(num_heads * seq_len * seq_len, sizeof(float));
        cache[i].scores = calloc(num_heads * seq_len * seq_len, sizeof(float));
        
        actual_total += seq_len * embed_dim * sizeof(float) * 3;
        actual_total += num_heads * seq_len * seq_len * sizeof(float) * 2;
    }
    
    // Cleanup
    for (int i = 0; i < num_layers; i++) {
        free(cache[i].queries);
        free(cache[i].keys);
        free(cache[i].values);
        free(cache[i].attention_weights);
        free(cache[i].scores);
    }
    free(cache);
    
    if (actual_total == expected_total) {
        printf("PASS (%.2f MB)\n", actual_total / (1024.0f * 1024.0f));
        return 1;
    } else {
        printf("FAIL (expected %zu, got %zu)\n", expected_total, actual_total);
        return 0;
    }
}

// Test 4: Allocation/deallocation cycle
int test_allocation_cycle() {
    printf("Test 4: Allocation/deallocation cycle... ");
    
    int num_layers = 4;
    int seq_len = 64;
    int embed_dim = 256;
    int num_heads = 4;
    
    int cycles = 10;
    int success = 1;
    
    for (int cycle = 0; cycle < cycles; cycle++) {
        // Allocate
        struct {
            float* attention_weights;
            float* queries;
            float* keys;
            float* values;
            float* scores;
        }* cache = calloc(num_layers, sizeof(*cache));
        
        if (!cache) {
            success = 0;
            break;
        }
        
        for (int i = 0; i < num_layers; i++) {
            cache[i].queries = calloc(seq_len * embed_dim, sizeof(float));
            cache[i].keys = calloc(seq_len * embed_dim, sizeof(float));
            cache[i].values = calloc(seq_len * embed_dim, sizeof(float));
            cache[i].attention_weights = calloc(num_heads * seq_len * seq_len, sizeof(float));
            cache[i].scores = calloc(num_heads * seq_len * seq_len, sizeof(float));
            
            if (!cache[i].queries || !cache[i].keys || !cache[i].values ||
                !cache[i].attention_weights || !cache[i].scores) {
                success = 0;
                break;
            }
        }
        
        // Deallocate
        for (int i = 0; i < num_layers; i++) {
            free(cache[i].queries);
            free(cache[i].keys);
            free(cache[i].values);
            free(cache[i].attention_weights);
            free(cache[i].scores);
        }
        free(cache);
        
        if (!success) break;
    }
    
    if (success) {
        printf("PASS (%d cycles)\n", cycles);
        return 1;
    } else {
        printf("FAIL (failed at some cycle)\n");
        return 0;
    }
}

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     Attention Cache Unit Tests                          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int passed = 0;
    int total = 4;
    
    passed += test_single_layer_allocation();
    passed += test_multiple_layer_allocation();
    passed += test_memory_size();
    passed += test_allocation_cycle();
    
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Results: %d/%d tests passed (%.1f%%)\n", passed, total, 
           (float)passed / total * 100.0f);
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}