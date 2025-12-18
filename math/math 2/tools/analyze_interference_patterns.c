/**
 * @file analyze_interference_patterns.c
 * @brief Analyze interference patterns in composite magnitudes
 * 
 * Goal: Determine if π × φ predicts which magnitudes produce composites
 * 
 * This tool analyzes the composite magnitude sequences to find:
 * 1. Periodicity in composite positions
 * 2. Relationship to π × φ
 * 3. Patterns that allow deterministic prediction
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
 * PRIME GENERATION
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

/* ============================================================================
 * ANALYSIS 1: COMPOSITE MAGNITUDE SEQUENCES
 * ============================================================================
 */

void analyze_composite_magnitudes() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("ANALYSIS 1: Composite Magnitude Sequences\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    int positions[] = {3, 6, 9};
    const char* pos_names[] = {"Position 3 (17+12n)", "Position 6 (7+12n)", "Position 9 (11+12n)"};
    
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("%s:\n", pos_names[p]);
        printf("Base = %lu\n\n", base);
        
        // Collect composite magnitudes
        uint64_t composite_mags[200];
        int comp_count = 0;
        
        for (uint64_t mag = 0; mag < 200 && comp_count < 200; mag++) {
            uint64_t candidate = base + mag * 12;
            if (!is_prime_trial(candidate)) {
                composite_mags[comp_count++] = mag;
            }
        }
        
        printf("First 50 composite magnitudes:\n");
        for (int i = 0; i < 50 && i < comp_count; i++) {
            printf("%lu ", composite_mags[i]);
            if ((i + 1) % 10 == 0) printf("\n");
        }
        printf("\n\n");
        
        // Analyze differences
        printf("Differences between consecutive composite magnitudes:\n");
        for (int i = 1; i < 50 && i < comp_count; i++) {
            uint64_t diff = composite_mags[i] - composite_mags[i-1];
            printf("%lu ", diff);
            if (i % 10 == 0) printf("\n");
        }
        printf("\n\n");
        
        // Statistics
        double sum_diff = 0.0;
        int diff_count = 0;
        for (int i = 1; i < comp_count; i++) {
            sum_diff += (composite_mags[i] - composite_mags[i-1]);
            diff_count++;
        }
        double avg_diff = sum_diff / diff_count;
        
        printf("Statistics:\n");
        printf("  Total composites in first 200: %d\n", comp_count);
        printf("  Average difference: %.2f\n", avg_diff);
        printf("  π × φ = %.4f\n", M_PI_PHI);
        printf("  avg_diff / π×φ = %.4f\n", avg_diff / M_PI_PHI);
        printf("\n");
    }
}

/* ============================================================================
 * ANALYSIS 2: MAGNITUDE MOD π×φ
 * ============================================================================
 */

void analyze_magnitude_mod_pi_phi() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("ANALYSIS 2: Magnitude mod π×φ\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("Testing if composite magnitudes cluster at specific π×φ intervals\n\n");
    
    int positions[] = {3, 6, 9};
    const char* pos_names[] = {"Position 3", "Position 6", "Position 9"};
    
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("%s (base = %lu):\n", pos_names[p], base);
        
        // Collect composite and prime magnitudes with their mod values
        printf("%-6s %-12s %-8s %-15s\n", "Mag", "Candidate", "Status", "mag mod π×φ");
        printf("────────────────────────────────────────────────────────────\n");
        
        for (uint64_t mag = 0; mag < 50; mag++) {
            uint64_t candidate = base + mag * 12;
            bool is_prime = is_prime_trial(candidate);
            double mod_val = fmod((double)mag, M_PI_PHI);
            
            printf("%-6lu %-12lu %-8s %-15.6f\n", 
                   mag, candidate, is_prime ? "PRIME" : "COMPOSITE", mod_val);
        }
        printf("\n");
    }
}

/* ============================================================================
 * ANALYSIS 3: PERIODICITY IN INTERFERENCE
 * ============================================================================
 */

