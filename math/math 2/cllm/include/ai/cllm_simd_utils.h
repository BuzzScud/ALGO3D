#ifndef CLLM_SIMD_UTILS_H
#define CLLM_SIMD_UTILS_H

/**
 * SIMD Utilities for CLLM Training
 * 
 * Provides vectorized operations using AVX2 for significant speedup
 */

/**
 * Compute dot product of two double vectors
 * Uses AVX2 when possible, falls back to scalar for remainder
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Length of vectors
 * @return Dot product a·b
 */
double dot_product_double(const double* a, const double* b, int n);

/**
 * Compute dot product of two vectors
 * Uses AVX2 when possible, falls back to scalar for remainder
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Length of vectors
 * @return Dot product a·b
 */
float dot_product(const float* a, const float* b, int n);

/**
 * AVX2-optimized dot product (requires n to be multiple of 8)
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Length of vectors (must be multiple of 8)
 * @return Dot product a·b
 */
float dot_product_avx2(const float* a, const float* b, int n);

/**
 * Scalar fallback dot product
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Length of vectors
 * @return Dot product a·b
 */
float dot_product_scalar(const float* a, const float* b, int n);

/**
 * Vectorized multiply-accumulate: result[i] += a[i] * b[i]
 * 
 * @param result Result vector (modified in-place)
 * @param a First input vector
 * @param b Second input vector
 * @param n Length of vectors
 */
void vector_multiply_accumulate(float* result, const float* a, const float* b, int n);

/**
 * Vectorized addition: result[i] = a[i] + b[i]
 * 
 * @param result Result vector
 * @param a First input vector
 * @param b Second input vector
 * @param n Length of vectors
 */
void vector_add(float* result, const float* a, const float* b, int n);

/**
 * Vectorized scalar multiplication: result[i] = a[i] * scalar
 * 
 * @param result Result vector
 * @param a Input vector
 * @param scalar Scalar multiplier
 * @param n Length of vectors
 */
void vector_scale(float* result, const float* a, float scalar, int n);

/**
 * SIMD matrix-vector multiplication for doubles: result = A * x
 * Uses AVX2 for vectorization
 * 
 * @param result Result vector [m]
 * @param A Matrix [m x n] in row-major order
 * @param x Input vector [n]
 * @param m Number of rows in A
 * @param n Number of columns in A
 */
void simd_matrix_vector_multiply_double(double* result, const double* A, const double* x, int m, int n);

/**
 * SIMD matrix-vector multiplication: result = A * x
 * Uses AVX2 for vectorization
 * 
 * @param result Result vector [m]
 * @param A Matrix [m x n] in row-major order
 * @param x Input vector [n]
 * @param m Number of rows in A
 * @param n Number of columns in A
 */
void simd_matrix_vector_multiply(float* result, const float* A, const float* x, int m, int n);

/**
 * SIMD matrix-matrix multiplication: C = A * B
 * Uses AVX2 for vectorization with cache-friendly blocking
 * 
 * @param C Result matrix [m x p] in row-major order
 * @param A First matrix [m x n] in row-major order
 * @param B Second matrix [n x p] in row-major order
 * @param m Number of rows in A
 * @param n Number of columns in A / rows in B
 * @param p Number of columns in B
 */
void simd_matrix_multiply(float* C, const float* A, const float* B, int m, int n, int p);

/**
 * SIMD transposed matrix-matrix multiplication: C = A^T * B
 * More cache-friendly for certain operations
 * 
 * @param C Result matrix [m x p] in row-major order
 * @param A First matrix [n x m] in row-major order (will be transposed)
 * @param B Second matrix [n x p] in row-major order
 * @param m Number of columns in A (rows in A^T)
 * @param n Number of rows in A and B
 * @param p Number of columns in B
 */
void simd_matrix_multiply_transposed(float* C, const float* A, const float* B, int m, int n, int p);

#endif /* CLLM_SIMD_UTILS_H */