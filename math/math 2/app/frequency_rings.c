// frequency_rings.c - Frequency Rings Mode
// Rings pulse at different frequencies

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "prime_math.h"

void draw_harmonic_resonance(SDL_Renderer* renderer, AppState* state) {
    // Mode 2: Frequency Rings - Each ring vibrates at its frequency
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    double time = state->animation_phase * 0.1;
    
    // Draw pulsing rings
    SDL_Color ring_colors[] = {
        {80, 80, 120, 255},
        {80, 120, 80, 255},
        {120, 80, 80, 255},
        {120, 120, 80, 255}
    };
    
    double frequencies[] = {12.0, 60.0, 60.0, 100.0};  // Hz
    
    for (int ring = 0; ring < 4; ring++) {
        double pulse = math_sin(time * frequencies[ring] * 0.1) * 0.3 + 0.7;
        double radius = get_ring_radius(ring, base_radius) * pulse;
        
        SDL_SetRenderDrawColor(renderer, 
            (int)(ring_colors[ring].r * pulse),
            (int)(ring_colors[ring].g * pulse),
            (int)(ring_colors[ring].b * pulse), 255);
        
        for (int i = 0; i < 360; i++) {
            double angle = i * MATH_PI / 180.0;
            int x = cx + (int)(radius * math_cos(angle));
            int y = cy + (int)(radius * math_sin(angle));
            if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
    
    // Draw center
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            if (dx*dx + dy*dy <= 9) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
    
    // Draw primes with pulsing based on their ring frequency
    int max_primes = 232;
    if (state->prime_count < max_primes) max_primes = state->prime_count;
    
    for (int i = 0; i < max_primes; i++) {
        int prime_index = i + 1;
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        
        double pulse = math_sin(time * frequencies[ring] * 0.1 + angle) * 0.3 + 0.7;
        double radius = get_ring_radius(ring, base_radius) * pulse;
        
        int x = cx + (int)(radius * math_cos(angle)) + (int)state->offset_x;
        int y = cy + (int)(radius * math_sin(angle)) + (int)state->offset_y;
        
        if (x < -10 || x >= RENDER_WIDTH+10 || y < -10 || y >= WINDOW_HEIGHT+10) {
            continue;
        }
        
        SDL_Color color;
        if (ring == 0) color = (SDL_Color){200, 200, 255, 255};
        else if (ring == 1) color = (SDL_Color){200, 255, 200, 255};
        else if (ring == 2) color = (SDL_Color){255, 200, 200, 255};
        else color = (SDL_Color){255, 255, 200, 255};
        
        color.r = (int)(color.r * pulse);
        color.g = (int)(color.g * pulse);
        color.b = (int)(color.b * pulse);
        
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