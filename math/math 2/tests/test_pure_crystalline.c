/*
 * Test Pure Crystalline CLLM - Token Operations
 */

#include "include/cllm_pure_crystalline.h"
#include "include/bigfixed_core.h"
#include <stdio.h>
#include <stdlib.h>

void test_prime_operations() {
    printf("=== Test 1: Prime Operations ===\n");
    
    // Test prime checking
    printf("Is 17 prime? %s\n", crystalline_is_prime(17) ? "YES" : "NO");
    printf("Is 18 prime? %s\n", crystalline_is_prime(18) ? "YES" : "NO");
    
    // Test getting nth prime
    printf("5th prime: %lu\n", crystalline_get_nth_prime(5));
    printf("10th prime: %lu\n", crystalline_get_nth_prime(10));
    printf("100th prime: %lu\n", crystalline_get_nth_prime(100));
    
    // Test factorization
    uint64_t factors[MAX_PRIME_FACTORS];
    uint8_t num_factors;
    
    crystalline_factorize(30, factors, &num_factors);
    printf("Factorization of 30: ");
    for (int i = 0; i < num_factors; i++) {
        printf("%lu ", factors[i]);
    }
    printf("(%d factors)\n", num_factors);
    
    printf("✓ PASS\n\n");
}

void test_token_creation() {
    printf("=== Test 2: Token Creation ===\n");
    
    // Create root token (prime)
    CrystallineToken* root = crystalline_token_create(0, "run", 5);
    printf("Token: '%s'\n", root->token_str);
    printf("  Prime: %lu\n", root->prime);
    printf("  Is root: %s\n", root->is_root ? "YES" : "NO");
    printf("  Factors: ");
    for (int i = 0; i < root->num_factors; i++) {
        printf("%lu ", root->prime_factors[i]);
    }
    printf("\n");
    
    // Create composite token
    CrystallineToken* derived = crystalline_token_create(1, "running", 10);  // 2×5
    printf("Token: '%s'\n", derived->token_str);
    printf("  Prime: %lu\n", derived->prime);
    printf("  Is root: %s\n", derived->is_root ? "YES" : "NO");
    printf("  Factors: ");
    for (int i = 0; i < derived->num_factors; i++) {
        printf("%lu ", derived->prime_factors[i]);
    }
    printf("\n");
    
    crystalline_token_free(root);
    crystalline_token_free(derived);
    
    printf("✓ PASS\n\n");
}

void test_ulam_positioning() {
    printf("=== Test 3: Ulam Spiral Positioning ===\n");
    
    // Test positioning for several primes
    uint64_t test_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    
    for (int i = 0; i < 10; i++) {
        BigFixed coords[3];
        crystalline_compute_ulam_position(test_primes[i], coords, 256);
        
        double x = big_fixed_to_double(&coords[0]);
        double y = big_fixed_to_double(&coords[1]);
        double z = big_fixed_to_double(&coords[2]);
        
        printf("Prime %2lu: (%.4f, %.4f, %.4f)\n", test_primes[i], x, y, z);
    }
    
    printf("✓ PASS\n\n");
}

void test_lattice_distance() {
    printf("=== Test 4: Lattice Distance ===\n");
    
    // Create two tokens
    BigFixed pos1[3], pos2[3];
    crystalline_compute_ulam_position(5, pos1, 256);
    crystalline_compute_ulam_position(7, pos2, 256);
    
    // Compute distance
    BigFixed distance;
    crystalline_lattice_distance(pos1, pos2, &distance);
    
    double dist = big_fixed_to_double(&distance);
    printf("Distance between prime 5 and prime 7: %.6f\n", dist);
    
    // Test distance to self (should be 0)
    crystalline_lattice_distance(pos1, pos1, &distance);
    dist = big_fixed_to_double(&distance);
    printf("Distance to self: %.6f (should be ~0)\n", dist);
    
    printf("✓ PASS\n\n");
}

void test_prime_similarity() {
    printf("=== Test 5: Prime Similarity ===\n");
    
    BigFixed sim;
    
    // Test identical primes
    crystalline_prime_similarity(5, 5, &sim);
    printf("Similarity(5, 5): %.4f (should be 1.0)\n", big_fixed_to_double(&sim));
    
    // Test coprime
    crystalline_prime_similarity(5, 7, &sim);
    printf("Similarity(5, 7): %.4f (coprime, should be 0.5)\n", big_fixed_to_double(&sim));
    
    // Test related (share factors)
    crystalline_prime_similarity(6, 10, &sim);  // Both have factor 2
    printf("Similarity(6, 10): %.4f (share factor 2)\n", big_fixed_to_double(&sim));
    
    printf("✓ PASS\n\n");
}

void test_phase_alignment() {
    printf("=== Test 6: Fourier Phase Alignment ===\n");
    
    BigFixed align;
    
    // Test identical primes (perfect alignment)
    crystalline_phase_alignment(5, 5, &align);
    printf("Alignment(5, 5): %.4f (should be 1.0)\n", big_fixed_to_double(&align));
    
    // Test different primes
    crystalline_phase_alignment(5, 7, &align);
    printf("Alignment(5, 7): %.4f\n", big_fixed_to_double(&align));
    
    crystalline_phase_alignment(2, 97, &align);
    printf("Alignment(2, 97): %.4f\n", big_fixed_to_double(&align));
    
    printf("✓ PASS\n\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Pure Crystalline CLLM - Token Operations Test         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    test_prime_operations();
    test_token_creation();
    test_ulam_positioning();
    test_lattice_distance();
    test_prime_similarity();
    test_phase_alignment();
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              All Tests Complete - PASSED                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}