#ifndef CRYSTAL_ABACUS_H
#define CRYSTAL_ABACUS_H

/*
 * crystal_abacus.h - Auto-generated header file
 * Source: crystal_abacus.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Local includes */
#include "crystal_abacus.h"

/* Function declarations */
void abacus_free(CrystalAbacus *abacus);
int abacus_next_prime(CrystalAbacus *abacus);
bool abacus_is_prime(CrystalAbacus *abacus, int m);
int prime_max_int_local(int a, int b);
int prime_min_int_local(int a, int b);
bool is_prime(uint64_t n);
int* generate_n_primes(int n);
double vibrational_transducer(double input, double prime_factor);
double lattice_vibrational_transducer(double input, double prime_factor, int depth);

#endif /* CRYSTAL_ABACUS_H */