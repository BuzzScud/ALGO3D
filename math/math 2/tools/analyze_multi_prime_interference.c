/**
 * Multi-Prime Interference Pattern Analysis
 * 
 * This tool investigates the infinitely recursing self-similar structure
 * of prime interference patterns at different phase angles with quadratic
 * relationships and polarity flips.
 * 
 * Key Concepts:
 * 1. Each prime creates interference at specific phase angles
 * 2. Interference patterns are quadratic (p² relationships)
 * 3. Polarity flips occur at p² ≡ 1 (mod 12)
 * 4. Patterns are self-similar at different scales
 * 5. Emergent patterns from multi-prime interaction
 */

#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <stdbool.h>
#include <string.h>

#define MAX_PRIMES 1000
#define MAX_MAGNITUDE 200
#define PHI 1.61803398874989484820

// Clock positions
typedef struct {
    int position;      // 0, 3, 6, 9
    int base_prime;    // 2, 5, 7, 11
} ClockPosition;

ClockPosition positions[] = {
    {0, 2},   // 12 o'clock
    {3, 5},   // 3 o'clock  
    {6, 7},   // 6 o'clock
    {9, 11}   // 9 o'clock
};

// Store all primes up to a limit
int primes[MAX_PRIMES];
int prime_count = 0;

// Generate primes using simple sieve
void generate_primes(int limit) {
    bool *is_prime = calloc(limit + 1, sizeof(bool));
    for (int i = 2; i <= limit; i++) is_prime[i] = true;
    
    for (int i = 2; i * i <= limit; i++) {
        if (is_prime[i]) {
            for (int j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }
    
    prime_count = 0;
    for (int i = 2; i <= limit && prime_count < MAX_PRIMES; i++) {
        if (is_prime[i]) {
            primes[prime_count++] = i;
        }
    }
    
    free(is_prime);
}

// Get clock position for a prime
int get_clock_position(int prime) {
    if (prime == 2) return 0;
    if (prime == 3) return 0;
    int mod12 = prime % 12;
    if (mod12 == 5) return 3;
    if (mod12 == 7) return 6;
    if (mod12 == 11) return 9;
    return -1;
}

// Calculate phase angle for a prime at its position
double get_phase_angle(int prime) {
    int pos = get_clock_position(prime);
    if (pos < 0) return 0.0;
    return (pos * MATH_PI / 6.0);  // 0, π/2, π, 3π/2
}

// Check if number is prime
bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 0; i < prime_count && primes[i] * primes[i] <= n; i++) {
        if (n % primes[i] == 0) return false;
    }
    return true;
}

// Analyze interference pattern for a specific position
typedef struct {
    int magnitude;
    int candidate;
    bool is_prime;
    int interfering_primes[10];  // Which primes create interference
    int interference_count;
    double phase_angles[10];     // Phase angles of interfering primes
    int quadratic_residues[10];  // p² mod 12 for each interfering prime
} InterferenceData;

