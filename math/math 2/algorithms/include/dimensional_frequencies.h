/**
 * @file dimensional_frequencies.h
 * @brief Dimensional frequency mapping based on base-60 mathematics
 * 
 * Provides general-purpose dimensional frequency mapping using prime
 * number sequences from the clock lattice. This is a domain-agnostic
 * algorithm that can be used by any system requiring dimensional
 * frequency modulation.
 */

#ifndef ALGORITHMS_DIMENSIONAL_FREQUENCIES_H
#define ALGORITHMS_DIMENSIONAL_FREQUENCIES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dimensional frequencies array
 * 
 * These frequencies are based on base-60 mathematics and correspond
 * to prime positions on the clock lattice. They are used for dimensional
 * modulation in multi-dimensional computational systems.
 */
extern const uint64_t DIMENSIONAL_FREQUENCIES[];

/**
 * @brief Count of available dimensional frequencies
 */
extern const size_t DIMENSIONAL_FREQUENCIES_COUNT;

/**
 * @brief Get dimensional frequency for a given dimension
 * 
 * Maps dimension indices to frequency values based on base-60
 * mathematical principles. The frequencies are derived from prime
 * positions on the clock lattice.
 * 
 * @param dimension Dimension index (0-based)
 * @return Frequency value for the dimension
 * 
 * @note For dimensions beyond the table, returns the last frequency value
 * @note This is a general-purpose algorithm, not application-specific
 * 
 * Example:
 * @code
 * uint64_t freq0 = get_dimensional_frequency(0);  // Returns 3
 * uint64_t freq1 = get_dimensional_frequency(1);  // Returns 7
 * uint64_t freq2 = get_dimensional_frequency(2);  // Returns 31
 * @endcode
 */
uint64_t get_dimensional_frequency(size_t dimension);

/**
 * @brief Get dimensional frequency with modulo 12 wrapping
 * 
 * This variant wraps the dimension using modulo 12 to match the
 * 12-fold symmetry of the clock lattice. Useful for systems that
 * operate on a 12-dimensional base with higher dimensions wrapping.
 * 
 * @param dimension Dimension index (can be any value)
 * @return Frequency value for dimension % 12
 * 
 * Example:
 * @code
 * uint64_t freq12 = get_dimensional_frequency_mod12(12);  // Same as dim 0
 * uint64_t freq13 = get_dimensional_frequency_mod12(13);  // Same as dim 1
 * @endcode
 */
uint64_t get_dimensional_frequency_mod12(size_t dimension);

/**
 * @brief Get the count of available dimensional frequencies
 * 
 * @return Number of frequencies in the table
 */
size_t get_dimensional_frequency_count(void);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_DIMENSIONAL_FREQUENCIES_H */