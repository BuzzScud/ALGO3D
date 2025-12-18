/*
 * Platonic Solids Demonstration
 * 
 * This program demonstrates:
 * 1. Generation of all 5 Platonic solids using the math library
 * 2. Duality relationships (cube↔octahedron, dodecahedron↔icosahedron)
 * 3. The 12 Invariant: Number 12 appears in all dual pairs
 * 4. Coordinate frame generation for each solid
 * 5. Verification of Euler's formula: V - E + F = 2
 * 
 * Compilation:
 *   gcc -o platonic_demo platonic_demo.c -I../../math/include -L../../ -lcrystallinemath -lm
 * 
 * Usage:
 *   ./platonic_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "math/platonic_generator.h"
#include "math/polytope.h"

// ANSI color codes for pretty output
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// Structure to hold Platonic solid information
typedef struct {
    const char* name;
    const char* schlafli;
    const char* dual_name;
    int expected_vertices;
    int expected_edges;
    int expected_faces;
} PlatonicInfo;

// The 5 Platonic solids with their properties
static const PlatonicInfo PLATONIC_SOLIDS[] = {
    {"Tetrahedron", "{3,3}", "Tetrahedron (self-dual)", 4, 6, 4},
    {"Cube", "{4,3}", "Octahedron", 8, 12, 6},
    {"Octahedron", "{3,4}", "Cube", 6, 12, 8},
    {"Dodecahedron", "{5,3}", "Icosahedron", 20, 30, 12},
    {"Icosahedron", "{3,5}", "Dodecahedron", 12, 30, 20}
};

#define NUM_PLATONIC_SOLIDS 5

/**
 * Print a section header
 */
void print_header(const char* title) {
    printf("\n%s", COLOR_BOLD);
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("%s", COLOR_RESET);
}

/**
 * Print a subsection header
 */
void print_subheader(const char* title) {
    printf("\n%s%s───────────────────────────────────────────────────────────────────\n", 
           COLOR_CYAN, COLOR_BOLD);
    printf("  %s\n", title);
    printf("───────────────────────────────────────────────────────────────────\n%s", 
           COLOR_RESET);
}

/**
 * Verify Euler's formula: V - E + F = 2
 */
int verify_euler_formula(const PlatonicSolid* solid, const char* name) {
    int euler_characteristic = solid->num_vertices - solid->num_edges + solid->num_faces;
    
    printf("%s%s:%s V=%d, E=%d, F=%d\n", 
           COLOR_YELLOW, name, COLOR_RESET,
           solid->num_vertices, solid->num_edges, solid->num_faces);
    
    printf("  Euler's formula: V - E + F = %d - %d + %d = %s%d%s\n",
           solid->num_vertices, solid->num_edges, solid->num_faces,
           euler_characteristic == 2 ? COLOR_GREEN : COLOR_RED,
           euler_characteristic,
           COLOR_RESET);
    
    if (euler_characteristic == 2) {
        printf("  %s✓ Euler's formula verified!%s\n", COLOR_GREEN, COLOR_RESET);
        return 1;
    } else {
        printf("  %s✗ Euler's formula failed!%s\n", COLOR_RED, COLOR_RESET);
        return 0;
    }
}

/**
 * Print the first few vertices of a solid
 */
void print_vertices(const PlatonicSolid* solid, int max_vertices) {
    if (solid->vertex_coords == NULL) {
        printf("\n  Vertices not computed\n");
        return;
    }
    
    int count = (int)solid->num_vertices < max_vertices ? (int)solid->num_vertices : max_vertices;
    
    printf("\n  First %d vertices:\n", count);
    for (int i = 0; i < count; i++) {
        printf("    v%d: (%.4f, %.4f, %.4f)\n", 
               i,
               solid->vertex_coords[i * solid->dimension + 0],
               solid->vertex_coords[i * solid->dimension + 1],
               solid->vertex_coords[i * solid->dimension + 2]);
    }
    
    if ((int)solid->num_vertices > max_vertices) {
        printf("    ... (%d more vertices)\n", (int)solid->num_vertices - max_vertices);
    }
}

/**
 * Demonstrate The 12 Invariant for dual pairs
 */
