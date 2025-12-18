/**
 * test_clock_mapping.c - Test Clock Position Mapping
 * 
 * Tests the π×φ metric and Babylonian clock structure mapping
 */

#include "clock_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openssl/bn.h>

#define NUM_TEST_K 10

void print_clock_position(const char* label, uint64_t k, ClockPosition pos) {
    printf("%s k=%lu:\n", label, k);
    printf("  Ring: %d, Position: %d\n", pos.ring, pos.position);
    printf("  Angle: %.6f rad (%.2f°)\n", pos.angle, pos.angle * 180.0 / M_PI);
    printf("  Radius: %.6f\n", pos.radius);
    printf("\n");
}

void test_clock_mapping_basic() {
    printf("=== Test 1: Basic Clock Mapping ===\n\n");
    
    // Test with small k values
    uint64_t test_k[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        ClockPosition pos = map_k_to_clock_u64(test_k[i]);
        print_clock_position("Test", test_k[i], pos);
    }
}

void test_pi_phi_metric() {
    printf("=== Test 2: π×φ Metric Properties ===\n\n");
    
    // Test that π×φ metric creates circular distribution
    uint64_t k_values[] = {1, 10, 100, 1000, 10000};
    
    printf("Testing circular distribution:\n");
    for (int i = 0; i < 5; i++) {
        double angle = compute_pi_phi_angle_u64(k_values[i]);
        printf("k=%lu → θ=%.6f rad (%.2f°)\n", 
               k_values[i], angle, angle * 180.0 / M_PI);
    }
    printf("\n");
    
    // Test that angles wrap around [0, 2π)
    printf("Testing angle normalization:\n");
    for (int i = 0; i < 5; i++) {
        double angle = compute_pi_phi_angle_u64(k_values[i]);
        printf("k=%lu → θ ∈ [0, 2π): %s\n", 
               k_values[i], (angle >= 0 && angle < 2 * M_PI) ? "✓" : "✗");
    }
    printf("\n");
}

void test_ring_distribution() {
    printf("=== Test 3: Ring Distribution ===\n\n");
    
    // Count how many k values map to each ring
    int ring_counts[4] = {0, 0, 0, 0};
    int num_samples = 1000;
    
    for (int i = 1; i <= num_samples; i++) {
        ClockPosition pos = map_k_to_clock_u64(i);
        if (pos.ring >= 0 && pos.ring < 4) {
            ring_counts[pos.ring]++;
        }
    }
    
    printf("Distribution of %d k values across rings:\n", num_samples);
    printf("Ring 0 (12 pos):  %d (%.1f%%)\n", ring_counts[0], 100.0 * ring_counts[0] / num_samples);
    printf("Ring 1 (60 pos):  %d (%.1f%%)\n", ring_counts[1], 100.0 * ring_counts[1] / num_samples);
    printf("Ring 2 (60 pos):  %d (%.1f%%)\n", ring_counts[2], 100.0 * ring_counts[2] / num_samples);
    printf("Ring 3 (100 pos): %d (%.1f%%)\n", ring_counts[3], 100.0 * ring_counts[3] / num_samples);
    printf("\n");
}

void test_pythagorean_triples() {
    printf("=== Test 4: Pythagorean Triple Detection ===\n\n");
    
    // Test with known Pythagorean triples
    // (3, 4, 5), (5, 12, 13), (8, 15, 17)
    
    // Map k values that should form triples
    ClockPosition pos1 = map_k_to_clock_u64(3);
    ClockPosition pos2 = map_k_to_clock_u64(4);
    ClockPosition pos3 = map_k_to_clock_u64(5);
    
    printf("Testing k=3, k=4, k=5:\n");
    printf("  pos1: ring=%d, angle=%.6f\n", pos1.ring, pos1.angle);
    printf("  pos2: ring=%d, angle=%.6f\n", pos2.ring, pos2.angle);
    printf("  pos3: ring=%d, angle=%.6f\n", pos3.ring, pos3.angle);
    
    bool is_triple = is_pythagorean_triple(pos1, pos2, pos3);
    printf("  Is Pythagorean triple: %s\n", is_triple ? "✓" : "✗");
    
    if (is_triple) {
        PythagoreanTriple triple;
        if (find_pythagorean_triple(pos1, pos2, pos3, &triple)) {
            printf("  Triple: (%lu, %lu, %lu) with p=%u, q=%u\n",
                   triple.a, triple.b, triple.c, triple.p, triple.q);
        }
    }
    printf("\n");
}

void test_dimensional_frequencies() {
    printf("=== Test 5: Dimensional Frequency Analysis ===\n\n");
    
    DimensionalFrequency dims[13];
    init_dimensional_frequencies(dims);
    
    printf("Dimensional frequencies:\n");
    for (int i = 0; i < 13; i++) {
        printf("  φ[%d] = %.1f\n", i, dims[i].frequency);
    }
    printf("\n");
    
    // Test alignment for a specific k
    uint64_t test_k = 42;
    double angle = compute_pi_phi_angle_u64(test_k);
    
    printf("Testing alignment for k=%lu (θ=%.6f):\n", test_k, angle);
    double total_alignment = compute_total_alignment(angle, dims);
    printf("  Total alignment: %.6f\n", total_alignment);
    
    double entropy = compute_entropy_from_alignment(total_alignment);
    printf("  Entropy: %.6f\n", entropy);
    printf("\n");
    
    // Show individual dimension alignments
    printf("Individual dimension alignments:\n");
    for (int i = 0; i < 13; i++) {
        printf("  dim[%d]: %.6f\n", i, dims[i].alignment);
    }
    printf("\n");
}

void test_full_recovery_pipeline() {
    printf("=== Test 6: Full Recovery Pipeline ===\n\n");
    
    // Create recovery context with 10 anchors
    ClockRecoveryContext* ctx = init_clock_recovery(10);
    if (!ctx) {
        printf("Failed to initialize recovery context\n");
        return;
    }
    
    // Add some test anchors
    BN_CTX* bn_ctx = BN_CTX_new();
    BIGNUM* k = BN_new();
    
    for (uint32_t i = 0; i < 10; i++) {
        BN_set_word(k, (i + 1) * 7);  // Use multiples of 7 as test k values
        add_anchor(ctx, k, i);
    }
    
    printf("Added 10 anchors to recovery context\n");
    printf("Anchor positions:\n");
    for (uint32_t i = 0; i < 10; i++) {
        printf("  Anchor %u: ring=%d, pos=%d, angle=%.6f\n",
               i, ctx->anchors[i].ring, ctx->anchors[i].position, ctx->anchors[i].angle);
    }
    printf("\n");
    
    // Test recovery (simplified - no actual Q point)
    BIGNUM* recovered_k = BN_new();
    BIGNUM* dummy_Q = BN_new();  // Placeholder
    
    bool success = recover_k_from_clock(ctx, dummy_Q, recovered_k);
    printf("Recovery %s\n", success ? "succeeded" : "failed");
    
    // Cleanup
    BN_free(k);
    BN_free(recovered_k);
    BN_free(dummy_Q);
    BN_CTX_free(bn_ctx);
    free_clock_recovery(ctx);
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Clock Position Mapping Test Suite                        ║\n");
    printf("║  Testing π×φ metric and Babylonian clock structure        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    test_clock_mapping_basic();
    test_pi_phi_metric();
    test_ring_distribution();
    test_pythagorean_triples();
    test_dimensional_frequencies();
    test_full_recovery_pipeline();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  All Tests Complete                                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}