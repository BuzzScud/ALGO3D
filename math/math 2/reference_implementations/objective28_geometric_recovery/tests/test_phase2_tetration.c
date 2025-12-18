#include <stdio.h>
#include <stdlib.h>
#include "../include/tetration_attractors.h"
#include "../include/crystal_abacus.h"

// Inline abs for uint64_t
static inline uint64_t u64_abs_diff(uint64_t a, uint64_t b) {
    return (a > b) ? (a - b) : (b - a);
}

void test_euler_totient(void) {
    printf("=== TEST 1: Euler Totient Function ===\n\n");
    
    uint64_t test_vals[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 20, 30, 100};
    uint64_t expected[] = {1, 1, 2, 2, 4, 2, 6, 4, 6, 4, 4, 8, 8, 8, 8, 40};
    
    printf("Testing φ(n) for various n:\n");
    for (int i = 0; i < 16; i++) {
        uint64_t n = test_vals[i];
        uint64_t phi = euler_totient(n);
        bool correct = (phi == expected[i]);
        
        printf("  φ(%lu) = %lu (expected %lu) %s\n",
               n, phi, expected[i], correct ? "✓" : "✗");
    }
    printf("\n");
}

void test_modular_tetration(void) {
    printf("=== TEST 2: Modular Tetration ===\n\n");
    
    printf("Computing tetration towers:\n\n");
    
    // Test small modulus for verification
    uint64_t modulus = 1000;
    
    printf("Modulus: %lu\n\n", modulus);
    
    uint32_t bases[] = {2, 3, 5};
    uint32_t heights[] = {2, 3, 4};
    
    for (int b = 0; b < 3; b++) {
        printf("Base %u:\n", bases[b]);
        for (int h = 0; h < 3; h++) {
            uint64_t result = modular_tetration(bases[b], heights[h], modulus);
            printf("  %u^^%u mod %lu = %lu\n", 
                   bases[b], heights[h], modulus, result);
        }
        printf("\n");
    }
}

void test_tetration_with_damping(void) {
    printf("=== TEST 3: Tetration with Damping ===\n\n");
    
    uint64_t modulus = 10000;
    double dampings[] = {0.99, 0.95, 0.90, 0.85};
    
    printf("Testing damping effect (modulus: %lu):\n\n", modulus);
    
    uint32_t base = 3;
    uint32_t height = 4;
    
    printf("Base: %u, Height: %u\n\n", base, height);
    
    for (int d = 0; d < 4; d++) {
        uint64_t result = tetration_with_damping(base, height, dampings[d], modulus);
        printf("  Damping %.2f: %lu\n", dampings[d], result);
    }
    
    printf("\n");
}

void test_attractor_system(void) {
    printf("=== TEST 4: Attractor System ===\n\n");
    
    // Use smaller modulus for testing
    uint64_t modulus = 100000;
    double damping = 0.95;
    
    printf("Creating attractor system:\n");
    printf("  Modulus: %lu\n", modulus);
    printf("  Damping: %.3f\n\n", damping);
    
    AttractorSystem* system = create_attractor_system(modulus, damping);
    if (!system) {
        printf("Failed to create attractor system\n");
        return;
    }
    
    printf("Computing all attractors...\n");
    compute_all_attractors(system);
    
    printf("\nAttractor values (first 10):\n");
    for (uint32_t i = 0; i < 10 && i < system->num_attractors; i++) {
        TetrationAttractor* attr = &system->attractors[i];
        printf("  [%2u] %u^^%u = %lu\n",
               i, attr->base, attr->height, attr->value);
    }
    
    printf("\n");
    
    // Test nearest attractor finding
    printf("Testing nearest attractor finding:\n");
    uint64_t targets[] = {100, 1000, 10000, 50000};
    
    for (int i = 0; i < 4; i++) {
        uint64_t target = targets[i];
        TetrationAttractor* nearest = find_nearest_attractor(system, target);
        
        if (nearest) {
            uint64_t dist = u64_abs_diff(nearest->value, target);
            printf("  Target %lu: nearest is %u^^%u = %lu (distance: %lu)\n",
                   target, nearest->base, nearest->height, nearest->value, dist);
        }
    }
    
    printf("\n");
    
    // Test N nearest
    printf("Finding 3 nearest attractors to target 5000:\n");
    TetrationAttractor* nearest[3];
    uint32_t found = find_n_nearest_attractors(system, 5000, 3, nearest);
    
    for (uint32_t i = 0; i < found; i++) {
        uint64_t dist = u64_abs_diff(nearest[i]->value, 5000);
        double bias = compute_attractor_bias(5000, nearest[i]);
        printf("  %u. %u^^%u = %lu (distance: %lu, bias: %.6f)\n",
               i + 1, nearest[i]->base, nearest[i]->height,
               nearest[i]->value, dist, bias);
    }
    
    free_attractor_system(system);
    printf("\n");
}

