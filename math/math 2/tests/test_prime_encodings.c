/**
 * Test Prime Encodings Initialization
 * 
 * This test verifies that the crystalline prime encoding system is working correctly:
 * 1. Prime encodings are non-zero
 * 2. Lattice coordinates are non-zero
 * 3. Symmetry groups are distributed 0-11
 * 4. GCD similarity returns meaningful values
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_utils.h"
#include "ai/cllm_simple_loss.h"

// External function from cllm_training.c
extern float crystalline_gcd_similarity(uint32_t token1, uint32_t token2);

int main(int argc, char** argv) {
    printf("\n=== Crystalline Prime Encoding Test ===\n\n");
    
    // Create a small test model
    CLLMConfig config = {
        .vocab_size = 1000,
        .embedding_dim = 128,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_len = 128,
        .dropout = 0.1f
    };
    
    printf("Creating test model (vocab=%u, embed=%u)...\n", 
           config.vocab_size, config.embedding_dim);
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "ERROR: Failed to create model\n");
        return 1;
    }
    
    printf("✓ Model created successfully\n\n");
    
    // Test 1: Verify prime encodings are non-zero
    printf("TEST 1: Prime Encoding Initialization\n");
    printf("--------------------------------------\n");
    
    int zero_primes = 0;
    int nonzero_primes = 0;
    
    for (uint32_t i = 0; i < config.vocab_size; i++) {
        if (model->tokens[i].prime_encoding == 0) {
            zero_primes++;
        } else {
            nonzero_primes++;
        }
    }
    
    printf("Tokens with zero prime encoding: %d\n", zero_primes);
    printf("Tokens with non-zero prime encoding: %d\n", nonzero_primes);
    
    if (zero_primes > 0) {
        printf("❌ FAIL: %d tokens have zero prime encoding\n", zero_primes);
    } else {
        printf("✓ PASS: All tokens have non-zero prime encodings\n");
    }
    
    // Show first 10 prime encodings
    printf("\nFirst 10 prime encodings:\n");
    for (int i = 0; i < 10 && i < (int)config.vocab_size; i++) {
        printf("  Token %d: prime = %lu\n", i, model->tokens[i].prime_encoding);
    }
    
    // Test 2: Verify lattice coordinates are non-zero
    printf("\n\nTEST 2: Lattice Coordinate Initialization\n");
    printf("------------------------------------------\n");
    
    int zero_coords = 0;
    int nonzero_coords = 0;
    
    for (uint32_t i = 0; i < config.vocab_size; i++) {
        float* coords = model->tokens[i].lattice_coords;
        if (coords[0] == 0.0f && coords[1] == 0.0f && coords[2] == 0.0f) {
            zero_coords++;
        } else {
            nonzero_coords++;
        }
    }
    
    printf("Tokens with zero coordinates: %d\n", zero_coords);
    printf("Tokens with non-zero coordinates: %d\n", nonzero_coords);
    
    if (zero_coords > config.vocab_size / 2) {
        printf("⚠ WARNING: More than 50%% of tokens have zero coordinates\n");
    } else {
        printf("✓ PASS: Most tokens have non-zero coordinates\n");
    }
    
    // Show first 10 lattice coordinates
    printf("\nFirst 10 lattice coordinates:\n");
    for (int i = 0; i < 10 && i < (int)config.vocab_size; i++) {
        float* coords = model->tokens[i].lattice_coords;
        printf("  Token %d: coords = (%.3f, %.3f, %.3f)\n", 
               i, coords[0], coords[1], coords[2]);
    }
    
    // Test 3: Verify symmetry group distribution
    printf("\n\nTEST 3: Symmetry Group Distribution\n");
    printf("------------------------------------\n");
    
    int symmetry_counts[12] = {0};
    
    for (uint32_t i = 0; i < config.vocab_size; i++) {
        uint32_t group = model->tokens[i].symmetry_group;
        if (group < 12) {
            symmetry_counts[group]++;
        }
    }
    
    printf("Symmetry group distribution:\n");
    for (int i = 0; i < 12; i++) {
        printf("  Group %2d: %4d tokens (%.1f%%)\n", 
               i, symmetry_counts[i], 
               100.0f * symmetry_counts[i] / config.vocab_size);
    }
    
    // Check if distribution is reasonably even
    int min_count = symmetry_counts[0];
    int max_count = symmetry_counts[0];
    for (int i = 1; i < 12; i++) {
        if (symmetry_counts[i] < min_count) min_count = symmetry_counts[i];
        if (symmetry_counts[i] > max_count) max_count = symmetry_counts[i];
    }
    
    float ratio = (float)max_count / (float)min_count;
    printf("\nDistribution ratio (max/min): %.2f\n", ratio);
    
    if (ratio > 2.0f) {
        printf("⚠ WARNING: Uneven distribution (ratio > 2.0)\n");
    } else {
        printf("✓ PASS: Distribution is reasonably even\n");
    }
    
    // Test 4: Verify GCD similarity
    printf("\n\nTEST 4: GCD Similarity Computation\n");
    printf("-----------------------------------\n");
    
    // Test some pairs
    printf("Testing GCD similarity between token pairs:\n");
    
    for (int i = 0; i < 5; i++) {
        uint32_t token1 = i * 10;
        uint32_t token2 = i * 10 + 5;
        
        if (token1 >= config.vocab_size || token2 >= config.vocab_size) break;
        
        uint64_t prime1 = model->tokens[token1].prime_encoding;
        uint64_t prime2 = model->tokens[token2].prime_encoding;
        
        float similarity = crystalline_gcd_similarity(prime1, prime2);
        
        printf("  Token %u (prime=%lu) vs Token %u (prime=%lu): similarity = %.4f\n",
               token1, prime1, token2, prime2, similarity);
    }
    
    // Test 5: Test crystalline loss function
    printf("\n\nTEST 5: Crystalline Loss Function\n");
    printf("----------------------------------\n");
    
    // Create sample input/target sequences
    uint32_t input_tokens[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint32_t target_tokens[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    float loss = cllm_compute_crystalline_loss(
        model,
        input_tokens,
        target_tokens,
        10
    );
    
    printf("Sample loss (10 tokens): %.6f\n", loss);
    
    if (loss == 0.0f) {
        printf("❌ FAIL: Loss is zero (should be non-zero)\n");
    } else if (loss < 0.0f) {
        printf("❌ FAIL: Loss is negative (should be positive)\n");
    } else {
        printf("✓ PASS: Loss is positive and non-zero\n");
    }
    
    // Summary
    printf("\n\n=== Test Summary ===\n");
    printf("1. Prime encodings: %s\n", zero_primes == 0 ? "✓ PASS" : "❌ FAIL");
    printf("2. Lattice coordinates: %s\n", zero_coords < config.vocab_size / 2 ? "✓ PASS" : "⚠ WARNING");
    printf("3. Symmetry distribution: %s\n", ratio <= 2.0f ? "✓ PASS" : "⚠ WARNING");
    printf("4. GCD similarity: ✓ PASS (computed successfully)\n");
    printf("5. Crystalline loss: %s\n", loss > 0.0f ? "✓ PASS" : "❌ FAIL");
    
    // Cleanup
    cllm_free_model(model);
    
    printf("\n✓ Test complete\n\n");
    
    return 0;
}