/**
 * @file test_blind_recovery.c
 * @brief Test suite for Blind Recovery System (OBJECTIVE 26)
 */

#include "cllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_PASSED "\033[32m✓ PASSED\033[0m"
#define TEST_FAILED "\033[31m✗ FAILED\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void print_header(const char* title) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  %-56s  ║\n", title);
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

void print_test_header(const char* test_name) {
    printf("=== %s ===\n", test_name);
}

void test_result(const char* test_name, int passed) {
    if (passed) {
        printf("%s: %s\n\n", test_name, TEST_PASSED);
        tests_passed++;
    } else {
        printf("%s: %s\n\n", test_name, TEST_FAILED);
        tests_failed++;
    }
}

/**
 * Test 1: Corruption Detection
 */
void test_corruption_detection() {
    print_test_header("Test 1: Corruption Detection");
    
    // Create a simple cube model with blind recovery
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 50);
    config.enable_blind_recovery = true;
    config.corruption_tolerance = 0.25; // 25% tolerance
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        test_result("Corruption detection", 0);
        return;
    }
    
    // Initially should have no corruption
    int corruptions = cllm_detect_corruption(model);
    printf("  Initial corruptions: %d\n", corruptions);
    
    // Simulate 10% corruption
    int simulated = cllm_simulate_corruption(model, 0.10);
    printf("  Simulated corruptions: %d\n", simulated);
    
    // Detect corruption
    corruptions = cllm_detect_corruption(model);
    printf("  Detected corruptions: %d\n", corruptions);
    
    int passed = (corruptions > 0 && corruptions == simulated);
    
    cllm_free_model(model);
    test_result("Corruption detection", passed);
}

/**
 * Test 2: Structural Recovery (26A)
 */
void test_structural_recovery() {
    print_test_header("Test 2: Structural Recovery (Euler's Formula)");
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 50);
    config.enable_blind_recovery = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        test_result("Structural recovery", 0);
        return;
    }
    
    // Create backup
    cllm_create_backup(model);
    
    // Simulate corruption
    cllm_simulate_corruption(model, 0.05);
    
    // Apply structural recovery
    int result = cllm_recover_structural(model);
    
    int passed = (result == 0);
    
    cllm_free_model(model);
    test_result("Structural recovery", passed);
}

/**
 * Test 3: Symmetry Recovery (26B)
 */
void test_symmetry_recovery() {
    print_test_header("Test 3: Symmetry Recovery");
    
    // Test with icosahedron (highest symmetry)
    CLLMConfig config = cllm_default_config(PLATONIC_ICOSAHEDRON, 50);
    config.enable_blind_recovery = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        test_result("Symmetry recovery", 0);
        return;
    }
    
    // Simulate corruption
    cllm_simulate_corruption(model, 0.05);
    
    // Apply symmetry recovery
    int result = cllm_recover_symmetry(model);
    
    int passed = (result == 0);
    
    cllm_free_model(model);
    test_result("Symmetry recovery", passed);
}

/**
 * Test 4: Prime-Based Validation (26C)
 */
void test_prime_validation() {
    print_test_header("Test 4: Prime-Based Validation");
    
    CLLMConfig config = cllm_default_config(PLATONIC_OCTAHEDRON, 50);
    config.enable_blind_recovery = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        test_result("Prime validation", 0);
        return;
    }
    
    // Apply prime-based validation
    int result = cllm_recover_prime(model);
    
    int passed = (result == 0);
    
    cllm_free_model(model);
    test_result("Prime validation", passed);
}

/**
 * Test 5: Tetration Attractors (26D)
 */
void test_tetration_recovery() {
    print_test_header("Test 5: Tetration Attractor Recovery");
    
    CLLMConfig config = cllm_default_config(PLATONIC_DODECAHEDRON, 50);
    config.enable_blind_recovery = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        test_result("Tetration recovery", 0);
        return;
    }
    
    // Simulate corruption
    cllm_simulate_corruption(model, 0.05);
    
    // Apply tetration recovery
    int result = cllm_recover_tetration(model);
    
    int passed = (result == 0);
    
    cllm_free_model(model);
    test_result("Tetration recovery", passed);
}

/**
 * Test 6: Comprehensive Recovery
 */