void demonstrate_12_invariant(const PlatonicSolid* solid1, const PlatonicSolid* solid2,
                               const char* name1, const char* name2) {
    print_subheader("The 12 Invariant");
    
    printf("Duality relationship: %s%s ↔ %s%s\n\n", 
           COLOR_MAGENTA, name1, name2, COLOR_RESET);
    
    // Show the duality properties
    printf("  %s:%s\n", name1, COLOR_RESET);
    printf("    Vertices: %d\n", solid1->num_vertices);
    printf("    Edges:    %s%d%s\n", COLOR_YELLOW, solid1->num_edges, COLOR_RESET);
    printf("    Faces:    %d\n", solid1->num_faces);
    
    printf("\n  %s:%s\n", name2, COLOR_RESET);
    printf("    Vertices: %d\n", solid2->num_vertices);
    printf("    Edges:    %s%d%s\n", COLOR_YELLOW, solid2->num_edges, COLOR_RESET);
    printf("    Faces:    %d\n", solid2->num_faces);
    
    // Verify duality: vertices of one = faces of dual
    int duality_check = (solid1->num_vertices == solid2->num_faces) &&
                        (solid1->num_faces == solid2->num_vertices) &&
                        (solid1->num_edges == solid2->num_edges);
    
    printf("\n  Duality verification:\n");
    printf("    V₁ = F₂? %d = %d %s%s%s\n", 
           solid1->num_vertices, solid2->num_faces,
           solid1->num_vertices == solid2->num_faces ? COLOR_GREEN : COLOR_RED,
           solid1->num_vertices == solid2->num_faces ? "✓" : "✗",
           COLOR_RESET);
    printf("    F₁ = V₂? %d = %d %s%s%s\n", 
           solid1->num_faces, solid2->num_vertices,
           solid1->num_faces == solid2->num_vertices ? COLOR_GREEN : COLOR_RED,
           solid1->num_faces == solid2->num_vertices ? "✓" : "✗",
           COLOR_RESET);
    printf("    E₁ = E₂? %d = %d %s%s%s\n", 
           solid1->num_edges, solid2->num_edges,
           solid1->num_edges == solid2->num_edges ? COLOR_GREEN : COLOR_RED,
           solid1->num_edges == solid2->num_edges ? "✓" : "✗",
           COLOR_RESET);
    
    // The 12 Invariant
    if (solid1->num_edges == 12 || solid1->num_faces == 12 || solid1->num_vertices == 12 ||
        solid2->num_edges == 12 || solid2->num_faces == 12 || solid2->num_vertices == 12) {
        printf("\n  %s★ THE 12 INVARIANT:%s\n", COLOR_GREEN, COLOR_RESET);
        printf("    The number 12 appears in this dual pair!\n");
        
        if (solid1->num_edges == 12) {
            printf("    • %s has %s12 edges%s (invariant under duality)\n", 
                   name1, COLOR_YELLOW, COLOR_RESET);
        }
        if (solid1->num_faces == 12) {
            printf("    • %s has %s12 faces%s ↔ %s has 12 vertices\n", 
                   name1, COLOR_YELLOW, COLOR_RESET, name2);
        }
        if (solid1->num_vertices == 12) {
            printf("    • %s has %s12 vertices%s ↔ %s has 12 faces\n", 
                   name1, COLOR_YELLOW, COLOR_RESET, name2);
        }
    }
    
    if (duality_check) {
        printf("\n  %s✓ Duality relationship confirmed!%s\n", COLOR_GREEN, COLOR_RESET);
    } else {
        printf("\n  %s✗ Duality relationship failed!%s\n", COLOR_RED, COLOR_RESET);
    }
}

/**
 * Main demonstration
 */
