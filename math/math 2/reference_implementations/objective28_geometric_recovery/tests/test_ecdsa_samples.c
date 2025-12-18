#include "../include/ecdsa_sample_loader.h"
#include "../include/anchor_tracking.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/**
 * Test ECDSA Sample Loading and Anchor Tracking Integration
 */

void test_load_single_sample() {
    printf("TEST: Load Single Sample\n");
    
    // Load first 8-bit sample
    ECDSASample* sample = load_ecdsa_sample("samples/secp256k1_8bit_sample_001.txt");
    
    assert(sample != NULL);
    assert(sample->bit_length == 8);
    assert(sample->curve_nid == 714);  // secp256k1
    assert(sample->k != NULL);
    assert(sample->r != NULL);
    assert(sample->s != NULL);
    
    printf("  ✓ Sample loaded successfully\n");
    
    // Print sample info
    print_ecdsa_sample(sample);
    
    // Cleanup
    free_ecdsa_sample(sample);
    
    printf("  ✓ Test passed\n\n");
}

void test_load_all_samples() {
    printf("TEST: Load All Samples\n");
    
    int num_samples = 0;
    ECDSASample** samples = load_all_samples("samples", &num_samples);
    
    assert(samples != NULL);
    assert(num_samples == 300);  // 160 secp256k1 + 140 secp192k1
    
    printf("  ✓ Loaded %d samples\n", num_samples);
    
    // Print statistics
    print_sample_statistics(samples, num_samples);
    
    // Cleanup
    free_ecdsa_samples(samples, num_samples);
    
    printf("  ✓ Test passed\n\n");
}

void test_load_by_bit_length() {
    printf("TEST: Load Samples by Bit Length\n");
    
    // Load 8-bit samples
    int num_samples = 0;
    ECDSASample** samples = load_samples_by_bit_length("samples", 8, &num_samples);
    
    assert(samples != NULL);
    assert(num_samples == 20);  // 10 secp256k1 + 10 secp192k1
    
    printf("  ✓ Loaded %d 8-bit samples\n", num_samples);
    
    // Verify all are 8-bit
    for (int i = 0; i < num_samples; i++) {
        assert(samples[i]->bit_length == 8);
    }
    
    printf("  ✓ All samples are 8-bit\n");
    
    // Cleanup
    free_ecdsa_samples(samples, num_samples);
    
    printf("  ✓ Test passed\n\n");
}

void test_anchor_tracking_with_samples() {
    printf("TEST: Anchor Tracking with Real ECDSA Samples\n");
    
    // Load 8-bit samples
    int num_samples = 0;
    ECDSASample** samples = load_samples_by_bit_length("samples", 8, &num_samples);
    
    assert(samples != NULL);
    assert(num_samples > 0);
    
    printf("  ✓ Loaded %d samples\n", num_samples);
    
    // Create anchor tracking system
    AnchorTrackingSystem* system = create_anchor_tracking_system(num_samples);
    assert(system != NULL);
    
    printf("  ✓ Created tracking system\n");
    
    // Create anchor tracking for each sample
    for (int i = 0; i < num_samples; i++) {
        AnchorTracking* anchor = create_anchor_tracking(i + 1, samples[i]->k, 10);
        assert(anchor != NULL);
        
        add_anchor_to_system(system, anchor);
    }
    
    printf("  ✓ Added %d anchors to system\n", num_samples);
    
    // Simulate recovery attempts with estimates
    // For testing, use k-2, k-1, k (converging to real k)
    for (int i = 0; i < num_samples; i++) {
        AnchorTracking* anchor = system->anchors[i];
        
        // Level 0: underestimate by 2
        BIGNUM* est = BN_dup(samples[i]->k);
        BN_sub_word(est, 2);
        update_anchor_estimate(anchor, est, 0);
        BN_free(est);
        
        // Level 1: underestimate by 1
        est = BN_dup(samples[i]->k);
        BN_sub_word(est, 1);
        update_anchor_estimate(anchor, est, 1);
        BN_free(est);
        
        // Level 2: exact match
        est = BN_dup(samples[i]->k);
        update_anchor_estimate(anchor, est, 2);
        BN_free(est);
    }
    
    printf("  ✓ Updated all anchor estimates (3 levels)\n");
    
    // Compute global statistics
    compute_global_statistics(system);
    
    printf("  ✓ Computed global statistics\n");
    
    // Print summary
    print_tracking_system_summary(system);
    
    // Verify convergence
    assert(system->global_convergence == true);
    printf("  ✓ Global convergence detected\n");
    
    // Export to CSV
    export_tracking_to_csv(system, "ecdsa_samples_tracking.csv");
    printf("  ✓ Exported to ecdsa_samples_tracking.csv\n");
    
    // Cleanup
    free_anchor_tracking_system(system);
    free_ecdsa_samples(samples, num_samples);
    
    printf("  ✓ Test passed\n\n");
}

