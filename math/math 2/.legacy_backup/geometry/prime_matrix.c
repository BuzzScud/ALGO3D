#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "../include/prime_matrix.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "../include/crystal_abacus.h"

// Create a matrix
Matrix* matrix_create(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return NULL;
    
    Matrix* m = malloc(sizeof(Matrix));
    if (!m) return NULL;
    
    m->rows = rows;
    m->cols = cols;
    
    // Allocate row pointers
    m->data = malloc(rows * sizeof(double*));
    if (!m->data) {
        free(m);
        return NULL;
    }
    
    // Allocate each row
    for (int i = 0; i < rows; i++) {
        m->data[i] = malloc(cols * sizeof(double));
        if (!m->data[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                free(m->data[j]);
            }
            free(m->data);
            free(m);
            return NULL;
        }
        
        // Initialize to zero
        for (int j = 0; j < cols; j++) {
            m->data[i][j] = 0.0;
        }
    }
    
    return m;
}

// Free a matrix
void matrix_free(Matrix* m) {
    if (!m) return;
    
    if (m->data) {
        for (int i = 0; i < m->rows; i++) {
            if (m->data[i]) {
                free(m->data[i]);
            }
        }
        free(m->data);
    }
    
    free(m);
}

// Create identity matrix
Matrix* matrix_create_identity(int size) {
    if (size <= 0) return NULL;
    
    Matrix* m = matrix_create(size, size);
    if (!m) return NULL;
    
    for (int i = 0; i < size; i++) {
        m->data[i][i] = 1.0;
    }
    
    return m;
}

// Matrix addition
Matrix* matrix_add(const Matrix* a, const Matrix* b) {
    if (!a || !b || a->rows != b->rows || a->cols != b->cols) return NULL;
    
    Matrix* result = matrix_create(a->rows, a->cols);
    if (!result) return NULL;
    
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            result->data[i][j] = a->data[i][j] + b->data[i][j];
        }
    }
    
    return result;
}

// Matrix subtraction
Matrix* matrix_subtract(const Matrix* a, const Matrix* b) {
    if (!a || !b || a->rows != b->rows || a->cols != b->cols) return NULL;
    
    Matrix* result = matrix_create(a->rows, a->cols);
    if (!result) return NULL;
    
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            result->data[i][j] = a->data[i][j] - b->data[i][j];
        }
    }
    
    return result;
}

// Matrix multiplication
Matrix* matrix_multiply(const Matrix* a, const Matrix* b) {
    if (!a || !b || a->cols != b->rows) return NULL;
    
    Matrix* result = matrix_create(a->rows, b->cols);
    if (!result) return NULL;
    
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < a->cols; k++) {
                sum += a->data[i][k] * b->data[k][j];
            }
            result->data[i][j] = sum;
        }
    }
    
    return result;
}

// Matrix transpose
Matrix* matrix_transpose(const Matrix* m) {
    if (!m) return NULL;
    
    Matrix* result = matrix_create(m->cols, m->rows);
    if (!result) return NULL;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            result->data[j][i] = m->data[i][j];
        }
    }
    
    return result;
}

// Matrix determinant (recursive, for small matrices)
double matrix_determinant(const Matrix* m) {
    if (!m || m->rows != m->cols) return 0.0;
    
    int n = m->rows;
    
    if (n == 1) {
        return m->data[0][0];
    }
    
    if (n == 2) {
        return m->data[0][0] * m->data[1][1] - m->data[0][1] * m->data[1][0];
    }
    
    // For larger matrices, use simple expansion (not efficient for large n)
    double det = 0.0;
    
    for (int j = 0; j < n; j++) {
        // Create minor matrix
        Matrix* minor = matrix_create(n-1, n-1);
        if (!minor) continue;
        
        for (int i = 1; i < n; i++) {
            int col = 0;
            for (int k = 0; k < n; k++) {
                if (k != j) {
                    minor->data[i-1][col] = m->data[i][k];
                    col++;
                }
            }
        }
        
        double minor_det = matrix_determinant(minor);
        det += (j % 2 == 0 ? 1.0 : -1.0) * m->data[0][j] * minor_det;
        
        matrix_free(minor);
    }
    
    return det;
}

// Matrix trace (sum of diagonal elements)
double matrix_trace(const Matrix* m) {
    if (!m || m->rows != m->cols) return 0.0;
    
    double trace = 0.0;
    for (int i = 0; i < m->rows; i++) {
        trace += m->data[i][i];
    }
    
    return trace;
}

// Check if matrix is identity
bool matrix_is_identity(const Matrix* m) {
    if (!m || m->rows != m->cols) return 0;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            double expected = (i == j) ? 1.0 : 0.0;
            if (math_abs(m->data[i][j] - expected) > 1e-10) {
                return 0;
            }
        }
    }
    
    return 1;
}

// Check if matrix is square
bool matrix_is_square(const Matrix* m) {
    return (m && m->rows == m->cols);
}

// Check if matrix is diagonal
bool matrix_is_diagonal(const Matrix* m) {
    if (!m || m->rows != m->cols) return 0;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            if (i != j && math_abs(m->data[i][j]) > 1e-10) {
                return 0;
            }
        }
    }
    
    return 1;
}

// BigInt Matrix functions
BigMatrix* big_matrix_create(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return NULL;
    
    BigMatrix* m = malloc(sizeof(BigMatrix));
    if (!m) return NULL;
    
    m->rows = rows;
    m->cols = cols;
    
    // Allocate row pointers
    m->data = malloc(rows * sizeof(BigInt**));
    if (!m->data) {
        free(m);
        return NULL;
    }
    
    // Allocate each row
    for (int i = 0; i < rows; i++) {
        m->data[i] = malloc(cols * sizeof(BigInt*));
        if (!m->data[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                for (int k = 0; k < cols; k++) {
                    big_free(m->data[j][k]);
                    free(m->data[j][k]);
                }
                free(m->data[j]);
            }
            free(m->data);
            free(m);
            return NULL;
        }
        
        // Initialize each BigInt
        for (int j = 0; j < cols; j++) {
            m->data[i][j] = malloc(sizeof(BigInt));
            if (!m->data[i][j]) {
                // Cleanup on failure
                for (int k = 0; k < j; k++) {
                    big_free(m->data[i][k]);
                    free(m->data[i][k]);
                }
                for (int l = 0; l < i; l++) {
                    for (int k = 0; k < cols; k++) {
                        big_free(m->data[l][k]);
                        free(m->data[l][k]);
                    }
                    free(m->data[l]);
                }
                free(m->data[i]);
                free(m->data);
                free(m);
                return NULL;
            }
            big_init(m->data[i][j]);
        }
    }
    
    return m;
}

// Free BigInt matrix
void big_matrix_free(BigMatrix* m) {
    if (!m) return;
    
    if (m->data) {
        for (int i = 0; i < m->rows; i++) {
            if (m->data[i]) {
                for (int j = 0; j < m->cols; j++) {
                    if (m->data[i][j]) {
                        big_free(m->data[i][j]);
                        free(m->data[i][j]);
                    }
                }
                free(m->data[i]);
            }
        }
        free(m->data);
    }
    
    free(m);
}

// Create BigInt identity matrix
BigMatrix* big_matrix_create_identity(int size) {
    if (size <= 0) return NULL;
    
    BigMatrix* m = big_matrix_create(size, size);
    if (!m) return NULL;
    
    for (int i = 0; i < size; i++) {
        big_from_int(m->data[i][i], 1);
    }
    
    return m;
}