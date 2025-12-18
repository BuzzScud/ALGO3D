/**
 * @file cllm_abacus_matrix.c
 * @brief Implementation of matrix operations using CrystallineAbacus
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/transcendental.h"
#include "ai/cllm_abacus_matrix.h"

// Error code mappings
#define MATH_ERROR_ALLOCATION MATH_ERROR_OUT_OF_MEMORY
#define MATH_ERROR_INVALID_ARGUMENT MATH_ERROR_INVALID_ARG
#define MATH_ERROR_INVALID_STATE MATH_ERROR_INVALID_ARG
#define MATH_ERROR_DIMENSION_MISMATCH MATH_ERROR_INVALID_ARG

/* ============================================================================
 * CREATION AND DESTRUCTION
 * ============================================================================
 */

AbacusMatrix* abacus_matrix_create(uint32_t rows, uint32_t cols, uint32_t base, int32_t precision) {
    if (rows == 0 || cols == 0 || base < 2) {
        return NULL;
    }
    
    AbacusMatrix* matrix = (AbacusMatrix*)malloc(sizeof(AbacusMatrix));
    if (!matrix) {
        return NULL;
    }
    
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->base = base;
    matrix->precision = precision;
    matrix->initialized = false;
    
    // Allocate array of pointers
    size_t total_elements = (size_t)rows * cols;
    matrix->data = (CrystallineAbacus**)calloc(total_elements, sizeof(CrystallineAbacus*));
    if (!matrix->data) {
        free(matrix);
        return NULL;
    }
    
    // Create each abacus element
    for (size_t i = 0; i < total_elements; i++) {
        matrix->data[i] = abacus_new(base);
        if (!matrix->data[i]) {
            // Cleanup on failure
            for (size_t j = 0; j < i; j++) {
                abacus_free(matrix->data[j]);
            }
            free(matrix->data);
            free(matrix);
            return NULL;
        }
        
        // Initialize to zero
        abacus_init_zero(matrix->data[i]);
        
        // Set precision
        abacus_set_precision(matrix->data[i], precision);
    }
    
    matrix->initialized = true;
    return matrix;
}

void abacus_matrix_free(AbacusMatrix* matrix) {
    if (!matrix) {
        return;
    }
    
    if (matrix->data) {
        size_t total_elements = (size_t)matrix->rows * matrix->cols;
        for (size_t i = 0; i < total_elements; i++) {
            if (matrix->data[i]) {
                abacus_free(matrix->data[i]);
            }
        }
        free(matrix->data);
    }
    
    free(matrix);
}

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

