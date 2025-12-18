#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

#include "cllm_angular_position.h"
#include "math/types.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== Testing: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ %s PASSED\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ %s FAILED\n", #name); \
        tests_failed++; \
    }

#define MATH_EPSILON 1e-6

// ============================================================================
// INDIVIDUAL TERM TESTS
// ============================================================================

int test_spiral_term() {
    // Test spiral term: k·π(1+√5)
    
    // For k=0, should be 0
    double term0 = angular_position_spiral_term(0);
    if (fabs(term0) > MATH_EPSILON) {
        printf("ERROR: spiral_term(0) = %.6f, expected 0\n", term0);
        return 0;
    }
    
    // For k=1, should be π(1+√5) ≈ 7.024815
    double term1 = angular_position_spiral_term(1);
    double expected1 = M_PI * (1.0 + sqrt(5.0));
    if (fabs(term1 - expected1) > MATH_EPSILON) {
        printf("ERROR: spiral_term(1) = %.6f, expected %.6f\n", term1, expected1);
        return 0;
    }
    
    // For k=10
    double term10 = angular_position_spiral_term(10);
    double expected10 = 10.0 * M_PI * (1.0 + sqrt(5.0));
    if (fabs(term10 - expected10) > MATH_EPSILON) {
        printf("ERROR: spiral_term(10) = %.6f, expected %.6f\n", term10, expected10);
        return 0;
    }
    
    return 1;
}

int test_index_term() {
    // Test index term: (n-1)·2π/(12·ln3)
    
    // For n=1, should be 0
    double term1 = angular_position_index_term(1);
    if (fabs(term1) > MATH_EPSILON) {
        printf("ERROR: index_term(1) = %.6f, expected 0\n", term1);
        return 0;
    }
    
    // For n=2, should be 2π/(12·ln3)
    double term2 = angular_position_index_term(2);
    double expected2 = (2.0 * M_PI) / (12.0 * LN_3);
    if (fabs(term2 - expected2) > MATH_EPSILON) {
        printf("ERROR: index_term(2) = %.6f, expected %.6f\n", term2, expected2);
        return 0;
    }
    
    // For n=0 or negative, should be 0
    double term0 = angular_position_index_term(0);
    if (fabs(term0) > MATH_EPSILON) {
        printf("ERROR: index_term(0) = %.6f, expected 0\n", term0);
        return 0;
    }
    
    return 1;
}

int test_phonetic_term() {
    // Test phonetic term: log₃(ν(λ))
    
    // Test with wavelength = 1.0 m
    double wavelength = 1.0;
    double term = angular_position_phonetic_term(wavelength);
    
    // ν = 343/1.0 = 343 Hz
    // log₃(343) = ln(343)/ln(3) ≈ 5.32
    double frequency = 343.0;
    double expected = log(frequency) / LN_3;
    
    if (fabs(term - expected) > MATH_EPSILON) {
        printf("ERROR: phonetic_term(1.0) = %.6f, expected %.6f\n", term, expected);
        return 0;
    }
    
    // Test with zero or negative wavelength
    double term_zero = angular_position_phonetic_term(0.0);
    if (fabs(term_zero) > MATH_EPSILON) {
        printf("ERROR: phonetic_term(0) should be 0\n");
        return 0;
    }
    
    return 1;
}

int test_omega_correction() {
    // Test omega correction: ω(p) = (3/144000)·f(p)
    
    // Test at 144000 (should have maximum correction)
    double omega_144000 = angular_position_omega_correction(144000);
    double lambda = cllm_get_einstein_lambda();
    
    // At 144000, distance = 0, so f(p) = 1
    if (fabs(omega_144000 - lambda) > MATH_EPSILON) {
        printf("ERROR: omega(144000) = %.10f, expected %.10f\n", omega_144000, lambda);
        return 0;
    }
    
    // Test at twin primes (should have double correction)
    double omega_lower = angular_position_omega_correction(TWIN_PRIME_LOWER);
    double omega_upper = angular_position_omega_correction(TWIN_PRIME_UPPER);
    
    // Twin primes should have larger correction
    if (omega_lower <= lambda || omega_upper <= lambda) {
        printf("ERROR: Twin prime corrections should be larger than lambda\n");
        return 0;
    }
    
    // Test far from 144000 (should have smaller correction)
    double omega_far = angular_position_omega_correction(100000);
    if (omega_far >= lambda) {
        printf("ERROR: Far correction should be smaller than lambda\n");
        return 0;
    }
    
    return 1;
}

