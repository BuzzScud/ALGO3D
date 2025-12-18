/**
 * Test Blind Recovery Mechanisms
 * 
 * Tests all four recovery methods:
 * 1. Structural Redundancy (Euler's formula)
 * 2. Symmetry-Based Reconstruction
 * 3. Prime-Based Validation
 * 4. Tetration Attractors
 * 5. Automatic Recovery (tries all methods)
 */

#include "ai/cllm_platonic.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  BLIND RECOVERY MECHANISMS TEST SUITE                    ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test with Cube model (balanced, good for testing)
    printf("Creating test model (Cube)...\n");
    PlatonicModelConfig config = platonic_config_create(
        PLATONIC_CUBE,
        100,   // Small vocab for faster testing
        128    // Small seq len
    );
    
    PlatonicModel* model = platonic_model_create(&config);
    if (!model) {
        printf("✗ Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // Test 1: Structural Recovery (Light Corruption)
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Test 1: Structural Recovery (5%% corruption)\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    platonic_corrupt_model(model, 5.0);
    RecoveryResult result1 = platonic_recover_structural(model);
    
    if (result1.success && result1.recovery_time_ms < 100.0) {
        printf("\n✓ PASS: Structural recovery successful\n");
        printf("  Time: %.2f ms\n", result1.recovery_time_ms);
        printf("  Iterations: %u\n", result1.iterations_used);
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Structural recovery failed\n");
    }
    
    // Test 2: Symmetry Recovery (Moderate Corruption)
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 2: Symmetry Recovery (10%% corruption)\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    platonic_corrupt_model(model, 10.0);
    RecoveryResult result2 = platonic_recover_symmetry(model);
    
    if (result2.success && result2.recovery_time_ms < 200.0) {
        printf("\n✓ PASS: Symmetry recovery successful\n");
        printf("  Time: %.2f ms\n", result2.recovery_time_ms);
        printf("  Iterations: %u\n", result2.iterations_used);
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Symmetry recovery failed\n");
    }
    
    // Test 3: Prime Recovery (Moderate Corruption)
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 3: Prime-Based Recovery (15%% corruption)\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    platonic_corrupt_model(model, 15.0);
    RecoveryResult result3 = platonic_recover_prime(model);
    
    if (result3.success && result3.recovery_time_ms < 300.0) {
        printf("\n✓ PASS: Prime recovery successful\n");
        printf("  Time: %.2f ms\n", result3.recovery_time_ms);
        printf("  Iterations: %u\n", result3.iterations_used);
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Prime recovery failed\n");
    }
    
    // Test 4: Tetration Recovery (Heavy Corruption)
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 4: Tetration Attractor Recovery (20%% corruption)\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    platonic_corrupt_model(model, 20.0);
    RecoveryResult result4 = platonic_recover_tetration(model);
    
    if (result4.success && result4.recovery_time_ms < 500.0) {
        printf("\n✓ PASS: Tetration recovery successful\n");
        printf("  Time: %.2f ms\n", result4.recovery_time_ms);
        printf("  Iterations: %u\n", result4.iterations_used);
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Tetration recovery failed\n");
    }
    
    // Test 5: Automatic Recovery (Heavy Corruption)
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 5: Automatic Recovery (25%% corruption)\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    platonic_corrupt_model(model, 25.0);
    RecoveryResult result5 = platonic_recover_auto(model);
    
    if (result5.success) {
        printf("\n✓ PASS: Automatic recovery successful\n");
        printf("  Method used: %s\n", result5.method_used);
        printf("  Time: %.2f ms\n", result5.recovery_time_ms);
        printf("  Iterations: %u\n", result5.iterations_used);
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Automatic recovery failed\n");
    }
    
    // Test 6: Model Validation After Recovery
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 6: Model Validation After Recovery\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    if (platonic_model_validate(model) && !model->is_corrupted) {
        printf("✓ PASS: Model is valid and not corrupted\n");
        printf("  Corruption level: %.1f%%\n", model->corruption_level);
        tests_passed++;
    } else {
        printf("✗ FAIL: Model validation failed\n");
    }
    
    // Test 7: Recovery Speed Comparison
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 7: Recovery Speed Comparison\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    printf("\nRecovery Method Performance:\n");
    printf("  1. Structural:  %.2f ms\n", result1.recovery_time_ms);
    printf("  2. Symmetry:    %.2f ms\n", result2.recovery_time_ms);
    printf("  3. Prime:       %.2f ms\n", result3.recovery_time_ms);
    printf("  4. Tetration:   %.2f ms\n", result4.recovery_time_ms);
    printf("  5. Automatic:   %.2f ms\n", result5.recovery_time_ms);
    
    // All methods should be reasonably fast
    if (result1.recovery_time_ms < 100.0 &&
        result2.recovery_time_ms < 200.0 &&
        result3.recovery_time_ms < 300.0 &&
        result4.recovery_time_ms < 500.0 &&
        result5.recovery_time_ms < 600.0) {
        printf("\n✓ PASS: All recovery methods are fast enough\n");
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Some recovery methods are too slow\n");
    }
    
    // Cleanup
    platonic_model_free(model);
    
    // Summary
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Passed: %d/%d\n", tests_passed, tests_total);
    printf("Success Rate: %.1f%%\n", (tests_passed * 100.0) / tests_total);
    printf("\n");
    
    if (tests_passed == tests_total) {
        printf("✓ ALL BLIND RECOVERY TESTS PASSED!\n");
        printf("✓ Models can recover from up to 25%% corruption!\n");
        printf("\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n\n");
        return 1;
    }
}
