#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "../include/crystal_abacus.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Platonic solid definitions
typedef struct {
    const char* name;
    int p_s;        // Base prime (2, 3, 5)
    int d;          // Dimension
    int target;     // p_s^d
    uint64_t representative_prime;
} PlatonicSolid;

static const PlatonicSolid SOLIDS[] = {
    {"Tetrahedron", 3, 3, 27, 29},
    {"Cube", 2, 2, 4, 5},
    {"Octahedron", 3, 3, 27, 23},
    {"Dodecahedron", 5, 3, 125, 127},
    {"Icosahedron", 3, 5, 243, 241}
};

#define NUM_SOLIDS 5

// Calculate geometric resonance score
double geometric_resonance(uint64_t n) {
    double score = 0.0;
    double sigma = 100.0;
    
    for (int i = 0; i < NUM_SOLIDS; i++) {
        double dist = (double)n - (double)SOLIDS[i].target;
        score += math_exp(-(dist * dist) / sigma);
    }
    
    return score;
}

// Calculate P_d(n) magnitude (simplified version)
double calculate_resonance_magnitude(uint64_t n, int d, uint64_t p_d) {
    // Cyclic term: e^(i·2π·n/p_d^d)
    double grid_size = math_pow((double)p_d, (double)d);
    double phase = (2.0 * M_PI * (double)n) / grid_size;
    
    // Geometric resonance
    double geom_res = geometric_resonance(n);
    
    // Simplified magnitude (full formula would include sine product)
    return geom_res * (1.0 + math_cos(phase));
}

// Test if a prime is a Platonic representative
bool is_platonic_prime(uint64_t n) {
    for (int i = 0; i < NUM_SOLIDS; i++) {
        if (n == SOLIDS[i].representative_prime) {
            return true;
        }
    }
    return false;
}

// Analyze prime's dimensional affinity
void analyze_prime_dimension(uint64_t prime) {
    printf("\n  Analyzing prime: %lu\n", prime);
    printf("  Mod 12: %lu\n", prime % 12);
    
    // Check proximity to each Platonic target
    for (int i = 0; i < NUM_SOLIDS; i++) {
        int64_t dist = (int64_t)prime - (int64_t)SOLIDS[i].target;
        double resonance = math_exp(-(dist * dist) / 100.0);
        
        printf("    %s (p_s=%d, d=%d, target=%d): distance=%ld, resonance=%.6f\n",
               SOLIDS[i].name, SOLIDS[i].p_s, SOLIDS[i].d, 
               SOLIDS[i].target, dist, resonance);
    }
    
    // Calculate overall resonance
    double total_res = geometric_resonance(prime);
    printf("    Total geometric resonance: %.6f\n", total_res);
    
    // Check if it's a Platonic representative
    if (is_platonic_prime(prime)) {
        printf("    *** PLATONIC REPRESENTATIVE PRIME ***\n");
    }
}

// Test 1: Verify Platonic prime assignments
void test_platonic_assignments() {
    printf("\n\033[1m\033[36m=== TEST 1: Platonic Prime Assignments ===\033[0m\n");
    
    for (int i = 0; i < NUM_SOLIDS; i++) {
        uint64_t prime = SOLIDS[i].representative_prime;
        
        // Verify it's actually prime
        bool is_prime_val = is_prime(prime);
        
        // Check proximity to target
        int64_t dist = (int64_t)prime - (int64_t)SOLIDS[i].target;
        
        // Calculate resonance
        double resonance = geometric_resonance(prime);
        
        printf("%s: prime=%lu, target=%d, distance=%ld, resonance=%.6f, is_prime=%s\n",
               SOLIDS[i].name, prime, SOLIDS[i].target, dist, resonance,
               is_prime_val ? "\033[32mYES\033[0m" : "\033[31mNO\033[0m");
    }
}

// Test 2: Scan for highest resonance primes in each range
void test_resonance_peaks() {
    printf("\n\033[1m\033[36m=== TEST 2: Resonance Peak Analysis ===\033[0m\n");
    
    struct {
        uint64_t start;
        uint64_t end;
        const char* range_name;
    } ranges[] = {
        {2, 10, "Tiny (2-10)"},
        {2, 50, "Small (2-50)"},
        {2, 150, "Medium (2-150)"},
        {2, 300, "Large (2-300)"},
        {2, 1000, "Extended (2-1000)"}
    };
    
    for (int r = 0; r < 5; r++) {
        printf("\nRange: %s\n", ranges[r].range_name);
        
        uint64_t best_prime = 0;
        double best_resonance = 0.0;
        
        for (uint64_t n = ranges[r].start; n <= ranges[r].end; n++) {
            if (is_prime(n)) {
                double res = geometric_resonance(n);
                if (res > best_resonance) {
                    best_resonance = res;
                    best_prime = n;
                }
            }
        }
        
        printf("  Highest resonance prime: %lu (resonance=%.6f)\n", 
               best_prime, best_resonance);
        analyze_prime_dimension(best_prime);
    }
}

