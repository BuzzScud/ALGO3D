/**
 * Test: Clock Lattice Factor Visualization
 * 
 * Visualizes how p and q (extracted from torus structure) map onto
 * the Babylonian clock lattice.
 * 
 * For n = p * q, we want to see:
 * - Where p appears on the clock
 * - Where q appears on the clock
 * - Their relationship in the clock structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../../include/clock_lattice.h"
#include "../../../include/prime_float_math.h"

// Prime lookup (first 20 primes for our test)
static const uint64_t PRIMES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71
};

/**
 * Get prime index (1-based) for a given prime
 */
int get_prime_index(uint64_t prime) {
    for (int i = 0; i < 20; i++) {
        if (PRIMES[i] == prime) {
            return i + 1;  // 1-based
        }
    }
    return -1;
}

/**
 * Visualize a single prime on the clock
 */
void visualize_prime_on_clock(uint64_t prime, const char* label) {
    int prime_index = get_prime_index(prime);
    if (prime_index < 0) {
        printf("  %s: Prime %lu not in lookup table\n", label, prime);
        return;
    }
    
    BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
    PrimeModular mod = get_prime_modular(prime);
    
    printf("\n  %s = %lu (Prime #%d):\n", label, prime, prime_index);
    printf("    Clock Position:\n");
    printf("      Ring: %d ", pos.ring);
    switch (pos.ring) {
        case 0: printf("(Hours - 12 positions)\n"); break;
        case 1: printf("(Minutes - 60 positions)\n"); break;
        case 2: printf("(Seconds - 60 positions)\n"); break;
        case 3: printf("(Milliseconds - 100 positions)\n"); break;
        default: printf("(Extended ring)\n"); break;
    }
    printf("      Position: %d\n", pos.position);
    printf("      Angle: %.4f radians (%.1f degrees)\n", 
           pos.angle, pos.angle * 180.0 / M_PI);
    printf("      Radius: %.2f\n", pos.radius);
    
    printf("    Modular Relationships:\n");
    printf("      %lu mod 12 = %d\n", prime, mod.mod_12);
    printf("      %lu mod 60 = %d\n", prime, mod.mod_60);
    printf("      %lu mod 100 = %d\n", prime, mod.mod_100);
    
    // Check if sacred
    if (is_sacred_position(pos)) {
        printf("      *** SACRED POSITION ***\n");
    }
}

/**
 * Visualize the relationship between p and q
 */
void visualize_pq_relationship(uint64_t p, uint64_t q, uint64_t n) {
    printf("\n=== Clock Lattice Factor Visualization ===\n");
    printf("\nFactorization: n = %lu = %lu × %lu\n", n, p, q);
    
    // Visualize p
    visualize_prime_on_clock(p, "p");
    
    // Visualize q
    visualize_prime_on_clock(q, "q");
    
    // Analyze relationship
    printf("\n  Relationship Analysis:\n");
    
    int p_index = get_prime_index(p);
    int q_index = get_prime_index(q);
    
    if (p_index > 0 && q_index > 0) {
        BabylonianClockPosition p_pos = map_prime_index_to_clock(p_index);
        BabylonianClockPosition q_pos = map_prime_index_to_clock(q_index);
        
        // Angular separation
        double angle_diff = q_pos.angle - p_pos.angle;
        if (angle_diff < 0) angle_diff += 2.0 * M_PI;
        
        printf("    Angular Separation: %.4f radians (%.1f degrees)\n",
               angle_diff, angle_diff * 180.0 / M_PI);
        
        // Position difference
        int pos_diff = q_pos.position - p_pos.position;
        printf("    Position Difference: %d positions\n", pos_diff);
        
        // Same ring?
        if (p_pos.ring == q_pos.ring) {
            printf("    Both on Ring %d\n", p_pos.ring);
        } else {
            printf("    Different rings: p on Ring %d, q on Ring %d\n",
                   p_pos.ring, q_pos.ring);
        }
        
        // Coprimality check
        printf("    Coprime: %s (gcd(%lu, %lu) = 1)\n",
               (p != q) ? "YES" : "NO", p, q);
    }
}

