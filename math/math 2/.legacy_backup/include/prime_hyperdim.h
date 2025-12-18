#ifndef PRIME_HYPERDIM_H
#define PRIME_HYPERDIM_H

/*
 * prime_hyperdim.h - Auto-generated header file
 * Source: prime_hyperdim.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Local includes */
#include "../include/prime_hyperdim.h"

/* Function declarations */
void hypervector_free(HyperVector* hv);
double hypervector_magnitude(const HyperVector* hv);
double hypervector_dot(const HyperVector* a, const HyperVector* b);
double hyper_sin(double x, int dimension);
double hyper_cos(double x, int dimension);
void hypervector_project_2d(const HyperVector* hv, double* x, double* y);
double hypervector_angle_2d(const HyperVector* hv);
double hypervector_distance(const HyperVector* a, const HyperVector* b);
int hypervector_similar(const HyperVector* a, const HyperVector* b, double threshold);


HyperVector prime_to_hypervector(int prime);
#endif /* PRIME_HYPERDIM_H */
