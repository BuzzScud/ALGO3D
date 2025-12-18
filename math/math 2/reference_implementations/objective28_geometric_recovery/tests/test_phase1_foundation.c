#include <stdio.h>
#include <stdlib.h>
#include "../include/crystal_abacus.h"
#include "../include/kissing_spheres.h"

void test_crystal_abacus(void) {
    printf("=== TEST 1: Crystal Abacus Prime Generation ===\n\n");
    
    CrystalAbacus* abacus = abacus_create();
    if (!abacus) {
        printf("Failed to create abacus\n");
        return;
    }
    
    printf("Generating first 50 primes with lattice filtering...\n");
    
    for (int i = 0; i < 50; i++) {
        uint64_t prime = abacus_next_prime(abacus);
        
        if (i < 20) {  // Print first 20
            printf("Prime %2d: %lu\n", i + 1, prime);
        }
    }
    
    printf("\nGenerated 50 primes\n");
    printf("Total primes in abacus: %u\n", abacus->num_primes);
    printf("Total embeddings seen: %u\n\n", abacus->num_seen);
    
    // Test is_prime
    printf("Testing is_prime function:\n");
    uint64_t test_vals[] = {97, 100, 101, 103, 104, 107, 109, 110};
    for (int i = 0; i < 8; i++) {
        bool prime = abacus_is_prime(abacus, test_vals[i]);
        printf("  %lu: %s\n", test_vals[i], prime ? "PRIME" : "COMPOSITE");
    }
    
    abacus_free(abacus);
    printf("\n");
}

