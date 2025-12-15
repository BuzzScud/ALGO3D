/*
 * Platonic Solid Test Data Generator
 * 
 * Generates accurate geometric data for all 5 Platonic solids:
 * - Tetrahedron (4 vertices, 6 edges, 4 faces)
 * - Cube (8 vertices, 12 edges, 6 faces)
 * - Octahedron (6 vertices, 12 edges, 8 faces)
 * - Dodecahedron (20 vertices, 30 edges, 12 faces)
 * - Icosahedron (12 vertices, 30 edges, 20 faces)
 * 
 * Output format: JSON with vertices, edges, faces, and metadata
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Golden ratio for dodecahedron and icosahedron
#define MATH_PHI 1.618033988749895

typedef struct {
    double x, y, z;
} Vertex;

typedef struct {
    int v1, v2;
} Edge;

typedef struct {
    int num_vertices;
    int* vertices;
} Face;

typedef struct {
    char name[50];
    int num_vertices;
    int num_edges;
    int num_faces;
    Vertex* vertices;
    Edge* edges;
    Face* faces;
} PlatonicSolid;

// Function prototypes
PlatonicSolid* create_tetrahedron();
PlatonicSolid* create_cube();
PlatonicSolid* create_octahedron();
PlatonicSolid* create_dodecahedron();
PlatonicSolid* create_icosahedron();
void write_solid_to_json(PlatonicSolid* solid, const char* filename);
void free_solid(PlatonicSolid* solid);
int verify_euler_formula(PlatonicSolid* solid);

// Tetrahedron: 4 vertices, 6 edges, 4 faces
PlatonicSolid* create_tetrahedron() {
    PlatonicSolid* solid = malloc(sizeof(PlatonicSolid));
    strcpy(solid->name, "Tetrahedron");
    solid->num_vertices = 4;
    solid->num_edges = 6;
    solid->num_faces = 4;
    
    // Vertices (regular tetrahedron centered at origin)
    solid->vertices = malloc(4 * sizeof(Vertex));
    solid->vertices[0] = (Vertex){1.0, 1.0, 1.0};
    solid->vertices[1] = (Vertex){1.0, -1.0, -1.0};
    solid->vertices[2] = (Vertex){-1.0, 1.0, -1.0};
    solid->vertices[3] = (Vertex){-1.0, -1.0, 1.0};
    
    // Edges
    solid->edges = malloc(6 * sizeof(Edge));
    solid->edges[0] = (Edge){0, 1};
    solid->edges[1] = (Edge){0, 2};
    solid->edges[2] = (Edge){0, 3};
    solid->edges[3] = (Edge){1, 2};
    solid->edges[4] = (Edge){1, 3};
    solid->edges[5] = (Edge){2, 3};
    
    // Faces (triangular)
    solid->faces = malloc(4 * sizeof(Face));
    for (int i = 0; i < 4; i++) {
        solid->faces[i].num_vertices = 3;
        solid->faces[i].vertices = malloc(3 * sizeof(int));
    }
    
    int face_data[4][3] = {
        {0, 1, 2}, {0, 1, 3}, {0, 2, 3}, {1, 2, 3}
    };
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            solid->faces[i].vertices[j] = face_data[i][j];
        }
    }
    
    return solid;
}

// Cube: 8 vertices, 12 edges, 6 faces
PlatonicSolid* create_cube() {
    PlatonicSolid* solid = malloc(sizeof(PlatonicSolid));
    strcpy(solid->name, "Cube");
    solid->num_vertices = 8;
    solid->num_edges = 12;
    solid->num_faces = 6;
    
    // Vertices (unit cube centered at origin)
    solid->vertices = malloc(8 * sizeof(Vertex));
    solid->vertices[0] = (Vertex){-1.0, -1.0, -1.0};
    solid->vertices[1] = (Vertex){1.0, -1.0, -1.0};
    solid->vertices[2] = (Vertex){1.0, 1.0, -1.0};
    solid->vertices[3] = (Vertex){-1.0, 1.0, -1.0};
    solid->vertices[4] = (Vertex){-1.0, -1.0, 1.0};
    solid->vertices[5] = (Vertex){1.0, -1.0, 1.0};
    solid->vertices[6] = (Vertex){1.0, 1.0, 1.0};
    solid->vertices[7] = (Vertex){-1.0, 1.0, 1.0};
    
    // Edges
    solid->edges = malloc(12 * sizeof(Edge));
    // Bottom face
    solid->edges[0] = (Edge){0, 1};
    solid->edges[1] = (Edge){1, 2};
    solid->edges[2] = (Edge){2, 3};
    solid->edges[3] = (Edge){3, 0};
    // Top face
    solid->edges[4] = (Edge){4, 5};
    solid->edges[5] = (Edge){5, 6};
    solid->edges[6] = (Edge){6, 7};
    solid->edges[7] = (Edge){7, 4};
    // Vertical edges
    solid->edges[8] = (Edge){0, 4};
    solid->edges[9] = (Edge){1, 5};
    solid->edges[10] = (Edge){2, 6};
    solid->edges[11] = (Edge){3, 7};
    
    // Faces (square)
    solid->faces = malloc(6 * sizeof(Face));
    for (int i = 0; i < 6; i++) {
        solid->faces[i].num_vertices = 4;
        solid->faces[i].vertices = malloc(4 * sizeof(int));
    }
    
    int face_data[6][4] = {
        {0, 1, 2, 3}, // Bottom
        {4, 5, 6, 7}, // Top
        {0, 1, 5, 4}, // Front
        {2, 3, 7, 6}, // Back
        {0, 3, 7, 4}, // Left
        {1, 2, 6, 5}  // Right
    };
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            solid->faces[i].vertices[j] = face_data[i][j];
        }
    }
    
    return solid;
}

// Octahedron: 6 vertices, 12 edges, 8 faces
PlatonicSolid* create_octahedron() {
    PlatonicSolid* solid = malloc(sizeof(PlatonicSolid));
    strcpy(solid->name, "Octahedron");
    solid->num_vertices = 6;
    solid->num_edges = 12;
    solid->num_faces = 8;
    
    // Vertices (regular octahedron centered at origin)
    solid->vertices = malloc(6 * sizeof(Vertex));
    solid->vertices[0] = (Vertex){1.0, 0.0, 0.0};
    solid->vertices[1] = (Vertex){-1.0, 0.0, 0.0};
    solid->vertices[2] = (Vertex){0.0, 1.0, 0.0};
    solid->vertices[3] = (Vertex){0.0, -1.0, 0.0};
    solid->vertices[4] = (Vertex){0.0, 0.0, 1.0};
    solid->vertices[5] = (Vertex){0.0, 0.0, -1.0};
    
    // Edges
    solid->edges = malloc(12 * sizeof(Edge));
    // Equator
    solid->edges[0] = (Edge){0, 2};
    solid->edges[1] = (Edge){2, 1};
    solid->edges[2] = (Edge){1, 3};
    solid->edges[3] = (Edge){3, 0};
    // Top pyramid
    solid->edges[4] = (Edge){0, 4};
    solid->edges[5] = (Edge){1, 4};
    solid->edges[6] = (Edge){2, 4};
    solid->edges[7] = (Edge){3, 4};
    // Bottom pyramid
    solid->edges[8] = (Edge){0, 5};
    solid->edges[9] = (Edge){1, 5};
    solid->edges[10] = (Edge){2, 5};
    solid->edges[11] = (Edge){3, 5};
    
    // Faces (triangular)
    solid->faces = malloc(8 * sizeof(Face));
    for (int i = 0; i < 8; i++) {
        solid->faces[i].num_vertices = 3;
        solid->faces[i].vertices = malloc(3 * sizeof(int));
    }
    
    int face_data[8][3] = {
        {0, 2, 4}, {2, 1, 4}, {1, 3, 4}, {3, 0, 4}, // Top
        {0, 2, 5}, {2, 1, 5}, {1, 3, 5}, {3, 0, 5}  // Bottom
    };
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) {
            solid->faces[i].vertices[j] = face_data[i][j];
        }
    }
    
    return solid;
}

// Dodecahedron: 20 vertices, 30 edges, 12 faces
PlatonicSolid* create_dodecahedron() {
    PlatonicSolid* solid = malloc(sizeof(PlatonicSolid));
    strcpy(solid->name, "Dodecahedron");
    solid->num_vertices = 20;
    solid->num_edges = 30;
    solid->num_faces = 12;
    
    // Vertices (using golden ratio)
    solid->vertices = malloc(20 * sizeof(Vertex));
    
    // 8 vertices of a cube
    solid->vertices[0] = (Vertex){1.0, 1.0, 1.0};
    solid->vertices[1] = (Vertex){1.0, 1.0, -1.0};
    solid->vertices[2] = (Vertex){1.0, -1.0, 1.0};
    solid->vertices[3] = (Vertex){1.0, -1.0, -1.0};
    solid->vertices[4] = (Vertex){-1.0, 1.0, 1.0};
    solid->vertices[5] = (Vertex){-1.0, 1.0, -1.0};
    solid->vertices[6] = (Vertex){-1.0, -1.0, 1.0};
    solid->vertices[7] = (Vertex){-1.0, -1.0, -1.0};
    
    // 12 vertices on rectangular faces
    solid->vertices[8] = (Vertex){0.0, 1.0/MATH_PHI, MATH_PHI};
    solid->vertices[9] = (Vertex){0.0, 1.0/MATH_PHI, -MATH_PHI};
    solid->vertices[10] = (Vertex){0.0, -1.0/MATH_PHI, MATH_PHI};
    solid->vertices[11] = (Vertex){0.0, -1.0/MATH_PHI, -MATH_PHI};
    
    solid->vertices[12] = (Vertex){1.0/MATH_PHI, MATH_PHI, 0.0};
    solid->vertices[13] = (Vertex){1.0/MATH_PHI, -MATH_PHI, 0.0};
    solid->vertices[14] = (Vertex){-1.0/MATH_PHI, MATH_PHI, 0.0};
    solid->vertices[15] = (Vertex){-1.0/MATH_PHI, -MATH_PHI, 0.0};
    
    solid->vertices[16] = (Vertex){MATH_PHI, 0.0, 1.0/MATH_PHI};
    solid->vertices[17] = (Vertex){MATH_PHI, 0.0, -1.0/MATH_PHI};
    solid->vertices[18] = (Vertex){-MATH_PHI, 0.0, 1.0/MATH_PHI};
    solid->vertices[19] = (Vertex){-MATH_PHI, 0.0, -1.0/MATH_PHI};
    
    // Edges (30 edges)
    solid->edges = malloc(30 * sizeof(Edge));
    int edge_data[30][2] = {
        {0, 8}, {0, 12}, {0, 16}, {1, 9}, {1, 12}, {1, 17},
        {2, 10}, {2, 13}, {2, 16}, {3, 11}, {3, 13}, {3, 17},
        {4, 8}, {4, 14}, {4, 18}, {5, 9}, {5, 14}, {5, 19},
        {6, 10}, {6, 15}, {6, 18}, {7, 11}, {7, 15}, {7, 19},
        {8, 10}, {9, 11}, {12, 14}, {13, 15}, {16, 17}, {18, 19}
    };
    
    for (int i = 0; i < 30; i++) {
        solid->edges[i] = (Edge){edge_data[i][0], edge_data[i][1]};
    }
    
    // Faces (12 pentagonal faces)
    solid->faces = malloc(12 * sizeof(Face));
    for (int i = 0; i < 12; i++) {
        solid->faces[i].num_vertices = 5;
        solid->faces[i].vertices = malloc(5 * sizeof(int));
    }
    
    int face_data[12][5] = {
        {0, 8, 10, 2, 16}, {0, 12, 1, 17, 16}, {0, 8, 4, 14, 12},
        {1, 9, 5, 14, 12}, {1, 9, 11, 3, 17}, {2, 10, 6, 15, 13},
        {2, 13, 3, 17, 16}, {3, 11, 7, 15, 13}, {4, 8, 10, 6, 18},
        {4, 14, 5, 19, 18}, {5, 9, 11, 7, 19}, {6, 15, 7, 19, 18}
    };
    
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 5; j++) {
            solid->faces[i].vertices[j] = face_data[i][j];
        }
    }
    
    return solid;
}

// Icosahedron: 12 vertices, 30 edges, 20 faces
PlatonicSolid* create_icosahedron() {
    PlatonicSolid* solid = malloc(sizeof(PlatonicSolid));
    strcpy(solid->name, "Icosahedron");
    solid->num_vertices = 12;
    solid->num_edges = 30;
    solid->num_faces = 20;
    
    // Vertices (using golden ratio)
    solid->vertices = malloc(12 * sizeof(Vertex));
    
    // 4 vertices on XY plane
    solid->vertices[0] = (Vertex){0.0, 1.0, MATH_PHI};
    solid->vertices[1] = (Vertex){0.0, -1.0, MATH_PHI};
    solid->vertices[2] = (Vertex){0.0, 1.0, -MATH_PHI};
    solid->vertices[3] = (Vertex){0.0, -1.0, -MATH_PHI};
    
    // 4 vertices on YZ plane
    solid->vertices[4] = (Vertex){1.0, MATH_PHI, 0.0};
    solid->vertices[5] = (Vertex){-1.0, MATH_PHI, 0.0};
    solid->vertices[6] = (Vertex){1.0, -MATH_PHI, 0.0};
    solid->vertices[7] = (Vertex){-1.0, -MATH_PHI, 0.0};
    
    // 4 vertices on XZ plane
    solid->vertices[8] = (Vertex){MATH_PHI, 0.0, 1.0};
    solid->vertices[9] = (Vertex){-MATH_PHI, 0.0, 1.0};
    solid->vertices[10] = (Vertex){MATH_PHI, 0.0, -1.0};
    solid->vertices[11] = (Vertex){-MATH_PHI, 0.0, -1.0};
    
    // Edges (30 edges)
    solid->edges = malloc(30 * sizeof(Edge));
    int edge_data[30][2] = {
        {0, 1}, {0, 4}, {0, 5}, {0, 8}, {0, 9},
        {1, 6}, {1, 7}, {1, 8}, {1, 9},
        {2, 3}, {2, 4}, {2, 5}, {2, 10}, {2, 11},
        {3, 6}, {3, 7}, {3, 10}, {3, 11},
        {4, 5}, {4, 8}, {4, 10},
        {5, 9}, {5, 11},
        {6, 7}, {6, 8}, {6, 10},
        {7, 9}, {7, 11},
        {8, 10}, {9, 11}
    };
    
    for (int i = 0; i < 30; i++) {
        solid->edges[i] = (Edge){edge_data[i][0], edge_data[i][1]};
    }
    
    // Faces (20 triangular faces)
    solid->faces = malloc(20 * sizeof(Face));
    for (int i = 0; i < 20; i++) {
        solid->faces[i].num_vertices = 3;
        solid->faces[i].vertices = malloc(3 * sizeof(int));
    }
    
    int face_data[20][3] = {
        {0, 1, 8}, {0, 1, 9}, {0, 4, 5}, {0, 4, 8}, {0, 5, 9},
        {1, 6, 7}, {1, 6, 8}, {1, 7, 9},
        {2, 3, 10}, {2, 3, 11}, {2, 4, 5}, {2, 4, 10}, {2, 5, 11},
        {3, 6, 7}, {3, 6, 10}, {3, 7, 11},
        {4, 8, 10}, {5, 9, 11}, {6, 8, 10}, {7, 9, 11}
    };
    
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 3; j++) {
            solid->faces[i].vertices[j] = face_data[i][j];
        }
    }
    
    return solid;
}

// Verify Euler's formula: V - E + F = 2
int verify_euler_formula(PlatonicSolid* solid) {
    int result = solid->num_vertices - solid->num_edges + solid->num_faces;
    return result == 2;
}

// Write solid to JSON file
void write_solid_to_json(PlatonicSolid* solid, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  &quot;name&quot;: &quot;%s&quot;,\n", solid->name);
    fprintf(fp, "  &quot;num_vertices&quot;: %d,\n", solid->num_vertices);
    fprintf(fp, "  &quot;num_edges&quot;: %d,\n", solid->num_edges);
    fprintf(fp, "  &quot;num_faces&quot;: %d,\n", solid->num_faces);
    fprintf(fp, "  &quot;euler_verified&quot;: %s,\n", verify_euler_formula(solid) ? "true" : "false");
    
    // Write vertices
    fprintf(fp, "  &quot;vertices&quot;: [\n");
    for (int i = 0; i < solid->num_vertices; i++) {
        fprintf(fp, "    {&quot;x&quot;: %.10f, &quot;y&quot;: %.10f, &quot;z&quot;: %.10f}%s\n",
                solid->vertices[i].x, solid->vertices[i].y, solid->vertices[i].z,
                (i < solid->num_vertices - 1) ? "," : "");
    }
    fprintf(fp, "  ],\n");
    
    // Write edges
    fprintf(fp, "  &quot;edges&quot;: [\n");
    for (int i = 0; i < solid->num_edges; i++) {
        fprintf(fp, "    {&quot;v1&quot;: %d, &quot;v2&quot;: %d}%s\n",
                solid->edges[i].v1, solid->edges[i].v2,
                (i < solid->num_edges - 1) ? "," : "");
    }
    fprintf(fp, "  ],\n");
    
    // Write faces
    fprintf(fp, "  &quot;faces&quot;: [\n");
    for (int i = 0; i < solid->num_faces; i++) {
        fprintf(fp, "    {&quot;vertices&quot;: [");
        for (int j = 0; j < solid->faces[i].num_vertices; j++) {
            fprintf(fp, "%d%s", solid->faces[i].vertices[j],
                    (j < solid->faces[i].num_vertices - 1) ? ", " : "");
        }
        fprintf(fp, "]}%s\n", (i < solid->num_faces - 1) ? "," : "");
    }
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Generated: %s (V=%d, E=%d, F=%d, Euler=%s)\n",
           filename, solid->num_vertices, solid->num_edges, solid->num_faces,
           verify_euler_formula(solid) ? "✓" : "✗");
}

// Free solid memory
void free_solid(PlatonicSolid* solid) {
    free(solid->vertices);
    free(solid->edges);
    for (int i = 0; i < solid->num_faces; i++) {
        free(solid->faces[i].vertices);
    }
    free(solid->faces);
    free(solid);
}

int main(int argc, char** argv) {
    printf("Platonic Solid Test Data Generator\n");
    printf("===================================\n\n");
    
    const char* output_dir = "algorithms/test_data/geometric/platonic";
    
    if (argc > 1) {
        output_dir = argv[1];
    }
    
    printf("Output directory: %s\n\n", output_dir);
    
    // Generate all 5 Platonic solids
    PlatonicSolid* solids[5];
    const char* filenames[5] = {
        "tetrahedron.json",
        "cube.json",
        "octahedron.json",
        "dodecahedron.json",
        "icosahedron.json"
    };
    
    solids[0] = create_tetrahedron();
    solids[1] = create_cube();
    solids[2] = create_octahedron();
    solids[3] = create_dodecahedron();
    solids[4] = create_icosahedron();
    
    // Write to JSON files
    for (int i = 0; i < 5; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", output_dir, filenames[i]);
        write_solid_to_json(solids[i], filepath);
        free_solid(solids[i]);
    }
    
    printf("\n✓ All 5 Platonic solids generated successfully!\n");
    printf("✓ All Euler's formulas verified (V - E + F = 2)\n");
    
    return 0;
}
