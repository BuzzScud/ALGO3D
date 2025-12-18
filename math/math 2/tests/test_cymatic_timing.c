/**
 * Unit tests for cymatic timing utilities
 */

#include "ai/cllm_cymatic_frequencies.h"
#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <assert.h>

#define MATH_EPSILON 1e-6

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing %s...\n", #name); \
    if (test_##name()) { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED\n"); \
        tests_failed++; \
    }

/**
 * Test: Timing period constants
 */
int test_period_constants(void) {
    // Verify periods are calculated correctly
    double period_432 = 1.0 / FREQ_432_HZ;
    double period_528 = 1.0 / FREQ_528_HZ;
    double period_schumann = 1.0 / FREQ_SCHUMANN;
    
    if (fabs(PERIOD_432_HZ - period_432) > MATH_EPSILON) return 0;
    if (fabs(PERIOD_528_HZ - period_528) > MATH_EPSILON) return 0;
    if (fabs(PERIOD_SCHUMANN - period_schumann) > MATH_EPSILON) return 0;
    
    return 1;
}

/**
 * Test: Nanosecond period constants
 */
int test_ns_constants(void) {
    // Verify nanosecond conversions
    uint64_t expected_432 = (uint64_t)(PERIOD_432_HZ * NS_PER_SECOND);
    uint64_t expected_528 = (uint64_t)(PERIOD_528_HZ * NS_PER_SECOND);
    uint64_t expected_schumann = (uint64_t)(PERIOD_SCHUMANN * NS_PER_SECOND);
    
    if (NS_432_HZ != expected_432) return 0;
    if (NS_528_HZ != expected_528) return 0;
    if (NS_SCHUMANN != expected_schumann) return 0;
    
    return 1;
}

/**
 * Test: Frequency to nanoseconds conversion
 */
int test_frequency_to_ns(void) {
    // Test 432 Hz
    uint64_t ns_432 = cymatic_frequency_to_ns(FREQ_432_HZ);
    if (ns_432 != NS_432_HZ) return 0;
    
    // Test 528 Hz
    uint64_t ns_528 = cymatic_frequency_to_ns(FREQ_528_HZ);
    if (ns_528 != NS_528_HZ) return 0;
    
    // Test Schumann
    uint64_t ns_schumann = cymatic_frequency_to_ns(FREQ_SCHUMANN);
    if (ns_schumann != NS_SCHUMANN) return 0;
    
    // Test zero frequency
    uint64_t ns_zero = cymatic_frequency_to_ns(0.0);
    if (ns_zero != 0) return 0;
    
    // Test negative frequency
    uint64_t ns_neg = cymatic_frequency_to_ns(-10.0);
    if (ns_neg != 0) return 0;
    
    return 1;
}

/**
 * Test: Nanoseconds to frequency conversion
 */
int test_ns_to_frequency(void) {
    // Test 432 Hz
    double freq_432 = cymatic_ns_to_frequency(NS_432_HZ);
    if (fabs(freq_432 - FREQ_432_HZ) > 0.1) return 0;
    
    // Test 528 Hz
    double freq_528 = cymatic_ns_to_frequency(NS_528_HZ);
    if (fabs(freq_528 - FREQ_528_HZ) > 0.1) return 0;
    
    // Test Schumann
    double freq_schumann = cymatic_ns_to_frequency(NS_SCHUMANN);
    if (fabs(freq_schumann - FREQ_SCHUMANN) > 0.01) return 0;
    
    // Test zero period
    double freq_zero = cymatic_ns_to_frequency(0);
    if (freq_zero != 0.0) return 0;
    
    return 1;
}

/**
 * Test: Round-trip conversion
 */
int test_round_trip_conversion(void) {
    // Test frequencies
    double test_freqs[] = {1.0, 10.0, 100.0, 432.0, 528.0, 1000.0};
    
    for (size_t i = 0; i < sizeof(test_freqs) / sizeof(test_freqs[0]); i++) {
        double freq = test_freqs[i];
        
        // Convert to ns and back
        uint64_t ns = cymatic_frequency_to_ns(freq);
        double freq_back = cymatic_ns_to_frequency(ns);
        
        // Should be approximately equal (within 1%)
        double error = fabs(freq_back - freq) / freq;
        if (error > 0.01) return 0;
    }
    
    return 1;
}