int test_psi_correction() {
    // Test psi correction: ψ(p) based on Plimpton 322
    
    // Test with a prime
    double psi = angular_position_psi_correction(119);
    
    // Should return some correction value
    // (exact value depends on nearest Plimpton triple)
    
    // Just verify it's finite and reasonable
    if (!isfinite(psi)) {
        printf("ERROR: psi correction is not finite\n");
        return 0;
    }
    
    if (fabs(psi) > 2.0 * M_PI) {
        printf("ERROR: psi correction too large: %.6f\n", psi);
        return 0;
    }
    
    return 1;
}

// ============================================================================
// HELPER FUNCTION TESTS
// ============================================================================

int test_normalize() {
    // Test angle normalization
    
    // Test positive angle
    double theta1 = 3.0 * M_PI;
    double norm1 = angular_position_normalize(theta1);
    double expected1 = M_PI;
    
    if (fabs(norm1 - expected1) > MATH_EPSILON) {
        printf("ERROR: normalize(3π) = %.6f, expected %.6f\n", norm1, expected1);
        return 0;
    }
    
    // Test negative angle
    double theta2 = -M_PI / 2.0;
    double norm2 = angular_position_normalize(theta2);
    double expected2 = 3.0 * M_PI / 2.0;
    
    if (fabs(norm2 - expected2) > MATH_EPSILON) {
        printf("ERROR: normalize(-π/2) = %.6f, expected %.6f\n", norm2, expected2);
        return 0;
    }
    
    // Test already normalized
    double theta3 = M_PI;
    double norm3 = angular_position_normalize(theta3);
    
    if (fabs(norm3 - theta3) > MATH_EPSILON) {
        printf("ERROR: normalize(π) = %.6f, expected %.6f\n", norm3, theta3);
        return 0;
    }
    
    return 1;
}

int test_clock_position() {
    // Test clock position conversion
    
    int hour;
    double minute;
    
    // Test 0 radians = 12 o'clock
    angular_position_to_clock(0.0, &hour, &minute);
    if (hour != 0 || fabs(minute) > MATH_EPSILON) {
        printf("ERROR: 0 rad should be 0:00, got %d:%.2f\n", hour, minute);
        return 0;
    }
    
    // Test π/2 radians = 3 o'clock
    angular_position_to_clock(M_PI / 2.0, &hour, &minute);
    if (hour != 3 || fabs(minute) > MATH_EPSILON) {
        printf("ERROR: π/2 rad should be 3:00, got %d:%.2f\n", hour, minute);
        return 0;
    }
    
    // Test π radians = 6 o'clock
    angular_position_to_clock(M_PI, &hour, &minute);
    if (hour != 6 || fabs(minute) > MATH_EPSILON) {
        printf("ERROR: π rad should be 6:00, got %d:%.2f\n", hour, minute);
        return 0;
    }
    
    // Test 3π/2 radians = 9 o'clock
    angular_position_to_clock(3.0 * M_PI / 2.0, &hour, &minute);
    if (hour != 9 || fabs(minute) > MATH_EPSILON) {
        printf("ERROR: 3π/2 rad should be 9:00, got %d:%.2f\n", hour, minute);
        return 0;
    }
    
    return 1;
}

int test_symmetry_group() {
    // Test symmetry group calculation
    
    if (angular_position_symmetry_group(5) != 5) {
        printf("ERROR: symmetry_group(5) != 5\n");
        return 0;
    }
    
    if (angular_position_symmetry_group(13) != 1) {
        printf("ERROR: symmetry_group(13) != 1\n");
        return 0;
    }
    
    if (angular_position_symmetry_group(144000) != 0) {
        printf("ERROR: symmetry_group(144000) != 0\n");
        return 0;
    }
    
    return 1;
}

