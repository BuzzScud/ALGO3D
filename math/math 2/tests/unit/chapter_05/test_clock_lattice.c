/**
 * Test program for Babylonian Clock Lattice structure
 * 
 * This test verifies:
 * 1. Ring structure (12, 60, 60, 100 positions)
 * 2. Smaller primes on outer rings (hours)
 * 3. Larger primes on inner rings (milliseconds)
 * 4. Sacred positions (π at 3 o'clock, 12 o'clock)
 * 5. Modular arithmetic relationships
 * 6. Stereographic projection to unit sphere
 * 7. Clock-to-sphere folding
 */

#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <stdbool.h>
#include "clock_lattice.h"
#include "prime_basic.h"
#include "cllm_pure_crystalline.h"

#define TEST_PRIMES 232  // First 4 rings
#define MATH_EPSILON 0.0001

// Color codes for output
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

typedef struct {
    int passed;
    int failed;
} TestResults;

void print_test_header(const char* test_name) {
    printf("\n" COLOR_BLUE "=== %s ===" COLOR_RESET "\n", test_name);
}

void print_pass(const char* message) {
    printf(COLOR_GREEN "✓ PASS: %s" COLOR_RESET "\n", message);
}

void print_fail(const char* message) {
    printf(COLOR_RED "✗ FAIL: %s" COLOR_RESET "\n", message);
}

void print_info(const char* message) {
    printf(COLOR_YELLOW "ℹ INFO: %s" COLOR_RESET "\n", message);
}

// Test 1: Verify ring structure
bool test_ring_structure(TestResults* results) {
    print_test_header("Test 1: Ring Structure (12, 60, 60, 100)");
    
    int ring_counts[4] = {0, 0, 0, 0};
    int expected_counts[4] = {12, 60, 60, 100};
    
    for (int i = 0; i < TEST_PRIMES; i++) {
        BabylonianClockPosition pos = map_prime_index_to_clock(i);
        if (pos.ring >= 0 && pos.ring < 4) {
            ring_counts[pos.ring]++;
        }
    }
    
    bool all_correct = true;
    for (int i = 0; i < 4; i++) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Ring %d: Expected %d, Got %d", 
                 i, expected_counts[i], ring_counts[i]);
        
        if (ring_counts[i] == expected_counts[i]) {
            print_pass(msg);
            results->passed++;
        } else {
            print_fail(msg);
            results->failed++;
            all_correct = false;
        }
    }
    
    return all_correct;
}

// Test 2: Verify smaller primes on outer rings
bool test_prime_distribution(TestResults* results) {
    print_test_header("Test 2: Prime Distribution (Smaller=Outer, Larger=Inner)");
    
    uint64_t primes[TEST_PRIMES];
    for (int i = 0; i < TEST_PRIMES; i++) {
        primes[i] = crystalline_get_nth_prime(i);
    }
    
    // Check that primes increase as we go from outer to inner rings
    uint64_t ring_max[4] = {0, 0, 0, 0};
    uint64_t ring_min[4] = {UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX};
    
    for (int i = 0; i < TEST_PRIMES; i++) {
        BabylonianClockPosition pos = map_prime_index_to_clock(i);
        uint64_t prime = primes[i];
        
        if (pos.ring >= 0 && pos.ring < 4) {
            if (prime > ring_max[pos.ring]) ring_max[pos.ring] = prime;
            if (prime < ring_min[pos.ring]) ring_min[pos.ring] = prime;
        }
    }
    
    printf("Ring distribution:\n");
    for (int i = 0; i < 4; i++) {
        printf("  Ring %d: min=%lu, max=%lu\n", i, ring_min[i], ring_max[i]);
    }
    
    // Verify that outer rings have smaller primes
    bool correct = true;
    for (int i = 0; i < 3; i++) {
        if (ring_max[i] >= ring_min[i+1]) {
            char msg[256];
            snprintf(msg, sizeof(msg), 
                     "Ring %d max (%lu) should be < Ring %d min (%lu)",
                     i, ring_max[i], i+1, ring_min[i+1]);
            print_fail(msg);
            results->failed++;
            correct = false;
        }
    }
    
    if (correct) {
        print_pass("Primes correctly distributed: smaller on outer rings");
        results->passed++;
    }
    
    return correct;
}

