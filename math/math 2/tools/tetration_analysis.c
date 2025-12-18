/**
 * @file tetration_analysis.c
 * @brief Tetration-based analysis of Platonic prime relationships
 * 
 * Implements tetration (repeated exponentiation) to depth 29 to map
 * all relationships in the Platonic prime system.
 * 
 * Tetration: ^^n a = a^(a^(a^(...))) (n times)
 * 
 * Depth 13: Visualization depth
 * Depth 29: Complete relationship mapping (Tetrahedron prime)
 */

#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <stdbool.h>
#include <stdint.h>
#include "crystal_abacus.h"

// Maximum safe tetration value (to avoid overflow)
#define MAX_TETRATION_VALUE 1000000000000ULL

/**
 * Calculate tetration: ^^n a
 * 
 * Returns 0 if result would overflow MAX_TETRATION_VALUE
 */
uint64_t tetration(int base, int depth) {
    if (depth == 0) return 1;
    if (depth == 1) return base;
    
    uint64_t result = base;
    for (int i = 1; i < depth; i++) {
        // Check for overflow
        if (result > 63) {  // 2^64 limit
            return 0;  // Would overflow
        }
        
        uint64_t new_result = 1;
        for (uint64_t j = 0; j < result; j++) {
            if (new_result > MAX_TETRATION_VALUE / base) {
                return 0;  // Would overflow
            }
            new_result *= base;
        }
        result = new_result;
        
        if (result > MAX_TETRATION_VALUE) {
            return 0;  // Exceeded safe limit
        }
    }
    
    return result;
}

/**
 * Calculate bounded tetration for visualization
 * 
 * Uses modular arithmetic to keep values bounded
 */
uint64_t tetration_mod(int base, int depth, uint64_t modulus) {
    if (depth == 0) return 1;
    if (depth == 1) return base % modulus;
    
    uint64_t result = base % modulus;
    for (int i = 1; i < depth; i++) {
        uint64_t new_result = 1;
        uint64_t exp = result;
        uint64_t base_mod = base % modulus;
        
        // Modular exponentiation
        while (exp > 0) {
            if (exp % 2 == 1) {
                new_result = (new_result * base_mod) % modulus;
            }
            base_mod = (base_mod * base_mod) % modulus;
            exp /= 2;
        }
        
        result = new_result;
    }
    
    return result;
}

/**
 * Analyze tetration patterns for Platonic primes
 */
void analyze_tetration_patterns(int max_depth) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  TETRATION ANALYSIS OF PLATONIC PRIMES                     ║\n");
    printf("║  Depth: 1 to %d                                           ║\n", max_depth);
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Platonic base primes
    int base_primes[] = {2, 3, 5};
    const char* names[] = {"Cube", "Tetrahedron", "Dodecahedron"};
    (void)names;  // Suppress unused variable warning
    
    printf("=== TETRATION VALUES (Direct Calculation) ===\n\n");
    printf("Base  Depth  Value (^^n a)  Nearest Prime  Distance\n");
    printf("----  -----  -------------  -------------  --------\n");
    
    for (int b = 0; b < 3; b++) {
        int base = base_primes[b];
        
        for (int depth = 1; depth <= max_depth && depth <= 5; depth++) {
            uint64_t value = tetration(base, depth);
            
            if (value == 0) {
                printf("%4d  %5d  OVERFLOW\n", base, depth);
                break;
            }
            
            // Find nearest prime
            uint64_t nearest_prime = 0;
            uint64_t min_dist = UINT64_MAX;
            
            for (int64_t offset = -50; offset <= 50; offset++) {
                int64_t candidate = (int64_t)value + offset;
                if (candidate < 2) continue;
                
                if (is_prime((uint64_t)candidate)) {
                    uint64_t dist = (uint64_t)llabs(offset);
                    if (dist < min_dist) {
                        min_dist = dist;
                        nearest_prime = (uint64_t)candidate;
                    }
                }
            }
            
            printf("%4d  %5d  %13lu  %13lu  %8lu\n",
                   base, depth, value, nearest_prime, min_dist);
        }
        printf("\n");
    }
    
    // Modular tetration for higher depths
    printf("\n=== TETRATION MODULO 1000000 (For Higher Depths) ===\n\n");
    printf("Base  Depth  Value (mod 1M)  Nearest Prime  Distance\n");
    printf("----  -----  --------------  -------------  --------\n");
    
    uint64_t modulus = 1000000;
    
    for (int b = 0; b < 3; b++) {
        int base = base_primes[b];
        
        for (int depth = 1; depth <= max_depth; depth++) {
            uint64_t value = tetration_mod(base, depth, modulus);
            
            // Find nearest prime
            uint64_t nearest_prime = 0;
            uint64_t min_dist = UINT64_MAX;
            
            for (int64_t offset = -50; offset <= 50; offset++) {
                int64_t candidate = (int64_t)value + offset;
                if (candidate < 2 || candidate > (int64_t)modulus) continue;
                
                if (is_prime((uint64_t)candidate)) {
                    uint64_t dist = (uint64_t)llabs(offset);
                    if (dist < min_dist) {
                        min_dist = dist;
                        nearest_prime = (uint64_t)candidate;
                    }
                }
            }
            
            printf("%4d  %5d  %14lu  %13lu  %8lu\n",
                   base, depth, value, nearest_prime, min_dist);
        }
        printf("\n");
    }
}

