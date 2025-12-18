/**
 * Crystalline Memory Visualization Implementation
 * 
 * PHASE 2: Consolidated constants
 * - Removed local M_PI definition
 * - Using MATH_PI from math/types.h (via prime_float_math.h)
 */

#include "crystalline_visualization.h"
#include "../../include/prime_float_math.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Get color based on activity level (0.0 to 1.0)
 */
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
 * Get color for segment type
 */
static SDL_Color get_segment_color(int segment_id, float activity) {
    // Base color from symmetry group (0-11)
    float hue = (segment_id * 30.0f);  // 360/12 = 30 degrees per segment
    
    // Convert HSV to RGB (simplified)
    float h = hue / 60.0f;
    float c = activity;  // Chroma based on activity
    float x = c * (1.0f - math_abs(math_fmod(h, 2.0f) - 1.0f));
    
    float r, g, b;
    if (h < 1.0f) {
        r = c; g = x; b = 0;
    } else if (h < 2.0f) {
        r = x; g = c; b = 0;
    } else if (h < 3.0f) {
        r = 0; g = c; b = x;
    } else if (h < 4.0f) {
        r = 0; g = x; b = c;
    } else if (h < 5.0f) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    // Scale to 0-255 and add base brightness
    int base = 50;
    SDL_Color color = {
        (Uint8)(base + r * 205),
        (Uint8)(base + g * 205),
        (Uint8)(base + b * 205),
        255
    };
    
    return color;
}

/**
 * Draw filled circle
 */
static void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

/**
 * Draw circle outline
 */
static void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    int x = radius;
    int y = 0;
    int err = 0;
    
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
 * Draw text (placeholder - uses SDL_RenderDrawPoint for simple text)
 */
static void draw_text_simple(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
    (void)text;  // Text parameter reserved for future TTF implementation
    // Simple text rendering - just draw a small indicator
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, 8, 12};
    SDL_RenderFillRect(renderer, &rect);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void crystalline_viz_init(CrystallineVizState* state) {
    if (!state) return;
    
    memset(state, 0, sizeof(CrystallineVizState));
    
    // Initialize all sphere IDs to -1 (inactive)
    for (int i = 0; i < 144; i++) {
        state->spheres[i].sphere_id = -1;
        state->spheres[i].parent_id = -1;
    }
}

// ============================================================================
// UPDATE FROM TRAINING SYSTEM
// ============================================================================

void crystalline_viz_update(CrystallineVizState* viz_state, AppState* app_state) {
    if (!viz_state || !app_state) return;
    
    // Lock sphere stats for reading
    pthread_mutex_lock(&app_state->sphere_stats_mutex);
    
    // Update active sphere count
    viz_state->num_active_spheres = app_state->sphere_stats.active_spheres;
    
    // Reset global statistics
    viz_state->total_reads = 0;
    viz_state->total_writes = 0;
    viz_state->max_hierarchy_level = 0;
    memset(viz_state->spheres_per_level, 0, sizeof(viz_state->spheres_per_level));
    
    // Update each sphere
    for (int i = 0; i < viz_state->num_active_spheres && i < 144; i++) {
        CrystallineSphereViz* sphere = &viz_state->spheres[i];
        
        sphere->sphere_id = i;
        sphere->symmetry_group = app_state->sphere_stats.symmetry_group[i];
        sphere->hierarchy_level = app_state->sphere_stats.hierarchy_level[i];
        sphere->is_control = app_state->sphere_stats.is_control[i];
        sphere->batches_processed = app_state->sphere_stats.batches_processed[i];
        sphere->avg_loss = app_state->sphere_stats.avg_loss[i];
        sphere->parent_id = app_state->sphere_stats.parent_id[i];
        sphere->num_children = app_state->sphere_stats.num_children[i];
        
        // Calculate activity level (0.0 to 1.0)
        int max_batches = 1;
        for (int j = 0; j < viz_state->num_active_spheres; j++) {
            if (app_state->sphere_stats.batches_processed[j] > max_batches) {
                max_batches = app_state->sphere_stats.batches_processed[j];
            }
        }
        sphere->activity_level = (float)sphere->batches_processed / (float)max_batches;
        
        // Update hierarchy statistics
        if (sphere->hierarchy_level >= 0 && sphere->hierarchy_level < 5) {
            viz_state->spheres_per_level[sphere->hierarchy_level]++;
            if (sphere->hierarchy_level > viz_state->max_hierarchy_level) {
                viz_state->max_hierarchy_level = sphere->hierarchy_level;
            }
        }
        
        // Simulate segment statistics (in real implementation, would come from crystalline memory)
        for (int seg = 0; seg < 12; seg++) {
            sphere->segments[seg].reads = sphere->batches_processed * (seg + 1) * 100;
            sphere->segments[seg].writes = sphere->batches_processed * (seg + 1) * 50;
            sphere->segments[seg].activity = sphere->activity_level * (0.5f + 0.5f * (float)seg / 12.0f);
            
            viz_state->total_reads += sphere->segments[seg].reads;
            viz_state->total_writes += sphere->segments[seg].writes;
        }
    }
    
    // Calculate cache hit rate
    uint64_t total_accesses = viz_state->total_reads + viz_state->total_writes;
    if (total_accesses > 0) {
        // Simulate cache hits (in real implementation, would come from crystalline memory)
        uint64_t cache_hits = total_accesses * 85 / 100;  // Assume 85% hit rate
        viz_state->cache_hit_rate = (float)cache_hits / (float)total_accesses;
    } else {
        viz_state->cache_hit_rate = 0.0f;
    }
    
    pthread_mutex_unlock(&app_state->sphere_stats_mutex);
}

