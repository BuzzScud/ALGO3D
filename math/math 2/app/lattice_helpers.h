// lattice_helpers.h - Helper functions for lattice cache visualization
#ifndef LATTICE_HELPERS_H
#define LATTICE_HELPERS_H

#include "app_common.h"
#include "lattice_cache.h"

// Get screen position for a prime using lattice cache
bool get_prime_screen_position(
    LatticeCache* cache,
    uint64_t prime,
    AppState* state,
    int* out_x,
    int* out_y,
    double* out_radius,
    double* out_angle
);

// Get the ring index for a prime based on its O exponent
int get_prime_ring_from_cache(LatticeCache* cache, uint64_t prime);

// Check if a prime is near the 143999 boundary
bool is_near_boundary(LatticeCache* cache, uint64_t prime);

// Get color for a prime based on its lattice properties
SDL_Color get_prime_lattice_color(LatticeCache* cache, uint64_t prime);

// Draw a prime using lattice cache coordinates
void draw_prime_from_cache(
    SDL_Renderer* renderer,
    LatticeCache* cache,
    uint64_t prime,
    AppState* state,
    int size
);

// Draw the Möbius void (empty center)
void draw_mobius_void(SDL_Renderer* renderer, AppState* state);

// Draw the 143999 boundary circle
void draw_boundary_marker(
    SDL_Renderer* renderer,
    LatticeCache* cache,
    AppState* state
);

#endif // LATTICE_HELPERS_H