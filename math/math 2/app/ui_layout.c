#include "ui_layout.h"
#include <string.h>
#include <stdbool.h>

void layout_init(LayoutContainer* layout, SDL_Rect bounds, LayoutType type, int padding, int spacing) {
    layout->bounds = bounds;
    layout->type = type;
    layout->align = ALIGN_START;
    layout->padding = padding;
    layout->spacing = spacing;
    layout->current_y = bounds.y + padding;
    layout->current_x = bounds.x + padding;
}

SDL_Rect layout_add_element(LayoutContainer* layout, int width, int height) {
    SDL_Rect element_rect = {0};
    
    if (layout->type == LAYOUT_VERTICAL) {
        element_rect.x = layout->bounds.x + layout->padding;
        element_rect.y = layout->current_y;
        element_rect.w = width > 0 ? width : (layout->bounds.w - 2 * layout->padding);
        element_rect.h = height;
        
        layout->current_y += height + layout->spacing;
    } else if (layout->type == LAYOUT_HORIZONTAL) {
        element_rect.x = layout->current_x;
        element_rect.y = layout->bounds.y + layout->padding;
        element_rect.w = width;
        element_rect.h = height > 0 ? height : (layout->bounds.h - 2 * layout->padding);
        
        layout->current_x += width + layout->spacing;
    }
    
    return element_rect;
}

SDL_Rect layout_add_label(LayoutContainer* layout, const char* text, int height) {
    (void)text;  // Text rendering handled separately
    return layout_add_element(layout, 0, height);  // Full width
}

SDL_Rect layout_add_button(LayoutContainer* layout, const char* text, int width, int height) {
    (void)text;  // Text rendering handled separately
    return layout_add_element(layout, width, height);
}

SDL_Rect layout_add_text_input(LayoutContainer* layout, const char* label, int width, int height) {
    (void)label;  // Label rendering handled separately
    return layout_add_element(layout, width, height);
}

SDL_Rect layout_add_slider(LayoutContainer* layout, const char* label, int width, int height) {
    (void)label;  // Label rendering handled separately
    return layout_add_element(layout, width, height);
}

void layout_add_separator(LayoutContainer* layout, int height) {
    layout_add_element(layout, 0, height);
}

void layout_add_spacing(LayoutContainer* layout, int height) {
    if (layout->type == LAYOUT_VERTICAL) {
        layout->current_y += height;
    } else {
        layout->current_x += height;
    }
}

void layout_reset(LayoutContainer* layout) {
    layout->current_y = layout->bounds.y + layout->padding;
    layout->current_x = layout->bounds.x + layout->padding;
}

int layout_get_remaining_height(LayoutContainer* layout) {
    return (layout->bounds.y + layout->bounds.h) - layout->current_y - layout->padding;
}

bool rect_contains_point(SDL_Rect rect, int x, int y) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}