void analyze_position_interference(int position, int max_magnitude, FILE *out) {
    ClockPosition *pos = NULL;
    for (int i = 0; i < 4; i++) {
        if (positions[i].position == position) {
            pos = &positions[i];
            break;
        }
    }
    if (!pos) return;
    
    fprintf(out, "\n=== POSITION %d (Base Prime %d) ===\n", position, pos->base_prime);
    fprintf(out, "Phase Angle: %.4f radians (%.1f degrees)\n\n", 
            get_phase_angle(pos->base_prime), 
            get_phase_angle(pos->base_prime) * 180.0 / MATH_PI);
    
    InterferenceData data[MAX_MAGNITUDE];
    int composite_count = 0;
    
    // Collect data for each magnitude
    for (int mag = 0; mag < max_magnitude; mag++) {
        int candidate = pos->base_prime + mag * 12;
        data[mag].magnitude = mag;
        data[mag].candidate = candidate;
        data[mag].is_prime = is_prime(candidate);
        data[mag].interference_count = 0;
        
        if (!data[mag].is_prime && candidate > 1) {
            composite_count++;
            
            // Find which primes divide this composite
            for (int i = 0; i < prime_count && primes[i] <= candidate; i++) {
                if (candidate % primes[i] == 0 && primes[i] != candidate) {
                    int idx = data[mag].interference_count;
                    if (idx < 10) {
                        data[mag].interfering_primes[idx] = primes[i];
                        data[mag].phase_angles[idx] = get_phase_angle(primes[i]);
                        data[mag].quadratic_residues[idx] = (primes[i] * primes[i]) % 12;
                        data[mag].interference_count++;
                    }
                }
            }
        }
    }
    
    fprintf(out, "Total Composites: %d / %d (%.1f%%)\n\n", 
            composite_count, max_magnitude, 
            100.0 * composite_count / max_magnitude);
    
    // Analyze interference patterns
    fprintf(out, "=== INTERFERENCE PATTERN ANALYSIS ===\n\n");
    
    // 1. Prime-by-Prime Interference Count
    fprintf(out, "1. INTERFERENCE BY PRIME:\n");
    int prime_interference_count[MAX_PRIMES] = {0};
    for (int mag = 0; mag < max_magnitude; mag++) {
        if (!data[mag].is_prime) {
            for (int i = 0; i < data[mag].interference_count; i++) {
                int p = data[mag].interfering_primes[i];
                for (int j = 0; j < prime_count; j++) {
                    if (primes[j] == p) {
                        prime_interference_count[j]++;
                        break;
                    }
                }
            }
        }
    }
    
    fprintf(out, "Prime | Count | Percentage | Phase Angle | p² mod 12\n");
    fprintf(out, "------|-------|------------|-------------|----------\n");
    for (int i = 0; i < 20 && i < prime_count; i++) {
        if (prime_interference_count[i] > 0) {
            fprintf(out, "%5d | %5d | %9.1f%% | %11.4f | %9d\n",
                    primes[i], prime_interference_count[i],
                    100.0 * prime_interference_count[i] / composite_count,
                    get_phase_angle(primes[i]),
                    (primes[i] * primes[i]) % 12);
        }
    }
    
    // 2. Phase Angle Correlation
    fprintf(out, "\n2. PHASE ANGLE PATTERNS:\n");
    double phase_diff_histogram[13] = {0};  // 0 to 2π in 12 bins
    int phase_pair_count = 0;
    
    for (int mag = 0; mag < max_magnitude; mag++) {
        if (!data[mag].is_prime && data[mag].interference_count >= 2) {
            for (int i = 0; i < data[mag].interference_count; i++) {
                for (int j = i + 1; j < data[mag].interference_count; j++) {
                    double diff = fabs(data[mag].phase_angles[i] - data[mag].phase_angles[j]);
                    if (diff > MATH_PI) diff = 2 * MATH_PI - diff;
                    int bin = (int)(diff * 6.0 / MATH_PI);  // 12 bins for 0 to 2π
                    if (bin >= 0 && bin < 13) {
                        phase_diff_histogram[bin]++;
                        phase_pair_count++;
                    }
                }
            }
        }
    }
    
    fprintf(out, "Phase Difference | Count | Percentage\n");
    fprintf(out, "-----------------|-------|------------\n");
    for (int i = 0; i < 13; i++) {
        double angle = i * MATH_PI / 6.0;
        fprintf(out, "%7.4f (%.0f°) | %5.0f | %9.1f%%\n",
                angle, angle * 180.0 / MATH_PI,
                phase_diff_histogram[i],
                phase_pair_count > 0 ? 100.0 * phase_diff_histogram[i] / phase_pair_count : 0.0);
    }
    
    // 3. Quadratic Residue Patterns
    fprintf(out, "\n3. QUADRATIC RESIDUE PATTERNS (p² mod 12):\n");
    int residue_count[12] = {0};
    int total_residues = 0;
    
    for (int mag = 0; mag < max_magnitude; mag++) {
        if (!data[mag].is_prime) {
            for (int i = 0; i < data[mag].interference_count; i++) {
                int res = data[mag].quadratic_residues[i];
                residue_count[res]++;
                total_residues++;
            }
        }
    }
    
    fprintf(out, "Residue | Count | Percentage\n");
    fprintf(out, "--------|-------|------------\n");
    for (int i = 0; i < 12; i++) {
        if (residue_count[i] > 0) {
            fprintf(out, "%7d | %5d | %9.1f%%\n",
                    i, residue_count[i],
                    100.0 * residue_count[i] / total_residues);
        }
    }
    
    // 4. Magnitude Modulo Patterns (for each interfering prime)
    fprintf(out, "\n4. MAGNITUDE MODULO PATTERNS:\n");
    for (int p_idx = 0; p_idx < 10 && p_idx < prime_count; p_idx++) {
        int p = primes[p_idx];
        if (prime_interference_count[p_idx] == 0) continue;
        
        int mod_histogram[20] = {0};  // Up to mod 20
        int mod_count = 0;
        
        for (int mag = 0; mag < max_magnitude; mag++) {
            if (!data[mag].is_prime) {
                for (int i = 0; i < data[mag].interference_count; i++) {
                    if (data[mag].interfering_primes[i] == p) {
                        int mod_val = mag % p;
                        if (mod_val < 20) {
                            mod_histogram[mod_val]++;
                            mod_count++;
                        }
                        break;
                    }
                }
            }
        }
        
        fprintf(out, "\nPrime %d (phase %.4f, p²≡%d mod 12):\n", 
                p, get_phase_angle(p), (p * p) % 12);
        fprintf(out, "mag mod %d | Count | Percentage\n", p);
        fprintf(out, "-----------|-------|------------\n");
        
        for (int i = 0; i < p && i < 20; i++) {
            if (mod_histogram[i] > 0) {
                fprintf(out, "%10d | %5d | %9.1f%%\n",
                        i, mod_histogram[i],
                        100.0 * mod_histogram[i] / mod_count);
            }
        }
    }
    
    // 5. Self-Similar Recursion Analysis
    fprintf(out, "\n5. SELF-SIMILAR RECURSION PATTERNS:\n");
    fprintf(out, "Looking for patterns that repeat at different scales...\n\n");
    
    // Check if composite pattern repeats at intervals
    for (int scale = 2; scale <= 20; scale++) {
        int matches = 0;
        int total = 0;
        
        for (int mag = 0; mag < max_magnitude - scale; mag++) {
            if (!data[mag].is_prime && !data[mag + scale].is_prime) {
                // Check if interference patterns are similar
                bool similar = true;
                if (data[mag].interference_count != data[mag + scale].interference_count) {
                    similar = false;
                } else {
                    for (int i = 0; i < data[mag].interference_count; i++) {
                        bool found = false;
                        for (int j = 0; j < data[mag + scale].interference_count; j++) {
                            if (data[mag].interfering_primes[i] == data[mag + scale].interfering_primes[j]) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            similar = false;
                            break;
                        }
                    }
                }
                
                if (similar) matches++;
                total++;
            }
        }
        
        if (total > 10 && matches * 100 / total > 30) {
            fprintf(out, "Scale %d: %d/%d matches (%.1f%%) - SIGNIFICANT RECURSION!\n",
                    scale, matches, total, 100.0 * matches / total);
        }
    }
}

