// app/visualization.c - Visualization modes implementation
// All modes based on clock abacus structure

#include "app_common.h"
#include "clock_abacus.h"
#include "all_modes.h"

void draw_visualization(SDL_Renderer* renderer, AppState* state) {
    // Center of visualization area (accounting for sidebar offset)
    int cx = RENDER_OFFSET_X + (RENDER_WIDTH / 2);
    int cy = RENDER_OFFSET_Y + (WINDOW_HEIGHT - RENDER_OFFSET_Y) / 2;
    
    switch (state->mode) {
        case MODE_ULAM_SPIRAL:
            draw_enhanced_ulam_spiral(renderer, state);
            break;
        
        case MODE_RECURSIVE_FRACTAL:
            for (int i = 0; i < (int)math_min(state->visible_primes, 100); i++) {
                draw_recursive_pattern(renderer, cx, cy, 200 * state->zoom, 
                                     state->recursion_depth, i, state);
            }
            break;
        
        case MODE_HARMONIC_RESONANCE:
            draw_harmonic_resonance(renderer, state);
            break;
        
        case MODE_HYPERDIM_PROJECTION:
            draw_hyperdim_projection(renderer, state);
            break;
        
        case MODE_SELF_SIMILAR_LAYERS:
            draw_self_similar_layers(renderer, state);
            break;
        
        case MODE_CLOCK_SUDOKU:
            draw_clock_abacus(renderer, state);
            break;
         
        case MODE_NATURAL_PRIME_LATTICE:
            draw_natural_prime_lattice(renderer, state);
            break;
        
        case MODE_FOLDED_ORIGAMI:
            draw_folding(renderer, state);
            break;
            
        default:
            break;
    }
    
    // Draw analyzed numbers for ALL modes
    draw_analyzed_numbers(renderer, state, cx, cy);
    
    // Draw factor connections for ALL modes if enabled
    if (state->show_factors && state->analysis_history.count > 0) {
        draw_factor_connections(renderer, state, cx, cy);
    }
    
    // Draw detailed overlay for ALL modes
    if (state->number_analysis.number > 0) {
        draw_detailed_overlay(renderer, state);
    }
}