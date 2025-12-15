/**
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * @file structural_mapping.c
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * @brief Structural mapping and geometric analysis (OBJECTIVE 28 - Layer 1)
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * 
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * Maps geometric structure properties and identifies deviations from ideal geometry.
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 */
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf

#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "blind_recovery/blind_recovery.h"
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Validate Euler's formula: V - E + F = 2
 * 
 * @param V Number of vertices
 * @param E Number of edges
 * @param F Number of faces
 * @return true if valid, false otherwise
 */
static bool validate_euler_formula(uint32_t V, uint32_t E, uint32_t F) {
    int euler = (int)V - (int)E + (int)F;
    return (euler == 2);
}

/**
 * Compute dimensional offsets from ideal geometry
 * 
 * @param num_vertices Actual number of vertices
 * @param num_edges Actual number of edges
 * @param num_faces Actual number of faces
 * @param offsets Output: dimensional offsets [3]
 */
static void compute_dimensional_offsets(
    uint32_t num_vertices,
    uint32_t num_edges,
    uint32_t num_faces,
    double* offsets
) {
    // Expected relationships for Platonic solids
    // For cube: V=8, E=12, F=6
    // Ratio: E/V ≈ 1.5, F/V ≈ 0.75
    
    double expected_edge_ratio = 1.5;
    double expected_face_ratio = 0.75;
    
    double actual_edge_ratio = (double)num_edges / (double)num_vertices;
    double actual_face_ratio = (double)num_faces / (double)num_vertices;
    
    offsets[0] = 0.0;  // Vertex offset (reference)
    offsets[1] = actual_edge_ratio - expected_edge_ratio;
    offsets[2] = actual_face_ratio - expected_face_ratio;
}

/**
 * Map geometric structure properties
 */
StructuralMap* map_structure(
    uint32_t num_vertices,
    uint32_t num_edges,
    uint32_t num_faces,
    const double* vertex_positions
) {
    if (num_vertices == 0 || num_edges == 0 || num_faces == 0) {
        return NULL;
    }
    
    printf("📐 Mapping geometric structure...\n");
    printf("  Vertices: %u\n", num_vertices);
    printf("  Edges: %u\n", num_edges);
    printf("  Faces: %u\n", num_faces);
    
    // Allocate structural map
    StructuralMap* map = malloc(sizeof(StructuralMap));
    if (!map) return NULL;
    
    map->num_vertices = num_vertices;
    map->num_edges = num_edges;
    map->num_faces = num_faces;
    map->coprime_matrix = NULL;  // Will be computed separately
    map->dimensional_offsets = calloc(3, sizeof(double));
    map->corruption_mask = calloc(num_vertices, sizeof(bool));
    map->corruption_percentage = 0.0;
    
    if (!map->dimensional_offsets || !map->corruption_mask) {
        free_structural_map(map);
        return NULL;
    }
    
    // Validate Euler's formula
    bool euler_valid = validate_euler_formula(num_vertices, num_edges, num_faces);
    printf("  Euler's formula (V-E+F=2): %s\n", euler_valid ? "✓ valid" : "✗ invalid");
    
    if (!euler_valid) {
        printf("  ⚠️  Geometric inconsistency detected\n");
    }
    
    // Compute dimensional offsets
    compute_dimensional_offsets(num_vertices, num_edges, num_faces,
                               map->dimensional_offsets);
    
    printf("  Dimensional offsets: [%.3f, %.3f, %.3f]\n",
           map->dimensional_offsets[0],
           map->dimensional_offsets[1],
           map->dimensional_offsets[2]);
    
    // Analyze vertex positions if provided
    if (vertex_positions) {
        uint32_t corrupted_count = 0;
        
        for (uint32_t i = 0; i < num_vertices; i++) {
            double x = vertex_positions[i * 3 + 0];
            double y = vertex_positions[i * 3 + 1];
            double z = vertex_positions[i * 3 + 2];
            
            // Check for NaN or Inf
            if (math_is_nan(x) || math_is_nan(y) || math_is_nan(z) ||
                math_is_inf(x) || math_is_inf(y) || math_is_inf(z)) {
                map->corruption_mask[i] = true;
                corrupted_count++;
            }
            
            // Check for extreme values
            double magnitude = math_sqrt(x*x + y*y + z*z);
            if (magnitude > 100.0) {  // Threshold
                map->corruption_mask[i] = true;
                corrupted_count++;
            }
        }
        
        map->corruption_percentage = (double)corrupted_count / (double)num_vertices;
        
        printf("  Corrupted vertices: %u (%.1f%%)\n",
               corrupted_count, map->corruption_percentage * 100.0);
    }
    
    printf("  ✓ Structural mapping complete\n");
    
    return map;
}

/**
 * Free structural map
 */
void free_structural_map(StructuralMap* map) {
    if (!map) return;
    
    if (map->coprime_matrix) {
        free_coprime_matrix(map->coprime_matrix, 3);  // Assuming 3D
    }
    
    if (map->dimensional_offsets) {
        free(map->dimensional_offsets);
    }
    
    if (map->corruption_mask) {
        free(map->corruption_mask);
    }
    
    free(map);
}
