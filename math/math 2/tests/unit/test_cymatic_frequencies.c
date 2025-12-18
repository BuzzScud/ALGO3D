/**
 * @file test_cymatic_frequencies.c
 * @brief Unit tests for cymatic frequency integration
 */

#include "ai/cllm_cymatic_frequencies.h"
#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <string.h>
#include <assert.h>

#define TEST_EPSILON 1e-6

/**
 * @brief Test helper: Check if two doubles are approximately equal
 */
static int doubles_equal(double a, double b, double epsilon) {
    return fabs(a - b) < epsilon;
}

/**
 * @brief Test: Basic cymatic correction calculation
 */
static void test_cymatic_correction_basic(void) {
    printf("Testing basic cymatic correction...\n");
    
    // Test with 432 Hz (reference frequency)
    double correction_432 = cymatic_correction(FREQ_432_HZ, 1.0, 0.0);
    assert(correction_432 >= -1.0 && correction_432 <= 1.0);
    
    // Test with 528 Hz
    double correction_528 = cymatic_correction(FREQ_528_HZ, 1.0, 0.0);
    assert(correction_528 >= -1.0 && correction_528 <= 1.0);
    
    // Test with Schumann resonance
    double correction_schumann = cymatic_correction(FREQ_SCHUMANN, 1.0, 0.0);
    assert(correction_schumann >= -1.0 && correction_schumann <= 1.0);
    
    // Test with zero frequency
    double correction_zero = cymatic_correction(0.0, 1.0, 0.0);
    assert(correction_zero == 0.0);
    
    printf("✓ Basic cymatic correction test passed\n");
}

/**
 * @brief Test: Amplitude scaling
 */
static void test_amplitude_scaling(void) {
    printf("Testing amplitude scaling...\n");
    
    double freq = FREQ_432_HZ;
    
    // Test with different amplitudes
    double corr_amp_1 = cymatic_correction(freq, 1.0, 0.0);
    double corr_amp_05 = cymatic_correction(freq, 0.5, 0.0);
    double corr_amp_2 = cymatic_correction(freq, 2.0, 0.0);
    
    // Amplitude should scale the correction proportionally
    assert(doubles_equal(corr_amp_05, corr_amp_1 * 0.5, TEST_EPSILON));
    assert(doubles_equal(corr_amp_2, corr_amp_1 * 2.0, TEST_EPSILON));
    
    printf("✓ Amplitude scaling test passed\n");
}

/**
 * @brief Test: Phase offset
 */
static void test_phase_offset(void) {
    printf("Testing phase offset...\n");
    
    double freq = FREQ_432_HZ;
    
    // Test with different phases
    double corr_phase_0 = cymatic_correction(freq, 1.0, 0.0);
    double corr_phase_pi = cymatic_correction(freq, 1.0, M_PI);
    
    // Phase shift of π should invert the correction
    assert(doubles_equal(corr_phase_pi, -corr_phase_0, TEST_EPSILON));
    
    printf("✓ Phase offset test passed\n");
}

/**
 * @brief Test: Frequency configuration
 */
static void test_frequency_config(void) {
    printf("Testing frequency configuration...\n");
    
    // Test all predefined frequency types
    CymaticFrequency config_432 = cymatic_get_frequency_config(CYMATIC_FREQ_432_HZ);
    assert(config_432.frequency == FREQ_432_HZ);
    assert(config_432.amplitude == 1.0);
    assert(config_432.phase == 0.0);
    
    CymaticFrequency config_528 = cymatic_get_frequency_config(CYMATIC_FREQ_528_HZ);
    assert(config_528.frequency == FREQ_528_HZ);
    
    CymaticFrequency config_schumann = cymatic_get_frequency_config(CYMATIC_FREQ_SCHUMANN);
    assert(config_schumann.frequency == FREQ_SCHUMANN);
    
    CymaticFrequency config_gamma = cymatic_get_frequency_config(CYMATIC_FREQ_GAMMA);
    assert(config_gamma.frequency == FREQ_GAMMA);
    
    // Test correction from config
    double correction = cymatic_correction_from_config(&config_432);
    assert(correction >= -1.0 && correction <= 1.0);
    
    // Test NULL config
    double correction_null = cymatic_correction_from_config(NULL);
    assert(correction_null == 0.0);
    
    printf("✓ Frequency configuration test passed\n");
}

