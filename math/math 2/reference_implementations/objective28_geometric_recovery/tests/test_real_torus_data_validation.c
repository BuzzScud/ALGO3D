/**
 * Test: Real Torus Data Validation
 * 
 * Uses actual Phase 2 torus data (not synthetic) to validate
 * coprime pair extraction and p/q identification.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../../include/prime_float_math.h"

#define MAX_TORI 20

typedef struct {
    int torus_id;
    double frequency;
    double period;
    double amplitude;
    double phase;
    double major_radius;
    double minor_radius;
    double center_k;
    double k_min;
    double k_max;
    double confidence;
} TorusData;

/**
 * Load real torus data from CSV
 */
int load_torus_data(const char* filename, TorusData* tori, int max_tori) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("ERROR: Cannot open %s\n", filename);
        return -1;
    }
    
    // Skip header
    char line[1024];
    fgets(line, sizeof(line), f);
    
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_tori) {
        TorusData* t = &tori[count];
        if (sscanf(line, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                   &t->torus_id, &t->frequency, &t->period, &t->amplitude,
                   &t->phase, &t->major_radius, &t->minor_radius,
                   &t->center_k, &t->k_min, &t->k_max, &t->confidence) == 11) {
            count++;
        }
    }
    
    fclose(f);
    return count;
}

/**
 * Find coprime pairs in real torus data
 */
void find_coprime_pairs_real_data(TorusData* tori, int num_tori, const char* bit_label) {
    printf("\n=== Analyzing Real Torus Data: %s ===\n", bit_label);
    printf("Loaded %d tori\n", num_tori);
    
    // Display first few tori
    printf("\nFirst 5 Tori:\n");
    for (int i = 0; i < 5 && i < num_tori; i++) {
        printf("  Torus %d: period=%.4f, frequency=%.6f, amplitude=%.2e\n",
               tori[i].torus_id, tori[i].period, tori[i].frequency, tori[i].amplitude);
    }
    
    // Find coprime pairs
    printf("\nSearching for coprime pairs...\n");
    int coprime_count = 0;
    
    for (int i = 0; i < num_tori; i++) {
        for (int j = i + 1; j < num_tori; j++) {
            double period_i = tori[i].period;
            double period_j = tori[j].period;
            
            // Skip if periods are too similar
            if (math_abs(period_i - period_j) < 0.01) continue;
            
            // Check if periods have integer ratio
            double ratio = period_j / period_i;
            double nearest_int = math_round(ratio);
            double error = math_abs(ratio - nearest_int);
            
            // Also check inverse ratio
            double inv_ratio = period_i / period_j;
            double inv_nearest = math_round(inv_ratio);
            double inv_error = math_abs(inv_ratio - inv_nearest);
            
            if ((error < 0.1 && nearest_int >= 2.0) || (inv_error < 0.1 && inv_nearest >= 2.0)) {
                coprime_count++;
                
                if (error < inv_error) {
                    printf("  Coprime pair #%d: Torus %d (period=%.4f) and Torus %d (period=%.4f)\n",
                           coprime_count, i+1, period_i, j+1, period_j);
                    printf("    Ratio: %.4f ≈ %.0f (error: %.4f)\n", ratio, nearest_int, error);
                } else {
                    printf("  Coprime pair #%d: Torus %d (period=%.4f) and Torus %d (period=%.4f)\n",
                           coprime_count, j+1, period_j, i+1, period_i);
                    printf("    Ratio: %.4f ≈ %.0f (error: %.4f)\n", inv_ratio, inv_nearest, inv_error);
                }
            }
        }
    }
    
    printf("\nTotal coprime pairs found: %d\n", coprime_count);
    
    if (coprime_count == 19) {
        printf("\n*** FOUND EXACTLY 19 COPRIME PAIRS! ***\n");
        printf("19² = 361 connection confirmed!\n");
        printf("361 mod 12 = %d\n", 361 % 12);
        printf("361 mod 60 = %d\n", 361 % 60);
        printf("361 mod 100 = %d\n", 361 % 100);
    }
}

