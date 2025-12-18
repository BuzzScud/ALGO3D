/**
 * @file cllm_abacus_matrix.h
 * @brief Matrix operations using CrystallineAbacus for arbitrary precision
 * 
 * This module provides matrix data structures and operations using the
 * CrystallineAbacus for arbitrary precision arithmetic. This enables:
 * - True arbitrary precision (no rounding errors)
 * - Geometric precision in higher dimensions
 * - Exact gradient computation
 * - Base-12 arithmetic aligned with geometric foundation
 * 
 * Design:
 * - Row-major storage for cache efficiency
 * - Base-12 default for geometric alignment
 * - Configurable precision (fractional digits)
 * - Sparse representation support (future)
 */

#ifndef CLLM_ABACUS_MATRIX_H
#define CLLM_ABACUS_MATRIX_H

#include "../math/include/math/abacus.h"
#include "../math/include/math/types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ABACUS MATRIX STRUCTURE
 * ============================================================================
 */

/**
 * @brief Matrix using CrystallineAbacus for arbitrary precision
 * 
 * Stores a 2D matrix where each element is a CrystallineAbacus.
 * Uses row-major order: element (i,j) is at data[i * cols + j]
 * 
 * Memory Layout:
 * - data[0..cols-1]: First row
 * - data[cols..2*cols-1]: Second row
 * - etc.
 * 
 * Example: 3x2 matrix
 *   [a b]
 *   [c d]  -> data = [a, b, c, d, e, f]
 *   [e f]
 */
typedef struct {
    CrystallineAbacus** data;    /**< Array of abacus pointers [rows * cols] */
    uint32_t rows;               /**< Number of rows */
    uint32_t cols;               /**< Number of columns */
    uint32_t base;               /**< Number base (12 for geometric alignment) */
    int32_t precision;           /**< Number of fractional digits */
    bool initialized;            /**< True if data is allocated and initialized */
} AbacusMatrix;

/* ============================================================================
 * CREATION AND DESTRUCTION
 * ============================================================================
 */

/**
 * @brief Create a new abacus matrix
 * @param rows Number of rows
 * @param cols Number of columns
 * @param base Number base (12 recommended for geometric alignment)
 * @param precision Number of fractional digits (10 recommended)
 * @return Pointer to new matrix, or NULL on error
 * 
 * Creates an uninitialized matrix. Call abacus_matrix_init_zero() or
 * abacus_matrix_from_doubles() to initialize values.
 */
AbacusMatrix* abacus_matrix_create(uint32_t rows, uint32_t cols, uint32_t base, int32_t precision);

/**
 * @brief Free an abacus matrix
 * @param matrix Matrix to free (can be NULL)
 * 
 * Frees all CrystallineAbacus elements and the matrix structure itself.
 */
void abacus_matrix_free(AbacusMatrix* matrix);

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

/**
 * @brief Initialize all elements to zero
 * @param matrix Matrix to initialize
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_init_zero(AbacusMatrix* matrix);

/**
 * @brief Initialize with random values
 * @param matrix Matrix to initialize
 * @param scale Scale factor for random values (e.g., 0.01 for small weights)
 * @return MATH_SUCCESS or error code
 * 
 * Initializes each element with random value in range [-scale, scale]
 * Uses uniform distribution.
 */
MathError abacus_matrix_init_random(AbacusMatrix* matrix, double scale);

/**
 * @brief Initialize with Xavier/Glorot initialization
 * @param matrix Matrix to initialize
 * @param fan_in Number of input units
 * @param fan_out Number of output units
 * @return MATH_SUCCESS or error code
 * 
 * Uses Xavier initialization: scale = sqrt(6 / (fan_in + fan_out))
 * Good for tanh/sigmoid activations.
 */
MathError abacus_matrix_init_xavier(AbacusMatrix* matrix, uint32_t fan_in, uint32_t fan_out);

/**
 * @brief Initialize with He initialization
 * @param matrix Matrix to initialize
 * @param fan_in Number of input units
 * @return MATH_SUCCESS or error code
 * 
 * Uses He initialization: scale = sqrt(2 / fan_in)
 * Good for ReLU activations.
 */
