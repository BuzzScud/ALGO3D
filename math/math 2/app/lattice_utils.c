// lattice_utils.c - Centralized lattice calculation utilities
// All lattice-related position and coordinate calculations

#include "app_common.h"
#include <stdlib.h>

// Get ring index for a prime (0-3 for the 4 clock rings)
int lattice_get_ring(int prime_index) {
    // Ring 0 (Hours): primes 1-12
    if (prime_index <= 12) return 0;
    // Ring 1 (Minutes): primes 13-72
    if (prime_index <= 72) return 1;
    // Ring 2 (Seconds): primes 73-132
    if (prime_index <= 132) return 2;
    // Ring 3 (Milliseconds): primes 133-232
    if (prime_index <= 232) return 3;
    // Beyond
    return 4;
}

// Get position on ring (0-based position within the ring)
int lattice_get_position_on_ring(int prime_index, int ring) {
    if (ring == 0) {
        // Hours: 12 positions
        return (prime_index - 1) % 12;
    } else if (ring == 1) {
        // Minutes: 60 positions
        return (prime_index - 13) % 60;
    } else if (ring == 2) {
        // Seconds: 60 positions
        return (prime_index - 73) % 60;
    } else if (ring == 3) {
        // Milliseconds: 100 positions
        return (prime_index - 133) % 100;
    }
    return 0;
}

// Get clock angle for a position on a ring (in radians)
double lattice_get_clock_angle(int position, int ring) {
    double angle_deg;
    
    if (ring == 0) {
        // Hours: 30 degrees per position, position 3 = 0 degrees (3 o'clock = 0)
        angle_deg = (position - 3) * 30.0;
    } else if (ring == 1 || ring == 2) {
        // Minutes/Seconds: 6 degrees per position, position 15 = 0 degrees
        angle_deg = (position - 15) * 6.0;
    } else {
        // Milliseconds: 3.6 degrees per position, position 25 = 0 degrees
        angle_deg = (position - 25) * 3.6;
    }
    
    return angle_deg * MATH_PI / 180.0;
}

// Get radius for a ring (as fraction of base radius)
double lattice_get_ring_radius(int ring, double base_radius) {
    if (ring == 0) return base_radius * 0.25;  // 25% (hours)
    if (ring == 1) return base_radius * 0.50;  // 50% (minutes)
    if (ring == 2) return base_radius * 0.75;  // 75% (seconds)
    if (ring == 3) return base_radius * 1.00;  // 100% (milliseconds)
    return base_radius * 1.25;                 // Beyond
}

// Complete lattice position calculation for a prime
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

LatticePosition lattice_calculate_position(int prime, AppState* state) {
    LatticePosition lp = {0};
    lp.prime = prime;
    lp.valid = false;
    
    // Find prime index
    int prime_index = find_prime_index(prime, state);
    if (prime_index < 0) return lp;
    
    lp.prime_index = prime_index;
    lp.ring = lattice_get_ring(prime_index);
    lp.position = lattice_get_position_on_ring(prime_index, lp.ring);
    lp.angle = lattice_get_clock_angle(lp.position, lp.ring);
    lp.radius = lattice_get_ring_radius(lp.ring, 350.0 * state->zoom);
    
    // Convert to Cartesian
    lp.x = lp.radius * math_cos(lp.angle);
    lp.y = lp.radius * math_sin(lp.angle);
    
    lp.valid = true;
    return lp;
}

// Convert lattice position to screen coordinates
void lattice_position_to_screen(LatticePosition lp, AppState* state, int* out_x, int* out_y) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    
    *out_x = cx + (int)lp.x + (int)state->offset_x;
    *out_y = cy + (int)lp.y + (int)state->offset_y;
}

// Check if a lattice position is visible on screen
bool lattice_is_visible(LatticePosition lp, AppState* state) {
    int x, y;
    lattice_position_to_screen(lp, state, &x, &y);
    return (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < WINDOW_HEIGHT);
}

// Get color for a prime based on its ring
SDL_Color lattice_get_ring_color(int ring) {
    SDL_Color colors[] = {
        {100, 150, 255, 255},  // Ring 0: Blue
        {100, 255, 150, 255},  // Ring 1: Green
        {255, 150, 100, 255},  // Ring 2: Orange
        {255, 255, 100, 255},  // Ring 3: Yellow
        {200, 200, 200, 255}   // Beyond: Gray
    };
    
    if (ring >= 0 && ring < 5) {
        return colors[ring];
    }
    return colors[4];
}

// Check if prime is near the 143999 boundary
bool lattice_is_near_boundary(int prime) {
    return (prime >= 129599 && prime <= 158399);  // Within 10% of 143999
}

// Get distance from center for a prime
double lattice_get_distance_from_center(int prime, AppState* state) {
    LatticePosition lp = lattice_calculate_position(prime, state);
    if (!lp.valid) return 0.0;
    return math_sqrt(lp.x * lp.x + lp.y * lp.y);
}

// Get angle in degrees for a prime
double lattice_get_angle_degrees(int prime, AppState* state) {
    LatticePosition lp = lattice_calculate_position(prime, state);
    if (!lp.valid) return 0.0;
    return lp.angle * 180.0 / MATH_PI;
}