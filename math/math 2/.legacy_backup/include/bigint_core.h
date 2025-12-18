#ifndef BIGINT_CORE_H
#define BIGINT_CORE_H

/*
 * BigInt Core - Arbitrary Precision Integer Operations
 * 
 * CRITICAL: This library uses ARBITRARY PRECISION for all calculations.
 * BigInt is the fundamental type for exact integer arithmetic.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Core type definitions */
#include "math/types.h"

/* Function declarations */
void big_init(BigInt *n);
void big_free(BigInt *n);
void big_from_int(BigInt *n, uint64_t val);
char* big_to_string(const BigInt *n);
void big_add(const BigInt *a, const BigInt *b, BigInt *result);
void big_sub(const BigInt *a, const BigInt *b, BigInt *result);
void big_mul(const BigInt *a, const BigInt *b, BigInt *result);
void big_div(const BigInt *a, const BigInt *b, BigInt *quotient, BigInt *remainder);
void big_mod(const BigInt *a, const BigInt *b, BigInt *result);
void big_copy(BigInt *dest, const BigInt *src);
bool big_is_zero(const BigInt *n);
int big_cmp(const BigInt *a, const BigInt *b);
void big_powmod(const BigInt *base, const BigInt *exp, const BigInt *mod, BigInt *result);
void big_shl(BigInt *n, int bits);
void big_shr(BigInt *n, int bits);
int big_is_prime(const BigInt *n, int iterations);
int big_is_prime_trial(BigInt *n);
int big_is_prime_fermat(BigInt *n, int iterations);
int big_is_prime_solovay_strassen(BigInt *n, int iterations);
int big_is_prime_miller_rabin(BigInt *n);
bool big_are_twin_primes(const BigInt *p1, const BigInt *p2);
bool big_is_sophie_germain_prime(const BigInt *p);
void big_prime_factorization(BigInt *n, BigInt*** factors, int** powers, int* count);
void big_euler_totient(const BigInt *n, BigInt *result);
void big_gcd(const BigInt *a, const BigInt *b, BigInt *result);
void big_lcm(const BigInt *a, const BigInt *b, BigInt *result);
int64_t big_to_int64(const BigInt *n);
void crystal_abacus_big_init(CrystalAbacusBig *ab);
void crystal_abacus_big_free(CrystalAbacusBig *ab);
void crystal_abacus_big_next_prime(CrystalAbacusBig *ab, BigInt *p);
void big_prime_add(const BigInt *a, const BigInt *b, BigInt *c);
void big_prime_subtract(const BigInt *a, const BigInt *b, BigInt *c);
void big_prime_multiply(const BigInt *a, const BigInt *b, BigInt *c);
void big_prime_divide(const BigInt *a, const BigInt *b, BigInt *q, BigInt *r);
double big_to_double(const BigInt *n);
int big_lattice_sign(BigInt *x, int depth);
int big_cmp_int(const BigInt *a, uint64_t n);
void big_lattice_add(const BigInt *a, const BigInt *b, BigInt *result, int depth);
bool big_is_mersenne_prime(const BigInt *p);

int big_mod_inverse(BigInt* result, const BigInt* a, const BigInt* m);

int big_mod_inverse(BigInt* result, const BigInt* a, const BigInt* m);

#endif /* BIGINT_CORE_H */
