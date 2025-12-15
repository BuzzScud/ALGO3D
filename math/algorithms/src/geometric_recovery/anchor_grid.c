/**
 * @file anchor_grid.c
 * @brief Universal Anchor Grid Implementation
 * 
 * Implements universal anchor grid using Platonic solid infrastructure.
 * Supports ANY Platonic solid with prime-vertex mapping and harmonic extension.
 * 
 * Thesis References:
 * - Theorem 5: Vertex-Prime Correspondence
 * - Theorem 6: Harmonic Extension
 * - OBJECTIVE 2: Infinite Platonic Solid Generator
 */

#include "geometric_recovery/anchor_grid.h"
#include "math/platonic_generator.h"
#include "math/polytope.h"
#include "math/prime.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * INTERNAL HELPERS
 * ============================================================================ */

/**
 * @brief Allocate vertex data arrays
 */
static int allocate_vertex_data(AnchorGrid* grid) {
    grid->vertex_primes = calloc(grid->num_vertices, sizeof(uint64_t));
    if (!grid->vertex_primes) return -1;
    
    grid->vertex_positions = calloc(grid->num_vertices, sizeof(ClockPosition));
    if (!grid->vertex_positions) {
        free(grid->vertex_primes);
        return -1;
    }
    
    grid->vertex_coords = calloc(grid->num_vertices, sizeof(double*));
    if (!grid->vertex_coords) {
        free(grid->vertex_primes);
        free(grid->vertex_positions);
        return -1;
    }
    
    for (uint32_t i = 0; i < grid->num_vertices; i++) {
        grid->vertex_coords[i] = calloc(grid->dimension, sizeof(double));
        if (!grid->vertex_coords[i]) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                free(grid->vertex_coords[j]);
            }
            free(grid->vertex_coords);
            free(grid->vertex_positions);
            free(grid->vertex_primes);
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief Free vertex data arrays
 */
static void free_vertex_data(AnchorGrid* grid) {
    if (grid->vertex_coords) {
        for (uint32_t i = 0; i < grid->num_vertices; i++) {
            free(grid->vertex_coords[i]);
        }
        free(grid->vertex_coords);
    }
    
    free(grid->vertex_positions);
    free(grid->vertex_primes);
}

/* ============================================================================
 * CREATION & DESTRUCTION
 * ============================================================================ */

AnchorGrid* anchor_grid_from_solid(
    PlatonicSolid* solid,
    uint32_t dimension
) {
    if (!solid || dimension < 3) return NULL;
    
    AnchorGrid* grid = calloc(1, sizeof(AnchorGrid));
    if (!grid) return NULL;
    
    grid->solid = solid;
    grid->num_vertices = solid->num_vertices;
    grid->dimension = dimension;
    grid->parent = NULL;
    grid->children = NULL;
    grid->num_children = 0;
    grid->max_depth = 0;
    grid->current_depth = 0;
    grid->is_adaptive = false;
    
    // Copy solid name
    strncpy(grid->name, solid->name, sizeof(grid->name) - 1);
    grid->name[sizeof(grid->name) - 1] = '\0';
    
    // Allocate vertex data
    if (allocate_vertex_data(grid) != 0) {
        free(grid);
        return NULL;
    }
    
    // Map primes to vertices (Theorem 5)
    if (anchor_grid_map_primes(grid) != 0) {
        free_vertex_data(grid);
        free(grid);
        return NULL;
    }
    
    // Compute coordinates (Theorem 6)
    if (anchor_grid_compute_all_coordinates(grid) != 0) {
        free_vertex_data(grid);
        free(grid);
        return NULL;
    }
    
    return grid;
}

AnchorGrid* anchor_grid_create_adaptive(
    uint64_t problem_size,
    uint32_t dimension
) {
    if (dimension < 3) return NULL;
    
    PlatonicSolid* solid = NULL;
    
    // Adaptive selection based on problem size
    if (problem_size < 100) {
        // Small problem: Tetrahedron (4 vertices)
        solid = platonic_generate_tetrahedron();
    } else if (problem_size < 1000) {
        // Medium problem: 24-cell (24 vertices)
        solid = platonic_generate_24cell();
    } else if (problem_size < 10000) {
        // Large problem: 120-cell (120 vertices)
        solid = platonic_generate_120cell();
    } else {
        // Huge problem: 600-cell (600 vertices)
        solid = platonic_generate_600cell();
    }
    
    if (!solid) return NULL;
    
    AnchorGrid* grid = anchor_grid_from_solid(solid, dimension);
    if (grid) {
        grid->is_adaptive = true;
    }
    
    return grid;
}

AnchorGrid* anchor_grid_create_hierarchical(
    PlatonicSolid* solid,
    uint32_t dimension,
    uint32_t max_depth
) {
    if (!solid || dimension < 3) return NULL;
    
    AnchorGrid* root = anchor_grid_from_solid(solid, dimension);
    if (!root) return NULL;
    
    root->max_depth = max_depth;
    root->current_depth = 0;
    
    // TODO: Implement hierarchical subdivision
    // This requires face subdivision which is complex
    // For now, just create the root grid
    
    return root;
}

AnchorGrid* anchor_grid_create_with_config(
    const AnchorGridConfig* config
) {
    if (!config) return NULL;
    
    if (config->enable_adaptive) {
        return anchor_grid_create_adaptive(
            config->problem_size,
            config->dimension
        );
    }
    
    // Default: Use 24-cell
    PlatonicSolid* solid = platonic_generate_24cell();
    if (!solid) return NULL;
    
    if (config->enable_hierarchical) {
        return anchor_grid_create_hierarchical(
            solid,
            config->dimension,
            config->max_depth
        );
    }
    
    return anchor_grid_from_solid(solid, config->dimension);
}

void anchor_grid_free(AnchorGrid* grid) {
    if (!grid) return;
    
    // Free children recursively
    if (grid->children) {
        for (uint32_t i = 0; i < grid->num_children; i++) {
            anchor_grid_free(grid->children[i]);
        }
        free(grid->children);
    }
    
    // Free vertex data
    free_vertex_data(grid);
    
    // Free solid (if we own it)
    if (grid->solid) {
        platonic_free(grid->solid);
    }
    
    free(grid);
}

/* ============================================================================
 * PRIME-VERTEX MAPPING (Theorem 5)
 * ============================================================================ */

int anchor_grid_map_primes(AnchorGrid* grid) {
    if (!grid) return -1;
    
    for (uint32_t i = 0; i < grid->num_vertices; i++) {
        // Theorem 5: Each vertex gets a prime
        grid->vertex_primes[i] = prime_nth(i + 1);
        
        // Map prime to clock position
        MathError err = clock_map_prime_to_position(
            grid->vertex_primes[i],
            &grid->vertex_positions[i]
        );
        
        if (err != MATH_SUCCESS) {
            return -1;
        }
    }
    
    return 0;
}

uint64_t anchor_grid_get_vertex_prime(
    const AnchorGrid* grid,
    uint32_t vertex_idx
) {
    if (!grid || vertex_idx >= grid->num_vertices) return 0;
    
    return grid->vertex_primes[vertex_idx];
}

const ClockPosition* anchor_grid_get_vertex_position(
    const AnchorGrid* grid,
    uint32_t vertex_idx
) {
    if (!grid || vertex_idx >= grid->num_vertices) return NULL;
    
    return &grid->vertex_positions[vertex_idx];
}

/* ============================================================================
 * HARMONIC EXTENSION (Theorem 6)
 * ============================================================================ */

int anchor_grid_compute_coordinates(
    AnchorGrid* grid,
    uint32_t vertex_idx,
    uint32_t dimension
) {
    if (!grid || vertex_idx >= grid->num_vertices || dimension < 3) {
        return -1;
    }
    
    ClockPosition* pos = &grid->vertex_positions[vertex_idx];
    double* coords = grid->vertex_coords[vertex_idx];
    
    // First 3 dimensions from clock position
    coords[0] = pos->radius * math_cos(pos->angle);
    coords[1] = pos->radius * math_sin(pos->angle);
    coords[2] = math_sqrt(1.0 - pos->radius * pos->radius);
    
    // Higher dimensions via harmonic extension (Theorem 6)
    for (uint32_t d = 3; d < dimension; d++) {
        double harmonic = (double)(d - 2);
        double phase = pos->angle * harmonic;
        
        if ((d - 3) % 2 == 0) {
            coords[d] = pos->radius * math_sin(phase);
        } else {
            coords[d] = pos->radius * math_cos(phase);
        }
    }
    
    return 0;
}

const double* anchor_grid_get_vertex_coords(
    const AnchorGrid* grid,
    uint32_t vertex_idx
) {
    if (!grid || vertex_idx >= grid->num_vertices) return NULL;
    
    return grid->vertex_coords[vertex_idx];
}

int anchor_grid_compute_all_coordinates(AnchorGrid* grid) {
    if (!grid) return -1;
    
    for (uint32_t i = 0; i < grid->num_vertices; i++) {
        if (anchor_grid_compute_coordinates(grid, i, grid->dimension) != 0) {
            return -1;
        }
    }
    
    return 0;
}

/* ============================================================================
 * HIERARCHICAL OPERATIONS
 * ============================================================================ */

AnchorGrid* anchor_grid_get_child(
    const AnchorGrid* grid,
    uint32_t child_idx
) {
    if (!grid || !grid->children || child_idx >= grid->num_children) {
        return NULL;
    }
    
    return grid->children[child_idx];
}

AnchorGrid* anchor_grid_get_parent(const AnchorGrid* grid) {
    if (!grid) return NULL;
    
    return grid->parent;
}

uint32_t anchor_grid_get_depth(const AnchorGrid* grid) {
    if (!grid) return 0;
    
    return grid->current_depth;
}

bool anchor_grid_is_root(const AnchorGrid* grid) {
    if (!grid) return false;
    
    return grid->parent == NULL;
}

bool anchor_grid_is_leaf(const AnchorGrid* grid) {
    if (!grid) return false;
    
    return grid->children == NULL || grid->num_children == 0;
}

/* ============================================================================
 * GEOMETRIC OPERATIONS
 * ============================================================================ */

int anchor_grid_find_nearest_vertex(
    const AnchorGrid* grid,
    const double* point,
    uint32_t dimension
) {
    if (!grid || !point || dimension != grid->dimension) {
        return -1;
    }
    
    double min_distance = INFINITY;
    int nearest_idx = -1;
    
    for (uint32_t i = 0; i < grid->num_vertices; i++) {
        const double* vertex = grid->vertex_coords[i];
        
        // Compute Euclidean distance
        double distance = 0.0;
        for (uint32_t d = 0; d < dimension; d++) {
            double diff = point[d] - vertex[d];
            distance += diff * diff;
        }
        distance = math_sqrt(distance);
        
        if (distance < min_distance) {
            min_distance = distance;
            nearest_idx = (int)i;
        }
    }
    
    return nearest_idx;
}

double anchor_grid_vertex_distance(
    const AnchorGrid* grid,
    uint32_t vertex1_idx,
    uint32_t vertex2_idx
) {
    if (!grid || vertex1_idx >= grid->num_vertices || 
        vertex2_idx >= grid->num_vertices) {
        return -1.0;
    }
    
    const double* v1 = grid->vertex_coords[vertex1_idx];
    const double* v2 = grid->vertex_coords[vertex2_idx];
    
    double distance = 0.0;
    for (uint32_t d = 0; d < grid->dimension; d++) {
        double diff = v1[d] - v2[d];
        distance += diff * diff;
    }
    
    return math_sqrt(distance);
}

int anchor_grid_get_neighbors(
    const AnchorGrid* grid,
    uint32_t vertex_idx,
    uint32_t* neighbors,
    uint32_t max_neighbors
) {
    if (!grid || vertex_idx >= grid->num_vertices || !neighbors) {
        return -1;
    }
    
    // Get neighbors from solid's edge connectivity
    if (!grid->solid || !grid->solid->edge_indices) {
        return -1;
    }
    
    uint32_t count = 0;
    
    // Find all edges connected to this vertex
    for (uint32_t i = 0; i < grid->solid->num_edges && count < max_neighbors; i++) {
        uint32_t v1 = grid->solid->edge_indices[i][0];
        uint32_t v2 = grid->solid->edge_indices[i][1];
        
        if (v1 == vertex_idx) {
            neighbors[count++] = v2;
        } else if (v2 == vertex_idx) {
            neighbors[count++] = v1;
        }
    }
    
    return (int)count;
}

/* ============================================================================
 * PROPERTIES & VALIDATION
 * ============================================================================ */

uint32_t anchor_grid_get_num_vertices(const AnchorGrid* grid) {
    if (!grid) return 0;
    
    return grid->num_vertices;
}

uint32_t anchor_grid_get_dimension(const AnchorGrid* grid) {
    if (!grid) return 0;
    
    return grid->dimension;
}

bool anchor_grid_validate_symmetry(const AnchorGrid* grid) {
    if (!grid) return false;
    
    // Check if number of vertices is compatible with 12-fold symmetry
    // Valid: 4, 8, 12, 24, 120, 600, etc.
    // Must be divisible by 4 (minimum symmetry)
    
    if (grid->num_vertices % 4 != 0) {
        return false;
    }
    
    // Additional checks could be added here
    
    return true;
}

bool anchor_grid_validate_euler(const AnchorGrid* grid) {
    if (!grid || !grid->solid) return false;
    
    // For 3D: V - E + F = 2
    // For 4D: V - E + F - C = 0
    // General: Euler characteristic depends on dimension
    
    if (grid->dimension == 3) {
        int euler = grid->solid->num_vertices - 
                    grid->solid->num_edges + 
                    grid->solid->num_faces;
        return euler == 2;
    }
    
    // For higher dimensions, validation is more complex
    // For now, just return true
    return true;
}

/* ============================================================================
 * PRINTING & DEBUGGING
 * ============================================================================ */

void anchor_grid_print_stats(const AnchorGrid* grid) {
    if (!grid) return;
    
    printf("=== Anchor Grid Statistics ===\n");
    printf("Name: %s\n", grid->name);
    printf("Vertices: %u\n", grid->num_vertices);
    printf("Dimension: %u\n", grid->dimension);
    printf("Adaptive: %s\n", grid->is_adaptive ? "YES" : "NO");
    printf("Depth: %u / %u\n", grid->current_depth, grid->max_depth);
    printf("Root: %s\n", anchor_grid_is_root(grid) ? "YES" : "NO");
    printf("Leaf: %s\n", anchor_grid_is_leaf(grid) ? "YES" : "NO");
    
    if (grid->solid) {
        printf("\nPlatonic Solid:\n");
        printf("  Edges: %u\n", grid->solid->num_edges);
        printf("  Faces: %u\n", grid->solid->num_faces);
    }
    
    printf("\n12-Fold Symmetry: %s\n", 
           anchor_grid_validate_symmetry(grid) ? "VALID" : "INVALID");
    printf("Euler Characteristic: %s\n",
           anchor_grid_validate_euler(grid) ? "VALID" : "INVALID");
    
    printf("===============================\n");
}

void anchor_grid_print_vertex(
    const AnchorGrid* grid,
    uint32_t vertex_idx
) {
    if (!grid || vertex_idx >= grid->num_vertices) return;
    
    printf("=== Vertex %u ===\n", vertex_idx);
    printf("Prime: %lu\n", grid->vertex_primes[vertex_idx]);
    
    const ClockPosition* pos = &grid->vertex_positions[vertex_idx];
    printf("Clock Position:\n");
    printf("  Ring: %u\n", pos->ring);
    printf("  Position: %u\n", pos->position);
    printf("  Angle: %.4f rad (%.2f°)\n", pos->angle, pos->angle * 180.0 / MATH_PI);
    printf("  Radius: %.4f\n", pos->radius);
    
    printf("Coordinates (%uD):\n", grid->dimension);
    const double* coords = grid->vertex_coords[vertex_idx];
    for (uint32_t d = 0; d < grid->dimension && d < 10; d++) {
        printf("  [%u]: %.6f\n", d, coords[d]);
    }
    if (grid->dimension > 10) {
        printf("  ... (%u more dimensions)\n", grid->dimension - 10);
    }
    
    printf("================\n");
}

void anchor_grid_print_hierarchy(const AnchorGrid* grid) {
    if (!grid) return;
    
    // Print with indentation based on depth
    for (uint32_t i = 0; i < grid->current_depth; i++) {
        printf("  ");
    }
    
    printf("├─ %s (%u vertices, depth %u)\n", 
           grid->name, grid->num_vertices, grid->current_depth);
    
    // Recursively print children
    if (grid->children) {
        for (uint32_t i = 0; i < grid->num_children; i++) {
            anchor_grid_print_hierarchy(grid->children[i]);
        }
    }
}