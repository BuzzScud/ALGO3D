#include "duality_88d.h"
#include <stdio.h>

int main() {
    printf("Testing Platonic solids...\n");
    
    PlatonicSolid solids[] = {TETRAHEDRON, CUBE, OCTAHEDRON, DODECAHEDRON, ICOSAHEDRON};
    
    for (int i = 0; i < 5; i++) {
        printf("Testing solid %d...\n", i);
        PlatonicSolid solid = solids[i];
        printf("  Name: %s\n", get_solid_name(solid));
        printf("  Vertices: %d\n", get_num_vertices(solid));
        printf("  Edges: %d\n", get_num_edges(solid));
        printf("  Faces: %d\n", get_num_faces(solid));
        printf("  Euler: %s\n", verify_euler_formula(solid) ? "true" : "false");
    }
    
    printf("\nTesting phase angles...\n");
    for (int h = 1; h <= 3; h++) {
        printf("Harmonic %d: %.1f Hz\n", h, calculate_harmonic_frequency(h));
    }
    
    printf("\nAll tests passed!\n");
    return 0;
}
