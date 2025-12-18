/**
 * @file test_blind_recovery_simple.c
 * @brief Simple test for blind recovery to debug memory issues
 */

#include "cllm.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Simple Blind Recovery Test ===\n\n");
    
    // Create a simple cube model
    printf("Creating cube model...\n");
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 50);
    config.enable_blind_recovery = true;
    config.corruption_tolerance = 0.25;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("❌ Failed to create model\n");
        return 1;
    }
    
    printf("✓ Model created successfully\n");
    printf("  Recovery enabled: %d\n", model->recovery.enabled);
    printf("  Corruption tolerance: %.1f%%\n", model->recovery.corruption_tolerance * 100);
    
    // Test corruption detection
    printf("\nTesting corruption detection...\n");
    int corruptions = cllm_detect_corruption(model);
    printf("  Detected corruptions: %d\n", corruptions);
    
    // Free model
    printf("\nFreeing model...\n");
    cllm_free_model(model);
    printf("✓ Model freed successfully\n");
    
    printf("\n✅ TEST PASSED\n");
    return 0;
}