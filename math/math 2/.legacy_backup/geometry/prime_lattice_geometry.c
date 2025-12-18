/*
 * prime_lattice_geometry.c - Crystalline Lattice Geometric Operations
 * 
 * Implementation of geometric operations for the crystalline lattice.
 * Uses standard precision and geometric consistency, not infinite precision.
 */

#include "prime_lattice_geometry.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

// Simple integer log₃ approximation
static int ilog3(uint64_t n) {
    if (n == 0) return 0;
    int result = 0;
    while (n > 1) {
        n /= 3;
        result++;
    }
    return result;
}

// Simple integer square root
static uint64_t isqrt(uint64_t n) {
    if (n < 2) return n;
    
    uint64_t x = n;
    uint64_t y = (x + 1) / 2;
    
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}

/* ============================================================================
 * CLOCK FACE MAPPING
 * ============================================================================ */

void map_prime_to_clock(uint64_t prime, LegacyClockPosition *pos) {
    if (!pos) return;
    
    // Map using 361-degree circle (19² = 361, squares to circles)
    pos->degree = map_to_361_circle(prime);
    
    // Convert to radians
    pos->angle_radians = (double)pos->degree * MATH_PI / 180.0;
    
    // Map to 12-hour clock position (0-11)
    pos->position = angle_to_clock_position(pos->angle_radians);
    
    // Determine quadrant (1-4)
    if (pos->angle_radians >= 0 && pos->angle_radians < MATH_PI / 2.0) {
        pos->quadrant = 1;
    } else if (pos->angle_radians >= MATH_PI / 2.0 && pos->angle_radians < MATH_PI) {
        pos->quadrant = 2;
    } else if (pos->angle_radians >= MATH_PI && pos->angle_radians < 3.0 * MATH_PI / 2.0) {
        pos->quadrant = 3;
    } else {
        pos->quadrant = 4;
    }
    
    // Check if on 3 o'clock boundary
    pos->on_boundary = is_on_boundary(prime);
}

int map_to_361_circle(uint64_t value) {
    // 361 = 19² (squares to circles mapping)
    // Map value to 0-360 using modular arithmetic
    return (int)(value % SQUARE_CIRCLE_MAP) % CIRCLE_DEGREES;
}

int angle_to_clock_position(double angle_radians) {
    // Normalize angle to [0, 2π)
    while (angle_radians < 0) angle_radians += 2.0 * MATH_PI;
    while (angle_radians >= 2.0 * MATH_PI) angle_radians -= 2.0 * MATH_PI;
    
    // Map to 12 positions (0-11)
    // 0 = 12 o'clock (top), 3 = 3 o'clock (right), etc.
    int position = (int)(angle_radians * CLOCK_POSITIONS / (2.0 * MATH_PI));
    
    if (position >= CLOCK_POSITIONS) position = CLOCK_POSITIONS - 1;
    if (position < 0) position = 0;
    
    return position;
}

bool is_on_boundary(uint64_t prime) {
    // 143999 is the boundary prime (inside edge)
    // 144000 closes the partition at 3 o'clock
    return (prime == BOUNDARY_PRIME);
}

/* ============================================================================
 * QUADRANT FOLDING
 * ============================================================================ */

void fold_to_q1(double angle, QuadrantFold *fold) {
    if (!fold) return;
    
    // Normalize angle to [0, 2π)
    while (angle < 0) angle += 2.0 * MATH_PI;
    while (angle >= 2.0 * MATH_PI) angle -= 2.0 * MATH_PI;
    
    // Determine quadrant and fold to Q1
    if (angle >= 0 && angle < MATH_PI / 2.0) {
        // Q1: no folding needed
        fold->quadrant = 1;
        fold->folded_angle = angle;
        fold->flip_x = false;
        fold->flip_y = false;
        fold->polarity = 1;
    } else if (angle >= MATH_PI / 2.0 && angle < MATH_PI) {
        // Q2: fold to Q1
        fold->quadrant = 2;
        fold->folded_angle = MATH_PI - angle;
        fold->flip_x = true;
        fold->flip_y = false;
        fold->polarity = -1;
    } else if (angle >= MATH_PI && angle < 3.0 * MATH_PI / 2.0) {
        // Q3: fold to Q1
        fold->quadrant = 3;
        fold->folded_angle = angle - MATH_PI;
        fold->flip_x = true;
        fold->flip_y = true;
        fold->polarity = 1;
    } else {
        // Q4: fold to Q1
        fold->quadrant = 4;
        fold->folded_angle = 2.0 * MATH_PI - angle;
        fold->flip_x = false;
        fold->flip_y = true;
        fold->polarity = -1;
    }
}