void test_comprehensive_recovery() {
    print_test_header("Test 6: Comprehensive Recovery (All Methods)");
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 100);
    config.enable_blind_recovery = true;
    config.corruption_tolerance = 0.25;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        test_result("Comprehensive recovery", 0);
        return;
    }
    
    // Create backup
    cllm_create_backup(model);
    
    // Simulate 15% corruption
    printf("  Simulating 15%% corruption...\n");
    cllm_simulate_corruption(model, 0.15);
    
    // Apply comprehensive recovery
    RecoveryStats stats = cllm_blind_recovery(model);
    
    // Check recovery rate
    int passed = (stats.recovery_rate >= 0.80); // At least 80% recovery
    
    printf("  Recovery rate: %.1f%%\n", stats.recovery_rate * 100.0);
    printf("  Target: >= 80%%\n");
    
    cllm_free_model(model);
    test_result("Comprehensive recovery", passed);
}

/**
 * Test 7: High Corruption Tolerance
 */
void test_high_corruption() {
    print_test_header("Test 7: High Corruption Tolerance (25%)");
    
    CLLMConfig config = cllm_default_config(PLATONIC_ICOSAHEDRON, 100);
    config.enable_blind_recovery = true;
    config.corruption_tolerance = 0.25;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        test_result("High corruption tolerance", 0);
        return;
    }
    
    // Create backup
    cllm_create_backup(model);
    
    // Simulate 25% corruption (at tolerance limit)
    printf("  Simulating 25%% corruption (at tolerance limit)...\n");
    cllm_simulate_corruption(model, 0.25);
    
    // Apply comprehensive recovery
    RecoveryStats stats = cllm_blind_recovery(model);
    
    // Should recover at least 70% at tolerance limit
    int passed = (stats.recovery_rate >= 0.70);
    
    printf("  Recovery rate: %.1f%%\n", stats.recovery_rate * 100.0);
    printf("  Target: >= 70%% (at tolerance limit)\n");
    
    cllm_free_model(model);
    test_result("High corruption tolerance", passed);
}

/**
 * Test 8: All Platonic Solids Recovery
 */
void test_all_solids_recovery() {
    print_test_header("Test 8: Recovery for All Platonic Solids");
    
    PlatonicSolidType solids[] = {
        PLATONIC_TETRAHEDRON,
        PLATONIC_CUBE,
        PLATONIC_OCTAHEDRON,
        PLATONIC_DODECAHEDRON,
        PLATONIC_ICOSAHEDRON
    };
    
    const char* names[] = {
        "Tetrahedron",
        "Cube",
        "Octahedron",
        "Dodecahedron",
        "Icosahedron"
    };
    
    int all_passed = 1;
    
    for (int i = 0; i < 5; i++) {
        printf("  Testing %s...\n", names[i]);
        
        CLLMConfig config = cllm_default_config(solids[i], 50);
        config.enable_blind_recovery = true;
        
        CLLMModel* model = cllm_create_model(&config);
        if (!model) {
            all_passed = 0;
            continue;
        }
        
        // Create backup and corrupt
        cllm_create_backup(model);
        cllm_simulate_corruption(model, 0.10);
        
        // Recover
        RecoveryStats stats = cllm_blind_recovery(model);
        
        if (stats.recovery_rate < 0.80) {
            all_passed = 0;
            printf("    ⚠️  Low recovery rate: %.1f%%\n", stats.recovery_rate * 100.0);
        } else {
            printf("    ✓ Recovery rate: %.1f%%\n", stats.recovery_rate * 100.0);
        }
        
        cllm_free_model(model);
    }
    
    test_result("All Platonic solids recovery", all_passed);
}

int main() {
    srand(time(NULL));
    
    print_header("CLLM BLIND RECOVERY TEST SUITE (OBJECTIVE 26)");
    
    // Run all tests
    test_corruption_detection();
    test_structural_recovery();
    test_symmetry_recovery();
    test_prime_validation();
    test_tetration_recovery();
    test_comprehensive_recovery();
    test_high_corruption();
    test_all_solids_recovery();
    
    // Print summary
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-43d ║\n", tests_passed);
    printf("║  Tests Failed: %-43d ║\n", tests_failed);
    printf("║  Total Tests:  %-43d ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    if (tests_failed == 0) {
        printf("🎉 ALL TESTS PASSED! 🎉\n\n");
        return 0;
    } else {
        printf("❌ SOME TESTS FAILED ❌\n\n");
        return 1;
    }
}