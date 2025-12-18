// lattice_utils.h - Lattice calculation utilities interface

#ifndef LATTICE_UTILS_H
#define LATTICE_UTILS_H

#include "app_common.h"

// Lattice position structure
typedef struct {
    int prime;
    int prime_index;
    int ring;
    int position;
    double angle;      // radians
    double radius;     // pixels
    double x, y;       // Cartesian coordinates
    bool valid;
} LatticePosition;

// Ring and position calculations
int lattice_get_ring(int prime_index);
int lattice_get_position_on_ring(int prime_index, int ring);
double lattice_get_clock_angle(int position, int ring);
double lattice_get_ring_radius(int ring, double base_radius);

// Complete position calculation
LatticePosition lattice_calculate_position(int prime, AppState* state);
void lattice_position_to_screen(LatticePosition lp, AppState* state, int* out_x, int* out_y);
bool lattice_is_visible(LatticePosition lp, AppState* state);

// Utilities
SDL_Color lattice_get_ring_color(int ring);
bool lattice_is_near_boundary(int prime);
double lattice_get_distance_from_center(int prime, AppState* state);
double lattice_get_angle_degrees(int prime, AppState* state);

#endif // LATTICE_UTILS_H