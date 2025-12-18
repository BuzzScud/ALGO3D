// clock_crystalline.c - Crystalline Lattice Clock Transformation
// CORRECT BABYLONIAN CLOCK STRUCTURE - Maps primes to clock ticks

#include "app_common.h"
#include "clock_lattice.h"

// Get Babylonian clock position for visualization
// This uses the CORRECT clock tick mapping (not spiral)
void get_crystalline_position_fast(int prime, uint64_t prime_index, double *angle, double *radius, AppState* state) {
    // Use the correct Babylonian clock mapping
    BabylonianClockPosition pos = map_prime_index_to_clock((int)prime_index);
    
    // Return angle and radius for visualization
    *angle = pos.angle;
    *radius = pos.radius * 350.0 * state->zoom;  // Scale to screen size
    
    // Add small perturbation based on prime's modular relationships
    PrimeModular pm = get_prime_modular((uint64_t)prime);
    *angle += 0.01 * (pm.mod_12 / 12.0);  // Small variation based on mod 12
}

// Draw clock sudoku with crystalline lattice transformation
// BABYLONIAN CLOCK STRUCTURE - Shows true ring structure
void draw_clock_sudoku_crystalline(SDL_Renderer* renderer, AppState* state) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    
    // Apply offset
    cx += (int)state->offset_x;
    cy += (int)state->offset_y;
    
    double base_radius = 350.0 * state->zoom;
    
    // Draw the four Babylonian rings
    SDL_Color ring_colors[] = {
        {80, 80, 120, 255},    // Ring 0: Hours (outer)
        {80, 120, 80, 255},    // Ring 1: Minutes
        {120, 80, 80, 255},    // Ring 2: Seconds
        {120, 120, 80, 255}    // Ring 3: Milliseconds (inner)
    };
    
    double ring_radii[] = {
        base_radius * 0.25,  // Hours (outer)
        base_radius * 0.50,  // Minutes
        base_radius * 0.75,  // Seconds
        base_radius * 1.00   // Milliseconds (inner)
    };
    
    // Draw rings
    for (int ring = 0; ring < 4; ring++) {
        SDL_SetRenderDrawColor(renderer, ring_colors[ring].r, 
                              ring_colors[ring].g, ring_colors[ring].b, 128);
        
        for (int i = 0; i < 360; i++) {
            double angle = i * MATH_PI / 180.0;
            int x = cx + (int)(ring_radii[ring] * math_cos(angle));
            int y = cy + (int)(ring_radii[ring] * math_sin(angle));
            if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
    
    // Draw clock ticks (12, 3, 6, 9 o'clock)
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for (int hour = 0; hour < 12; hour++) {
        double angle = (hour - 3) * (2.0 * MATH_PI / 12.0);  // 3 o'clock = 0°
        int x1 = cx + (int)(ring_radii[0] * 0.9 * math_cos(angle));
        int y1 = cy + (int)(ring_radii[0] * 0.9 * math_sin(angle));
        int x2 = cx + (int)(ring_radii[3] * 1.1 * math_cos(angle));
        int y2 = cy + (int)(ring_radii[3] * 1.1 * math_sin(angle));
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    
    // Highlight sacred positions
    // 3 o'clock (π)
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  // Gold
    double pi_angle = 0.0;  // 3 o'clock
    int pi_x = cx + (int)(ring_radii[0] * math_cos(pi_angle));
    int pi_y = cy + (int)(ring_radii[0] * math_sin(pi_angle));
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            if (dx*dx + dy*dy <= 16) {
                SDL_RenderDrawPoint(renderer, pi_x + dx, pi_y + dy);
            }
        }
    }
    
    // 12 o'clock (top)
    double top_angle = -MATH_PI / 2.0;
    int top_x = cx + (int)(ring_radii[0] * math_cos(top_angle));
    int top_y = cy + (int)(ring_radii[0] * math_sin(top_angle));
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            if (dx*dx + dy*dy <= 16) {
                SDL_RenderDrawPoint(renderer, top_x + dx, top_y + dy);
            }
        }
    }
    
    // Draw primes at their clock positions
    
    // Draw primes (first 232 for the four rings)
    int max_primes = 232;
    if (state->visible_primes < max_primes) max_primes = state->visible_primes;
    if (max_primes > MAX_PRIMES) max_primes = MAX_PRIMES;
    
    for (int i = 0; i < max_primes; i++) {
        int prime = state->primes[i];
        uint64_t prime_index = (uint64_t)(i + 1);
        
        // Get Babylonian clock position
        BabylonianClockPosition pos = map_prime_index_to_clock((int)prime_index);
        
        double angle, radius;
        get_crystalline_position_fast(prime, prime_index, &angle, &radius, state);
        
        // Calculate screen position
        int x = cx + (int)(radius * math_cos(angle));
        int y = cy + (int)(radius * math_sin(angle));
        
        // Skip if off-screen
        if (x < -20 || x >= RENDER_WIDTH + 20 || y < -20 || y >= WINDOW_HEIGHT + 20) {
            continue;
        }
        
        // Color based on ring (Babylonian structure)
        SDL_Color prime_color = ring_colors[pos.ring];
        prime_color.r += 100;  // Brighten for visibility
        prime_color.g += 100;
        prime_color.b += 100;
        
        // Highlight if sacred position
        if (is_sacred_position(pos)) {
            prime_color = (SDL_Color){255, 255, 0, 255};  // Yellow for sacred
        }
        
        // Highlight analyzed number
        if (state->number_analysis.number == prime) {
            prime_color = (SDL_Color){255, 0, 255, 255};  // Magenta for analyzed
            // Draw larger for visibility
            SDL_SetRenderDrawColor(renderer, prime_color.r, prime_color.g, prime_color.b, prime_color.a);
            for (int dy = -5; dy <= 5; dy++) {
                for (int dx = -5; dx <= 5; dx++) {
                    if (dx*dx + dy*dy <= 25) {
                        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                    }
                }
            }
            continue;
        }
        
        SDL_SetRenderDrawColor(renderer, prime_color.r, prime_color.g, prime_color.b, prime_color.a);
        
        // Draw prime as filled circle
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 4) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    }
    
    // Draw center point
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
        }
    }
    
    // Draw analyzed number separately (even if not in primes array)
    if (state->number_analysis.number > 0) {
        int analyzed_num = state->number_analysis.number;
        
        // Calculate position using EXACT same crystalline transformation as primes
        uint64_t prime_index = 0;
        // Find approximate index (for positioning)
        for (int i = 0; i < state->prime_count; i++) {
            if (state->primes[i] >= analyzed_num) {
                prime_index = (uint64_t)i;
                break;
            }
        }
        if (prime_index == 0) prime_index = (uint64_t)state->prime_count;
        
        // Use EXACT same function as regular primes
        double angle, radius;
        get_crystalline_position_fast(analyzed_num, prime_index, &angle, &radius, state);
        
        int x = cx + (int)(radius * math_cos(angle));
        int y = cy + (int)(radius * math_sin(angle));
        
        // Draw with bright yellow/gold highlighting (extra large)
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        
        // Draw large highlighted point (12 pixel radius)
        for (int dy = -12; dy <= 12; dy++) {
            for (int dx = -12; dx <= 12; dx++) {
                if (dx*dx + dy*dy <= 144) {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                        SDL_RenderDrawPoint(renderer, px, py);
                    }
                }
            }
        }
        
        // Draw pulsing ring around it
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 200);
        for (int r = 14; r <= 18; r++) {
            for (int a = 0; a < 360; a += 5) {
                int px = x + (int)(r * math_cos(a * MATH_PI / 180.0));
                int py = y + (int)(r * math_sin(a * MATH_PI / 180.0));
                if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }
    }
    
    // Draw label
    SDL_Color label_color = {100, 255, 100, 255};
    draw_text(renderer, "CRYSTALLINE LATTICE (Pure Prime Distribution)", 50, WINDOW_HEIGHT - 30, label_color);
}