void test_lattice_embedding(void) {
    printf("=== TEST 2: Lattice Embedding (Recursive) ===\n\n");
    
    uint64_t test_k[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    
    for (int i = 0; i < 10; i++) {
        uint64_t k = test_k[i];
        
        printf("k = %lu:\n", k);
        
        // Embed with depth 3
        LatticeEmbedding* emb = embed_k(k, 3);
        if (!emb) {
            printf("  Failed to embed\n");
            continue;
        }
        
        // Print base embedding (15D)
        printf("  Base (15D): [");
        for (int j = 0; j < 15; j++) {
            printf("%lu", emb->residues[j]);
            if (j < 14) printf(", ");
        }
        printf("]\n");
        
        // Print sub-embedding if exists
        if (emb->sub) {
            printf("  Sub-1 (15D): [");
            for (int j = 0; j < 15; j++) {
                printf("%lu", emb->sub->residues[j]);
                if (j < 14) printf(", ");
            }
            printf("]\n");
            
            if (emb->sub->sub) {
                printf("  Sub-2 (15D): [");
                for (int j = 0; j < 15; j++) {
                    printf("%lu", emb->sub->sub->residues[j]);
                    if (j < 14) printf(", ");
                }
                printf("]\n");
            }
        }
        
        printf("  Depth: %u\n\n", emb->depth);
        
        free_embedding(emb);
    }
}

void test_kissing_spheres(void) {
    printf("=== TEST 3: Kissing Spheres Hierarchy ===\n\n");
    
    uint64_t center = 100;
    uint32_t depth = 3;
    
    printf("Creating kissing sphere hierarchy:\n");
    printf("  Center: %lu\n", center);
    printf("  Depth: %u\n\n", depth);
    
    KissingSphere* root = create_kissing_sphere_hierarchy(center, depth);
    if (!root) {
        printf("Failed to create hierarchy\n");
        return;
    }
    
    printf("Created hierarchy successfully\n\n");
    
    // Count spheres
    uint32_t total = count_spheres(root);
    printf("Total spheres in hierarchy: %u\n", total);
    printf("Expected (12^0 + 12^1 + 12^2 + 12^3): %d\n\n", 
           1 + 12 + 144 + 1728);
    
    // Print first few levels
    printf("Hierarchy structure (first 2 levels):\n");
    print_hierarchy(root, 0);
    
    // Test nearest sphere finding
    printf("\n\nTesting nearest sphere finding:\n");
    uint64_t targets[] = {95, 100, 105, 150, 200};
    
    for (int i = 0; i < 5; i++) {
        uint64_t target = targets[i];
        KissingSphere* nearest = find_nearest_sphere(root, target);
        
        if (nearest) {
            printf("  Target %lu: nearest sphere at %lu (distance: %ld)\n",
                   target, nearest->center, 
                   (int64_t)nearest->center - (int64_t)target);
        }
    }
    
    // Test N nearest
    printf("\nFinding 5 nearest spheres to target 100:\n");
    KissingSphere* nearest[5];
    uint32_t found = find_n_nearest_spheres(root, 100, 5, nearest);
    
    for (uint32_t i = 0; i < found; i++) {
        printf("  %u. Sphere at %lu (distance: %ld)\n",
               i + 1, nearest[i]->center,
               (int64_t)nearest[i]->center - 100);
    }
    
    // Test anchor marking
    printf("\nMarking top 3 as anchors:\n");
    for (uint32_t i = 0; i < 3 && i < found; i++) {
        mark_as_anchor(nearest[i], 0.9 - i * 0.1);
        printf("  Marked sphere at %lu as anchor (confidence: %.2f)\n",
               nearest[i]->center, nearest[i]->confidence);
    }
    
    // Get all anchors
    KissingSphere* anchors[10];
    uint32_t num_anchors = get_all_anchors(root, anchors, 10);
    printf("\nTotal anchors in hierarchy: %u\n", num_anchors);
    
    free_kissing_sphere(root);
    printf("\n");
}

void test_geometric_filters(void) {
    printf("=== TEST 4: Geometric Filters ===\n\n");
    
    printf("Testing wheel/clock filter (mod 30):\n");
    uint64_t test_vals[] = {1, 7, 11, 13, 17, 19, 23, 29, 30, 31, 37};
    
    for (int i = 0; i < 11; i++) {
        uint64_t k = test_vals[i];
        uint32_t pos = wheel_position(k);
        bool viable = recursive_clock_check(k, 3);
        uint32_t qr = quadratic_residue_mod30(k);
        
        printf("  k=%lu: pos=%u, viable=%s, qr=%u\n",
               k, pos, viable ? "YES" : "NO", qr);
    }
    
    printf("\nTesting dynamic gate filter (kissing spheres):\n");
    uint64_t gate_tests[] = {100, 200, 300, 500, 1000, 2000};
    
    for (int i = 0; i < 6; i++) {
        uint64_t k = gate_tests[i];
        bool near = near_dynamic_gate(k, 3);
        
        printf("  k=%lu: near_gate=%s\n", k, near ? "YES" : "NO");
    }
    
    printf("\nTesting pre-filter (combined):\n");
    uint64_t filter_tests[] = {2, 3, 5, 7, 11, 13, 15, 17, 21, 23, 25, 29};
    
    for (int i = 0; i < 12; i++) {
        uint64_t k = filter_tests[i];
        bool pass = pre_filter_candidate(k);
        
        printf("  k=%lu: pre_filter=%s\n", k, pass ? "PASS" : "REJECT");
    }
    
    printf("\n");
}

void test_integration(void) {
    printf("=== TEST 5: Integration Test ===\n\n");
    
    printf("Generating primes and creating kissing sphere hierarchy:\n\n");
    
    // Generate some primes
    CrystalAbacus* abacus = abacus_create();
    if (!abacus) {
        printf("Failed to create abacus\n");
        return;
    }
    
    printf("Generating 10 primes...\n");
    for (int i = 0; i < 10; i++) {
        uint64_t prime = abacus_next_prime(abacus);
        printf("  Prime %2d: %lu\n", i + 1, prime);
    }
    
    // Use last prime as center for hierarchy
    uint64_t center = abacus->primes[abacus->num_primes - 1];
    
    printf("\nCreating kissing sphere hierarchy around prime %lu...\n", center);
    
    KissingSphere* root = create_kissing_sphere_hierarchy(center, 2);
    if (!root) {
        printf("Failed to create hierarchy\n");
        abacus_free(abacus);
        return;
    }
    
    uint32_t total = count_spheres(root);
    printf("Created hierarchy with %u spheres\n", total);
    
    // Find nearest spheres to other primes
    printf("\nFinding nearest spheres to other primes:\n");
    for (uint32_t i = 0; i < abacus->num_primes - 1 && i < 5; i++) {
        uint64_t target = abacus->primes[i];
        KissingSphere* nearest = find_nearest_sphere(root, target);
        
        if (nearest) {
            printf("  Prime %lu: nearest sphere at %lu (distance: %ld)\n",
                   target, nearest->center,
                   (int64_t)nearest->center - (int64_t)target);
        }
    }
    
    free_kissing_sphere(root);
    abacus_free(abacus);
    
    printf("\n");
}

int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 - Phase 1 Foundation Tests                  ║\n");
    printf("║  Crystal Abacus + Kissing Spheres Implementation          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    test_crystal_abacus();
    test_lattice_embedding();
    test_kissing_spheres();
    test_geometric_filters();
    test_integration();
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  All Phase 1 Foundation Tests Complete                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}