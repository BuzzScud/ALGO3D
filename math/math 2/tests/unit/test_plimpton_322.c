/**
 * Unit tests for Plimpton 322 system
 * Tests Pythagorean triple generation and ratio calculations
 */

#include "../../include/plimpton_322.h"
#include <stdio.h>
#include <assert.h>
#include "math/math.h"

#define MATH_EPSILON 0.0001

void test_coprime_verification() {
    printf("Testing coprime verification...\n");
    
    // Test coprime pairs
    assert(is_coprime(3, 2) == true);   // gcd(3,2) = 1
    assert(is_coprime(5, 3) == true);   // gcd(5,3) = 1
    assert(is_coprime(7, 4) == true);   // gcd(7,4) = 1
    
    // Test non-coprime pairs
    assert(is_coprime(4, 2) == false);  // gcd(4,2) = 2
    assert(is_coprime(6, 3) == false);  // gcd(6,3) = 3
    assert(is_coprime(10, 5) == false); // gcd(10,5) = 5
    
    printf("✓ Coprime verification works correctly\n");
}

void test_plimpton_constraints() {
    printf("Testing Plimpton 322 constraints...\n");
    
    // Valid pairs
    assert(verify_plimpton_constraints(3, 2) == true);   // coprime, not both odd, p>q
    assert(verify_plimpton_constraints(4, 3) == true);   // coprime, not both odd, p>q
    assert(verify_plimpton_constraints(5, 2) == true);   // coprime, not both odd, p>q
    
    // Invalid: both odd
    assert(verify_plimpton_constraints(3, 1) == false);  // both odd
    assert(verify_plimpton_constraints(5, 3) == false);  // both odd
    
    // Invalid: not coprime
    assert(verify_plimpton_constraints(4, 2) == false);  // gcd(4,2) = 2
    assert(verify_plimpton_constraints(6, 3) == false);  // gcd(6,3) = 3
    
    // Invalid: p <= q
    assert(verify_plimpton_constraints(2, 3) == false);  // p < q
    assert(verify_plimpton_constraints(2, 2) == false);  // p = q
    
    printf("✓ Plimpton constraints verification works correctly\n");
}

void test_plimpton_ratios() {
    printf("Testing Plimpton 322 ratio calculations...\n");
    
    // Test with known triple (3,4,5) from p=2, q=1
    PlimptonRatios ratios1 = calculate_plimpton_ratios(2, 1);
    
    // For p=2, q=1:
    // p²=4, q²=1, p²+q²=5, p²-q²=3, 2pq=4
    // ratio_b_d = 3/5 = 0.6
    // ratio_c_d = 4/5 = 0.8
    assert(fabs(ratios1.ratio_b_d - 0.6) < MATH_EPSILON);
    assert(fabs(ratios1.ratio_c_d - 0.8) < MATH_EPSILON);
    
    // Verify ratios sum to 1.0 (approximately, due to product)
    // Actually they don't sum to 1, but their squares do: (3/5)² + (4/5)² = 1
    
    printf("  p=2, q=1: b/d=%.4f, c/d=%.4f, product=%.4f\n", 
           ratios1.ratio_b_d, ratios1.ratio_c_d, ratios1.product);
    
    // Test with another known triple (5,12,13) from p=3, q=2
    PlimptonRatios ratios2 = calculate_plimpton_ratios(3, 2);
    
    // For p=3, q=2:
    // p²=9, q²=4, p²+q²=13, p²-q²=5, 2pq=12
    // ratio_b_d = 5/13 ≈ 0.3846
    // ratio_c_d = 12/13 ≈ 0.9231
    assert(fabs(ratios2.ratio_b_d - 5.0/13.0) < MATH_EPSILON);
    assert(fabs(ratios2.ratio_c_d - 12.0/13.0) < MATH_EPSILON);
    
    printf("  p=3, q=2: b/d=%.4f, c/d=%.4f, product=%.4f\n", 
           ratios2.ratio_b_d, ratios2.ratio_c_d, ratios2.product);
    
    printf("✓ Plimpton ratio calculations are correct\n");
}

void test_pythagorean_triple_generation() {
    printf("Testing Pythagorean triple generation...\n");
    
    // Test known triples
    PythagoreanTriple triple1, triple2, triple3;
    generate_pythagorean_triple(2, 1, &triple1);
    generate_pythagorean_triple(3, 2, &triple2);
    generate_pythagorean_triple(4, 1, &triple3);
    
    assert(triple1.a == 3);
    assert(triple1.b == 4);
    assert(triple1.c == 5);
    
    assert(triple2.a == 5);
    assert(triple2.b == 12);
    assert(triple2.c == 13);
    
    assert(triple3.a == 15);
    assert(triple3.b == 8);
    assert(triple3.c == 17);
    
    printf("  (2,1) → (%lu, %lu, %lu)\n", triple1.a, triple1.b, triple1.c);
    printf("  (3,2) → (%lu, %lu, %lu)\n", triple2.a, triple2.b, triple2.c);
    printf("  (4,1) → (%lu, %lu, %lu)\n", triple3.a, triple3.b, triple3.c);
    
    printf("✓ Pythagorean triple generation works correctly\n");
}

