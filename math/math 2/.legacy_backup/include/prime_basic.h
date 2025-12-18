#ifndef PRIME_BASIC_H
#define PRIME_BASIC_H

/*
 * prime_basic.h - Auto-generated header file
 * Source: prime_basic.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Local includes */

/* Function declarations */
double prime_add(double a, double b);
double prime_subtract(double a, double b);
double prime_multiply(double a, double b);
double prime_divide(double a, double b);
double shannon_entropy(const double* data, int n);
double relative_entropy(const double* p, const double* q, int n);
double wave_simulation(double amplitude, double frequency, int harmonics, double phase, int overtones, double damping);
double tetration_log_approx(double x, int height);
double super_logarithm(double x);
double fibonacci_prime(int n);
double golden_stabilizer(double x, int iterations, int depth);
double resonance_frequency(double base_freq, double modifier);
double harmonic_mean(const double* values, int n);
double check_self_similarity(const double* pattern, int size, int scale);
double fractal_dimension(const double* data, int size);

#endif /* PRIME_BASIC_H */