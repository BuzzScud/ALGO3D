#ifndef PRIME_MATRIX_H
#define PRIME_MATRIX_H

/*
 * prime_matrix.h - Auto-generated header file
 * Source: prime_matrix.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Local includes */
#include "../include/prime_matrix.h"
#include "../include/crystal_abacus.h"

/* Function declarations */
Matrix* matrix_create(int rows, int cols);
void matrix_free(Matrix* m);
Matrix* matrix_create_identity(int size);
Matrix* matrix_add(const Matrix* a, const Matrix* b);
Matrix* matrix_subtract(const Matrix* a, const Matrix* b);
Matrix* matrix_multiply(const Matrix* a, const Matrix* b);
Matrix* matrix_transpose(const Matrix* m);
double matrix_determinant(const Matrix* m);
double matrix_trace(const Matrix* m);
bool matrix_is_identity(const Matrix* m);
bool matrix_is_square(const Matrix* m);
bool matrix_is_diagonal(const Matrix* m);
void big_matrix_free(BigMatrix* m);

#endif /* PRIME_MATRIX_H */