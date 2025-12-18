// prime_demo.c - Demonstration of Prime Math Library capabilities
#include "prime_math.h"
#include <stdio.h>
#include <stdlib.h>

void demonstrate_bigint_operations() {
    printf("=== BigInt Operations Demo ===\n");
    
    BigInt a, b, sum, product, quotient, remainder;
    big_init(&a);
    big_init(&b);
    big_init(&sum);
    big_init(&product);
    big_init(&quotient);
    big_init(&remainder);
    
    // Create large numbers
    big_from_int(&a, 123456789);
    big_from_int(&b, 987654321);
    
    // Perform operations
    big_add(&a, &b, &sum);
    big_mul(&a, &b, &product);
    big_div(&a, &b, &quotient, &remainder);
    
    // Display results
    char* a_str = big_to_string(&a);
    char* b_str = big_to_string(&b);
    char* sum_str = big_to_string(&sum);
    char* product_str = big_to_string(&product);
    char* quot_str = big_to_string(&quotient);
    char* rem_str = big_to_string(&remainder);
    
    printf("a = %s\n", a_str);
    printf("b = %s\n", b_str);
    printf("a + b = %s\n", sum_str);
    printf("a * b = %s\n", product_str);
    printf("a / b = %s, remainder %s\n", quot_str, rem_str);
    
    // Cleanup
    free(a_str);
    free(b_str);
    free(sum_str);
    free(product_str);
    free(quot_str);
    free(rem_str);
    
    big_free(&a);
    big_free(&b);
    big_free(&sum);
    big_free(&product);
    big_free(&quotient);
    big_free(&remainder);
    
    printf("\n");
}

void demonstrate_prime_generation() {
    printf("=== Prime Generation Demo ===\n");
    
    CrystalAbacus* abacus = abacus_create();
    printf("Crystal Abacus created successfully\n");
    
    printf("First 10 primes: ");
    for (int i = 0; i < 10; i++) {
        int prime = abacus_next_prime(abacus);
        printf("%d ", prime);
    }
    printf("\n");
    
    // Test primality
    int test_numbers[] = {17, 25, 97, 100, 151};
    printf("Primality tests:\n");
    for (int i = 0; i < 5; i++) {
        int n = test_numbers[i];
        bool is_p = abacus_is_prime(abacus, n);
        printf("  %d is %sprime\n", n, is_p ? "" : "not ");
    }
    
    abacus_free(abacus);
    printf("\n");
}

void demonstrate_lattice_mathematics() {
    printf("=== Lattice Mathematics Demo ===\n");
    
    printf("Lattice depth comparison for sin(π/4):\n");
    double x = 3.141592653589793 / 4.0;
    
    for (int depth = 0; depth <= 5; depth++) {
        double result = lattice_sin(x, depth);
        printf("  Depth %d: %.6f\n", depth, result);
    }
    
    printf("\nLattice arithmetic examples:\n");
    double a = 10.0, b = 3.0;
    
    for (int depth = 1; depth <= 3; depth++) {
        double sum = lattice_add(a, b, depth);
        double product = lattice_multiply(a, b, depth);
        printf("  Depth %d: %.1f + %.1f = %.6f, %.1f * %.1f = %.6f\n",
               depth, a, b, sum, a, b, product);
    }
    
    printf("\n");
}

void demonstrate_trigonometric_functions() {
    printf("=== Trigonometric Functions Demo ===\n");
    
    double angles[] = {0.0, 3.14159/6.0, 3.14159/4.0, 3.14159/3.0, 3.14159/2.0};
    int num_angles = sizeof(angles) / sizeof(angles[0]);
    
    printf("Angle\t\tPrime Sin\tLattice Sin\tPrime Cos\tLattice Cos\n");
    printf("------\t\t---------\t------------\t---------\t------------\n");
    
    for (int i = 0; i < num_angles; i++) {
        double angle = angles[i];
        double sin_p = math_sin(angle);
        double sin_l = lattice_sin(angle, 3);
        double cos_p = math_cos(angle);
        double cos_l = lattice_cos(angle, 3);
        
        printf("%.4f\t\t%.6f\t%.6f\t%.6f\t%.6f\n",
               angle, sin_p, sin_l, cos_p, cos_l);
    }
    
    printf("\n");
}

void demonstrate_prime_properties() {
    printf("=== Prime Properties Demo ===\n");
    
    BigInt p1, p2, test_num;
    big_init(&p1);
    big_init(&p2);
    big_init(&test_num);
    
    // Test various prime properties
    printf("Twin Prime Tests:\n");
    int twin_pairs[][2] = {{3,5}, {5,7}, {11,13}, {17,19}, {23,25}};
    for (int i = 0; i < 5; i++) {
        big_from_int(&p1, twin_pairs[i][0]);
        big_from_int(&p2, twin_pairs[i][1]);
        bool are_twins = big_are_twin_primes(&p1, &p2);
        printf("  %d and %d: %s\n", twin_pairs[i][0], twin_pairs[i][1],
               are_twins ? "twin primes ✓" : "not twin primes");
    }
    
    printf("\nMersenne Prime Tests:\n");
    int mersenne_candidates[] = {3, 7, 15, 31, 63, 127};
    for (int i = 0; i < 6; i++) {
        big_from_int(&test_num, mersenne_candidates[i]);
        bool is_mersenne = big_is_mersenne_prime(&test_num);
        printf("  %d: %s\n", mersenne_candidates[i],
               is_mersenne ? "Mersenne prime ✓" : "not Mersenne prime");
    }
    
    printf("\nSophie Germain Prime Tests:\n");
    int sophie_candidates[] = {2, 3, 5, 11, 23, 29};
    for (int i = 0; i < 6; i++) {
        big_from_int(&test_num, sophie_candidates[i]);
        bool is_sophie = big_is_sophie_germain_prime(&test_num);
        printf("  %d: %s\n", sophie_candidates[i],
               is_sophie ? "Sophie Germain prime ✓" : "not Sophie Germain prime");
    }
    
    big_free(&p1);
    big_free(&p2);
    big_free(&test_num);
    
    printf("\n");
}

void demonstrate_library_info() {
    printf("=== Library Information ===\n");
    
    printf("Library Version: %s\n", prime_math_version());
    printf("Description: %s\n", prime_math_description());
    
    PrimeMathStats stats = prime_math_stats();
    printf("Current Stats:\n");
    printf("  Lattice Depth: %d\n", stats.lattice_depth);
    printf("  Prime Cache Size: %d\n", stats.prime_cache_size);
    printf("  Rainbow Table Count: %d\n", stats.rainbow_table_count);
    printf("  Rainbow Stable: %s\n", stats.rainbow_stable ? "Yes" : "No");
    printf("  Abacus Primes Generated: %d\n", stats.abacus_primes_generated);
    
    printf("\n");
}

int main() {
    printf("Prime Math Library - Comprehensive Demo\n");
    printf("========================================\n\n");
    
    // Initialize library with lattice depth 4
    prime_math_init_depth(4);
    
    // Run demonstrations
    demonstrate_bigint_operations();
    demonstrate_prime_generation();
    demonstrate_lattice_mathematics();
    demonstrate_trigonometric_functions();
    demonstrate_prime_properties();
    demonstrate_library_info();
    
    printf("Demo completed successfully! 🎉\n");
    printf("\nNote: This is an alpha implementation with many placeholder functions.\n");
    printf("The core architecture is functional and demonstrates the prime-based\n");
    printf("mathematics approach. Full mathematical functionality requires\n");
    printf("completing the remaining implementations.\n");
    
    // Cleanup
    prime_math_cleanup();
    
    return 0;
}