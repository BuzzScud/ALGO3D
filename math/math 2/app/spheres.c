// app/spheres.c - 3D sphere visualization implementation
#include "prime_hyperdim.h"
// CORRECTED to work with actual prime_math library

#include "app_common.h"

void init_spheres(AppState* state) {
    state->sphere_count = math_min(20, state->prime_count);
    state->spheres = (Sphere*)calloc(state->sphere_count, sizeof(Sphere));
    
    for (int i = 0; i < state->sphere_count; i++) {
        int prime = state->primes[i];
        HyperVector hv;
        hv = prime_to_hypervector(prime);
        
        state->spheres[i].x = hv.components[0] * 100;
        state->spheres[i].y = hv.components[1] * 100;
        state->spheres[i].z = hv.components[2] * 50;
        
        state->spheres[i].radius = 5 + math_log((double)prime) / 2;
        
        SDL_Color color = get_vibrational_color(prime / 100.0, prime);
        state->spheres[i].color = color;
        
        state->spheres[i].conn_count = 0;
        for (int j = 0; j < i && state->spheres[i].conn_count < 8; j++) {
            int other_prime = state->primes[j];
            
            // Check if twin primes or close primes
            if (abs(prime - other_prime) == 2 || abs(prime - other_prime) < 10) {
                state->spheres[i].connections[state->spheres[i].conn_count++] = j;
            }
        }
        
        // Free hypervector components
        hypervector_free(&hv);
    }
}

void draw_spheres(SDL_Renderer* renderer, AppState* state) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    
    // Sort spheres by Z coordinate for proper depth rendering
    for (int i = 0; i < state->sphere_count - 1; i++) {
        for (int j = i + 1; j < state->sphere_count; j++) {
            if (state->spheres[i].z < state->spheres[j].z) {
                Sphere temp = state->spheres[i];
                state->spheres[i] = state->spheres[j];
                state->spheres[j] = temp;
            }
        }
    }
    
    // Draw connections first (behind spheres)
    if (state->show_connections) {
        for (int i = 0; i < state->sphere_count; i++) {
            for (int c = 0; c < state->spheres[i].conn_count; c++) {
                int j = state->spheres[i].connections[c];
                
                // 3D to 2D projection
                double scale1 = 200.0 / (200 + state->spheres[i].z);
                double scale2 = 200.0 / (200 + state->spheres[j].z);
                
                int x1 = cx + (int)(state->spheres[i].x * scale1 * state->zoom) + state->offset_x;
                int y1 = cy + (int)(state->spheres[i].y * scale1 * state->zoom) + state->offset_y;
                
                int x2 = cx + (int)(state->spheres[j].x * scale2 * state->zoom) + state->offset_x;
                int y2 = cy + (int)(state->spheres[j].y * scale2 * state->zoom) + state->offset_y;
                
                // Draw connection with depth-based alpha
                int alpha = 100 - (int)((state->spheres[i].z + state->spheres[j].z) / 4);
                alpha = int_clamp(alpha, 20, 100);
                
                SDL_SetRenderDrawColor(renderer, 100, 150, 200, alpha);
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            }
        }
    }
    
    // Draw spheres
    for (int i = 0; i < state->sphere_count; i++) {
        // Apply rotation
        double rot_x = state->spheres[i].x;
        double rot_y = state->spheres[i].y * math_cos(state->rotation_x) + 
                       state->spheres[i].z * math_sin(state->rotation_x);
        double rot_z = state->spheres[i].z * math_cos(state->rotation_x) - 
                       state->spheres[i].y * math_sin(state->rotation_x);
        
        // Rotate around Y axis
        double final_x = rot_x * math_cos(state->rotation_y) + rot_z * math_sin(state->rotation_y);
        double final_z = rot_z * math_cos(state->rotation_y) - rot_x * math_sin(state->rotation_y);
        
        // 3D to 2D projection with perspective
        double perspective_scale = 200.0 / (200 + final_z);
        
        int screen_x = cx + (int)(final_x * perspective_scale * state->zoom) + state->offset_x;
        int screen_y = cy + (int)(rot_y * perspective_scale * state->zoom) + state->offset_y;
        
        // Calculate sphere size based on distance
        int sphere_radius = (int)(state->spheres[i].radius * perspective_scale * state->zoom);
        
        // Draw sphere with shading
        for (int dy = -sphere_radius; dy <= sphere_radius; dy++) {
            for (int dx = -sphere_radius; dx <= sphere_radius; dx++) {
                double dist_sq = dx*dx + dy*dy;
                if (dist_sq <= sphere_radius * sphere_radius) {
                    double dist = math_sqrt(dist_sq);
                    double shade_factor = 1.0 - (dist / sphere_radius);
                    
                    SDL_Color color = state->spheres[i].color;
                    color.r = (uint8_t)(color.r * shade_factor);
                    color.g = (uint8_t)(color.g * shade_factor);
                    color.b = (uint8_t)(color.b * shade_factor);
                    
                    color.a = 255 - (int)((final_z + 100) / 2);
                    color.a = int_clamp(color.a, 0, 255);
                    
                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                    SDL_RenderDrawPoint(renderer, screen_x + dx, screen_y + dy);
                }
            }
        }
        
        // Draw prime number label
        if (sphere_radius > 10) {
            char label[16];
            snprintf(label, sizeof(label), "%d", state->primes[i]);
            SDL_Color white = {255, 255, 255, 255};
            draw_text(renderer, label, screen_x - 10, screen_y - 5, white);
        }
    }
    
    // Update rotation if animating
    if (state->animate) {
        state->rotation_x += 0.01;
        state->rotation_y += 0.007;
        state->rotation_z += 0.003;
        
        // Oscillate sphere positions
        for (int i = 0; i < state->sphere_count; i++) {
            double oscillation = math_sin(state->animation_phase + i * 0.5);
            state->spheres[i].z += oscillation * 2;
        }
    }
    
    // Draw title
    SDL_Color title_color = {100, 200, 255, 255};
    draw_text(renderer, "3D PRIME SPHERE VISUALIZATION", cx - 100, 50, title_color);
    
    // Draw info
    char info[128];
    snprintf(info, sizeof(info), "Spheres: %d | Connections: Twin Primes", state->sphere_count);
    SDL_Color info_color = {200, 200, 200, 255};
    draw_text(renderer, info, 50, WINDOW_HEIGHT - 30, info_color);
}