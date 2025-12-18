// all_modes.h - Declarations for all visualization modes
#ifndef ALL_MODES_H
#define ALL_MODES_H

#include "app_common.h"

// Mode 0: Clock Spiral
void draw_enhanced_ulam_spiral(SDL_Renderer* renderer, AppState* state);

// Mode 2: Frequency Rings
void draw_harmonic_resonance(SDL_Renderer* renderer, AppState* state);

// Mode 3: 4D Clock
void draw_hyperdim_projection(SDL_Renderer* renderer, AppState* state);

// Mode 4: Ring Evolution
void draw_self_similar_layers(SDL_Renderer* renderer, AppState* state);

// Mode 7: Clock Folding
void draw_folding(SDL_Renderer* renderer, AppState* state);

#endif // ALL_MODES_H
