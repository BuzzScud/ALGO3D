/**
 * test_platonic_solids.c - Test Platonic Solid Generation and Overlay
 * 
 * Tests all 5 Platonic solids and their overlay in 13D space
 */

#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void test_individual_solids() {
    printf("=== Test 1: Individual Platonic Solids ===\n\n");
    
    const char* names[] = {"Tetrahedron", "Cube", "Octahedron", "Dodecahedron", "Icosahedron"};
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        PlatonicSolid* solid = create_platonic_solid((PlatonicSolidType)i);
        if (!solid) {
            printf("Failed to create %s\n", names[i]);
            continue;
        }
        
        printf("%s:\n", solid->name);
        printf("  Vertices: %u\n", solid->num_vertices);
        printf("  Edges: %u\n", solid->num_edges);
        printf("  Faces: %u\n", solid->num_faces);
        
        // Verify Euler's formula
        bool euler_ok = verify_euler_formula(solid);
        printf("  Euler's formula (V-E+F=2): %s\n", euler_ok ? "✓" : "✗");
        
        // Show first vertex
        printf("  First vertex (13D): [%.3f, %.3f, %.3f, ...]\n",
               solid->vertices[0], solid->vertices[1], solid->vertices[2]);
        
        // Show clock position
        printf("  First vertex clock: ring=%d, pos=%d, angle=%.3f\n",
               solid->clock_positions[0].ring,
               solid->clock_positions[0].position,
               solid->clock_positions[0].angle);
        
        printf("\n");
        
        free_platonic_solid(solid);
    }
}

void test_dual_relationships() {
    printf("=== Test 2: Dual Relationships ===\n\n");
    
    const char* names[] = {"Tetrahedron", "Cube", "Octahedron", "Dodecahedron", "Icosahedron"};
    
    printf("Dual pairs:\n");
    printf("  Tetrahedron <-> %s\n", names[get_dual_solid(SOLID_TETRAHEDRON)]);
    printf("  Cube <-> %s\n", names[get_dual_solid(SOLID_CUBE)]);
    printf("  Dodecahedron <-> %s\n", names[get_dual_solid(SOLID_DODECAHEDRON)]);
    
    printf("\n");
}

void test_platonic_overlay() {
    printf("=== Test 3: Platonic Overlay ===\n\n");
    
    // Create overlay with tolerance 0.1
    PlatonicOverlay* overlay = create_platonic_overlay(0.1);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    printf("Created overlay of all 5 Platonic solids\n");
    printf("Tolerance: %.3f\n\n", overlay->tolerance);
    
    // Print each solid
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        printf("%s: %u vertices\n", 
               overlay->solids[i].name,
               overlay->solids[i].num_vertices);
    }
    
    printf("\nShared vertices found: %u\n", overlay->num_shared_vertices);
    
    // Show first few shared vertices
    printf("\nFirst 10 shared vertices:\n");
    for (uint32_t i = 0; i < 10 && i < overlay->num_shared_vertices; i++) {
        SharedVertex* sv = &overlay->shared_vertices[i];
        printf("  Vertex %u: shared by %u solids, stability=%.2f\n",
               i, sv->num_solids, sv->stability);
        printf("    Solids: ");
        for (uint32_t j = 0; j < sv->num_solids; j++) {
            printf("%s ", overlay->solids[sv->solid_ids[j]].name);
        }
        printf("\n");
        printf("    Clock: ring=%d, pos=%d\n", sv->clock.ring, sv->clock.position);
    }
    
    printf("\n");
    
    free_platonic_overlay(overlay);
}

void test_overlay_statistics() {
    printf("=== Test 4: Overlay Statistics ===\n\n");
    
    PlatonicOverlay* overlay = create_platonic_overlay(0.1);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    OverlayStatistics stats = compute_overlay_statistics(overlay);
    print_overlay_statistics(&stats);
    
    // Verify expected ranges
    printf("Validation:\n");
    printf("  Total vertices = 50 (4+8+6+20+12): %s\n",
           stats.total_vertices == 50 ? "✓" : "✗");
    printf("  Shared vertices > 0: %s\n",
           stats.shared_vertices > 0 ? "✓" : "✗");
    printf("  Average stability > 0: %s\n",
           stats.avg_stability > 0.0 ? "✓" : "✗");
    
    printf("\n");
    
    free_platonic_overlay(overlay);
}

