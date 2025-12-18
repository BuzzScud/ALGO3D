// app/ui/left_sidebar.h - Left sidebar API
#ifndef LEFT_SIDEBAR_H
#define LEFT_SIDEBAR_H

#include "../app_common.h"

// Render left sidebar with vertical tabs
void render_left_sidebar(SDL_Renderer* renderer, AppState* state);

// Render horizontal submenu bar
void render_submenu(SDL_Renderer* renderer, AppState* state);

// Handle sidebar click
void handle_sidebar_click(AppState* state, int x, int y);

// Handle submenu click
void handle_submenu_click(AppState* state, int x, int y);

#endif // LEFT_SIDEBAR_H