void analyze_interference_periodicity() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("ANALYSIS 3: Periodicity in Interference Pattern\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("Looking for periodic patterns in composite occurrences\n\n");
    
    int positions[] = {3, 6, 9};
    const char* pos_names[] = {"Position 3", "Position 6", "Position 9"};
    
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("%s (base = %lu):\n", pos_names[p], base);
        
        // Create histogram of composite positions mod various values
        int hist_5[5] = {0};
        int hist_6[6] = {0};
        int hist_12[12] = {0};
        int hist_pi_phi[6] = {0};  // Divide π×φ ≈ 5.08 into 6 bins
        
        int total_composites = 0;
        
        for (uint64_t mag = 0; mag < 200; mag++) {
            uint64_t candidate = base + mag * 12;
            if (!is_prime_trial(candidate)) {
                hist_5[mag % 5]++;
                hist_6[mag % 6]++;
                hist_12[mag % 12]++;
                
                double mod_pi_phi = fmod((double)mag, M_PI_PHI);
                int bin = (int)(mod_pi_phi / M_PI_PHI * 6.0);
                if (bin >= 0 && bin < 6) hist_pi_phi[bin]++;
                
                total_composites++;
            }
        }
        
        printf("\nComposite distribution (first 200 magnitudes):\n");
        printf("Total composites: %d\n\n", total_composites);
        
        printf("Magnitude mod 5:\n");
        for (int i = 0; i < 5; i++) {
            printf("  mod %d: %d (%.1f%%)\n", i, hist_5[i], 100.0 * hist_5[i] / total_composites);
        }
        
        printf("\nMagnitude mod 6:\n");
        for (int i = 0; i < 6; i++) {
            printf("  mod %d: %d (%.1f%%)\n", i, hist_6[i], 100.0 * hist_6[i] / total_composites);
        }
        
        printf("\nMagnitude mod 12:\n");
        for (int i = 0; i < 12; i++) {
            printf("  mod %d: %d (%.1f%%)\n", i, hist_12[i], 100.0 * hist_12[i] / total_composites);
        }
        
        printf("\nMagnitude mod π×φ (binned):\n");
        for (int i = 0; i < 6; i++) {
            double bin_start = i * M_PI_PHI / 6.0;
            double bin_end = (i + 1) * M_PI_PHI / 6.0;
            printf("  [%.2f-%.2f): %d (%.1f%%)\n", 
                   bin_start, bin_end, hist_pi_phi[i], 100.0 * hist_pi_phi[i] / total_composites);
        }
        
        printf("\n");
    }
}

/* ============================================================================
 * ANALYSIS 4: COMPOSITE MAGNITUDE PATTERNS
 * ============================================================================
 */

void analyze_composite_patterns() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("ANALYSIS 4: Patterns in Composite Magnitudes\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("Looking for mathematical patterns in composite magnitude sequences\n\n");
    
    int positions[] = {3, 6, 9};
    const char* pos_names[] = {"Position 3", "Position 6", "Position 9"};
    
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("%s (base = %lu):\n", pos_names[p], base);
        
        // Collect composite magnitudes
        uint64_t composite_mags[100];
        int comp_count = 0;
        
        for (uint64_t mag = 0; mag < 100; mag++) {
            uint64_t candidate = base + mag * 12;
            if (!is_prime_trial(candidate)) {
                composite_mags[comp_count++] = mag;
            }
        }
        
        // Test various patterns
        printf("\nTesting pattern: magnitude = a × π×φ + b\n");
        for (int i = 0; i < 20 && i < comp_count; i++) {
            double a = composite_mags[i] / M_PI_PHI;
            double b = fmod((double)composite_mags[i], M_PI_PHI);
            printf("  mag=%lu: %.2f × π×φ + %.2f\n", composite_mags[i], a, b);
        }
        
        printf("\nTesting pattern: magnitude = a × φ + b\n");
        for (int i = 0; i < 20 && i < comp_count; i++) {
            double a = composite_mags[i] / M_PHI;
            double b = fmod((double)composite_mags[i], M_PHI);
            printf("  mag=%lu: %.2f × φ + %.2f\n", composite_mags[i], a, b);
        }
        
        printf("\nTesting pattern: magnitude = a × π + b\n");
        for (int i = 0; i < 20 && i < comp_count; i++) {
            double a = composite_mags[i] / M_PI;
            double b = fmod((double)composite_mags[i], M_PI);
            printf("  mag=%lu: %.2f × π + %.2f\n", composite_mags[i], a, b);
        }
        
        printf("\n");
    }
}