void test_optimal_anchor_selection() {
    printf("=== Test 5: Optimal Anchor Selection ===\n\n");
    
    PlatonicOverlay* overlay = create_platonic_overlay(0.1);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Select top 10 most stable anchors
    uint32_t num_anchors = 10;
    SharedVertex** anchors = select_optimal_anchors(overlay, num_anchors);
    
    if (anchors) {
        printf("Selected %u optimal anchors:\n", num_anchors);
        for (uint32_t i = 0; i < num_anchors; i++) {
            printf("  Anchor %u: stability=%.2f, shared by %u solids\n",
                   i, anchors[i]->stability, anchors[i]->num_solids);
        }
        printf("\n");
        
        // Verify they're sorted by stability
        bool sorted = true;
        for (uint32_t i = 1; i < num_anchors; i++) {
            if (anchors[i]->stability > anchors[i-1]->stability) {
                sorted = false;
                break;
            }
        }
        printf("Anchors sorted by stability: %s\n", sorted ? "✓" : "✗");
        
        free(anchors);
    }
    
    printf("\n");
    
    free_platonic_overlay(overlay);
}

void test_nearest_vertices() {
    printf("=== Test 6: Nearest Shared Vertices ===\n\n");
    
    PlatonicOverlay* overlay = create_platonic_overlay(0.1);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Create a target clock position
    ClockPosition target;
    target.ring = 1;
    target.position = 30;
    target.angle = M_PI;
    target.radius = 0.5;
    
    printf("Target clock position: ring=%d, pos=%d, angle=%.3f\n",
           target.ring, target.position, target.angle);
    
    // Find 5 nearest shared vertices
    uint32_t k = 5;
    SharedVertex** nearest = find_nearest_shared_vertices(overlay, target, k);
    
    if (nearest) {
        printf("\n%u nearest shared vertices:\n", k);
        for (uint32_t i = 0; i < k; i++) {
            double dist = clock_distance_pi_phi(target, nearest[i]->clock);
            printf("  Vertex %u: distance=%.4f, stability=%.2f\n",
                   i, dist, nearest[i]->stability);
            printf("    Clock: ring=%d, pos=%d\n",
                   nearest[i]->clock.ring, nearest[i]->clock.position);
        }
        
        // Verify they're sorted by distance
        bool sorted = true;
        for (uint32_t i = 1; i < k; i++) {
            double d1 = clock_distance_pi_phi(target, nearest[i-1]->clock);
            double d2 = clock_distance_pi_phi(target, nearest[i]->clock);
            if (d2 < d1) {
                sorted = false;
                break;
            }
        }
        printf("\nVertices sorted by distance: %s\n", sorted ? "✓" : "✗");
        
        free(nearest);
    }
    
    printf("\n");
    
    free_platonic_overlay(overlay);
}

void test_13d_geometry() {
    printf("=== Test 7: 13D Geometry ===\n\n");
    
    // Create two test vertices
    double v1[13] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double v2[13] = {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double v3[13] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    // Test distance computation
    double d12 = compute_13d_distance(v1, v2);
    double d13 = compute_13d_distance(v1, v3);
    
    printf("Distance between v1 and v2: %.4f\n", d12);
    printf("Distance between v1 and v3: %.4f\n", d13);
    printf("Expected: d12 ≈ √2 = 1.414, d13 = 0\n");
    
    // Test vertex matching
    bool match12 = vertices_match(v1, v2, 0.1);
    bool match13 = vertices_match(v1, v3, 0.1);
    
    printf("\nVertex matching (tolerance=0.1):\n");
    printf("  v1 matches v2: %s (expected: no)\n", match12 ? "yes" : "no");
    printf("  v1 matches v3: %s (expected: yes)\n", match13 ? "yes" : "no");
    
    printf("\n");
}

void test_ring_mapping() {
    printf("=== Test 8: Ring Mapping ===\n\n");
    
    PlatonicOverlay* overlay = create_platonic_overlay(0.1);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Count vertices per ring for each solid
    printf("Vertices per ring:\n");
    for (int s = 0; s < NUM_PLATONIC_SOLIDS; s++) {
        PlatonicSolid* solid = &overlay->solids[s];
        int ring_counts[4] = {0, 0, 0, 0};
        
        for (uint32_t v = 0; v < solid->num_vertices; v++) {
            int ring = solid->clock_positions[v].ring;
            if (ring >= 0 && ring < 4) {
                ring_counts[ring]++;
            }
        }
        
        printf("  %s: R0=%d, R1=%d, R2=%d, R3=%d\n",
               solid->name, ring_counts[0], ring_counts[1], 
               ring_counts[2], ring_counts[3]);
    }
    
    printf("\n");
    
    free_platonic_overlay(overlay);
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Platonic Solids Test Suite                               ║\n");
    printf("║  Testing 5 Platonic solids in 13D space                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    test_individual_solids();
    test_dual_relationships();
    test_platonic_overlay();
    test_overlay_statistics();
    test_optimal_anchor_selection();
    test_nearest_vertices();
    test_13d_geometry();
    test_ring_mapping();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  All Tests Complete                                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}