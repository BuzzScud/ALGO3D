// app/enhanced_visualization.c - Enhanced visualization using compact vectors
// Integrates 360-degree precision and Platonic solid overlays

#include "app_common.h"
#include "math/compact_vector.h"
#include "math/prime.h"
#include "prime_math_custom.h"  // For prime_cos, prime_sin, etc.

// Get precise 360-degree angle for a number using compact vectors
double get_precise_angle(int number, ClockContext* clock_ctx) {
    if (!clock_ctx || number <= 0) return 0.0;
    
    // Create compact vector for the number
    CompactVector vec;
    MathError err = compact_vector_create(&vec, number, clock_ctx);
    if (err != MATH_SUCCESS) return 0.0;
    
    // Get precise clock position with factor refinement
    PreciseClockPosition pos;
    err = get_precise_clock_position(&vec, &pos, clock_ctx);
    if (err != MATH_SUCCESS) return 0.0;
    
    // Return the refined angle (0-360 degrees)
    return pos.refined_angle;
}

// Draw enhanced factor lines with 360-degree precision
void draw_enhanced_factor_lines(SDL_Renderer* renderer, AppState* state, 
                                int cx, int cy, double scale) {
    if (!state->show_factors || state->analysis_history.count == 0) return;
    if (!state->clock_ctx) return;
    
    // Determine which numbers to show factors for
    int start_idx = 0;
    int end_idx = state->analysis_history.count;
    
    if (state->analysis_history.selected_index >= 0) {
        // Show only selected number's factors
        start_idx = state->analysis_history.selected_index;
        end_idx = start_idx + 1;
    }
    
    // Draw factors for each number in range
    for (int h = start_idx; h < end_idx; h++) {
        int num = state->analysis_history.numbers[h];
        SDL_Color color = state->analysis_history.colors[h];
        
        // Get number position using precise angle
        double num_angle_deg = get_precise_angle(num, state->clock_ctx);
        double num_angle_rad = num_angle_deg * MATH_PI / 180.0;
        double num_radius = get_radial_distance(num, state->zoom) * scale;
        int num_x = cx + (int)(num_radius * math_cos(num_angle_rad));
        int num_y = cy + (int)(num_radius * math_sin(num_angle_rad));
        
        // Find factors
        for (int i = 2; i < num; i++) {
            if (num % i == 0) {
                // This is a factor - draw line to it with precise angle
                double factor_angle_deg = get_precise_angle(i, state->clock_ctx);
                double factor_angle_rad = factor_angle_deg * MATH_PI / 180.0;
                double factor_radius = get_radial_distance(i, state->zoom) * scale;
                int factor_x = cx + (int)(factor_radius * math_cos(factor_angle_rad));
                int factor_y = cy + (int)(factor_radius * math_sin(factor_angle_rad));
                
                // Calculate angle difference for color intensity
                double angle_diff = math_abs(num_angle_deg - factor_angle_deg);
                if (angle_diff > 180.0) angle_diff = 360.0 - angle_diff;
                
                // Use angle difference to modulate alpha (closer = brighter)
                int alpha = (int)(255 * (1.0 - angle_diff / 180.0) * 0.5);
                if (alpha < 50) alpha = 50;
                
                // Draw line with varying thickness based on factor size
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
                SDL_RenderDrawLine(renderer, num_x, num_y, factor_x, factor_y);
                
                // Draw thicker line for prime factors
                if (prime_is_prime(i)) {
                    SDL_RenderDrawLine(renderer, num_x + 1, num_y, factor_x + 1, factor_y);
                    SDL_RenderDrawLine(renderer, num_x, num_y + 1, factor_x, factor_y + 1);
                }
            }
        }
        
        // Draw phase offset indicator if available
        CompactVector vec;
        if (compact_vector_create(&vec, num, state->clock_ctx) == MATH_SUCCESS) {
            PreciseClockPosition pos;
            if (get_precise_clock_position(&vec, &pos, state->clock_ctx) == MATH_SUCCESS) {
                // Draw small circle at base position
                double base_angle_rad = pos.base_angle * MATH_PI / 180.0;
                int base_x = cx + (int)(num_radius * math_cos(base_angle_rad));
                int base_y = cy + (int)(num_radius * math_sin(base_angle_rad));
                
                // Draw arc from base to refined position
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 150);
                for (double t = 0.0; t <= 1.0; t += 0.1) {
                    double interp_angle = pos.base_angle + t * pos.phase_offset;
                    double interp_rad = interp_angle * MATH_PI / 180.0;
                    int arc_x = cx + (int)(num_radius * math_cos(interp_rad));
                    int arc_y = cy + (int)(num_radius * math_sin(interp_rad));
                    SDL_RenderDrawPoint(renderer, arc_x, arc_y);
                }
            }
        }
    }
}

