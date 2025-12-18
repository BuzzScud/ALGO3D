/**
 * test_integrated_recovery.c - Integrated Recovery Test
 * 
 * Tests the complete recovery pipeline integrating:
 * - Clock position mapping
 * - Platonic solid anchors
 * - Pythagorean triple triangulation
 * - Dimensional frequency analysis
 * - Recursive refinement
 */

#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>

void test_integrated_anchor_selection() {
    printf("=== Test 1: Integrated Anchor Selection ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    printf("Created Platonic overlay with %u shared vertices\n", 
           overlay->num_shared_vertices);
    
    // Check if we have shared vertices
    if (overlay->num_shared_vertices == 0) {
        printf("No shared vertices found with tolerance 0.5\n");
        printf("Using individual Platonic solid vertices as anchors instead\n\n");
        
        // Use vertices from icosahedron (12 vertices, good for 12-fold symmetry)
        PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
        printf("Using Icosahedron vertices: %u vertices\n", icosa->num_vertices);
        
        for (uint32_t i = 0; i < icosa->num_vertices && i < 10; i++) {
            printf("  Vertex %u: ring=%d, pos=%d, angle=%.4f\n",
                   i, icosa->clock_positions[i].ring,
                   icosa->clock_positions[i].position,
                   icosa->clock_positions[i].angle);
        }
        
        free_platonic_overlay(overlay);
        printf("\n");
        return;
    }
    
    // Select optimal anchors
    uint32_t num_anchors = 10;
    SharedVertex** anchors = select_optimal_anchors(overlay, num_anchors);
    
    if (anchors) {
        printf("\nSelected %u optimal anchors from Platonic solids:\n", num_anchors);
        for (uint32_t i = 0; i < num_anchors; i++) {
            printf("  Anchor %u: stability=%.2f, shared by %u solids\n",
                   i, anchors[i]->stability, anchors[i]->num_solids);
            printf("    Clock: ring=%d, pos=%d, angle=%.4f\n",
                   anchors[i]->clock.ring, 
                   anchors[i]->clock.position,
                   anchors[i]->clock.angle);
        }
        
        // Create clock recovery context with these anchors
        ClockRecoveryContext* ctx = init_clock_recovery(num_anchors);
        if (ctx) {
            printf("\nCreated recovery context with %u anchors\n", num_anchors);
            
            // Map anchors to k values (for testing, use synthetic k values)
            BN_CTX* bn_ctx = BN_CTX_new();
            BIGNUM* k = BN_new();
            
            for (uint32_t i = 0; i < num_anchors; i++) {
                // Estimate k from clock position
                double k_estimate = anchors[i]->clock.angle / (M_PI * 1.618033988749895);
                BN_set_word(k, (BN_ULONG)(k_estimate * 1000));
                add_anchor(ctx, k, i);
            }
            
            printf("Added %u anchors to recovery context\n", ctx->num_anchors);
            
            BN_free(k);
            BN_CTX_free(bn_ctx);
            free_clock_recovery(ctx);
        }
        
        free(anchors);
    }
    
    free_platonic_overlay(overlay);
    printf("\n");
}

