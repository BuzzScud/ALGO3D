// clock_4d.c - 4D Clock Rotation Mode
// Each ring represents a dimension, rotates through 4D space

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "prime_math.h"

void draw_hyperdim_projection(SDL_Renderer* renderer, AppState* state) {
    // Mode 3: 4D Clock - Rotate through 4D space
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    double time = state->animation_phase * 0.05;
    
    // 4D rotation angles
    double rot_xy = time;
    double rot_zw = time * 0.7;
    
    // Draw rings with 4D projection
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
            
            // 4D coordinates
            double x4d = radius * math_cos(angle);
            double y4d = radius * math_sin(angle);
            double z4d = ring * 50.0;  // Depth based on ring
            double w4d = math_sin(time + ring) * 100.0;  // 4th dimension
            
            // Rotate in 4D
            double x_rot = x4d * math_cos(rot_xy) - y4d * math_sin(rot_xy);
            double y_rot = x4d * math_sin(rot_xy) + y4d * math_cos(rot_xy);
            double z_rot = z4d * math_cos(rot_zw) - w4d * math_sin(rot_zw);
            
            // Project to 2D
            double perspective = 1.0 / (1.0 + z_rot / 500.0);
            int x = cx + (int)(x_rot * perspective);
            int y = cy + (int)(y_rot * perspective);
            
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
    
    // Draw primes with 4D rotation
    int max_primes = 232;
    if (state->prime_count < max_primes) max_primes = state->prime_count;
    
    for (int i = 0; i < max_primes; i++) {
        int prime_index = i + 1;
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        double radius = get_ring_radius(ring, base_radius);
        
        // 4D coordinates
        double x4d = radius * math_cos(angle);
        double y4d = radius * math_sin(angle);
        double z4d = ring * 50.0;
        double w4d = math_sin(time + ring + angle) * 100.0;
        
        // Rotate
        double x_rot = x4d * math_cos(rot_xy) - y4d * math_sin(rot_xy);
        double y_rot = x4d * math_sin(rot_xy) + y4d * math_cos(rot_xy);
        double z_rot = z4d * math_cos(rot_zw) - w4d * math_sin(rot_zw);
        
        // Project
        double perspective = 1.0 / (1.0 + z_rot / 500.0);
        int x = cx + (int)(x_rot * perspective) + (int)state->offset_x;
        int y = cy + (int)(y_rot * perspective) + (int)state->offset_y;
        
        if (x < -10 || x >= RENDER_WIDTH+10 || y < -10 || y >= WINDOW_HEIGHT+10) {
            continue;
        }
        
        SDL_Color color;
        if (ring == 0) color = (SDL_Color){200, 200, 255, 255};
        else if (ring == 1) color = (SDL_Color){200, 255, 200, 255};
        else if (ring == 2) color = (SDL_Color){255, 200, 200, 255};
        else color = (SDL_Color){255, 255, 200, 255};
        
        // Fade based on depth
        int alpha = (int)(255 * perspective);
        if (alpha > 255) alpha = 255;
        if (alpha < 50) alpha = 50;
        
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
        int size = (int)(3 * perspective);
        if (size < 1) size = 1;
        
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