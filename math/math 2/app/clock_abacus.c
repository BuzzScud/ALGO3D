// clock_abacus.c - The TRUE Clock Abacus Visualization
// Based on the ancient understanding: Clock as the original abacus
// 
// Structure:
// - Ring 1: 12 primes (hours)
// - Ring 2: 60 primes (minutes)
// - Ring 3: 60 primes (seconds)
// - Ring 4: 100 primes (milliseconds)
// Total: 232 primes
//
// Only circles (π/∞/0) and prime dots are drawn
// NO spokes, NO lines between primes, NO artificial geometry

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "prime_math.h"

// Get ring number for a prime index (1-based, Babylonian)
int get_prime_ring(int prime_index) {
    if (prime_index <= 12) return 0;       // Hours
    if (prime_index <= 72) return 1;       // Minutes (12+60)
    if (prime_index <= 132) return 2;      // Seconds (72+60)
    if (prime_index <= 232) return 3;      // Milliseconds (132+100)
    return 4;                              // Beyond (continue pattern)
}

// Get position on ring (1-based)
int get_position_on_ring(int prime_index, int ring) {
    if (ring == 0) return prime_index;           // 1-12
    if (ring == 1) return prime_index - 12;      // 1-60
    if (ring == 2) return prime_index - 72;      // 1-60
    if (ring == 3) return prime_index - 132;     // 1-100
    return 0;
}

// Get angle in radians for a position on a ring
// 12 o'clock = -π/2 (top), 3 o'clock = 0 (right)
double get_clock_angle(int position, int ring) {
    double angle_deg;
    
    if (ring == 0) {
        // Hours: 30° per position, position 3 = 0°
        angle_deg = (position - 3) * 30.0;
    } else if (ring == 1 || ring == 2) {
        // Minutes/Seconds: 6° per position, position 15 = 0°
        angle_deg = (position - 15) * 6.0;
    } else {
        // Milliseconds: 3.6° per position, position 25 = 0°
        angle_deg = (position - 25) * 3.6;
    }
    
    return angle_deg * MATH_PI / 180.0;
}

// Get radius for a ring
double get_ring_radius(int ring, double base_radius) {
    if (ring == 0) return base_radius * 0.25;      // 25% (hours)
    if (ring == 1) return base_radius * 0.50;      // 50% (minutes)
    if (ring == 2) return base_radius * 0.75;      // 75% (seconds)
    if (ring == 3) return base_radius * 1.00;      // 100% (milliseconds)
    return base_radius * 1.25;                     // Beyond
}