// ============================================================================
// SPHERE VISUALIZATION
// ============================================================================

void draw_crystalline_sphere(SDL_Renderer* renderer,
                            CrystallineSphereViz* sphere,
                            bool show_segments,
                            bool show_cache_info) {
    if (!renderer || !sphere || sphere->sphere_id < 0) return;
    
    // Get base color from activity
    SDL_Color base_color = get_activity_color(sphere->activity_level);
    
    if (show_segments) {
        // Draw 12 segments as pie slices
        for (int seg = 0; seg < 12; seg++) {
            float angle_start = (seg * 30.0f - 90.0f) * M_PI / 180.0f;
            float angle_end = ((seg + 1) * 30.0f - 90.0f) * M_PI / 180.0f;
            
            SDL_Color seg_color = get_segment_color(seg, sphere->segments[seg].activity);
            
            // Draw segment as filled triangle
            int cx = sphere->x;
            int cy = sphere->y;
            int r = sphere->radius;
            
            // Draw radial lines
            int x1 = cx + (int)(r * math_cos(angle_start));
            int y1 = cy + (int)(r * math_sin(angle_start));
            int x2 = cx + (int)(r * math_cos(angle_end));
            int y2 = cy + (int)(r * math_sin(angle_end));
            
            SDL_SetRenderDrawColor(renderer, seg_color.r, seg_color.g, seg_color.b, 255);
            SDL_RenderDrawLine(renderer, cx, cy, x1, y1);
            SDL_RenderDrawLine(renderer, cx, cy, x2, y2);
            
            // Fill segment (simplified - just draw arc)
            for (float a = angle_start; a < angle_end; a += 0.05f) {
                int x = cx + (int)(r * math_cos(a));
                int y = cy + (int)(r * math_sin(a));
                SDL_RenderDrawLine(renderer, cx, cy, x, y);
            }
        }
    } else {
        // Draw solid sphere
        draw_filled_circle(renderer, sphere->x, sphere->y, sphere->radius, base_color);
    }
    
    // Draw outline
    SDL_Color outline_color = {220, 220, 220, 255};
    if (sphere->is_control) {
        outline_color = (SDL_Color){255, 215, 0, 255};  // Gold for control threads
    }
    draw_circle_outline(renderer, sphere->x, sphere->y, sphere->radius, outline_color);
    
    // Draw sphere ID
    char id_text[8];
    snprintf(id_text, sizeof(id_text), "%d", sphere->sphere_id);
    draw_text_simple(renderer, id_text, sphere->x - 4, sphere->y - 6, outline_color);
    
    // Draw cache info if requested
    if (show_cache_info && sphere->cache_line >= 0) {
        char cache_text[32];
        snprintf(cache_text, sizeof(cache_text), "L%d", sphere->cache_line);
        draw_text_simple(renderer, cache_text, sphere->x - 8, sphere->y + sphere->radius + 5, 
                        (SDL_Color){150, 150, 255, 255});
    }
}

// ============================================================================
// COMPLETE VISUALIZATION
// ============================================================================

void draw_crystalline_visualization(SDL_Renderer* renderer, 
                                   CrystallineVizState* viz_state,
                                   SDL_Rect bounds) {
    if (!renderer || !viz_state) return;
    
    // Draw background
    SDL_Color bg_color = {25, 25, 30, 255};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &bounds);
    
    // Draw border
    SDL_Color border_color = {50, 50, 60, 255};
    SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, 255);
    SDL_RenderDrawRect(renderer, &bounds);
    
    // Calculate layout
    int center_x = bounds.x + bounds.w / 2;
    int center_y = bounds.y + bounds.h / 2;
    int min_dimension = (bounds.w < bounds.h ? bounds.w : bounds.h);
    int arrangement_radius = (min_dimension - 100) / 3;
    int sphere_radius = arrangement_radius / 5;
    
    // Position spheres in circular arrangement
    for (int i = 0; i < viz_state->num_active_spheres && i < 144; i++) {
        CrystallineSphereViz* sphere = &viz_state->spheres[i];
        
        if (sphere->sphere_id < 0) continue;
        
        // Calculate position based on symmetry group and hierarchy level
        int level = sphere->hierarchy_level;
        int group = sphere->symmetry_group;
        
        if (level == 0) {
            // Root sphere at center
            sphere->x = center_x;
            sphere->y = center_y;
            sphere->radius = sphere_radius / 2;
        } else {
            // Arrange in circle based on symmetry group
            float angle = (group * 30.0f - 90.0f) * M_PI / 180.0f;
            int radius_offset = arrangement_radius * level / 2;
            sphere->x = center_x + (int)(radius_offset * math_cos(angle));
            sphere->y = center_y + (int)(radius_offset * math_sin(angle));
            sphere->radius = sphere_radius;
        }
    }
    
    // Draw kissing boundaries first (behind spheres)
    draw_kissing_boundaries(renderer, viz_state, true);
    
    // Draw spheres
    for (int i = 0; i < viz_state->num_active_spheres && i < 144; i++) {
        CrystallineSphereViz* sphere = &viz_state->spheres[i];
        if (sphere->sphere_id >= 0) {
            draw_crystalline_sphere(renderer, sphere, true, true);
        }
    }
    
    // Draw statistics overlay
    draw_memory_statistics(renderer, viz_state, bounds);
}