double unfold_from_q1(double folded_angle, const QuadrantFold *fold) {
    if (!fold) return folded_angle;
    
    switch (fold->quadrant) {
        case 1:
            return folded_angle;
        case 2:
            return MATH_PI - folded_angle;
        case 3:
            return MATH_PI + folded_angle;
        case 4:
            return 2.0 * MATH_PI - folded_angle;
        default:
            return folded_angle;
    }
}

/* ============================================================================
 * KISSING SPHERES
 * ============================================================================ */

bool spheres_are_kissing(const LatticeSphere *s1, const LatticeSphere *s2, double tolerance) {
    if (!s1 || !s2) return false;
    
    // Calculate distance between centers
    double dx = s2->center_x - s1->center_x;
    double dy = s2->center_y - s1->center_y;
    double distance = math_sqrt(dx * dx + dy * dy);
    
    // Sum of radii
    double sum_radii = s1->radius + s2->radius;
    
    // Kissing if distance ≈ sum of radii (within tolerance)
    double diff = math_abs(distance - sum_radii);
    return (diff <= tolerance);
}

double sphere_gap(const LatticeSphere *s1, const LatticeSphere *s2) {
    if (!s1 || !s2) return 0.0;
    
    // Calculate distance between centers
    double dx = s2->center_x - s1->center_x;
    double dy = s2->center_y - s1->center_y;
    double distance = math_sqrt(dx * dx + dy * dy);
    
    // Gap = distance - sum of radii
    // This gap encodes the curvature of π
    double sum_radii = s1->radius + s2->radius;
    return distance - sum_radii;
}

/* ============================================================================
 * RADIAL LINES AND CONCENTRIC RINGS
 * ============================================================================ */

int get_radial_line(uint64_t prime) {
    // Map prime to one of 12 radial lines
    // Uses clock position
    LegacyClockPosition pos;
    map_prime_to_clock(prime, &pos);
    return pos.position;
}

int get_concentric_ring(uint64_t prime) {
    // Ring number based on log₃ scale
    // Self-similar structure at each scale
    return ilog3(prime);
}

bool is_on_radial_line(uint64_t prime, int line_index) {
    return (get_radial_line(prime) == line_index);
}

bool is_on_concentric_ring(uint64_t prime, int ring_number) {
    return (get_concentric_ring(prime) == ring_number);
}

/* ============================================================================
 * MODULAR ARITHMETIC
 * ============================================================================ */

int modular_circle_map(uint64_t value) {
    return map_to_361_circle(value);
}

void apply_golden_damping(BigInt *value, int depth) {
    if (!value || depth <= 0) return;
    
    // Apply golden ratio damping: value *= (987/1597)^depth
    // Using rational approximation for exact arithmetic
    
    for (int i = 0; i < depth; i++) {
        // Multiply by 987
        BigInt temp, numerator;
        big_init(&temp);
        big_init(&numerator);
        big_from_int(&numerator, PHI_DEN);  // 987
        big_mul(value, &numerator, &temp);
        
        // Divide by 1597
        BigInt denominator, quotient, remainder;
        big_init(&denominator);
        big_init(&quotient);
        big_init(&remainder);
        big_from_int(&denominator, PHI_NUM);  // 1597
        big_div(&temp, &denominator, &quotient, &remainder);
        
        big_copy(value, &quotient);
        
        big_free(&temp);
        big_free(&numerator);
        big_free(&denominator);
        big_free(&quotient);
        big_free(&remainder);
    }
}

int log3_approx(uint64_t n) {
    return ilog3(n);
}

/* ============================================================================
 * VECTOR OPERATIONS IN Q1
 * ============================================================================ */

void vector_add_q1(const Vector2D *a, const Vector2D *b, Vector2D *result) {
    if (!a || !b || !result) return;
    
    result->x = a->x + b->x;
    result->y = a->y + b->y;
}

void vector_scale_rational(const Vector2D *v, int numerator, int denominator, Vector2D *result) {
    if (!v || !result || denominator == 0) return;
    
    result->x = v->x * (double)numerator / (double)denominator;
    result->y = v->y * (double)numerator / (double)denominator;
}

