/**
 * Sphere Visualization Module
 * 
 * Displays the 12 kissing spheres in a circular arrangement
 * with real-time activity indicators, batch counts, and loss values.
 * 
 * PHASE 2: Consolidated constants
 * - Removed local M_PI definition
 * - Using MATH_PI from math/types.h (via prime_float_math.h)
 */

#include "../app_common.h"
#include "../../include/prime_float_math.h"
#include <stdio.h>

// PHASE 3, DAY 11: Crystalline Memory Visualization Integration
#include "crystalline_visualization.h"

// Global crystalline visualization state
static CrystallineVizState g_crystalline_viz_state = {0};
static bool g_crystalline_viz_initialized = false;

/**
 * Draw a filled circle (sphere representation)
 */
static void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

/**
 * Draw a circle outline
 */
static void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

/**
 * Get color based on activity level (0.0 to 1.0)
 * Note: Currently unused but kept for future activity-based coloring
 */
__attribute__((unused))
static SDL_Color get_activity_color(float activity) {
    SDL_Color color;
    
    if (activity < 0.01f) {
        // Inactive - dark gray
        color = (SDL_Color){40, 40, 50, 255};
    } else if (activity < 0.3f) {
        // Low activity - blue
        color = (SDL_Color){60, 100, 180, 255};
    } else if (activity < 0.6f) {
        // Medium activity - cyan
        color = (SDL_Color){80, 180, 200, 255};
    } else if (activity < 0.9f) {
        // High activity - yellow
        color = (SDL_Color){220, 200, 80, 255};
    } else {
        // Very high activity - orange/red
        color = (SDL_Color){255, 140, 60, 255};
    }
    
    return color;
}

/**
 * PHASE 6: Get color based on entropy level (0.0 to 1.0)
 * 
 * Maps entropy values to a color spectrum:
 * - Low entropy (0.0-0.3): Blue (ordered, low complexity)
 * - Medium entropy (0.3-0.6): Cyan/Green (moderate complexity)
 * - High entropy (0.6-0.9): Yellow/Orange (high complexity)
 * - Very high entropy (0.9-1.0): Red (maximum complexity)
 */
static SDL_Color get_entropy_color(float normalized_entropy) {
    SDL_Color color;
    
    if (normalized_entropy < 0.3f) {
        // Low entropy - blue spectrum (ordered)
        int intensity = 80 + (int)(normalized_entropy * 200.0f);
        color = (SDL_Color){40, 60, intensity, 255};
    } else if (normalized_entropy < 0.6f) {
        // Medium entropy - cyan/green spectrum
        float t = (normalized_entropy - 0.3f) / 0.3f;
        int r = 40 + (int)(t * 80.0f);
        int g = 150 + (int)(t * 80.0f);
        int b = 200 - (int)(t * 100.0f);
        color = (SDL_Color){r, g, b, 255};
    } else if (normalized_entropy < 0.9f) {
        // High entropy - yellow/orange spectrum
        float t = (normalized_entropy - 0.6f) / 0.3f;
        int r = 120 + (int)(t * 135.0f);
        int g = 230 - (int)(t * 90.0f);
        int b = 100 - (int)(t * 100.0f);
        color = (SDL_Color){r, g, b, 255};
    } else {
        // Very high entropy - red spectrum (maximum complexity)
        int intensity = 200 + (int)((normalized_entropy - 0.9f) * 550.0f);
        if (intensity > 255) intensity = 255;
        color = (SDL_Color){intensity, 60, 40, 255};
    }
    
    return color;
}

/**
 * Get color based on thread state (UI Integration)
 */
// TODO: Use this when implementing thread state visualization in Phase 4.2
static SDL_Color get_thread_state_color(int state) __attribute__((unused));
static SDL_Color get_thread_state_color(int state) {
    switch (state) {
        case 0: // THREAD_STATE_IDLE
            return (SDL_Color){40, 40, 50, 255};  // Dark gray
        case 1: // THREAD_STATE_WORKING
            return (SDL_Color){100, 200, 100, 255};  // Green
        case 2: // THREAD_STATE_CONTROL
            return (SDL_Color){200, 150, 50, 255};  // Gold
        case 3: // THREAD_STATE_WAITING
            return (SDL_Color){100, 150, 200, 255};  // Blue
        case 4: // THREAD_STATE_TERMINATED
            return (SDL_Color){200, 50, 50, 255};  // Red
        default:
            return (SDL_Color){60, 60, 70, 255};  // Default gray
    }
}

/**
 * Draw the 12 kissing spheres in a circular arrangement
 */

/**
 * COMPLETE 3D KISSING SPHERES VISUALIZATION
 * 
 * This is the CORRECT implementation based on deep mathematical analysis:
 * - 13 spheres: 1 control + 12 workers
 * - Kissing spheres (3D, not 2D circles)
 * - Nested structure (Metatron's Cube style)
 * - Shared memory (workers read from control sphere's abacus)
 * - Icosahedral placement (golden ratio vertices)
 * - Recursive capability (workers can spawn children)
 * - Hyper-dimensional with quadratic mirroring
 * 
 * Features:
 * - Mouse drag rotation
 * - Keyboard controls (arrow keys)
 * - Auto-rotation
 * - Activity visualization (brightness, pulsing, wireframe density)
 * - Recursive children visualization
 */

