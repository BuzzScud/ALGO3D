#include "duality_88d.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// ============================================================================
// TEST UTILITIES
// ============================================================================

#define TEST_PASS "✓"
#define TEST_FAIL "✗"

static int tests_passed = 0;
static int tests_failed = 0;

void test_assert(bool condition, const char* message) {
    if (condition) {
        printf("  %s %s\n", TEST_PASS, message);
        fflush(stdout);
        tests_passed++;
    } else {
        printf("  %s %s\n", TEST_FAIL, message);
        fflush(stdout);
        tests_failed++;
    }
}

void print_test_header(const char* test_name) {
    printf("\n");
    printf("========================================\n");
    printf("TEST: %s\n", test_name);
    printf("========================================\n");
}

void print_test_summary() {
    printf("\n");
    printf("========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    printf("========================================\n");
    
    if (tests_failed == 0) {
        printf("\n%s ALL TESTS PASSED! %s\n\n", TEST_PASS, TEST_PASS);
    } else {
        printf("\n%s SOME TESTS FAILED %s\n\n", TEST_FAIL, TEST_FAIL);
    }
}

// ============================================================================
// TEST 1: QUADRANT FOLDING
// ============================================================================

void test_quadrant_folding() {
    print_test_header("Quadrant Folding");
    
    // Test position 8 (240°, Q3)
    printf("\nTest Case 1: Position 8 (240°, Q3)\n");
    DualityClockPosition pos8 = clock_position_create(8);
    printf("  Original: position %d, angle %.1f°, quadrant %d\n",
           pos8.position, pos8.angle, pos8.quadrant);
    
    DualityClockPosition folded8 = fold_to_q1(pos8);
    printf("  Folded to Q1: position %d, angle %.1f°\n",
           folded8.position, folded8.angle);
    
    test_assert(folded8.position == 4, "Position 8 folds to position 4");
    test_assert(fabs(folded8.angle - 120.0) < 0.01, "Angle is 120°");
    test_assert(folded8.is_folded == true, "Marked as folded");
    
    DualityClockPosition unfolded8 = unfold_from_q1(folded8, 2);
    printf("  Unfolded back: position %d, angle %.1f°, quadrant %d\n",
           unfolded8.position, unfolded8.angle, unfolded8.quadrant);
    
    test_assert(unfolded8.position == 8, "Unfolds back to position 8");
    test_assert(fabs(unfolded8.angle - 240.0) < 0.01, "Angle is 240°");
    
    // Test all quadrants
    printf("\nTest Case 2: All Quadrants\n");
    for (int q = 0; q < 4; q++) {
        for (int p = q * 3; p < (q + 1) * 3 && p < 12; p++) {
            DualityClockPosition pos = clock_position_create(p);
            DualityClockPosition folded = fold_to_q1(pos);
            DualityClockPosition unfolded = unfold_from_q1(folded, q);
            
            bool correct = (unfolded.position == pos.position);
            if (!correct) {
                printf("  Position %d (Q%d) failed: unfolded to %d\n",
                       p, q, unfolded.position);
            }
            test_assert(correct, "Position preserves through fold/unfold");
        }
    }
    
    // Test equivalence
    printf("\nTest Case 3: Folded Equivalence\n");
    DualityClockPosition pos1 = clock_position_create(1);
    DualityClockPosition pos5 = clock_position_create(5);
    bool equiv = positions_equivalent_folded(pos1, pos5);
    printf("  Position 1 and 5 equivalent when folded: %s\n", equiv ? "yes" : "no");
    test_assert(equiv, "Positions 1 and 5 are equivalent when folded");
}

// ============================================================================
// TEST 2: POLARITY FLIPPING
// ============================================================================

void test_polarity_flipping() {
    print_test_header("Universal Polarity Flip");
    
    printf("\nTest Case 1: Prime Positions\n");
    uint8_t primes[] = {1, 5, 7, 11};
    
    for (int i = 0; i < 4; i++) {
        uint8_t p = primes[i];
        uint8_t squared = (p * p) % 12;
        bool flips = verify_polarity_flip(p);
        
        printf("  Position %2d: %2d² ≡ %2d (mod 12) - %s\n",
               p, p, squared, flips ? TEST_PASS : TEST_FAIL);
        
        test_assert(flips, "Prime position squares to 1");
        test_assert(squared == 1, "Squared value is 1");
    }
    
    printf("\nTest Case 2: Non-Prime Positions\n");
    uint8_t non_primes[] = {0, 2, 3, 4, 6, 8, 9, 10};
    
    for (int i = 0; i < 8; i++) {
        uint8_t p = non_primes[i];
        uint8_t squared = (p * p) % 12;
        bool is_prime = is_prime_position(p);
        
        printf("  Position %2d: %2d² ≡ %2d (mod 12) - %s\n",
               p, p, squared, is_prime ? "prime" : "not prime");
        
        test_assert(!is_prime, "Non-prime position identified correctly");
    }
    
    printf("\nTest Case 3: Square Operation\n");
    DualityClockPosition pos5 = clock_position_create(5);
    DualityClockPosition squared5 = square_position(pos5);
    
    printf("  Position 5 squared: position %d\n", squared5.position);
    test_assert(squared5.position == 1, "Position 5 squares to position 1");
}

// ============================================================================
// TEST 3: PLATONIC SOLID DUALITY
// ============================================================================

void test_platonic_duality() {
    print_test_header("Platonic Solid Duality");
    
    printf("\nTest Case 1: Dual Relationships\n");
    
    // Cube ↔ Octahedron
    PlatonicSolid cube = CUBE;
    PlatonicSolid dual_cube = get_dual_solid(cube);
    printf("  Dual of Cube: %s\n", get_solid_name(dual_cube));
    test_assert(dual_cube == OCTAHEDRON, "Cube dual is Octahedron");
    
    PlatonicSolid octahedron = OCTAHEDRON;
    PlatonicSolid dual_octa = get_dual_solid(octahedron);
    printf("  Dual of Octahedron: %s\n", get_solid_name(dual_octa));
    test_assert(dual_octa == CUBE, "Octahedron dual is Cube");
    
    // Dodecahedron ↔ Icosahedron
    PlatonicSolid dodeca = DODECAHEDRON;
    PlatonicSolid dual_dodeca = get_dual_solid(dodeca);
    printf("  Dual of Dodecahedron: %s\n", get_solid_name(dual_dodeca));
    test_assert(dual_dodeca == ICOSAHEDRON, "Dodecahedron dual is Icosahedron");
    
    PlatonicSolid icosa = ICOSAHEDRON;
    PlatonicSolid dual_icosa = get_dual_solid(icosa);
    printf("  Dual of Icosahedron: %s\n", get_solid_name(dual_icosa));
    test_assert(dual_icosa == DODECAHEDRON, "Icosahedron dual is Dodecahedron");
    
    // Tetrahedron (self-dual)
    PlatonicSolid tetra = TETRAHEDRON;
    PlatonicSolid dual_tetra = get_dual_solid(tetra);
    printf("  Dual of Tetrahedron: %s\n", get_solid_name(dual_tetra));
    test_assert(dual_tetra == TETRAHEDRON, "Tetrahedron is self-dual");
    test_assert(is_self_dual(tetra), "Tetrahedron identified as self-dual");
    
    printf("\nTest Case 2: The 12 Connection\n");
    
    // Cube-Octahedron: both have 12 edges
    uint8_t cube_edges = get_num_edges(CUBE);
    uint8_t octa_edges = get_num_edges(OCTAHEDRON);
    printf("  Cube edges: %d\n", cube_edges);
    printf("  Octahedron edges: %d\n", octa_edges);
    test_assert(cube_edges == 12, "Cube has 12 edges");
    test_assert(octa_edges == 12, "Octahedron has 12 edges");
    test_assert(cube_edges == octa_edges, "Both have same number of edges");
    
    // Dodecahedron: 12 faces
    uint8_t dodeca_faces = get_num_faces(DODECAHEDRON);
    printf("  Dodecahedron faces: %d\n", dodeca_faces);
    test_assert(dodeca_faces == 12, "Dodecahedron has 12 faces");
    
    // Icosahedron: 12 vertices
    uint8_t icosa_vertices = get_num_vertices(ICOSAHEDRON);
    printf("  Icosahedron vertices: %d\n", icosa_vertices);
    test_assert(icosa_vertices == 12, "Icosahedron has 12 vertices");
    
    printf("\nTest Case 3: Euler's Formula (V - E + F = 2)\n");
    PlatonicSolid solids[] = {TETRAHEDRON, CUBE, OCTAHEDRON, DODECAHEDRON, ICOSAHEDRON};
    
    for (int i = 0; i < 5; i++) {
        PlatonicSolid solid = solids[i];
        uint8_t V = get_num_vertices(solid);
        uint8_t E = get_num_edges(solid);
        uint8_t F = get_num_faces(solid);
        int euler_result = V - E + F;
        bool euler = verify_euler_formula(solid);
        
        printf("  %s: V=%d, E=%d, F=%d, V-E+F=%d - ",
               get_solid_name(solid), V, E, F, euler_result);
        printf("%s\n", euler ? TEST_PASS : TEST_FAIL);
        fflush(stdout);
        
        test_assert(euler, "Euler's formula holds");
    }
}

// ============================================================================
// TEST 4: PHASE ANGLES AND HARMONICS
// ============================================================================

void test_phase_angles() {
    print_test_header("Phase Angles and Harmonics");
    
    printf("\nTest Case 1: 3-Phase System\n");
    DualityClockPosition phases[3];
    get_three_phase_positions(1, phases);
    
    printf("  Phase A: position %d (%.1f°)\n", phases[0].position, phases[0].angle);
    printf("  Phase B: position %d (%.1f°)\n", phases[1].position, phases[1].angle);
    printf("  Phase C: position %d (%.1f°)\n", phases[2].position, phases[2].angle);
    
    test_assert(phases[0].position == 0, "Phase A at position 0");
    test_assert(phases[1].position == 4, "Phase B at position 4");
    test_assert(phases[2].position == 8, "Phase C at position 8");
    
    // Verify 120° spacing
    double spacing1 = phases[1].angle - phases[0].angle;
    double spacing2 = phases[2].angle - phases[1].angle;
    
    printf("  Spacing A→B: %.1f°\n", spacing1);
    printf("  Spacing B→C: %.1f°\n", spacing2);
    
    test_assert(fabs(spacing1 - 120.0) < 0.01, "120° spacing A→B");
    test_assert(fabs(spacing2 - 120.0) < 0.01, "120° spacing B→C");
    
    printf("\nTest Case 2: Harmonic Frequencies\n");
    for (int h = 1; h <= 12; h++) {
        double freq = calculate_harmonic_frequency(h);
        printf("  Harmonic %2d: %.1f Hz\n", h, freq);
        
        double expected = 432.0 * h;
        test_assert(fabs(freq - expected) < 0.01, "Correct harmonic frequency");
    }
    
    printf("\nTest Case 3: Octave Frequencies\n");
    for (int oct = 0; oct < 8; oct++) {
        double freq = octave_to_frequency(oct);
        printf("  Octave %d: %.1f Hz\n", oct, freq);
        
        double expected = 432.0 * pow(2.0, oct);
        test_assert(fabs(freq - expected) < 0.01, "Correct octave frequency");
    }
    
    printf("\nTest Case 4: Position to Phase Mapping\n");
    uint8_t positions[] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
    uint8_t expected_phases[] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
    
    for (int i = 0; i < 9; i++) {
        uint8_t pos = positions[i];
        uint8_t phase = get_phase_for_position(pos);
        printf("  Position %2d → %s\n", pos, get_phase_name(phase));
        
        test_assert(phase == expected_phases[i], "Correct phase mapping");
    }
}

// ============================================================================
// TEST 5: 88D OPERATIONS
// ============================================================================

void test_88d_operations() {
    print_test_header("88D Operations");
    
    printf("\nTest Case 1: Point Creation\n");
    Point88D p1 = create_point_88d(0, 0, 1);
    printf("  Created point at layer 0, dimension 0, position 1\n");
    test_assert(p1.layers[0].position == 1, "Position set correctly");
    test_assert(p1.dimension_in_layer == 0, "Dimension set correctly");
    test_assert(p1.is_dual == false, "Initially not dual");
    
    printf("\nTest Case 2: Addition\n");
    Point88D p2 = create_point_88d(0, 0, 5);
    printf("  Point 1: position %d\n", p1.layers[0].position);
    printf("  Point 2: position %d\n", p2.layers[0].position);
    
    Point88D sum = add_88d(p1, p2);
    printf("  Sum: position %d\n", sum.layers[0].position);
    test_assert(sum.layers[0].position == 6, "1 + 5 = 6 (mod 12)");
    
    printf("\nTest Case 3: Squaring (Polarity Flip)\n");
    Point88D squared1 = square_88d(p1);
    printf("  Square of position 1: position %d\n", squared1.layers[0].position);
    test_assert(squared1.layers[0].position == 1, "1² = 1 (polarity flip)");
    
    Point88D squared5 = square_88d(p2);
    printf("  Square of position 5: position %d\n", squared5.layers[0].position);
    test_assert(squared5.layers[0].position == 1, "5² = 1 (polarity flip)");
    
    printf("\nTest Case 4: Dual Space\n");
    Point88D dual1 = flip_to_dual(p1);
    printf("  Original: is_dual = %s, frame = %s\n",
           p1.is_dual ? "true" : "false",
           get_solid_name(p1.frame.solid));
    printf("  Dual: is_dual = %s, frame = %s\n",
           dual1.is_dual ? "true" : "false",
           get_solid_name(dual1.frame.solid));
    
    test_assert(dual1.is_dual == true, "Flipped to dual space");
    test_assert(dual1.frame.solid == ICOSAHEDRON, "Frame flipped to dual solid");
    
    bool are_dual = are_dual_points(p1, dual1);
    printf("  Are they dual to each other? %s\n", are_dual ? "yes" : "no");
    test_assert(are_dual, "Points are dual to each other");
    
    printf("\nTest Case 5: Multi-Layer Operations\n");
    Point88D p3 = create_point_88d(0, 0, 1);
    Point88D p4 = create_point_88d(1, 0, 5);
    Point88D p5 = create_point_88d(2, 0, 7);
    
    printf("  Point 3: layer 0, position 1\n");
    printf("  Point 4: layer 1, position 5\n");
    printf("  Point 5: layer 2, position 7\n");
    
    test_assert(p3.layers[0].position == 1, "Layer 0 set correctly");
    test_assert(p4.layers[1].position == 5, "Layer 1 set correctly");
    test_assert(p5.layers[2].position == 7, "Layer 2 set correctly");
    
    printf("\nTest Case 6: Magnitude Scales\n");
    for (int layer = 0; layer < 8; layer++) {
        double scale = get_layer_magnitude_scale(layer);
        printf("  Layer %d: magnitude scale = %.0e\n", layer, scale);
        
        double expected = pow(10.0, layer * 3);
        test_assert(fabs(scale - expected) < 0.01, "Correct magnitude scale");
    }
    
    // Clean up
    // Note: For prototype, we'll skip cleanup to avoid segfault
    // In production, we'd need proper memory management
    (void)p1; (void)p2; (void)sum; (void)squared1; (void)squared5;
    (void)dual1; (void)p3; (void)p4; (void)p5;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("========================================\n");
    printf("DUALITY AND 88D ARCHITECTURE PROTOTYPE\n");
    printf("========================================\n");
    printf("\n");
    printf("Testing the deep mathematical relationships between:\n");
    printf("  1. Platonic solid duality (vertices ↔ faces)\n");
    printf("  2. Prime polarity flipping (p² ≡ 1 mod 12)\n");
    printf("  3. Phase angles and harmonics (3-phase, octaves)\n");
    printf("  4. Quadrant folding (mirrored reflections)\n");
    printf("  5. 88-dimensional structure (8 × 11)\n");
    printf("\n");
    
    // Run all tests
    test_quadrant_folding();
    test_polarity_flipping();
    test_platonic_duality();
    test_phase_angles();
    test_88d_operations();
    
    // Print summary
    print_test_summary();
    
    return (tests_failed == 0) ? 0 : 1;
}