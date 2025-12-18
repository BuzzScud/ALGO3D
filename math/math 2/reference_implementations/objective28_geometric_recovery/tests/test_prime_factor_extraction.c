/**
 * test_prime_factor_extraction.c - Test p/q extraction from torus structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/prime_factor_extraction.h"
#include "../include/multi_torus_tracker.h"

/**
 * Test Case 1: Simple case with n=10 (p=2, q=5)
 */
void test_simple_extraction() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 1: Simple Extraction (n=10, p=2, q=5)              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Create tracker
    MultiTorusTracker* tracker = create_multi_torus_tracker(20, 100, 10);
    assert(tracker != NULL);
    
    // Simulate oscillations with periods 2 and 5
    // This simulates the torus structure for n=10
    for (int i = 0; i < 100; i++) {
        // Oscillation with period 2 (p=2)
        double osc_p = 2.0 * math_sin((double)i * 3.14159 / 2.0);
        
        // Oscillation with period 5 (q=5)
        double osc_q = 3.0 * math_sin((double)i * 3.14159 / 5.0);
        
        // Combined signal
        double k_estimate = 5.0 + osc_p + osc_q;
        
        multi_torus_add_sample(tracker, k_estimate);
    }
    
    // Identify tori
    int num_tori = identify_tori(tracker);
    printf("  Identified %d tori\n", num_tori);
    assert(num_tori >= 2);
    
    // Extract p and q
    PrimeFactorResult* result = extract_prime_factors_from_torus(tracker, 10);
    assert(result != NULL);
    
    // Print results
    print_prime_factor_result(result);
    
    // Verify extraction
    assert(result->extraction_successful);
    assert(result->verification_passed);
    assert(result->p == 2);
    assert(result->q == 5);
    assert(result->p * result->q == 10);
    
    printf("  ✓ Test 1 PASSED\n");
    
    // Cleanup
    free_prime_factor_result(result);
    free_multi_torus_tracker(tracker);
}

/**
 * Test Case 2: Larger primes (n=15, p=3, q=5)
 */
void test_larger_primes() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 2: Larger Primes (n=15, p=3, q=5)                  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Create tracker
    MultiTorusTracker* tracker = create_multi_torus_tracker(20, 150, 15);
    assert(tracker != NULL);
    
    // Simulate oscillations with periods 3 and 5
    for (int i = 0; i < 150; i++) {
        // Oscillation with period 3 (p=3)
        double osc_p = 2.5 * math_sin((double)i * 3.14159 / 3.0);
        
        // Oscillation with period 5 (q=5)
        double osc_q = 3.5 * math_sin((double)i * 3.14159 / 5.0);
        
        // Combined signal
        double k_estimate = 7.5 + osc_p + osc_q;
        
        multi_torus_add_sample(tracker, k_estimate);
    }
    
    // Identify tori
    int num_tori = identify_tori(tracker);
    printf("  Identified %d tori\n", num_tori);
    assert(num_tori >= 2);
    
    // Extract p and q
    PrimeFactorResult* result = extract_prime_factors_from_torus(tracker, 15);
    assert(result != NULL);
    
    // Print results
    print_prime_factor_result(result);
    
    // Verify extraction
    assert(result->extraction_successful);
    assert(result->verification_passed);
    assert(result->p == 3);
    assert(result->q == 5);
    assert(result->p * result->q == 15);
    
    printf("  ✓ Test 2 PASSED\n");
    
    // Cleanup
    free_prime_factor_result(result);
    free_multi_torus_tracker(tracker);
}

/**
 * Test Case 3: Export functionality
 */
void test_export() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 3: Export Functionality                             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Create tracker
    MultiTorusTracker* tracker = create_multi_torus_tracker(20, 100, 10);
    assert(tracker != NULL);
    
    // Simulate oscillations
    for (int i = 0; i < 100; i++) {
        double osc_p = 2.0 * math_sin((double)i * 3.14159 / 2.0);
        double osc_q = 3.0 * math_sin((double)i * 3.14159 / 5.0);
        double k_estimate = 5.0 + osc_p + osc_q;
        multi_torus_add_sample(tracker, k_estimate);
    }
    
    // Identify tori and extract factors
    identify_tori(tracker);
    PrimeFactorResult* result = extract_prime_factors_from_torus(tracker, 10);
    assert(result != NULL);
    
    // Export to file
    export_prime_factor_result(result, "test_results/prime_factor_extraction.csv");
    
    printf("  ✓ Test 3 PASSED\n");
    
    // Cleanup
    free_prime_factor_result(result);
    free_multi_torus_tracker(tracker);
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  PHASE 2: Prime Factor Extraction Tests                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Run tests
    test_simple_extraction();
    test_larger_primes();
    test_export();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ✓ ALL TESTS PASSED                                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}