/*
 * analyze_rainbow_structure.c - Deep analysis of rainbow table structure
 * 
 * This tool analyzes the geometric structure of the rainbow table to determine
 * if pre-generation affects the crystalline lattice properties.
 */

#include "prime_rainbow.h"
#include "bigint_core.h"
#include "prime_lattice_core.h"
#include "cllm_mathematical_constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Analyze symmetry group distribution
void analyze_symmetry_distribution(int num_primes) {
    printf("\n=== Symmetry Group Distribution (mod 12) ===\n");
    
    int groups[12] = {0};
    
    for (int i = 0; i < num_primes; i++) {
        BigInt* prime = rainbow_table_get_prime(i);
        if (prime) {
            uint64_t p = bigint_to_uint64(prime);
            int group = p % 12;
            groups[group]++;
        }
    }
    
    printf("Group | Count | Percentage\n");
    printf("------|-------|------------\n");
    for (int i = 0; i < 12; i++) {
        double pct = (groups[i] * 100.0) / num_primes;
        printf("  %2d  | %5d | %6.2f%%\n", i, groups[i], pct);
    }
    
    // Check for 12-fold symmetry balance
    printf("\nSymmetry Balance Analysis:\n");
    int viable_groups[] = {1, 5, 7, 11};  // Primes can only be in these groups (except 2, 3)
    int total_viable = 0;
    for (int i = 0; i < 4; i++) {
        total_viable += groups[viable_groups[i]];
    }
    
    printf("Viable groups (1,5,7,11): %d primes (%.2f%%)\n", 
           total_viable, (total_viable * 100.0) / num_primes);
    printf("Special primes (2,3): %d primes\n", groups[2] + groups[3]);
}

// Analyze angular positions
void analyze_angular_positions(int num_primes) {
    printf("\n=== Angular Position Analysis ===\n");
    
    double angles[12] = {0};  // Average angle per symmetry group
    int counts[12] = {0};
    
    for (int i = 0; i < num_primes && i < 1000; i++) {  // Sample first 1000
        BigInt* prime = rainbow_table_get_prime(i);
        if (prime) {
            uint64_t p = bigint_to_uint64(prime);
            int k = p % 12;
            
            // Calculate θ(n,k,λ,ω,ψ)
            double theta = theta_n(p, k, "a", 432, 3, 4, false);
            angles[k] += theta;
            counts[k]++;
        }
    }
    
    printf("Group | Avg θ | Count\n");
    printf("------|-------|-------\n");
    for (int i = 0; i < 12; i++) {
        if (counts[i] > 0) {
            double avg = angles[i] / counts[i];
            printf("  %2d  | %6.2f | %5d\n", i, avg, counts[i]);
        }
    }
}

// Analyze L(n,d,k,λ) values
void analyze_lattice_values(int num_primes) {
    printf("\n=== L(n,d,k,λ) Lattice Formula Analysis ===\n");
    
    double L_values[12] = {0};  // Average L value per dimension
    
    for (int i = 0; i < num_primes && i < 100; i++) {  // Sample first 100
        BigInt* prime = rainbow_table_get_prime(i);
        if (prime) {
            uint64_t p = bigint_to_uint64(prime);
            int k = p % 12;
            
            for (int d = 0; d < 12; d++) {
                double L = L_lattice(p, d, k, "a", 432, 3, 4);
                L_values[d] += L;
            }
        }
    }
    
    printf("Dimension | Avg L Value\n");
    printf("----------|-------------\n");
    for (int d = 0; d < 12; d++) {
        double avg = L_values[d] / (num_primes < 100 ? num_primes : 100);
        printf("    %2d    | %12.6f\n", d, avg);
    }
}

// Analyze tree structure
void analyze_tree_structure() {
    printf("\n=== Rainbow Table Tree Structure ===\n");
    
    PrimeRainbowTable* table = rainbow_table_get();
    if (!table) {
        printf("ERROR: Could not get rainbow table\n");
        return;
    }
    
    printf("Total primes: %d\n", table->count);
    printf("Stability: %s\n", table->is_stable ? "STABLE" : "UNSTABLE");
    
    printf("\nFold Progression (12-fold):\n");
    for (int i = 0; i < 12; i++) {
        printf("  Fold %2d: %.6f\n", i, table->fold_progression[i]);
    }
    
    printf("\nNegative Space (complementary):\n");
    for (int i = 0; i < 12; i++) {
        printf("  Space %2d: %.6f\n", i, table->negative_space[i]);
    }
    
    // Check self-similarity
    double similarity = rainbow_table_self_similarity(table);
    printf("\nSelf-Similarity: %.6f\n", similarity);
    
    // Check stability
    double stability = rainbow_table_check_stability(table);
    printf("Stability Metric: %.6f\n", stability);
}

// Measure generation time
void measure_generation_time(int target_count) {
    printf("\n=== Generation Time Analysis ===\n");
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    int generated = rainbow_table_generate_primes(target_count);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Generated %d primes in %.6f seconds\n", generated, elapsed);
    printf("Rate: %.2f primes/second\n", generated / elapsed);
    printf("Time per prime: %.6f ms\n", (elapsed * 1000.0) / generated);
}

// Analyze BigInt usage
void analyze_bigint_usage(int num_primes) {
    printf("\n=== BigInt Arbitrary Precision Analysis ===\n");
    
    int small_primes = 0;   // < 2^32
    int medium_primes = 0;  // 2^32 to 2^64
    int large_primes = 0;   // > 2^64 (requires BigInt)
    
    for (int i = 0; i < num_primes; i++) {
        BigInt* prime = rainbow_table_get_prime(i);
        if (prime) {
            uint64_t p = bigint_to_uint64(prime);
            if (p < 0xFFFFFFFFULL) {
                small_primes++;
            } else {
                medium_primes++;
            }
            // Note: large_primes would require checking BigInt size
        }
    }
    
    printf("Small primes (< 2^32): %d (%.2f%%)\n", 
           small_primes, (small_primes * 100.0) / num_primes);
    printf("Medium primes (2^32-2^64): %d (%.2f%%)\n", 
           medium_primes, (medium_primes * 100.0) / num_primes);
    printf("Large primes (> 2^64): %d (%.2f%%)\n", 
           large_primes, (large_primes * 100.0) / num_primes);
    
    printf("\nBigInt Structure Size: %zu bytes\n", sizeof(BigInt));
    printf("Total memory for %d primes: %.2f KB\n", 
           num_primes, (num_primes * sizeof(BigInt)) / 1024.0);
}

int main(int argc, char** argv) {
    int target_count = 10000;  // Default
    
    if (argc > 1) {
        target_count = atoi(argv[1]);
    }
    
    printf("=======================================================\n");
    printf("  Rainbow Table Structure Analysis\n");
    printf("  Target: %d primes\n", target_count);
    printf("=======================================================\n");
    
    // Initialize rainbow table
    rainbow_table_init();
    
    // Load important primes
    int important = rainbow_table_load_important_primes();
    printf("\nLoaded %d important primes\n", important);
    
    // Measure generation time
    measure_generation_time(target_count - important);
    
    int total_primes = rainbow_table_get_count();
    printf("\nTotal primes in table: %d\n", total_primes);
    
    // Analyze structure
    analyze_tree_structure();
    analyze_symmetry_distribution(total_primes);
    analyze_angular_positions(total_primes);
    analyze_lattice_values(total_primes);
    analyze_bigint_usage(total_primes);
    
    printf("\n=======================================================\n");
    printf("  Analysis Complete\n");
    printf("=======================================================\n");
    
    return 0;
}