// Test 3: Dimensional clustering analysis
void test_dimensional_clustering() {
    printf("\n\033[1m\033[36m=== TEST 3: Dimensional Clustering ===\033[0m\n");
    
    // Count primes near each p_s^d for various d
    struct {
        int p_s;
        int d;
        uint64_t target;
        int count_within_10;
        int count_within_50;
    } clusters[15];
    
    int cluster_idx = 0;
    
    // Generate targets: 2^d, 3^d, 5^d for d=1 to 5
    for (int p_s = 2; p_s <= 5; p_s += (p_s == 2 ? 1 : 2)) {
        for (int d = 1; d <= 5; d++) {
            uint64_t target = 1;
            for (int i = 0; i < d; i++) {
                target *= p_s;
            }
            
            if (target > 1000000) break;
            
            clusters[cluster_idx].p_s = p_s;
            clusters[cluster_idx].d = d;
            clusters[cluster_idx].target = target;
            clusters[cluster_idx].count_within_10 = 0;
            clusters[cluster_idx].count_within_50 = 0;
            
            // Count primes within distance
            for (uint64_t n = (target > 50 ? target - 50 : 2); 
                 n <= target + 50 && n <= 1000000; n++) {
                if (is_prime(n)) {
                    int64_t dist = llabs((int64_t)n - (int64_t)target);
                    if (dist <= 10) clusters[cluster_idx].count_within_10++;
                    if (dist <= 50) clusters[cluster_idx].count_within_50++;
                }
            }
            
            cluster_idx++;
        }
    }
    
    printf("\nPrime clustering near p_s^d:\n");
    printf("p_s  d  target      within_10  within_50\n");
    printf("---  -  ----------  ---------  ---------\n");
    for (int i = 0; i < cluster_idx; i++) {
        printf("%3d  %d  %10lu  %9d  %9d\n",
               clusters[i].p_s, clusters[i].d, clusters[i].target,
               clusters[i].count_within_10, clusters[i].count_within_50);
    }
}

// Test 4: Mod 12 correlation with Platonic primes
void test_mod12_correlation() {
    printf("\n\033[1m\033[36m=== TEST 4: Mod 12 Correlation ===\033[0m\n");
    
    int mod12_counts[12] = {0};
    int platonic_mod12[NUM_SOLIDS];
    
    // Get mod 12 for Platonic primes
    printf("\nPlatonic primes mod 12:\n");
    for (int i = 0; i < NUM_SOLIDS; i++) {
        platonic_mod12[i] = SOLIDS[i].representative_prime % 12;
        printf("  %s (%lu) ≡ %d (mod 12)\n", 
               SOLIDS[i].name, SOLIDS[i].representative_prime, 
               platonic_mod12[i]);
    }
    
    // Count all primes by mod 12
    printf("\nAll primes up to 10,000 by mod 12:\n");
    for (uint64_t n = 2; n <= 10000; n++) {
        if (is_prime(n)) {
            mod12_counts[n % 12]++;
        }
    }
    
    for (int i = 0; i < 12; i++) {
        const char* color = (i == 1 || i == 5 || i == 7 || i == 11) ? "\033[32m" : "\033[31m";
        printf("  mod 12 = %2d: %s%4d\033[0m primes\n", i, color, mod12_counts[i]);
    }
}

// Test 5: Performance comparison
void test_performance_comparison() {
    printf("\n\033[1m\033[36m=== TEST 5: Performance Comparison ===\033[0m\n");
    
    // Test primes at different scales
    uint64_t test_primes[] = {
        1009, 10007, 100003, 1000003, 10000019, 100000007
    };
    
    printf("\nPrime         Validation (ns)  Resonance Calc (ns)  Speedup\n");
    printf("------------  ---------------  -------------------  -------\n");
    
    for (int i = 0; i < 6; i++) {
        uint64_t prime = test_primes[i];
        
        // Time validation (average of 100 runs)
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int j = 0; j < 100; j++) {
            is_prime(prime);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        uint64_t validation_ns = ((end.tv_sec - start.tv_sec) * 1000000000ULL +
                                 (end.tv_nsec - start.tv_nsec)) / 100;
        
        // Time resonance calculation (average of 100 runs)
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int j = 0; j < 100; j++) {
            geometric_resonance(prime);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        uint64_t resonance_ns = ((end.tv_sec - start.tv_sec) * 1000000000ULL +
                                (end.tv_nsec - start.tv_nsec)) / 100;
        
        double speedup = (double)validation_ns / (double)resonance_ns;
        
        const char* color = speedup < 1.0 ? "\033[31m" : "\033[32m";
        printf("%12lu  %15lu  %19lu  %s%7.2fx\033[0m\n",
               prime, validation_ns, resonance_ns, color, speedup);
    }
}

int main() {
    printf("\n\033[1m\033[34m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1m\033[34m║  PLATONIC PRIME RESONANCE ANALYSIS                         ║\033[0m\n");
    printf("\033[1m\033[34m║  Testing Geometric Proto-Language Prime Framework         ║\033[0m\n");
    printf("\033[1m\033[34m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    
    // Run all tests
    test_platonic_assignments();
    test_resonance_peaks();
    test_dimensional_clustering();
    test_mod12_correlation();
    test_performance_comparison();
    
    printf("\n\033[1m\033[32m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1m\033[32m║  ANALYSIS COMPLETE                                         ║\033[0m\n");
    printf("\033[1m\033[32m╚════════════════════════════════════════════════════════════╝\033[0m\n\n");
    
    return 0;
}
