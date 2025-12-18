// app/ui/tabs/tab_crawler.h - Crawler Tab Interface
#ifndef TAB_CRAWLER_H
#define TAB_CRAWLER_H

#include "../../app_common.h"
#include "../layout_manager.h"

/**
 * Initialize the crawler tab
 */
void init_crawler_tab(AppState* state);

/**
 * Render the crawler tab (Crystalline UI)
 */
void render_crawler_tab(SDL_Renderer* renderer, AppState* state);

/**
 * Update the crawler tab state
 */
void update_crawler_tab(AppState* state);

/**
 * Draw the crawler control tab with layout system (legacy)
 */
void draw_crawler_tab_with_layout(AppState* state, const TabLayout* layout);

/**
 * Handle mouse clicks in the crawler tab
 */
void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y);

/**
 * Handle keyboard input in the crawler tab
 */
void handle_crawler_tab_keyboard(AppState* state, int key);

/**
 * Handle mouse button down events
 */
void handle_crawler_tab_mouse_down(SDL_MouseButtonEvent* event, AppState* state);

/**
 * Handle mouse button up events
 */
void handle_crawler_tab_mouse_up(SDL_MouseButtonEvent* event, AppState* state);

/**
 * Handle mouse motion events
 */
void handle_crawler_tab_mouse_motion(SDL_MouseMotionEvent* event, AppState* state);

/**
 * Cleanup crawler tab resources
 */
void cleanup_crawler_tab(void);

#endif // TAB_CRAWLER_H