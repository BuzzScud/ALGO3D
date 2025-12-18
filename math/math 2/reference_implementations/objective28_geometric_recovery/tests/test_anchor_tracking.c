#include "../include/anchor_tracking.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void test_create_anchor_tracking() {
    printf("TEST: Create Anchor Tracking\n");
    
    // Create a BIGNUM for real_k
    BIGNUM* real_k = BN_new();
    BN_set_word(real_k, 42);
    
    // Create anchor tracking
    AnchorTracking* anchor = create_anchor_tracking(1, real_k, 10);
    
    assert(anchor != NULL);
    assert(anchor->anchor_id == 1);
    assert(anchor->max_recursion_levels == 10);
    assert(anchor->real_k != NULL);
    assert(BN_cmp(anchor->real_k, real_k) == 0);
    
    printf("  ✓ Anchor created successfully\n");
    printf("  ✓ Real k: 42\n");
    printf("  ✓ Real angle: %.2f°\n", anchor->real_angle);
    
    // Cleanup
    free_anchor_tracking(anchor);
    BN_free(real_k);
    
    printf("  ✓ Test passed\n\n");
}

void test_update_anchor_estimate() {
    printf("TEST: Update Anchor Estimate\n");
    
    // Create real_k = 42
    BIGNUM* real_k = BN_new();
    BN_set_word(real_k, 42);
    
    // Create anchor
    AnchorTracking* anchor = create_anchor_tracking(1, real_k, 10);
    
    // Create estimated_k = 40 (underestimate)
    BIGNUM* estimated_k = BN_new();
    BN_set_word(estimated_k, 40);
    
    // Update estimate at level 0
    update_anchor_estimate(anchor, estimated_k, 0);
    
    assert(anchor->estimated_k != NULL);
    assert(anchor->num_recursion_levels == 1);
    assert(anchor->error_direction == -1);  // Underestimate
    
    printf("  ✓ Estimate updated: 40 (underestimate)\n");
    printf("  ✓ Error magnitude: %.2f\n", anchor->error_magnitude);
    printf("  ✓ Error direction: %d\n", anchor->error_direction);
    
    // Update estimate at level 1 (closer to real value)
    BN_set_word(estimated_k, 41);
    update_anchor_estimate(anchor, estimated_k, 1);
    
    assert(anchor->num_recursion_levels == 2);
    assert(anchor->is_converging == true);  // Error decreased
    
    printf("  ✓ Estimate updated: 41 (converging)\n");
    printf("  ✓ Converging: %s\n", anchor->is_converging ? "YES" : "NO");
    printf("  ✓ Convergence rate: %.4f\n", anchor->convergence_rate);
    
    // Cleanup
    free_anchor_tracking(anchor);
    BN_free(real_k);
    BN_free(estimated_k);
    
    printf("  ✓ Test passed\n\n");
}

void test_anchor_tracking_system() {
    printf("TEST: Anchor Tracking System\n");
    
    // Create system with 3 anchors
    AnchorTrackingSystem* system = create_anchor_tracking_system(3);
    assert(system != NULL);
    assert(system->num_anchors == 3);
    
    printf("  ✓ System created with 3 anchors\n");
    
    // Create 3 anchors with different k values
    BIGNUM* k1 = BN_new();
    BIGNUM* k2 = BN_new();
    BIGNUM* k3 = BN_new();
    
    BN_set_word(k1, 10);
    BN_set_word(k2, 50);
    BN_set_word(k3, 100);
    
    AnchorTracking* anchor1 = create_anchor_tracking(1, k1, 10);
    AnchorTracking* anchor2 = create_anchor_tracking(2, k2, 10);
    AnchorTracking* anchor3 = create_anchor_tracking(3, k3, 10);
    
    // Add anchors to system
    add_anchor_to_system(system, anchor1);
    add_anchor_to_system(system, anchor2);
    add_anchor_to_system(system, anchor3);
    
    printf("  ✓ Added 3 anchors to system\n");
    
    // Update estimates for all anchors
    BIGNUM* est = BN_new();
    
    // Anchor 1: estimate = 12 (overestimate)
    BN_set_word(est, 12);
    update_anchor_estimate(anchor1, est, 0);
    
    // Anchor 2: estimate = 48 (underestimate)
    BN_set_word(est, 48);
    update_anchor_estimate(anchor2, est, 0);
    
    // Anchor 3: estimate = 105 (overestimate)
    BN_set_word(est, 105);
    update_anchor_estimate(anchor3, est, 0);
    
    printf("  ✓ Updated estimates for all anchors\n");
    
    // Compute global statistics
    compute_global_statistics(system);
    
    printf("  ✓ Average error: %.2f\n", system->avg_error);
    printf("  ✓ Overestimates: %u\n", system->num_overestimates);
    printf("  ✓ Underestimates: %u\n", system->num_underestimates);
    
    assert(system->num_overestimates == 2);
    assert(system->num_underestimates == 1);
    
    // Analyze lattice structure
    analyze_lattice_structure(system);
    
    printf("  ✓ Lattice spacing: %.2f\n", system->lattice_spacing);
    printf("  ✓ Lattice regularity: %.4f\n", system->lattice_regularity);
    
    // Print summary
    print_tracking_system_summary(system);
    
    // Export to CSV
    export_tracking_to_csv(system, "anchor_tracking_test.csv");
    printf("  ✓ Exported to anchor_tracking_test.csv\n");
    
    // Cleanup
    BN_free(est);
    BN_free(k1);
    BN_free(k2);
    BN_free(k3);
    free_anchor_tracking_system(system);
    
    printf("  ✓ Test passed\n\n");
}