/* ============================================================================
 * ANALYSIS 5: PREDICTIVE FORMULA TEST
 * ============================================================================
 */

void test_predictive_formulas() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("ANALYSIS 5: Testing Predictive Formulas\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("Testing various formulas to predict composite magnitudes\n\n");
    
    int positions[] = {3, 6, 9};
    const char* pos_names[] = {"Position 3", "Position 6", "Position 9"};
    
    for (int p = 0; p < 3; p++) {
        int position = positions[p];
        uint64_t base = (position == 3) ? 17 : (position == 6) ? 7 : 11;
        
        printf("%s (base = %lu):\n", pos_names[p], base);
        
        // Collect actual composite/prime status
        bool is_composite[100];
        for (uint64_t mag = 0; mag < 100; mag++) {
            uint64_t candidate = base + mag * 12;
            is_composite[mag] = !is_prime_trial(candidate);
        }
        
        // Test Formula 1: magnitude mod 5 predicts composites
        printf("\nFormula 1: Composite if (magnitude mod 5) ∈ {specific values}\n");
        int mod5_composite_counts[5] = {0};
        int mod5_prime_counts[5] = {0};
        
        for (uint64_t mag = 0; mag < 100; mag++) {
            int mod5 = mag % 5;
            if (is_composite[mag]) {
                mod5_composite_counts[mod5]++;
            } else {
                mod5_prime_counts[mod5]++;
            }
        }
        
        printf("  Composite distribution by (mag mod 5):\n");
        for (int i = 0; i < 5; i++) {
            int total = mod5_composite_counts[i] + mod5_prime_counts[i];
            printf("    mod %d: %d composites, %d primes (%.1f%% composite)\n", 
                   i, mod5_composite_counts[i], mod5_prime_counts[i],
                   100.0 * mod5_composite_counts[i] / total);
        }
        
        // Test Formula 2: magnitude mod π×φ predicts composites
        printf("\nFormula 2: Composite if (magnitude mod π×φ) ∈ {specific ranges}\n");
        
        // Divide π×φ into 10 bins
        int bins = 10;
        int bin_composite_counts[10] = {0};
        int bin_prime_counts[10] = {0};
        
        for (uint64_t mag = 0; mag < 100; mag++) {
            double mod_pi_phi = fmod((double)mag, M_PI_PHI);
            int bin = (int)(mod_pi_phi / M_PI_PHI * bins);
            if (bin >= bins) bin = bins - 1;
            
            if (is_composite[mag]) {
                bin_composite_counts[bin]++;
            } else {
                bin_prime_counts[bin]++;
            }
        }
        
        printf("  Composite distribution by (mag mod π×φ) bins:\n");
        for (int i = 0; i < bins; i++) {
            double bin_start = i * M_PI_PHI / bins;
            double bin_end = (i + 1) * M_PI_PHI / bins;
            int total = bin_composite_counts[i] + bin_prime_counts[i];
            if (total > 0) {
                printf("    [%.2f-%.2f): %d composites, %d primes (%.1f%% composite)\n", 
                       bin_start, bin_end, bin_composite_counts[i], bin_prime_counts[i],
                       100.0 * bin_composite_counts[i] / total);
            }
        }
        
        printf("\n");
    }
}

/* ============================================================================
 * ANALYSIS 6: CROSS-POSITION PRODUCT PREDICTION
 * ============================================================================
 */

