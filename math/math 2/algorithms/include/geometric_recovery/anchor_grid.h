/**
 * @file anchor_grid.h
 * @brief Universal Anchor Grid for Geometric Recovery
 * 
 * Revolutionary universal anchor grid that works with ANY Platonic solid,
 * not just the 24-cell. Implements Thesis Theorems 5 & 6.
 * 
 * Key Features:
 * - Uses platonic_generator.h infrastructure (not hardcoded)
 * - Supports ANY Platonic solid (4V to 600V+)
 * - Prime-vertex mapping (Theorem 5)
 * - Harmonic extension to nD (Theorem 6)
 * - Hierarchical nesting (infinite depth)
 * - Adaptive selection based on problem size
 * - Maintains 12-fold symmetry
 * 
 * Thesis References:
 * - Theorem 5: Vertex-Prime Correspondence
 * - Theorem 6: Harmonic Extension
 * - OBJECTIVE 2: Infinite Platonic Solid Generator
 * 
 * Usage:
 *   AnchorGrid* grid = anchor_grid_create_adaptive(problem_size, dimension);
 *   // Use grid for geometric recovery
 *   anchor_grid_free(grid);
 */

#ifndef ANCHOR_GRID_H
#define ANCHOR_GRID_H

#include <stdint.h>
#include <stdbool.h>
#include "math/types.h"
#include "math/clock.h"
#include "math/polytope.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Universal Anchor Grid
 * 
 * Represents an anchor grid based on ANY Platonic solid.
 * Supports hierarchical nesting and adaptive selection.
 */
typedef struct AnchorGrid {
    // Platonic solid structure
    PlatonicSolid* solid;           // From platonic_generator.h
    uint32_t num_vertices;          // Dynamic (4, 8, 12, 24, 120, 600, ...)
    uint32_t dimension;             // Dynamic (3D, 4D, 5D, ..., nD)
    
    // Vertex data (Theorem 5: Prime-Vertex Mapping)
    uint64_t* vertex_primes;        // Prime for each vertex
    ClockPosition* vertex_positions; // Clock position for each vertex
    double** vertex_coords;         // nD coordinates (harmonic extension)
    
    // Hierarchical nesting
    struct AnchorGrid* parent;      // Parent grid (coarser)
    struct AnchorGrid** children;   // Child grids (finer)
    uint32_t num_children;
    uint32_t max_depth;
    uint32_t current_depth;
    
    // Metadata
    char name[64];                  // Solid name (e.g., "24-cell", "600-cell")
    bool is_adaptive;               // Created via adaptive selection
} AnchorGrid;

/**
 * @brief Anchor grid configuration
 */
typedef struct {
    uint32_t dimension;             // Target dimension
    uint32_t max_depth;             // Maximum nesting depth (0 = no nesting)
    bool enable_hierarchical;       // Enable hierarchical nesting
    bool enable_adaptive;           // Enable adaptive selection
    uint64_t problem_size;          // Problem size (for adaptive selection)
} AnchorGridConfig;

/* ============================================================================
 * CREATION & DESTRUCTION
 * ============================================================================ */

/**
 * @brief Create anchor grid from Platonic solid
 * 
 * @param solid Platonic solid (from platonic_generator.h)
 * @param dimension Target dimension (3, 4, 5, ..., n)
 * @return Anchor grid or NULL on error
 */
AnchorGrid* anchor_grid_from_solid(
    PlatonicSolid* solid,
    uint32_t dimension
);

/**
 * @brief Create anchor grid with adaptive selection
 * 
 * Automatically selects appropriate Platonic solid based on problem size:
 * - problem_size < 100: Tetrahedron (4 vertices)
 * - problem_size < 1000: 24-cell (24 vertices)
 * - problem_size < 10000: 120-cell (120 vertices)
 * - problem_size >= 10000: 600-cell (600 vertices)
 * 
 * @param problem_size Problem size (number of samples, search space, etc.)
 * @param dimension Target dimension
 * @return Anchor grid or NULL on error
 */
