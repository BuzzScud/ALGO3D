// clock_abacus.h - The TRUE Clock Abacus Visualization
#ifndef CLOCK_ABACUS_H
#define CLOCK_ABACUS_H

#include "app_common.h"

// Get ring number for a prime index (1-based, Babylonian)
int get_prime_ring(int prime_index);

// Get position on ring (1-based)
int get_position_on_ring(int prime_index, int ring);

// Get angle in radians for a position on a ring
double get_clock_angle(int position, int ring);

// Get radius for a ring
double get_ring_radius(int ring, double base_radius);

// Draw the clock abacus
void draw_clock_abacus(SDL_Renderer* renderer, AppState* state);

// Optional: Draw the sacred triangle for reference
void draw_sacred_triangle(SDL_Renderer* renderer, AppState* state);

#endif // CLOCK_ABACUS_H