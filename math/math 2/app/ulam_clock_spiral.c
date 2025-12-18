// ulam_clock_spiral.c - Clock Spiral Mode
// Primes spiral outward from center following clock positions

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "prime_math.h"

void draw_enhanced_ulam_spiral(SDL_Renderer* renderer, AppState* state) {
    // Mode 0: Clock Spiral - Primes spiral outward from center in clock positions
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    
    // Draw subtle spiral guide (optional)
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    for (int i = 0; i < 360; i++) {
        double angle = i * MATH_PI / 180.0;
        double r = (i / 360.0) * base_radius;
        int x = cx + (int)(r * math_cos(angle));
        int y = cy + (int)(r * math_sin(angle));
        if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    
    // Draw center point
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            if (dx*dx + dy*dy <= 9) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
    
    // Draw primes spiraling outward
    int max_primes = 232;
    if (state->prime_count < max_primes) max_primes = state->prime_count;
    
    for (int i = 0; i < max_primes; i++) {
        int prime_index = i + 1;  // Babylonian
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        
        // Spiral effect: radius increases with index
        double spiral_radius = (prime_index / 232.0) * base_radius;
        
        int x = cx + (int)(spiral_radius * math_cos(angle)) + (int)state->offset_x;
        int y = cy + (int)(spiral_radius * math_sin(angle)) + (int)state->offset_y;
        
        if (x < -10 || x >= RENDER_WIDTH+10 || y < -10 || y >= WINDOW_HEIGHT+10) {
            continue;
        }
        
        // Color based on ring
        SDL_Color color;
        if (ring == 0) color = (SDL_Color){200, 200, 255, 255};
        else if (ring == 1) color = (SDL_Color){200, 255, 200, 255};
        else if (ring == 2) color = (SDL_Color){255, 200, 200, 255};
        else color = (SDL_Color){255, 255, 200, 255};
        
        // Draw prime dot
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
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
}