AnchorGrid* anchor_grid_create_adaptive(
    uint64_t problem_size,
    uint32_t dimension
);

/**
 * @brief Create hierarchical anchor grid
 * 
 * Creates nested grids with increasing refinement.
 * 
 * @param solid Base Platonic solid
 * @param dimension Target dimension
 * @param max_depth Maximum nesting depth
 * @return Root anchor grid or NULL on error
 */
AnchorGrid* anchor_grid_create_hierarchical(
    PlatonicSolid* solid,
    uint32_t dimension,
    uint32_t max_depth
);

/**
 * @brief Create anchor grid with configuration
 * 
 * @param config Configuration
 * @return Anchor grid or NULL on error
 */
AnchorGrid* anchor_grid_create_with_config(
    const AnchorGridConfig* config
);

/**
 * @brief Free anchor grid
 * 
 * Frees all resources including children (if hierarchical).
 * 
 * @param grid Anchor grid to free
 */
void anchor_grid_free(AnchorGrid* grid);

/* ============================================================================
 * PRIME-VERTEX MAPPING (Theorem 5)
 * ============================================================================ */

/**
 * @brief Map primes to vertices
 * 
 * Implements Theorem 5: Vertex-Prime Correspondence
 * Each vertex is assigned a prime number in order.
 * 
 * @param grid Anchor grid
 * @return 0 on success, -1 on error
 */
int anchor_grid_map_primes(AnchorGrid* grid);

/**
 * @brief Get prime for vertex
 * 
 * @param grid Anchor grid
 * @param vertex_idx Vertex index
 * @return Prime number or 0 on error
 */
uint64_t anchor_grid_get_vertex_prime(
    const AnchorGrid* grid,
    uint32_t vertex_idx
);

/**
 * @brief Get clock position for vertex
 * 
 * @param grid Anchor grid
 * @param vertex_idx Vertex index
 * @return Clock position or NULL on error
 */
const ClockPosition* anchor_grid_get_vertex_position(
    const AnchorGrid* grid,
    uint32_t vertex_idx
);

/* ============================================================================
 * HARMONIC EXTENSION (Theorem 6)
 * ============================================================================ */

/**
 * @brief Compute nD coordinates via harmonic extension
 * 
 * Implements Theorem 6: Harmonic Extension
 * Extends 3D clock positions to nD using harmonic functions.
 * 
 * Formula for dimension d > 3:
 *   harmonic = d - 2
 *   phase = angle * harmonic
 *   x_d = radius * sin(phase)  if (d-3) is even
 *   x_d = radius * cos(phase)  if (d-3) is odd
 * 
 * @param grid Anchor grid
 * @param vertex_idx Vertex index
 * @param dimension Target dimension
 * @return 0 on success, -1 on error
 */
int anchor_grid_compute_coordinates(
    AnchorGrid* grid,
    uint32_t vertex_idx,
    uint32_t dimension
);

/**
 * @brief Get nD coordinates for vertex
 * 
 * @param grid Anchor grid
 * @param vertex_idx Vertex index
 * @return Coordinate array or NULL on error
 */
const double* anchor_grid_get_vertex_coords(
    const AnchorGrid* grid,
    uint32_t vertex_idx
);

/**
 * @brief Compute all vertex coordinates
 * 
 * @param grid Anchor grid
 * @return 0 on success, -1 on error
 */
int anchor_grid_compute_all_coordinates(AnchorGrid* grid);

/* ============================================================================
 * HIERARCHICAL OPERATIONS
 * ============================================================================ */

/**
 * @brief Get child grid at index
 * 
 * @param grid Parent grid
 * @param child_idx Child index
 * @return Child grid or NULL on error
 */
AnchorGrid* anchor_grid_get_child(
    const AnchorGrid* grid,
    uint32_t child_idx
);

