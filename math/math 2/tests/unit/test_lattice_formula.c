/**
 * @file test_lattice_formula.c
 * @brief Unit tests for complete L(n,d,k,λ,ω,ψ) lattice formula
 */

#include "ai/cllm_lattice_formula.h"
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
 * @brief Test: Octahedral symmetry calculation
 */
static void test_calculate_O(void) {
    printf("Testing octahedral symmetry O(n,k,λ)...\n");
    
    double O1 = calculate_O(10, 5, "default");
    assert(O1 >= 0.0);
    
    double O2 = calculate_O(100, 10, "dub");
    assert(O2 >= 0.0);
    
    // Same parameters should give same result
    double O3 = calculate_O(10, 5, "default");
    assert(doubles_equal(O1, O3, TEST_EPSILON));
    
    printf("✓ Octahedral symmetry test passed\n");
}

/**
 * @brief Test: Dimensional frequency
 */
static void test_dimensional_frequency(void) {
    printf("Testing dimensional frequencies...\n");
    
    // Test all 12 dimensions
    for (uint32_t d = 1; d <= 12; d++) {
        uint64_t freq = get_dimensional_frequency(d);
        assert(freq > 0);
    }
    
    // Test invalid dimensions
    assert(get_dimensional_frequency(0) == 1);
    assert(get_dimensional_frequency(13) == 1);
    
    // Verify frequencies are prime-based
    assert(get_dimensional_frequency(1) == 2);
    assert(get_dimensional_frequency(2) == 3);
    assert(get_dimensional_frequency(3) == 5);
    
    printf("✓ Dimensional frequency test passed\n");
}

/**
 * @brief Test: Möbius twist
 */
static void test_mobius_twist(void) {
    printf("Testing Möbius twist...\n");
    
    // Even k should give +1
    assert(calculate_mobius_twist(0) == 1.0);
    assert(calculate_mobius_twist(2) == 1.0);
    assert(calculate_mobius_twist(4) == 1.0);
    
    // Odd k should give -1
    assert(calculate_mobius_twist(1) == -1.0);
    assert(calculate_mobius_twist(3) == -1.0);
    assert(calculate_mobius_twist(5) == -1.0);
    
    printf("✓ Möbius twist test passed\n");
}

/**
 * @brief Test: Complete angular position
 */
static void test_angular_position_complete(void) {
    printf("Testing complete angular position...\n");
    
    double theta1 = angular_position_complete(10, 5, "default", 432.0, 2, 1);
    assert(theta1 >= 0.0 && theta1 < 2.0 * M_PI);
    
    double theta2 = angular_position_complete(100, 10, "dub", 528.0, 3, 2);
    assert(theta2 >= 0.0 && theta2 < 2.0 * M_PI);
    
    // Different parameters should give different results
    assert(!doubles_equal(theta1, theta2, TEST_EPSILON));
    
    printf("✓ Complete angular position test passed\n");
}

/**
 * @brief Test: Parameter validation
 */
static void test_parameter_validation(void) {
    printf("Testing parameter validation...\n");
    
    // Valid parameters
    LatticeFormulaParams valid = {
        .n = 10,
        .d = 6,
        .k = 5,
        .lambda = "default",
        .omega = 432.0,
        .p = 2,
        .q = 1
    };
    assert(validate_lattice_params(&valid));
    
    // Invalid dimension (0)
    LatticeFormulaParams invalid_d0 = valid;
    invalid_d0.d = 0;
    assert(!validate_lattice_params(&invalid_d0));
    
    // Invalid dimension (>12)
    LatticeFormulaParams invalid_d13 = valid;
    invalid_d13.d = 13;
    assert(!validate_lattice_params(&invalid_d13));
    
    // NULL lambda
    LatticeFormulaParams invalid_lambda = valid;
    invalid_lambda.lambda = NULL;
    assert(!validate_lattice_params(&invalid_lambda));
    
    // Negative omega
    LatticeFormulaParams invalid_omega = valid;
    invalid_omega.omega = -1.0;
    assert(!validate_lattice_params(&invalid_omega));
    
    // NULL params
    assert(!validate_lattice_params(NULL));
    
    printf("✓ Parameter validation test passed\n");
}

/**
 * @brief Test: Context initialization
 */
static void test_context_init(void) {
    printf("Testing context initialization...\n");
    
    LatticeFormulaContext ctx;
    
    // Initialize with caching
    assert(lattice_formula_context_init(&ctx, true, false));
    assert(ctx.use_cache == true);
    assert(ctx.verbose == false);
    assert(ctx.entropy_ctx != NULL);
    lattice_formula_context_destroy(&ctx);
    
    // Initialize without caching
    assert(lattice_formula_context_init(&ctx, false, true));
    assert(ctx.use_cache == false);
    assert(ctx.verbose == true);
    assert(ctx.entropy_ctx == NULL);
    lattice_formula_context_destroy(&ctx);
    
    // NULL context
    assert(!lattice_formula_context_init(NULL, true, false));
    
    printf("✓ Context initialization test passed\n");
}

/**
 * @brief Test: Complete formula calculation
 */
static void test_L_lattice_complete(void) {
    printf("Testing complete L(n,d,k,λ,ω,ψ) formula...\n");
    
    LatticeFormulaParams params = {
        .n = 10,
        .d = 6,
        .k = 5,
        .lambda = "default",
        .omega = 432.0,
        .p = 2,
        .q = 1
    };
    
    // Calculate without context
    double L1 = L_lattice_complete(&params, NULL);
    assert(!isnan(L1));
    assert(!isinf(L1));
    
    // Calculate with context
    LatticeFormulaContext ctx;
    lattice_formula_context_init(&ctx, true, false);
    double L2 = L_lattice_complete(&params, &ctx);
    assert(!isnan(L2));
    assert(!isinf(L2));
    
    // Results should be the same
    assert(doubles_equal(L1, L2, TEST_EPSILON));
    
    lattice_formula_context_destroy(&ctx);
    
    printf("✓ Complete formula calculation test passed\n");
}