int test_boundary_detection() {
    // Test 144000 boundary detection
    
    double distance;
    
    // Test at 144000
    if (!angular_position_is_near_boundary(144000, &distance)) {
        printf("ERROR: 144000 should be near boundary\n");
        return 0;
    }
    
    if (fabs(distance) > MATH_EPSILON) {
        printf("ERROR: Distance at 144000 should be 0, got %.2f\n", distance);
        return 0;
    }
    
    // Test at twin primes
    if (!angular_position_is_near_boundary(TWIN_PRIME_LOWER, NULL)) {
        printf("ERROR: Twin prime lower should be near boundary\n");
        return 0;
    }
    
    if (!angular_position_is_near_boundary(TWIN_PRIME_UPPER, NULL)) {
        printf("ERROR: Twin prime upper should be near boundary\n");
        return 0;
    }
    
    // Test far from boundary
    if (angular_position_is_near_boundary(100000, NULL)) {
        printf("ERROR: 100000 should not be near boundary\n");
        return 0;
    }
    
    return 1;
}

int test_twin_prime_detection() {
    // Test twin prime detection
    
    if (!angular_position_is_twin_prime(TWIN_PRIME_LOWER)) {
        printf("ERROR: %lu should be twin prime\n", (unsigned long)TWIN_PRIME_LOWER);
        return 0;
    }
    
    if (!angular_position_is_twin_prime(TWIN_PRIME_UPPER)) {
        printf("ERROR: %lu should be twin prime\n", (unsigned long)TWIN_PRIME_UPPER);
        return 0;
    }
    
    if (angular_position_is_twin_prime(144000)) {
        printf("ERROR: 144000 should not be twin prime\n");
        return 0;
    }
    
    return 1;
}

// ============================================================================
// FREQUENCY/WAVELENGTH TESTS
// ============================================================================

int test_wavelength_frequency_conversion() {
    // Test wavelength to frequency conversion
    
    double wavelength = 1.0;  // 1 meter
    double frequency = wavelength_to_frequency(wavelength);
    double expected_freq = 343.0;  // Hz
    
    if (fabs(frequency - expected_freq) > MATH_EPSILON) {
        printf("ERROR: wavelength_to_frequency(1.0) = %.2f, expected %.2f\n", 
               frequency, expected_freq);
        return 0;
    }
    
    // Test round-trip conversion
    double wavelength2 = frequency_to_wavelength(frequency);
    if (fabs(wavelength2 - wavelength) > MATH_EPSILON) {
        printf("ERROR: Round-trip conversion failed\n");
        return 0;
    }
    
    return 1;
}

int test_phonetic_wavelength() {
    // Test phonetic wavelength mapping
    
    // Test vowels (should have longer wavelengths)
    double lambda_a = get_phonetic_wavelength('a');
    double lambda_i = get_phonetic_wavelength('i');
    
    if (lambda_a <= 0.0 || lambda_i <= 0.0) {
        printf("ERROR: Phonetic wavelengths should be positive\n");
        return 0;
    }
    
    // Test consonants (should have shorter wavelengths)
    double lambda_s = get_phonetic_wavelength('s');
    double lambda_m = get_phonetic_wavelength('m');
    
    if (lambda_s >= lambda_m) {
        printf("ERROR: Sibilant 's' should have shorter wavelength than nasal 'm'\n");
        return 0;
    }
    
    // Test case insensitivity
    double lambda_A = get_phonetic_wavelength('A');
    if (fabs(lambda_A - lambda_a) > MATH_EPSILON) {
        printf("ERROR: Phonetic wavelength should be case-insensitive\n");
        return 0;
    }
    
    return 1;
}

// ============================================================================
// PLIMPTON 322 TESTS
// ============================================================================

