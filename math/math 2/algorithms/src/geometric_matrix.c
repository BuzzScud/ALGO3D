/**
 * @file geometric_matrix.c
 * @brief Implementation of Geometric Matrix Storage
 * 
 * This implements the true 88D architecture using Platonic solid coordinate
 * frames and barycentric interpolation for parameter storage.
 */

#define _USE_MATH_DEFINES
#include <math.h>  // Include standard math.h first
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "geometric_matrix.h"
#include "math/abacus.h"
#include "math/transcendental.h"  // For math_sin, math_cos, math_sqrt

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define PlatonicGeometry structure (simplified version)
struct PlatonicGeometry {
    uint32_t num_vertices;
    double* vertices;  // [num_vertices * 3]
    uint32_t num_edges;
    uint32_t num_faces;
};

// Forward declare the function we'll use
extern struct PlatonicGeometry* cllm_generate_platonic_solid(PlatonicSolidType solid);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Helper to copy abacus value (replaces copy_abacus_value which doesn't exist)
 */
static void copy_abacus_value(CrystallineAbacus* dest, const CrystallineAbacus* src) {
    if (!dest || !src) return;
    
    // Copy the value by copying beads
    dest->num_beads = src->num_beads;
    dest->negative = src->negative;
    for (size_t i = 0; i < src->num_beads && i < dest->capacity; i++) {
        dest->beads[i] = src->beads[i];
    }
}

/**
 * @brief Map (row, col) to normalized coordinates [0, 1] × [0, 1]
 */
static void normalize_coordinates(
    uint32_t row,
    uint32_t col,
    uint32_t rows,
    uint32_t cols,
    double* u,
    double* v
) {
    *u = (double)row / (double)(rows - 1);
    *v = (double)col / (double)(cols - 1);
}

/**
 * @brief Map normalized coordinates to 3D point on unit sphere
 * 
 * Uses spherical coordinates:
 * θ = 2π * u (azimuthal angle)
 * φ = π * v (polar angle)
 * 
 * x = sin(φ) * cos(θ)
 * y = sin(φ) * sin(θ)
 * z = cos(φ)
 */
static void map_to_sphere(double u, double v, double* x, double* y, double* z) {
    double theta = 2.0 * M_PI * u;
    double phi = M_PI * v;
    
    *x = math_sin(phi) * math_cos(theta);  // Use custom math library
    *y = math_sin(phi) * math_sin(theta);  // Use custom math library
    *z = math_cos(phi);  // Use custom math library
}

/**
 * @brief Compute distance between two 3D points
 */
static double distance_3d(double x1, double y1, double z1, double x2, double y2, double z2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dz = z2 - z1;
    return math_sqrt(dx*dx + dy*dy + dz*dz);  // Use custom math library
}

// ============================================================================
// PLATONIC SOLID SELECTION
// ============================================================================

PlatonicSolidType geometric_matrix_choose_solid(uint32_t rows, uint32_t cols) {
    size_t total = (size_t)rows * (size_t)cols;
    
    // Choose solid based on matrix size
    // Larger matrices benefit from more vertices for better interpolation
    if (total <= 100) {
        return PLATONIC_TETRAHEDRON;    // 4 vertices - smallest
    } else if (total <= 1000) {
        return PLATONIC_CUBE;           // 8 vertices
    } else if (total <= 5000) {
        return PLATONIC_OCTAHEDRON;     // 6 vertices
    } else if (total <= 20000) {
        return PLATONIC_DODECAHEDRON;   // 20 vertices - best for large matrices
    } else {
        return PLATONIC_ICOSAHEDRON;    // 12 vertices
    }
}

/**
 * @brief Get number of vertices for a Platonic solid
 */
static uint32_t get_num_vertices(PlatonicSolidType solid) {
    switch (solid) {
        case PLATONIC_TETRAHEDRON: return 4;
        case PLATONIC_CUBE: return 8;
        case PLATONIC_OCTAHEDRON: return 6;
        case PLATONIC_DODECAHEDRON: return 20;
        case PLATONIC_ICOSAHEDRON: return 12;
        default: return 4;
    }
}

/**
 * @brief Initialize vertex positions for a Platonic solid
 * 
 * Uses the existing Platonic solid generators from the codebase.
 */
static int initialize_vertex_positions(GeometricMatrix* matrix) {
    // Use existing Platonic solid generator
    PlatonicGeometry* geom = cllm_generate_platonic_solid(matrix->solid);
    if (!geom) {
        fprintf(stderr, "ERROR: Failed to generate Platonic solid\n");
        return -1;
    }
    
    // Allocate vertex positions
    matrix->vertex_positions = (double*)calloc(matrix->num_vertices * 3, sizeof(double));
    if (!matrix->vertex_positions) {
        fprintf(stderr, "ERROR: Failed to allocate vertex positions\n");
        return -1;
    }
    
    // Copy vertex positions from geometry
    // Note: PlatonicGeometry stores vertices as array of 3D points
    for (uint32_t i = 0; i < matrix->num_vertices && i < geom->num_vertices; i++) {
        matrix->vertex_positions[i * 3 + 0] = geom->vertices[i * 3 + 0];
        matrix->vertex_positions[i * 3 + 1] = geom->vertices[i * 3 + 1];
        matrix->vertex_positions[i * 3 + 2] = geom->vertices[i * 3 + 2];
    }
    
    // Note: We don't free geom here as it might be managed by the generator
    // TODO: Check if we need to free PlatonicGeometry
    
    return 0;
}

// ============================================================================
// CREATION AND DESTRUCTION
// ============================================================================

GeometricMatrix* geometric_matrix_create(
    uint32_t rows,
    uint32_t cols,
    const char* name
) {
    // Choose appropriate Platonic solid
    PlatonicSolidType solid = geometric_matrix_choose_solid(rows, cols);
    return geometric_matrix_create_with_solid(rows, cols, solid, name);
}

GeometricMatrix* geometric_matrix_create_with_solid(
    uint32_t rows,
    uint32_t cols,
    PlatonicSolidType solid,
    const char* name
) {
    if (rows == 0 || cols == 0) {
        fprintf(stderr, "ERROR: Invalid matrix dimensions: %u × %u\n", rows, cols);
        return NULL;
    }
    
    // Allocate structure
    GeometricMatrix* matrix = (GeometricMatrix*)calloc(1, sizeof(GeometricMatrix));
    if (!matrix) {
        fprintf(stderr, "ERROR: Failed to allocate GeometricMatrix\n");
        return NULL;
    }
    
    // Set basic properties
    matrix->solid = solid;
    matrix->num_vertices = get_num_vertices(solid);
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->is_initialized = false;
    
    // Copy name
    if (name) {
        strncpy(matrix->name, name, sizeof(matrix->name) - 1);
        matrix->name[sizeof(matrix->name) - 1] = '\0';
    } else {
        snprintf(matrix->name, sizeof(matrix->name), "matrix_%ux%u", rows, cols);
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&matrix->lock, NULL) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize mutex\n");
        free(matrix);
        return NULL;
    }
    
    // Allocate vertex values
    matrix->vertex_values = (CrystallineAbacus**)calloc(matrix->num_vertices, sizeof(CrystallineAbacus*));
    if (!matrix->vertex_values) {
        fprintf(stderr, "ERROR: Failed to allocate vertex values array\n");
        pthread_mutex_destroy(&matrix->lock);
        free(matrix);
        return NULL;
    }
    
    // Create CrystallineAbacus for each vertex
    for (uint32_t i = 0; i < matrix->num_vertices; i++) {
        matrix->vertex_values[i] = abacus_new(60);  // Base-60
        if (!matrix->vertex_values[i]) {
            fprintf(stderr, "ERROR: Failed to create abacus for vertex %u\n", i);
            // Cleanup already allocated
            for (uint32_t j = 0; j < i; j++) {
                abacus_free(matrix->vertex_values[j]);
            }
            free(matrix->vertex_values);
            pthread_mutex_destroy(&matrix->lock);
            free(matrix);
            return NULL;
        }
    }
    
    // Initialize vertex positions
    if (initialize_vertex_positions(matrix) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize vertex positions\n");
        for (uint32_t i = 0; i < matrix->num_vertices; i++) {
            abacus_free(matrix->vertex_values[i]);
        }
        free(matrix->vertex_values);
        pthread_mutex_destroy(&matrix->lock);
        free(matrix);
        return NULL;
    }
    
    // Initialize symmetry operations (12-fold symmetry)
    // For now, we'll use identity matrix
    // TODO: Implement proper 12-fold symmetry operations
    matrix->num_symmetries = 12;
    matrix->symmetry_ops = (double*)calloc(matrix->num_symmetries * 9, sizeof(double));
    if (!matrix->symmetry_ops) {
        fprintf(stderr, "ERROR: Failed to allocate symmetry operations\n");
        for (uint32_t i = 0; i < matrix->num_vertices; i++) {
            abacus_free(matrix->vertex_values[i]);
        }
        free(matrix->vertex_values);
        free(matrix->vertex_positions);
        pthread_mutex_destroy(&matrix->lock);
        free(matrix);
        return NULL;
    }
    
    // Initialize with identity matrices
    for (uint32_t i = 0; i < matrix->num_symmetries; i++) {
        double* mat = &matrix->symmetry_ops[i * 9];
        mat[0] = 1.0; mat[1] = 0.0; mat[2] = 0.0;
        mat[3] = 0.0; mat[4] = 1.0; mat[5] = 0.0;
        mat[6] = 0.0; mat[7] = 0.0; mat[8] = 1.0;
    }
    
    printf("Created geometric matrix '%s' [%u×%u] with %u vertices (%s)\n",
           matrix->name, rows, cols, matrix->num_vertices,
           solid == PLATONIC_TETRAHEDRON ? "Tetrahedron" :
           solid == PLATONIC_CUBE ? "Cube" :
           solid == PLATONIC_OCTAHEDRON ? "Octahedron" :
           solid == PLATONIC_DODECAHEDRON ? "Dodecahedron" :
           solid == PLATONIC_ICOSAHEDRON ? "Icosahedron" : "Unknown");
    
    return matrix;
}

