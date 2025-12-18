#include "math/abacus.h"
#include <stdio.h>
#include <stdlib.h>

int tests_passed = 0;
int tests_total = 0;

void test_division(const char* name, uint64_t a, uint64_t b, uint64_t expected_q, uint64_t expected_r) {
    tests_total++;
    printf("Test %d: %lu / %lu = %lu remainder %lu\n", tests_total, a, b, expected_q, expected_r);
    
    CrystallineAbacus* num_a = abacus_from_uint64(a, 10);
    CrystallineAbacus* num_b = abacus_from_uint64(b, 10);
    CrystallineAbacus* quotient = abacus_new(10);
    CrystallineAbacus* remainder = abacus_new(10);
    
    MathError err = abacus_div(quotient, remainder, num_a, num_b);
    
    if (err == MATH_SUCCESS) {
        uint64_t q, r;
        abacus_to_uint64(quotient, &q);
        abacus_to_uint64(remainder, &r);
        
        if (q == expected_q && r == expected_r) {
            printf("  ✓ PASS\n");
            tests_passed++;
        } else {
            printf("  ✗ FAIL: Got q=%lu, r=%lu\n", q, r);
        }
    } else {
        printf("  ✗ FAIL: Error %d\n", err);
    }
    
    abacus_free(num_a);
    abacus_free(num_b);
    abacus_free(quotient);
    abacus_free(remainder);
}

int main() {
    printf("=== Comprehensive Abacus Division Tests ===\n\n");
    
    // Basic tests
    printf("=== Basic Division ===\n");
    test_division("10/2", 10, 2, 5, 0);
    test_division("10/3", 10, 3, 3, 1);
    test_division("15/4", 15, 4, 3, 3);
    test_division("100/7", 100, 7, 14, 2);
    
    // Edge cases
    printf("\n=== Edge Cases ===\n");
    test_division("1/1", 1, 1, 1, 0);
    test_division("0/5", 0, 5, 0, 0);
    test_division("5/10", 5, 10, 0, 5);
    test_division("7/7", 7, 7, 1, 0);
    
    // Large numbers
    printf("\n=== Large Numbers ===\n");
    test_division("1000/3", 1000, 3, 333, 1);
    test_division("9999/11", 9999, 11, 909, 0);
    test_division("12345/67", 12345, 67, 184, 17);
    
    // Powers
    printf("\n=== Powers ===\n");
    test_division("64/8", 64, 8, 8, 0);
    test_division("125/5", 125, 5, 25, 0);
    test_division("256/16", 256, 16, 16, 0);
    
    printf("\n=================================================\n");
    printf("RESULTS: %d/%d tests passed (%.1f%%)\n", 
           tests_passed, tests_total, 
           tests_total > 0 ? (100.0 * tests_passed / tests_total) : 0.0);
    printf("=================================================\n");
    
    return tests_passed == tests_total ? 0 : 1;
}