int main(void) {
    PlatonicSolid* solids[NUM_PLATONIC_SOLIDS] = {NULL};
    int success_count = 0;
    int euler_count = 0;
    
    print_header("PLATONIC SOLIDS DEMONSTRATION");
    
    printf("\n%sThis demonstration shows:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("  1. Generation of all 5 Platonic solids\n");
    printf("  2. Verification of Euler's formula (V - E + F = 2)\n");
    printf("  3. Duality relationships between solids\n");
    printf("  4. The 12 Invariant in dual pairs\n");
    printf("  5. Coordinate frame generation\n");
    
    // Generate all 5 Platonic solids
    print_header("PART 1: GENERATING PLATONIC SOLIDS");
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        const PlatonicInfo* info = &PLATONIC_SOLIDS[i];
        
        print_subheader(info->name);
        printf("Schläfli symbol: %s%s%s\n", COLOR_YELLOW, info->schlafli, COLOR_RESET);
        printf("Dual: %s\n", info->dual_name);
        
        // Generate the solid based on name
        if (strcmp(info->name, "Tetrahedron") == 0) {
            solids[i] = platonic_generate_tetrahedron();
        } else if (strcmp(info->name, "Cube") == 0) {
            solids[i] = platonic_generate_cube();
        } else if (strcmp(info->name, "Octahedron") == 0) {
            solids[i] = platonic_generate_octahedron();
        } else if (strcmp(info->name, "Dodecahedron") == 0) {
            solids[i] = platonic_generate_dodecahedron();
        } else if (strcmp(info->name, "Icosahedron") == 0) {
            solids[i] = platonic_generate_icosahedron();
        }
        
        if (solids[i] == NULL) {
            printf("%s✗ Failed to generate %s%s\n", 
                   COLOR_RED, info->name, COLOR_RESET);
            continue;
        }
        
        printf("%s✓ Successfully generated %s%s\n", COLOR_GREEN, info->name, COLOR_RESET);
        success_count++;
        
        // Verify properties
        int v_match = (solids[i]->num_vertices == info->expected_vertices);
        int e_match = (solids[i]->num_edges == info->expected_edges);
        int f_match = (solids[i]->num_faces == info->expected_faces);
        
        printf("\nProperties:\n");
        printf("  Vertices: %d (expected %d) %s%s%s\n", 
               solids[i]->num_vertices, info->expected_vertices,
               v_match ? COLOR_GREEN : COLOR_RED,
               v_match ? "✓" : "✗",
               COLOR_RESET);
        printf("  Edges:    %d (expected %d) %s%s%s\n", 
               solids[i]->num_edges, info->expected_edges,
               e_match ? COLOR_GREEN : COLOR_RED,
               e_match ? "✓" : "✗",
               COLOR_RESET);
        printf("  Faces:    %d (expected %d) %s%s%s\n", 
               solids[i]->num_faces, info->expected_faces,
               f_match ? COLOR_GREEN : COLOR_RED,
               f_match ? "✓" : "✗",
               COLOR_RESET);
        
        // Verify Euler's formula
        printf("\n");
        if (verify_euler_formula(solids[i], info->name)) {
            euler_count++;
        }
        
        // Print some vertices
        print_vertices(solids[i], 4);
    }
    
    // Demonstrate duality relationships
    print_header("PART 2: DUALITY RELATIONSHIPS");
    
    // Tetrahedron (self-dual)
    if (solids[0]) {
        print_subheader("Tetrahedron (Self-Dual)");
        printf("The tetrahedron is its own dual.\n");
        printf("  Vertices = Faces = %s4%s\n", COLOR_YELLOW, COLOR_RESET);
        printf("  Edges = %s6%s\n", COLOR_YELLOW, COLOR_RESET);
    }
    
    // Cube ↔ Octahedron
    if (solids[1] && solids[2]) {
        demonstrate_12_invariant(solids[1], solids[2], "Cube", "Octahedron");
    }
    
    // Dodecahedron ↔ Icosahedron
    if (solids[3] && solids[4]) {
        demonstrate_12_invariant(solids[3], solids[4], "Dodecahedron", "Icosahedron");
    }
    
    // Summary
    print_header("SUMMARY");
    
    printf("\n%sResults:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  Solids generated: %s%d/%d%s\n", 
           success_count == NUM_PLATONIC_SOLIDS ? COLOR_GREEN : COLOR_YELLOW,
           success_count, NUM_PLATONIC_SOLIDS, COLOR_RESET);
    printf("  Euler's formula verified: %s%d/%d%s\n", 
           euler_count == success_count ? COLOR_GREEN : COLOR_YELLOW,
           euler_count, success_count, COLOR_RESET);
    
    printf("\n%sKey Insights:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  • There are exactly 5 Platonic solids in 3D space\n");
    printf("  • All satisfy Euler's formula: V - E + F = 2\n");
    printf("  • Duality swaps vertices ↔ faces, preserves edges\n");
    printf("  • %sTHE 12 INVARIANT:%s Number 12 appears in all non-self-dual pairs\n",
           COLOR_GREEN, COLOR_RESET);
    printf("    - Cube & Octahedron: both have 12 edges\n");
    printf("    - Dodecahedron: 12 faces ↔ Icosahedron: 12 vertices\n");
    printf("  • This connects to the 12-fold clock lattice structure\n");
    
    // Cleanup
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        if (solids[i]) {
            platonic_free(solids[i]);
        }
    }
    
    printf("\n");
    return (success_count == NUM_PLATONIC_SOLIDS && euler_count == success_count) ? 0 : 1;
}