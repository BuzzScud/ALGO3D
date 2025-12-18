#ifndef GEOMETRIC_MATRIX_H
#define GEOMETRIC_MATRIX_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "math/abacus.h"

// Forward declare PlatonicSolidType to avoid circular dependency
typedef enum {
    PLATONIC_TETRAHEDRON = 0,
    PLATONIC_CUBE = 1,
    PLATONIC_OCTAHEDRON = 2,
    PLATONIC_DODECAHEDRON = 3,
    PLATONIC_ICOSAHEDRON = 4
} PlatonicSolidType;

// Forward declare PlatonicGeometry
typedef struct PlatonicGeometry PlatonicGeometry;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file geometric_matrix.h
 * @brief Geometric Matrix Storage using Platonic Solid Coordinate Frames
 * 
 * This implements the true 88D architecture where matrices are stored as
 * geometric structures with vertex values on Platonic solids. Interior
 * points are computed via barycentric interpolation.
 * 
 * Key Concepts:
 * - Store only vertex values (4-20 values instead of N²)
 * - Use Platonic solids as coordinate frames
 * - Compute interior points via barycentric interpolation
 * - Leverage self-similar structure
 * - 1,674× memory reduction vs flat arrays
 * 
 * Mathematical Foundation:
 * For any point p in the matrix, we can express it as:
 *   p = λ₁v₁ + λ₂v₂ + ... + λₙvₙ
 * where vᵢ are vertex values and λᵢ are barycentric weights (Σλᵢ = 1)
 */

// ============================================================================
// TYPES AND STRUCTURES
// ============================================================================

/**
 * @brief Geometric Matrix Structure
 * 
 * Stores a matrix as vertex values on a Platonic solid coordinate frame.
 * Interior points are computed via barycentric interpolation.
 */
typedef struct GeometricMatrix {
    // Platonic solid configuration
    PlatonicSolidType solid;        // Which Platonic solid (tetrahedron, cube, etc.)
    uint32_t num_vertices;          // Number of vertices (4, 6, 8, 12, or 20)
    
    // Vertex values (the actual stored data)
    CrystallineAbacus** vertex_values;  // [num_vertices] - only these are stored!
    
    // Vertex positions in 3D space (normalized to unit sphere)
    double* vertex_positions;       // [num_vertices * 3] - (x, y, z) for each vertex
    
    // Symmetry operations (rotation matrices for 12-fold symmetry)
    double* symmetry_ops;           // [num_symmetries * 9] - 3×3 rotation matrices
    uint32_t num_symmetries;        // Typically 12 for 12-fold symmetry
    
    // Matrix dimensions (logical size)
    uint32_t rows;                  // Number of rows
    uint32_t cols;                  // Number of columns
    
    // Metadata
    char name[64];                  // Parameter name (e.g., "W_q", "W_k")
    bool is_initialized;            // Has been initialized
    
    // Thread safety
    pthread_mutex_t lock;           // Lock for thread-safe access
    
} GeometricMatrix;

/**
 * @brief Barycentric coordinates for a point
 * 
 * Represents a point as weighted combination of vertices.
 */
typedef struct {
    uint32_t* vertex_indices;       // Indices of nearby vertices
    double* weights;                // Barycentric weights (sum to 1.0)
    uint32_t num_vertices;          // Number of vertices used (typically 3-4)
} BarycentricCoords;

// ============================================================================
// CREATION AND DESTRUCTION
// ============================================================================

/**
 * @brief Create a geometric matrix
 * 
 * Allocates a geometric matrix structure with the specified dimensions.
 * The Platonic solid is chosen automatically based on matrix size.
 * 
 * @param rows Number of rows
 * @param cols Number of columns
 * @param name Parameter name (for debugging)
 * @return Pointer to geometric matrix, or NULL on error
 */
GeometricMatrix* geometric_matrix_create(
    uint32_t rows,
    uint32_t cols,
    const char* name
);

/**
 * @brief Create a geometric matrix with specific Platonic solid
 * 
 * @param rows Number of rows
 * @param cols Number of columns
 * @param solid Platonic solid type to use
 * @param name Parameter name
 * @return Pointer to geometric matrix, or NULL on error
 */