void test_damping_adjustment(void) {
    printf("=== TEST 5: Damping Adjustment ===\n\n");
    
    double current = 0.95;
    double rates[] = {0.1, 0.05, 0.0, -0.05, -0.1};
    const char* labels[] = {"Fast converging", "Slow converging", "Stable", 
                           "Slow diverging", "Fast diverging"};
    
    printf("Testing damping adjustment:\n");
    printf("  Current damping: %.3f\n\n", current);
    
    for (int i = 0; i < 5; i++) {
        double new_damping = adjust_damping(current, rates[i]);
        printf("  %s (rate: %+.2f): %.3f → %.3f\n",
               labels[i], rates[i], current, new_damping);
    }
    
    printf("\n");
}

void test_integration_with_primes(void) {
    printf("=== TEST 6: Integration with Crystal Abacus ===\n\n");
    
    // Generate some primes
    CrystalAbacus* abacus = abacus_create();
    if (!abacus) {
        printf("Failed to create abacus\n");
        return;
    }
    
    printf("Generating 20 primes...\n");
    for (int i = 0; i < 20; i++) {
        abacus_next_prime(abacus);
    }
    
    printf("Generated %u primes\n\n", abacus->num_primes);
    
    // Create attractor system with modulus based on largest prime
    uint64_t largest_prime = abacus->primes[abacus->num_primes - 1];
    uint64_t modulus = largest_prime * largest_prime;  // Use p^2 as modulus
    
    printf("Creating attractor system:\n");
    printf("  Largest prime: %lu\n", largest_prime);
    printf("  Modulus: %lu\n\n", modulus);
    
    AttractorSystem* system = create_attractor_system(modulus, 0.95);
    if (!system) {
        printf("Failed to create system\n");
        abacus_free(abacus);
        return;
    }
    
    compute_all_attractors(system);
    
    // Find nearest attractors to each prime
    printf("Finding nearest attractors to primes:\n");
    for (uint32_t i = 0; i < 10 && i < abacus->num_primes; i++) {
        uint64_t prime = abacus->primes[i];
        TetrationAttractor* nearest = find_nearest_attractor(system, prime);
        
        if (nearest) {
            uint64_t dist = u64_abs_diff(nearest->value, prime);
            double bias = compute_attractor_bias(prime, nearest);
            printf("  Prime %lu: attractor %u^^%u = %lu (dist: %lu, bias: %.6f)\n",
                   prime, nearest->base, nearest->height, nearest->value, dist, bias);
        }
    }
    
    free_attractor_system(system);
    abacus_free(abacus);
    printf("\n");
}

int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 - Phase 2 Tetration Tests                   ║\n");
    printf("║  Tetration Attractors with Damping (NO math.h!)           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    test_euler_totient();
    test_modular_tetration();
    test_tetration_with_damping();
    test_attractor_system();
    test_damping_adjustment();
    test_integration_with_primes();
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  All Phase 2 Tetration Tests Complete                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}