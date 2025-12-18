// lattice_helpers.c - Helper functions for lattice cache visualization
#include "app_common.h"
#include "lattice_cache.h"

// Get screen position for a prime using lattice cache
// Returns true if prime found in cache, false otherwise
bool get_prime_screen_position(
    LatticeCache* cache,
    uint64_t prime,
    AppState* state,
    int* out_x,
    int* out_y,
    double* out_radius,
    double* out_angle
) {
    if (!cache || !state || !out_x || !out_y) return false;
    
    // Look up prime in cache
    PrimeLatticeEntry* entry = lattice_cache_lookup(cache, prime);
    if (!entry) return false;
    
    // Get lattice coordinates
    double radius = entry->radius;  // 3^O (exponential!)
    double angle = entry->angle;    // Full θ calculation
    
    // Apply zoom and scale
    double scaled_radius = radius * state->zoom * 0.01;  // Scale factor for visibility
    
    // Convert to Cartesian coordinates
    double x = scaled_radius * math_cos(angle);
    double y = scaled_radius * math_sin(angle);
    
    // Apply pan offset and center on screen
    int center_x = RENDER_WIDTH / 2;
    int center_y = WINDOW_HEIGHT / 2;
    
    *out_x = center_x + (int)(x + state->offset_x);
    *out_y = center_y + (int)(y + state->offset_y);
    
    // Return raw values if requested
    if (out_radius) *out_radius = radius;
    if (out_angle) *out_angle = angle;
    
    return true;
}

// Get the ring index for a prime based on its O exponent
// Returns 0 (inner), 1 (middle), or 2 (outer)
int get_prime_ring_from_cache(LatticeCache* cache, uint64_t prime) {
    if (!cache) return 0;
    
    PrimeLatticeEntry* entry = lattice_cache_lookup(cache, prime);
    if (!entry) return 0;
    
    // Use O exponent to determine ring
    // O grows logarithmically, so we can use thresholds
    double O = entry->O;
    
    if (O < 5.0) return 0;      // Inner ring (small primes)
    else if (O < 10.0) return 1; // Middle ring
    else return 2;               // Outer ring (large primes)
}

// Check if a prime is near the 143999 boundary
bool is_near_boundary(LatticeCache* cache, uint64_t prime) {
    if (!cache) return false;
    
    // 143999 is the theoretical boundary
    // Check if prime is within 10% of this value
    if (prime >= 129599 && prime <= 158399) {
        return true;
    }
    
    return false;
}

// Get color for a prime based on its lattice properties
SDL_Color get_prime_lattice_color(LatticeCache* cache, uint64_t prime) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color cyan = {0, 255, 255, 255};
    SDL_Color magenta = {255, 0, 255, 255};
    SDL_Color red = {255, 100, 100, 255};
    
    if (!cache) return white;
    
    PrimeLatticeEntry* entry = lattice_cache_lookup(cache, prime);
    if (!entry) return white;
    
    // Color based on Möbius twist
    if (entry->mobius == 1) {
        return cyan;  // Positive twist
    } else {
        return magenta;  // Negative twist
    }
    
    // Special color for boundary primes
    if (is_near_boundary(cache, prime)) {
        return red;
    }
    
    // Default based on ring
    int ring = get_prime_ring_from_cache(cache, prime);
    switch (ring) {
        case 0: return yellow;
        case 1: return cyan;
        case 2: return white;
        default: return white;
    }
}

// Draw a prime using lattice cache coordinates
void draw_prime_from_cache(
    SDL_Renderer* renderer,
    LatticeCache* cache,
    uint64_t prime,
    AppState* state,
    int size
) {
    if (!renderer || !cache || !state) return;
    
    int x, y;
    double radius, angle;
    
    if (!get_prime_screen_position(cache, prime, state, &x, &y, &radius, &angle)) {
        return;  // Prime not in cache
    }
    
    // Get color based on lattice properties
    SDL_Color color = get_prime_lattice_color(cache, prime);
    
    // Draw the prime
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Draw as filled circle
    for (int dy = -size; dy <= size; dy++) {
        for (int dx = -size; dx <= size; dx++) {
            if (dx*dx + dy*dy <= size*size) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
    
    // Highlight boundary primes with extra ring
    if (is_near_boundary(cache, prime)) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        int ring_size = size + 2;
        for (int i = 0; i < 360; i += 10) {
            double a = i * MATH_PI / 180.0;
            int rx = x + (int)(ring_size * math_cos(a));
            int ry = y + (int)(ring_size * math_sin(a));
            SDL_RenderDrawPoint(renderer, rx, ry);
        }
    }
}

// Draw the Möbius void (empty center)
void draw_mobius_void(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    int center_x = RENDER_WIDTH / 2;
    int center_y = WINDOW_HEIGHT / 2;
    
    // The void radius depends on zoom
    // At default zoom, it should be visible
    double void_radius = 50.0 * state->zoom;
    
    // Draw semi-transparent dark circle
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 180);
    
    for (int dy = -(int)void_radius; dy <= (int)void_radius; dy++) {
        for (int dx = -(int)void_radius; dx <= (int)void_radius; dx++) {
            if (dx*dx + dy*dy <= void_radius*void_radius) {
                int x = center_x + dx + (int)state->offset_x;
                int y = center_y + dy + (int)state->offset_y;
                if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
        }
    }
    
    // Draw boundary circle
    SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255);
    for (int i = 0; i < 360; i++) {
        double a = i * MATH_PI / 180.0;
        int x = center_x + (int)(void_radius * math_cos(a)) + (int)state->offset_x;
        int y = center_y + (int)(void_radius * math_sin(a)) + (int)state->offset_y;
        if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

// Draw the 143999 boundary circle
void draw_boundary_marker(
    SDL_Renderer* renderer,
    LatticeCache* cache,
    AppState* state
) {
    if (!renderer || !cache || !state) return;
    
    // Look up 143999 in cache (or closest prime)
    PrimeLatticeEntry* entry = lattice_cache_lookup(cache, 143999);
    if (!entry) {
        // Try nearby primes
        entry = lattice_cache_lookup(cache, 143993);  // Previous prime
    }
    
    if (!entry) return;
    
    // Get radius for boundary
    double boundary_radius = entry->radius * state->zoom * 0.01;
    
    int center_x = RENDER_WIDTH / 2;
    int center_y = WINDOW_HEIGHT / 2;
    
    // Draw dashed circle
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 200);
    
    for (int i = 0; i < 360; i += 2) {  // Dashed effect
        double a = i * MATH_PI / 180.0;
        int x = center_x + (int)(boundary_radius * math_cos(a)) + (int)state->offset_x;
        int y = center_y + (int)(boundary_radius * math_sin(a)) + (int)state->offset_y;
        
        if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}