// app/ui/scroll_panel.c - Scrollable Panel Implementation
#include "scroll_panel.h"
#include <stdlib.h>
#include <stdio.h>

ScrollPanel* scroll_panel_create(int x, int y, int width, int height) {
    ScrollPanel* panel = calloc(1, sizeof(ScrollPanel));
    if (!panel) return NULL;
    
    // Create base container (VBox for vertical scrolling)
    panel->container = layout_container_create(LAYOUT_VBOX, x, y, width - 20, height);
    if (!panel->container) {
        free(panel);
        return NULL;
    }
    
    // Initialize scrollbar properties
    panel->show_scrollbar = true;
    panel->scrollbar_width = 15;
    panel->scrollbar_handle_height = 50;
    panel->scrollbar_handle_y = 0;
    panel->scrollbar_dragging = false;
    panel->drag_start_y = 0;
    panel->drag_start_offset = 0;
    
    // Mouse wheel settings
    panel->wheel_scroll_speed = 20;
    
    // Initialize rects
    panel->viewport_rect.x = x;
    panel->viewport_rect.y = y;
    panel->viewport_rect.w = width - 20;  // Leave space for scrollbar
    panel->viewport_rect.h = height;
    
    panel->content_rect.x = x;
    panel->content_rect.y = y;
    panel->content_rect.w = width - 20;
    panel->content_rect.h = height;
    
    return panel;
}

void scroll_panel_destroy(ScrollPanel* panel) {
    if (!panel) return;
    
    if (panel->container) {
        layout_container_destroy(panel->container);
    }
    
    free(panel);
}

bool scroll_panel_add_child(ScrollPanel* panel, LayoutElement* child) {
    if (!panel || !panel->container) return false;
    return layout_container_add_child(panel->container, child);
}

void scroll_panel_calculate(ScrollPanel* panel) {
    if (!panel || !panel->container) return;
    
    // Calculate container layout
    layout_container_calculate(panel->container);
    
    // Update content height
    panel->content_rect.h = panel->container->content_height;
    
    // Update scrollbar visibility
    panel->show_scrollbar = (panel->content_rect.h > panel->viewport_rect.h);
    
    // Calculate scrollbar handle size
    if (panel->show_scrollbar) {
        float ratio = (float)panel->viewport_rect.h / (float)panel->content_rect.h;
        panel->scrollbar_handle_height = (int)(panel->viewport_rect.h * ratio);
        if (panel->scrollbar_handle_height < 20) {
            panel->scrollbar_handle_height = 20;  // Minimum handle size
        }
    }
}

void scroll_panel_render(ScrollPanel* panel, SDL_Renderer* renderer) {
    if (!panel || !renderer) return;
    
    // Set clip rect to viewport
    SDL_RenderSetClipRect(renderer, &panel->viewport_rect);
    
    // Apply scroll offset to all children
    for (int i = 0; i < panel->container->child_count; i++) {
        LayoutElement* child = panel->container->children[i];
        if (child) {
            child->y -= panel->container->scroll_offset_y;
        }
    }
    
    // Render container
    layout_container_render(panel->container, renderer);
    
    // Restore child positions
    for (int i = 0; i < panel->container->child_count; i++) {
        LayoutElement* child = panel->container->children[i];
        if (child) {
            child->y += panel->container->scroll_offset_y;
        }
    }
    
    // Clear clip rect
    SDL_RenderSetClipRect(renderer, NULL);
    
    // Render scrollbar if needed
    if (panel->show_scrollbar) {
        int scrollbar_x = panel->viewport_rect.x + panel->viewport_rect.w;
        int scrollbar_y = panel->viewport_rect.y;
        int scrollbar_h = panel->viewport_rect.h;
        
        // Draw scrollbar background
        SDL_Rect scrollbar_bg = {scrollbar_x, scrollbar_y, panel->scrollbar_width, scrollbar_h};
        SDL_SetRenderDrawColor(renderer, 40, 45, 50, 255);
        SDL_RenderFillRect(renderer, &scrollbar_bg);
        
        // Draw scrollbar border
        SDL_SetRenderDrawColor(renderer, 60, 65, 70, 255);
        SDL_RenderDrawRect(renderer, &scrollbar_bg);
        
        // Calculate handle position
        float scroll_ratio = (float)panel->container->scroll_offset_y / 
                            (float)(panel->content_rect.h - panel->viewport_rect.h);
        int handle_max_y = scrollbar_h - panel->scrollbar_handle_height;
        panel->scrollbar_handle_y = (int)(scroll_ratio * handle_max_y);
        
        // Draw scrollbar handle
        SDL_Rect handle = {
            scrollbar_x + 2,
            scrollbar_y + panel->scrollbar_handle_y,
            panel->scrollbar_width - 4,
            panel->scrollbar_handle_height
        };
        
        SDL_SetRenderDrawColor(renderer, 100, 120, 140, 255);
        SDL_RenderFillRect(renderer, &handle);
        
        SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
        SDL_RenderDrawRect(renderer, &handle);
    }
}

