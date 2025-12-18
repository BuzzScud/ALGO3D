#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "../include/crystal_abacus.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Extended Platonic solid structure
typedef struct {
    char name[64];
    int dimension;
    int vertices;
    int edges;
    int faces;
    int cells;
    int p_s;              // Base prime
    int d;                // Dimension exponent
    uint64_t target;      // p_s^d
    uint64_t prime;       // Representative prime
    double resonance;     // Geometric resonance
    int mod12;            // Residue class
    double distance;      // Distance to target
} ExtendedPlatonicSolid;

// Calculate geometric resonance for extended targets
double calculate_extended_resonance(uint64_t n, uint64_t* targets, int num_targets) {
    double score = 0.0;
    double sigma = 100.0;
    
    for (int i = 0; i < num_targets; i++) {
        double dist = (double)n - (double)targets[i];
        score += math_exp(-(dist * dist) / sigma);
    }
    
    return score;
}

// Find best prime near target
uint64_t find_best_prime_near_target(uint64_t target, double* resonance_out, 
                                     uint64_t* all_targets, int num_targets) {
    uint64_t best_prime = 0;
    double best_resonance = 0.0;
    
    // Search within ±50 of target
    int64_t start = (target > 50) ? (target - 50) : 2;
    uint64_t end = target + 50;
    
    for (uint64_t n = start; n <= end; n++) {
        if (is_prime(n)) {
            double res = calculate_extended_resonance(n, all_targets, num_targets);
            if (res > best_resonance) {
                best_resonance = res;
                best_prime = n;
            }
        }
    }
    
    *resonance_out = best_resonance;
    return best_prime;
}

// Check if prime is already assigned
bool is_already_assigned(uint64_t prime, ExtendedPlatonicSolid* solids, int count) {
    for (int i = 0; i < count; i++) {
        if (solids[i].prime == prime) {
            return true;
        }
    }
    return false;
}

