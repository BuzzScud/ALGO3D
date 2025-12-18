/**
 * SIMD Utilities for CLLM Training
 * 
 * Provides vectorized operations using AVX2 for significant speedup
 * Supports both float and double precision
 */

#include <immintrin.h>
#include <stdint.h>
#include <string.h>
#include "../include/cllm_cache.h"

/**
 * AVX2 dot product for doubles (4 doubles at a time)
 * Requires: n must be multiple of 4
 */
double dot_product_avx2_double(const double* a, const double* b, int n) {
    __m256d sum = _mm256_setzero_pd();
    
    for (int i = 0; i < n; i += 4) {
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        sum = _mm256_fmadd_pd(va, vb, sum);
    }
    
    // Horizontal sum
    __m128d sum_high = _mm256_extractf128_pd(sum, 1);
    __m128d sum_low = _mm256_castpd256_pd128(sum);
    __m128d sum128 = _mm_add_pd(sum_high, sum_low);
    
    sum128 = _mm_hadd_pd(sum128, sum128);
    
    return _mm_cvtsd_f64(sum128);
}

/**
 * AVX2 dot product (8 floats at a time)
 * Requires: n must be multiple of 8
 */
float dot_product_avx2(const float* a, const float* b, int n) {
    __m256 sum = _mm256_setzero_ps();
    
    for (int i = 0; i < n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        sum = _mm256_fmadd_ps(va, vb, sum);
    }
    
    // Horizontal sum
    __m128 sum_high = _mm256_extractf128_ps(sum, 1);
    __m128 sum_low = _mm256_castps256_ps128(sum);
    __m128 sum128 = _mm_add_ps(sum_high, sum_low);
    
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    return _mm_cvtss_f32(sum128);
}

/**
 * Fallback for non-multiple-of-8 sizes
 */
float dot_product_scalar(const float* a, const float* b, int n) {
    float sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

/**
 * Adaptive dot product for doubles (uses AVX2 when possible)
 */
double dot_product_double(const double* a, const double* b, int n) {
    int n_vec = (n / 4) * 4;  // Round down to multiple of 4
    
    double sum = 0.0;
    if (n_vec > 0) {
        sum = dot_product_avx2_double(a, b, n_vec);
    }
    
    // Handle remainder
    for (int i = n_vec; i < n; i++) {
        sum += a[i] * b[i];
    }
    
    return sum;
}

/**
 * Adaptive dot product (uses AVX2 when possible)
 * This is the main function to use - automatically chooses best implementation
 */
float dot_product(const float* a, const float* b, int n) {
    int n_vec = (n / 8) * 8;  // Round down to multiple of 8
    
    float sum = 0.0;
    if (n_vec > 0) {
        sum = dot_product_avx2(a, b, n_vec);
    }
    
    // Handle remainder
    for (int i = n_vec; i < n; i++) {
        sum += a[i] * b[i];
    }
    
    return sum;
}

/**
 * Vectorized element-wise multiplication and accumulation
 * result[i] += a[i] * b[i] for all i
 */
void vector_multiply_accumulate(float* result, const float* a, const float* b, int n) {
    int n_vec = (n / 8) * 8;
    
    // Vectorized part
    for (int i = 0; i < n_vec; i += 8) {
        __m256 vr = _mm256_loadu_ps(&result[i]);
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        vr = _mm256_fmadd_ps(va, vb, vr);
        _mm256_storeu_ps(&result[i], vr);
    }
    
    // Scalar remainder
    for (int i = n_vec; i < n; i++) {
        result[i] += a[i] * b[i];
    }
}

/**
 * Vectorized element-wise addition
 * result[i] = a[i] + b[i] for all i
 */
void vector_add(float* result, const float* a, const float* b, int n) {
    int n_vec = (n / 8) * 8;
    
    // Vectorized part
    for (int i = 0; i < n_vec; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        __m256 vr = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&result[i], vr);
    }
    
    // Scalar remainder
    for (int i = n_vec; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/**
 * Vectorized scalar multiplication
 * result[i] = a[i] * scalar for all i
 */
void vector_scale(float* result, const float* a, float scalar, int n) {
    int n_vec = (n / 8) * 8;
    __m256 vscalar = _mm256_set1_ps(scalar);
    
    // Vectorized part
    for (int i = 0; i < n_vec; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vr = _mm256_mul_ps(va, vscalar);
        _mm256_storeu_ps(&result[i], vr);
    }
    
    // Scalar remainder
    for (int i = n_vec; i < n; i++) {
        result[i] = a[i] * scalar;
    }
}

/**
 * Matrix-vector multiplication: result = A * x
 * A is m x n matrix in row-major order
 * x is n-dimensional vector
 * result is m-dimensional vector
 */
/**
 * SIMD matrix-vector multiplication for doubles: result = A * x
 */
void simd_matrix_vector_multiply_double(double* result, const double* A, const double* x, int m, int n) {
    for (int i = 0; i < m; i++) {
        result[i] = dot_product_double(&A[i * n], x, n);
    }
}

void simd_matrix_vector_multiply(float* result, const float* A, const float* x, int m, int n) {
    for (int i = 0; i < m; i++) {
        result[i] = dot_product(&A[i * n], x, n);
    }
}

/**
 * Matrix-matrix multiplication: C = A * B
 * Uses cache-friendly blocking and AVX2 vectorization
 * All matrices in row-major order
 */
void simd_matrix_multiply(float* C, const float* A, const float* B, int m, int n, int p) {
    // Initialize C to zero
    memset(C, 0, m * p * sizeof(float));
    
    // Block size for cache optimization (tune based on cache size)
    const int BLOCK_SIZE = 32;  // Optimized for L1 cache (32KB)
    
    // Blocked matrix multiplication
    for (int i0 = 0; i0 < m; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < p; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < n; k0 += BLOCK_SIZE) {
                // Process block
                int i_max = (i0 + BLOCK_SIZE < m) ? i0 + BLOCK_SIZE : m;
                int j_max = (j0 + BLOCK_SIZE < p) ? j0 + BLOCK_SIZE : p;
                int k_max = (k0 + BLOCK_SIZE < n) ? k0 + BLOCK_SIZE : n;
                
                for (int i = i0; i < i_max; i++) {
                    for (int k = k0; k < k_max; k++) {
                        float a_ik = A[i * n + k];
                        __m256 va = _mm256_set1_ps(a_ik);
                        
                        int j = j0;
                        int j_vec = j0 + ((j_max - j0) / 8) * 8;
                        
                        // Vectorized inner loop
                        for (; j < j_vec; j += 8) {
                            __m256 vb = _mm256_loadu_ps(&B[k * p + j]);
                            __m256 vc = _mm256_loadu_ps(&C[i * p + j]);
                            vc = _mm256_fmadd_ps(va, vb, vc);
                            _mm256_storeu_ps(&C[i * p + j], vc);
                        }
                        
                        // Scalar remainder
                        for (; j < j_max; j++) {
                            C[i * p + j] += a_ik * B[k * p + j];
                        }
                    }
                }
            }
        }
    }
}