// Draw Platonic solid overlay for a number
void draw_platonic_solid_overlay(SDL_Renderer* renderer, AppState* state,
                                 int number, int cx, int cy, double scale) {
    if (!state->clock_ctx || number <= 0) return;
    
    // Create compact vector for the number
    CompactVector vec;
    if (compact_vector_create(&vec, number, state->clock_ctx) != MATH_SUCCESS) return;
    
    // Select Platonic solid based on magnitude
    PlatonicSolidType solid_type;
    if (vec.magnitude_offset < 12) {
        solid_type = PLATONIC_TETRAHEDRON;  // 4 vertices
    } else if (vec.magnitude_offset < 60) {
        solid_type = PLATONIC_CUBE;         // 8 vertices
    } else if (vec.magnitude_offset < 144) {
        solid_type = PLATONIC_OCTAHEDRON;   // 6 vertices
    } else if (vec.magnitude_offset < 720) {
        solid_type = PLATONIC_DODECAHEDRON; // 20 vertices
    } else {
        solid_type = PLATONIC_ICOSAHEDRON;  // 12 vertices
    }
    
    // Get the solid
    PlatonicSolid solid;
    if (get_platonic_solid(solid_type, &solid) != MATH_SUCCESS) return;
    
    // Get number position
    double num_angle_deg = get_precise_angle(number, state->clock_ctx);
    double num_angle_rad = num_angle_deg * MATH_PI / 180.0;
    double num_radius = get_radial_distance(number, state->zoom) * scale;
    int num_x = cx + (int)(num_radius * math_cos(num_angle_rad));
    int num_y = cy + (int)(num_radius * math_sin(num_angle_rad));
    
    // Draw solid vertices around the number
    double solid_radius = 20.0 * (1.0 + vec.magnitude_offset / 100.0);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180);
    
    for (int i = 0; i < solid.num_vertices; i++) {
        // Project 3D vertex to 2D (simple orthographic projection)
        double x = solid.vertices[i * 3 + 0];
        double y = solid.vertices[i * 3 + 1];
        // z coordinate used for depth (ignored for now)
        
        // Rotate by number's angle
        double rotated_x = x * math_cos(num_angle_rad) - y * math_sin(num_angle_rad);
        double rotated_y = x * math_sin(num_angle_rad) + y * math_cos(num_angle_rad);
        
        int vertex_x = num_x + (int)(rotated_x * solid_radius);
        int vertex_y = num_y + (int)(rotated_y * solid_radius);
        
        // Draw vertex as small circle
        for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
                if (dx*dx + dy*dy <= 4) {
                    SDL_RenderDrawPoint(renderer, vertex_x + dx, vertex_y + dy);
                }
            }
        }
    }
    
    // Draw edges
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 120);
    for (int i = 0; i < solid.num_edges; i++) {
        int v1 = solid.edges[i * 2 + 0];
        int v2 = solid.edges[i * 2 + 1];
        
        // Get vertex positions
        double x1 = solid.vertices[v1 * 3 + 0];
        double y1 = solid.vertices[v1 * 3 + 1];
        double x2 = solid.vertices[v2 * 3 + 0];
        double y2 = solid.vertices[v2 * 3 + 1];
        
        // Rotate both vertices
        double rx1 = x1 * math_cos(num_angle_rad) - y1 * math_sin(num_angle_rad);
        double ry1 = x1 * math_sin(num_angle_rad) + y1 * math_cos(num_angle_rad);
        double rx2 = x2 * math_cos(num_angle_rad) - y2 * math_sin(num_angle_rad);
        double ry2 = x2 * math_sin(num_angle_rad) + y2 * math_cos(num_angle_rad);
        
        int vx1 = num_x + (int)(rx1 * solid_radius);
        int vy1 = num_y + (int)(ry1 * solid_radius);
        int vx2 = num_x + (int)(rx2 * solid_radius);
        int vy2 = num_y + (int)(ry2 * solid_radius);
        
        SDL_RenderDrawLine(renderer, vx1, vy1, vx2, vy2);
    }
}