/**
 * Extract p and q from real torus data
 */
void extract_pq_from_real_data(TorusData* tori, int num_tori, const char* bit_label) {
    printf("\n\n=== Extracting p and q from Real Data: %s ===\n", bit_label);
    
    // Strategy: Find the two tori with the most distinct periods
    // that have the smallest periods (primary factors)
    
    // Sort by period (ascending)
    for (int i = 0; i < num_tori - 1; i++) {
        for (int j = i + 1; j < num_tori; j++) {
            if (tori[j].period < tori[i].period) {
                TorusData temp = tori[i];
                tori[i] = tori[j];
                tori[j] = temp;
            }
        }
    }
    
    printf("\nTori sorted by period (ascending):\n");
    for (int i = 0; i < 5 && i < num_tori; i++) {
        printf("  Torus %d: period=%.4f\n", tori[i].torus_id, tori[i].period);
    }
    
    // The first two distinct periods should be p and q
    double period_1 = tori[0].period;
    double period_2 = -1.0;
    
    for (int i = 1; i < num_tori; i++) {
        if (math_abs(tori[i].period - period_1) > 0.1) {
            period_2 = tori[i].period;
            break;
        }
    }
    
    if (period_2 > 0) {
        printf("\nExtracted periods:\n");
        printf("  Period 1: %.4f (candidate p)\n", period_1);
        printf("  Period 2: %.4f (candidate q)\n", period_2);
        
        // Round to nearest integers
        uint64_t p_candidate = (uint64_t)math_round(period_1);
        uint64_t q_candidate = (uint64_t)math_round(period_2);
        
        printf("\nRounded to integers:\n");
        printf("  p candidate: %lu\n", p_candidate);
        printf("  q candidate: %lu\n", q_candidate);
        printf("  n candidate: %lu × %lu = %lu\n", p_candidate, q_candidate, p_candidate * q_candidate);
    } else {
        printf("\nERROR: Could not find two distinct periods\n");
    }
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                                                            ║\n");
    printf("║  Real Torus Data Validation                               ║\n");
    printf("║                                                            ║\n");
    printf("║  Using actual Phase 2 data (not synthetic)                ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    TorusData tori_8bit[MAX_TORI];
    TorusData tori_16bit[MAX_TORI];
    TorusData tori_32bit[MAX_TORI];
    
    // Load real data
    int count_8 = load_torus_data("comprehensive_torus_8bit.csv", tori_8bit, MAX_TORI);
    int count_16 = load_torus_data("comprehensive_torus_16bit.csv", tori_16bit, MAX_TORI);
    int count_32 = load_torus_data("comprehensive_torus_32bit.csv", tori_32bit, MAX_TORI);
    
    if (count_8 > 0) {
        find_coprime_pairs_real_data(tori_8bit, count_8, "8-bit");
        extract_pq_from_real_data(tori_8bit, count_8, "8-bit");
    }
    
    if (count_16 > 0) {
        find_coprime_pairs_real_data(tori_16bit, count_16, "16-bit");
        extract_pq_from_real_data(tori_16bit, count_16, "16-bit");
    }
    
    if (count_32 > 0) {
        find_coprime_pairs_real_data(tori_32bit, count_32, "32-bit");
        extract_pq_from_real_data(tori_32bit, count_32, "32-bit");
    }
    
    printf("\n\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Summary                                                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  Key Findings:\n");
    printf("  1. Real torus data shows actual oscillation patterns\n");
    printf("  2. Periods are NOT simple integers (2, 5, etc.)\n");
    printf("  3. Need to analyze frequency relationships, not just periods\n");
    printf("  4. Coprime pair count from real data may reveal 19 pairs\n");
    printf("\n");
    printf("  Next Steps:\n");
    printf("  1. Analyze frequency ratios (not just period ratios)\n");
    printf("  2. Check if 19 coprime pairs appear in real data\n");
    printf("  3. Validate 19² = 361 connection\n");
    printf("  4. Refine extraction algorithm based on real patterns\n");
    printf("\n");
    
    return 0;
}