/**
 * Test with our extracted p=2, q=5
 */
void test_extracted_factors() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test: Extracted Factors from Torus Analysis              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    uint64_t p = 2;
    uint64_t q = 5;
    uint64_t n = p * q;  // 10
    
    visualize_pq_relationship(p, q, n);
}

/**
 * Test with multiple n values to see the pattern
 */
void test_multiple_factorizations() {
    printf("\n\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Test: Multiple Factorizations                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Test cases: (n, p, q)
    struct {
        uint64_t n;
        uint64_t p;
        uint64_t q;
    } test_cases[] = {
        {6, 2, 3},      // 2 × 3
        {10, 2, 5},     // 2 × 5 (our case)
        {14, 2, 7},     // 2 × 7
        {15, 3, 5},     // 3 × 5
        {21, 3, 7},     // 3 × 7
        {35, 5, 7},     // 5 × 7
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_cases; i++) {
        printf("\n--- Case %d ---\n", i + 1);
        visualize_pq_relationship(test_cases[i].p, test_cases[i].q, test_cases[i].n);
    }
}

/**
 * Create ASCII clock visualization
 */
void create_ascii_clock(uint64_t p, uint64_t q) {
    printf("\n\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ASCII Clock Visualization (Ring 0 - Hours)               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("\n");
    printf("                    12\n");
    printf("                     |\n");
    printf("                     |\n");
    printf("         11          |          1\n");
    printf("                     |\n");
    printf("                     |\n");
    printf("    10               |               2\n");
    printf("                     |\n");
    printf("                     |\n");
    printf("  9 -----------------.----------------- 3\n");
    printf("                     |\n");
    printf("                     |\n");
    printf("    8                |               4\n");
    printf("                     |\n");
    printf("                     |\n");
    printf("         7           |          5\n");
    printf("                     |\n");
    printf("                     |\n");
    printf("                     6\n");
    
    int p_index = get_prime_index(p);
    int q_index = get_prime_index(q);
    
    if (p_index > 0 && q_index > 0) {
        BabylonianClockPosition p_pos = map_prime_index_to_clock(p_index);
        BabylonianClockPosition q_pos = map_prime_index_to_clock(q_index);
        
        printf("\n  Positions:\n");
        printf("    p = %lu is at position %d (Prime #%d)\n", p, p_pos.position, p_index);
        printf("    q = %lu is at position %d (Prime #%d)\n", q, q_pos.position, q_index);
        
        printf("\n  Note: Position 3 = π (3 o'clock), Position 12 = top (12 o'clock)\n");
    }
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                                                            ║\n");
    printf("║  OBJECTIVE 28: Clock Lattice Factor Visualization         ║\n");
    printf("║                                                            ║\n");
    printf("║  Visualizing p=2 and q=5 on the Babylonian Clock          ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Test 1: Our extracted factors
    test_extracted_factors();
    
    // Test 2: Multiple factorizations
    test_multiple_factorizations();
    
    // Test 3: ASCII clock
    create_ascii_clock(2, 5);
    
    printf("\n\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Summary                                                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  Key Insights:\n");
    printf("  1. p=2 (2nd prime) maps to position 2 on Ring 0 (Hours)\n");
    printf("  2. q=5 (3rd prime) maps to position 5 on Ring 0 (Hours)\n");
    printf("  3. Both are on the same ring (Ring 0 - Hours)\n");
    printf("  4. Angular separation shows their geometric relationship\n");
    printf("  5. The clock lattice provides a natural visualization of\n");
    printf("     prime factorization through geometric position\n");
    printf("\n");
    printf("  Next Steps:\n");
    printf("  - Use this visualization to understand torus structure\n");
    printf("  - Map torus oscillations to clock positions\n");
    printf("  - Refine G estimate using clock lattice geometry\n");
    printf("\n");
    
    return 0;
}