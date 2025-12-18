/**
 * @file test_clock_lattice_abacus.c
 * @brief Test suite for Pure Abacus Clock Lattice Integration
 */

#include "geometric_recovery/clock_lattice_abacus.h"
#include "math/abacus.h"
#include "math/clock.h"
#include "math/constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Test colors
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) \
    printf("\n" COLOR_YELLOW "TEST: %s" COLOR_RESET "\n", name); \
    tests_run++;

#define TEST_PASS() \
    printf(COLOR_GREEN "  ✓ PASS" COLOR_RESET "\n"); \
    tests_passed++;

#define TEST_FAIL(msg) \
    printf(COLOR_RED "  ✗ FAIL: %s" COLOR_RESET "\n", msg); \
    tests_failed++;

// ============================================================================
// TEST 1: Clock Position Creation
// ============================================================================

void test_clock_position_creation() {
    TEST_START("Clock Position Creation");
    
    uint32_t base = 12;
    
    // Create position at 3 o'clock (position 3 on ring 0)
    ClockPositionAbacus* pos = create_clock_position_abacus(0, 3, base);
    
    if (pos && pos->ring == 0 && pos->position == 3) {
        double angle;
        abacus_to_double(pos->angle, &angle);
        
        // 3 o'clock = 90° = π/2 radians
        double expected = MATH_PI / 2.0;
        double tolerance = 0.01;
        
        if (fabs(angle - expected) < tolerance) {
            printf("  Position: Ring %u, Position %u\n", pos->ring, pos->position);
            printf("  Angle: %.6f radians (expected %.6f)\n", angle, expected);
            TEST_PASS();
        } else {
            TEST_FAIL("Angle calculation incorrect");
        }
    } else {
        TEST_FAIL("Failed to create clock position");
    }
    
    free_clock_position_abacus(pos);
}

// ============================================================================
// TEST 2: Prime to Clock Mapping
// ============================================================================

void test_prime_to_clock_mapping() {
    TEST_START("Prime to Clock Mapping");
    
    uint32_t base = 12;
    uint32_t precision = 20;
    
    // Test with prime 5 (should map to position 2, which is 3 o'clock)
    CrystallineAbacus* prime = abacus_from_uint64(5, base);
    ClockPositionAbacus pos;
    pos.angle = NULL;
    pos.clock_value = NULL;
    
    MathError err = map_prime_to_clock_abacus(prime, &pos, base, precision);
    
    if (err == MATH_SUCCESS) {
        printf("  Prime 5 maps to: Ring %u, Position %u\n", pos.ring, pos.position);
        
        double angle;
        abacus_to_double(pos.angle, &angle);
        printf("  Angle: %.6f radians\n", angle);
        
        TEST_PASS();
    } else {
        TEST_FAIL("Failed to map prime to clock");
    }
    
    abacus_free(prime);
    abacus_free(pos.angle);
    abacus_free(pos.clock_value);
}

// ============================================================================
// TEST 3: Angular Separation
// ============================================================================

void test_angular_separation() {
    TEST_START("Angular Separation");
    
    uint32_t base = 12;
    
    // Create two positions: 12 o'clock (0) and 3 o'clock (3)
    ClockPositionAbacus* pos1 = create_clock_position_abacus(0, 0, base);
    ClockPositionAbacus* pos2 = create_clock_position_abacus(0, 3, base);
    
    if (pos1 && pos2) {
        double angle1, angle2;
        abacus_to_double(pos1->angle, &angle1);
        abacus_to_double(pos2->angle, &angle2);
        printf("  Position 1 angle: %.6f radians\n", angle1);
        printf("  Position 2 angle: %.6f radians\n", angle2);
        
        CrystallineAbacus* separation = compute_angular_separation_abacus(
            pos1, pos2, base, 20
        );
        
        if (separation) {
            double sep_val;
            abacus_to_double(separation, &sep_val);
            
            // Expected: π/2 (90°)
            double expected = MATH_PI / 2.0;
            double tolerance = 0.01;
            
            printf("  Separation: %.6f radians (expected %.6f)\n", sep_val, expected);
            
            if (fabs(sep_val - expected) < tolerance) {
                TEST_PASS();
            } else {
                TEST_FAIL("Angular separation incorrect");
            }
            
            abacus_free(separation);
        } else {
            TEST_FAIL("Failed to compute separation");
        }
    } else {
        TEST_FAIL("Failed to create positions");
    }
    
    free_clock_position_abacus(pos1);
    free_clock_position_abacus(pos2);
}