void test_convergence_tracking() {
    printf("TEST: Convergence Tracking\n");
    
    // Create anchor with real_k = 100
    BIGNUM* real_k = BN_new();
    BN_set_word(real_k, 100);
    
    AnchorTracking* anchor = create_anchor_tracking(1, real_k, 10);
    
    // Simulate convergence: 80 -> 90 -> 95 -> 98 -> 99 -> 100
    BIGNUM* est = BN_new();
    uint64_t estimates[] = {80, 90, 95, 98, 99, 100};
    
    for (uint32_t i = 0; i < 6; i++) {
        BN_set_word(est, estimates[i]);
        update_anchor_estimate(anchor, est, i);
        
        printf("  Level %u: estimate=%lu, error=%.2f, converging=%s, rate=%.4f\n",
               i, estimates[i], anchor->error_magnitude,
               anchor->is_converging ? "YES" : "NO",
               anchor->convergence_rate);
    }
    
    // Should be converging
    assert(anchor->is_converging == true);
    assert(anchor->error_magnitude < 1.0);
    
    printf("  ✓ Convergence verified\n");
    
    // Cleanup
    free_anchor_tracking(anchor);
    BN_free(real_k);
    BN_free(est);
    
    printf("  ✓ Test passed\n\n");
}

void test_error_vector_computation() {
    printf("TEST: Error Vector Computation\n");
    
    // Create anchor with real_k = 50
    BIGNUM* real_k = BN_new();
    BN_set_word(real_k, 50);
    
    AnchorTracking* anchor = create_anchor_tracking(1, real_k, 10);
    
    // Set estimate = 55
    BIGNUM* est = BN_new();
    BN_set_word(est, 55);
    update_anchor_estimate(anchor, est, 0);
    
    // Check error vector
    printf("  Error vector in 13D space:\n");
    for (uint32_t i = 0; i < 13; i++) {
        printf("    Dim %u: %.4f\n", i, anchor->error_vector[i]);
    }
    
    // Compute magnitude of error vector
    double magnitude = 0.0;
    for (uint32_t i = 0; i < 13; i++) {
        magnitude += anchor->error_vector[i] * anchor->error_vector[i];
    }
    magnitude = math_sqrt(magnitude);
    
    printf("  ✓ Error vector magnitude: %.4f\n", magnitude);
    
    // Cleanup
    free_anchor_tracking(anchor);
    BN_free(real_k);
    BN_free(est);
    
    printf("  ✓ Test passed\n\n");
}

int main() {
    printf("\n========================================\n");
    printf("ANCHOR TRACKING TEST SUITE\n");
    printf("========================================\n\n");
    
    test_create_anchor_tracking();
    test_update_anchor_estimate();
    test_anchor_tracking_system();
    test_convergence_tracking();
    test_error_vector_computation();
    
    printf("========================================\n");
    printf("ALL TESTS PASSED! ✓\n");
    printf("========================================\n\n");
    
    return 0;
}