/**
 * @file tensor_ops.h
 * @brief General tensor operations for multi-dimensional data
 * 
 * Provides tensor distance, product, and manipulation functions
 * using crystalline mathematics.
 */

#ifndef TENSOR_OPS_H
#define TENSOR_OPS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate tensor distance with golden ratio geometry
 * 
 * For 2D: det(t1 - t2) = (dx * gy - dy * gx) mod p
 * where g = (φ, φ²) is golden ratio geometry
 * 
 * For N-D: Generalized determinant calculation
 * 
 * @param t1 First tensor
 * @param t2 Second tensor
 * @param dim Tensor dimension
 * @param p_mod Modulus for calculation
 * @return Tensor distance
 */
double tensor_distance(const double* t1, const double* t2,
                       size_t dim, uint64_t p_mod);

/**
 * @brief Generate random anchor tensors
 * 
 * Creates anchor points using golden ratio scaling.
 * 
 * @param anchors Output anchors (pre-allocated: num_anchors * dim)
 * @param num_anchors Number of anchors
 * @param dim Tensor dimension
 * @param seed Random seed
 */
void tensor_generate_anchors(double* anchors, size_t num_anchors,
                              size_t dim, uint64_t seed);

/**
 * @brief Estimate value from anchor triangulation
 * 
 * Uses distances to anchors to estimate target value.
 * 
 * @param query Query tensor
 * @param anchors Anchor tensors (num_anchors * dim)
 * @param num_anchors Number of anchors
 * @param dim Tensor dimension
 * @param p_mod Modulus
 * @return Estimated value
 */
uint64_t tensor_estimate_from_anchors(const double* query,
                                       const double* anchors,
                                       size_t num_anchors,
                                       size_t dim,
                                       uint64_t p_mod);

/**
 * @brief Tensor outer product
 * 
 * Computes: result[i,j] = a[i] * b[j]
 * 
 * @param a First vector
 * @param a_len Length of a
 * @param b Second vector
 * @param b_len Length of b
 * @param result Output tensor (pre-allocated: a_len * b_len)
 */
void tensor_outer_product(const double* a, size_t a_len,
                          const double* b, size_t b_len,
                          double* result);

#ifdef __cplusplus
}
#endif

#endif /* TENSOR_OPS_H */