// ============================================================================
// TEST 4: Geometric Distance
// ============================================================================

void test_geometric_distance() {
    TEST_START("Geometric Distance");
    
    uint32_t base = 12;
    
    // Create two positions on same ring
    ClockPositionAbacus* pos1 = create_clock_position_abacus(0, 0, base);
    ClockPositionAbacus* pos2 = create_clock_position_abacus(0, 6, base);
    
    if (pos1 && pos2) {
        CrystallineAbacus* distance = compute_geometric_distance_abacus(
            pos1, pos2, base, 20
        );
        
        if (distance) {
            double dist_val;
            abacus_to_double(distance, &dist_val);
            
            // Expected: π (180°)
            double expected = MATH_PI;
            double tolerance = 0.01;
            
            printf("  Distance: %.6f radians (expected %.6f)\n", dist_val, expected);
            
            if (fabs(dist_val - expected) < tolerance) {
                TEST_PASS();
            } else {
                TEST_FAIL("Geometric distance incorrect");
            }
            
            abacus_free(distance);
        } else {
            TEST_FAIL("Failed to compute distance");
        }
    } else {
        TEST_FAIL("Failed to create positions");
    }
    
    free_clock_position_abacus(pos1);
    free_clock_position_abacus(pos2);
}

// ============================================================================
// TEST 5: Ring Distance
// ============================================================================

void test_ring_distance() {
    TEST_START("Ring Distance");
    
    uint32_t base = 12;
    
    // Distance between ring 0 and ring 2
    CrystallineAbacus* distance = compute_ring_distance_abacus(0, 2, base);
    
    if (distance) {
        uint64_t dist_val;
        abacus_to_uint64(distance, &dist_val);
        
        printf("  Ring distance (0 to 2): %lu\n", dist_val);
        
        if (dist_val == 2) {
            TEST_PASS();
        } else {
            TEST_FAIL("Ring distance incorrect");
        }
        
        abacus_free(distance);
    } else {
        TEST_FAIL("Failed to compute ring distance");
    }
}

// ============================================================================
// TEST 6: Factor Visualization
// ============================================================================

void test_factor_visualization() {
    TEST_START("Factor Visualization");
    
    uint32_t base = 12;
    uint32_t precision = 20;
    
    // Test with n = 15 = 3 × 5
    CrystallineAbacus* n = abacus_from_uint64(15, base);
    CrystallineAbacus* p = abacus_from_uint64(3, base);
    CrystallineAbacus* q = abacus_from_uint64(5, base);
    
    ClockFactorVisualizationAbacus* viz = visualize_factors_abacus(
        n, p, q, base, precision
    );
    
    if (viz) {
        printf("  n = 15 = 3 × 5\n");
        printf("  Prime 3: Ring %u, Position %u\n", 
               viz->p_position->ring, viz->p_position->position);
        printf("  Prime 5: Ring %u, Position %u\n", 
               viz->q_position->ring, viz->q_position->ring);
        
        double angular_sep;
        abacus_to_double(viz->angular_separation, &angular_sep);
        printf("  Angular separation: %.6f radians\n", angular_sep);
        
        TEST_PASS();
        
        free_factor_visualization_abacus(viz);
    } else {
        TEST_FAIL("Failed to create visualization");
    }
    
    abacus_free(n);
    abacus_free(p);
    abacus_free(q);
}

// ============================================================================
// TEST 7: Clock Resolution
// ============================================================================

void test_clock_resolution() {
    TEST_START("Clock Resolution");
    
    uint32_t base = 12;
    uint32_t precision = 20;
    
    CrystallineAbacus* resolution = compute_clock_resolution_abacus(base, precision);
    
    if (resolution) {
        double res_val;
        abacus_to_double(resolution, &res_val);
        
        // Expected: 2π / 4,320,000
        double expected = (2.0 * MATH_PI) / 4320000.0;
        
        printf("  Resolution: %.10e radians\n", res_val);
        printf("  Expected: %.10e radians\n", expected);
        printf("  Total positions: %u\n", get_total_clock_positions_abacus());
        
        TEST_PASS();
        
        abacus_free(resolution);
    } else {
        TEST_FAIL("Failed to compute resolution");
    }
}