void test_multiple_bit_lengths() {
    printf("TEST: Anchor Tracking Across Multiple Bit Lengths\n");
    
    int bit_lengths[] = {8, 16, 32, 64};
    int num_bit_lengths = 4;
    
    for (int b = 0; b < num_bit_lengths; b++) {
        int bit_length = bit_lengths[b];
        
        printf("\n  Testing %d-bit samples...\n", bit_length);
        
        // Load samples
        int num_samples = 0;
        ECDSASample** samples = load_samples_by_bit_length("samples", bit_length, &num_samples);
        
        if (!samples || num_samples == 0) {
            printf("    ⚠ No samples found for %d-bit\n", bit_length);
            continue;
        }
        
        printf("    ✓ Loaded %d samples\n", num_samples);
        
        // Create tracking system
        AnchorTrackingSystem* system = create_anchor_tracking_system(num_samples);
        
        // Add anchors
        for (int i = 0; i < num_samples; i++) {
            AnchorTracking* anchor = create_anchor_tracking(i + 1, samples[i]->k, 5);
            add_anchor_to_system(system, anchor);
        }
        
        // Simulate convergence
        for (int i = 0; i < num_samples; i++) {
            AnchorTracking* anchor = system->anchors[i];
            
            // Start with 50% error
            BIGNUM* est = BN_dup(samples[i]->k);
            BN_div_word(est, 2);
            update_anchor_estimate(anchor, est, 0);
            BN_free(est);
            
            // Improve to 75%
            est = BN_dup(samples[i]->k);
            BIGNUM* three_quarters = BN_new();
            BN_mul_word(est, 3);
            BN_div_word(est, 4);
            update_anchor_estimate(anchor, est, 1);
            BN_free(est);
            BN_free(three_quarters);
            
            // Improve to 90%
            est = BN_dup(samples[i]->k);
            BN_mul_word(est, 9);
            BN_div_word(est, 10);
            update_anchor_estimate(anchor, est, 2);
            BN_free(est);
            
            // Final: exact
            est = BN_dup(samples[i]->k);
            update_anchor_estimate(anchor, est, 3);
            BN_free(est);
        }
        
        // Compute statistics
        compute_global_statistics(system);
        
        printf("    ✓ Average error: %.2f\n", system->avg_error);
        printf("    ✓ Convergence: %s\n", system->global_convergence ? "YES" : "NO");
        
        // Cleanup
        free_anchor_tracking_system(system);
        free_ecdsa_samples(samples, num_samples);
    }
    
    printf("\n  ✓ Test passed\n\n");
}

int main() {
    printf("\n========================================\n");
    printf("ECDSA SAMPLE LOADER TEST SUITE\n");
    printf("========================================\n\n");
    
    test_load_single_sample();
    test_load_all_samples();
    test_load_by_bit_length();
    test_anchor_tracking_with_samples();
    test_multiple_bit_lengths();
    
    printf("========================================\n");
    printf("ALL TESTS PASSED! ✓\n");
    printf("========================================\n\n");
    
    return 0;
}