// Draw sphere hierarchy visualization
void draw_sphere_hierarchy(SDL_Renderer* renderer, AppState* state,
                          int number, int cx, int cy, double scale) {
    if (!state->clock_ctx || number <= 0) return;
    
    // Create sphere hierarchy for the number
    CompactSphere* root = create_sphere_hierarchy(number, 3, state->clock_ctx);
    if (!root) return;
    
    // Get number position
    double num_angle_deg = get_precise_angle(number, state->clock_ctx);
    double num_angle_rad = num_angle_deg * MATH_PI / 180.0;
    double num_radius = get_radial_distance(number, state->zoom) * scale;
    int num_x = cx + (int)(num_radius * math_cos(num_angle_rad));
    int num_y = cy + (int)(num_radius * math_sin(num_angle_rad));
    
    // Draw root sphere
    double root_radius = 30.0;
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 180);
    
    // Draw circle for root
    for (int angle = 0; angle < 360; angle += 5) {
        double rad = angle * MATH_PI / 180.0;
        int x = num_x + (int)(root_radius * math_cos(rad));
        int y = num_y + (int)(root_radius * math_sin(rad));
        SDL_RenderDrawPoint(renderer, x, y);
    }
    
    // Draw child spheres (12-fold symmetry)
    if (root->num_children > 0) {
        double child_radius = root_radius / 3.0;
        SDL_SetRenderDrawColor(renderer, 0, 200, 255, 150);
        
        for (int i = 0; i < root->num_children && i < 12; i++) {
            // Position child at 30-degree intervals
            double child_angle = (i * 30.0 + num_angle_deg) * MATH_PI / 180.0;
            int child_x = num_x + (int)(root_radius * 0.7 * math_cos(child_angle));
            int child_y = num_y + (int)(root_radius * 0.7 * math_sin(child_angle));
            
            // Draw child circle
            for (int angle = 0; angle < 360; angle += 10) {
                double rad = angle * MATH_PI / 180.0;
                int x = child_x + (int)(child_radius * math_cos(rad));
                int y = child_y + (int)(child_radius * math_sin(rad));
                SDL_RenderDrawPoint(renderer, x, y);
            }
            
            // Draw line from root to child
            SDL_SetRenderDrawColor(renderer, 0, 180, 255, 100);
            SDL_RenderDrawLine(renderer, num_x, num_y, child_x, child_y);
        }
    }
    
    // Clean up
    free_sphere_hierarchy(root);
}

// Draw all enhanced visualizations
void draw_all_enhanced_visualizations(SDL_Renderer* renderer, AppState* state,
                                     int cx, int cy, double scale) {
    // Draw enhanced factor lines
    draw_enhanced_factor_lines(renderer, state, cx, cy, scale);
    
    // Draw Platonic solid overlays for selected numbers
    if (state->analysis_history.selected_index >= 0) {
        int num = state->analysis_history.numbers[state->analysis_history.selected_index];
        draw_platonic_solid_overlay(renderer, state, num, cx, cy, scale);
        draw_sphere_hierarchy(renderer, state, num, cx, cy, scale);
    }
}