// Draw a circle (representing π/∞/0)
void draw_circle_ring(SDL_Renderer* renderer, int cx, int cy, double radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Draw circle with many points for smoothness
    for (int i = 0; i < 360; i++) {
        double angle = i * MATH_PI / 180.0;
        int x = cx + (int)(radius * math_cos(angle));
        int y = cy + (int)(radius * math_sin(angle));
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

// Draw the clock abacus
void draw_clock_abacus(SDL_Renderer* renderer, AppState* state) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    
    // Draw the four rings (circles representing π/∞/0)
    SDL_Color ring_colors[] = {
        {80, 80, 120, 255},    // Ring 1 (hours) - subtle blue
        {80, 120, 80, 255},    // Ring 2 (minutes) - subtle green
        {120, 80, 80, 255},    // Ring 3 (seconds) - subtle red
        {120, 120, 80, 255}    // Ring 4 (milliseconds) - subtle yellow
    };
    
    for (int ring = 0; ring < 4; ring++) {
        double radius = get_ring_radius(ring, base_radius);
        draw_circle_ring(renderer, cx, cy, radius, ring_colors[ring]);
    }
    
    // Draw center point (representing 1)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            if (dx*dx + dy*dy <= 9) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
    
    // Draw primes (first 232 primes for the four rings)
    int max_primes = 232;
    if (state->prime_count < max_primes) max_primes = state->prime_count;
    
    for (int i = 0; i < max_primes; i++) {
        int prime_index = i + 1;  // Babylonian: start at 1
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        double radius = get_ring_radius(ring, base_radius);
        
        // Convert to screen coordinates
        int x = cx + (int)(radius * math_cos(angle)) + (int)state->offset_x;
        int y = cy + (int)(radius * math_sin(angle)) + (int)state->offset_y;
        
        // Skip if off-screen
        if (x < -10 || x >= RENDER_WIDTH+10 || y < -10 || y >= WINDOW_HEIGHT+10) {
            continue;
        }
        
        // Color based on ring
        SDL_Color prime_color;
        if (ring == 0) {
            prime_color = (SDL_Color){200, 200, 255, 255};  // Bright blue (hours)
        } else if (ring == 1) {
            prime_color = (SDL_Color){200, 255, 200, 255};  // Bright green (minutes)
        } else if (ring == 2) {
            prime_color = (SDL_Color){255, 200, 200, 255};  // Bright red (seconds)
        } else {
            prime_color = (SDL_Color){255, 255, 200, 255};  // Bright yellow (milliseconds)
        }
        
        // Special color for sacred positions
        if (prime_index == 3) {
            // Prime #3 = 5 at 3 o'clock (SACRED)
            prime_color = (SDL_Color){255, 255, 0, 255};  // Pure yellow
        } else if (prime_index == 12) {
            // Prime #12 = 37 at 12 o'clock (TOP)
            prime_color = (SDL_Color){255, 0, 255, 255};  // Pure magenta
        }
        
        // Draw prime dot (3-pixel for visibility)
        SDL_SetRenderDrawColor(renderer, prime_color.r, prime_color.g, prime_color.b, 255);
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 4) {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                        SDL_RenderDrawPoint(renderer, px, py);
                    }
                }
            }
        }
    }
    
    // Draw analyzed numbers (if any)
    for (int i = 0; i < state->analysis_history.count; i++) {
        int analyzed_num = state->analysis_history.numbers[i];
        SDL_Color color = state->analysis_history.colors[i];
        
        // Find this number in primes to get its index
        int prime_index = -1;
        for (int j = 0; j < state->prime_count; j++) {
            if (state->primes[j] == analyzed_num) {
                prime_index = j + 1;  // Babylonian
                break;
            }
        }
        
        if (prime_index > 0 && prime_index <= 232) {
            int ring = get_prime_ring(prime_index);
            int position = get_position_on_ring(prime_index, ring);
            double angle = get_clock_angle(position, ring);
            double radius = get_ring_radius(ring, base_radius);
            
            int x = cx + (int)(radius * math_cos(angle)) + (int)state->offset_x;
            int y = cy + (int)(radius * math_sin(angle)) + (int)state->offset_y;
            
            // Draw larger dot with outline
            bool is_selected = (state->analysis_history.selected_index == i);
            int size = is_selected ? 10 : 8;
            
            // White outline if selected
            if (is_selected) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                for (int dy = -size-2; dy <= size+2; dy++) {
                    for (int dx = -size-2; dx <= size+2; dx++) {
                        int dist_sq = dx*dx + dy*dy;
                        if (dist_sq <= (size+2)*(size+2) && dist_sq > size*size) {
                            int px = x + dx;
                            int py = y + dy;
                            if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                                SDL_RenderDrawPoint(renderer, px, py);
                            }
                        }
                    }
                }
            }
            
            // Draw colored dot
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            for (int dy = -size; dy <= size; dy++) {
                for (int dx = -size; dx <= size; dx++) {
                    if (dx*dx + dy*dy <= size*size) {
                        int px = x + dx;
                        int py = y + dy;
                        if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                            SDL_RenderDrawPoint(renderer, px, py);
                        }
                    }
                }
            }
        }
    }
}

// Wrapper functions for compatibility with existing code
double get_exact_angle(int prime) {
    // Find prime index using binary search (O(log n) instead of O(n))
    extern AppState* app_state_global;
    if (!app_state_global) return 0.0;
    
    // Use the optimized binary search function
    extern int find_prime_index(int prime, AppState* state);
    int prime_index = find_prime_index(prime, app_state_global);
    
    if (prime_index <= 0) return 0.0;
    
    int ring = get_prime_ring(prime_index);
    int position = get_position_on_ring(prime_index, ring);
    return get_clock_angle(position, ring);
}

double get_radial_distance(int prime, double zoom) {
    // Find prime index
    extern AppState* app_state_global;
    if (!app_state_global) return 0.0;
    
    // Use the optimized binary search function
    int prime_index = find_prime_index(prime, app_state_global);
    
    if (prime_index <= 0) return 0.0;
    
    int ring = get_prime_ring(prime_index);
    double base_radius = 350.0;
    return get_ring_radius(ring, base_radius) * zoom;
}

// Optional: Draw the sacred triangle for reference
void draw_sacred_triangle(SDL_Renderer* renderer, AppState* state) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    
    // Get positions of the three sacred points
    // Point 1: Center (1)
    int p1_x = cx;
    int p1_y = cy;
    
    // Point 2: 12 o'clock (Prime #12 = 37)
    double angle_12 = -MATH_PI / 2.0;  // -90° = top
    double radius_12 = get_ring_radius(0, base_radius);  // Ring 0 (hours)
    int p2_x = cx + (int)(radius_12 * math_cos(angle_12));
    int p2_y = cy + (int)(radius_12 * math_sin(angle_12));
    
    // Point 3: 3 o'clock (Prime #3 = 5)
    double angle_3 = 0.0;  // 0° = right
    double radius_3 = get_ring_radius(0, base_radius);  // Ring 0 (hours)
    int p3_x = cx + (int)(radius_3 * math_cos(angle_3));
    int p3_y = cy + (int)(radius_3 * math_sin(angle_3));
    
    // Draw triangle edges (subtle)
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 128);
    SDL_RenderDrawLine(renderer, p1_x, p1_y, p2_x, p2_y);  // Center to 12
    SDL_RenderDrawLine(renderer, p2_x, p2_y, p3_x, p3_y);  // 12 to 3
    SDL_RenderDrawLine(renderer, p3_x, p3_y, p1_x, p1_y);  // 3 to center
}