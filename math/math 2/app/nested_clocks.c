// nested_clocks.c - Nested Clocks Mode
// Fractal self-similar clock structures

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "prime_math.h"

void draw_recursive_pattern(SDL_Renderer* renderer, double cx, double cy, double size, int depth, int prime_idx, AppState* state) {
    double radius = size;
    // Mode 1: Nested Clocks - Each position contains a mini-clock
    if (depth <= 0 || radius < 5) return;
    
    int prime_index = prime_idx + 1;  // Babylonian
    if (prime_index > 232) return;
    
    int ring = get_prime_ring(prime_index);
    int position = get_position_on_ring(prime_index, ring);
    double angle = get_clock_angle(position, ring);
    
    // Draw this level's prime
    int x = cx + (int)(radius * math_cos(angle));
    int y = cy + (int)(radius * math_sin(angle));
    
    SDL_Color color;
    if (ring == 0) color = (SDL_Color){200, 200, 255, 255};
    else if (ring == 1) color = (SDL_Color){200, 255, 200, 255};
    else if (ring == 2) color = (SDL_Color){255, 200, 200, 255};
    else color = (SDL_Color){255, 255, 200, 255};
    
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
    
    // Draw mini-clock at this position
    double mini_radius = radius * 0.3;
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 128);
    for (int i = 0; i < 360; i += 30) {
        double a = i * MATH_PI / 180.0;
        int mx = x + (int)(mini_radius * math_cos(a));
        int my = y + (int)(mini_radius * math_sin(a));
        if (mx >= 0 && mx < RENDER_WIDTH && my >= 0 && my < WINDOW_HEIGHT) {
            SDL_RenderDrawPoint(renderer, mx, my);
        }
    }
    
    // Recurse to next level
    if (depth > 1) {
        draw_recursive_pattern(renderer, x, y, mini_radius, depth - 1, prime_idx + 1, state);
    }
}