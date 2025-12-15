/**
 * @file clock_position_wrapper.c
 * @brief Wrapper for clock position functions
 * 
 * Provides compatibility layer for algorithms that need clock positions.
 * Uses NEW math library exclusively.
 */

#include "math/types.h"
#include "math/clock.h"
#include "math/prime.h"
#include "../include/clock_lattice.h"

/**
 * @brief Map prime index to clock position
 * 
 * This is a wrapper around the NEW math library function.
 * Returns a BabylonianClockPosition (which is typedef'd to ClockPosition).
 * 
 * @param prime_index Prime index (1-based: 1 = first prime = 2)
 * @return Clock position
 */
BabylonianClockPosition map_prime_index_to_clock(int prime_index) {
    BabylonianClockPosition pos;
    clock_map_index_to_position((uint64_t)prime_index, &pos);
    return pos;
}