/**
 * @brief Test: Resonance factor calculation
 */
static void test_resonance_factor(void) {
    printf("Testing resonance factor calculation...\n");
    
    // Perfect resonance (same frequency)
    double resonance_same = cymatic_resonance_factor(FREQ_432_HZ, FREQ_432_HZ);
    assert(resonance_same > 0.99); // Should be very close to 1.0
    
    // Octave relationship (2:1 ratio)
    double resonance_octave = cymatic_resonance_factor(FREQ_432_HZ, FREQ_432_HZ * 2.0);
    assert(resonance_octave > 0.9); // Strong resonance
    
    // Fifth relationship (3:2 ratio)
    double resonance_fifth = cymatic_resonance_factor(FREQ_432_HZ, FREQ_432_HZ * 1.5);
    assert(resonance_fifth > 0.9); // Strong resonance
    
    // Non-harmonic relationship
    double resonance_non = cymatic_resonance_factor(FREQ_432_HZ, FREQ_432_HZ * 1.234);
    assert(resonance_non < 0.9); // Weaker resonance
    
    // Zero frequency
    double resonance_zero = cymatic_resonance_factor(0.0, FREQ_432_HZ);
    assert(resonance_zero == 0.0);
    
    printf("✓ Resonance factor test passed\n");
}

/**
 * @brief Test: Harmonic series
 */
static void test_harmonic_series(void) {
    printf("Testing harmonic series...\n");
    
    double base_freq = FREQ_432_HZ;
    
    // Test first 5 harmonics
    for (uint32_t i = 1; i <= 5; i++) {
        double harmonic = cymatic_harmonic(base_freq, i);
        assert(doubles_equal(harmonic, base_freq * i, TEST_EPSILON));
    }
    
    // Test zero harmonic
    double harmonic_zero = cymatic_harmonic(base_freq, 0);
    assert(harmonic_zero == 0.0);
    
    printf("✓ Harmonic series test passed\n");
}

/**
 * @brief Test: Natural harmonic detection
 */
static void test_natural_harmonic_detection(void) {
    printf("Testing natural harmonic detection...\n");
    
    double tolerance = 0.1;
    
    // Test exact harmonics of 432 Hz
    assert(cymatic_is_natural_harmonic(FREQ_432_HZ, tolerance));
    assert(cymatic_is_natural_harmonic(FREQ_432_HZ * 2.0, tolerance));
    assert(cymatic_is_natural_harmonic(FREQ_432_HZ * 3.0, tolerance));
    
    // Test non-harmonics
    assert(!cymatic_is_natural_harmonic(FREQ_432_HZ * 1.5 + 10.0, tolerance));
    
    // Test zero frequency
    assert(!cymatic_is_natural_harmonic(0.0, tolerance));
    
    printf("✓ Natural harmonic detection test passed\n");
}

/**
 * @brief Test: Combined correction from multiple frequencies
 */
static void test_combined_correction(void) {
    printf("Testing combined correction...\n");
    
    // Create array of frequencies
    CymaticFrequency frequencies[3];
    frequencies[0] = cymatic_get_frequency_config(CYMATIC_FREQ_432_HZ);
    frequencies[1] = cymatic_get_frequency_config(CYMATIC_FREQ_528_HZ);
    frequencies[2] = cymatic_get_frequency_config(CYMATIC_FREQ_SCHUMANN);
    
    // Calculate combined correction
    double combined = cymatic_combined_correction(frequencies, 3);
    assert(combined >= -1.0 && combined <= 1.0);
    
    // Test with NULL array
    double combined_null = cymatic_combined_correction(NULL, 3);
    assert(combined_null == 0.0);
    
    // Test with zero count
    double combined_zero = cymatic_combined_correction(frequencies, 0);
    assert(combined_zero == 0.0);
    
    printf("✓ Combined correction test passed\n");
}

/**
 * @brief Test: Frequency names
 */
