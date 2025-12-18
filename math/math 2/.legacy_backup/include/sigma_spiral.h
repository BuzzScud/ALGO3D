/**
 * @file sigma_spiral.h
 * @brief Sigma spiral generation using golden ratio iteration
 * 
 * The sigma spiral is different from the Ulam spiral. It uses golden ratio
 * iteration with modular reduction to create a bounded spiral pattern.
 * 
 * Formula: σ(iter) = (φ^iter) mod p_mod
 * 
 * This creates a deterministic pseudo-random sequence that exhibits
 * spiral-like properties in the modular space.
 */

#ifndef SIGMA_SPIRAL_H
#define SIGMA_SPIRAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate sigma spiral value at iteration
 * 
 * Computes: σ(iter) = (φ^iter) mod p_mod
 * 
 * @param iter Iteration count
 * @param p_mod Modulus (default: 2^32 + 13)
 * @return Spiral value
 */
double sigma_spiral(uint32_t iter, uint64_t p_mod);

/**
 * @brief Generate sigma spiral sequence
 * 
 * Generates a sequence of sigma spiral values.
 * 
 * @param output Output array (pre-allocated)
 * @param len Sequence length
 * @param p_mod Modulus
 */
void sigma_spiral_sequence(double* output, size_t len, uint64_t p_mod);

/**
 * @brief Generate 2D sigma spiral coordinates
 * 
 * Maps sigma spiral values to 2D coordinates using modular arithmetic.
 * 
 * @param x_coords Output X coordinates (pre-allocated)
 * @param y_coords Output Y coordinates (pre-allocated)
 * @param len Number of points
 * @param grid_size Size of 2D grid
 * @param p_mod Modulus
 */
void sigma_spiral_2d(uint32_t* x_coords, uint32_t* y_coords,
                     size_t len, uint32_t grid_size, uint64_t p_mod);

/**
 * @brief Calculate sigma spiral with custom base
 * 
 * Computes: σ(iter, base) = (base^iter) mod p_mod
 * 
 * @param iter Iteration count
 * @param base Base value (default: φ)
 * @param p_mod Modulus
 * @return Spiral value
 */
double sigma_spiral_custom_base(uint32_t iter, double base, uint64_t p_mod);

/**
 * @brief Analyze sigma spiral distribution
 * 
 * Calculates statistics about the spiral distribution.
 * 
 * @param sequence Spiral sequence
 * @param len Sequence length
 * @param mean Output mean value
 * @param variance Output variance
 * @param coverage Output coverage ratio (unique values / total)
 */
void sigma_spiral_analyze(const double* sequence, size_t len,
                          double* mean, double* variance, double* coverage);

#ifdef __cplusplus
}
#endif

#endif /* SIGMA_SPIRAL_H */