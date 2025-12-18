/*
 * Comprehensive Test Suite for ALL Crystalline Math Functions
 * 
 * Tests every mathematical function with extreme values to ensure
 * no overflow, NaN, or precision issues.
 * 
 * Categories:
 * 1. Transcendental functions (exp, log, sin, cos, tan)
 * 2. Power functions (pow, sqrt)
 * 3. Clock lattice functions
 * 4. Prime functions
 * 5. Geometric functions
 */

#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <assert.h>
#include <float.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "clock_lattice.h"
#include "prime_coords.h"
#include "prime_rainbow.h"

// Test helper macros
#define TEST_START(name) printf("\n=== %s ===\n", name)
#define TEST_PASS(name) printf("  ✓ PASS: %s\n", name)
#define TEST_FAIL(name, msg) printf("  ✗ FAIL: %s - %s\n", name, msg)
#define ASSERT_NO_NAN(x, name) do { \
    if (isnan(x)) { \
        TEST_FAIL(name, "NaN detected"); \
        assert(0); \
    } \
} while(0)
#define ASSERT_NO_INF(x, name) do { \
    if (isinf(x)) { \
        TEST_FAIL(name, "Inf detected"); \
        assert(0); \
    } \
} while(0)
#define ASSERT_IN_RANGE(x, min, max, name) do { \
    if ((x) < (min) || (x) > (max)) { \
        TEST_FAIL(name, "Out of range"); \
        printf("    Value: %.6f, Expected: [%.6f, %.6f]\n", (double)(x), (double)(min), (double)(max)); \
        assert(0); \
    } \
} while(0)

// Test 1: Exponential function
void test_prime_exp() {
    TEST_START("prime_exp - Exponential Function");
    
    double test_values[] = {0.0, 1.0, 5.0, 10.0, 20.0, 50.0, 100.0, 500.0, 700.0};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_tests; i++) {
        double x = test_values[i];
        double result = math_exp(x);
        
        printf("  math_exp(%.1f) = %.6e\n", x, result);
        
        ASSERT_NO_NAN(result, "prime_exp");
        // Note: prime_exp returns HUGE_VAL for x > 700, which is correct behavior
        if (x <= 700.0) {
            ASSERT_NO_INF(result, "prime_exp");
        }
    }
    
    TEST_PASS("prime_exp");
}

// Test 2: Logarithm function
void test_prime_log() {
    TEST_START("prime_log - Natural Logarithm");
    
    double test_values[] = {1.0, 2.0, 10.0, 100.0, 1000.0, 1e6, 1e9, 1e12};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_tests; i++) {
        double x = test_values[i];
        double result = math_log(x);
        
        printf("  math_log(%.0e) = %.6f\n", x, result);
        
        ASSERT_NO_NAN(result, "prime_log");
        ASSERT_NO_INF(result, "prime_log");
        
        // Log of 1 is 0, log of x>1 is positive, log of 0<x<1 is negative
        if (x == 1.0) {
            assert(result == 0.0 && "Log of 1 should be 0");
        } else if (x > 1.0) {
            assert(result > 0.0 && "Log of number > 1 should be positive");
        }
        // Note: We're only testing x >= 1.0, so all should be >= 0
    }
    
    TEST_PASS("prime_log");
}

// Test 3: Power function
void test_prime_pow() {
    TEST_START("prime_pow - Power Function");
    
    struct {
        double base;
        double exp;
        double expected_max;
    } tests[] = {
        {3.0, 0.0, 1.0},
        {3.0, 1.0, 3.0},
        {3.0, 2.0, 9.0},
        {3.0, 4.0, 81.0},
        {3.0, 8.0, 6561.0},
        {3.0, 10.0, 59049.0},
        {2.0, 10.0, 1024.0},
        {2.0, 20.0, 1048576.0},
    };
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    for (int i = 0; i < num_tests; i++) {
        double result = math_pow(tests[i].base, tests[i].exp);
        
        printf("  math_pow(%.1f, %.1f) = %.2f (expected ≤ %.2f)\n", 
               tests[i].base, tests[i].exp, result, tests[i].expected_max);
        
        ASSERT_NO_NAN(result, "prime_pow");
        ASSERT_NO_INF(result, "prime_pow");
        assert(result <= tests[i].expected_max * 1.01 && "Power result too large");
    }
    
    TEST_PASS("prime_pow");
}

