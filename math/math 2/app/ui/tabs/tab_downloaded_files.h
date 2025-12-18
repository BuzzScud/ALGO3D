// app/ui/tabs/tab_downloaded_files.h - Downloaded Files Viewer Tab API
#ifndef TAB_DOWNLOADED_FILES_H
#define TAB_DOWNLOADED_FILES_H

#include "../../app_common.h"

// Render Downloaded Files tab
void draw_downloaded_files_tab(SDL_Renderer* renderer, AppState* state);

// Handle click events in Downloaded Files tab
void handle_downloaded_files_click(AppState* state, int x, int y);

#endif // TAB_DOWNLOADED_FILES_H