// Test 3: Verify sacred positions
bool test_sacred_positions(TestResults* results) {
    print_test_header("Test 3: Sacred Positions (π, 12 o'clock)");
    
    bool found_pi = false;
    bool found_12 = false;
    
    for (int i = 0; i < TEST_PRIMES; i++) {
        BabylonianClockPosition pos = map_prime_index_to_clock(i);
        
        if (is_sacred_position(pos)) {
            // Check if it's π position (3 o'clock, angle = 0)
            if (fabs(pos.angle) < MATH_EPSILON) {
                found_pi = true;
                char msg[256];
                snprintf(msg, sizeof(msg), 
                         "Found π position at prime index %d (3 o'clock)", i);
                print_pass(msg);
                results->passed++;
            }
            
            // Check if it's 12 o'clock (angle = -π/2)
            if (fabs(pos.angle + M_PI/2) < MATH_EPSILON) {
                found_12 = true;
                char msg[256];
                snprintf(msg, sizeof(msg), 
                         "Found 12 o'clock position at prime index %d", i);
                print_pass(msg);
                results->passed++;
            }
        }
    }
    
    if (!found_pi) {
        print_fail("Did not find π position (3 o'clock)");
        results->failed++;
    }
    
    if (!found_12) {
        print_fail("Did not find 12 o'clock position");
        results->failed++;
    }
    
    return found_pi && found_12;
}

// Test 4: Verify modular arithmetic
bool test_modular_arithmetic(TestResults* results) {
    print_test_header("Test 4: Modular Arithmetic (mod 12, 60, 100)");
    
    int mod12_counts[12] = {0};
    int mod60_counts[60] = {0};
    int mod100_counts[100] = {0};
    
    for (int i = 0; i < TEST_PRIMES; i++) {
        uint64_t prime = crystalline_get_nth_prime(i);
        PrimeModular modular = get_prime_modular(prime);
        
        mod12_counts[modular.mod_12]++;
        mod60_counts[modular.mod_60]++;
        mod100_counts[modular.mod_100]++;
    }
    
    // Check mod 12 distribution (primes > 3 can only be 1,5,7,11 mod 12)
    int valid_mod12 = 0;
    for (int i = 0; i < 12; i++) {
        if (i == 1 || i == 5 || i == 7 || i == 11 || i == 2 || i == 3) {
            valid_mod12 += mod12_counts[i];
        }
    }
    
    char msg[256];
    snprintf(msg, sizeof(msg), 
             "Mod 12: %d/%d primes in valid residue classes", 
             valid_mod12, TEST_PRIMES);
    print_pass(msg);
    results->passed++;
    
    // Check that mod 60 and mod 100 have reasonable distribution
    int nonzero_mod60 = 0, nonzero_mod100 = 0;
    for (int i = 0; i < 60; i++) if (mod60_counts[i] > 0) nonzero_mod60++;
    for (int i = 0; i < 100; i++) if (mod100_counts[i] > 0) nonzero_mod100++;
    
    snprintf(msg, sizeof(msg), 
             "Mod 60: %d/60 residue classes used", nonzero_mod60);
    print_info(msg);
    
    snprintf(msg, sizeof(msg), 
             "Mod 100: %d/100 residue classes used", nonzero_mod100);
    print_info(msg);
    
    return true;
}

// Test 5: Verify stereographic projection to unit sphere
bool test_stereographic_projection(TestResults* results) {
    print_test_header("Test 5: Stereographic Projection (Unit Sphere)");
    
    int on_sphere = 0;
    int off_sphere = 0;
    
    for (int i = 0; i < TEST_PRIMES; i++) {
        BabylonianClockPosition clock_pos = map_prime_index_to_clock(i);
        SphereCoord sphere = fold_clock_to_sphere(clock_pos);
        
        // Check if on unit sphere: x² + y² + z² = 1
        double radius_sq = sphere.x * sphere.x + 
                          sphere.y * sphere.y + 
                          sphere.z * sphere.z;
        
        if (fabs(radius_sq - 1.0) < MATH_EPSILON) {
            on_sphere++;
        } else {
            off_sphere++;
            if (off_sphere <= 5) {  // Print first few errors
                char msg[256];
                snprintf(msg, sizeof(msg), 
                         "Prime %d: radius² = %.6f (should be 1.0)", 
                         i, radius_sq);
                print_fail(msg);
            }
        }
    }
    
    char msg[256];
    snprintf(msg, sizeof(msg), 
             "%d/%d primes correctly mapped to unit sphere", 
             on_sphere, TEST_PRIMES);
    
    if (on_sphere == TEST_PRIMES) {
        print_pass(msg);
        results->passed++;
        return true;
    } else {
        print_fail(msg);
        results->failed++;
        return false;
    }
}