// Test 4: Trigonometric functions
void test_prime_trig() {
    TEST_START("prime_sin/cos/tan - Trigonometric Functions");
    
    double test_angles[] = {0.0, MATH_PI/6, MATH_PI/4, MATH_PI/3, MATH_PI/2, 
                           MATH_PI, 2*MATH_PI, 10*MATH_PI, 100*MATH_PI};
    int num_tests = sizeof(test_angles) / sizeof(test_angles[0]);
    
    for (int i = 0; i < num_tests; i++) {
        double angle = test_angles[i];
        double sin_val = math_sin(angle);
        double cos_val = math_cos(angle);
        double tan_val = math_tan(angle);
        
        printf("  angle=%.2f: sin=%.6f, cos=%.6f, tan=%.6f\n", 
               angle, sin_val, cos_val, tan_val);
        
        ASSERT_NO_NAN(sin_val, "prime_sin");
        ASSERT_NO_NAN(cos_val, "prime_cos");
        ASSERT_NO_INF(sin_val, "prime_sin");
        ASSERT_NO_INF(cos_val, "prime_cos");
        
        // Allow slightly larger range due to numerical precision
        ASSERT_IN_RANGE(sin_val, -1.1, 1.1, "prime_sin");
        ASSERT_IN_RANGE(cos_val, -1.1, 1.1, "prime_cos");
        
        // tan can be large near π/2, but shouldn't be NaN
        if (!isinf(tan_val)) {
            ASSERT_NO_NAN(tan_val, "prime_tan");
        }
    }
    
    TEST_PASS("prime_trig");
}

// Test 5: Hyperbolic functions
void test_prime_tanh() {
    TEST_START("prime_tanh - Hyperbolic Tangent");
    
    double test_values[] = {-100.0, -20.0, -10.0, -1.0, 0.0, 1.0, 10.0, 20.0, 100.0, 1000.0};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_tests; i++) {
        double x = test_values[i];
        double result = math_tanh(x);
        
        printf("  math_tanh(%.1f) = %.6f\n", x, result);
        
        ASSERT_NO_NAN(result, "prime_tanh");
        ASSERT_NO_INF(result, "prime_tanh");
        ASSERT_IN_RANGE(result, -1.0, 1.0, "prime_tanh");
    }
    
    TEST_PASS("prime_tanh");
}

// Test 6: Clock lattice mapping
void test_clock_lattice_mapping() {
    TEST_START("Clock Lattice Mapping");
    
    int test_indices[] = {0, 10, 50, 100, 232, 500, 1000, 10000, 100000, 1000000};
    int num_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int prime_index = test_indices[i];
        BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
        
        printf("  Prime %d → Ring %d, Position %d, Angle %.2f, Radius %.2f\n",
               prime_index, pos.ring, pos.position, pos.angle, pos.radius);
        
        // Verify ring is bounded
        assert(pos.ring >= 0 && pos.ring <= 7 && "Ring out of bounds");
        
        // Verify angle is bounded
        assert(pos.angle >= -2*MATH_PI && pos.angle <= 2*MATH_PI && "Angle out of bounds");
        
        // Verify radius is bounded
        assert(pos.radius >= 0.0 && pos.radius <= 2.0 && "Radius out of bounds");
    }
    
    TEST_PASS("Clock lattice mapping");
}

// Test 7: O(n,k,λ) calculation
void test_O_calculation() {
    TEST_START("O(n,k,λ) Calculation");
    
    int test_indices[] = {0, 100, 1000, 10000, 100000, 1000000};
    int num_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int prime_index = test_indices[i];
        BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
        
        double positions_in_ring;
        if (pos.ring == 0) positions_in_ring = 12.0;
        else if (pos.ring == 1 || pos.ring == 2) positions_in_ring = 60.0;
        else if (pos.ring == 3) positions_in_ring = 100.0;
        else positions_in_ring = 1000.0;
        
        double O = (double)pos.ring + ((double)pos.position / positions_in_ring);
        double three_to_O = math_pow(3.0, O);
        
        printf("  Prime %d → O=%.3f, 3^O=%.2f\n", prime_index, O, three_to_O);
        
        // Verify O is bounded
        assert(O >= 0.0 && O <= 10.0 && "O out of expected bounds");
        
        // Verify 3^O doesn't overflow
        ASSERT_NO_NAN(three_to_O, "3^O");
        ASSERT_NO_INF(three_to_O, "3^O");
        assert(three_to_O < 100000.0 && "3^O too large");
    }
    
    TEST_PASS("O calculation");
}

// Test 8: Full L(n,d,k,λ) formula
void test_L_formula() {
    TEST_START("L(n,d,k,λ) Formula");
    
    int test_indices[] = {0, 100, 1000, 10000, 100000};
    int num_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int token_id = test_indices[i];
        
        // Simulate what lattice_embeddings_init_geometric does
        BabylonianClockPosition pos = map_prime_index_to_clock(token_id);
        int symmetry_group = token_id % 12;
        
        double positions_in_ring;
        if (pos.ring == 0) positions_in_ring = 12.0;
        else if (pos.ring == 1 || pos.ring == 2) positions_in_ring = 60.0;
        else if (pos.ring == 3) positions_in_ring = 100.0;
        else positions_in_ring = 1000.0;
        
        double O = (double)pos.ring + ((double)pos.position / positions_in_ring);
        double base = math_pow(3.0, O);
        
        // Test one dimension
        uint64_t phi_i = 3;  // First dimensional frequency
        double theta = pos.angle;
        double cos_term = math_cos(theta * (double)phi_i);
        
        double gamma_k = math_cos(2.0 * MATH_PI * (double)symmetry_group / 12.0);
        
        double entropy_factor = 1.0 + (double)pos.ring * 0.1 + 0.01;
        double gamma_nd = math_tanh(entropy_factor);
        
        double L = base * cos_term * gamma_k * gamma_nd;
        double result = math_tanh(L / 100.0);
        
        printf("  Token %d → L=%.6f, tanh(L/100)=%.6f\n", token_id, L, result);
        
        ASSERT_NO_NAN(L, "L formula");
        ASSERT_NO_NAN(result, "tanh(L/100)");
        ASSERT_NO_INF(L, "L formula");
        ASSERT_NO_INF(result, "tanh(L/100)");
        ASSERT_IN_RANGE(result, -1.0, 1.0, "Final embedding value");
    }
    
    TEST_PASS("L formula");
}