int test_plimpton_triple_search() {
    // Test finding nearest Plimpton triple
    
    PlimptonTriple triple;
    int index = find_nearest_plimpton_triple(119, &triple);
    
    if (index < 0) {
        printf("ERROR: Should find nearest triple for 119\n");
        return 0;
    }
    
    // Verify it's a valid Pythagorean triple
    uint64_t a_sq = triple.a * triple.a;
    uint64_t b_sq = triple.b * triple.b;
    uint64_t c_sq = triple.c * triple.c;
    
    if (a_sq + b_sq != c_sq) {
        printf("ERROR: Found triple is not Pythagorean: %lu² + %lu² != %lu²\n",
               (unsigned long)triple.a, (unsigned long)triple.b, (unsigned long)triple.c);
        return 0;
    }
    
    return 1;
}

// ============================================================================
// COMPLETE ANGULAR POSITION TESTS
// ============================================================================

int test_angular_position_calculation() {
    // Test complete angular position calculation
    
    AngularPosition pos;
    angular_position_calculate(5, 3, 1, 1.0, &pos);
    
    // Verify input parameters
    if (pos.prime != 5 || pos.prime_index != 3 || pos.dimension != 1) {
        printf("ERROR: Input parameters not set correctly\n");
        return 0;
    }
    
    // Verify theta is finite
    if (!isfinite(pos.theta)) {
        printf("ERROR: theta is not finite\n");
        return 0;
    }
    
    // Verify normalized theta is in [0, 2π)
    if (pos.theta_normalized < 0.0 || pos.theta_normalized >= 2.0 * M_PI) {
        printf("ERROR: theta_normalized out of range: %.6f\n", pos.theta_normalized);
        return 0;
    }
    
    // Verify clock position
    if (pos.clock_hour < 0 || pos.clock_hour >= 12) {
        printf("ERROR: clock_hour out of range: %d\n", pos.clock_hour);
        return 0;
    }
    
    // Verify symmetry group
    if (pos.symmetry_group != 5) {
        printf("ERROR: symmetry_group should be 5, got %d\n", pos.symmetry_group);
        return 0;
    }
    
    return 1;
}

int test_angular_position_144000() {
    // Test angular position at 144000 boundary
    
    AngularPosition pos;
    angular_position_calculate(144000, 1000, 1, 1.0, &pos);
    
    // Should be marked as near boundary
    if (!pos.is_near_144000) {
        printf("ERROR: 144000 should be marked as near boundary\n");
        return 0;
    }
    
    // Distance should be 0
    if (fabs(pos.distance_to_144000) > MATH_EPSILON) {
        printf("ERROR: Distance to 144000 should be 0, got %.2f\n", pos.distance_to_144000);
        return 0;
    }
    
    // Omega correction should be maximum
    double lambda = cllm_get_einstein_lambda();
    if (fabs(pos.omega_correction - lambda) > MATH_EPSILON) {
        printf("ERROR: Omega correction at 144000 should be lambda\n");
        return 0;
    }
    
    return 1;
}

int test_angular_position_twin_primes() {
    // Test angular position at twin primes
    
    AngularPosition pos_lower, pos_upper;
    angular_position_calculate(TWIN_PRIME_LOWER, 1000, 1, 1.0, &pos_lower);
    angular_position_calculate(TWIN_PRIME_UPPER, 1000, 1, 1.0, &pos_upper);
    
    // Both should be marked as twin primes
    if (!pos_lower.is_twin_prime) {
        printf("ERROR: %lu should be marked as twin prime\n", 
               (unsigned long)TWIN_PRIME_LOWER);
        return 0;
    }
    
    if (!pos_upper.is_twin_prime) {
        printf("ERROR: %lu should be marked as twin prime\n",
               (unsigned long)TWIN_PRIME_UPPER);
        return 0;
    }
    
    // Both should have enhanced omega correction
    double lambda = cllm_get_einstein_lambda();
    if (pos_lower.omega_correction <= lambda || pos_upper.omega_correction <= lambda) {
        printf("ERROR: Twin primes should have enhanced omega correction\n");
        return 0;
    }
    
    return 1;
}

