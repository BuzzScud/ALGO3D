// ring_evolution.c - Ring Evolution Mode
// Shows how primes sequentially fill each ring

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "prime_math.h"

void draw_self_similar_layers(SDL_Renderer* renderer, AppState* state) {
    // Mode 4: Ring Evolution - Animate how primes fill each ring
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    
    // Draw all 4 rings
    SDL_Color ring_colors[] = {
        {80, 80, 120, 255},
        {80, 120, 80, 255},
        {120, 80, 80, 255},
        {120, 120, 80, 255}
    };
    
    for (int ring = 0; ring < 4; ring++) {
        double radius = get_ring_radius(ring, base_radius);
        SDL_SetRenderDrawColor(renderer, ring_colors[ring].r, ring_colors[ring].g, ring_colors[ring].b, 255);
        
        for (int i = 0; i < 360; i++) {
            double angle = i * MATH_PI / 180.0;
            int x = cx + (int)(radius * math_cos(angle));
            int y = cy + (int)(radius * math_sin(angle));
            SDL_RenderDrawPoint(renderer, x, y);
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
    
    // Draw primes with animation showing ring filling
    int max_primes = 232;
    if (state->prime_count < max_primes) max_primes = state->prime_count;
    
    // Use animation time to show progressive filling
    double time_factor = state->animation_phase * 0.1;
    int visible_count = (int)(time_factor * max_primes) % (max_primes + 60);
    if (visible_count > max_primes) visible_count = max_primes;
    
    for (int i = 0; i < visible_count; i++) {
        int prime_index = i + 1;
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        double radius = get_ring_radius(ring, base_radius);
        
        int x = cx + (int)(radius * math_cos(angle)) + (int)state->offset_x;
        int y = cy + (int)(radius * math_sin(angle)) + (int)state->offset_y;
        
        if (x < -10 || x >= RENDER_WIDTH+10 || y < -10 || y >= WINDOW_HEIGHT+10) {
            continue;
        }
        
        // Color based on ring
        SDL_Color color;
        if (ring == 0) color = (SDL_Color){200, 200, 255, 255};
        else if (ring == 1) color = (SDL_Color){200, 255, 200, 255};
        else if (ring == 2) color = (SDL_Color){255, 200, 200, 255};
        else color = (SDL_Color){255, 255, 200, 255};
        
        // Pulse the most recent prime
        if (i == visible_count - 1) {
            double pulse = math_sin(time_factor * 10.0) * 0.5 + 0.5;
            color.r = (int)(color.r * (0.5 + pulse * 0.5));
            color.g = (int)(color.g * (0.5 + pulse * 0.5));
            color.b = (int)(color.b * (0.5 + pulse * 0.5));
        }
        
        // Draw prime dot
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        int size = (i == visible_count - 1) ? 4 : 2;  // Larger for newest
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
    
    // Draw ring completion indicators
    int ring_boundaries[] = {12, 72, 132, 232};
    for (int r = 0; r < 4; r++) {
        if (visible_count >= ring_boundaries[r]) {
            // Ring is complete - draw checkmark
            double radius = get_ring_radius(r, base_radius);
            int check_x = cx + (int)(radius * 1.2);
            int check_y = cy;
            
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderDrawLine(renderer, check_x - 5, check_y, check_x, check_y + 5);
            SDL_RenderDrawLine(renderer, check_x, check_y + 5, check_x + 10, check_y - 10);
        }
    }
}