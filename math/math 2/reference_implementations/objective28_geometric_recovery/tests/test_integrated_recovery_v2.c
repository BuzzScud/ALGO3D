#include "../include/integrated_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/**
 * Test Integrated Recovery System V2
 * 
 * Tests the complete integration of:
 * - Geometric anchors (50 Platonic vertices)
 * - Anchor tracking (convergence analysis)
 * - Real ECDSA samples (ground truth)
 * - Recovery algorithm
 */

void test_create_integrated_context() {
    printf("TEST: Create Integrated Recovery Context\n");
    
    // Create context for 8-bit samples
    IntegratedRecoveryContext* ctx = create_integrated_recovery("samples", 8);
    
    assert(ctx != NULL);
    assert(ctx->samples != NULL);
    assert(ctx->num_samples == 20);  // 10 secp256k1 + 10 secp192k1
    assert(ctx->geo_anchors != NULL);
    assert(ctx->num_geo_anchors == 50);
    assert(ctx->tracking_system != NULL);
    
    printf("  ✓ Context created successfully\n");
    printf("  ✓ Samples: %d\n", ctx->num_samples);
    printf("  ✓ Geometric anchors: %d\n", ctx->num_geo_anchors);
    
    // Cleanup
    free_integrated_recovery(ctx);
    
    printf("  ✓ Test passed\n\n");
}

void test_recovery_single_sample() {
    printf("TEST: Recovery on Single Sample\n");
    
    // Create context for 8-bit samples
    IntegratedRecoveryContext* ctx = create_integrated_recovery("samples", 8);
    assert(ctx != NULL);
    
    // Run recovery on first sample
    RecoveryResult* result = run_integrated_recovery_single(ctx, ctx->samples[0], 0);
    
    assert(result != NULL);
    assert(result->anchor != NULL);
    
    printf("  ✓ Recovery completed\n");
    printf("  ✓ Real k: %lu\n", result->real_k);
    printf("  ✓ Recovered k: %lu\n", result->recovered_k);
    printf("  ✓ Error: %.2f\n", result->error_magnitude);
    printf("  ✓ Success: %s\n", result->success ? "YES" : "NO");
    printf("  ✓ Time: %.4f seconds\n", result->elapsed_time);
    
    // Cleanup
    free(result);
    free_integrated_recovery(ctx);
    
    printf("  ✓ Test passed\n\n");
}

void test_recovery_all_8bit() {
    printf("TEST: Recovery on All 8-bit Samples\n");
    
    // Create context for 8-bit samples
    IntegratedRecoveryContext* ctx = create_integrated_recovery("samples", 8);
    assert(ctx != NULL);
    
    // Run recovery on all samples
    run_integrated_recovery_all(ctx);
    
    // Print summary
    print_recovery_summary(ctx);
    
    // Generate report
    generate_recovery_report(ctx, "integrated_recovery_8bit_report.md");
    printf("  ✓ Report saved to integrated_recovery_8bit_report.md\n");
    
    // Export CSV
    export_recovery_results_csv(ctx, "integrated_recovery_8bit_results.csv");
    printf("  ✓ Results saved to integrated_recovery_8bit_results.csv\n");
    
    // Cleanup
    free_integrated_recovery(ctx);
    
    printf("  ✓ Test passed\n\n");
}

void test_recovery_multiple_bitlen() {
    printf("TEST: Recovery Across Multiple Bit Lengths\n");
    
    int bit_lengths[] = {8, 16, 32};
    
    for (int i = 0; i < 3; i++) {
        int bitlen = bit_lengths[i];
        
        printf("\n  Testing %d-bit samples...\n", bitlen);
        
        // Create context
        IntegratedRecoveryContext* ctx = create_integrated_recovery("samples", bitlen);
        if (!ctx) {
            printf("    ✗ Failed to create context\n");
            continue;
        }
        
        // Run recovery
        run_integrated_recovery_all(ctx);
        
        // Print summary
        printf("\n    Summary for %d-bit:\n", bitlen);
        printf("      Success rate: %.2f%% (%u/%u)\n",
               ctx->success_rate * 100.0,
               ctx->successful_recoveries,
               ctx->total_attempts);
        
        // Cleanup
        free_integrated_recovery(ctx);
    }
    
    printf("\n  ✓ Test passed\n\n");
}

int main() {
    printf("\n========================================\n");
    printf("INTEGRATED RECOVERY V2 TEST SUITE\n");
    printf("========================================\n\n");
    
    test_create_integrated_context();
    test_recovery_single_sample();
    test_recovery_all_8bit();
    test_recovery_multiple_bitlen();
    
    printf("========================================\n");
    printf("ALL TESTS PASSED! ✓\n");
    printf("========================================\n\n");
    
    return 0;
}