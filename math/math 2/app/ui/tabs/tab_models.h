// tab_models.h - Model Management Tab Header
#ifndef TAB_MODELS_H
#define TAB_MODELS_H

#include "../../app_common.h"

// Initialize models tab
void init_models_tab(AppState* state);

// Cleanup models tab
void cleanup_models_tab(void);

// Draw models tab
void draw_models_tab(AppState* state);

// Render models tab (Crystalline UI)
void render_models_tab(SDL_Renderer* renderer, AppState* state);

// Handle mouse button down events
void handle_models_tab_mouse_down(SDL_MouseButtonEvent* event, AppState* state);

// Handle mouse button up events
void handle_models_tab_mouse_up(SDL_MouseButtonEvent* event, AppState* state);

// Handle mouse motion events
void handle_models_tab_mouse_motion(SDL_MouseMotionEvent* event, AppState* state);

// Handle click events
void handle_models_tab_click(AppState* state, int x, int y);

#endif // TAB_MODELS_H