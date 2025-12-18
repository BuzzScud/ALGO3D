// app/ui/layout_engine.c - Comprehensive Layout Engine Implementation
#include "layout_engine.h"
#include <stdlib.h>
#include <string.h>

// Helper function to calculate VBox layout
static void calculate_vbox_layout(LayoutContainer* container) {
    if (!container || container->child_count == 0) return;
    
    int current_y = container->base.y + container->padding;
    int available_width = container->base.width - (2 * container->padding);
    
    for (int i = 0; i < container->child_count; i++) {
        LayoutElement* child = container->children[i];
        if (!child || !child->visible) continue;
        
        // Position child
        child->y = current_y;
        
        // Handle horizontal alignment
        switch (container->h_align) {
            case ALIGN_START:
                child->x = container->base.x + container->padding;
                break;
            case ALIGN_CENTER:
                child->x = container->base.x + container->padding + (available_width - child->width) / 2;
                break;
            case ALIGN_END:
                child->x = container->base.x + container->base.width - container->padding - child->width;
                break;
            case ALIGN_STRETCH:
                child->x = container->base.x + container->padding;
                child->width = available_width;
                break;
        }
        
        current_y += child->height + container->spacing;
    }
    
    // Update content height
    container->content_height = current_y - container->base.y - container->spacing + container->padding;
}

// Helper function to calculate HBox layout
static void calculate_hbox_layout(LayoutContainer* container) {
    if (!container || container->child_count == 0) return;
    
    int current_x = container->base.x + container->padding;
    int available_height = container->base.height - (2 * container->padding);
    
    for (int i = 0; i < container->child_count; i++) {
        LayoutElement* child = container->children[i];
        if (!child || !child->visible) continue;
        
        // Position child
        child->x = current_x;
        
        // Handle vertical alignment
        switch (container->v_align) {
            case ALIGN_START:
                child->y = container->base.y + container->padding;
                break;
            case ALIGN_CENTER:
                child->y = container->base.y + container->padding + (available_height - child->height) / 2;
                break;
            case ALIGN_END:
                child->y = container->base.y + container->base.height - container->padding - child->height;
                break;
            case ALIGN_STRETCH:
                child->y = container->base.y + container->padding;
                child->height = available_height;
                break;
        }
        
        current_x += child->width + container->spacing;
    }
    
    // Update content width
    container->content_width = current_x - container->base.x - container->spacing + container->padding;
}

// Helper function to calculate Grid layout
static void calculate_grid_layout(LayoutContainer* container) {
    if (!container || container->child_count == 0) return;
    if (container->grid_rows <= 0 || container->grid_cols <= 0) return;
    
    int available_width = container->base.width - (2 * container->padding) - ((container->grid_cols - 1) * container->spacing);
    int available_height = container->base.height - (2 * container->padding) - ((container->grid_rows - 1) * container->spacing);
    
    int cell_width = available_width / container->grid_cols;
    int cell_height = available_height / container->grid_rows;
    
    for (int i = 0; i < container->child_count; i++) {
        LayoutElement* child = container->children[i];
        if (!child || !child->visible) continue;
        
        int row = i / container->grid_cols;
        int col = i % container->grid_cols;
        
        if (row >= container->grid_rows) break;  // No more space
        
        child->x = container->base.x + container->padding + (col * (cell_width + container->spacing));
        child->y = container->base.y + container->padding + (row * (cell_height + container->spacing));
        child->width = cell_width;
        child->height = cell_height;
    }
}

LayoutElement* layout_element_create(int x, int y, int width, int height) {
    LayoutElement* elem = calloc(1, sizeof(LayoutElement));
    if (!elem) return NULL;
    
    elem->x = x;
    elem->y = y;
    elem->width = width;
    elem->height = height;
    elem->min_width = 0;
    elem->min_height = 0;
    elem->visible = true;
    elem->enabled = true;
    elem->user_data = NULL;
    elem->render = NULL;
    elem->handle_input = NULL;
    elem->destroy = NULL;
    
    return elem;
}

void layout_element_destroy(LayoutElement* element) {
    if (!element) return;
    
    if (element->destroy) {
        element->destroy(element);
    }
    
    free(element);
}

