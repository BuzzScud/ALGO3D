#ifndef PRIME_LATTICE_GEOMETRY_H
#define PRIME_LATTICE_GEOMETRY_H

/*
 * prime_lattice_geometry.h - Auto-generated header file
 * Source: prime_lattice_geometry.c
 */

#include <stdint.h>
#include <stdbool.h>
#include "bigint_core.h"
#include <stddef.h>
#include "math/types.h"

/* Local includes */
#include "prime_lattice_geometry.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/* Function declarations */
void map_prime_to_clock(uint64_t prime, LegacyClockPosition *pos);
int map_to_361_circle(uint64_t value);
int angle_to_clock_position(double angle_radians);
bool is_on_boundary(uint64_t prime);
void fold_to_q1(double angle, QuadrantFold *fold);
double unfold_from_q1(double folded_angle, const QuadrantFold *fold);
bool spheres_are_kissing(const LatticeSphere *s1, const LatticeSphere *s2, double tolerance);
double sphere_gap(const LatticeSphere *s1, const LatticeSphere *s2);
int get_radial_line(uint64_t prime);
int get_concentric_ring(uint64_t prime);
bool is_on_radial_line(uint64_t prime, int line_index);
bool is_on_concentric_ring(uint64_t prime, int ring_number);
int modular_circle_map(uint64_t value);
void apply_golden_damping(BigInt *value, int depth);
int log3_approx(uint64_t n);
void vector_add_q1(const Vector2D *a, const Vector2D *b, Vector2D *result);
void vector_scale_rational(const Vector2D *v, int numerator, int denominator, Vector2D *result);
void build_prime_tower(BigInt *result, const uint64_t *primes, int count,
                       const BigInt *modulus, bool apply_damping_flag);
void tetration_damped(BigInt *result, uint64_t base, int height,
                      const BigInt *modulus);
bool is_prime_geometric(uint64_t n);
uint64_t next_prime_geometric(uint64_t n);
uint64_t count_primes_geometric(uint64_t n);
void lattice_geometry_init(void);
void lattice_geometry_cleanup(void);

#endif /* PRIME_LATTICE_GEOMETRY_H */