/**
 * @file dimensional_frequencies.c
 * @brief Dimensional frequency mapping based on base-60 mathematics
 * 
 * Maps dimensions to frequencies using prime number sequences.
 * This is a general-purpose algorithm that can be used by any system
 * requiring dimensional frequency mapping.
 */

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Dimensional frequencies based on base-60 mathematics
 * 
 * These frequencies correspond to prime positions on the clock lattice
 * and are used for dimensional modulation in multi-dimensional systems.
 * 
 * The sequence follows the pattern of primes at specific clock positions:
 * - Position 1 (mod 12 ≡ 1): 3, 13, 37, 61, 73, ...
 * - Position 2 (mod 12 ≡ 2): 2 (special case)
 * - Position 3 (mod 12 ≡ 3): 3, 7, 19, 31, 43, ...
 * - Position 5 (mod 12 ≡ 5): 5, 17, 29, 41, 53, ...
 * - Position 6 (mod 12 ≡ 6): 7, 19, 31, 43, ...
 * - Position 9 (mod 12 ≡ 9): 11, 23, 47, 59, 71, ...
 * - Position 11 (mod 12 ≡ 11): 11, 23, 47, 59, 71, 83, 89, ...
 */
const uint64_t DIMENSIONAL_FREQUENCIES[] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37,
    41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89
};

const size_t DIMENSIONAL_FREQUENCIES_COUNT = 
    sizeof(DIMENSIONAL_FREQUENCIES) / sizeof(DIMENSIONAL_FREQUENCIES[0]);

/**
 * @brief Get dimensional frequency for a given dimension
 * 
 * This function maps dimension indices to frequency values based on
 * base-60 mathematical principles. The frequencies are derived from
 * prime positions on the clock lattice.
 * 
 * @param dimension Dimension index (0-based)
 * @return Frequency value for the dimension
 * 
 * @note For dimensions beyond the table, returns the last frequency value
 * @note This is a general-purpose algorithm, not application-specific
 */
uint64_t get_dimensional_frequency(size_t dimension) {
    if (dimension >= DIMENSIONAL_FREQUENCIES_COUNT) {
        // For dimensions beyond our table, return the last frequency
        return DIMENSIONAL_FREQUENCIES[DIMENSIONAL_FREQUENCIES_COUNT - 1];
    }
    return DIMENSIONAL_FREQUENCIES[dimension];
}

/**
 * @brief Get dimensional frequency with modulo 12 wrapping
 * 
 * This variant wraps the dimension using modulo 12 to match the
 * 12-fold symmetry of the clock lattice.
 * 
 * @param dimension Dimension index (can be any value)
 * @return Frequency value for dimension % 12
 */
uint64_t get_dimensional_frequency_mod12(size_t dimension) {
    size_t wrapped_dim = dimension % 12;
    if (wrapped_dim >= DIMENSIONAL_FREQUENCIES_COUNT) {
        wrapped_dim = DIMENSIONAL_FREQUENCIES_COUNT - 1;
    }
    return DIMENSIONAL_FREQUENCIES[wrapped_dim];
}

/**
 * @brief Get the count of available dimensional frequencies
 * 
 * @return Number of frequencies in the table
 */
size_t get_dimensional_frequency_count(void) {
    return DIMENSIONAL_FREQUENCIES_COUNT;
}