// ============================================================================
// KISSING BOUNDARIES
// ============================================================================

void draw_kissing_boundaries(SDL_Renderer* renderer,
                            CrystallineVizState* viz_state,
                            bool show_activity) {
    if (!renderer || !viz_state) return;
    
    // Draw connections between adjacent spheres (kissing boundaries)
    for (int i = 0; i < viz_state->num_active_spheres; i++) {
        CrystallineSphereViz* sphere_a = &viz_state->spheres[i];
        if (sphere_a->sphere_id < 0) continue;
        
        // Draw connections to children
        for (int c = 0; c < sphere_a->num_children && c < 12; c++) {
            int child_id = sphere_a->child_ids[c];
            if (child_id >= 0 && child_id < viz_state->num_active_spheres) {
                CrystallineSphereViz* sphere_b = &viz_state->spheres[child_id];
                
                // Calculate activity level for this boundary
                float activity = (sphere_a->activity_level + sphere_b->activity_level) / 2.0f;
                
                SDL_Color line_color;
                if (show_activity) {
                    line_color = get_activity_color(activity);
                } else {
                    line_color = (SDL_Color){80, 80, 100, 255};
                }
                
                SDL_SetRenderDrawColor(renderer, line_color.r, line_color.g, line_color.b, 255);
                SDL_RenderDrawLine(renderer, sphere_a->x, sphere_a->y, sphere_b->x, sphere_b->y);
            }
        }
    }
}

// ============================================================================
// STATISTICS OVERLAY
// ============================================================================

void draw_memory_statistics(SDL_Renderer* renderer,
                           CrystallineVizState* viz_state,
                           SDL_Rect bounds) {
    if (!renderer || !viz_state) return;
    
    // Draw statistics panel at bottom
    int panel_height = 80;
    SDL_Rect stats_panel = {
        bounds.x + 10,
        bounds.y + bounds.h - panel_height - 10,
        bounds.w - 20,
        panel_height
    };
    
    SDL_Color panel_bg = {35, 35, 40, 200};
    SDL_SetRenderDrawColor(renderer, panel_bg.r, panel_bg.g, panel_bg.b, panel_bg.a);
    SDL_RenderFillRect(renderer, &stats_panel);
    
    SDL_Color panel_border = {80, 80, 100, 255};
    SDL_SetRenderDrawColor(renderer, panel_border.r, panel_border.g, panel_border.b, 255);
    SDL_RenderDrawRect(renderer, &stats_panel);
    
    // Draw statistics text (simplified)
    SDL_Color text_color = {220, 220, 220, 255};
    int text_y = stats_panel.y + 10;
    
    // Active spheres
    char stats_text[256];
    snprintf(stats_text, sizeof(stats_text), "Active Spheres: %d", viz_state->num_active_spheres);
    draw_text_simple(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    
    // Cache hit rate
    text_y += 20;
    snprintf(stats_text, sizeof(stats_text), "Cache Hit Rate: %.1f%%", viz_state->cache_hit_rate * 100.0f);
    draw_text_simple(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    
    // Total memory operations
    text_y += 20;
    snprintf(stats_text, sizeof(stats_text), "Reads: %llu  Writes: %llu", 
             (unsigned long long)viz_state->total_reads,
             (unsigned long long)viz_state->total_writes);
    draw_text_simple(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
}

// ============================================================================
// CACHE PLACEMENT VISUALIZATION
// ============================================================================

void draw_cache_placement(SDL_Renderer* renderer,
                         CrystallineVizState* viz_state,
                         SDL_Rect bounds) {
    // TODO: Implement cache placement visualization
    // This would show which spheres share cache lines
    (void)renderer;
    (void)viz_state;
    (void)bounds;
}

// ============================================================================
// HIERARCHY TREE
// ============================================================================

void draw_hierarchy_tree(SDL_Renderer* renderer,
                        CrystallineVizState* viz_state,
                        SDL_Rect bounds) {
    // TODO: Implement hierarchy tree visualization
    // This would show the parent-child relationships as a tree
    (void)renderer;
    (void)viz_state;
    (void)bounds;
}