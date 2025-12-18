// app/ui/tabs/tab_url_manager.h - URL Manager Tab API
#ifndef TAB_URL_MANAGER_H
#define TAB_URL_MANAGER_H

#include "../../app_common.h"

// Render URL Manager tab
void draw_url_manager_tab(SDL_Renderer* renderer, AppState* state);

// Handle click events in URL Manager tab
void handle_url_manager_click(AppState* state, int x, int y);

#endif // TAB_URL_MANAGER_H