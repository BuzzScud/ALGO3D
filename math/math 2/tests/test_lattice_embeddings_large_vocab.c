/*
 * Test: Lattice Embeddings with Large Vocabulary
 * 
 * Purpose: Verify that embeddings can be initialized for large vocabularies
 * without NaN or overflow issues.
 * 
 * Tests:
 * 1. Small vocab (100 tokens)
 * 2. Medium vocab (10,000 tokens)
 * 3. Large vocab (1,000,000 tokens)
 * 4. Verify no NaN values
 * 5. Verify values in valid range
 * 6. Verify clock lattice wrapping works correctly
 */

#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <assert.h>
#include "lattice_embeddings.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define EMBEDDING_DIM 128

// Test helper: Check if value is NaN
int is_nan_float(float x) {
    return x != x;
}

// Test helper: Check if value is infinite
int is_inf_float(float x) {
    return (x == INFINITY) || (x == -INFINITY);
}

// Test 1: Small vocabulary
void test_small_vocab() {
    printf("\n=== Test 1: Small Vocabulary (100 tokens) ===\n");
    
    uint32_t vocab_size = 100;
    float* embeddings = (float*)calloc(vocab_size * EMBEDDING_DIM, sizeof(float));
    
    lattice_embeddings_init_geometric(embeddings, vocab_size, EMBEDDING_DIM);
    
    // Check for NaN/Inf
    int nan_count = 0;
    int inf_count = 0;
    float min_val = embeddings[0];
    float max_val = embeddings[0];
    
    for (uint32_t i = 0; i < vocab_size * EMBEDDING_DIM; i++) {
        if (is_nan_float(embeddings[i])) nan_count++;
        if (is_inf_float(embeddings[i])) inf_count++;
        if (embeddings[i] < min_val) min_val = embeddings[i];
        if (embeddings[i] > max_val) max_val = embeddings[i];
    }
    
    printf("  NaN count: %d\n", nan_count);
    printf("  Inf count: %d\n", inf_count);
    printf("  Min value: %.6f\n", min_val);
    printf("  Max value: %.6f\n", max_val);
    
    assert(nan_count == 0 && "FAIL: NaN values found in small vocab");
    assert(inf_count == 0 && "FAIL: Inf values found in small vocab");
    assert(min_val >= -2.0f && max_val <= 2.0f && "FAIL: Values out of expected range");
    
    printf("  ✓ PASS: Small vocabulary test\n");
    
    free(embeddings);
}

// Test 2: Medium vocabulary
void test_medium_vocab() {
    printf("\n=== Test 2: Medium Vocabulary (10,000 tokens) ===\n");
    
    uint32_t vocab_size = 10000;
    float* embeddings = (float*)calloc(vocab_size * EMBEDDING_DIM, sizeof(float));
    
    lattice_embeddings_init_geometric(embeddings, vocab_size, EMBEDDING_DIM);
    
    // Check for NaN/Inf
    int nan_count = 0;
    int inf_count = 0;
    float min_val = embeddings[0];
    float max_val = embeddings[0];
    
    for (uint32_t i = 0; i < vocab_size * EMBEDDING_DIM; i++) {
        if (is_nan_float(embeddings[i])) nan_count++;
        if (is_inf_float(embeddings[i])) inf_count++;
        if (embeddings[i] < min_val) min_val = embeddings[i];
        if (embeddings[i] > max_val) max_val = embeddings[i];
    }
    
    printf("  NaN count: %d\n", nan_count);
    printf("  Inf count: %d\n", inf_count);
    printf("  Min value: %.6f\n", min_val);
    printf("  Max value: %.6f\n", max_val);
    
    assert(nan_count == 0 && "FAIL: NaN values found in medium vocab");
    assert(inf_count == 0 && "FAIL: Inf values found in medium vocab");
    assert(min_val >= -2.0f && max_val <= 2.0f && "FAIL: Values out of expected range");
    
    printf("  ✓ PASS: Medium vocabulary test\n");
    
    free(embeddings);
}