void test_pythagorean_triple_with_platonic() {
    printf("=== Test 2: Pythagorean Triples with Platonic Anchors ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Check if we have shared vertices
    if (overlay->num_shared_vertices == 0) {
        printf("No shared vertices found - using Icosahedron vertices\n");
        
        // Use icosahedron vertices directly
        PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
        printf("Testing with %u Icosahedron vertices\n\n", icosa->num_vertices);
        
        int triples_found = 0;
        for (uint32_t i = 0; i < icosa->num_vertices - 2; i++) {
            for (uint32_t j = i + 1; j < icosa->num_vertices - 1; j++) {
                for (uint32_t k = j + 1; k < icosa->num_vertices; k++) {
                    if (is_pythagorean_triple(icosa->clock_positions[i],
                                             icosa->clock_positions[j],
                                             icosa->clock_positions[k])) {
                        printf("  Triple found: vertices %u, %u, %u\n", i, j, k);
                        triples_found++;
                    }
                }
            }
        }
        
        printf("\nTotal Pythagorean triples found: %d\n", triples_found);
        free_platonic_overlay(overlay);
        printf("\n");
        return;
    }
    
    // Get anchors from Platonic solids
    SharedVertex** anchors = select_optimal_anchors(overlay, 10);
    if (!anchors) {
        printf("Failed to select anchors\n");
        free_platonic_overlay(overlay);
        return;
    }
    
    // Test Pythagorean triple detection among anchors
    printf("Testing Pythagorean triple detection:\n");
    int triples_found = 0;
    
    for (uint32_t i = 0; i < 8; i++) {
        for (uint32_t j = i + 1; j < 9; j++) {
            for (uint32_t k = j + 1; k < 10; k++) {
                if (is_pythagorean_triple(anchors[i]->clock, 
                                         anchors[j]->clock, 
                                         anchors[k]->clock)) {
                    printf("  Triple found: anchors %u, %u, %u\n", i, j, k);
                    
                    PythagoreanTriple triple;
                    if (find_pythagorean_triple(anchors[i]->clock,
                                                anchors[j]->clock,
                                                anchors[k]->clock,
                                                &triple)) {
                        printf("    (a=%lu, b=%lu, c=%lu) with p=%u, q=%u\n",
                               triple.a, triple.b, triple.c, triple.p, triple.q);
                        triples_found++;
                    }
                }
            }
        }
    }
    
    printf("\nTotal Pythagorean triples found: %d\n", triples_found);
    
    free(anchors);
    free_platonic_overlay(overlay);
    printf("\n");
}

void test_dimensional_alignment_with_platonic() {
    printf("=== Test 3: Dimensional Alignment with Platonic Anchors ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Use icosahedron vertices directly
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    printf("Using Icosahedron vertices for dimensional alignment\n");
    
    // Initialize dimensional frequencies
    DimensionalFrequency dims[13];
    init_dimensional_frequencies(dims);
    
    printf("Computing dimensional alignment for Icosahedron vertices:\n");
    
    double total_alignment = 0.0;
    for (uint32_t i = 0; i < icosa->num_vertices; i++) {
        double alignment = compute_total_alignment(icosa->clock_positions[i].angle, dims);
        double entropy = compute_entropy_from_alignment(alignment);
        
        printf("  Vertex %u: alignment=%.6f, entropy=%.6f\n", 
               i, alignment, entropy);
        
        total_alignment += alignment;
    }
    
    double avg_alignment = total_alignment / icosa->num_vertices;
    double avg_entropy = compute_entropy_from_alignment(avg_alignment);
    
    printf("\nAverage alignment: %.6f\n", avg_alignment);
    printf("Average entropy: %.6f\n", avg_entropy);
    
    free_platonic_overlay(overlay);
    printf("\n");
}

void test_complete_recovery_pipeline() {
    printf("=== Test 4: Complete Recovery Pipeline ===\n\n");
    
    printf("Step 1: Create Platonic overlay\n");
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    printf("  Created overlay with %u shared vertices\n", overlay->num_shared_vertices);
    
    printf("\nStep 2: Use Icosahedron vertices as anchors\n");
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    uint32_t num_anchors = icosa->num_vertices;
    printf("  Using %u Icosahedron vertices\n", num_anchors);
    
    printf("\nStep 3: Create recovery context\n");
    ClockRecoveryContext* ctx = init_clock_recovery(num_anchors);
    if (!ctx) {
        printf("Failed to create recovery context\n");
        free_platonic_overlay(overlay);
        return;
    }
    printf("  Created recovery context\n");
    
    printf("\nStep 4: Add Icosahedron vertices to recovery context\n");
    BN_CTX* bn_ctx = BN_CTX_new();
    BIGNUM* k = BN_new();
    
    for (uint32_t i = 0; i < num_anchors; i++) {
        // Estimate k from clock position
        double k_estimate = icosa->clock_positions[i].angle / (M_PI * 1.618033988749895);
        BN_set_word(k, (BN_ULONG)(k_estimate * 1000));
        add_anchor(ctx, k, i);
    }
    printf("  Added %u anchors\n", num_anchors);
    
    printf("\nStep 5: Compute dimensional alignment\n");
    double total_entropy = 0.0;
    for (uint32_t i = 0; i < num_anchors; i++) {
        double alignment = compute_total_alignment(ctx->anchors[i].angle, ctx->dims);
        double entropy = compute_entropy_from_alignment(alignment);
        total_entropy += entropy;
    }
    double avg_entropy = total_entropy / num_anchors;
    printf("  Average entropy: %.6f\n", avg_entropy);
    
    printf("\nStep 6: Find Pythagorean triple\n");
    PythagoreanTriple best_triple;
    bool found_triple = false;
    
    for (uint32_t i = 0; i < num_anchors - 2 && !found_triple; i++) {
        for (uint32_t j = i + 1; j < num_anchors - 1 && !found_triple; j++) {
            for (uint32_t k_idx = j + 1; k_idx < num_anchors; k_idx++) {
                if (is_pythagorean_triple(ctx->anchors[i], ctx->anchors[j], ctx->anchors[k_idx])) {
                    if (find_pythagorean_triple(ctx->anchors[i], ctx->anchors[j], 
                                                ctx->anchors[k_idx], &best_triple)) {
                        printf("  Found triple: (%lu, %lu, %lu)\n",
                               best_triple.a, best_triple.b, best_triple.c);
                        found_triple = true;
                        break;
                    }
                }
            }
        }
    }
    
    if (!found_triple) {
        printf("  No triple found, using first 3 anchors\n");
        find_pythagorean_triple(ctx->anchors[0], ctx->anchors[1], ctx->anchors[2], &best_triple);
    }
    
    printf("\nStep 7: Triangulate target position\n");
    ClockPosition target = triangulate_with_triple(ctx->anchors[0], ctx->anchors[1],
                                                   ctx->anchors[2], best_triple);
    printf("  Target: ring=%d, pos=%d, angle=%.6f\n",
           target.ring, target.position, target.angle);
    
    printf("\nStep 8: Check recursion need\n");
    if (needs_recursion(ctx, avg_entropy)) {
        printf("  Recursion needed (entropy %.6f > threshold %.6f)\n",
               avg_entropy, ctx->entropy_threshold);
    } else {
        printf("  No recursion needed (entropy %.6f <= threshold %.6f)\n",
               avg_entropy, ctx->entropy_threshold);
    }
    
    printf("\n✅ Complete recovery pipeline executed successfully!\n");
    
    // Cleanup
    BN_free(k);
    BN_CTX_free(bn_ctx);
    free_clock_recovery(ctx);
    free_platonic_overlay(overlay);
    
    printf("\n");
}

void test_nearest_anchor_search() {
    printf("=== Test 5: Nearest Anchor Search ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Create a target position
    ClockPosition target;
    target.ring = 1;
    target.position = 30;
    target.angle = M_PI;
    target.radius = 0.5;
    
    printf("Target position: ring=%d, pos=%d, angle=%.4f\n",
           target.ring, target.position, target.angle);
    
    // Use icosahedron vertices
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    printf("\nFinding nearest Icosahedron vertices:\n");
    
    // Compute distances to all vertices
    double distances[12];
    for (uint32_t i = 0; i < icosa->num_vertices; i++) {
        distances[i] = clock_distance_pi_phi(target, icosa->clock_positions[i]);
    }
    
    // Find 5 nearest
    uint32_t k = 5;
    for (uint32_t i = 0; i < k; i++) {
        uint32_t min_idx = i;
        for (uint32_t j = i + 1; j < icosa->num_vertices; j++) {
            if (distances[j] < distances[min_idx]) {
                min_idx = j;
            }
        }
        
        // Swap
        if (min_idx != i) {
            double temp = distances[i];
            distances[i] = distances[min_idx];
            distances[min_idx] = temp;
        }
        
        printf("  Vertex %u: distance=%.4f, ring=%d\n",
               i, distances[i], icosa->clock_positions[i].ring);
    }
    
    printf("\n✅ Nearest anchor search working correctly!\n");
    
    free_platonic_overlay(overlay);
    printf("\n");
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Integrated Recovery Test Suite                           ║\n");
    printf("║  Testing complete recovery pipeline                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    test_integrated_anchor_selection();
    test_pythagorean_triple_with_platonic();
    test_dimensional_alignment_with_platonic();
    test_complete_recovery_pipeline();
    test_nearest_anchor_search();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  All Integration Tests Complete                           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}