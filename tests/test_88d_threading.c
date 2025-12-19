/**
 * Test script to verify 88D threading is active
 * 
 * Compile: gcc -o test_88d_threading test_88d_threading.c -L./math\ math\ 2/cllm -lcllm -lpthread
 * Run: ./test_88d_threading
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// Include CLLM headers
#include "cllm/include/ai/cllm.h"
#include "cllm/include/ai/cllm_price_projection.h"
#include "cllm/include/ai/cllm_88d_integration.h"

int main() {
    printf("=== 88D Threading Verification Test ===\n\n");
    
    // Test 1: Create CLLM model (should initialize 88D threading)
    printf("Test 1: Creating CLLM model...\n");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = 1000,
        .max_seq_len = 512,
        .embedding_dim = 768,
        .hidden_dim = 3072,
        .num_layers = 8,
        .num_heads = 12,
        .enable_blind_recovery = false,
        .enable_harmonic_integration = true,
        .enable_ntt_attention = true,
        .enable_kissing_spheres = true
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    if (!model) {
        fprintf(stderr, "✗ FAILED: Could not create CLLM model\n");
        return 1;
    }
    
    printf("  ✓ CLLM model created\n");
    
    // Test 2: Verify 88D threading is initialized
    printf("\nTest 2: Verifying 88D threading...\n");
    
    if (!model->threads) {
        fprintf(stderr, "✗ FAILED: Thread pool is NULL (88D threading not initialized)\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("  ✓ Thread pool exists\n");
    
    // Test 3: Verify thread count
    printf("\nTest 3: Verifying thread count...\n");
    
    if (model->threads->num_threads != 96) {
        fprintf(stderr, "✗ FAILED: Expected 96 threads, got %u\n", model->threads->num_threads);
        cllm_free_model(model);
        return 1;
    }
    
    printf("  ✓ Thread count correct: %u threads (8 layers × 12 threads)\n", model->threads->num_threads);
    
    // Test 4: Verify thread structure
    printf("\nTest 4: Verifying thread structure...\n");
    
    bool all_threads_valid = true;
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 0; dim <= 11; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get(model->threads, layer, dim);
            if (!thread) {
                fprintf(stderr, "  ✗ Thread [%d][%d] is NULL\n", layer, dim);
                all_threads_valid = false;
            }
        }
    }
    
    if (!all_threads_valid) {
        fprintf(stderr, "✗ FAILED: Some threads are NULL\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("  ✓ All 96 threads are valid\n");
    
    // Test 5: Verify token assignments
    printf("\nTest 5: Verifying token assignments...\n");
    
    if (!model->token_assignments) {
        fprintf(stderr, "✗ FAILED: Token assignments are NULL\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("  ✓ Token assignments exist\n");
    printf("  ✓ Vocab size: %u\n", model->vocab_size);
    
    // Test 6: Test price projection (uses 88D threading)
    printf("\nTest 6: Testing price projection with 88D threading...\n");
    
    double historical_prices[] = {100.0, 101.0, 102.0, 103.0, 104.0, 105.0};
    
    PriceProjectionConfig proj_config = {
        .depth_prime = 127,
        .base = 105.0,
        .steps = 20,
        .projection_count = 3,
        .omega_hz = 432.0,
        .decimals = 8
    };
    
    PriceProjectionResult* result = cllm_price_projection_compute(
        &proj_config,
        historical_prices,
        sizeof(historical_prices) / sizeof(historical_prices[0])
    );
    
    if (!result) {
        fprintf(stderr, "✗ FAILED: Price projection returned NULL\n");
        cllm_free_model(model);
        return 1;
    }
    
    if (!result->success) {
        fprintf(stderr, "✗ FAILED: Price projection failed: %s\n", 
                result->error_message ? result->error_message : "Unknown error");
        cllm_price_projection_free_result(result);
        cllm_free_model(model);
        return 1;
    }
    
    printf("  ✓ Price projection successful\n");
    printf("  ✓ Generated %u projection lines\n", result->num_lines);
    printf("  ✓ Steps per line: %u\n", result->steps_per_line);
    
    // Display sample results
    if (result->projection_lines && result->num_lines > 0) {
        printf("\n  Sample projection (first line, first 5 points):\n");
        for (uint32_t i = 0; i < 5 && i < result->steps_per_line; i++) {
            printf("    Step %u: %.2f\n", i, result->projection_lines[0][i]);
        }
    }
    
    cllm_price_projection_free_result(result);
    
    // Cleanup
    cllm_free_model(model);
    
    printf("\n=== All Tests Passed ===\n");
    printf("✓ 88D threading is active and working correctly\n");
    
    return 0;
}