/**
 * Transposed matrix-matrix multiplication: C = A^T * B
 * A is n x m (will be transposed to m x n)
 * B is n x p
 * C is m x p
 * More cache-friendly when A needs to be transposed
 */
void simd_matrix_multiply_transposed(float* C, const float* A, const float* B, int m, int n, int p) {
    // Initialize C to zero
    memset(C, 0, m * p * sizeof(float));
    
    // For A^T * B, we compute C[i,j] = sum_k A[k,i] * B[k,j]
    // This is more cache-friendly than explicitly transposing A
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            float sum = 0.0;
            
            // Vectorized dot product
            int k = 0;
            int k_vec = (n / 8) * 8;
            __m256 vsum = _mm256_setzero_ps();
            
            for (; k < k_vec; k += 8) {
                // Load A[k:k+8, i] (strided access)
                __m256 va = _mm256_set_ps(
                    A[(k+7)*m + i], A[(k+6)*m + i], A[(k+5)*m + i], A[(k+4)*m + i],
                    A[(k+3)*m + i], A[(k+2)*m + i], A[(k+1)*m + i], A[k*m + i]
                );
                
                // Load B[k:k+8, j] (strided access)
                __m256 vb = _mm256_set_ps(
                    B[(k+7)*p + j], B[(k+6)*p + j], B[(k+5)*p + j], B[(k+4)*p + j],
                    B[(k+3)*p + j], B[(k+2)*p + j], B[(k+1)*p + j], B[k*p + j]
                );
                
                vsum = _mm256_fmadd_ps(va, vb, vsum);
            }
            
            // Horizontal sum
            __m128 sum_high = _mm256_extractf128_ps(vsum, 1);
            __m128 sum_low = _mm256_castps256_ps128(vsum);
            __m128 sum128 = _mm_add_ps(sum_high, sum_low);
            sum128 = _mm_hadd_ps(sum128, sum128);
            sum128 = _mm_hadd_ps(sum128, sum128);
            sum = _mm_cvtss_f32(sum128);
            
            // Scalar remainder
            for (; k < n; k++) {
                sum += A[k * m + i] * B[k * p + j];
            }
            
            C[i * p + j] = sum;
        }
    }
}