/* ============================================================================
 * PRIME EXPONENTIATION TOWERS
 * ============================================================================ */

void build_prime_tower(BigInt *result, const uint64_t *primes, int count,
                       const BigInt *modulus, bool apply_damping_flag) {
    if (!result || !primes || count <= 0) return;
    
    // Start from top of tower
    big_from_int(result, 1);
    
    for (int i = count - 1; i >= 0; i--) {
        // Apply damping if requested
        if (apply_damping_flag) {
            apply_golden_damping(result, i);
        }
        
        // Compute primes[i]^result mod modulus
        BigInt base, temp;
        big_init(&base);
        big_init(&temp);
        big_from_int(&base, primes[i]);
        
        if (modulus) {
            big_powmod(&base, result, modulus, &temp);
        } else {
            // Without modulus, use repeated squaring
            big_from_int(&temp, 1);
            BigInt exp_copy;
            big_init(&exp_copy);
            big_copy(&exp_copy, result);
            
            while (!big_is_zero(&exp_copy)) {
                if (exp_copy.d[0] & 1) {
                    BigInt prod;
                    big_init(&prod);
                    big_mul(&temp, &base, &prod);
                    big_copy(&temp, &prod);
                    big_free(&prod);
                }
                
                BigInt square;
                big_init(&square);
                big_mul(&base, &base, &square);
                big_copy(&base, &square);
                big_free(&square);
                
                big_shr(&exp_copy, 1);
            }
            
            big_free(&exp_copy);
        }
        
        big_copy(result, &temp);
        big_free(&base);
        big_free(&temp);
    }
}

void tetration_damped(BigInt *result, uint64_t base, int height,
                      const BigInt *modulus) {
    if (!result || height <= 0) return;
    
    if (height == 1) {
        big_from_int(result, base);
        return;
    }
    
    // Recursive: base^(tetration(base, height-1))
    BigInt tower;
    big_init(&tower);
    tetration_damped(&tower, base, height - 1, modulus);
    
    // Apply golden damping
    apply_golden_damping(&tower, height);
    
    // Compute base^tower mod modulus
    BigInt base_bigint;
    big_init(&base_bigint);
    big_from_int(&base_bigint, base);
    
    if (modulus) {
        big_powmod(&base_bigint, &tower, modulus, result);
    } else {
        // Without modulus, use repeated squaring
        big_from_int(result, 1);
        BigInt exp_copy;
        big_init(&exp_copy);
        big_copy(&exp_copy, &tower);
        
        while (!big_is_zero(&exp_copy)) {
            if (exp_copy.d[0] & 1) {
                BigInt prod;
                big_init(&prod);
                big_mul(result, &base_bigint, &prod);
                big_copy(result, &prod);
                big_free(&prod);
            }
            
            BigInt square;
            big_init(&square);
            big_mul(&base_bigint, &base_bigint, &square);
            big_copy(&base_bigint, &square);
            big_free(&square);
            
            big_shr(&exp_copy, 1);
        }
        
        big_free(&exp_copy);
    }
    
    big_free(&tower);
    big_free(&base_bigint);
}

/* ============================================================================
 * GEOMETRIC PRIME TESTING
 * ============================================================================ */

bool is_prime_geometric(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    // Use geometric filters
    // 1. Check clock moduli (12, 60, 118)
    if (n > 12 && n % 12 == 0) return false;
    
    // 2. Trial division up to √n
    uint64_t limit = isqrt(n);
    for (uint64_t i = 3; i <= limit; i += 2) {
        if (n % i == 0) return false;
    }
    
    return true;
}

uint64_t next_prime_geometric(uint64_t n) {
    if (n < 2) return 2;
    
    uint64_t candidate = (n % 2 == 0) ? n + 1 : n + 2;
    
    while (!is_prime_geometric(candidate)) {
        candidate += 2;
    }
    
    return candidate;
}

uint64_t count_primes_geometric(uint64_t n) {
    if (n < 2) return 0;
    
    uint64_t count = 0;
    for (uint64_t i = 2; i <= n; i++) {
        if (is_prime_geometric(i)) {
            count++;
        }
    }
    
    return count;
}

/* ============================================================================
 * LATTICE INITIALIZATION
 * ============================================================================ */

void lattice_geometry_init(void) {
    // Initialize any global structures if needed
    // For now, everything is stateless
}

void lattice_geometry_cleanup(void) {
    // Cleanup any global structures if needed
    // For now, everything is stateless
}