void analyze_product_prediction() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("ANALYSIS 6: Predicting Cross-Position Products\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("Can we predict when a magnitude will produce a cross-position product?\n\n");
    
    // Generate small primes at each position
    uint64_t primes_0[] = {2, 3, 5};
    uint64_t primes_3[50];
    uint64_t primes_6[50];
    uint64_t primes_9[50];
    
    int count_3 = 0, count_6 = 0, count_9 = 0;
    
    for (uint64_t mag = 0; mag < 50; mag++) {
        uint64_t c3 = 17 + mag * 12;
        uint64_t c6 = 7 + mag * 12;
        uint64_t c9 = 11 + mag * 12;
        
        if (is_prime_trial(c3)) primes_3[count_3++] = c3;
        if (is_prime_trial(c6)) primes_6[count_6++] = c6;
        if (is_prime_trial(c9)) primes_9[count_9++] = c9;
    }
    
    printf("Generated primes:\n");
    printf("  Position 0: 3 primes (2, 3, 5)\n");
    printf("  Position 3: %d primes\n", count_3);
    printf("  Position 6: %d primes\n", count_6);
    printf("  Position 9: %d primes\n\n", count_9);
    
    // For position 3, predict which magnitudes will be products
    printf("Position 3 (17+12n) - Predicting composites:\n");
    printf("%-6s %-12s %-8s %-30s\n", "Mag", "Candidate", "Status", "Predicted Product");
    printf("────────────────────────────────────────────────────────────────────\n");
    
    for (uint64_t mag = 0; mag < 30; mag++) {
        uint64_t candidate = 17 + mag * 12;
        bool is_prime = is_prime_trial(candidate);
        
        // Check if candidate is a product of known primes
        bool predicted_composite = false;
        uint64_t factor1 = 0, factor2 = 0;
        
        // Check products with position 0 primes
        for (int i = 0; i < 3; i++) {
            if (candidate % primes_0[i] == 0) {
                factor1 = primes_0[i];
                factor2 = candidate / primes_0[i];
                predicted_composite = true;
                break;
            }
        }
        
        // Check products with other positions
        if (!predicted_composite) {
            for (int i = 0; i < count_6 && primes_6[i] * primes_6[i] <= candidate; i++) {
                if (candidate % primes_6[i] == 0) {
                    factor1 = primes_6[i];
                    factor2 = candidate / primes_6[i];
                    predicted_composite = true;
                    break;
                }
            }
        }
        
        if (!predicted_composite) {
            for (int i = 0; i < count_9 && primes_9[i] * primes_9[i] <= candidate; i++) {
                if (candidate % primes_9[i] == 0) {
                    factor1 = primes_9[i];
                    factor2 = candidate / primes_9[i];
                    predicted_composite = true;
                    break;
                }
            }
        }
        
        // Check same-position products
        if (!predicted_composite) {
            for (int i = 0; i < count_3 && primes_3[i] * primes_3[i] <= candidate; i++) {
                if (candidate % primes_3[i] == 0) {
                    factor1 = primes_3[i];
                    factor2 = candidate / primes_3[i];
                    predicted_composite = true;
                    break;
                }
            }
        }
        
        char prediction[50] = "";
        if (predicted_composite) {
            snprintf(prediction, sizeof(prediction), "%lu × %lu", factor1, factor2);
        }
        
        printf("%-6lu %-12lu %-8s %-30s %s\n", 
               mag, candidate, is_prime ? "PRIME" : "COMPOSITE", 
               prediction,
               (predicted_composite == !is_prime) ? "✓" : "✗");
    }
    
    printf("\n");
}

/* ============================================================================
 * MAIN
 * ============================================================================
 */

int main() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  INTERFERENCE PATTERN ANALYSIS                               ║\n");
    printf("║  Testing if π × φ predicts composite magnitudes              ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    printf("Constants:\n");
    printf("  π = %.15f\n", M_PI);
    printf("  φ = %.15f\n", M_PHI);
    printf("  π × φ = %.15f\n\n", M_PI_PHI);
    
    analyze_composite_magnitudes();
    analyze_magnitude_mod_pi_phi();
    analyze_interference_periodicity();
    analyze_product_prediction();
    
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("ANALYSIS COMPLETE\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}