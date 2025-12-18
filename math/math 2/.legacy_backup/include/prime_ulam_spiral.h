/**
 * @file prime_ulam_spiral.h
 * @brief Ulam spiral generation and prime visualization
 * 
 * The Ulam spiral is a graphical depiction of the set of prime numbers,
 * devised by mathematician Stanisław Ulam in 1963. It reveals interesting
 * diagonal patterns in the distribution of primes.
 */

#ifndef PRIME_ULAM_SPIRAL_H
#define PRIME_ULAM_SPIRAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generate Ulam spiral grid
 * 
 * Creates a spiral pattern starting from center, placing consecutive
 * integers in a spiral pattern: right, up, left, down, repeat.
 * 
 * @param grid Output grid (must be pre-allocated: size * size)
 * @param size Grid size (must be odd number)
 */
void prime_ulam_spiral_generate(uint64_t* grid, uint32_t size);

/**
 * @brief Check if position in grid contains a prime number
 * 
 * @param grid Ulam spiral grid
 * @param size Grid size
 * @param x X coordinate
 * @param y Y coordinate
 * @return true if position contains prime
 */
bool prime_ulam_is_prime_at(const uint64_t* grid, uint32_t size,
                             uint32_t x, uint32_t y);

/**
 * @brief Calculate prime density in circular region
 * 
 * Counts primes within radius of (x, y) and returns density.
 * 
 * @param grid Ulam spiral grid
 * @param size Grid size
 * @param x Center X coordinate
 * @param y Center Y coordinate
 * @param radius Search radius
 * @return Prime density (0.0 to 1.0)
 */
double prime_ulam_density(const uint64_t* grid, uint32_t size,
                          uint32_t x, uint32_t y, uint32_t radius);

/**
 * @brief Find diagonal patterns in Ulam spiral
 * 
 * Analyzes diagonals for prime-rich lines (Ulam's discovery).
 * 
 * @param grid Ulam spiral grid
 * @param size Grid size
 * @param diagonal_scores Output array for diagonal scores (size*2 elements)
 * @return Number of significant diagonals found
 */
uint32_t prime_ulam_find_diagonals(const uint64_t* grid, uint32_t size,
                                    double* diagonal_scores);

/**
 * @brief Get value at grid position
 * 
 * @param grid Ulam spiral grid
 * @param size Grid size
 * @param x X coordinate
 * @param y Y coordinate
 * @return Value at position, or 0 if out of bounds
 */
uint64_t prime_ulam_get_value(const uint64_t* grid, uint32_t size,
                               uint32_t x, uint32_t y);

#ifdef __cplusplus
}
#endif

#endif /* PRIME_ULAM_SPIRAL_H */