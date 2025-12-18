#ifndef PRIME_RAINBOW_H
#define PRIME_RAINBOW_H

/*
 * prime_rainbow.h - Auto-generated header file
 * Source: prime_rainbow.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
   #include "clock_lattice.h"

/* Local includes */
#include "prime_rainbow.h"
#include "crystal_abacus.h"

/* Function declarations */
void rainbow_table_init(void);
void rainbow_table_cleanup(void);
double fast_prime_angle(int prime_index);
double fast_prime_radius(int prime_index);
double fast_prime_frequency(int prime_index);
int fast_prime_layer(int prime_index);
void fast_prime_fold_coords(int prime_index, double* x, double* y, double* z);

   // Helper functions using clock lattice
   PrimeModular fast_prime_modular(uint64_t prime);
   bool fast_prime_is_sacred(int prime_index);
   int fast_prime_position(int prime_index);
double big_fast_prime_angle(BigInt *prime);
double big_fast_prime_radius(BigInt *prime);
int big_fast_prime_layer(BigInt *prime);

// Prime storage functions - Rainbow Table as Complete Abacus
PrimeRainbowTable* rainbow_table_get(void);  // Get the global rainbow table instance

// OPTIMIZED: Index-based operations (NEW)
int rainbow_table_add_prime_index(uint32_t prime_index);
int rainbow_table_add_prime_value(uint64_t prime_value);
uint64_t rainbow_table_get_prime_by_index(uint32_t table_index);
uint32_t rainbow_table_get_prime_index(uint32_t table_index);
const ClockLatticeEntry* rainbow_table_get_entry(uint32_t table_index);

// LEGACY: BigInt-based operations (deprecated, for backward compatibility)
int rainbow_table_add_prime(BigInt* prime);
BigInt* rainbow_table_get_prime(int index);

// Common operations
int rainbow_table_get_count(void);
int rainbow_table_generate_primes(int target_count);
int rainbow_table_load_important_primes(void);

// Search operations
int rainbow_table_find_prime(uint64_t prime_value);
bool rainbow_table_contains_prime(uint64_t prime_value);

// Statistics
void rainbow_table_get_stats(uint32_t* count, uint32_t* capacity, size_t* memory_bytes);

// Analysis functions
double rainbow_table_check_stability(PrimeRainbowTable* table);
double rainbow_table_self_similarity(PrimeRainbowTable* table);

#endif /* PRIME_RAINBOW_H */