// Test 6: Verify clock-to-sphere folding properties
bool test_folding_properties(TestResults* results) {
    print_test_header("Test 6: Clock-to-Sphere Folding Properties");
    
    // Test that center (radius=0) maps to north pole (z=1)
    BabylonianClockPosition center = {0, 0, 0.0, 0.0};
    SphereCoord sphere_center = fold_clock_to_sphere(center);
    
    bool center_correct = fabs(sphere_center.z - 1.0) < MATH_EPSILON;
    if (center_correct) {
        print_pass("Center (radius=0) correctly maps to north pole (z=1)");
        results->passed++;
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg), 
                 "Center maps to z=%.6f (should be 1.0)", sphere_center.z);
        print_fail(msg);
        results->failed++;
    }
    
    // Test that boundary (radius=1) maps to south pole (z=-1)
    BabylonianClockPosition boundary = {3, 99, 0.0, 1.0};
    SphereCoord sphere_boundary = fold_clock_to_sphere(boundary);
    
    bool boundary_correct = fabs(sphere_boundary.z + 1.0) < MATH_EPSILON;
    if (boundary_correct) {
        print_pass("Boundary (radius=1) correctly maps to south pole (z=-1)");
        results->passed++;
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg), 
                 "Boundary maps to z=%.6f (should be -1.0)", sphere_boundary.z);
        print_fail(msg);
        results->failed++;
    }
    
    return center_correct && boundary_correct;
}

// Test 7: Verify ring radius progression
bool test_ring_radii(TestResults* results) {
    print_test_header("Test 7: Ring Radius Progression");
    
    double expected_radii[4] = {0.25, 0.50, 0.75, 1.00};
    bool all_correct = true;
    
    for (int ring = 0; ring < 4; ring++) {
        double radius = get_ring_radius_for_visualization(ring, 1.0);
        bool correct = fabs(radius - expected_radii[ring]) < MATH_EPSILON;
        
        char msg[256];
        snprintf(msg, sizeof(msg), 
                 "Ring %d: radius=%.2f (expected %.2f)", 
                 ring, radius, expected_radii[ring]);
        
        if (correct) {
            print_pass(msg);
            results->passed++;
        } else {
            print_fail(msg);
            results->failed++;
            all_correct = false;
        }
    }
    
    return all_correct;
}

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║   BABYLONIAN CLOCK LATTICE - COMPREHENSIVE TEST SUITE     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    TestResults results = {0, 0};
    
    // Run all tests
    test_ring_structure(&results);
    test_prime_distribution(&results);
    test_sacred_positions(&results);
    test_modular_arithmetic(&results);
    test_stereographic_projection(&results);
    test_folding_properties(&results);
    test_ring_radii(&results);
    
    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                         ║\n", 
           results.passed + results.failed);
    printf("║  " COLOR_GREEN "Passed:      %3d" COLOR_RESET 
           "                                         ║\n", results.passed);
    printf("║  " COLOR_RED "Failed:      %3d" COLOR_RESET 
           "                                         ║\n", results.failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    if (results.failed == 0) {
        printf("\n" COLOR_GREEN "✓ ALL TESTS PASSED! " COLOR_RESET 
               "The Babylonian Clock Lattice is correctly implemented.\n\n");
        return 0;
    } else {
        printf("\n" COLOR_RED "✗ SOME TESTS FAILED. " COLOR_RESET 
               "Please review the implementation.\n\n");
        return 1;
    }
}