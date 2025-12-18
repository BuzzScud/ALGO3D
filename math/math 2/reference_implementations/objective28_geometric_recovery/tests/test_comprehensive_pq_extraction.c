/**
 * test_comprehensive_pq_extraction.c - Comprehensive p and q Extraction
 * 
 * This test implements THREE approaches to extract p and q:
 * 
 * APPROACH A: Oscillation Period Analysis
 * - Analyze all 20 torus oscillation periods
 * - Find tori with coprime periods (gcd = 1)
 * - These represent p-torus and q-torus
 * 
 * APPROACH B: Torus Centers/Amplitudes
 * - Use torus centers as p and q estimates
 * - Use amplitude ratios to validate p/q relationship
 * - Extract from geometric properties
 * 
 * APPROACH C: Clock Lattice Factor Visualization
 * - Map n to clock lattice
 * - Use clock positions to identify factors
 * - Visualize p and q on clock
 * 
 * ADDITIONAL: Test n, n², n³ for ellipse dimensions
 * - Measure inside/outside bounds
 * - Triangulate using graph size
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "../include/multi_torus_tracker.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_TORI 20
#define MAX_SAMPLES 20

/**
 * Torus data structure
 */
typedef struct {
    int torus_id;
    double frequency;
    int period;
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
 * Sample with complete torus data
 */
typedef struct {
    int sample_id;
    uint64_t true_k;
    int num_tori;
    TorusData tori[MAX_TORI];
    double intersection_size;
    double reduction_factor;
} CompleteSampleData;

/**
 * p and q extraction result
 */
typedef struct {
    // Approach A: Period analysis
    uint64_t p_from_period;
    uint64_t q_from_period;
    int p_torus_id;
    int q_torus_id;
    int gcd_periods;
    
    // Approach B: Centers/amplitudes
    uint64_t p_from_center;
    uint64_t q_from_center;
    double amplitude_ratio;
    
    // Approach C: Clock lattice
    uint64_t p_from_clock;
    uint64_t q_from_clock;
    int clock_ring_p;
    int clock_ring_q;
    
    // Validation
    bool all_coprime;
    bool all_prime;
    uint64_t product_pq;
    
    // Graph dimensions
    uint64_t n;
    uint64_t n_squared;
    uint64_t n_cubed;
    double ellipse_major;
    double ellipse_minor;
} PQExtractionResult;

/**
 * Compute GCD
 */
uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Check if number is prime (simple test)
 */
bool is_prime_simple(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    for (uint64_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

/**
 * Load torus data from CSV
 */
int load_torus_data(const char* filename, TorusData* tori, int max_tori) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    
    // Skip header
    char line[1024];
    fgets(line, sizeof(line), f);
    
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_tori) {
        TorusData* t = &tori[count];
        
        int parsed = sscanf(line, "%d,%lf,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &t->torus_id,
            &t->frequency,
            &t->period,
            &t->amplitude,
            &t->phase,
            &t->major_radius,
            &t->minor_radius,
            &t->center_k,
            &t->k_min,
            &t->k_max,
            &t->confidence
        );
        
        if (parsed >= 11) {
            count++;
        }
    }
    
    fclose(f);
    return count;
}

/**
 * APPROACH A: Extract p and q from oscillation periods
 */
void approach_a_period_analysis(
    const TorusData* tori,
    int num_tori,
    PQExtractionResult* result
) {
    printf("\n========================================\n");
    printf("APPROACH A: Oscillation Period Analysis\n");
    printf("========================================\n\n");
    
    printf("Analyzing %d tori for coprime periods...\n\n", num_tori);
    
    // Find pairs with coprime periods
    int coprime_pairs = 0;
    for (int i = 0; i < num_tori; i++) {
        for (int j = i + 1; j < num_tori; j++) {
            uint64_t period_i = (uint64_t)tori[i].period;
            uint64_t period_j = (uint64_t)tori[j].period;
            uint64_t g = gcd(period_i, period_j);
            
            if (g == 1) {
                printf("  Coprime pair found: Torus %d (period=%lu) and Torus %d (period=%lu)\n",
                       tori[i].torus_id, period_i, tori[j].torus_id, period_j);
                coprime_pairs++;
                
                // Use first coprime pair as p and q candidates
                if (result->p_torus_id == 0) {
                    result->p_torus_id = tori[i].torus_id;
                    result->q_torus_id = tori[j].torus_id;
                    result->p_from_period = period_i;
                    result->q_from_period = period_j;
                    result->gcd_periods = g;
                }
            }
        }
    }
    
    printf("\nTotal coprime pairs: %d\n", coprime_pairs);
    
    if (result->p_torus_id > 0) {
        printf("\nSelected p-torus: Torus %d (period=%lu)\n", result->p_torus_id, result->p_from_period);
        printf("Selected q-torus: Torus %d (period=%lu)\n", result->q_torus_id, result->q_from_period);
        printf("GCD(p_period, q_period) = %d ✓\n", result->gcd_periods);
    } else {
        printf("\n⚠ WARNING: No coprime period pairs found!\n");
        printf("This suggests all tori have the same period (period 2).\n");
    }
}

/**
 * APPROACH B: Extract p and q from torus centers/amplitudes
 */
void approach_b_center_amplitude(
    const TorusData* tori,
    int num_tori,
    PQExtractionResult* result
) {
    printf("\n========================================\n");
    printf("APPROACH B: Torus Centers/Amplitudes\n");
    printf("========================================\n\n");
    
    if (num_tori < 2) {
        printf("ERROR: Need at least 2 tori\n");
        return;
    }
    
    // Use first two tori as p and q candidates
    const TorusData* torus_1 = &tori[0];
    const TorusData* torus_2 = &tori[1];
    
    printf("Torus 1 (p-torus candidate):\n");
    printf("  Center k: %.2f\n", torus_1->center_k);
    printf("  Amplitude: %.2f\n", torus_1->amplitude);
    printf("  Major radius: %.2f\n", torus_1->major_radius);
    printf("  Minor radius: %.2f\n\n", torus_1->minor_radius);
    
    printf("Torus 2 (q-torus candidate):\n");
    printf("  Center k: %.2f\n", torus_2->center_k);
    printf("  Amplitude: %.2f\n", torus_2->amplitude);
    printf("  Major radius: %.2f\n", torus_2->major_radius);
    printf("  Minor radius: %.2f\n\n", torus_2->minor_radius);
    
    // Extract p and q from centers
    result->p_from_center = (uint64_t)fabs(torus_1->center_k);
    result->q_from_center = (uint64_t)fabs(torus_2->center_k);
    
    // Compute amplitude ratio
    if (torus_2->amplitude > 0) {
        result->amplitude_ratio = torus_1->amplitude / torus_2->amplitude;
    }
    
    printf("Extracted values:\n");
    printf("  p (from center): %lu\n", result->p_from_center);
    printf("  q (from center): %lu\n", result->q_from_center);
    printf("  Amplitude ratio (p/q): %.6f\n", result->amplitude_ratio);
    
    // Check if they're prime
    bool p_prime = is_prime_simple(result->p_from_center);
    bool q_prime = is_prime_simple(result->q_from_center);
    
    printf("  p is prime: %s\n", p_prime ? "YES ✓" : "NO ✗");
    printf("  q is prime: %s\n", q_prime ? "YES ✓" : "NO ✗");
}

/**
 * APPROACH C: Clock lattice factor visualization
 */
void approach_c_clock_lattice(
    uint64_t n,
    PQExtractionResult* result
) {
    printf("\n========================================\n");
    printf("APPROACH C: Clock Lattice Visualization\n");
    printf("========================================\n\n");
    
    printf("Graph boundary n = %lu\n\n", n);
    
    // Try to factor n
    printf("Attempting factorization of n...\n");
    
    uint64_t sqrt_n = (uint64_t)math_sqrt((double)n);
    bool found = false;
    
    for (uint64_t p = 2; p <= sqrt_n && !found; p++) {
        if (n % p == 0) {
            uint64_t q = n / p;
            
            // Check if both are prime
            if (is_prime_simple(p) && is_prime_simple(q)) {
                result->p_from_clock = p;
                result->q_from_clock = q;
                found = true;
                
                printf("  Found prime factorization: %lu × %lu = %lu\n", p, q, n);
                
                // Map to clock rings (simplified)
                // Ring 0: 1-12, Ring 1: 13-60, Ring 2: 61-120, Ring 3: 121+
                if (p <= 12) result->clock_ring_p = 0;
                else if (p <= 60) result->clock_ring_p = 1;
                else if (p <= 120) result->clock_ring_p = 2;
                else result->clock_ring_p = 3;
                
                if (q <= 12) result->clock_ring_q = 0;
                else if (q <= 60) result->clock_ring_q = 1;
                else if (q <= 120) result->clock_ring_q = 2;
                else result->clock_ring_q = 3;
                
                printf("  p on clock ring: %d\n", result->clock_ring_p);
                printf("  q on clock ring: %d\n", result->clock_ring_q);
            }
        }
    }
    
    if (!found) {
        printf("  ⚠ No prime factorization found for n\n");
        printf("  n may be prime or have only composite factors\n");
    }
}

/**
 * Test graph dimensions: n, n², n³
 */
void test_graph_dimensions(uint64_t n, PQExtractionResult* result) {
    printf("\n========================================\n");
    printf("GRAPH DIMENSIONS ANALYSIS\n");
    printf("========================================\n\n");
    
    result->n = n;
    result->n_squared = n * n;  // May overflow for large n
    result->n_cubed = 0;  // Would definitely overflow
    
    printf("Graph dimensions:\n");
    printf("  n   = %lu\n", result->n);
    printf("  n²  = %lu", result->n_squared);
    if (result->n_squared < n) {
        printf(" (OVERFLOW!)");
    }
    printf("\n");
    printf("  n³  = (too large to compute)\n\n");
    
    // Compute ellipse dimensions
    // Major axis = n, Minor axis = n/φ (golden ratio)
    double phi = (1.0 + math_sqrt(5.0)) / 2.0;
    result->ellipse_major = (double)n;
    result->ellipse_minor = (double)n / phi;
    
    printf("Ellipse dimensions (using golden ratio):\n");
    printf("  Major axis: %.2f\n", result->ellipse_major);
    printf("  Minor axis: %.2f\n", result->ellipse_minor);
    printf("  Ratio (major/minor): %.6f (should be φ = %.6f)\n", 
           result->ellipse_major / result->ellipse_minor, phi);
}

/**
 * Validate all results
 */
void validate_results(PQExtractionResult* result) {
    printf("\n========================================\n");
    printf("VALIDATION & COMPARISON\n");
    printf("========================================\n\n");
    
    printf("Results from each approach:\n\n");
    
    printf("Approach A (Period Analysis):\n");
    printf("  p = %lu (from period)\n", result->p_from_period);
    printf("  q = %lu (from period)\n", result->q_from_period);
    printf("  Coprime: %s\n\n", result->gcd_periods == 1 ? "YES ✓" : "NO ✗");
    
    printf("Approach B (Centers/Amplitudes):\n");
    printf("  p = %lu (from center)\n", result->p_from_center);
    printf("  q = %lu (from center)\n", result->q_from_center);
    printf("  Both prime: ");
    bool b_p_prime = is_prime_simple(result->p_from_center);
    bool b_q_prime = is_prime_simple(result->q_from_center);
    printf("%s\n\n", (b_p_prime && b_q_prime) ? "YES ✓" : "NO ✗");
    
    printf("Approach C (Clock Lattice):\n");
    printf("  p = %lu (from factorization)\n", result->p_from_clock);
    printf("  q = %lu (from factorization)\n", result->q_from_clock);
    printf("  Both prime: ");
    bool c_p_prime = is_prime_simple(result->p_from_clock);
    bool c_q_prime = is_prime_simple(result->q_from_clock);
    printf("%s\n\n", (c_p_prime && c_q_prime) ? "YES ✓" : "NO ✗");
    
    // Check consistency
    printf("Consistency check:\n");
    bool consistent_ab = (result->p_from_period == result->p_from_center) &&
                         (result->q_from_period == result->q_from_center);
    bool consistent_ac = (result->p_from_period == result->p_from_clock) &&
                         (result->q_from_period == result->q_from_clock);
    bool consistent_bc = (result->p_from_center == result->p_from_clock) &&
                         (result->q_from_center == result->q_from_clock);
    
    printf("  A ↔ B: %s\n", consistent_ab ? "CONSISTENT ✓" : "DIFFERENT");
    printf("  A ↔ C: %s\n", consistent_ac ? "CONSISTENT ✓" : "DIFFERENT");
    printf("  B ↔ C: %s\n", consistent_bc ? "CONSISTENT ✓" : "DIFFERENT");
}

/**
 * Test comprehensive extraction for a bit length
 */
void test_bit_length(int bit_length, uint64_t max_k) {
    printf("\n\n");
    printf("===========================================\n");
    printf("TESTING %d-BIT SAMPLES\n", bit_length);
    printf("===========================================\n");
    
    // Load torus data - try comprehensive file first
    char filename[256];
    snprintf(filename, sizeof(filename), "comprehensive_torus_%dbit.csv", bit_length);
    
    TorusData tori[MAX_TORI];
    int num_tori = load_torus_data(filename, tori, MAX_TORI);
    
    if (num_tori == 0) {
        // Try multi_torus file as fallback
        snprintf(filename, sizeof(filename), "multi_torus_%dbit.csv", bit_length);
        num_tori = load_torus_data(filename, tori, MAX_TORI);
    }
    
    if (num_tori == 0) {
        printf("\nERROR: Could not load torus data\n");
        printf("Skipping this bit length.\n");
        return;
    }
    
    printf("\nLoaded %d tori from %s\n", num_tori, filename);
    
    // Initialize result
    PQExtractionResult result = {0};
    
    // Run all three approaches
    approach_a_period_analysis(tori, num_tori, &result);
    approach_b_center_amplitude(tori, num_tori, &result);
    approach_c_clock_lattice(max_k, &result);
    
    // Test graph dimensions
    test_graph_dimensions(max_k, &result);
    
    // Validate
    validate_results(&result);
}

int main() {
    printf("===========================================\n");
    printf("COMPREHENSIVE p AND q EXTRACTION\n");
    printf("===========================================\n");
    printf("\nTesting THREE approaches:\n");
    printf("  A) Oscillation period analysis\n");
    printf("  B) Torus centers/amplitudes\n");
    printf("  C) Clock lattice factorization\n");
    printf("\nPlus: Graph dimensions (n, n², n³)\n");
    
    // Test all bit lengths
    test_bit_length(8, 255);
    test_bit_length(16, 65535);
    test_bit_length(32, 4294967295UL);
    
    printf("\n\n");
    printf("===========================================\n");
    printf("COMPREHENSIVE ANALYSIS COMPLETE\n");
    printf("===========================================\n");
    
    return 0;
}