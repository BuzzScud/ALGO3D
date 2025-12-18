#ifndef PRIME_COORDS_H
#define PRIME_COORDS_H

/*
 * prime_coords.h - Auto-generated header file
 * Source: prime_coords.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Local includes */
#include "../include/prime_coords.h"
#include "../include/crystal_abacus.h"

/* Function declarations */
void ulam_spiral_coords(int n, double* x, double* y);
void golden_spiral_coords(int n, double* x, double* y);
void archimedes_spiral_coords(double angle, double spacing, double* x, double* y);
void log_spiral_coords(double a, double b, double angle, double* x, double* y);
void cartesian_to_polar(double x, double y, double* r, double* theta);
void polar_to_cartesian(double r, double theta, double* x, double* y);
void cartesian_to_spherical(double x, double y, double z, double* r, double* theta, double* phi);
void map_to_clock_position(int number, int max_numbers, double* angle, double* radius);
void quadratic_mirror_fold(double x, double y, double fold_amount, double* new_x, double* new_y);
void spiral_collapse(double x, double y, double collapse_rate, double* new_x, double* new_y);
void big_ulam_spiral_coords(BigInt* n, double* x, double* y);
void prime_transform_coords(double x, double y, int prime, double* new_x, double* new_y);
void torus_fold_coords(double x, double y, double major_radius, double minor_radius, double* new_x, double* new_y);
void hilbert_coords(int index, int order, double* x, double* y);

#endif /* PRIME_COORDS_H */