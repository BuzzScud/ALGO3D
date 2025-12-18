#ifndef PRIME_LATTICE_CORE_H
#define PRIME_LATTICE_CORE_H

/*
 * prime_lattice_core.h - Auto-generated header file
 * Source: prime_lattice_core.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Local includes */
#include "../include/prime_lattice_core.h"

/* Function declarations */
void lattice_init(void);
void lattice_cleanup(void);
double nu_lambda(const char *lambda_phon);
void update_phi_freqs(const double *phi_base, double *phi_updated,
                      const char *lambda_phon, int count);
void pythagorean_triple(uint64_t p, uint64_t q, PythagoreanTriple *triple);
double pythagorean_ratio(uint64_t p, uint64_t q);
int mobius_twist(int k);
double theta_n(uint64_t n, int k, const char *lambda_phon,
               uint16_t omega, uint64_t p, uint64_t q, bool use_ratio);
double r_n(uint64_t prime);
void map_prime_to_clock_phonetic(uint64_t prime, LegacyClockPosition *pos,
                        const char *lambda_phon);
uint64_t map_clock_to_prime(double theta, double r, uint8_t quadrant,
                            double tolerance, uint64_t max_search);
void lattice_embed(uint64_t value, LatticeEmbed *embed);
double lattice_entropy(uint64_t n, uint64_t d);
double gmp_tetration(double base, int height, bool damp);
double lattice_tetration_log_approx(double P, int T);
double entropy_equilibrator(double exp, double P, int T);
uint64_t* generate_primes_geometric(uint64_t limit, uint64_t *count);
double O_exponent(uint64_t n, int k, const char *lambda_phon);
double L_lattice(uint64_t n, uint64_t d, int k, const char *lambda_phon,
                 uint16_t omega, uint64_t p, uint64_t q);

/* Arbitrary precision lattice formula */
void L_lattice_bigfixed(BigFixed *result, uint64_t n, uint64_t d, int k, 
                        const char *lambda_phon, uint16_t omega, uint64_t p, uint64_t q,
                        int precision_bits);

/* Dimensional layer function */
double Z_n_d(uint64_t n, uint64_t d, const char *lambda_phon);

/* Prime function */
double P_n_d_k(uint64_t n, uint64_t d, int k, const char *lambda_phon,
               uint16_t omega, uint64_t p, uint64_t q);

/* Complete clock mapping */
void map_prime_complete(uint64_t prime, uint64_t n, CompleteClockMapping *mapping,
                        const char *lambda_phon);

#endif /* PRIME_LATTICE_CORE_H */