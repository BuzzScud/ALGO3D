#ifndef TAB_TRAINING_H
#define TAB_TRAINING_H

#include "app_common.h"

void init_training_tab(AppState* state);
void cleanup_training_tab(void);
void draw_training_tab(SDL_Renderer* renderer, AppState* state);
void handle_training_tab_click(AppState* state, int x, int y);
void handle_training_tab_mouse_down(AppState* state, int x, int y);
void handle_training_tab_mouse_up(AppState* state, int x, int y);
void handle_training_tab_mouse_motion(AppState* state, int x, int y);
void handle_training_tab_keydown(AppState* state, SDL_Keycode key);
void handle_training_tab_text_input(AppState* state, const char* text);
void handle_training_tab_scroll(AppState* state, int wheel_y);

#endif /* TAB_TRAINING_H */