// 3D vector operations
typedef struct {
    float x, y, z;
} Vec3;

static Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static Vec3 vec3_sub(Vec3 a, Vec3 b) __attribute__((unused));
static Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static Vec3 vec3_scale(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

static float vec3_length(Vec3 v) {
    return math_sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len < 0.0001f) return (Vec3){0, 0, 0};
    return vec3_scale(v, 1.0f / len);
}

static Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// Rotate vector around X axis
static Vec3 rotate_x(Vec3 v, float angle) {
    float c = math_cos(angle);
    float s = math_sin(angle);
    return (Vec3){
        v.x,
        v.y * c - v.z * s,
        v.y * s + v.z * c
    };
}

// Rotate vector around Y axis
static Vec3 rotate_y(Vec3 v, float angle) {
    float c = math_cos(angle);
    float s = math_sin(angle);
    return (Vec3){
        v.x * c + v.z * s,
        v.y,
        -v.x * s + v.z * c
    };
}

// Rotate vector around Z axis
static Vec3 rotate_z(Vec3 v, float angle) __attribute__((unused));
static Vec3 rotate_z(Vec3 v, float angle) {
    float c = math_cos(angle);
    float s = math_sin(angle);
    return (Vec3){
        v.x * c - v.y * s,
        v.x * s + v.y * c,
        v.z
    };
}

// Project 3D point to 2D screen with perspective
static void project_3d(Vec3 p, float rot_x, float rot_y, int center_x, int center_y, 
                int* screen_x, int* screen_y, float* depth) {
    // Apply rotations
    Vec3 rotated = rotate_x(p, rot_x);
    rotated = rotate_y(rotated, rot_y);
    
    // Perspective projection
    float camera_dist = 2.5f;
    float perspective = camera_dist / (camera_dist + rotated.z / 100.0f);
    
    *screen_x = center_x + (int)(rotated.x * perspective);
    *screen_y = center_y - (int)(rotated.y * perspective);
    *depth = rotated.z;
}

// Draw a 3D circle (for clock faces)
static void draw_3d_circle(SDL_Renderer* renderer, Vec3 center, float radius, 
                    Vec3 normal, float rot_x, float rot_y, 
                    int screen_cx, int screen_cy, SDL_Color color, int num_segments) {
    // Find two perpendicular vectors in the plane
    Vec3 u, v;
    if (math_abs(normal.z) < 0.9f) {
        u = vec3_normalize((Vec3){-normal.y, normal.x, 0});
    } else {
        u = vec3_normalize((Vec3){0, -normal.z, normal.y});
    }
    v = vec3_cross(normal, u);
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    for (int i = 0; i < num_segments; i++) {
        float angle1 = (i * 2.0f * MATH_PI) / num_segments;
        float angle2 = ((i + 1) * 2.0f * MATH_PI) / num_segments;
        
        Vec3 p1 = vec3_add(center, vec3_add(
            vec3_scale(u, math_cos(angle1) * radius),
            vec3_scale(v, math_sin(angle1) * radius)
        ));
        
        Vec3 p2 = vec3_add(center, vec3_add(
            vec3_scale(u, math_cos(angle2) * radius),
            vec3_scale(v, math_sin(angle2) * radius)
        ));
        
        int x1, y1, x2, y2;
        float d1, d2;
        project_3d(p1, rot_x, rot_y, screen_cx, screen_cy, &x1, &y1, &d1);
        project_3d(p2, rot_x, rot_y, screen_cx, screen_cy, &x2, &y2, &d2);
        
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

// Draw clock tick marks (12 positions)
static void draw_clock_ticks(SDL_Renderer* renderer, Vec3 center, float radius,
                      Vec3 normal, float rot_x, float rot_y,
                      int screen_cx, int screen_cy, SDL_Color color) {
    // Find perpendicular vectors
    Vec3 u, v;
    if (math_abs(normal.z) < 0.9f) {
        u = vec3_normalize((Vec3){-normal.y, normal.x, 0});
    } else {
        u = vec3_normalize((Vec3){0, -normal.z, normal.y});
    }
    v = vec3_cross(normal, u);
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Draw 12 tick marks
    for (int i = 0; i < 12; i++) {
        float angle = (i * 2.0f * MATH_PI) / 12.0f;
        
        Vec3 tick_pos = vec3_add(center, vec3_add(
            vec3_scale(u, math_cos(angle) * radius),
            vec3_scale(v, math_sin(angle) * radius)
        ));
        
        int tx, ty;
        float td;
        project_3d(tick_pos, rot_x, rot_y, screen_cx, screen_cy, &tx, &ty, &td);
        
        // Draw tick mark (small filled circle)
        for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
                if (dx*dx + dy*dy <= 4) {
                    SDL_RenderDrawPoint(renderer, tx + dx, ty + dy);
                }
            }
        }
    }
}