MathError abacus_matrix_init_he(AbacusMatrix* matrix, uint32_t fan_in);

/* ============================================================================
 * CONVERSION
 * ============================================================================
 */

/**
 * @brief Initialize matrix from double array
 * @param matrix Matrix to initialize
 * @param data Double array [rows * cols] in row-major order
 * @return MATH_SUCCESS or error code
 * 
 * Converts each double to CrystallineAbacus with specified precision.
 */
MathError abacus_matrix_from_doubles(AbacusMatrix* matrix, const double* data);

/**
 * @brief Convert matrix to double array
 * @param matrix Matrix to convert
 * @param data Output double array [rows * cols] (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 * 
 * Converts each CrystallineAbacus to double.
 * May lose precision if abacus has more digits than double can represent.
 */
MathError abacus_matrix_to_doubles(const AbacusMatrix* matrix, double* data);

/**
 * @brief Get a single row as doubles
 * 
 * Converts one row of the matrix to double precision values.
 * 
 * @param matrix Matrix to read from
 * @param row Row index
 * @param output Output array [cols]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError abacus_matrix_get_row_as_doubles(
    const AbacusMatrix* matrix,
    uint32_t row,
    double* output
);

/**
 * @brief Set a single row from doubles
 * 
 * Sets one row of the matrix from double precision values.
 * 
 * @param matrix Matrix to write to
 * @param row Row index
 * @param input Input array [cols]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError abacus_matrix_set_row_from_doubles(
    AbacusMatrix* matrix,
    uint32_t row,
    const double* input
);

/* ============================================================================
 * ELEMENT ACCESS
 * ============================================================================
 */

/**
 * @brief Get element at (row, col)
 * @param matrix Matrix to access
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @return Pointer to CrystallineAbacus, or NULL if out of bounds
 * 
 * Returns pointer to internal abacus. Do not free!
 */
CrystallineAbacus* abacus_matrix_get(const AbacusMatrix* matrix, uint32_t row, uint32_t col);

/**
 * @brief Set element at (row, col)
 * @param matrix Matrix to modify
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @param value Value to set (will be copied)
 * @return MATH_SUCCESS or error code
 * 
 * Copies the value into the matrix. Original value is not modified.
 */
MathError abacus_matrix_set(AbacusMatrix* matrix, uint32_t row, uint32_t col, const CrystallineAbacus* value);

/**
 * @brief Get element as double
 * @param matrix Matrix to access
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @param value Output value
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_get_double(const AbacusMatrix* matrix, uint32_t row, uint32_t col, double* value);

/**
 * @brief Set element from double
 * @param matrix Matrix to modify
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @param value Value to set
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_set_double(AbacusMatrix* matrix, uint32_t row, uint32_t col, double value);

/* ============================================================================
 * MATRIX OPERATIONS
 * ============================================================================
 */

/**
 * @brief Matrix addition: result = A + B
 * @param result Output matrix (must be pre-allocated, same size as A and B)
 * @param A First matrix
 * @param B Second matrix
 * @return MATH_SUCCESS or error code
 * 
 * Element-wise addition. A and B must have same dimensions.
 */
MathError abacus_matrix_add(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* B);

/**
 * @brief Matrix subtraction: result = A - B
 * @param result Output matrix (must be pre-allocated, same size as A and B)
 * @param A First matrix
 * @param B Second matrix
 * @return MATH_SUCCESS or error code
 * 
 * Element-wise subtraction. A and B must have same dimensions.
 */
MathError abacus_matrix_sub(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* B);

/**
 * @brief Matrix multiplication: result = A × B
 * @param result Output matrix (must be pre-allocated, size [A.rows × B.cols])
 * @param A First matrix [m × n]
 * @param B Second matrix [n × p]
 * @return MATH_SUCCESS or error code
 * 
 * Standard matrix multiplication. A.cols must equal B.rows.
 * Complexity: O(m × n × p)
 */
MathError abacus_matrix_mul(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* B);

