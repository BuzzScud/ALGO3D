// app/ui/scroll_panel.h - Scrollable Panel Component
#ifndef SCROLL_PANEL_H
#define SCROLL_PANEL_H

#include "layout_engine.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

// Scroll panel structure
typedef struct {
    LayoutContainer* container;  // Base container
    
    // Scrollbar properties
    bool show_scrollbar;
    int scrollbar_width;
    int scrollbar_handle_height;
    int scrollbar_handle_y;
    bool scrollbar_dragging;
    int drag_start_y;
    int drag_start_offset;
    
    // Mouse wheel support
    int wheel_scroll_speed;
    
    // Content bounds
    SDL_Rect content_rect;
    SDL_Rect viewport_rect;
} ScrollPanel;

/**
 * Create a scroll panel
 * 
 * @param x X position
 * @param y Y position
 * @param width Width
 * @param height Height
 * @return New scroll panel or NULL on failure
 */
ScrollPanel* scroll_panel_create(int x, int y, int width, int height);

/**
 * Destroy a scroll panel
 * 
 * @param panel Panel to destroy
 */
void scroll_panel_destroy(ScrollPanel* panel);

/**
 * Add a child element to scroll panel
 * 
 * @param panel Scroll panel
 * @param child Child element to add
 * @return true on success, false on failure
 */
bool scroll_panel_add_child(ScrollPanel* panel, LayoutElement* child);

/**
 * Calculate scroll panel layout
 * 
 * @param panel Scroll panel
 */
void scroll_panel_calculate(ScrollPanel* panel);

/**
 * Render scroll panel
 * 
 * @param panel Scroll panel
 * @param renderer SDL renderer
 */
void scroll_panel_render(ScrollPanel* panel, SDL_Renderer* renderer);

/**
 * Handle input for scroll panel
 * 
 * @param panel Scroll panel
 * @param event SDL event
 * @return true if event was handled
 */
bool scroll_panel_handle_input(ScrollPanel* panel, SDL_Event* event);

/**
 * Scroll to specific offset
 * 
 * @param panel Scroll panel
 * @param offset_y Vertical offset in pixels
 */
void scroll_panel_scroll_to(ScrollPanel* panel, int offset_y);

/**
 * Scroll by delta
 * 
 * @param panel Scroll panel
 * @param delta_y Vertical delta in pixels
 */
void scroll_panel_scroll_by(ScrollPanel* panel, int delta_y);

/**
 * Check if point is inside scrollbar
 * 
 * @param panel Scroll panel
 * @param x X coordinate
 * @param y Y coordinate
 * @return true if inside scrollbar
 */
bool scroll_panel_is_in_scrollbar(ScrollPanel* panel, int x, int y);

#endif // SCROLL_PANEL_H