bool scroll_panel_is_in_scrollbar(ScrollPanel* panel, int x, int y) {
    if (!panel || !panel->show_scrollbar) return false;
    
    int scrollbar_x = panel->viewport_rect.x + panel->viewport_rect.w;
    int scrollbar_y = panel->viewport_rect.y;
    
    return (x >= scrollbar_x && x < scrollbar_x + panel->scrollbar_width &&
            y >= scrollbar_y && y < scrollbar_y + panel->viewport_rect.h);
}

bool scroll_panel_handle_input(ScrollPanel* panel, SDL_Event* event) {
    if (!panel || !event) return false;
    
    switch (event->type) {
        case SDL_MOUSEWHEEL: {
            // Mouse wheel scrolling
            if (panel->show_scrollbar) {
                int delta = -event->wheel.y * panel->wheel_scroll_speed;
                scroll_panel_scroll_by(panel, delta);
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                int x = event->button.x;
                int y = event->button.y;
                
                // Check if clicking on scrollbar
                if (scroll_panel_is_in_scrollbar(panel, x, y)) {
                    panel->scrollbar_dragging = true;
                    panel->drag_start_y = y;
                    panel->drag_start_offset = panel->container->scroll_offset_y;
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                panel->scrollbar_dragging = false;
                return panel->show_scrollbar;
            }
            break;
        }
        
        case SDL_MOUSEMOTION: {
            if (panel->scrollbar_dragging) {
                int delta_y = event->motion.y - panel->drag_start_y;
                
                // Convert pixel delta to content delta
                float scrollbar_h = panel->viewport_rect.h - panel->scrollbar_handle_height;
                float content_h = panel->content_rect.h - panel->viewport_rect.h;
                float ratio = content_h / scrollbar_h;
                
                int new_offset = panel->drag_start_offset + (int)(delta_y * ratio);
                scroll_panel_scroll_to(panel, new_offset);
                return true;
            }
            break;
        }
    }
    
    // Pass input to children if not handled
    return layout_container_handle_input(panel->container, event);
}

void scroll_panel_scroll_to(ScrollPanel* panel, int offset_y) {
    if (!panel) return;
    
    int max_offset = panel->content_rect.h - panel->viewport_rect.h;
    if (max_offset < 0) max_offset = 0;
    
    // Clamp offset
    if (offset_y < 0) offset_y = 0;
    if (offset_y > max_offset) offset_y = max_offset;
    
    panel->container->scroll_offset_y = offset_y;
}

void scroll_panel_scroll_by(ScrollPanel* panel, int delta_y) {
    if (!panel) return;
    
    int new_offset = panel->container->scroll_offset_y + delta_y;
    scroll_panel_scroll_to(panel, new_offset);
}