// Discover new Platonic solids
void discover_platonic_solids(int max_dimension, uint64_t max_value) {
    printf("\n\033[1m\033[34m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1m\033[34m║  DISCOVERING EXTENDED PLATONIC SOLIDS                      ║\033[0m\n");
    printf("\033[1m\033[34m║  Dimensions: 3 to %d                                       ║\033[0m\n", max_dimension);
    printf("\033[1m\033[34m║  Maximum value: %lu                                   ║\033[0m\n", max_value);
    printf("\033[1m\033[34m╚════════════════════════════════════════════════════════════╝\033[0m\n\n");
    
    // Allocate storage
    ExtendedPlatonicSolid* solids = malloc(1000 * sizeof(ExtendedPlatonicSolid));
    uint64_t* all_targets = malloc(1000 * sizeof(uint64_t));
    int solid_count = 0;
    int target_count = 0;
    
    // First pass: Generate all targets
    printf("Generating dimensional targets...\n");
    
    int base_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23};
    int num_base_primes = 9;
    
    for (int dim = 1; dim <= max_dimension; dim++) {
        for (int bp_idx = 0; bp_idx < num_base_primes; bp_idx++) {
            int p_s = base_primes[bp_idx];
            
            for (int d = 1; d <= dim; d++) {
                uint64_t target = 1;
                for (int i = 0; i < d; i++) {
                    target *= p_s;
                    if (target > max_value) break;
                }
                
                if (target <= max_value && target >= 2) {
                    // Check if target already exists
                    bool exists = false;
                    for (int i = 0; i < target_count; i++) {
                        if (all_targets[i] == target) {
                            exists = true;
                            break;
                        }
                    }
                    
                    if (!exists) {
                        all_targets[target_count++] = target;
                    }
                }
            }
        }
    }
    
    printf("✓ Generated %d unique targets\n\n", target_count);
    
    // Second pass: Find best primes for each target
    printf("Finding representative primes for each target...\n");
    
    for (int dim = 3; dim <= max_dimension; dim++) {
        for (int bp_idx = 0; bp_idx < num_base_primes; bp_idx++) {
            int p_s = base_primes[bp_idx];
            
            for (int d = 1; d <= dim; d++) {
                uint64_t target = 1;
                for (int i = 0; i < d; i++) {
                    target *= p_s;
                    if (target > max_value) break;
                }
                
                if (target > max_value || target < 2) continue;
                
                double resonance;
                uint64_t prime = find_best_prime_near_target(target, &resonance, 
                                                             all_targets, target_count);
                
                if (prime > 0 && !is_already_assigned(prime, solids, solid_count)) {
                    // Create solid entry
                    ExtendedPlatonicSolid solid;
                    snprintf(solid.name, 64, "%dD-%d^%d", dim, p_s, d);
                    solid.dimension = dim;
                    solid.vertices = 0;
                    solid.edges = 0;
                    solid.faces = 0;
                    solid.cells = 0;
                    solid.p_s = p_s;
                    solid.d = d;
                    solid.target = target;
                    solid.prime = prime;
                    solid.resonance = resonance;
                    solid.mod12 = prime % 12;
                    solid.distance = math_abs((double)prime - (double)target);
                    
                    solids[solid_count++] = solid;
                }
            }
        }
    }
    
    printf("✓ Found %d extended Platonic solids\n\n", solid_count);
    
    // Sort by dimension, then by resonance
    for (int i = 0; i < solid_count - 1; i++) {
        for (int j = i + 1; j < solid_count; j++) {
            if (solids[j].dimension < solids[i].dimension ||
                (solids[j].dimension == solids[i].dimension && 
                 solids[j].resonance > solids[i].resonance)) {
                ExtendedPlatonicSolid temp = solids[i];
                solids[i] = solids[j];
                solids[j] = temp;
            }
        }
    }
    
    // Display results by dimension
    printf("\033[1m\033[36m=== EXTENDED PLATONIC SOLIDS BY DIMENSION ===\033[0m\n\n");
    
    int current_dim = -1;
    for (int i = 0; i < solid_count; i++) {
        if (solids[i].dimension != current_dim) {
            current_dim = solids[i].dimension;
            printf("\n\033[1m\033[33m--- %dD SOLIDS ---\033[0m\n\n", current_dim);
            printf("Name          p_s  d  Target     Prime      Resonance  mod12  Distance\n");
            printf("------------  ---  -  ---------  ---------  ---------  -----  --------\n");
        }
        
        printf("%-12s  %3d  %d  %9lu  %9lu  %9.6f  %5d  %8.1f\n",
               solids[i].name, solids[i].p_s, solids[i].d,
               solids[i].target, solids[i].prime,
               solids[i].resonance, solids[i].mod12, solids[i].distance);
    }
    
    // Known 3D Platonic solids
    printf("\n\n\033[1m\033[36m=== CLASSICAL 3D PLATONIC SOLIDS (REFERENCE) ===\033[0m\n\n");
    printf("Name          Formula  Target  Prime  Resonance\n");
    printf("------------  -------  ------  -----  ---------\n");
    printf("Tetrahedron   3^3      27      29     1.924\n");
    printf("Cube          2^2      4       5      1.006\n");
    printf("Octahedron    3^3      27      23     1.731\n");
    printf("Dodecahedron  5^3      125     127    0.961\n");
    printf("Icosahedron   3^5      243     241    0.961\n");
    
    // 4D Regular Polytopes
    printf("\n\n\033[1m\033[36m=== 4D REGULAR POLYTOPES (6 TYPES) ===\033[0m\n\n");
    printf("Name          Vertices  Formula  Target  Candidate Prime\n");
    printf("------------  --------  -------  ------  ---------------\n");
    
    struct {
        const char* name;
        int vertices;
        int p_s;
        int d;
    } polytopes_4d[] = {
        {"5-cell", 5, 5, 1},
        {"Tesseract", 16, 2, 4},
        {"16-cell", 8, 2, 3},
        {"24-cell", 24, 3, 3},
        {"120-cell", 600, 5, 4},
        {"600-cell", 120, 5, 3}
    };
    
    for (int i = 0; i < 6; i++) {
        uint64_t target = 1;
        for (int j = 0; j < polytopes_4d[i].d; j++) {
            target *= polytopes_4d[i].p_s;
        }
        
        double res;
        uint64_t prime = find_best_prime_near_target(target, &res, 
                                                     all_targets, target_count);
        
        printf("%-12s  %8d  %d^%d     %6lu  %15lu\n",
               polytopes_4d[i].name, polytopes_4d[i].vertices,
               polytopes_4d[i].p_s, polytopes_4d[i].d,
               target, prime);
    }
    
    // Export to CSV
    printf("\n\033[1m\033[36m=== EXPORTING DATA ===\033[0m\n\n");
    FILE* fp = fopen("extended_platonic_solids.csv", "w");
    if (fp) {
        fprintf(fp, "dimension,name,p_s,d,target,prime,resonance,mod12,distance\n");
        for (int i = 0; i < solid_count; i++) {
            fprintf(fp, "%d,%s,%d,%d,%lu,%lu,%.6f,%d,%.1f\n",
                    solids[i].dimension, solids[i].name,
                    solids[i].p_s, solids[i].d,
                    solids[i].target, solids[i].prime,
                    solids[i].resonance, solids[i].mod12,
                    solids[i].distance);
        }
        fclose(fp);
        printf("✓ Exported data to extended_platonic_solids.csv\n");
    }
    
    // Summary statistics
    printf("\n\033[1m\033[36m=== SUMMARY STATISTICS ===\033[0m\n\n");
    
    int dim_counts[20] = {0};
    int mod12_counts[12] = {0};
    
    for (int i = 0; i < solid_count; i++) {
        dim_counts[solids[i].dimension]++;
        mod12_counts[solids[i].mod12]++;
    }
    
    printf("Solids by dimension:\n");
    for (int d = 3; d <= max_dimension; d++) {
        if (dim_counts[d] > 0) {
            printf("  %dD: %d solids\n", d, dim_counts[d]);
        }
    }
    
    printf("\nMod 12 distribution:\n");
    for (int i = 0; i < 12; i++) {
        if (mod12_counts[i] > 0) {
            const char* color = (i == 1 || i == 5 || i == 7 || i == 11) ? "\033[32m" : "\033[31m";
            printf("  %smod 12 = %2d: %3d solids\033[0m\n", color, i, mod12_counts[i]);
        }
    }
    
    // Cleanup
    free(solids);
    free(all_targets);
    
    printf("\n\033[1m\033[32m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1m\033[32m║  DISCOVERY COMPLETE                                        ║\033[0m\n");
    printf("\033[1m\033[32m╚════════════════════════════════════════════════════════════╝\033[0m\n\n");
}

int main(int argc, char* argv[]) {
    int max_dimension = 8;
    uint64_t max_value = 1000000;
    
    if (argc > 1) {
        max_dimension = atoi(argv[1]);
    }
    if (argc > 2) {
        max_value = strtoull(argv[2], NULL, 10);
    }
    
    discover_platonic_solids(max_dimension, max_value);
    
    return 0;
}
