#ifndef PRIME_BIGINT_TRANSCENDENTAL_H
#define PRIME_BIGINT_TRANSCENDENTAL_H

/*
 * prime_bigint_transcendental.h - Auto-generated header file
 * Source: prime_bigint_transcendental.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include "abacus_legacy.h"

// Forward declarations
void big_pi(BigFixed *result, int precision_bits);
#include <stddef.h>
#include "math/types.h"

/* Local includes */
#include "prime_bigint_transcendental.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Function declarations */
void big_set_precision_guard(int bits);
int big_get_precision_guard(void);
void big_ln(BigFixed *result, const BigInt *n, int precision_bits);
void big_log2(BigFixed *result, const BigInt *n, int precision_bits);
void big_log3(BigFixed *result, const BigInt *n, int precision_bits);
void big_log10(BigFixed *result, const BigInt *n, int precision_bits);
void big_log_base(BigFixed *result, const BigInt *n, const BigInt *base, int precision_bits);
void big_exp(BigFixed *result, const BigFixed *n, int precision_bits);
void big_pow(BigFixed *result, const BigInt *base, const BigFixed *exponent, int precision_bits);
void big_pow_int(BigInt *result, const BigInt *base, const BigInt *exponent);
void big_sin(BigFixed *result, const BigFixed *theta, int precision_bits);
void big_cos(BigFixed *result, const BigFixed *theta, int precision_bits);
void big_tan(BigFixed *result, const BigFixed *theta, int precision_bits);
void big_atan(BigFixed *result, const BigFixed *x, int precision_bits);
void big_atan2(BigFixed *result, const BigFixed *y, const BigFixed *x, int precision_bits);
void big_prime_tower(BigInt *result, const BigInt **primes, int count, 
                     const BigInt *modulus, BigFixed *damping);
void big_tetration_damped(BigInt *result, const BigInt *base, int height,
                          const BigInt *modulus, bool apply_damping);
void big_modpow_euler(BigInt *result, const BigInt *base, const BigInt *exp, const BigInt *mod);
void big_truncate_dust(BigInt *result, const BigFixed *value, 
                       int target_bits, int guard_bits);
void big_round_dust(BigInt *result, const BigFixed *value,
                    int target_bits, int guard_bits);

void big_tanh(BigFixed *result, const BigFixed *x, int precision_bits);

#endif /* PRIME_BIGINT_TRANSCENDENTAL_H */