void geometric_matrix_free(GeometricMatrix* matrix) {
    if (!matrix) return;
    
    // Free vertex values
    if (matrix->vertex_values) {
        for (uint32_t i = 0; i < matrix->num_vertices; i++) {
            if (matrix->vertex_values[i]) {
                abacus_free(matrix->vertex_values[i]);
            }
        }
        free(matrix->vertex_values);
    }
    
    // Free vertex positions
    if (matrix->vertex_positions) {
        free(matrix->vertex_positions);
    }
    
    // Free symmetry operations
    if (matrix->symmetry_ops) {
        free(matrix->symmetry_ops);
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&matrix->lock);
    
    // Free structure
    free(matrix);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

int geometric_matrix_init_zeros(GeometricMatrix* matrix) {
    if (!matrix) return -1;
    
    pthread_mutex_lock(&matrix->lock);
    
    for (uint32_t i = 0; i < matrix->num_vertices; i++) {
        abacus_init_zero(matrix->vertex_values[i]);
    }
    
    matrix->is_initialized = true;
    
    pthread_mutex_unlock(&matrix->lock);
    
    return 0;
}

int geometric_matrix_init_ones(GeometricMatrix* matrix) {
    if (!matrix) return -1;
    
    pthread_mutex_lock(&matrix->lock);
    
    CrystallineAbacus* one = abacus_from_uint64(1, 60);
    if (!one) {
        pthread_mutex_unlock(&matrix->lock);
        return -1;
    }
    
    for (uint32_t i = 0; i < matrix->num_vertices; i++) {
        // Copy one to vertex_values[i]
        abacus_free(matrix->vertex_values[i]);
        matrix->vertex_values[i] = abacus_copy(one);
    }
    
    abacus_free(one);
    matrix->is_initialized = true;
    
    pthread_mutex_unlock(&matrix->lock);
    
    return 0;
}

int geometric_matrix_init_uniform(
    GeometricMatrix* matrix,
    double min,
    double max,
    uint64_t seed
) {
    if (!matrix) return -1;
    
    pthread_mutex_lock(&matrix->lock);
    
    // Simple LCG for random numbers
    uint64_t state = seed;
    
    for (uint32_t i = 0; i < matrix->num_vertices; i++) {
        // Generate random value in [min, max]
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        double u = (double)(state >> 32) / (double)UINT32_MAX;
        double value = min + u * (max - min);
        
        // Convert to abacus
        CrystallineAbacus* val_abacus = abacus_from_double(value, 60, 10);
        if (val_abacus) {
            copy_abacus_value(matrix->vertex_values[i], val_abacus);
            abacus_free(val_abacus);
        }
    }
    
    matrix->is_initialized = true;
    
    pthread_mutex_unlock(&matrix->lock);
    
    return 0;
}

int geometric_matrix_init_xavier(
    GeometricMatrix* matrix,
    uint32_t fan_in,
    uint32_t fan_out,
    uint64_t seed
) {
    if (!matrix) return -1;
    
    // Xavier: U(-√(6/(fan_in + fan_out)), √(6/(fan_in + fan_out)))
    double limit = math_sqrt(6.0 / (double)(fan_in + fan_out));  // Use custom math library
    
    return geometric_matrix_init_uniform(matrix, -limit, limit, seed);
}

int geometric_matrix_init_he(
    GeometricMatrix* matrix,
    uint32_t fan_in,
    uint64_t seed
) {
    if (!matrix) return -1;
    
    // He: U(-√(6/fan_in), √(6/fan_in))
    double limit = math_sqrt(6.0 / (double)fan_in);  // Use custom math library
    
    return geometric_matrix_init_uniform(matrix, -limit, limit, seed);
}

// ============================================================================
// ACCESS OPERATIONS
// ============================================================================

int geometric_matrix_set_vertex(
    GeometricMatrix* matrix,
    uint32_t vertex_idx,
    const CrystallineAbacus* value
) {
    if (!matrix || !value) return -1;
    if (vertex_idx >= matrix->num_vertices) return -1;
    
    pthread_mutex_lock(&matrix->lock);
    
    copy_abacus_value(matrix->vertex_values[vertex_idx], value);
    
    pthread_mutex_unlock(&matrix->lock);
    
    return 0;
}

int geometric_matrix_get_vertex(
    const GeometricMatrix* matrix,
    uint32_t vertex_idx,
    CrystallineAbacus* result
) {
    if (!matrix || !result) return -1;
    if (vertex_idx >= matrix->num_vertices) return -1;
    
    pthread_mutex_lock((pthread_mutex_t*)&matrix->lock);
    
    copy_abacus_value(result, matrix->vertex_values[vertex_idx]);
    
    pthread_mutex_unlock((pthread_mutex_t*)&matrix->lock);
    
    return 0;
}

// ============================================================================
// BARYCENTRIC INTERPOLATION
// ============================================================================

int geometric_matrix_compute_barycentric(
    const GeometricMatrix* matrix,
    uint32_t row,
    uint32_t col,
    BarycentricCoords* coords
) {
    if (!matrix || !coords) return -1;
    
    // Normalize coordinates to [0, 1]
    double u, v;
    normalize_coordinates(row, col, matrix->rows, matrix->cols, &u, &v);
    
    // Map to 3D point on sphere
    double px, py, pz;
    map_to_sphere(u, v, &px, &py, &pz);
    
    // Find nearest vertices (use 4 nearest for tetrahedral interpolation)
    uint32_t k = 4;
    if (k > matrix->num_vertices) k = matrix->num_vertices;
    
    // Allocate arrays
    coords->vertex_indices = (uint32_t*)malloc(k * sizeof(uint32_t));
    coords->weights = (double*)malloc(k * sizeof(double));
    coords->num_vertices = k;
    
    if (!coords->vertex_indices || !coords->weights) {
        free(coords->vertex_indices);
        free(coords->weights);
        return -1;
    }
    
    // Find k nearest vertices
    double* distances = (double*)malloc(matrix->num_vertices * sizeof(double));
    if (!distances) {
        free(coords->vertex_indices);
        free(coords->weights);
        return -1;
    }
    
    for (uint32_t i = 0; i < matrix->num_vertices; i++) {
        double vx = matrix->vertex_positions[i * 3 + 0];
        double vy = matrix->vertex_positions[i * 3 + 1];
        double vz = matrix->vertex_positions[i * 3 + 2];
        distances[i] = distance_3d(px, py, pz, vx, vy, vz);
    }
    
    // Select k nearest (simple selection, not optimal but works)
    for (uint32_t i = 0; i < k; i++) {
        uint32_t min_idx = 0;
        double min_dist = distances[0];
        
        for (uint32_t j = 1; j < matrix->num_vertices; j++) {
            if (distances[j] < min_dist) {
                min_dist = distances[j];
                min_idx = j;
            }
        }
        
        coords->vertex_indices[i] = min_idx;
        distances[min_idx] = 1e10;  // Mark as used
    }
    
    free(distances);
    
    // Compute weights using inverse distance weighting
    double total_weight = 0.0;
    for (uint32_t i = 0; i < k; i++) {
        uint32_t v_idx = coords->vertex_indices[i];
        double vx = matrix->vertex_positions[v_idx * 3 + 0];
        double vy = matrix->vertex_positions[v_idx * 3 + 1];
        double vz = matrix->vertex_positions[v_idx * 3 + 2];
        double dist = distance_3d(px, py, pz, vx, vy, vz);
        
        // Inverse distance weight (with small epsilon to avoid division by zero)
        coords->weights[i] = 1.0 / (dist + 1e-10);
        total_weight += coords->weights[i];
    }
    
    // Normalize weights to sum to 1.0
    for (uint32_t i = 0; i < k; i++) {
        coords->weights[i] /= total_weight;
    }
    
    return 0;
}

void barycentric_coords_free(BarycentricCoords* coords) {
    if (!coords) return;
    
    if (coords->vertex_indices) {
        free(coords->vertex_indices);
        coords->vertex_indices = NULL;
    }
    
    if (coords->weights) {
        free(coords->weights);
        coords->weights = NULL;
    }
    
    coords->num_vertices = 0;
}

int geometric_matrix_get(
    const GeometricMatrix* matrix,
    uint32_t row,
    uint32_t col,
    CrystallineAbacus* result
) {
    if (!matrix || !result) return -1;
    if (row >= matrix->rows || col >= matrix->cols) return -1;
    
    // Compute barycentric coordinates
    BarycentricCoords coords;
    if (geometric_matrix_compute_barycentric(matrix, row, col, &coords) != 0) {
        return -1;
    }
    
    // Interpolate from vertex values
    abacus_init_zero(result);
    
    CrystallineAbacus* temp = abacus_new(60);
    CrystallineAbacus* weighted = abacus_new(60);
    
    for (uint32_t i = 0; i < coords.num_vertices; i++) {
        uint32_t v_idx = coords.vertex_indices[i];
        double weight = coords.weights[i];
        
        // Get vertex value
        copy_abacus_value(temp, matrix->vertex_values[v_idx]);
        
        // Multiply by weight
        CrystallineAbacus* weight_abacus = abacus_from_double(weight, 60, 10);
        abacus_mul(weighted, temp, weight_abacus);
        abacus_free(weight_abacus);
        
        // Add to result
        abacus_add(result, result, weighted);
    }
    
    abacus_free(temp);
    abacus_free(weighted);
    barycentric_coords_free(&coords);
    
    return 0;
}

int geometric_matrix_accumulate_gradient(
    GeometricMatrix* matrix,
    uint32_t row,
    uint32_t col,
    const CrystallineAbacus* gradient
) {
    if (!matrix || !gradient) return -1;
    if (row >= matrix->rows || col >= matrix->cols) return -1;
    
    // Compute barycentric coordinates
    BarycentricCoords coords;
    if (geometric_matrix_compute_barycentric(matrix, row, col, &coords) != 0) {
        return -1;
    }
    
    pthread_mutex_lock(&matrix->lock);
    
    // Distribute gradient to vertices using barycentric weights
    CrystallineAbacus* weighted_grad = abacus_new(60);
    
    for (uint32_t i = 0; i < coords.num_vertices; i++) {
        uint32_t v_idx = coords.vertex_indices[i];
        double weight = coords.weights[i];
        
        // Multiply gradient by weight
        CrystallineAbacus* weight_abacus = abacus_from_double(weight, 60, 10);
        abacus_mul(weighted_grad, gradient, weight_abacus);
        abacus_free(weight_abacus);
        
        // Add to vertex gradient
        abacus_add(matrix->vertex_values[v_idx], matrix->vertex_values[v_idx], weighted_grad);
    }
    
    abacus_free(weighted_grad);
    barycentric_coords_free(&coords);
    
    pthread_mutex_unlock(&matrix->lock);
    
    return 0;
}

// ============================================================================
// UTILITY OPERATIONS
// ============================================================================

size_t geometric_matrix_memory_usage(const GeometricMatrix* matrix) {
    if (!matrix) return 0;
    
    size_t total = sizeof(GeometricMatrix);
    
    // Vertex values
    total += matrix->num_vertices * sizeof(CrystallineAbacus*);
    total += matrix->num_vertices * sizeof(CrystallineAbacus);  // Approximate
    
    // Vertex positions
    total += matrix->num_vertices * 3 * sizeof(double);
    
    // Symmetry operations
    total += matrix->num_symmetries * 9 * sizeof(double);
    
    return total;
}

void geometric_matrix_print_info(const GeometricMatrix* matrix) {
    if (!matrix) return;
    
    printf("GeometricMatrix '%s':\n", matrix->name);
    printf("  Dimensions: %u × %u\n", matrix->rows, matrix->cols);
    printf("  Platonic solid: ");
    switch (matrix->solid) {
        case PLATONIC_TETRAHEDRON: printf("Tetrahedron"); break;
        case PLATONIC_CUBE: printf("Cube"); break;
        case PLATONIC_OCTAHEDRON: printf("Octahedron"); break;
        case PLATONIC_DODECAHEDRON: printf("Dodecahedron"); break;
        case PLATONIC_ICOSAHEDRON: printf("Icosahedron"); break;
        default: printf("Unknown"); break;
    }
    printf(" (%u vertices)\n", matrix->num_vertices);
    printf("  Memory usage: %zu bytes (%.2f KB)\n",
           geometric_matrix_memory_usage(matrix),
           geometric_matrix_memory_usage(matrix) / 1024.0);
    printf("  Initialized: %s\n", matrix->is_initialized ? "Yes" : "No");
}

// ============================================================================
// MATRIX OPERATIONS (Stubs for now)
// ============================================================================

int geometric_matrix_vector_multiply(
    const GeometricMatrix* matrix,
    const CrystallineAbacus** x,
    CrystallineAbacus** y
) {
    // TODO: Implement
    (void)matrix;
    (void)x;
    (void)y;
    fprintf(stderr, "ERROR: geometric_matrix_vector_multiply not yet implemented\n");
    return -1;
}

int geometric_matrix_multiply(
    GeometricMatrix* C,
    const GeometricMatrix* A,
    const GeometricMatrix* B
) {
    // TODO: Implement
    (void)C;
    (void)A;
    (void)B;
    fprintf(stderr, "ERROR: geometric_matrix_multiply not yet implemented\n");
    return -1;
}

GeometricMatrix* geometric_matrix_copy(const GeometricMatrix* src) {
    // TODO: Implement
    (void)src;
    fprintf(stderr, "ERROR: geometric_matrix_copy not yet implemented\n");
    return NULL;
}

int geometric_matrix_scale(GeometricMatrix* matrix, double scale) {
    // TODO: Implement
    (void)matrix;
    (void)scale;
    fprintf(stderr, "ERROR: geometric_matrix_scale not yet implemented\n");
    return -1;
}

int geometric_matrix_add(
    GeometricMatrix* result,
    const GeometricMatrix* a,
    const GeometricMatrix* b
) {
    // TODO: Implement
    (void)result;
    (void)a;
    (void)b;
    fprintf(stderr, "ERROR: geometric_matrix_add not yet implemented\n");
    return -1;
}