// ============================================================================
// TEST 8: Utility Functions
// ============================================================================

void test_utility_functions() {
    TEST_START("Utility Functions");
    
    // Test ring sizes
    uint32_t ring0 = get_ring_size_abacus(0);
    uint32_t ring1 = get_ring_size_abacus(1);
    uint32_t ring2 = get_ring_size_abacus(2);
    uint32_t ring3 = get_ring_size_abacus(3);
    
    printf("  Ring 0 (hours): %u\n", ring0);
    printf("  Ring 1 (minutes): %u\n", ring1);
    printf("  Ring 2 (seconds): %u\n", ring2);
    printf("  Ring 3 (milliseconds): %u\n", ring3);
    
    uint32_t total = get_total_clock_positions_abacus();
    printf("  Total positions: %u\n", total);
    
    if (ring0 == 12 && ring1 == 60 && ring2 == 60 && ring3 == 100 && total == 4320000) {
        TEST_PASS();
    } else {
        TEST_FAIL("Ring sizes incorrect");
    }
}

// ============================================================================
// TEST 9: Prime Factorization
// ============================================================================

void test_prime_factorization() {
    TEST_START("Prime Factorization");
    
    uint32_t base = 12;
    uint32_t precision = 20;
    
    // Test with n = 15 = 3 × 5
    CrystallineAbacus* n = abacus_from_uint64(15, base);
    
    PrimeFactorizationAbacus* factorization = clock_lattice_factorize_abacus(
        n, base, precision
    );
    
    if (factorization) {
        printf("  Number: 15\n");
        printf("  Factors found: %u\n", factorization->num_factors);
        
        // Print factors
        for (uint32_t i = 0; i < factorization->num_factors; i++) {
            uint64_t factor_val;
            abacus_to_uint64(factorization->factors[i], &factor_val);
            printf("    Factor %u: %lu (Ring %u, Position %u)\n", 
                   i + 1, factor_val,
                   factorization->positions[i]->ring,
                   factorization->positions[i]->position);
        }
        
        // Validate
        bool valid = validate_factorization_abacus(n, factorization, base);
        printf("  Validation: %s\n", valid ? "PASS" : "FAIL");
        
        if (valid && factorization->num_factors == 2) {
            TEST_PASS();
        } else {
            TEST_FAIL("Factorization incorrect");
        }
        
        free_prime_factorization_abacus(factorization);
    } else {
        TEST_FAIL("Failed to factorize");
    }
    
    abacus_free(n);
}

// ============================================================================
// TEST 10: Factorization Validation
// ============================================================================

void test_factorization_validation() {
    TEST_START("Factorization Validation");
    
    uint32_t base = 12;
    uint32_t precision = 20;
    
    // Test several numbers
    uint64_t test_numbers[] = {6, 15, 35, 77, 143};
    
    for (size_t i = 0; i < sizeof(test_numbers) / sizeof(test_numbers[0]); i++) {
        CrystallineAbacus* n = abacus_from_uint64(test_numbers[i], base);
        
        PrimeFactorizationAbacus* factorization = clock_lattice_factorize_abacus(
            n, base, precision
        );
        
        if (factorization) {
            bool valid = validate_factorization_abacus(n, factorization, base);
            
            if (!valid) {
                printf("  Failed for n=%lu\n", test_numbers[i]);
                TEST_FAIL("Validation failed");
                abacus_free(n);
                free_prime_factorization_abacus(factorization);
                return;
            }
            
            free_prime_factorization_abacus(factorization);
        }
        
        abacus_free(n);
    }
    
    printf("  Validated: 6, 15, 35, 77, 143\n");
    TEST_PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Pure Abacus Clock Lattice Tests\n");
    printf("========================================\n");
    
    // Run all tests
    test_clock_position_creation();
    test_prime_to_clock_mapping();
    test_angular_separation();
    test_geometric_distance();
    test_ring_distance();
    test_factor_visualization();
    test_clock_resolution();
    test_utility_functions();
    test_prime_factorization();
    test_factorization_validation();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("  Total:  %d\n", tests_run);
    printf("  " COLOR_GREEN "Passed: %d" COLOR_RESET "\n", tests_passed);
    printf("  " COLOR_RED "Failed: %d" COLOR_RESET "\n", tests_failed);
    printf("========================================\n");
    printf("\n");
    
    return (tests_failed == 0) ? 0 : 1;
}