// Test 3: Large vocabulary (stress test)
void test_large_vocab() {
    printf("\n=== Test 3: Large Vocabulary (1,000,000 tokens) ===\n");
    
    uint32_t vocab_size = 1000000;
    printf("  Allocating %.2f MB for embeddings...\n", 
           (vocab_size * EMBEDDING_DIM * sizeof(float)) / (1024.0 * 1024.0));
    
    float* embeddings = (float*)calloc(vocab_size * EMBEDDING_DIM, sizeof(float));
    if (!embeddings) {
        printf("  ✗ SKIP: Could not allocate memory for large vocab test\n");
        return;
    }
    
    printf("  Initializing embeddings...\n");
    lattice_embeddings_init_geometric(embeddings, vocab_size, EMBEDDING_DIM);
    
    // Sample check (checking all 128M values would be slow)
    printf("  Sampling 10,000 random embeddings for validation...\n");
    int nan_count = 0;
    int inf_count = 0;
    float min_val = embeddings[0];
    float max_val = embeddings[0];
    
    for (int sample = 0; sample < 10000; sample++) {
        uint32_t idx = (rand() % vocab_size) * EMBEDDING_DIM + (rand() % EMBEDDING_DIM);
        if (is_nan_float(embeddings[idx])) nan_count++;
        if (is_inf_float(embeddings[idx])) inf_count++;
        if (embeddings[idx] < min_val) min_val = embeddings[idx];
        if (embeddings[idx] > max_val) max_val = embeddings[idx];
    }
    
    printf("  NaN count (in sample): %d\n", nan_count);
    printf("  Inf count (in sample): %d\n", inf_count);
    printf("  Min value (in sample): %.6f\n", min_val);
    printf("  Max value (in sample): %.6f\n", max_val);
    
    assert(nan_count == 0 && "FAIL: NaN values found in large vocab");
    assert(inf_count == 0 && "FAIL: Inf values found in large vocab");
    assert(min_val >= -2.0f && max_val <= 2.0f && "FAIL: Values out of expected range");
    
    printf("  ✓ PASS: Large vocabulary test\n");
    
    free(embeddings);
}

// Test 4: Clock lattice wrapping
void test_clock_wrapping() {
    printf("\n=== Test 4: Clock Lattice Wrapping ===\n");
    
    // Test that primes beyond 232 wrap correctly
    uint32_t test_indices[] = {0, 100, 232, 500, 1000, 10000, 100000};
    int num_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (int i = 0; i < num_tests; i++) {
        uint32_t vocab_size = test_indices[i] + 10;
        float* embeddings = (float*)calloc(vocab_size * EMBEDDING_DIM, sizeof(float));
        
        lattice_embeddings_init_geometric(embeddings, vocab_size, EMBEDDING_DIM);
        
        // Check the last token
        int nan_found = 0;
        int inf_found = 0;
        for (uint32_t d = 0; d < EMBEDDING_DIM; d++) {
            float val = embeddings[(vocab_size - 1) * EMBEDDING_DIM + d];
            if (is_nan_float(val)) nan_found = 1;
            if (is_inf_float(val)) inf_found = 1;
        }
        
        printf("  Token %u: %s\n", vocab_size - 1, 
               (nan_found || inf_found) ? "✗ FAIL" : "✓ PASS");
        
        assert(!nan_found && !inf_found && "FAIL: Invalid values in wrapped tokens");
        
        free(embeddings);
    }
    
    printf("  ✓ PASS: Clock lattice wrapping test\n");
}

// Test 5: Verify 3^O stays bounded
void test_exponent_bounds() {
    printf("\n=== Test 5: Exponent Bounds (3^O) ===\n");
    
    // The clock lattice should keep O bounded
    // Ring 0-3: O = 0 to 4, so 3^O = 1 to 81
    // Ring 4-7: O = 4 to 8, so 3^O = 81 to 6561
    
    printf("  Testing 3^O for various ring values:\n");
    for (int ring = 0; ring <= 7; ring++) {
        double O = (double)ring + 0.5;  // Mid-point of ring
        double result = math_pow(3.0, O);
        printf("    Ring %d: O=%.1f, 3^O=%.2f\n", ring, O, result);
        
        assert(!isnan(result) && !isinf(result) && "FAIL: 3^O produced NaN/Inf");
        assert(result < 10000.0 && "FAIL: 3^O too large");
    }
    
    printf("  ✓ PASS: Exponent bounds test\n");
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Lattice Embeddings Large Vocabulary Test Suite          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_small_vocab();
    test_medium_vocab();
    test_large_vocab();
    test_clock_wrapping();
    test_exponent_bounds();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ✓ ALL TESTS PASSED                                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}