// Draw a 3D sphere (wireframe with variable density)
static void draw_3d_sphere_wireframe(SDL_Renderer* renderer, Vec3 center, float radius,
                               float rot_x, float rot_y, int screen_cx, int screen_cy,
                               SDL_Color color, int alpha, int density) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
    
    int lat_lines = 3 + density;  // More lines = denser wireframe
    int lon_lines = 6 + density * 2;
    
    // Draw latitude circles
    for (int lat = -lat_lines; lat <= lat_lines; lat++) {
        float y = (lat / (float)lat_lines) * radius;
        float r = math_sqrt(radius * radius - y * y);
        if (r < 0.1f) continue;
        
        const int num_points = 24 + density * 12;
        for (int i = 0; i < num_points; i++) {
            float angle1 = (i * 2.0f * MATH_PI) / num_points;
            float angle2 = ((i + 1) * 2.0f * MATH_PI) / num_points;
            
            Vec3 p1 = vec3_add(center, (Vec3){
                math_cos(angle1) * r, y, math_sin(angle1) * r
            });
            Vec3 p2 = vec3_add(center, (Vec3){
                math_cos(angle2) * r, y, math_sin(angle2) * r
            });
            
            int x1, y1, x2, y2;
            float d1, d2;
            project_3d(p1, rot_x, rot_y, screen_cx, screen_cy, &x1, &y1, &d1);
            project_3d(p2, rot_x, rot_y, screen_cx, screen_cy, &x2, &y2, &d2);
            
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
    
    // Draw longitude circles
    for (int lon = 0; lon < lon_lines; lon++) {
        float angle_offset = (lon * MATH_PI) / lon_lines;
        
        const int num_points = 24 + density * 12;
        for (int i = 0; i < num_points; i++) {
            float angle1 = (i * 2.0f * MATH_PI) / num_points;
            float angle2 = ((i + 1) * 2.0f * MATH_PI) / num_points;
            
            Vec3 p1 = vec3_add(center, (Vec3){
                math_cos(angle_offset) * math_sin(angle1) * radius,
                math_cos(angle1) * radius,
                math_sin(angle_offset) * math_sin(angle1) * radius
            });
            Vec3 p2 = vec3_add(center, (Vec3){
                math_cos(angle_offset) * math_sin(angle2) * radius,
                math_cos(angle2) * radius,
                math_sin(angle_offset) * math_sin(angle2) * radius
            });
            
            int x1, y1, x2, y2;
            float d1, d2;
            project_3d(p1, rot_x, rot_y, screen_cx, screen_cy, &x1, &y1, &d1);
            project_3d(p2, rot_x, rot_y, screen_cx, screen_cy, &x2, &y2, &d2);
            
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}

/**
 * Draw spheres in 3D mode - COMPLETE CORRECT IMPLEMENTATION
 * 
 * This implements the TRUE 3D kissing spheres visualization with:
 * - 13 spheres (1 control + 12 workers)
 * - Icosahedral placement
 * - Concentric rings (Babylonian: 12, 60, 60, 100)
 * - 3 clock faces embedded on control sphere
 * - 12 radial lines
 * - Kissing points highlighted
 * - Activity-based visualization
 * - Recursive children support
 */
static void draw_spheres_3d(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds __attribute__((unused)),
                           int center_x, int center_y, int arrangement_radius, 
                           int sphere_radius, int max_batches) {
    (void)sphere_radius;  // We calculate our own sizes
    
    // Golden ratio for icosahedron
    const float phi = (1.0f + math_sqrt(5.0f)) / 2.0f;
    
    // 12 icosahedron vertices (normalized)
    Vec3 vertices[12] = {
        {0, 1, phi},   {0, -1, phi},   {0, 1, -phi},  {0, -1, -phi},
        {1, phi, 0},   {-1, phi, 0},   {1, -phi, 0},  {-1, -phi, 0},
        {phi, 0, 1},   {-phi, 0, 1},   {phi, 0, -1},  {-phi, 0, -1}
    };
    
    // Normalize vertices
    for (int i = 0; i < 12; i++) {
        vertices[i] = vec3_normalize(vertices[i]);
    }
    
    // Get rotation from state (with auto-rotation)
    static float auto_rot = 0.0f;
    auto_rot += 0.005f;  // Slow auto-rotation
    
    float rot_x = (float)state->rotation_x + 0.5f;
    float rot_y = (float)state->rotation_y + auto_rot;
    
    // Sphere sizes
    float control_radius = arrangement_radius * 0.5f;
    float worker_radius = arrangement_radius * 0.12f;
    
    Vec3 control_center = {0, 0, 0};
    
    // === STEP 1: Draw concentric rings (Babylonian: 12, 60, 60, 100) ===
    float ring_radii[4] = {0.25f, 0.5f, 0.75f, 1.0f};
    SDL_Color ring_colors[4] = {
        {100, 150, 255, 80},  // Inner - blue
        {150, 200, 255, 60},
        {200, 220, 255, 40},
        {220, 230, 255, 20}   // Outer - light blue
    };
    
    for (int ring = 0; ring < 4; ring++) {
        float r = control_radius * ring_radii[ring];
        draw_3d_sphere_wireframe(renderer, control_center, r, rot_x, rot_y,
                                  center_x, center_y, ring_colors[ring], 60, 0);
    }
    
    // === STEP 2: Draw control sphere (semi-transparent gold) ===
    SDL_Color control_color = {200, 180, 100, 255};
    draw_3d_sphere_wireframe(renderer, control_center, control_radius, rot_x, rot_y,
                              center_x, center_y, control_color, 100, 1);
    
    // === STEP 3: Draw 3 clock faces embedded on control sphere ===
    float clock_r = control_radius * 0.95f;
    
    // XY plane (horizontal)
    draw_3d_circle(renderer, control_center, clock_r, (Vec3){0, 0, 1},
                   rot_x, rot_y, center_x, center_y, (SDL_Color){80, 120, 160, 255}, 72);
    draw_clock_ticks(renderer, control_center, clock_r, (Vec3){0, 0, 1},
                     rot_x, rot_y, center_x, center_y, (SDL_Color){120, 160, 200, 255});
    
    // XZ plane (vertical front)
    draw_3d_circle(renderer, control_center, clock_r, (Vec3){0, 1, 0},
                   rot_x, rot_y, center_x, center_y, (SDL_Color){80, 120, 160, 255}, 72);
    draw_clock_ticks(renderer, control_center, clock_r, (Vec3){0, 1, 0},
                     rot_x, rot_y, center_x, center_y, (SDL_Color){120, 160, 200, 255});
    
    // YZ plane (vertical side)
    draw_3d_circle(renderer, control_center, clock_r, (Vec3){1, 0, 0},
                   rot_x, rot_y, center_x, center_y, (SDL_Color){80, 120, 160, 255}, 72);
    draw_clock_ticks(renderer, control_center, clock_r, (Vec3){1, 0, 0},
                     rot_x, rot_y, center_x, center_y, (SDL_Color){120, 160, 200, 255});
    
    // === STEP 4: Draw 12 radial lines (color-coded by symmetry group) ===
    SDL_Color radial_colors[12] = {
        {255, 100, 100, 255}, {100, 255, 100, 255}, {100, 100, 255, 255},
        {255, 255, 100, 255}, {255, 100, 255, 255}, {100, 255, 255, 255},
        {255, 150, 100, 255}, {150, 255, 100, 255}, {100, 150, 255, 255},
        {255, 100, 150, 255}, {100, 255, 150, 255}, {150, 100, 255, 255}
    };
    
    for (int i = 0; i < 12; i++) {
        Vec3 endpoint = vec3_scale(vertices[i], control_radius + worker_radius + 5.0f);
        
        int x1, y1, x2, y2;
        float d1, d2;
        project_3d(control_center, rot_x, rot_y, center_x, center_y, &x1, &y1, &d1);
        project_3d(endpoint, rot_x, rot_y, center_x, center_y, &x2, &y2, &d2);
        
        SDL_SetRenderDrawColor(renderer, radial_colors[i].r, radial_colors[i].g, 
                               radial_colors[i].b, 128);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    
    // === STEP 5: Draw 12 worker spheres with activity visualization ===
    static float pulse_phase = 0.0f;
    pulse_phase += 0.1f;
    
    for (int i = 0; i < 12; i++) {
        Vec3 worker_pos = vec3_scale(vertices[i], control_radius);
        
        // Calculate activity level
        float activity = 0.0f;
        if (max_batches > 0 && i < 12) {
            activity = (float)state->sphere_stats.batches_processed[i] / (float)max_batches;
        }
        
        // Activity affects brightness and wireframe density
        SDL_Color worker_color = radial_colors[i];
        float brightness = 0.5f + activity * 0.5f;  // 50-100% brightness
        worker_color.r = (Uint8)(worker_color.r * brightness);
        worker_color.g = (Uint8)(worker_color.g * brightness);
        worker_color.b = (Uint8)(worker_color.b * brightness);
        
        // Pulsing effect for active workers
        float pulse = 1.0f;
        if (activity > 0.1f) {
            pulse = 1.0f + 0.1f * math_sin(pulse_phase + i * 0.5f);
        }
        
        // Wireframe density based on activity
        int density = (int)(activity * 3.0f);  // 0-3
        
        // Draw worker sphere
        draw_3d_sphere_wireframe(renderer, worker_pos, worker_radius * pulse, 
                                  rot_x, rot_y, center_x, center_y, 
                                  worker_color, 255, density);
        
        // === STEP 6: Draw kissing point (bright highlight) ===
        int kx, ky;
        float kd;
        project_3d(worker_pos, rot_x, rot_y, center_x, center_y, &kx, &ky, &kd);
        
        // Bright pulsing highlight at kissing point
        int highlight_size = 3 + (int)(activity * 2.0f);
        Uint8 highlight_alpha = (Uint8)(200 + 55 * math_sin(pulse_phase + i * 0.5f));
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, highlight_alpha);
        for (int dx = -highlight_size; dx <= highlight_size; dx++) {
            for (int dy = -highlight_size; dy <= highlight_size; dy++) {
                if (dx*dx + dy*dy <= highlight_size * highlight_size) {
                    SDL_RenderDrawPoint(renderer, kx + dx, ky + dy);
                }
            }
        }
    }
}

/**
 * PHASE 5: Draw spheres in 2D arrangement showing full hierarchy
 * 
 * Layout:
 * - Center: Root sphere (small, gray, "CTRL")
 * - Circle: 12 level-1 spheres (always visible, even if inactive)
 * - Around parents: Level-2 children (smaller, connected)
 * 
 * Colors:
 * - Root: Gray
 * - Control threads: Purple gradient
 * - Worker threads: Activity gradient (green -> yellow -> red)
 * - Inactive: Dark gray outline
 */
static void draw_spheres_2d(SDL_Renderer* renderer, AppState* state,
                           int center_x, int center_y, int arrangement_radius, 
                           int sphere_radius, int max_batches) {
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color control_color = {150, 100, 200, 255};  // Purple for control threads
    SDL_Color ghost_color = {40, 40, 50, 255};       // Gray for inactive positions
    
    // ========================================================================
    // PART 1: Draw root sphere (sphere 0) at center
    // ========================================================================
    
    int root_radius = sphere_radius / 2;  // Half size for root
    
    // Check if root is active
    int root_active = (state->sphere_stats.hierarchy_level[0] >= 0);
    
    if (root_active) {
        // Root is always control thread
        SDL_Color root_color = {100, 100, 120, 255};  // Gray
        
        // Draw filled circle
        draw_filled_circle(renderer, center_x, center_y, root_radius, root_color);
        
        // Draw border
        draw_circle_outline(renderer, center_x, center_y, root_radius, text_color);
        
        // Draw "CTRL" label
        draw_text(renderer, "CTRL", center_x - 12, center_y + root_radius + 5, text_color);
        
        // Draw sphere ID
        draw_text(renderer, "0", center_x - 3, center_y - 4, text_color);
    }
    
    // ========================================================================
    // PART 2: Draw 12 level-1 spheres in circle (ALWAYS 12 positions)
    // ========================================================================
    
    for (int i = 0; i < 12; i++) {
        int sphere_id = i + 1;  // Spheres 1-12
        
        // Calculate position using prime_* functions (NO math.h!)
        float angle = (i * 2.0f * 3.14159265359f) / 12.0f - 3.14159265359f / 2.0f;  // Start at top
        int x = center_x + (int)(arrangement_radius * math_cos(angle));
        int y = center_y + (int)(arrangement_radius * math_sin(angle));
        
        // Check if this sphere is active
        int is_active = (state->sphere_stats.hierarchy_level[sphere_id] >= 0);
        
        if (is_active) {
            // Active sphere - determine color based on role
            SDL_Color sphere_color;
            
            if (state->sphere_stats.is_control[sphere_id]) {
                // Control thread - purple gradient based on children count
                int num_children = state->sphere_stats.num_children[sphere_id];
                int intensity = 100 + (num_children * 12);  // More children = brighter
                if (intensity > 255) intensity = 255;
                sphere_color = (SDL_Color){intensity, 50, 200, 255};
            } else {
                // Worker thread - PHASE 6: Use entropy-based coloring if available
                // Get entropy data from training system
                extern void* get_training_system(void);
                void* training_system = get_training_system();
                
                bool use_entropy_color = false;
                float entropy_value = 0.0f;
                
                if (training_system) {
                    extern void* threaded_training_get_entropy_context(void* system);
                    void* entropy_ctx = threaded_training_get_entropy_context(training_system);
                    
                    if (entropy_ctx) {
                        // Get dimension entropy for this sphere's symmetry group
                        int dimension = sphere_id % 12;  // Map sphere to dimension (0-11)
                        
                        extern const void* get_dimension_stats(const void* ctx, unsigned int dimension);
                        const void* dim_stats = get_dimension_stats(entropy_ctx, (unsigned int)dimension);
                        
                        if (dim_stats) {
                            // Cast to access the structure
                            typedef struct {
                                unsigned int dimension;
                                double current_entropy;
                                double min_entropy;
                                double max_entropy;
                                double avg_entropy;
                                uint64_t sample_count;
                                double entropy_variance;
                            } DimensionEntropyStats;
                            
                            const DimensionEntropyStats* stats = (const DimensionEntropyStats*)dim_stats;
                            
                            // Normalize entropy to 0-1 range
                            if (stats->max_entropy > stats->min_entropy) {
                                entropy_value = (float)((stats->current_entropy - stats->min_entropy) / 
                                                       (stats->max_entropy - stats->min_entropy));
                            } else {
                                entropy_value = 0.5f;  // Default to medium if no range
                            }
                            
                            use_entropy_color = true;
                        }
                    }
                }
                
                if (use_entropy_color) {
                    // Use entropy-based coloring
                    sphere_color = get_entropy_color(entropy_value);
                } else {
                    // Fallback to activity gradient based on batch count
                    float activity = 0.0f;
                    if (max_batches > 0) {
                        activity = (float)state->sphere_stats.batches_processed[sphere_id] / (float)max_batches;
                    }
                    if (activity > 1.0f) activity = 1.0f;
                    
                    // Green -> Yellow -> Red gradient
                    int r = (int)(activity * 255);
                    int g = (int)((1.0f - activity) * 255);
                    sphere_color = (SDL_Color){r, g, 0, 255};
                }
            }
            
            // Draw filled circle
            draw_filled_circle(renderer, x, y, sphere_radius, sphere_color);
            
            // Draw border
            draw_circle_outline(renderer, x, y, sphere_radius, text_color);
            
            // Draw connection to root
            SDL_SetRenderDrawColor(renderer, 50, 50, 60, 128);
            SDL_RenderDrawLine(renderer, center_x, center_y, x, y);
            
            // Draw sphere ID
            char id_str[8];
            snprintf(id_str, sizeof(id_str), "%d", sphere_id);
            draw_text(renderer, id_str, x - 4, y - 4, text_color);
            
            // Draw role indicator
            if (state->sphere_stats.is_control[sphere_id]) {
                draw_text(renderer, "C", x - 3, y + sphere_radius + 5, control_color);
            }
            
            // Draw batch count (outside circle)
            if (!state->sphere_stats.is_control[sphere_id] && state->sphere_stats.batches_processed[sphere_id] > 0) {
                char batch_str[16];
                snprintf(batch_str, sizeof(batch_str), "%d", 
                        state->sphere_stats.batches_processed[sphere_id]);
                int text_x = x + (int)((sphere_radius + 15) * math_cos(angle)) - 10;
                int text_y = y + (int)((sphere_radius + 15) * math_sin(angle)) - 6;
                draw_text(renderer, batch_str, text_x, text_y, (SDL_Color){180, 180, 180, 255});
            }
            
        } else {
            // Inactive position - draw ghost outline
            draw_circle_outline(renderer, x, y, sphere_radius, ghost_color);
            
            // Draw symmetry group number
            char group_str[8];
            snprintf(group_str, sizeof(group_str), "%d", i);
            draw_text(renderer, group_str, x - 4, y - 4, ghost_color);
        }
    }
    
    // ========================================================================
    // PART 3: Draw level-2 children around their parents
    // ========================================================================
    
    int child_radius = sphere_radius / 2;  // Half size for children
    int child_offset = sphere_radius + 20;  // Distance from parent center
    
    // Iterate through all possible spheres (13-143)
    for (int sphere_id = 13; sphere_id < 144; sphere_id++) {
        // Check if this sphere is active
        if (state->sphere_stats.hierarchy_level[sphere_id] < 0) continue;
        
        // Get parent ID
        int parent_id = state->sphere_stats.parent_id[sphere_id];
        if (parent_id < 1 || parent_id > 12) continue;  // Must be level-1 parent
        
        // Calculate parent position
        int parent_index = parent_id - 1;  // 0-11
        float parent_angle = (parent_index * 2.0f * 3.14159265359f) / 12.0f - 3.14159265359f / 2.0f;
        int parent_x = center_x + (int)(arrangement_radius * math_cos(parent_angle));
        int parent_y = center_y + (int)(arrangement_radius * math_sin(parent_angle));
        
        // Calculate child position around parent
        // Distribute children evenly around parent
        int num_siblings = state->sphere_stats.num_children[parent_id];
        int child_index = 0;
        
        // Find this child's index among siblings
        for (int s = 13; s < sphere_id; s++) {
            if (state->sphere_stats.parent_id[s] == parent_id) {
                child_index++;
            }
        }
        
        // Position child around parent
        float child_angle = parent_angle + (2.0f * 3.14159265359f * child_index) / (float)num_siblings;
        int child_x = parent_x + (int)(child_offset * math_cos(child_angle));
        int child_y = parent_y + (int)(child_offset * math_sin(child_angle));
        
        // Determine color (children are always workers initially)
        float activity = 0.0f;
        if (max_batches > 0) {
            activity = (float)state->sphere_stats.batches_processed[sphere_id] / (float)max_batches;
        }
        if (activity > 1.0f) activity = 1.0f;
        
        int r = (int)(activity * 255);
        int g = (int)((1.0f - activity) * 255);
        SDL_Color child_color = {r, g, 0, 255};
        
        // Draw filled circle
        draw_filled_circle(renderer, child_x, child_y, child_radius, child_color);
        
        // Draw border
        draw_circle_outline(renderer, child_x, child_y, child_radius, text_color);
        
        // Draw connection to parent
        SDL_SetRenderDrawColor(renderer, 50, 50, 60, 128);
        SDL_RenderDrawLine(renderer, parent_x, parent_y, child_x, child_y);
        
        // Draw sphere ID
        char id_str[8];
        snprintf(id_str, sizeof(id_str), "%d", sphere_id);
        draw_text(renderer, id_str, child_x - 4, child_y - 3, text_color);
        
        // Draw batch count
        if (state->sphere_stats.batches_processed[sphere_id] > 0) {
            char batch_str[16];
            snprintf(batch_str, sizeof(batch_str), "%d", state->sphere_stats.batches_processed[sphere_id]);
            draw_text(renderer, batch_str, child_x - 6, child_y + child_radius + 5, (SDL_Color){180, 180, 180, 255});
        }
    }
}

void draw_sphere_visualization(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds) {
    if (!state) return;
       
       // PHASE 3, DAY 11: Initialize crystalline visualization on first call
       if (!g_crystalline_viz_initialized) {
           crystalline_viz_init(&g_crystalline_viz_state);
           g_crystalline_viz_initialized = true;
       }
       
       // Update crystalline visualization state from training system
       crystalline_viz_update(&g_crystalline_viz_state, state);
    
    SDL_Color bg_color = {25, 25, 30, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color grid_color = {50, 50, 60, 255};
    SDL_Color center_color = {100, 100, 120, 255};
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &bounds);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawRect(renderer, &bounds);
    
    // Calculate center and radius for sphere arrangement
    // Use smaller dimension and leave margin for spheres at edges
    int center_x = bounds.x + bounds.w / 2;
    int center_y = bounds.y + bounds.h / 2;
    int min_dimension = (bounds.w < bounds.h ? bounds.w : bounds.h);
    int arrangement_radius = (min_dimension - 100) / 3;  // Leave 100px margin
    int sphere_radius = arrangement_radius / 5;
    
    // Ensure spheres fit within bounds
    if (arrangement_radius + sphere_radius * 2 > min_dimension / 2) {
        arrangement_radius = (min_dimension / 2) - sphere_radius * 2 - 20;
    }
    
    // Draw title
    draw_text(renderer, "KISSING SPHERES ARCHITECTURE", bounds.x + 10, bounds.y + 10, text_color);
    
    // Lock sphere_stats for reading
    pthread_mutex_lock(&state->sphere_stats_mutex);
    
    // UI Integration: Draw subtitle with real-time status
    if (state->training_metrics) {
        char status_text[128];
        snprintf(status_text, sizeof(status_text), "Real-time Metrics | Epoch %d | Loss: %.4f",
                state->training_current_epoch, state->training_loss);
        draw_text(renderer, status_text, bounds.x + 10, bounds.y + 30, 
                 (SDL_Color){150, 150, 150, 255});
    }
    
    // Draw center sphere (Node Zero - Control Thread)
    draw_filled_circle(renderer, center_x, center_y, sphere_radius / 2, center_color);
    draw_circle_outline(renderer, center_x, center_y, sphere_radius / 2, text_color);
    draw_text(renderer, "0", center_x - 4, center_y - 6, text_color);
    
    // Find max batch count for normalization
    int max_batches = 1;
    for (int i = 0; i < 12; i++) {
        if (state->sphere_stats.batches_processed[i] > max_batches) {
            max_batches = state->sphere_stats.batches_processed[i];
        }
    }
    
    // Draw spheres based on visualization mode
    if (state->sphere_viz_mode == SPHERE_VIZ_3D) {
        draw_spheres_3d(renderer, state, bounds, center_x, center_y, 
                       arrangement_radius, sphere_radius, max_batches);
    } else if (state->sphere_viz_mode == SPHERE_VIZ_CRYSTALLINE) {
        // Draw crystalline geometric patterns and memory structure
        draw_crystalline_visualization(renderer, &g_crystalline_viz_state, bounds);
        return;  // Crystalline viz has its own statistics display
    } else {
        draw_spheres_2d(renderer, state, center_x, center_y, 
                       arrangement_radius, sphere_radius, max_batches);
    }
    
    // Draw statistics panel at bottom (increased height for vertical legend)
    int stats_y = bounds.y + bounds.h - 110;
    SDL_Rect stats_panel = {bounds.x + 10, stats_y, bounds.w - 20, 100};
    SDL_SetRenderDrawColor(renderer, 35, 35, 40, 255);
    SDL_RenderFillRect(renderer, &stats_panel);
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawRect(renderer, &stats_panel);
    
    // Display statistics
    char stats_text[256];
    int text_y = stats_panel.y + 8;
    
    snprintf(stats_text, sizeof(stats_text), "Active Spheres: %d / 12", 
             state->sphere_stats.active_spheres);
    draw_text(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    text_y += 18;
    
    snprintf(stats_text, sizeof(stats_text), "Total Batches: %d", 
             state->sphere_stats.total_batches);
    draw_text(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    text_y += 18;
    
    snprintf(stats_text, sizeof(stats_text), "Gradient Norm: %.4f", 
             state->sphere_stats.total_gradient_norm);
    draw_text(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    
       // Unlock sphere_stats after reading all data
       pthread_mutex_unlock(&state->sphere_stats_mutex);

    // Draw legend - PHASE 6: Check if entropy coloring is active
    int legend_x = stats_panel.x + stats_panel.w - 200;
    int legend_y = stats_panel.y + 8;
    
    // Check if entropy data is available
    extern void* get_training_system(void);
    void* training_system = get_training_system();
    bool has_entropy = false;
    
    if (training_system) {
        extern void* threaded_training_get_entropy_context(void* system);
        void* entropy_ctx = threaded_training_get_entropy_context(training_system);
        if (entropy_ctx) {
            has_entropy = true;
        }
    }
    
    if (has_entropy) {
        // PHASE 6: Entropy-based color legend
        draw_text(renderer, "Entropy Level:", legend_x, legend_y, text_color);
        legend_y += 18;
        
        struct {
            const char* label;
            SDL_Color color;
        } entropy_legend[] = {
            {"Low (Ordered)", {40, 60, 180, 255}},
            {"Medium-Low", {80, 180, 150, 255}},
            {"Medium", {120, 230, 100, 255}},
            {"Medium-High", {200, 200, 60, 255}},
            {"High (Complex)", {255, 100, 40, 255}}
        };
        
        for (int i = 0; i < 5; i++) {
            int item_y = legend_y + (i * 16);
            draw_filled_circle(renderer, legend_x + 5, item_y + 5, 4, entropy_legend[i].color);
            draw_text(renderer, entropy_legend[i].label, legend_x + 15, item_y, 
                     (SDL_Color){180, 180, 180, 255});
        }
    } else {
        // Fallback to activity-based color legend
        draw_text(renderer, "Activity Level:", legend_x, legend_y, text_color);
        legend_y += 18;
        
        struct {
            const char* label;
            SDL_Color color;
        } legend_items[] = {
            {"Inactive", {40, 40, 50, 255}},
            {"Low", {60, 100, 180, 255}},
            {"Medium", {80, 180, 200, 255}},
            {"High", {220, 200, 80, 255}},
            {"Very High", {255, 140, 60, 255}}
        };
        
        for (int i = 0; i < 5; i++) {
            int item_y = legend_y + (i * 16);
            draw_filled_circle(renderer, legend_x + 5, item_y + 5, 4, legend_items[i].color);
            draw_text(renderer, legend_items[i].label, legend_x + 15, item_y, 
                     (SDL_Color){180, 180, 180, 255});
        }
    }
}

/**
 * Draw detailed per-sphere statistics table
 */
void draw_sphere_stats_table(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds) {
    if (!state) return;
    
    SDL_Color bg_color = {25, 25, 30, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color header_color = {100, 150, 200, 255};
    SDL_Color grid_color = {50, 50, 60, 255};
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &bounds);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawRect(renderer, &bounds);
    
    int x = bounds.x + 10;
    int y = bounds.y + 10;
    
    // Title
    draw_text(renderer, "PER-SPHERE STATISTICS", x, y, header_color);
    y += 25;
    
    // Table header
    draw_text(renderer, "Sphere", x, y, header_color);
    draw_text(renderer, "Batches", x + 80, y, header_color);
    draw_text(renderer, "Avg Loss", x + 160, y, header_color);
    draw_text(renderer, "Status", x + 250, y, header_color);
    y += 20;
    
    // Draw horizontal line
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawLine(renderer, x, y, x + bounds.w - 20, y);
    y += 5;
    
    // Draw data for each sphere
    for (int i = 0; i < 12; i++) {
        char sphere_num[8];
        snprintf(sphere_num, sizeof(sphere_num), "#%d", i);
        draw_text(renderer, sphere_num, x, y, text_color);
        
        char batches[16];
        snprintf(batches, sizeof(batches), "%d", state->sphere_stats.batches_processed[i]);
        draw_text(renderer, batches, x + 80, y, text_color);
        
        char loss[16];
        if (state->sphere_stats.batches_processed[i] > 0) {
            snprintf(loss, sizeof(loss), "%.4f", state->sphere_stats.avg_loss[i]);
        } else {
            snprintf(loss, sizeof(loss), "-");
        }
        draw_text(renderer, loss, x + 160, y, text_color);
        
        const char* status = state->sphere_stats.batches_processed[i] > 0 ? "Active" : "Idle";
        SDL_Color status_color = state->sphere_stats.batches_processed[i] > 0 ? 
            (SDL_Color){100, 255, 100, 255} : (SDL_Color){100, 100, 100, 255};
        draw_text(renderer, status, x + 250, y, status_color);
        
        y += 16;
        
        // Stop if we run out of space
        if (y > bounds.y + bounds.h - 20) break;
    }
}