/**
 * @brief Matrix-vector multiplication: result = A × v
 * @param result Output vector (must be pre-allocated, size [A.rows × 1])
 * @param A Matrix [m × n]
 * @param v Vector [n × 1]
 * @return MATH_SUCCESS or error code
 * 
 * Optimized for matrix-vector case.
 */
MathError abacus_matrix_mul_vector(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* v);

/**
 * @brief Scalar multiplication: result = scalar × matrix
 * @param result Output matrix (can be same as input for in-place operation)
 * @param matrix Input matrix
 * @param scalar Scalar value
 * @return MATH_SUCCESS or error code
 * 
 * Multiplies each element by scalar.
 */
MathError abacus_matrix_scale(AbacusMatrix* result, const AbacusMatrix* matrix, const CrystallineAbacus* scalar);

/**
 * @brief Scalar multiplication (double): result = scalar × matrix
 * @param result Output matrix (can be same as input for in-place operation)
 * @param matrix Input matrix
 * @param scalar Scalar value (double)
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_scale_double(AbacusMatrix* result, const AbacusMatrix* matrix, double scalar);

/**
 * @brief Matrix transpose: result = A^T
 * @param result Output matrix (must be pre-allocated, size [A.cols × A.rows])
 * @param A Input matrix [m × n]
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_transpose(AbacusMatrix* result, const AbacusMatrix* A);

/**
 * @brief Element-wise multiplication (Hadamard product): result = A ⊙ B
 * @param result Output matrix (must be pre-allocated, same size as A and B)
 * @param A First matrix
 * @param B Second matrix
 * @return MATH_SUCCESS or error code
 * 
 * Element-wise multiplication. A and B must have same dimensions.
 */
MathError abacus_matrix_hadamard(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* B);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Copy matrix
 * @param src Source matrix
 * @return Pointer to new matrix (deep copy), or NULL on error
 */
AbacusMatrix* abacus_matrix_copy(const AbacusMatrix* src);

/**
 * @brief Print matrix for debugging
 * @param matrix Matrix to print
 * @param max_rows Maximum rows to print (0 = all)
 * @param max_cols Maximum columns to print (0 = all)
 * 
 * Prints matrix in readable format. Large matrices are truncated.
 */
void abacus_matrix_print(const AbacusMatrix* matrix, uint32_t max_rows, uint32_t max_cols);

/**
 * @brief Get memory usage in bytes
 * @param matrix Matrix to analyze
 * @return Total memory usage in bytes
 * 
 * Includes matrix structure, data array, and all CrystallineAbacus elements.
 */
size_t abacus_matrix_memory_usage(const AbacusMatrix* matrix);

/**
 * @brief Check if two matrices are equal (within tolerance)
 * @param A First matrix
 * @param B Second matrix
 * @param tolerance Maximum difference for equality (in double representation)
 * @return true if equal within tolerance
 */
bool abacus_matrix_equals(const AbacusMatrix* A, const AbacusMatrix* B, double tolerance);

/**
 * @brief Compute Frobenius norm: ||A||_F = sqrt(sum(A_ij^2))
 * @param matrix Input matrix
 * @param norm Output norm value
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_norm_frobenius(const AbacusMatrix* matrix, double* norm);

/**
 * @brief Compute sum of all elements
 * @param matrix Input matrix
 * @param sum Output sum (as CrystallineAbacus)
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_sum(const AbacusMatrix* matrix, CrystallineAbacus* sum);

/**
 * @brief Compute sum of all elements (as double)
 * @param matrix Input matrix
 * @param sum Output sum
 * @return MATH_SUCCESS or error code
 */
MathError abacus_matrix_sum_double(const AbacusMatrix* matrix, double* sum);

/* ============================================================================
 * VALIDATION
 * ============================================================================
 */

/**
 * @brief Validate matrix structure
 * @param matrix Matrix to validate
 * @return true if valid (all elements initialized, dimensions consistent)
 */
bool abacus_matrix_is_valid(const AbacusMatrix* matrix);

/**
 * @brief Check if matrix contains any NaN or infinite values
 * @param matrix Matrix to check
 * @return true if all values are finite
 */
bool abacus_matrix_is_finite(const AbacusMatrix* matrix);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_ABACUS_MATRIX_H */