int test_angular_position_validation() {
    // Test angular position validation
    
    AngularPosition pos;
    angular_position_calculate(17, 7, 2, 0.5, &pos);
    
    if (!angular_position_validate(&pos)) {
        printf("ERROR: Valid position failed validation\n");
        return 0;
    }
    
    // Test invalid position
    pos.theta_normalized = -1.0;  // Invalid
    if (angular_position_validate(&pos)) {
        printf("ERROR: Invalid position passed validation\n");
        return 0;
    }
    
    return 1;
}

int test_angular_position_distance() {
    // Test angular distance calculation
    
    AngularPosition pos1, pos2;
    angular_position_calculate(5, 3, 1, 1.0, &pos1);
    angular_position_calculate(7, 4, 1, 1.0, &pos2);
    
    double distance = angular_position_distance(&pos1, &pos2);
    
    // Distance should be non-negative
    if (distance < 0.0) {
        printf("ERROR: Distance should be non-negative\n");
        return 0;
    }
    
    // Distance should be at most π
    if (distance > M_PI + MATH_EPSILON) {
        printf("ERROR: Distance should be at most π, got %.6f\n", distance);
        return 0;
    }
    
    // Distance to self should be 0
    double self_distance = angular_position_distance(&pos1, &pos1);
    if (fabs(self_distance) > MATH_EPSILON) {
        printf("ERROR: Distance to self should be 0, got %.6f\n", self_distance);
        return 0;
    }
    
    return 1;
}

int test_angular_position_print() {
    // Test printing functions (just verify they don't crash)
    
    AngularPosition pos;
    angular_position_calculate(13, 6, 2, 0.8, &pos);
    
    angular_position_print(&pos);
    angular_position_print_detailed(&pos);
    
    return 1;
}

int test_angular_position_batch() {
    // Test batch calculation
    
    uint64_t primes[] = {2, 3, 5, 7, 11};
    uint64_t indices[] = {1, 2, 3, 4, 5};
    AngularPosition results[5];
    
    angular_position_calculate_batch(primes, indices, 1, 1.0, 5, results);
    
    // Verify all results
    for (int i = 0; i < 5; i++) {
        if (results[i].prime != primes[i]) {
            printf("ERROR: Batch result %d has wrong prime\n", i);
            return 0;
        }
        
        if (!angular_position_validate(&results[i])) {
            printf("ERROR: Batch result %d failed validation\n", i);
            return 0;
        }
    }
    
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     ANGULAR POSITION SYSTEM - COMPREHENSIVE TESTS         ║\n");
    printf("║     Complete Mathematical Formula Implementation          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Individual Term Tests
    printf("\n--- INDIVIDUAL TERM TESTS ---\n");
    TEST(spiral_term);
    TEST(index_term);
    TEST(phonetic_term);
    TEST(omega_correction);
    TEST(psi_correction);
    
    // Helper Function Tests
    printf("\n--- HELPER FUNCTION TESTS ---\n");
    TEST(normalize);
    TEST(clock_position);
    TEST(symmetry_group);
    TEST(boundary_detection);
    TEST(twin_prime_detection);
    
    // Frequency/Wavelength Tests
    printf("\n--- FREQUENCY/WAVELENGTH TESTS ---\n");
    TEST(wavelength_frequency_conversion);
    TEST(phonetic_wavelength);
    
    // Plimpton 322 Tests
    printf("\n--- PLIMPTON 322 TESTS ---\n");
    TEST(plimpton_triple_search);
    
    // Complete Angular Position Tests
    printf("\n--- COMPLETE ANGULAR POSITION TESTS ---\n");
    TEST(angular_position_calculation);
    TEST(angular_position_144000);
    TEST(angular_position_twin_primes);
    TEST(angular_position_validation);
    TEST(angular_position_distance);
    TEST(angular_position_print);
    TEST(angular_position_batch);
    
    // Summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                         ║\n", 
           tests_passed + tests_failed);
    printf("║  Passed:       %3d ✓                                       ║\n", 
           tests_passed);
    printf("║  Failed:       %3d ✗                                       ║\n", 
           tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n",
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return tests_failed > 0 ? 1 : 0;
}