void test_pythagorean_triple_verification() {
    printf("Testing Pythagorean triple verification...\n");
    
    // Generate and verify triples
    PythagoreanTriple triple1, triple2, triple3;
    generate_pythagorean_triple(2, 1, &triple1);
    generate_pythagorean_triple(3, 2, &triple2);
    generate_pythagorean_triple(4, 1, &triple3);
    
    assert(verify_pythagorean_triple_validity(&triple1) == true);
    assert(verify_pythagorean_triple_validity(&triple2) == true);
    assert(verify_pythagorean_triple_validity(&triple3) == true);
    
    // Verify the Pythagorean theorem manually
    // (3,4,5): 3² + 4² = 9 + 16 = 25 = 5²
    assert(3*3 + 4*4 == 5*5);
    
    // (5,12,13): 5² + 12² = 25 + 144 = 169 = 13²
    assert(5*5 + 12*12 == 13*13);
    
    printf("  (3,4,5): 3² + 4² = %d = 5² ✓\n", 3*3 + 4*4);
    printf("  (5,12,13): 5² + 12² = %d = 13² ✓\n", 5*5 + 12*12);
    
    printf("✓ Pythagorean triple verification works correctly\n");
}

void test_optimal_generators() {
    printf("Testing optimal generator finding...\n");
    
    uint64_t p, q;
    
    // Test with various primes
    bool found1 = find_optimal_generators(5, &p, &q);
    assert(found1 == true);
    assert(verify_plimpton_constraints(p, q) == true);
    printf("  Prime 5: p=%lu, q=%lu (constraints satisfied)\n", p, q);
    
    bool found2 = find_optimal_generators(13, &p, &q);
    assert(found2 == true);
    assert(verify_plimpton_constraints(p, q) == true);
    printf("  Prime 13: p=%lu, q=%lu (constraints satisfied)\n", p, q);
    
    bool found3 = find_optimal_generators(17, &p, &q);
    assert(found3 == true);
    assert(verify_plimpton_constraints(p, q) == true);
    printf("  Prime 17: p=%lu, q=%lu (constraints satisfied)\n", p, q);
    
    printf("✓ Optimal generator finding works correctly\n");
}

void test_ratio_properties() {
    printf("Testing ratio mathematical properties...\n");
    
    // Test that ratios satisfy Pythagorean relationship
    PlimptonRatios ratios = calculate_plimpton_ratios(3, 2);
    
    // For Pythagorean triples: (b/d)² + (c/d)² = 1
    double sum_of_squares = ratios.ratio_b_d * ratios.ratio_b_d + 
                           ratios.ratio_c_d * ratios.ratio_c_d;
    
    printf("  (b/d)² + (c/d)² = %.6f (should be 1.0)\n", sum_of_squares);
    assert(fabs(sum_of_squares - 1.0) < MATH_EPSILON);
    
    printf("✓ Ratios satisfy Pythagorean relationship\n");
}

void test_known_plimpton_triples() {
    printf("Testing known Plimpton 322 triples...\n");
    
    // Known triples from Plimpton 322 tablet
    struct {
        uint64_t p, q;
        uint64_t expected_a, expected_b, expected_c;
    } known_triples[] = {
        {2, 1, 3, 4, 5},
        {3, 2, 5, 12, 13},
        {4, 1, 15, 8, 17},
        {4, 3, 7, 24, 25},
        {5, 2, 21, 20, 29},
        {5, 4, 9, 40, 41},
    };
    
    int num_triples = sizeof(known_triples) / sizeof(known_triples[0]);
    
    for (int i = 0; i < num_triples; i++) {
        PythagoreanTriple triple;
        generate_pythagorean_triple(
            known_triples[i].p,
            known_triples[i].q,
            &triple
        );
        
        assert(triple.a == known_triples[i].expected_a);
        assert(triple.b == known_triples[i].expected_b);
        assert(triple.c == known_triples[i].expected_c);
        assert(verify_pythagorean_triple_validity(&triple) == true);
        
        printf("  (%lu,%lu) → (%lu,%lu,%lu) ✓\n", 
               known_triples[i].p, known_triples[i].q,
               triple.a, triple.b, triple.c);
    }
    
    printf("✓ All known Plimpton 322 triples verified\n");
}

int main() {
    printf("\n=== Plimpton 322 System Unit Tests ===\n\n");
    
    test_coprime_verification();
    printf("\n");
    
    test_plimpton_constraints();
    printf("\n");
    
    test_plimpton_ratios();
    printf("\n");
    
    test_pythagorean_triple_generation();
    printf("\n");
    
    test_pythagorean_triple_verification();
    printf("\n");
    
    test_optimal_generators();
    printf("\n");
    
    test_ratio_properties();
    printf("\n");
    
    test_known_plimpton_triples();
    printf("\n");
    
    printf("✅ All Plimpton 322 tests passed!\n\n");
    
    return 0;
}