/**
 * Test: Get period for frequency type
 */
int test_get_period_ns(void) {
    // Test all frequency types
    if (cymatic_get_period_ns(CYMATIC_FREQ_432_HZ) != NS_432_HZ) return 0;
    if (cymatic_get_period_ns(CYMATIC_FREQ_528_HZ) != NS_528_HZ) return 0;
    if (cymatic_get_period_ns(CYMATIC_FREQ_SCHUMANN) != NS_SCHUMANN) return 0;
    if (cymatic_get_period_ns(CYMATIC_FREQ_GAMMA) != NS_GAMMA) return 0;
    if (cymatic_get_period_ns(CYMATIC_FREQ_ALPHA) != NS_ALPHA) return 0;
    if (cymatic_get_period_ns(CYMATIC_FREQ_THETA) != NS_THETA) return 0;
    if (cymatic_get_period_ns(CYMATIC_FREQ_DELTA) != NS_DELTA) return 0;
    if (cymatic_get_period_ns(CYMATIC_FREQ_BETA) != NS_BETA) return 0;
    
    // Test custom (should return 0)
    if (cymatic_get_period_ns(CYMATIC_FREQ_CUSTOM) != 0) return 0;
    
    return 1;
}

/**
 * Test: Period values are reasonable
 */
int test_period_values(void) {
    // 432 Hz should be about 2.3 ms
    if (NS_432_HZ < 2000000 || NS_432_HZ > 2500000) return 0;
    
    // 528 Hz should be about 1.9 ms
    if (NS_528_HZ < 1800000 || NS_528_HZ > 2000000) return 0;
    
    // Schumann should be about 127.7 ms
    if (NS_SCHUMANN < 127000000 || NS_SCHUMANN > 128000000) return 0;
    
    // Gamma (40 Hz) should be 25 ms
    if (NS_GAMMA != 25000000) return 0;
    
    return 1;
}

/**
 * Test: Frequency ordering
 */
int test_frequency_ordering(void) {
    // Higher frequency = shorter period
    if (NS_528_HZ >= NS_432_HZ) return 0;  // 528 Hz faster than 432 Hz
    if (NS_GAMMA >= NS_SCHUMANN) return 0;  // 40 Hz faster than 7.83 Hz
    if (NS_BETA >= NS_ALPHA) return 0;      // 20 Hz faster than 10 Hz
    
    return 1;
}

/**
 * Test: Schumann harmonics with timing
 */
int test_schumann_harmonics_timing(void) {
    // Get fundamental and harmonics
    double fundamental = cymatic_schumann_harmonic(1);
    double harmonic2 = cymatic_schumann_harmonic(2);
    double harmonic3 = cymatic_schumann_harmonic(3);
    
    // Convert to periods
    uint64_t period1 = cymatic_frequency_to_ns(fundamental);
    uint64_t period2 = cymatic_frequency_to_ns(harmonic2);
    uint64_t period3 = cymatic_frequency_to_ns(harmonic3);
    
    // Higher harmonics should have shorter periods
    if (period2 >= period1) return 0;
    if (period3 >= period2) return 0;
    
    return 1;
}

/**
 * Test: Timing precision
 */
int test_timing_precision(void) {
    // Nanosecond precision should be sufficient for all frequencies
    
    // For 432 Hz (2.3 ms period), 1 ns is 0.00004% error
    double error_432 = 1.0 / NS_432_HZ;
    if (error_432 > 0.001) return 0;  // Less than 0.1% error
    
    // For Schumann (127.7 ms period), 1 ns is negligible
    double error_schumann = 1.0 / NS_SCHUMANN;
    if (error_schumann > 0.00001) return 0;  // Less than 0.001% error
    
    return 1;
}

int main(void) {
    printf("=== Cymatic Timing Utilities Unit Tests ===\n\n");
    
    TEST(period_constants);
    TEST(ns_constants);
    TEST(frequency_to_ns);
    TEST(ns_to_frequency);
    TEST(round_trip_conversion);
    TEST(get_period_ns);
    TEST(period_values);
    TEST(frequency_ordering);
    TEST(schumann_harmonics_timing);
    TEST(timing_precision);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}