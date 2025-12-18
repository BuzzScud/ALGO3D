// app/natural_prime_lattice.c - Minimal Clock Abacus
// Shows ONLY prime dots at clock positions - NO circles
// Reveals the pure prime pattern in clock structure

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "prime_math.h"

// Draw natural prime lattice - ONLY primes, NO circles
void draw_natural_prime_lattice(SDL_Renderer* renderer, AppState* state) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    
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
            prime_color = (SDL_Color){255, 255, 0, 255};  // Pure yellow (3 o'clock)
        } else if (prime_index == 12) {
            prime_color = (SDL_Color){255, 0, 255, 255};  // Pure magenta (12 o'clock)
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