LayoutContainer* layout_container_create(LayoutType type, int x, int y, int width, int height) {
    LayoutContainer* container = calloc(1, sizeof(LayoutContainer));
    if (!container) return NULL;
    
    // Initialize base element
    container->base.x = x;
    container->base.y = y;
    container->base.width = width;
    container->base.height = height;
    container->base.visible = true;
    container->base.enabled = true;
    
    // Initialize container properties
    container->type = type;
    container->children = NULL;
    container->child_count = 0;
    container->child_capacity = 0;
    container->padding = SPACING_MEDIUM;
    container->spacing = SPACING_SMALL;
    container->h_align = ALIGN_START;
    container->v_align = ALIGN_START;
    container->grid_rows = 1;
    container->grid_cols = 1;
    container->scroll_offset_x = 0;
    container->scroll_offset_y = 0;
    container->content_width = width;
    container->content_height = height;
    
    return container;
}

void layout_container_destroy(LayoutContainer* container) {
    if (!container) return;
    
    // Destroy all children
    for (int i = 0; i < container->child_count; i++) {
        if (container->children[i]) {
            layout_element_destroy(container->children[i]);
        }
    }
    
    free(container->children);
    free(container);
}

bool layout_container_add_child(LayoutContainer* container, LayoutElement* child) {
    if (!container || !child) return false;
    
    // Expand capacity if needed
    if (container->child_count >= container->child_capacity) {
        int new_capacity = container->child_capacity == 0 ? 8 : container->child_capacity * 2;
        LayoutElement** new_children = realloc(container->children, new_capacity * sizeof(LayoutElement*));
        if (!new_children) return false;
        
        container->children = new_children;
        container->child_capacity = new_capacity;
    }
    
    container->children[container->child_count++] = child;
    return true;
}

bool layout_container_remove_child(LayoutContainer* container, LayoutElement* child) {
    if (!container || !child) return false;
    
    for (int i = 0; i < container->child_count; i++) {
        if (container->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < container->child_count - 1; j++) {
                container->children[j] = container->children[j + 1];
            }
            container->child_count--;
            return true;
        }
    }
    
    return false;
}

void layout_container_set_padding(LayoutContainer* container, int padding) {
    if (container) {
        container->padding = padding;
    }
}

void layout_container_set_spacing(LayoutContainer* container, int spacing) {
    if (container) {
        container->spacing = spacing;
    }
}

void layout_container_set_alignment(LayoutContainer* container, LayoutAlignment h_align, LayoutAlignment v_align) {
    if (container) {
        container->h_align = h_align;
        container->v_align = v_align;
    }
}

void layout_container_set_grid(LayoutContainer* container, int rows, int cols) {
    if (container && container->type == LAYOUT_GRID) {
        container->grid_rows = rows > 0 ? rows : 1;
        container->grid_cols = cols > 0 ? cols : 1;
    }
}

void layout_container_calculate(LayoutContainer* container) {
    if (!container) return;
    
    switch (container->type) {
        case LAYOUT_VBOX:
            calculate_vbox_layout(container);
            break;
        case LAYOUT_HBOX:
            calculate_hbox_layout(container);
            break;
        case LAYOUT_GRID:
            calculate_grid_layout(container);
            break;
        case LAYOUT_CONTAINER:
            // Base container doesn't auto-layout children
            break;
    }
}

void layout_container_render(LayoutContainer* container, SDL_Renderer* renderer) {
    if (!container || !renderer || !container->base.visible) return;
    
    // Render all visible children
    for (int i = 0; i < container->child_count; i++) {
        LayoutElement* child = container->children[i];
        if (child && child->visible && child->render) {
            child->render(child, renderer);
        }
    }
}

bool layout_container_handle_input(LayoutContainer* container, SDL_Event* event) {
    if (!container || !event || !container->base.enabled) return false;
    
    // Handle input for children (reverse order for proper z-ordering)
    for (int i = container->child_count - 1; i >= 0; i--) {
        LayoutElement* child = container->children[i];
        if (child && child->visible && child->enabled && child->handle_input) {
            if (child->handle_input(child, event)) {
                return true;  // Event was handled
            }
        }
    }
    
    return false;
}