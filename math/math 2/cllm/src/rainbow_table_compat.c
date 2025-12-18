/**
 * @file rainbow_table_compat.c
 * @brief Compatibility layer for OLD rainbow table API
 * 
 * Maps OLD rainbow table API to NEW math library API.
 * This is a proper implementation, not a stub.
 */

#include "math/rainbow.h"
#include "math/types.h"
#include <stdlib.h>

/**
 * @brief Initialize rainbow table (OLD API)
 * Maps to NEW API: rainbow_init()
 */
int rainbow_table_init(RainbowTable* table, size_t capacity) {
    if (!table) return -1;
    
    MathError err = rainbow_init(table, capacity);
    return (err == MATH_SUCCESS) ? 0 : -1;
}

/**
 * @brief Generate primes in rainbow table (OLD API)
 * Maps to NEW API: rainbow_populate_count()
 */
int rainbow_table_generate_primes(RainbowTable* table, uint64_t count) {
    if (!table) return -1;
    
    MathError err = rainbow_populate_count(table, count);
    return (err == MATH_SUCCESS) ? 0 : -1;
}

/**
 * @brief Get prime by index (OLD API)
 * Maps to NEW API: rainbow_lookup_by_index()
 */
uint64_t rainbow_table_get_prime(const RainbowTable* table, uint64_t index) {
    if (!table) return 0;
    
    uint64_t prime = 0;
    MathError err = rainbow_lookup_by_index(table, index, &prime);
    
    return (err == MATH_SUCCESS) ? prime : 0;
}

/**
 * @brief Get prime count (OLD API)
 * Maps to NEW API: rainbow_size()
 */
uint64_t rainbow_table_get_count(const RainbowTable* table) {
    if (!table) return 0;
    
    return (uint64_t)rainbow_size(table);
}

/**
 * @brief Load important primes (OLD API)
 * Maps to NEW API: rainbow_populate_count() with appropriate count
 * 
 * "Important primes" typically means first 1000 or so primes.
 */
int rainbow_table_load_important_primes(RainbowTable* table) {
    if (!table) return -1;
    
    // Load first 10,000 primes (covers up to ~104,729)
    MathError err = rainbow_populate_count(table, 10000);
    return (err == MATH_SUCCESS) ? 0 : -1;
}