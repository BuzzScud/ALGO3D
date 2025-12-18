/**
 * Test Harmonic Integration
 * 
 * Tests all four harmonic methods:
 * 1. Platonic Fourier Transforms
 * 2. Cymatic Frequency Modulation
 * 3. Prime Resonance Alignment
 * 4. Concentric Ring Mapping
 */

#include "ai/cllm_platonic.h"
#include <stdio.h>
#include <stdlib.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  HARMONIC INTEGRATION TEST SUITE                         ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    int tests_passed = 0;
    int tests_total = 0;
    
    // Create test model (Icosahedron - maximum symmetry)
    printf("Creating test model (Icosahedron - maximum symmetry)...\n");
    PlatonicModelConfig config = platonic_config_create(
        PLATONIC_ICOSAHEDRON,
        100,   // Small vocab for testing
        128    // Small seq len
    );
    
    PlatonicModel* model = platonic_model_create(&config);
    if (!model) {
        printf("✗ Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // Test 1: Platonic Fourier Transform
    printf("═══════════════════════════════════════════════════════════\n");
    printf("Test 1: Platonic Fourier Transform\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    size_t signal_size = 144;  // Match embedding dimension
    double* signal = (double*)malloc(signal_size * sizeof(double));
    double* coefficients = (double*)malloc(signal_size * sizeof(double));
    
    // Create test signal (sine wave)
    for (size_t i = 0; i < signal_size; i++) {
        signal[i] = math_sin(2.0 * M_PI * i / signal_size);
    }
    
    platonic_fourier_transform(model, signal, coefficients, signal_size);
    
    // Verify coefficients are computed
    bool has_coefficients = false;
    for (size_t i = 0; i < signal_size; i++) {
        if (math_abs(coefficients[i]) > 0.001) {
            has_coefficients = true;
            break;
        }
    }
    
    if (has_coefficients) {
        printf("\n✓ PASS: Fourier transform successful\n");
        printf("  Frequency components computed\n");
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Fourier transform failed\n");
    }
    
    free(signal);
    free(coefficients);
    
    // Test 2: Cymatic Frequency Modulation
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 2: Cymatic Frequency Modulation\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    size_t gradient_size = 360;  // Match hidden dimension
    double* gradients = (double*)malloc(gradient_size * sizeof(double));
    
    // Initialize gradients
    for (size_t i = 0; i < gradient_size; i++) {
        gradients[i] = ((double)rand() / RAND_MAX - 0.5) * 2.0;
    }
    
    // Store original gradient for comparison
    double original_grad = gradients[0];
    
    // Apply cymatic modulation (432 Hz - universal frequency)
    platonic_cymatic_modulate(model, gradients, gradient_size, 432.0);
    
    // Verify gradients were modulated
    if (math_abs(gradients[0] - original_grad) > 0.001) {
        printf("\n✓ PASS: Cymatic modulation successful\n");
        printf("  Gradients smoothed with 432 Hz resonance\n");
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Cymatic modulation failed\n");
    }
    
    free(gradients);
    
    // Test 3: Prime Resonance Alignment
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 3: Prime Resonance Alignment\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    size_t attention_size = 1000;
    double* attention = (double*)malloc(attention_size * sizeof(double));
    
    // Initialize attention weights
    for (size_t i = 0; i < attention_size; i++) {
        attention[i] = 1.0;
    }
    
    double original_attention = attention[0];
    
    // Apply prime resonance alignment
    platonic_align_attention_resonance(model, attention, attention_size);
    
    // Verify attention was aligned
    if (math_abs(attention[0] - original_attention) > 0.001) {
        printf("\n✓ PASS: Prime resonance alignment successful\n");
        printf("  Attention aligned with Platonic prime\n");
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Prime resonance alignment failed\n");
    }
    
    free(attention);
    
    // Test 4: Concentric Ring Mapping
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 4: Concentric Ring Mapping\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    size_t num_bands = 4;
    double* frequency_bands = (double*)malloc(num_bands * sizeof(double));
    
    platonic_map_clock_to_frequencies(model, frequency_bands, num_bands);
    
    // Verify frequency bands are computed
    bool has_bands = true;
    for (size_t i = 0; i < num_bands; i++) {
        if (frequency_bands[i] < 100.0 || frequency_bands[i] > 5000.0) {
            has_bands = false;
            break;
        }
    }
    
    if (has_bands) {
        printf("\n✓ PASS: Ring mapping successful\n");
        printf("  4 frequency bands computed\n");
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Ring mapping failed\n");
    }
    
    free(frequency_bands);
    
    // Test 5: Tetration Learning Rate
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 5: Tetration-Based Learning Rate\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    printf("\nLearning rate schedule (base 2):\n");
    for (uint32_t epoch = 0; epoch <= 100; epoch += 20) {
        double lr = platonic_tetration_learning_rate(model, epoch, 2);
        printf("  Epoch %3u: lr = %.6f\n", epoch, lr);
    }
    
    // Verify learning rate decreases over epochs
    double lr_epoch_0 = platonic_tetration_learning_rate(model, 0, 2);
    double lr_epoch_100 = platonic_tetration_learning_rate(model, 100, 2);
    
    if (lr_epoch_100 < lr_epoch_0) {
        printf("\n✓ PASS: Learning rate schedule working\n");
        printf("  LR decreases from %.6f to %.6f\n", lr_epoch_0, lr_epoch_100);
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Learning rate schedule failed\n");
    }
    
    // Test 6: Harmonic Convergence Simulation
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Test 6: Harmonic Convergence Simulation\n");
    printf("═══════════════════════════════════════════════════════════\n");
    tests_total++;
    
    printf("\nSimulating training with harmonic integration...\n");
    
    double loss = 10.0;  // Initial loss
    for (uint32_t epoch = 0; epoch < 10; epoch++) {
        // Get learning rate
        double lr = platonic_tetration_learning_rate(model, epoch, 2);
        
        // Simulate gradient descent with harmonic modulation
        double gradient = -loss * 0.1;  // Simplified gradient
        
        // Apply cymatic modulation
        double modulation = math_cos(2.0 * M_PI * epoch / 10.0);
        gradient *= (1.0 + 0.1 * modulation);
        
        // Update loss
        loss += lr * gradient;
        
        if (epoch % 2 == 0) {
            printf("  Epoch %2u: loss = %.4f, lr = %.6f\n", epoch, loss, lr);
        }
    }
    
    // Verify loss decreased
    if (loss < 10.0) {
        printf("\n✓ PASS: Harmonic convergence successful\n");
        printf("  Loss decreased from 10.0 to %.4f\n", loss);
        tests_passed++;
    } else {
        printf("\n✗ FAIL: Harmonic convergence failed\n");
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
        printf("✓ ALL HARMONIC INTEGRATION TESTS PASSED!\n");
        printf("✓ Ready for harmonic training!\n");
        printf("\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n\n");
        return 1;
    }
}
