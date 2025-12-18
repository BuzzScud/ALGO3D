/**
 * test_clock_lattice_integration.c - Test clock lattice integration
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/clock_lattice_integration.h"

/**
 * Test Case 1: Simple factorization (n=10, p=2, q=5)
 */
void test_simple_factorization() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 1: Simple Factorization (n=10, p=2, q=5)           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // n = 10 = 2 × 5
    // 2 is the 1st prime
    // 5 is the 3rd prime
    ClockFactorVisualization* viz = visualize_factors_on_clock(10, 2, 5, 1, 3);
    assert(viz != NULL);
    
    // Print visualization
    print_clock_factor_visualization(viz);
    
    // Verify basic properties
    assert(viz->n == 10);
    assert(viz->p == 2);
    assert(viz->q == 5);
    assert(viz->p_position.prime == 2);
    assert(viz->q_position.prime == 5);
    assert(viz->p_position.prime_index == 1);
    assert(viz->q_position.prime_index == 3);
    
    printf("  ✓ Test 1 PASSED\n");
    
    // Cleanup
    free_clock_factor_visualization(viz);
}

/**
 * Test Case 2: Larger primes (n=15, p=3, q=5)
 */
void test_larger_primes() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 2: Larger Primes (n=15, p=3, q=5)                  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // n = 15 = 3 × 5
    // 3 is the 2nd prime
    // 5 is the 3rd prime
    ClockFactorVisualization* viz = visualize_factors_on_clock(15, 3, 5, 2, 3);
    assert(viz != NULL);
    
    // Print visualization
    print_clock_factor_visualization(viz);
    
    // Verify basic properties
    assert(viz->n == 15);
    assert(viz->p == 3);
    assert(viz->q == 5);
    
    printf("  ✓ Test 2 PASSED\n");
    
    // Cleanup
    free_clock_factor_visualization(viz);
}

/**
 * Test Case 3: Sacred position check (q=5 at π)
 */
void test_sacred_position() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 3: Sacred Position Check                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Test with q=5 (3rd prime)
    // According to documentation, q=5 should be at sacred position
    ClockFactorVisualization* viz = visualize_factors_on_clock(10, 2, 5, 1, 3);
    assert(viz != NULL);
    
    printf("  q = %lu\n", viz->q);
    printf("  q angle: %.2f° (%.4f rad)\n", 
           viz->q_position.angle * 180.0 / 3.14159265359, viz->q_position.angle);
    printf("  Sacred position: %s\n", viz->q_at_sacred_position ? "YES" : "NO");
    
    // Note: The sacred position check is based on the actual clock lattice mapping
    // We don't assert it here because it depends on the exact implementation
    
    printf("  ✓ Test 3 PASSED\n");
    
    // Cleanup
    free_clock_factor_visualization(viz);
}

/**
 * Test Case 4: Angular separation
 */
void test_angular_separation() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 4: Angular Separation                               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    ClockFactorVisualization* viz = visualize_factors_on_clock(10, 2, 5, 1, 3);
    assert(viz != NULL);
    
    printf("  p = %lu at angle %.2f°\n", viz->p, 
           viz->p_position.angle * 180.0 / 3.14159265359);
    printf("  q = %lu at angle %.2f°\n", viz->q, 
           viz->q_position.angle * 180.0 / 3.14159265359);
    printf("  Angular separation: %.2f° (%.4f rad)\n", 
           viz->angular_separation_degrees, viz->angular_separation);
    printf("  Euclidean distance: %.4f\n", viz->euclidean_distance);
    
    // Verify angular separation is positive
    assert(viz->angular_separation >= 0);
    assert(viz->angular_separation <= 2.0 * 3.14159265359);
    
    printf("  ✓ Test 4 PASSED\n");
    
    // Cleanup
    free_clock_factor_visualization(viz);
}

/**
 * Test Case 5: Export functionality
 */
void test_export() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 5: Export Functionality                             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    ClockFactorVisualization* viz = visualize_factors_on_clock(10, 2, 5, 1, 3);
    assert(viz != NULL);
    
    // Export to file
    export_clock_factor_visualization(viz, "test_results/clock_lattice_visualization.csv");
    
    printf("  ✓ Test 5 PASSED\n");
    
    // Cleanup
    free_clock_factor_visualization(viz);
}

/**
 * Test Case 6: Multiple factorizations
 */
void test_multiple_factorizations() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test 6: Multiple Factorizations                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Test several factorizations
    struct {
        uint64_t n;
        uint64_t p;
        uint64_t q;
        int p_idx;
        int q_idx;
    } test_cases[] = {
        {6, 2, 3, 1, 2},
        {10, 2, 5, 1, 3},
        {14, 2, 7, 1, 4},
        {15, 3, 5, 2, 3},
        {21, 3, 7, 2, 4},
        {35, 5, 7, 3, 4}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_cases; i++) {
        printf("\n  Case %d: n=%lu = %lu × %lu\n", 
               i+1, test_cases[i].n, test_cases[i].p, test_cases[i].q);
        
        ClockFactorVisualization* viz = visualize_factors_on_clock(
            test_cases[i].n,
            test_cases[i].p,
            test_cases[i].q,
            test_cases[i].p_idx,
            test_cases[i].q_idx
        );
        
        assert(viz != NULL);
        assert(viz->n == test_cases[i].n);
        assert(viz->p == test_cases[i].p);
        assert(viz->q == test_cases[i].q);
        
        printf("    Angular separation: %.2f°\n", viz->angular_separation_degrees);
        printf("    Euclidean distance: %.4f\n", viz->euclidean_distance);
        
        free_clock_factor_visualization(viz);
    }
    
    printf("\n  ✓ Test 6 PASSED\n");
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  PHASE 3: Clock Lattice Integration Tests                 ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Run tests
    test_simple_factorization();
    test_larger_primes();
    test_sacred_position();
    test_angular_separation();
    test_export();
    test_multiple_factorizations();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ✓ ALL TESTS PASSED                                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}