/**
 * @brief Get parent grid
 * 
 * @param grid Child grid
 * @return Parent grid or NULL if root
 */
AnchorGrid* anchor_grid_get_parent(const AnchorGrid* grid);

/**
 * @brief Get depth in hierarchy
 * 
 * @param grid Anchor grid
 * @return Depth (0 = root)
 */
uint32_t anchor_grid_get_depth(const AnchorGrid* grid);

/**
 * @brief Check if grid is root
 * 
 * @param grid Anchor grid
 * @return true if root, false otherwise
 */
bool anchor_grid_is_root(const AnchorGrid* grid);

/**
 * @brief Check if grid is leaf
 * 
 * @param grid Anchor grid
 * @return true if leaf, false otherwise
 */
bool anchor_grid_is_leaf(const AnchorGrid* grid);

/* ============================================================================
 * GEOMETRIC OPERATIONS
 * ============================================================================ */

/**
 * @brief Find nearest vertex to point
 * 
 * @param grid Anchor grid
 * @param point Point in nD space
 * @param dimension Dimension of point
 * @return Nearest vertex index or -1 on error
 */
int anchor_grid_find_nearest_vertex(
    const AnchorGrid* grid,
    const double* point,
    uint32_t dimension
);

/**
 * @brief Compute distance between vertices
 * 
 * @param grid Anchor grid
 * @param vertex1_idx First vertex index
 * @param vertex2_idx Second vertex index
 * @return Distance or -1.0 on error
 */
double anchor_grid_vertex_distance(
    const AnchorGrid* grid,
    uint32_t vertex1_idx,
    uint32_t vertex2_idx
);

/**
 * @brief Get neighbors of vertex
 * 
 * @param grid Anchor grid
 * @param vertex_idx Vertex index
 * @param neighbors Output array (must be pre-allocated)
 * @param max_neighbors Maximum number of neighbors
 * @return Number of neighbors or -1 on error
 */
int anchor_grid_get_neighbors(
    const AnchorGrid* grid,
    uint32_t vertex_idx,
    uint32_t* neighbors,
    uint32_t max_neighbors
);

/* ============================================================================
 * PROPERTIES & VALIDATION
 * ============================================================================ */

/**
 * @brief Get number of vertices
 * 
 * @param grid Anchor grid
 * @return Number of vertices
 */
uint32_t anchor_grid_get_num_vertices(const AnchorGrid* grid);

/**
 * @brief Get dimension
 * 
 * @param grid Anchor grid
 * @return Dimension
 */
uint32_t anchor_grid_get_dimension(const AnchorGrid* grid);

/**
 * @brief Validate 12-fold symmetry
 * 
 * Checks if grid maintains 12-fold symmetry (RULE 6).
 * 
 * @param grid Anchor grid
 * @return true if valid, false otherwise
 */
bool anchor_grid_validate_symmetry(const AnchorGrid* grid);

/**
 * @brief Validate Euler characteristic
 * 
 * Checks if V - E + F = 2 (for 3D) or appropriate value for nD.
 * 
 * @param grid Anchor grid
 * @return true if valid, false otherwise
 */
bool anchor_grid_validate_euler(const AnchorGrid* grid);

/* ============================================================================
 * PRINTING & DEBUGGING
 * ============================================================================ */

/**
 * @brief Print anchor grid statistics
 * 
 * @param grid Anchor grid
 */
void anchor_grid_print_stats(const AnchorGrid* grid);

/**
 * @brief Print vertex information
 * 
 * @param grid Anchor grid
 * @param vertex_idx Vertex index
 */
void anchor_grid_print_vertex(
    const AnchorGrid* grid,
    uint32_t vertex_idx
);

/**
 * @brief Print hierarchy tree
 * 
 * @param grid Root anchor grid
 */
void anchor_grid_print_hierarchy(const AnchorGrid* grid);

#ifdef __cplusplus
}
#endif

#endif // ANCHOR_GRID_H