/**
 * @brief Test: Formula with different parameters
 */
static void test_formula_variations(void) {
    printf("Testing formula with different parameters...\n");
    
    // Test different dimensions
    for (uint32_t d = 1; d <= 12; d++) {
        LatticeFormulaParams params = {
            .n = 100,
            .d = d,
            .k = 5,
            .lambda = "default",
            .omega = 432.0,
            .p = 2,
            .q = 1
        };
        
        double L = L_lattice_complete(&params, NULL);
        assert(!isnan(L));
        assert(!isinf(L));
    }
    
    // Test different phonetic layers
    const char* layers[] = {"default", "dub", "knbt", "k'anchay", "kub"};
    for (size_t i = 0; i < sizeof(layers) / sizeof(layers[0]); i++) {
        LatticeFormulaParams params = {
            .n = 100,
            .d = 6,
            .k = 5,
            .lambda = layers[i],
            .omega = 432.0,
            .p = 2,
            .q = 1
        };
        
        double L = L_lattice_complete(&params, NULL);
        assert(!isnan(L));
        assert(!isinf(L));
    }
    
    // Test different cymatic frequencies
    double frequencies[] = {432.0, 528.0, 7.83, 40.0};
    for (size_t i = 0; i < sizeof(frequencies) / sizeof(frequencies[0]); i++) {
        LatticeFormulaParams params = {
            .n = 100,
            .d = 6,
            .k = 5,
            .lambda = "default",
            .omega = frequencies[i],
            .p = 2,
            .q = 1
        };
        
        double L = L_lattice_complete(&params, NULL);
        assert(!isnan(L));
        assert(!isinf(L));
    }
    
    printf("✓ Formula variations test passed\n");
}

/**
 * @brief Test: Simplified formula
 */
static void test_L_lattice_simple(void) {
    printf("Testing simplified L_lattice...\n");
    
    double L1 = L_lattice_simple(10, 6, 5);
    assert(!isnan(L1));
    assert(!isinf(L1));
    
    double L2 = L_lattice_simple(100, 12, 10);
    assert(!isnan(L2));
    assert(!isinf(L2));
    
    // Both should be valid values
    // (We don't require them to be different as the formula is complex)
    
    printf("✓ Simplified formula test passed\n");
}

/**
 * @brief Test: Formula breakdown
 */
static void test_formula_breakdown(void) {
    printf("Testing formula breakdown...\n");
    
    LatticeFormulaParams params = {
        .n = 10,
        .d = 6,
        .k = 5,
        .lambda = "default",
        .omega = 432.0,
        .p = 2,
        .q = 1
    };
    
    double O, theta, product, mobius, nu, einstein, psi, gamma;
    
    L_lattice_breakdown(&params, &O, &theta, &product, &mobius, &nu, &einstein, &psi, &gamma);
    
    // Verify all components are valid
    assert(!isnan(O) && !isinf(O));
    assert(!isnan(theta) && !isinf(theta));
    assert(!isnan(product) && !isinf(product));
    assert(!isnan(mobius) && !isinf(mobius));
    assert(!isnan(nu) && !isinf(nu));
    assert(!isnan(einstein) && !isinf(einstein));
    assert(!isnan(psi) && !isinf(psi));
    assert(!isnan(gamma) && !isinf(gamma));
    
    // Verify specific values
    assert(mobius == 1.0 || mobius == -1.0); // Möbius twist
    assert(theta >= 0.0 && theta < 2.0 * M_PI); // Angular position
    
    printf("✓ Formula breakdown test passed\n");
}

/**
 * @brief Test: Formula consistency
 */
static void test_formula_consistency(void) {
    printf("Testing formula consistency...\n");
    
    LatticeFormulaParams params = {
        .n = 100,
        .d = 6,
        .k = 5,
        .lambda = "default",
        .omega = 432.0,
        .p = 2,
        .q = 1
    };
    
    // Calculate multiple times with same parameters
    double L1 = L_lattice_complete(&params, NULL);
    double L2 = L_lattice_complete(&params, NULL);
    double L3 = L_lattice_complete(&params, NULL);
    
    // Results should be identical
    assert(doubles_equal(L1, L2, TEST_EPSILON));
    assert(doubles_equal(L2, L3, TEST_EPSILON));
    
    printf("✓ Formula consistency test passed\n");
}

/**
 * @brief Test: Invalid parameters handling
 */
static void test_invalid_parameters(void) {
    printf("Testing invalid parameters handling...\n");
    
    // NULL params
    double L1 = L_lattice_complete(NULL, NULL);
    assert(L1 == 0.0);
    
    // Invalid dimension
    LatticeFormulaParams invalid = {
        .n = 10,
        .d = 0,
        .k = 5,
        .lambda = "default",
        .omega = 432.0,
        .p = 2,
        .q = 1
    };
    double L2 = L_lattice_complete(&invalid, NULL);
    assert(L2 == 0.0);
    
    printf("✓ Invalid parameters handling test passed\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n=== Lattice Formula Tests ===\n\n");
    
    test_calculate_O();
    test_dimensional_frequency();
    test_mobius_twist();
    test_angular_position_complete();
    test_parameter_validation();
    test_context_init();
    test_L_lattice_complete();
    test_formula_variations();
    test_L_lattice_simple();
    test_formula_breakdown();
    test_formula_consistency();
    test_invalid_parameters();
    
    printf("\n=== All Lattice Formula Tests Passed ===\n\n");
    
    return 0;
}