static void test_frequency_names(void) {
    printf("Testing frequency names...\n");
    
    // Test all frequency type names
    const char* name_432 = cymatic_frequency_name(CYMATIC_FREQ_432_HZ);
    assert(name_432 != NULL);
    assert(strlen(name_432) > 0);
    
    const char* name_528 = cymatic_frequency_name(CYMATIC_FREQ_528_HZ);
    assert(name_528 != NULL);
    
    const char* name_schumann = cymatic_frequency_name(CYMATIC_FREQ_SCHUMANN);
    assert(name_schumann != NULL);
    
    const char* name_gamma = cymatic_frequency_name(CYMATIC_FREQ_GAMMA);
    assert(name_gamma != NULL);
    
    printf("✓ Frequency names test passed\n");
}

/**
 * @brief Test: Schumann resonance harmonics
 */
static void test_schumann_harmonics(void) {
    printf("Testing Schumann resonance harmonics...\n");
    
    // Test first 5 Schumann harmonics
    double h1 = cymatic_schumann_harmonic(1);
    assert(doubles_equal(h1, 7.83, TEST_EPSILON));
    
    double h2 = cymatic_schumann_harmonic(2);
    assert(doubles_equal(h2, 14.3, TEST_EPSILON));
    
    double h3 = cymatic_schumann_harmonic(3);
    assert(doubles_equal(h3, 20.8, TEST_EPSILON));
    
    double h4 = cymatic_schumann_harmonic(4);
    assert(doubles_equal(h4, 27.3, TEST_EPSILON));
    
    double h5 = cymatic_schumann_harmonic(5);
    assert(doubles_equal(h5, 33.8, TEST_EPSILON));
    
    // Test invalid harmonics
    assert(cymatic_schumann_harmonic(0) == 0.0);
    assert(cymatic_schumann_harmonic(6) == 0.0);
    
    printf("✓ Schumann harmonics test passed\n");
}

/**
 * @brief Test: Correction magnitude bounds
 */
static void test_correction_bounds(void) {
    printf("Testing correction magnitude bounds...\n");
    
    // Test various frequencies to ensure corrections are bounded
    double frequencies[] = {
        FREQ_432_HZ, FREQ_528_HZ, FREQ_SCHUMANN, FREQ_GAMMA,
        FREQ_ALPHA, FREQ_THETA, FREQ_DELTA, FREQ_BETA,
        100.0, 1000.0, 10000.0
    };
    
    for (size_t i = 0; i < sizeof(frequencies) / sizeof(frequencies[0]); i++) {
        double correction = cymatic_correction(frequencies[i], 1.0, 0.0);
        assert(correction >= -1.0 && correction <= 1.0);
    }
    
    printf("✓ Correction bounds test passed\n");
}

/**
 * @brief Test: All predefined frequency types
 */
static void test_all_frequency_types(void) {
    printf("Testing all predefined frequency types...\n");
    
    CymaticFrequencyType types[] = {
        CYMATIC_FREQ_432_HZ,
        CYMATIC_FREQ_528_HZ,
        CYMATIC_FREQ_SCHUMANN,
        CYMATIC_FREQ_GAMMA,
        CYMATIC_FREQ_ALPHA,
        CYMATIC_FREQ_THETA,
        CYMATIC_FREQ_DELTA,
        CYMATIC_FREQ_BETA,
        CYMATIC_FREQ_CUSTOM
    };
    
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        CymaticFrequency config = cymatic_get_frequency_config(types[i]);
        assert(config.frequency > 0.0);
        assert(config.amplitude == 1.0);
        assert(config.phase == 0.0);
        
        double correction = cymatic_correction_from_config(&config);
        assert(correction >= -1.0 && correction <= 1.0);
    }
    
    printf("✓ All frequency types test passed\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n=== Cymatic Frequency Tests ===\n\n");
    
    test_cymatic_correction_basic();
    test_amplitude_scaling();
    test_phase_offset();
    test_frequency_config();
    test_resonance_factor();
    test_harmonic_series();
    test_natural_harmonic_detection();
    test_combined_correction();
    test_frequency_names();
    test_schumann_harmonics();
    test_correction_bounds();
    test_all_frequency_types();
    
    printf("\n=== All Cymatic Frequency Tests Passed ===\n\n");
    
    return 0;
}