/**
 * Map relationships through tetration depth 29
 */
void map_tetration_relationships_depth_29() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  TETRATION DEPTH 29 RELATIONSHIP MAPPING                   ║\n");
    printf("║  Mapping all Platonic relationships through 29 levels     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Use modular arithmetic to keep values bounded
    uint64_t modulus = 1000000;
    
    int base_primes[] = {2, 3, 5};
    const char* names[] = {"Cube (2)", "Tetrahedron (3)", "Dodecahedron (5)"};
    
    printf("Mapping through 29 levels of tetration (mod %lu):\n\n", modulus);
    
    for (int b = 0; b < 3; b++) {
        int base = base_primes[b];
        printf("=== %s ===\n\n", names[b]);
        
        printf("Depth  Value (mod 1M)  Nearest Prime  mod12  Resonance\n");
        printf("-----  --------------  -------------  -----  ---------\n");
        
        for (int depth = 1; depth <= 29; depth++) {
            uint64_t value = tetration_mod(base, depth, modulus);
            
            // Find nearest prime
            uint64_t nearest_prime = 0;
            uint64_t min_dist = UINT64_MAX;
            
            for (int64_t offset = -50; offset <= 50; offset++) {
                int64_t candidate = (int64_t)value + offset;
                if (candidate < 2 || candidate > (int64_t)modulus) continue;
                
                if (is_prime((uint64_t)candidate)) {
                    uint64_t dist = (uint64_t)llabs(offset);
                    if (dist < min_dist) {
                        min_dist = dist;
                        nearest_prime = (uint64_t)candidate;
                    }
                }
            }
            
            if (nearest_prime > 0) {
                // Calculate geometric resonance
                double resonance = 0.0;
                // Simplified resonance for display
                int platonic_targets[] = {4, 27, 125, 243};
                for (int i = 0; i < 4; i++) {
                    double dist = (double)nearest_prime - (double)platonic_targets[i];
                    resonance += exp(-(dist * dist) / 100.0);
                }
                
                printf("%5d  %14lu  %13lu  %5lu  %9.6f\n",
                       depth, value, nearest_prime, nearest_prime % 12, resonance);
            } else {
                printf("%5d  %14lu  NO PRIME FOUND\n", depth, value);
            }
        }
        printf("\n");
    }
}

/**
 * Analyze depth-13 visualization structure
 */
void analyze_depth_13_structure() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  DEPTH 13 VISUALIZATION STRUCTURE                          ║\n");
    printf("║  13-level hierarchical mapping of Platonic relationships   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Depth 13 represents a complete visualization hierarchy:\n\n");
    
    // Calculate tetration values for depth 13
    int base_primes[] = {2, 3, 5};
    const char* names[] = {"Cube", "Tetrahedron", "Dodecahedron"};
    
    printf("Base  Name           ^^13 (mod 1M)  Nearest Prime  Significance\n");
    printf("----  -------------  -------------  -------------  ------------\n");
    
    for (int b = 0; b < 3; b++) {
        int base = base_primes[b];
        uint64_t value = tetration_mod(base, 13, 1000000);
        
        // Find nearest prime
        uint64_t nearest_prime = 0;
        for (int64_t offset = -50; offset <= 50; offset++) {
            int64_t candidate = (int64_t)value + offset;
            if (candidate >= 2 && candidate <= 1000000 && is_prime((uint64_t)candidate)) {
                nearest_prime = (uint64_t)candidate;
                break;
            }
        }
        
        printf("%4d  %-13s  %13lu  %13lu  13-level hierarchy\n",
               base, names[b], value, nearest_prime);
    }
    
    printf("\n13 is significant because:\n");
    printf("  - 13th prime = 41\n");
    printf("  - 13 = 12 + 1 (one beyond the clock)\n");
    printf("  - 13 levels = complete visualization depth\n");
    printf("  - Fibonacci: 13 is the 7th Fibonacci number\n");
}

int main() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  TETRATION ANALYSIS SUITE                                  ║\n");
    printf("║  Deep Analysis of Platonic Prime Relationships            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Analyze tetration patterns
    analyze_tetration_patterns(29);
    
    // Map relationships through depth 29
    map_tetration_relationships_depth_29();
    
    // Analyze depth 13 structure
    analyze_depth_13_structure();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ANALYSIS COMPLETE                                         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}