MathError abacus_matrix_init_zero(AbacusMatrix* matrix) {
    if (!matrix || !matrix->data) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    size_t total_elements = (size_t)matrix->rows * matrix->cols;
    for (size_t i = 0; i < total_elements; i++) {
        MathError err = abacus_init_zero(matrix->data[i]);
        if (err != MATH_SUCCESS) {
            return err;
        }
    }
    
    matrix->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_init_random(AbacusMatrix* matrix, double scale) {
    if (!matrix || !matrix->data) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    size_t total_elements = (size_t)matrix->rows * matrix->cols;
    for (size_t i = 0; i < total_elements; i++) {
        // Generate random value in [-scale, scale]
        double random_value = ((double)rand() / RAND_MAX) * 2.0 * scale - scale;
        
        // Convert to abacus
        CrystallineAbacus* temp = abacus_from_double(random_value, matrix->base, matrix->precision);
        if (!temp) {
            return MATH_ERROR_ALLOCATION;
        }
        
        // Copy to matrix element
        abacus_free(matrix->data[i]);
        matrix->data[i] = temp;
    }
    
    matrix->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_init_xavier(AbacusMatrix* matrix, uint32_t fan_in, uint32_t fan_out) {
    if (!matrix || !matrix->data) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Xavier initialization: scale = sqrt(6 / (fan_in + fan_out))
    double scale = math_sqrt(6.0 / (fan_in + fan_out));
    
    return abacus_matrix_init_random(matrix, scale);
}

MathError abacus_matrix_init_he(AbacusMatrix* matrix, uint32_t fan_in) {
    if (!matrix || !matrix->data) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // He initialization: scale = sqrt(2 / fan_in)
    double scale = math_sqrt(2.0 / fan_in);
    
    return abacus_matrix_init_random(matrix, scale);
}

/* ============================================================================
 * CONVERSION
 * ============================================================================
 */

MathError abacus_matrix_from_doubles(AbacusMatrix* matrix, const double* data) {
    if (!matrix || !matrix->data || !data) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    size_t total_elements = (size_t)matrix->rows * matrix->cols;
    for (size_t i = 0; i < total_elements; i++) {
        // Convert double to abacus
        CrystallineAbacus* temp = abacus_from_double(data[i], matrix->base, matrix->precision);
        if (!temp) {
            return MATH_ERROR_ALLOCATION;
        }
        
        // Replace existing abacus
        abacus_free(matrix->data[i]);
        matrix->data[i] = temp;
    }
    
    matrix->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_to_doubles(const AbacusMatrix* matrix, double* data) {
    if (!matrix || !matrix->data || !data) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!matrix->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    size_t total_elements = (size_t)matrix->rows * matrix->cols;
    for (size_t i = 0; i < total_elements; i++) {
        MathError err = abacus_to_double(matrix->data[i], &data[i]);
        if (err != MATH_SUCCESS) {
            return err;
        }
    }
    
    return MATH_SUCCESS;
}

/**
 * Get a single row as doubles
 */
MathError abacus_matrix_get_row_as_doubles(
    const AbacusMatrix* matrix,
    uint32_t row,
    double* output
) {
    if (!matrix || !output) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (row >= matrix->rows) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Convert each element in the row
    for (uint32_t col = 0; col < matrix->cols; col++) {
        MathError err = abacus_matrix_get_double(matrix, row, col, &output[col]);
        if (err != MATH_SUCCESS) {
            return err;
        }
    }
    
    return MATH_SUCCESS;
}

/**
 * Set a single row from doubles
 */
MathError abacus_matrix_set_row_from_doubles(
    AbacusMatrix* matrix,
    uint32_t row,
    const double* input
) {
    if (!matrix || !input) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (row >= matrix->rows) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Set each element in the row
    for (uint32_t col = 0; col < matrix->cols; col++) {
        MathError err = abacus_matrix_set_double(matrix, row, col, input[col]);
        if (err != MATH_SUCCESS) {
            return err;
        }
    }
    
    return MATH_SUCCESS;
}

/* ============================================================================
 * ELEMENT ACCESS
 * ============================================================================
 */

CrystallineAbacus* abacus_matrix_get(const AbacusMatrix* matrix, uint32_t row, uint32_t col) {
    if (!matrix || !matrix->data) {
        return NULL;
    }
    
    if (row >= matrix->rows || col >= matrix->cols) {
        return NULL;
    }
    
    size_t index = (size_t)row * matrix->cols + col;
    return matrix->data[index];
}

MathError abacus_matrix_set(AbacusMatrix* matrix, uint32_t row, uint32_t col, const CrystallineAbacus* value) {
    if (!matrix || !matrix->data || !value) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (row >= matrix->rows || col >= matrix->cols) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    size_t index = (size_t)row * matrix->cols + col;
    
    // Copy the value
    CrystallineAbacus* copy = abacus_copy(value);
    if (!copy) {
        return MATH_ERROR_ALLOCATION;
    }
    
    // Replace existing value
    abacus_free(matrix->data[index]);
    matrix->data[index] = copy;
    
    return MATH_SUCCESS;
}

MathError abacus_matrix_get_double(const AbacusMatrix* matrix, uint32_t row, uint32_t col, double* value) {
    if (!value) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    CrystallineAbacus* abacus = abacus_matrix_get(matrix, row, col);
    if (!abacus) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    return abacus_to_double(abacus, value);
}

MathError abacus_matrix_set_double(AbacusMatrix* matrix, uint32_t row, uint32_t col, double value) {
    if (!matrix || !matrix->data) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (row >= matrix->rows || col >= matrix->cols) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Convert double to abacus
    CrystallineAbacus* temp = abacus_from_double(value, matrix->base, matrix->precision);
    if (!temp) {
        return MATH_ERROR_ALLOCATION;
    }
    
    size_t index = (size_t)row * matrix->cols + col;
    
    // Replace existing value
    abacus_free(matrix->data[index]);
    matrix->data[index] = temp;
    
    return MATH_SUCCESS;
}

/* ============================================================================
 * MATRIX OPERATIONS
 * ============================================================================
 */

MathError abacus_matrix_add(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* B) {
    if (!result || !A || !B) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!A->initialized || !B->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    // Check dimensions
    if (A->rows != B->rows || A->cols != B->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    if (result->rows != A->rows || result->cols != A->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    // Element-wise addition
    size_t total_elements = (size_t)A->rows * A->cols;
    for (size_t i = 0; i < total_elements; i++) {
        MathError err = abacus_add(result->data[i], A->data[i], B->data[i]);
        if (err != MATH_SUCCESS) {
            return err;
        }
    }
    
    result->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_sub(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* B) {
    if (!result || !A || !B) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!A->initialized || !B->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    // Check dimensions
    if (A->rows != B->rows || A->cols != B->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    if (result->rows != A->rows || result->cols != A->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    // Element-wise subtraction
    size_t total_elements = (size_t)A->rows * A->cols;
    for (size_t i = 0; i < total_elements; i++) {
        MathError err = abacus_sub(result->data[i], A->data[i], B->data[i]);
        if (err != MATH_SUCCESS) {
            return err;
        }
    }
    
    result->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_mul(AbacusMatrix* result, const AbacusMatrix* A, const AbacusMatrix* B) {
    if (!result || !A || !B) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!A->initialized || !B->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    // Check dimensions: A is [m × n], B is [n × p], result is [m × p]
    if (A->cols != B->rows) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    if (result->rows != A->rows || result->cols != B->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    uint32_t m = A->rows;
    uint32_t n = A->cols;
    uint32_t p = B->cols;
    
    // Initialize result to zero
    MathError err = abacus_matrix_init_zero(result);
    if (err != MATH_SUCCESS) {
        return err;
    }
    
    // Matrix multiplication: C[i,j] = sum(A[i,k] * B[k,j])
    for (uint32_t i = 0; i < m; i++) {
        for (uint32_t j = 0; j < p; j++) {
            CrystallineAbacus* sum = abacus_new(result->base);
            if (!sum) {
                return MATH_ERROR_ALLOCATION;
            }
            abacus_init_zero(sum);
            abacus_set_precision(sum, result->precision);
            
            for (uint32_t k = 0; k < n; k++) {
                // Get A[i,k] and B[k,j]
                CrystallineAbacus* a_ik = abacus_matrix_get(A, i, k);
                CrystallineAbacus* b_kj = abacus_matrix_get(B, k, j);
                
                if (!a_ik || !b_kj) {
                    abacus_free(sum);
                    return MATH_ERROR_NULL_POINTER;
                }
                
                // Compute product
                CrystallineAbacus* product = abacus_new(result->base);
                if (!product) {
                    abacus_free(sum);
                    return MATH_ERROR_ALLOCATION;
                }
                abacus_set_precision(product, result->precision);
                
                err = abacus_mul(product, a_ik, b_kj);
                if (err != MATH_SUCCESS) {
                    abacus_free(product);
                    abacus_free(sum);
                    return err;
                }
                
                // Normalize: convert to double and back to fix exponent issues
                double product_val;
                err = abacus_to_double(product, &product_val);
                abacus_free(product);
                if (err != MATH_SUCCESS) {
                    abacus_free(sum);
                    return err;
                }
                
                product = abacus_from_double(product_val, result->base, result->precision);
                if (!product) {
                    abacus_free(sum);
                    return MATH_ERROR_OUT_OF_MEMORY;
                }
                
                // Add to sum
                CrystallineAbacus* new_sum = abacus_new(result->base);
                if (!new_sum) {
                    abacus_free(product);
                    abacus_free(sum);
                    return MATH_ERROR_ALLOCATION;
                }
                abacus_set_precision(new_sum, result->precision);
                
                err = abacus_add(new_sum, sum, product);
                abacus_free(product);
                
                if (err != MATH_SUCCESS) {
                    abacus_free(new_sum);
                    abacus_free(sum);
                    return err;
                }
                
                abacus_free(sum);
                sum = new_sum;
            }
            
            // Set result[i,j] = sum
            size_t index = (size_t)i * result->cols + j;
            abacus_free(result->data[index]);
            result->data[index] = sum;
        }
    }
    
    result->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_scale(AbacusMatrix* result, const AbacusMatrix* matrix, const CrystallineAbacus* scalar) {
    if (!result || !matrix || !scalar) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!matrix->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    if (result->rows != matrix->rows || result->cols != matrix->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    // Element-wise multiplication by scalar
    size_t total_elements = (size_t)matrix->rows * matrix->cols;
    for (size_t i = 0; i < total_elements; i++) {
        // Multiply
        CrystallineAbacus* temp = abacus_new(result->base);
        if (!temp) {
            return MATH_ERROR_ALLOCATION;
        }
        abacus_set_precision(temp, result->precision);
        
        MathError err = abacus_mul(temp, matrix->data[i], scalar);
        if (err != MATH_SUCCESS) {
            abacus_free(temp);
            return err;
        }
        
        // Normalize: convert to double and back
        double temp_val;
        err = abacus_to_double(temp, &temp_val);
        abacus_free(temp);
        if (err != MATH_SUCCESS) {
            return err;
        }
        
        // Free old value and assign new one
        abacus_free(result->data[i]);
        result->data[i] = abacus_from_double(temp_val, result->base, result->precision);
        if (!result->data[i]) {
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        if (err != MATH_SUCCESS) {
            return err;
        }
    }
    
    result->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_scale_double(AbacusMatrix* result, const AbacusMatrix* matrix, double scalar) {
    if (!result || !matrix) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Convert scalar to abacus
    CrystallineAbacus* scalar_abacus = abacus_from_double(scalar, matrix->base, matrix->precision);
    if (!scalar_abacus) {
        return MATH_ERROR_ALLOCATION;
    }
    
    MathError err = abacus_matrix_scale(result, matrix, scalar_abacus);
    abacus_free(scalar_abacus);
    
    return err;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

AbacusMatrix* abacus_matrix_copy(const AbacusMatrix* src) {
    if (!src || !src->data || !src->initialized) {
        return NULL;
    }
    
    AbacusMatrix* copy = abacus_matrix_create(src->rows, src->cols, src->base, src->precision);
    if (!copy) {
        return NULL;
    }
    
    // Copy each element
    size_t total_elements = (size_t)src->rows * src->cols;
    for (size_t i = 0; i < total_elements; i++) {
        abacus_free(copy->data[i]);
        copy->data[i] = abacus_copy(src->data[i]);
        if (!copy->data[i]) {
            abacus_matrix_free(copy);
            return NULL;
        }
    }
    
    copy->initialized = true;
    return copy;
}

void abacus_matrix_print(const AbacusMatrix* matrix, uint32_t max_rows, uint32_t max_cols) {
    if (!matrix || !matrix->data || !matrix->initialized) {
        printf("Matrix: NULL or uninitialized\n");
        return;
    }
    
    printf("AbacusMatrix [%u × %u] (base=%u, precision=%d):\n", 
           matrix->rows, matrix->cols, matrix->base, matrix->precision);
    
    uint32_t rows_to_print = (max_rows == 0 || max_rows > matrix->rows) ? matrix->rows : max_rows;
    uint32_t cols_to_print = (max_cols == 0 || max_cols > matrix->cols) ? matrix->cols : max_cols;
    
    for (uint32_t i = 0; i < rows_to_print; i++) {
        printf("  [");
        for (uint32_t j = 0; j < cols_to_print; j++) {
            double value;
            MathError err = abacus_matrix_get_double(matrix, i, j, &value);
            if (err == MATH_SUCCESS) {
                printf("%10.6f", value);
            } else {
                printf("     ERROR");
            }
            
            if (j < cols_to_print - 1) {
                printf(", ");
            }
        }
        
        if (cols_to_print < matrix->cols) {
            printf(", ... (%u more)", matrix->cols - cols_to_print);
        }
        
        printf("]\n");
    }
    
    if (rows_to_print < matrix->rows) {
        printf("  ... (%u more rows)\n", matrix->rows - rows_to_print);
    }
}

size_t abacus_matrix_memory_usage(const AbacusMatrix* matrix) {
    if (!matrix) {
        return 0;
    }
    
    size_t total = sizeof(AbacusMatrix);
    
    if (matrix->data) {
        // Pointer array
        total += (size_t)matrix->rows * matrix->cols * sizeof(CrystallineAbacus*);
        
        // Each abacus element
        size_t total_elements = (size_t)matrix->rows * matrix->cols;
        for (size_t i = 0; i < total_elements; i++) {
            if (matrix->data[i]) {
                total += abacus_memory_usage(matrix->data[i]);
            }
        }
    }
    
    return total;
}

MathError abacus_matrix_sum_double(const AbacusMatrix* matrix, double* sum) {
    if (!matrix || !sum) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!matrix->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    *sum = 0.0;
    
    size_t total_elements = (size_t)matrix->rows * matrix->cols;
    for (size_t i = 0; i < total_elements; i++) {
        double value;
        MathError err = abacus_to_double(matrix->data[i], &value);
        if (err != MATH_SUCCESS) {
            return err;
        }
        *sum += value;
    }
    
    return MATH_SUCCESS;
}

bool abacus_matrix_is_valid(const AbacusMatrix* matrix) {
    if (!matrix || !matrix->data) {
        return false;
    }
    
    if (matrix->rows == 0 || matrix->cols == 0) {
        return false;
    }
    
    if (matrix->base < 2) {
        return false;
    }
    
    if (!matrix->initialized) {
        return false;
    }
    
    // Check all elements are non-NULL
    size_t total_elements = (size_t)matrix->rows * matrix->cols;
    for (size_t i = 0; i < total_elements; i++) {
        if (!matrix->data[i]) {
            return false;
        }
    }
    
    return true;
}
MathError abacus_matrix_transpose(AbacusMatrix* result, const AbacusMatrix* matrix) {
    if (!result || !matrix) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!matrix->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    // Result must have swapped dimensions
    if (result->rows != matrix->cols || result->cols != matrix->rows) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    // Transpose: result[j][i] = matrix[i][j]
    for (uint32_t i = 0; i < matrix->rows; i++) {
        for (uint32_t j = 0; j < matrix->cols; j++) {
            CrystallineAbacus* src = abacus_matrix_get(matrix, i, j);
            if (!src) {
                return MATH_ERROR_NULL_POINTER;
            }
            
            MathError err = abacus_matrix_set(result, j, i, src);
            if (err != MATH_SUCCESS) {
                return err;
            }
        }
    }
    
    result->initialized = true;
    return MATH_SUCCESS;
}

MathError abacus_matrix_hadamard(AbacusMatrix* result, const AbacusMatrix* a, const AbacusMatrix* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (!a->initialized || !b->initialized) {
        return MATH_ERROR_INVALID_STATE;
    }
    
    // All matrices must have same dimensions
    if (a->rows != b->rows || a->cols != b->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    if (result->rows != a->rows || result->cols != a->cols) {
        return MATH_ERROR_DIMENSION_MISMATCH;
    }
    
    // Element-wise multiplication
    for (uint32_t i = 0; i < a->rows; i++) {
        for (uint32_t j = 0; j < a->cols; j++) {
            CrystallineAbacus* a_elem = abacus_matrix_get(a, i, j);
            CrystallineAbacus* b_elem = abacus_matrix_get(b, i, j);
            
            if (!a_elem || !b_elem) {
                return MATH_ERROR_NULL_POINTER;
            }
            
            // Multiply elements
            CrystallineAbacus* product = abacus_new(result->base);
            if (!product) {
                return MATH_ERROR_OUT_OF_MEMORY;
            }
            abacus_set_precision(product, result->precision);
            
            MathError err = abacus_mul(product, a_elem, b_elem);
            if (err != MATH_SUCCESS) {
                abacus_free(product);
                return err;
            }
            
            // Normalize: convert to double and back
            double product_val;
            err = abacus_to_double(product, &product_val);
            abacus_free(product);
            if (err != MATH_SUCCESS) {
                return err;
            }
            
            product = abacus_from_double(product_val, result->base, result->precision);
            if (!product) {
                return MATH_ERROR_OUT_OF_MEMORY;
            }
            
            err = abacus_matrix_set(result, i, j, product);
            abacus_free(product);
            
            if (err != MATH_SUCCESS) {
                return err;
            }
        }
    }
    
    result->initialized = true;
    return MATH_SUCCESS;
}
