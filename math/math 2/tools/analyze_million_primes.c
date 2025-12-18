#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "crystal_abacus.h"

// Platonic target definitions
typedef struct {
    int p_s;
    int d;
    uint64_t target;
} PlatonicTarget;

static const PlatonicTarget PLATONIC_TARGETS[] = {
    {2, 1, 2},
    {2, 2, 4},
    {2, 3, 8},
    {2, 4, 16},
    {2, 5, 32},
    {2, 6, 64},
    {2, 7, 128},
    {2, 8, 256},
    {2, 9, 512},
    {2, 10, 1024},
    {3, 1, 3},
    {3, 2, 9},
    {3, 3, 27},
    {3, 4, 81},
    {3, 5, 243},
    {3, 6, 729},
    {3, 7, 2187},
    {3, 8, 6561},
    {3, 9, 19683},
    {3, 10, 59049},
    {5, 1, 5},
    {5, 2, 25},
    {5, 3, 125},
    {5, 4, 625},
    {5, 5, 3125},
    {5, 6, 15625},
    {5, 7, 78125},
    {7, 1, 7},
    {7, 2, 49},
    {7, 3, 343},
    {7, 4, 2401},
    {7, 5, 16807},
    {11, 1, 11},
    {11, 2, 121},
    {11, 3, 1331},
    {11, 4, 14641},
    {13, 1, 13},
    {13, 2, 169},
    {13, 3, 2197},
};

#define NUM_TARGETS (sizeof(PLATONIC_TARGETS) / sizeof(PlatonicTarget))

// Calculate geometric resonance
double geometric_resonance(uint64_t n) {
    double score = 0.0;
    double sigma = 100.0;
    
    for (size_t i = 0; i < NUM_TARGETS; i++) {
        if (PLATONIC_TARGETS[i].target > 1000000) continue;
        double dist = (double)n - (double)PLATONIC_TARGETS[i].target;
        score += exp(-(dist * dist) / sigma);
    }
    
    return score;
}

// Prime data structure
typedef struct {
    uint64_t prime;
    double resonance;
    int mod12;
    uint64_t gap_to_next;
    int nearest_target_index;
    double distance_to_target;
} PrimeData;

// Generate all primes up to limit using Sieve of Eratosthenes
uint64_t* generate_primes_sieve(uint64_t limit, uint64_t* count) {
    // Allocate bit array
    uint8_t* is_prime = calloc((limit + 1) / 8 + 1, 1);
    
    // Initialize all as prime
    for (uint64_t i = 2; i <= limit; i++) {
        is_prime[i / 8] |= (1 << (i % 8));
    }
    
    // Sieve
    for (uint64_t i = 2; i * i <= limit; i++) {
        if (is_prime[i / 8] & (1 << (i % 8))) {
            for (uint64_t j = i * i; j <= limit; j += i) {
                is_prime[j / 8] &= ~(1 << (j % 8));
            }
        }
    }
    
    // Count primes
    *count = 0;
    for (uint64_t i = 2; i <= limit; i++) {
        if (is_prime[i / 8] & (1 << (i % 8))) {
            (*count)++;
        }
    }
    
    // Extract primes
    uint64_t* primes = malloc(*count * sizeof(uint64_t));
    uint64_t idx = 0;
    for (uint64_t i = 2; i <= limit; i++) {
        if (is_prime[i / 8] & (1 << (i % 8))) {
            primes[idx++] = i;
        }
    }
    
    free(is_prime);
    return primes;
}

