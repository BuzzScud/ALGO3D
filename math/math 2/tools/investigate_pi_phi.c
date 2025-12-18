/**
 * @file investigate_pi_phi.c
 * @brief Investigation of π × φ relationship in prime generation
 * 
 * This tool implements comprehensive tests to understand the relationship
 * between π (pi), φ (golden ratio), and prime number distribution.
 * 
 * User's Critical Insight:
 * "I found significant relationships with pi*golden_ratio as I believe 
 *  the golden ratio is the result of pi across hyper dimensional space."
 * 
 * Goal: Derive complete deterministic formula with NO trial division.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "math/math.h"

#define M_PI 3.14159265358979323846
#define M_PHI ((1.0 + sqrt(5.0)) / 2.0)
#define M_PI_PHI (M_PI * M_PHI)

/* ============================================================================
 * PRIME GENERATION (Ground Truth)
 * ============================================================================
 */

bool is_prime_trial(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (uint64_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

void generate_primes_sieve(uint64_t* primes, uint64_t count, uint64_t limit) {
    bool* is_prime = calloc(limit + 1, sizeof(bool));
    for (uint64_t i = 2; i <= limit; i++) {
        is_prime[i] = true;
    }
    
    for (uint64_t i = 2; i * i <= limit; i++) {
        if (is_prime[i]) {
            for (uint64_t j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }
    
    uint64_t idx = 0;
    for (uint64_t i = 2; i <= limit && idx < count; i++) {
        if (is_prime[i]) {
            primes[idx++] = i;
        }
    }
    
    free(is_prime);
}

int get_clock_position(uint64_t prime) {
    if (prime == 2 || prime == 3 || prime == 5) return 0;
    uint64_t mod = prime % 12;
    switch (mod) {
        case 1:
        case 5: return 3;
        case 7: return 6;
        case 11: return 9;
        default: return -1;
    }
}

/* ============================================================================
 * INVESTIGATION 1: π × φ IN PRIME GAPS
 * ============================================================================
 */

void investigate_pi_phi_gaps() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("INVESTIGATION 1: π × φ in Prime Gaps\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("π × φ = %.10f\n\n", M_PI_PHI);
    
    // Generate first 1000 primes
    uint64_t primes[1000];
    generate_primes_sieve(primes, 1000, 10000);
    
    printf("Testing gap relationships:\n");
    printf("%-6s %-10s %-8s %-12s %-12s %-12s\n", 
           "Index", "Prime", "Gap", "gap/π×φ", "gap mod π×φ", "log(gap)/log(π×φ)");
    printf("─────────────────────────────────────────────────────────────────────\n");
    
    double sum_ratio = 0.0;
    double sum_mod = 0.0;
    int count = 0;
    
    for (int i = 1; i < 100; i++) {  // First 100 gaps
        uint64_t gap = primes[i] - primes[i-1];
        double ratio = (double)gap / M_PI_PHI;
        double mod_val = fmod((double)gap, M_PI_PHI);
        double log_ratio = log((double)gap) / log(M_PI_PHI);
        
        printf("%-6d %-10lu %-8lu %-12.6f %-12.6f %-12.6f\n", 
               i, primes[i], gap, ratio, mod_val, log_ratio);
        
        sum_ratio += ratio;
        sum_mod += mod_val;
        count++;
    }
    
    printf("\nStatistics:\n");
    printf("  Average gap/π×φ: %.6f\n", sum_ratio / count);
    printf("  Average gap mod π×φ: %.6f\n", sum_mod / count);
    printf("\n");
}

/* ============================================================================
 * INVESTIGATION 2: PHASE ANGLE CORRECTIONS
 * ============================================================================
 */

void investigate_phase_angles() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("INVESTIGATION 2: Phase Angle Corrections\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    // Generate known primes
    uint64_t primes[3000];
    generate_primes_sieve(primes, 3000, 30000);
    
    // Test phase corrections for each position
    int positions[] = {3, 6, 9};
    const char* pos_names[] = {"Position 3 (mod 12 ≡ 5)", 
                                "Position 6 (mod 12 ≡ 7)", 
                                "Position 9 (mod 12 ≡ 11)"};
    
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        double theta = 2.0 * M_PI * position / 12.0;
        
        printf("%s (θ = %.4f rad = %.1f°):\n", pos_names[p], theta, theta * 180.0 / M_PI);
        printf("Base prime: %lu\n\n", base);
        
        // Test different phase correction formulas
        printf("Testing: candidate = base + magnitude × 12 × cos(θ + magnitude × δθ)\n");
        printf("%-4s %-12s %-12s %-12s %-12s\n", "Mag", "Simple", "δθ=π/100", "δθ=π/50", "δθ=π/25");
        printf("────────────────────────────────────────────────────────────\n");
        
        for (uint64_t mag = 0; mag < 20; mag++) {
            uint64_t simple = base + mag * 12;
            
            double phase1 = mag * M_PI / 100.0;
            uint64_t with_phase1 = base + (uint64_t)(mag * 12 * cos(theta + phase1));
            with_phase1 = (with_phase1 / 12) * 12 + (base % 12);
            
            double phase2 = mag * M_PI / 50.0;
            uint64_t with_phase2 = base + (uint64_t)(mag * 12 * cos(theta + phase2));
            with_phase2 = (with_phase2 / 12) * 12 + (base % 12);
            
            double phase3 = mag * M_PI / 25.0;
            uint64_t with_phase3 = base + (uint64_t)(mag * 12 * cos(theta + phase3));
            with_phase3 = (with_phase3 / 12) * 12 + (base % 12);
            
            printf("%-4lu %-12lu %-12lu %-12lu %-12lu\n", 
                   mag, simple, with_phase1, with_phase2, with_phase3);
        }
        printf("\n");
    }
}

/* ============================================================================
 * INVESTIGATION 3: GOLDEN RATIO GROWTH
 * ============================================================================
 */

void investigate_phi_growth() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("INVESTIGATION 3: Golden Ratio Growth\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("φ = %.10f\n\n", M_PHI);
    
    // Test φ growth in prime gaps
    uint64_t primes[1000];
    generate_primes_sieve(primes, 1000, 10000);
    
    printf("Testing gap growth vs φ^(n/k):\n");
    printf("%-6s %-10s %-8s %-12s %-12s %-12s\n", 
           "Index", "Prime", "Gap", "φ^(n/100)", "φ^(n/50)", "φ^(n/25)");
    printf("─────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 1; i < 50; i++) {
        uint64_t gap = primes[i] - primes[i-1];
        double phi_100 = pow(M_PHI, (double)i / 100.0);
        double phi_50 = pow(M_PHI, (double)i / 50.0);
        double phi_25 = pow(M_PHI, (double)i / 25.0);
        
        printf("%-6d %-10lu %-8lu %-12.6f %-12.6f %-12.6f\n", 
               i, primes[i], gap, phi_100, phi_50, phi_25);
    }
    printf("\n");
    
    // Test φ-modulated formula
    printf("Testing formula with φ growth:\n");
    printf("candidate = base + magnitude × 12 × φ^(magnitude/k)\n\n");
    
    int positions[] = {3, 6, 9};
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("Position %d (base = %lu):\n", position, base);
        printf("%-4s %-12s %-12s %-12s %-12s\n", "Mag", "Simple", "k=100", "k=50", "k=25");
        printf("────────────────────────────────────────────────────────────\n");
        
        for (uint64_t mag = 0; mag < 20; mag++) {
            uint64_t simple = base + mag * 12;
            uint64_t with_phi_100 = base + (uint64_t)(mag * 12 * pow(M_PHI, (double)mag / 100.0));
            uint64_t with_phi_50 = base + (uint64_t)(mag * 12 * pow(M_PHI, (double)mag / 50.0));
            uint64_t with_phi_25 = base + (uint64_t)(mag * 12 * pow(M_PHI, (double)mag / 25.0));
            
            // Round to correct modular class
            with_phi_100 = (with_phi_100 / 12) * 12 + (base % 12);
            with_phi_50 = (with_phi_50 / 12) * 12 + (base % 12);
            with_phi_25 = (with_phi_25 / 12) * 12 + (base % 12);
            
            printf("%-4lu %-12lu %-12lu %-12lu %-12lu\n", 
                   mag, simple, with_phi_100, with_phi_50, with_phi_25);
        }
        printf("\n");
    }
}

/* ============================================================================
 * INVESTIGATION 4: HYPERDIMENSIONAL PROJECTION
 * ============================================================================
 */

void investigate_hyperdimensional() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("INVESTIGATION 4: Hyperdimensional Projection (π × φ)\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("π × φ = %.10f\n", M_PI_PHI);
    printf("Close to prime 5 (3rd prime at 3 o'clock)\n\n");
    
    // Test dimensional projection
    printf("Testing: candidate = base + magnitude × 12 × (π×φ)^(1/dimension)\n\n");
    
    int positions[] = {3, 6, 9};
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("Position %d (base = %lu):\n", position, base);
        printf("%-4s %-12s %-12s %-12s %-12s\n", "Mag", "Simple", "dim=3", "dim=4", "dim=5");
        printf("────────────────────────────────────────────────────────────\n");
        
        for (uint64_t mag = 0; mag < 20; mag++) {
            uint64_t simple = base + mag * 12;
            
            double dim3 = pow(M_PI_PHI, 1.0 / 3.0);
            uint64_t with_dim3 = base + (uint64_t)(mag * 12 * dim3);
            with_dim3 = (with_dim3 / 12) * 12 + (base % 12);
            
            double dim4 = pow(M_PI_PHI, 1.0 / 4.0);
            uint64_t with_dim4 = base + (uint64_t)(mag * 12 * dim4);
            with_dim4 = (with_dim4 / 12) * 12 + (base % 12);
            
            double dim5 = pow(M_PI_PHI, 1.0 / 5.0);
            uint64_t with_dim5 = base + (uint64_t)(mag * 12 * dim5);
            with_dim5 = (with_dim5 / 12) * 12 + (base % 12);
            
            printf("%-4lu %-12lu %-12lu %-12lu %-12lu\n", 
                   mag, simple, with_dim3, with_dim4, with_dim5);
        }
        printf("\n");
    }
}

/* ============================================================================
 * INVESTIGATION 5: INTERFERENCE PATTERNS
 * ============================================================================
 */

void investigate_interference() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("INVESTIGATION 5: Cross-Position Interference\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    // Analyze composites in each sequence
    int positions[] = {3, 6, 9};
    const char* pos_names[] = {"Position 3", "Position 6", "Position 9"};
    
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("%s (base = %lu):\n", pos_names[p], base);
        printf("Analyzing composites in sequence...\n\n");
        
        printf("%-4s %-12s %-8s %-30s\n", "Mag", "Candidate", "Status", "Factorization");
        printf("────────────────────────────────────────────────────────────────────\n");
        
        for (uint64_t mag = 0; mag < 30; mag++) {
            uint64_t candidate = base + mag * 12;
            
            if (is_prime_trial(candidate)) {
                printf("%-4lu %-12lu %-8s\n", mag, candidate, "PRIME");
            } else {
                // Find factors
                printf("%-4lu %-12lu %-8s ", mag, candidate, "COMPOSITE");
                
                for (uint64_t f = 2; f * f <= candidate; f++) {
                    if (candidate % f == 0) {
                        uint64_t f2 = candidate / f;
                        int pos1 = get_clock_position(f);
                        int pos2 = get_clock_position(f2);
                        printf("%lu (pos %d) × %lu (pos %d)\n", f, pos1, f2, pos2);
                        break;
                    }
                }
            }
        }
        printf("\n");
    }
}

/* ============================================================================
 * MAIN
 * ============================================================================
 */

int main(int argc, char** argv) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  π × φ INVESTIGATION: Hyperdimensional Prime Generation      ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    printf("Constants:\n");
    printf("  π = %.15f\n", M_PI);
    printf("  φ = %.15f\n", M_PHI);
    printf("  π × φ = %.15f\n", M_PI_PHI);
    printf("\n");
    
    // Run all investigations
    investigate_pi_phi_gaps();
    investigate_phase_angles();
    investigate_phi_growth();
    investigate_hyperdimensional();
    investigate_interference();
    
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("INVESTIGATION COMPLETE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\nAnalyze results to derive complete deterministic formula.\n\n");
    
    return 0;
}