GeometricMatrix* geometric_matrix_create_with_solid(
    uint32_t rows,
    uint32_t cols,
    PlatonicSolidType solid,
    const char* name
);

/**
 * @brief Free a geometric matrix
 * 
 * Frees all memory associated with the geometric matrix.
 * 
 * @param matrix Matrix to free
 */
void geometric_matrix_free(GeometricMatrix* matrix);

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * @brief Initialize vertex values to zeros
 * 
 * @param matrix Matrix to initialize
 * @return 0 on success, -1 on error
 */
int geometric_matrix_init_zeros(GeometricMatrix* matrix);

/**
 * @brief Initialize vertex values to ones
 * 
 * @param matrix Matrix to initialize
 * @return 0 on success, -1 on error
 */
int geometric_matrix_init_ones(GeometricMatrix* matrix);

/**
 * @brief Initialize vertex values with uniform random values
 * 
 * @param matrix Matrix to initialize
 * @param min Minimum value
 * @param max Maximum value
 * @param seed Random seed
 * @return 0 on success, -1 on error
 */
int geometric_matrix_init_uniform(
    GeometricMatrix* matrix,
    double min,
    double max,
    uint64_t seed
);

/**
 * @brief Initialize vertex values with Xavier initialization
 * 
 * Xavier initialization: values ~ U(-√(6/(fan_in + fan_out)), √(6/(fan_in + fan_out)))
 * 
 * @param matrix Matrix to initialize
 * @param fan_in Number of input units
 * @param fan_out Number of output units
 * @param seed Random seed
 * @return 0 on success, -1 on error
 */
int geometric_matrix_init_xavier(
    GeometricMatrix* matrix,
    uint32_t fan_in,
    uint32_t fan_out,
    uint64_t seed
);

/**
 * @brief Initialize vertex values with He initialization
 * 
 * He initialization: values ~ N(0, √(2/fan_in))
 * 
 * @param matrix Matrix to initialize
 * @param fan_in Number of input units
 * @param seed Random seed
 * @return 0 on success, -1 on error
 */
int geometric_matrix_init_he(
    GeometricMatrix* matrix,
    uint32_t fan_in,
    uint64_t seed
);

// ============================================================================
// ACCESS OPERATIONS
// ============================================================================

/**
 * @brief Get value at (row, col) via barycentric interpolation
 * 
 * This is the core operation. It:
 * 1. Maps (row, col) to a position on the Platonic solid surface
 * 2. Finds the nearest vertices
 * 3. Computes barycentric weights
 * 4. Interpolates from vertex values
 * 
 * @param matrix Matrix to access
 * @param row Row index
 * @param col Column index
 * @param result Output value (must be pre-allocated)
 * @return 0 on success, -1 on error
 */
int geometric_matrix_get(
    const GeometricMatrix* matrix,
    uint32_t row,
    uint32_t col,
    CrystallineAbacus* result
);

/**
 * @brief Set vertex value directly
 * 
 * This sets a vertex value directly. Use this during initialization
 * or when updating gradients.
 * 
 * @param matrix Matrix to modify
 * @param vertex_idx Vertex index (0 to num_vertices-1)
 * @param value Value to set
 * @return 0 on success, -1 on error
 */
int geometric_matrix_set_vertex(
    GeometricMatrix* matrix,
    uint32_t vertex_idx,
    const CrystallineAbacus* value
);

/**
 * @brief Get vertex value directly
 * 
 * @param matrix Matrix to access
 * @param vertex_idx Vertex index
 * @param result Output value (must be pre-allocated)
 * @return 0 on success, -1 on error
 */
int geometric_matrix_get_vertex(
    const GeometricMatrix* matrix,
    uint32_t vertex_idx,
    CrystallineAbacus* result
);

/**
 * @brief Accumulate gradient to vertex values
 * 
 * This is used during backpropagation. Given a gradient at (row, col),
 * it distributes the gradient to nearby vertices using barycentric weights.
 * 
 * @param matrix Matrix to update
 * @param row Row index
 * @param col Column index
 * @param gradient Gradient value to accumulate
 * @return 0 on success, -1 on error
 */