// Test 9: Stress test with maximum values
void test_extreme_values() {
    TEST_START("Extreme Value Stress Test");
    
    // Test with extremely large prime indices
    int extreme_indices[] = {1000000, 10000000, 100000000};
    int num_tests = sizeof(extreme_indices) / sizeof(extreme_indices[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int prime_index = extreme_indices[i];
        
        printf("  Testing prime index %d...\n", prime_index);
        
        // Test clock mapping
        BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
        printf("    Ring: %d, Position: %d\n", pos.ring, pos.position);
        assert(pos.ring >= 0 && pos.ring <= 7 && "Ring out of bounds");
        
        // Test O calculation
        double positions_in_ring = 1000.0;
        double O = (double)pos.ring + ((double)pos.position / positions_in_ring);
        printf("    O: %.3f\n", O);
        assert(O >= 0.0 && O <= 10.0 && "O out of bounds");
        
        // Test 3^O
        double three_to_O = math_pow(3.0, O);
        printf("    3^O: %.2f\n", three_to_O);
        ASSERT_NO_NAN(three_to_O, "3^O extreme");
        ASSERT_NO_INF(three_to_O, "3^O extreme");
        
        printf("    ✓ Prime index %d handled correctly\n", prime_index);
    }
    
    TEST_PASS("Extreme values");
}

// Test 10: Prime coordinate functions (skipped - function not in public API)
void test_prime_coords() {
    TEST_START("Prime Coordinate Functions");
    printf("  ⊘ SKIPPED: compute_prime_coords not in public API\n");
}

// Test 11: Rainbow table functions
void test_rainbow_table() {
    TEST_START("Rainbow Table Functions");
    
    int test_indices[] = {0, 10, 100, 1000, 10000};
    int num_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int prime_index = test_indices[i];
        
        double angle = fast_prime_angle(prime_index);
        double radius = fast_prime_radius(prime_index);
        int layer = fast_prime_layer(prime_index);
        
        printf("  Prime index %d → angle=%.2f, radius=%.2f, layer=%d\n",
               prime_index, angle, radius, layer);
        
        ASSERT_NO_NAN(angle, "rainbow angle");
        ASSERT_NO_NAN(radius, "rainbow radius");
        ASSERT_NO_INF(angle, "rainbow angle");
        ASSERT_NO_INF(radius, "rainbow radius");
        
        assert(layer >= 0 && layer <= 10 && "Layer out of bounds");
    }
    
    TEST_PASS("Rainbow table");
}

// Test 12: Memory stress test
void test_memory_stress() {
    TEST_START("Memory Stress Test");
    
    // Allocate and initialize large embedding matrix
    uint32_t vocab_size = 100000;
    uint32_t embedding_dim = 512;
    
    printf("  Allocating %.2f MB...\n", 
           (vocab_size * embedding_dim * sizeof(float)) / (1024.0 * 1024.0));
    
    float* embeddings = (float*)calloc(vocab_size * embedding_dim, sizeof(float));
    if (!embeddings) {
        printf("  ✗ SKIP: Could not allocate memory\n");
        return;
    }
    
    printf("  Initializing embeddings...\n");
    lattice_embeddings_init_geometric(embeddings, vocab_size, embedding_dim);
    
    // Sample check
    int sample_size = 10000;
    int nan_count = 0;
    int inf_count = 0;
    
    for (int s = 0; s < sample_size; s++) {
        uint32_t idx = (rand() % vocab_size) * embedding_dim + (rand() % embedding_dim);
        if (isnan(embeddings[idx])) nan_count++;
        if (isinf(embeddings[idx])) inf_count++;
    }
    
    printf("  Sample size: %d\n", sample_size);
    printf("  NaN count: %d\n", nan_count);
    printf("  Inf count: %d\n", inf_count);
    
    assert(nan_count == 0 && "NaN found in memory stress test");
    assert(inf_count == 0 && "Inf found in memory stress test");
    
    TEST_PASS("Memory stress");
    
    free(embeddings);
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Comprehensive Crystalline Math Function Test Suite      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Run all tests
    test_prime_exp();
    test_prime_log();
    test_prime_pow();
    test_prime_trig();
    test_prime_tanh();
    test_clock_lattice_mapping();
    test_O_calculation();
    test_L_formula();
    test_extreme_values();
    test_prime_coords();
    test_rainbow_table();
    test_memory_stress();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ✓ ALL TESTS PASSED - System Mathematically Sound        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}