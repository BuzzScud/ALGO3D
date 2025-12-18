#ifndef PRIME_LATTICE_H
#define PRIME_LATTICE_H

/*
 * prime_lattice.h - Auto-generated header file
 * Source: prime_lattice.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "prime_basic.h"
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Local includes */
#include "prime_lattice.h"
/* REMOVED: #include "crystal_abacus.h" - Conflicts with NEW math library */

/* Function declarations */
double lattice_add(double a, double b, int depth);
double lattice_subtract(double a, double b, int depth);
double lattice_multiply(double a, double b, int depth);
double lattice_divide(double a, double b, int depth);
double lattice_sin(double x, int depth);
double lattice_cos(double x, int depth);
double lattice_abs(double x, int depth);

#endif /* PRIME_LATTICE_H */