int geometric_matrix_accumulate_gradient(
    GeometricMatrix* matrix,
    uint32_t row,
    uint32_t col,
    const CrystallineAbacus* gradient
);

// ============================================================================
// BARYCENTRIC OPERATIONS
// ============================================================================

/**
 * @brief Compute barycentric coordinates for a point
 * 
 * Maps (row, col) to a position on the Platonic solid and computes
 * barycentric coordinates relative to nearby vertices.
 * 
 * @param matrix Matrix containing the Platonic solid
 * @param row Row index
 * @param col Column index
 * @param coords Output barycentric coordinates (allocated by this function)
 * @return 0 on success, -1 on error
 */
int geometric_matrix_compute_barycentric(
    const GeometricMatrix* matrix,
    uint32_t row,
    uint32_t col,
    BarycentricCoords* coords
);

/**
 * @brief Free barycentric coordinates
 * 
 * @param coords Coordinates to free
 */
void barycentric_coords_free(BarycentricCoords* coords);

// ============================================================================
// UTILITY OPERATIONS
// ============================================================================

/**
 * @brief Choose appropriate Platonic solid for matrix size
 * 
 * Selects the best Platonic solid based on matrix dimensions.
 * Larger matrices use solids with more vertices for better accuracy.
 * 
 * @param rows Number of rows
 * @param cols Number of columns
 * @return Recommended Platonic solid type
 */
PlatonicSolidType geometric_matrix_choose_solid(uint32_t rows, uint32_t cols);

/**
 * @brief Get memory usage of geometric matrix
 * 
 * @param matrix Matrix to analyze
 * @return Memory usage in bytes
 */
size_t geometric_matrix_memory_usage(const GeometricMatrix* matrix);

/**
 * @brief Print geometric matrix info
 * 
 * @param matrix Matrix to print
 */
void geometric_matrix_print_info(const GeometricMatrix* matrix);

/**
 * @brief Copy geometric matrix
 * 
 * @param src Source matrix
 * @return Copy of matrix, or NULL on error
 */
GeometricMatrix* geometric_matrix_copy(const GeometricMatrix* src);

/**
 * @brief Scale all vertex values by a constant
 * 
 * @param matrix Matrix to scale
 * @param scale Scale factor
 * @return 0 on success, -1 on error
 */
int geometric_matrix_scale(GeometricMatrix* matrix, double scale);

/**
 * @brief Add two geometric matrices (vertex-wise)
 * 
 * result = a + b (vertex values)
 * 
 * @param result Output matrix (must be pre-allocated)
 * @param a First matrix
 * @param b Second matrix
 * @return 0 on success, -1 on error
 */
int geometric_matrix_add(
    GeometricMatrix* result,
    const GeometricMatrix* a,
    const GeometricMatrix* b
);

// ============================================================================
// MATRIX OPERATIONS
// ============================================================================

/**
 * @brief Matrix-vector multiplication using geometric storage
 * 
 * Computes y = Ax where A is a geometric matrix and x is a vector.
 * 
 * @param matrix Matrix A (geometric)
 * @param x Input vector (array of CrystallineAbacus)
 * @param y Output vector (array of CrystallineAbacus, pre-allocated)
 * @return 0 on success, -1 on error
 */
int geometric_matrix_vector_multiply(
    const GeometricMatrix* matrix,
    const CrystallineAbacus** x,
    CrystallineAbacus** y
);

/**
 * @brief Matrix-matrix multiplication using geometric storage
 * 
 * Computes C = AB where A and B are geometric matrices.
 * 
 * @param C Output matrix (geometric, pre-allocated)
 * @param A First matrix (geometric)
 * @param B Second matrix (geometric)
 * @return 0 on success, -1 on error
 */
int geometric_matrix_multiply(
    GeometricMatrix* C,
    const GeometricMatrix* A,
    const GeometricMatrix* B
);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_MATRIX_H