// Analyze cross-position interference
void analyze_cross_position_interference(int max_magnitude, FILE *out) {
    fprintf(out, "\n\n=== CROSS-POSITION INTERFERENCE ANALYSIS ===\n\n");
    
    // For each position, analyze which OTHER positions create interference
    for (int pos_idx = 0; pos_idx < 4; pos_idx++) {
        ClockPosition *pos = &positions[pos_idx];
        
        fprintf(out, "Position %d (Base %d):\n", pos->position, pos->base_prime);
        
        int cross_interference[4] = {0};  // Count from each position
        int total_composites = 0;
        
        for (int mag = 0; mag < max_magnitude; mag++) {
            int candidate = pos->base_prime + mag * 12;
            if (!is_prime(candidate) && candidate > 1) {
                total_composites++;
                
                // Check which positions the factors come from
                for (int i = 0; i < prime_count && primes[i] <= candidate; i++) {
                    if (candidate % primes[i] == 0 && primes[i] != candidate) {
                        int factor_pos = get_clock_position(primes[i]);
                        if (factor_pos >= 0) {
                            for (int j = 0; j < 4; j++) {
                                if (positions[j].position == factor_pos) {
                                    cross_interference[j]++;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        fprintf(out, "  Interference from Position 0: %d (%.1f%%)\n", 
                cross_interference[0], 100.0 * cross_interference[0] / total_composites);
        fprintf(out, "  Interference from Position 3: %d (%.1f%%)\n", 
                cross_interference[1], 100.0 * cross_interference[1] / total_composites);
        fprintf(out, "  Interference from Position 6: %d (%.1f%%)\n", 
                cross_interference[2], 100.0 * cross_interference[2] / total_composites);
        fprintf(out, "  Interference from Position 9: %d (%.1f%%)\n\n", 
                cross_interference[3], 100.0 * cross_interference[3] / total_composites);
    }
}

// Analyze emergent patterns from multi-prime interference
void analyze_emergent_patterns(int max_magnitude, FILE *out) {
    fprintf(out, "\n\n=== EMERGENT MULTI-PRIME PATTERNS ===\n\n");
    
    // Look for patterns involving 2, 3, 5 together (the first 3 primes)
    fprintf(out, "1. FUNDAMENTAL TRIAD (2, 3, 5) INTERFERENCE:\n\n");
    
    for (int pos_idx = 1; pos_idx < 4; pos_idx++) {  // Skip position 0
        ClockPosition *pos = &positions[pos_idx];
        
        int triad_interference = 0;
        int pair_interference = 0;
        int single_interference = 0;
        int total_composites = 0;
        
        for (int mag = 0; mag < max_magnitude; mag++) {
            int candidate = pos->base_prime + mag * 12;
            if (!is_prime(candidate) && candidate > 1) {
                total_composites++;
                
                bool has_2 = (candidate % 2 == 0);
                bool has_3 = (candidate % 3 == 0);
                bool has_5 = (candidate % 5 == 0);
                
                int count = has_2 + has_3 + has_5;
                if (count == 3) triad_interference++;
                else if (count == 2) pair_interference++;
                else if (count == 1) single_interference++;
            }
        }
        
        fprintf(out, "Position %d:\n", pos->position);
        fprintf(out, "  All three (2,3,5): %d (%.1f%%)\n", 
                triad_interference, 100.0 * triad_interference / total_composites);
        fprintf(out, "  Two of three:      %d (%.1f%%)\n", 
                pair_interference, 100.0 * pair_interference / total_composites);
        fprintf(out, "  One of three:      %d (%.1f%%)\n", 
                single_interference, 100.0 * single_interference / total_composites);
        fprintf(out, "  None (other):      %d (%.1f%%)\n\n", 
                total_composites - triad_interference - pair_interference - single_interference,
                100.0 * (total_composites - triad_interference - pair_interference - single_interference) / total_composites);
    }
    
    // Look for π × φ relationships in interference spacing
    fprintf(out, "2. π × φ IN INTERFERENCE SPACING:\n\n");
    
    for (int pos_idx = 1; pos_idx < 4; pos_idx++) {
        ClockPosition *pos = &positions[pos_idx];
        
        int prev_composite_mag = -1;
        double spacing_sum = 0.0;
        int spacing_count = 0;
        
        for (int mag = 0; mag < max_magnitude; mag++) {
            int candidate = pos->base_prime + mag * 12;
            if (!is_prime(candidate) && candidate > 1) {
                if (prev_composite_mag >= 0) {
                    int spacing = mag - prev_composite_mag;
                    spacing_sum += spacing;
                    spacing_count++;
                }
                prev_composite_mag = mag;
            }
        }
        
        double avg_spacing = spacing_count > 0 ? spacing_sum / spacing_count : 0.0;
        double pi_phi = MATH_PI * PHI;
        
        fprintf(out, "Position %d: Average composite spacing = %.4f\n", 
                pos->position, avg_spacing);
        fprintf(out, "  Ratio to π×φ (%.4f): %.4f\n", pi_phi, avg_spacing / pi_phi);
        fprintf(out, "  Ratio to φ (%.4f): %.4f\n", PHI, avg_spacing / PHI);
        fprintf(out, "  Ratio to π (%.4f): %.4f\n\n", MATH_PI, avg_spacing / MATH_PI);
    }
}

int main() {
    printf("Multi-Prime Interference Pattern Analysis\n");
    printf("==========================================\n\n");
    
    // Generate primes
    generate_primes(10000);
    printf("Generated %d primes\n\n", prime_count);
    
    // Open output file
    FILE *out = fopen("MULTI_PRIME_INTERFERENCE_RESULTS.txt", "w");
    if (!out) {
        fprintf(stderr, "Error opening output file\n");
        return 1;
    }
    
    fprintf(out, "MULTI-PRIME INTERFERENCE PATTERN ANALYSIS\n");
    fprintf(out, "==========================================\n");
    fprintf(out, "Investigating infinitely recursing self-similar structure\n");
    fprintf(out, "with quadratic relationships and polarity flips\n\n");
    
    // Analyze each position
    int max_mag = 150;
    for (int i = 0; i < 4; i++) {
        analyze_position_interference(positions[i].position, max_mag, out);
    }
    
    // Cross-position analysis
    analyze_cross_position_interference(max_mag, out);
    
    // Emergent patterns
    analyze_emergent_patterns(max_mag, out);
    
    fclose(out);
    
    printf("Analysis complete! Results written to MULTI_PRIME_INTERFERENCE_RESULTS.txt\n");
    printf("\nKey investigations:\n");
    printf("1. Prime-by-prime interference counts and phase angles\n");
    printf("2. Phase angle correlation patterns\n");
    printf("3. Quadratic residue patterns (p² mod 12)\n");
    printf("4. Magnitude modulo patterns for each prime\n");
    printf("5. Self-similar recursion at different scales\n");
    printf("6. Cross-position interference\n");
    printf("7. Emergent multi-prime patterns\n");
    printf("8. π × φ relationships in spacing\n");
    
    return 0;
}