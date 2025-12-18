/**
 * @file test_phase4_oscillation.c
 * @brief Test Phase 4: Oscillation Detection
 * 
 * Tests FFT-based oscillation detection with pure crystalline math
 */

#include "../include/oscillation_detection.h"
#include "../include/ecdlp_integration.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>

// Test colors
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

void print_test_header(const char* test_name) {
    printf("\n");
    printf("========================================\n");
    printf("TEST: %s\n", test_name);
    printf("========================================\n");
}

void print_test_result(const char* test_name, bool passed) {
    if (passed) {
        printf("%s[PASS]%s %s\n", GREEN, RESET, test_name);
    } else {
        printf("%s[FAIL]%s %s\n", RED, RESET, test_name);
    }
}

// ============================================================================
// TEST 1: Power of 2 Check
// ============================================================================

bool test_power_of_2() {
    print_test_header("Power of 2 Check");
    
    bool success = true;
    
    // Test powers of 2
    uint32_t powers[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    for (int i = 0; i < 11; i++) {
        if (!is_power_of_2(powers[i])) {
            printf("  FAIL: %u should be power of 2\n", powers[i]);
            success = false;
        }
    }
    
    // Test non-powers of 2
    uint32_t non_powers[] = {3, 5, 6, 7, 9, 10, 15, 100, 1000};
    for (int i = 0; i < 9; i++) {
        if (is_power_of_2(non_powers[i])) {
            printf("  FAIL: %u should NOT be power of 2\n", non_powers[i]);
            success = false;
        }
    }
    
    if (success) {
        printf("  All power of 2 checks passed\n");
    }
    
    return success;
}

// ============================================================================
// TEST 2: Next Power of 2
// ============================================================================

bool test_next_power_of_2() {
    print_test_header("Next Power of 2");
    
    bool success = true;
    
    struct {
        uint32_t input;
        uint32_t expected;
    } tests[] = {
        {0, 1},
        {1, 1},
        {2, 2},
        {3, 4},
        {5, 8},
        {100, 128},
        {1000, 1024},
        {1024, 1024}
    };
    
    for (int i = 0; i < 8; i++) {
        uint32_t result = next_power_of_2(tests[i].input);
        if (result != tests[i].expected) {
            printf("  FAIL: next_power_of_2(%u) = %u, expected %u\n",
                   tests[i].input, result, tests[i].expected);
            success = false;
        }
    }
    
    if (success) {
        printf("  All next power of 2 tests passed\n");
    }
    
    return success;
}

// ============================================================================
// TEST 3: DFT on Simple Signal
// ============================================================================

bool test_dft_simple() {
    print_test_header("DFT on Simple Signal");
    
    // Create simple sine wave
    uint32_t n = 16;
    double* input = (double*)malloc(n * sizeof(double));
    Complex* output = (Complex*)malloc(n * sizeof(Complex));
    
    if (!input || !output) {
        if (input) free(input);
        if (output) free(output);
        return false;
    }
    
    // Generate sine wave at frequency 2
    for (uint32_t i = 0; i < n; i++) {
        double t = (double)i / (double)n;
        input[i] = math_sin(2.0 * 2.0 * 3.14159265358979323846 * t);
    }
    
    // Compute DFT
    dft_crystalline(input, output, n);
    
    // Check that frequency 2 has highest magnitude
    double max_mag = 0.0;
    uint32_t max_idx = 0;
    
    for (uint32_t i = 0; i < n; i++) {
        double mag = complex_magnitude(&output[i]);
        if (mag > max_mag) {
            max_mag = mag;
            max_idx = i;
        }
    }
    
    printf("  Peak at index %u (expected 2)\n", max_idx);
    printf("  Peak magnitude: %.6f\n", max_mag);
    
    bool success = (max_idx == 2);
    
    free(input);
    free(output);
    
    return success;
}

// ============================================================================
// TEST 4: FFT on Simple Signal
// ============================================================================

bool test_fft_simple() {
    print_test_header("FFT on Simple Signal");
    
    // Create simple sine wave
    uint32_t n = 64;  // Power of 2
    double* input = (double*)malloc(n * sizeof(double));
    Complex* output = (Complex*)malloc(n * sizeof(Complex));
    
    if (!input || !output) {
        if (input) free(input);
        if (output) free(output);
        return false;
    }
    
    // Generate sine wave at frequency 4
    for (uint32_t i = 0; i < n; i++) {
        double t = (double)i / (double)n;
        input[i] = math_sin(2.0 * 4.0 * 3.14159265358979323846 * t);
    }
    
    // Compute FFT
    bool fft_success = fft_crystalline(input, output, n);
    
    if (!fft_success) {
        printf("  FFT computation failed\n");
        free(input);
        free(output);
        return false;
    }
    
    // Check that frequency 4 has highest magnitude
    double max_mag = 0.0;
    uint32_t max_idx = 0;
    
    for (uint32_t i = 0; i < n/2; i++) {  // Only check first half (Nyquist)
        double mag = complex_magnitude(&output[i]);
        if (mag > max_mag) {
            max_mag = mag;
            max_idx = i;
        }
    }
    
    printf("  Peak at index %u (expected 4)\n", max_idx);
    printf("  Peak magnitude: %.6f\n", max_mag);
    
    bool success = (max_idx == 4);
    
    free(input);
    free(output);
    
    return success;
}

// ============================================================================
// TEST 5: Power Spectrum
// ============================================================================

bool test_power_spectrum() {
    print_test_header("Power Spectrum");
    
    uint32_t n = 64;
    double* input = (double*)malloc(n * sizeof(double));
    Complex* fft_output = (Complex*)malloc(n * sizeof(Complex));
    double* power_spectrum = (double*)malloc(n * sizeof(double));
    
    if (!input || !fft_output || !power_spectrum) {
        if (input) free(input);
        if (fft_output) free(fft_output);
        if (power_spectrum) free(power_spectrum);
        return false;
    }
    
    // Generate sine wave
    for (uint32_t i = 0; i < n; i++) {
        double t = (double)i / (double)n;
        input[i] = math_sin(2.0 * 8.0 * 3.14159265358979323846 * t);
    }
    
    // Compute FFT
    if (!fft_crystalline(input, fft_output, n)) {
        free(input);
        free(fft_output);
        free(power_spectrum);
        return false;
    }
    
    // Compute power spectrum
    compute_power_spectrum(fft_output, power_spectrum, n);
    
    // Find dominant frequency
    double dominant_freq = find_dominant_frequency(power_spectrum, n, 1.0);
    
    // Expected: 8 cycles / 64 samples * 1.0 sampling_rate = 0.125 Hz
    printf("  Dominant frequency: %.6f Hz (expected 0.125)\n", dominant_freq);
    
    bool success = (dominant_freq >= 0.12 && dominant_freq <= 0.13);
    
    free(input);
    free(fft_output);
    free(power_spectrum);
    
    return success;
}

// ============================================================================
// TEST 6: EC Trajectory Sampling
// ============================================================================

bool test_ec_trajectory_sampling() {
    print_test_header("EC Trajectory Sampling");
    
    // Create ECDLP instance
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) {
        printf("  Failed to create ECDLP instance\n");
        return false;
    }
    
    // Sample trajectory
    uint32_t num_samples = 32;
    ECTrajectory* trajectory = sample_ec_trajectory(instance, 1, 1, num_samples);
    
    if (!trajectory) {
        printf("  Failed to sample trajectory\n");
        ecdlp_free_instance(instance);
        return false;
    }
    
    printf("  Sampled %u points\n", trajectory->num_samples);
    printf("  k range: %lu to %lu\n", 
           trajectory->samples[0].k, 
           trajectory->samples[num_samples-1].k);
    printf("  First sample magnitude: %.6f\n", trajectory->samples[0].magnitude);
    printf("  Last sample magnitude: %.6f\n", trajectory->samples[num_samples-1].magnitude);
    
    bool success = (trajectory->num_samples == num_samples);
    
    free_ec_trajectory(trajectory);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// TEST 7: Oscillation Detection
// ============================================================================

bool test_oscillation_detection() {
    print_test_header("Oscillation Detection");
    
    // Create ECDLP instance
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) {
        printf("  Failed to create ECDLP instance\n");
        return false;
    }
    
    // Sample trajectory (power of 2 for FFT)
    uint32_t num_samples = 64;
    ECTrajectory* trajectory = sample_ec_trajectory(instance, 1, 1, num_samples);
    
    if (!trajectory) {
        printf("  Failed to sample trajectory\n");
        ecdlp_free_instance(instance);
        return false;
    }
    
    // Detect oscillations
    OscillationMap* map = detect_oscillations(trajectory);
    
    if (!map) {
        printf("  Failed to detect oscillations\n");
        free_ec_trajectory(trajectory);
        ecdlp_free_instance(instance);
        return false;
    }
    
    printf("  Detected oscillations in %u dimensions\n", map->num_dimensions);
    printf("  Global amplitude: %.6f\n", map->global_amplitude);
    printf("  Converging: %s\n", map->is_converging ? "YES" : "NO");
    
    // Print first 3 dimensions
    printf("\n  First 3 dimensions:\n");
    for (int i = 0; i < 3; i++) {
        printf("    Dim %d: freq=%.4f, amp=%.4f, stable=%s\n",
               i,
               map->signatures[i].frequency,
               map->signatures[i].amplitude,
               map->signatures[i].is_stable ? "YES" : "NO");
    }
    
    bool success = (map->num_dimensions == 15);
    
    free_oscillation_map(map);
    free_ec_trajectory(trajectory);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// TEST 8: Cross-Correlation
// ============================================================================

bool test_cross_correlation() {
    print_test_header("Cross-Correlation");
    
    // Create ECDLP instance
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) return false;
    
    // Sample trajectory
    ECTrajectory* trajectory = sample_ec_trajectory(instance, 1, 1, 64);
    if (!trajectory) {
        ecdlp_free_instance(instance);
        return false;
    }
    
    // Compute cross-correlation between dimensions 0 and 1
    double corr_01 = compute_cross_correlation(trajectory, 0, 1);
    double corr_00 = compute_cross_correlation(trajectory, 0, 0);  // Should be 1.0
    
    printf("  Correlation(dim0, dim1): %.6f\n", corr_01);
    printf("  Correlation(dim0, dim0): %.6f (expected 1.0)\n", corr_00);
    
    bool success = (corr_00 > 0.99 && corr_00 < 1.01);
    
    free_ec_trajectory(trajectory);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║  PHASE 4: OSCILLATION DETECTION TESTS ║\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run tests
    bool result;
    
    result = test_power_of_2();
    print_test_result("Power of 2 Check", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_next_power_of_2();
    print_test_result("Next Power of 2", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_dft_simple();
    print_test_result("DFT on Simple Signal", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_fft_simple();
    print_test_result("FFT on Simple Signal", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_power_spectrum();
    print_test_result("Power Spectrum", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_ec_trajectory_sampling();
    print_test_result("EC Trajectory Sampling", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_oscillation_detection();
    print_test_result("Oscillation Detection", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_cross_correlation();
    print_test_result("Cross-Correlation", result);
    total_tests++; if (result) passed_tests++;
    
    // Summary
    printf("\n");
    printf("========================================\n");
    printf("SUMMARY\n");
    printf("========================================\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %s%d%s\n", GREEN, passed_tests, RESET);
    printf("Failed: %s%d%s\n", RED, total_tests - passed_tests, RESET);
    printf("Success rate: %.1f%%\n", 100.0 * passed_tests / total_tests);
    printf("\n");
    
    return (passed_tests == total_tests) ? 0 : 1;
}