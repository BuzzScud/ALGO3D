// app/enhanced_visualization.h - Enhanced visualization header
#ifndef ENHANCED_VISUALIZATION_H
#define ENHANCED_VISUALIZATION_H

#include "app_common.h"
#include "math/compact_vector.h"
#include <SDL2/SDL.h>

// Get precise 360-degree angle for a number using compact vectors
double get_precise_angle(int number, ClockContext* clock_ctx);

// Draw enhanced factor lines with 360-degree precision
void draw_enhanced_factor_lines(SDL_Renderer* renderer, AppState* state, 
                                int cx, int cy, double scale);

// Draw Platonic solid overlay for a number
void draw_platonic_solid_overlay(SDL_Renderer* renderer, AppState* state,
                                 int number, int cx, int cy, double scale);

// Draw sphere hierarchy visualization
void draw_sphere_hierarchy(SDL_Renderer* renderer, AppState* state,
                          int number, int cx, int cy, double scale);

// Draw all enhanced visualizations
void draw_all_enhanced_visualizations(SDL_Renderer* renderer, AppState* state,
                                     int cx, int cy, double scale);

#endif // ENHANCED_VISUALIZATION_H