// Analyze all primes
void analyze_all_primes(uint64_t limit) {
    printf("\n\033[1m\033[34m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1m\033[34m║  COMPREHENSIVE PRIME ANALYSIS UP TO %lu            ║\033[0m\n", limit);
    printf("\033[1m\033[34m╚════════════════════════════════════════════════════════════╝\033[0m\n\n");
    
    // Generate all primes
    printf("Generating primes using Sieve of Eratosthenes...\n");
    uint64_t count;
    uint64_t* primes = generate_primes_sieve(limit, &count);
    printf("✓ Generated %lu primes\n\n", count);
    
    // Analyze each prime
    printf("Analyzing prime properties...\n");
    PrimeData* data = malloc(count * sizeof(PrimeData));
    
    for (uint64_t i = 0; i < count; i++) {
        data[i].prime = primes[i];
        data[i].resonance = geometric_resonance(primes[i]);
        data[i].mod12 = primes[i] % 12;
        data[i].gap_to_next = (i < count - 1) ? (primes[i + 1] - primes[i]) : 0;
        
        // Find nearest target
        double min_dist = 1e9;
        int nearest_idx = -1;
        for (size_t j = 0; j < NUM_TARGETS; j++) {
            if (PLATONIC_TARGETS[j].target > limit) continue;
            double dist = fabs((double)primes[i] - (double)PLATONIC_TARGETS[j].target);
            if (dist < min_dist) {
                min_dist = dist;
                nearest_idx = j;
            }
        }
        data[i].nearest_target_index = nearest_idx;
        data[i].distance_to_target = min_dist;
        
        if ((i + 1) % 10000 == 0) {
            printf("  Processed %lu / %lu primes (%.1f%%)\r", 
                   i + 1, count, 100.0 * (i + 1) / count);
            fflush(stdout);
        }
    }
    printf("✓ Analyzed all %lu primes          \n\n", count);
    
    // Find top 100 highest resonance primes
    printf("\033[1m\033[36m=== TOP 100 HIGHEST RESONANCE PRIMES ===\033[0m\n\n");
    
    // Sort by resonance
    for (uint64_t i = 0; i < count - 1; i++) {
        for (uint64_t j = i + 1; j < count; j++) {
            if (data[j].resonance > data[i].resonance) {
                PrimeData temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
        if (i >= 99) break;  // Only need top 100
    }
    
    printf("Rank  Prime      Resonance  mod12  Nearest Target (p_s^d)  Distance\n");
    printf("----  ---------  ---------  -----  ---------------------  --------\n");
    for (int i = 0; i < 100 && i < (int)count; i++) {
        int target_idx = data[i].nearest_target_index;
        if (target_idx >= 0) {
            printf("%4d  %9lu  %9.6f  %5d  %d^%d = %lu  %8.1f\n",
                   i + 1, data[i].prime, data[i].resonance, data[i].mod12,
                   PLATONIC_TARGETS[target_idx].p_s,
                   PLATONIC_TARGETS[target_idx].d,
                   PLATONIC_TARGETS[target_idx].target,
                   data[i].distance_to_target);
        }
    }
    
    // Mod 12 distribution
    printf("\n\033[1m\033[36m=== MOD 12 DISTRIBUTION ===\033[0m\n\n");
    int mod12_counts[12] = {0};
    for (uint64_t i = 0; i < count; i++) {
        mod12_counts[data[i].mod12]++;
    }
    
    printf("mod12  Count    Percentage\n");
    printf("-----  -------  ----------\n");
    for (int i = 0; i < 12; i++) {
        const char* color = (i == 1 || i == 5 || i == 7 || i == 11) ? "\033[32m" : "\033[31m";
        printf("%s%5d  %7d  %9.2f%%\033[0m\n", 
               color, i, mod12_counts[i], 100.0 * mod12_counts[i] / count);
    }
    
    // Gap analysis
    printf("\n\033[1m\033[36m=== PRIME GAP ANALYSIS ===\033[0m\n\n");
    uint64_t max_gap = 0;
    uint64_t max_gap_prime = 0;
    uint64_t total_gap = 0;
    
    for (uint64_t i = 0; i < count - 1; i++) {
        if (data[i].gap_to_next > max_gap) {
            max_gap = data[i].gap_to_next;
            max_gap_prime = data[i].prime;
        }
        total_gap += data[i].gap_to_next;
    }
    
    printf("Average gap: %.2f\n", (double)total_gap / (count - 1));
    printf("Maximum gap: %lu (after prime %lu)\n", max_gap, max_gap_prime);
    
    // Export to CSV
    printf("\n\033[1m\033[36m=== EXPORTING DATA ===\033[0m\n\n");
    FILE* fp = fopen("prime_analysis_data.csv", "w");
    if (fp) {
        fprintf(fp, "prime,resonance,mod12,gap,nearest_target,distance\n");
        for (uint64_t i = 0; i < count; i++) {
            int target_idx = data[i].nearest_target_index;
            fprintf(fp, "%lu,%.6f,%d,%lu,%lu,%.1f\n",
                    data[i].prime, data[i].resonance, data[i].mod12,
                    data[i].gap_to_next,
                    target_idx >= 0 ? PLATONIC_TARGETS[target_idx].target : 0,
                    data[i].distance_to_target);
        }
        fclose(fp);
        printf("✓ Exported data to prime_analysis_data.csv\n");
    }
    
    // Cleanup
    free(primes);
    free(data);
    
    printf("\n\033[1m\033[32m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1m\033[32m║  ANALYSIS COMPLETE                                         ║\033[0m\n");
    printf("\033[1m\033[32m╚════════════════════════════════════════════════════════════╝\033[0m\n\n");
}

int main(int argc, char* argv[]) {
    uint64_t limit = 1000000;
    
    if (argc > 1) {
        limit = strtoull(argv[1], NULL, 10);
    }
    
    analyze_all_primes(limit);
    
    return 0;
}