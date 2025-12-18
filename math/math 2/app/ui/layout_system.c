/**
 * Layout System Implementation
 */

#include "layout_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static void compute_grid_layout(LayoutNode* node);
static void compute_flex_layout(LayoutNode* node);
static void compute_absolute_layout(LayoutNode* node);
static void compute_node_layout(LayoutNode* node);

// ============================================================================
// LAYOUT CREATION & DESTRUCTION
// ============================================================================

Layout* layout_create(int window_width, int window_height) {
    Layout* layout = calloc(1, sizeof(Layout));
    if (!layout) return NULL;
    
    layout->window_width = window_width;
    layout->window_height = window_height;
    layout->needs_recompute = true;
    layout->root = NULL;
    
    return layout;
}

void layout_destroy(Layout* layout) {
    if (!layout) return;
    
    if (layout->root) {
        layout_node_destroy(layout->root);
    }
    
    free(layout);
}

LayoutNode* layout_node_create(const char* id, LayoutType type) {
    LayoutNode* node = calloc(1, sizeof(LayoutNode));
    if (!node) return NULL;
    
    // Set ID
    if (id) {
        strncpy(node->id, id, sizeof(node->id) - 1);
    }
    
    // Set type
    node->type = type;
    
    // Initialize constraints with defaults
    node->constraints.min_width = 0;
    node->constraints.min_height = 0;
    node->constraints.max_width = INT_MAX;
    node->constraints.max_height = INT_MAX;
    
    // Initialize children array
    node->children_capacity = 8;
    node->children = calloc(node->children_capacity, sizeof(LayoutNode*));
    node->num_children = 0;
    
    // Visible by default
    node->visible = true;
    
    return node;
}

void layout_node_destroy(LayoutNode* node) {
    if (!node) return;
    
    // Destroy all children
    for (int i = 0; i < node->num_children; i++) {
        layout_node_destroy(node->children[i]);
    }
    
    free(node->children);
    free(node);
}

// ============================================================================
// LAYOUT TREE MANIPULATION
// ============================================================================

void layout_set_root(Layout* layout, LayoutNode* root) {
    if (!layout) return;
    
    if (layout->root) {
        layout_node_destroy(layout->root);
    }
    
    layout->root = root;
    layout->needs_recompute = true;
}

void layout_node_add_child(LayoutNode* parent, LayoutNode* child) {
    if (!parent || !child) return;
    
    // Resize children array if needed
    if (parent->num_children >= parent->children_capacity) {
        parent->children_capacity *= 2;
        parent->children = realloc(parent->children, 
                                  parent->children_capacity * sizeof(LayoutNode*));
    }
    
    // Add child
    parent->children[parent->num_children++] = child;
    child->parent = parent;
}

void layout_node_remove_child(LayoutNode* parent, LayoutNode* child) {
    if (!parent || !child) return;
    
    // Find and remove child
    for (int i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < parent->num_children - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->num_children--;
            child->parent = NULL;
            break;
        }
    }
}

LayoutNode* layout_find_node(Layout* layout, const char* id) {
    if (!layout || !layout->root || !id) return NULL;
    
    // Recursive search
    LayoutNode* stack[256];
    int stack_size = 0;
    
    stack[stack_size++] = layout->root;
    
    while (stack_size > 0) {
        LayoutNode* node = stack[--stack_size];
        
        if (strcmp(node->id, id) == 0) {
            return node;
        }
        
        // Add children to stack
        for (int i = 0; i < node->num_children; i++) {
            if (stack_size < 256) {
                stack[stack_size++] = node->children[i];
            }
        }
    }
    
    return NULL;
}

// ============================================================================
// LAYOUT CONFIGURATION
// ============================================================================

void layout_node_configure_grid(LayoutNode* node, int rows, int columns,
                                 int row_gap, int column_gap) {
    if (!node) return;
    
    node->type = LAYOUT_GRID;
    node->config.grid.rows = rows;
    node->config.grid.columns = columns;
    node->config.grid.row_gap = row_gap;
    node->config.grid.column_gap = column_gap;
    node->config.grid.row_align = ALIGN_START;
    node->config.grid.column_align = ALIGN_START;
}

void layout_node_configure_flex(LayoutNode* node, FlexDirection direction,
                                 Alignment main_align, Alignment cross_align, int gap) {
    if (!node) return;
    
    node->type = LAYOUT_FLEX;
    node->config.flex.direction = direction;
    node->config.flex.main_align = main_align;
    node->config.flex.cross_align = cross_align;
    node->config.flex.gap = gap;
    node->config.flex.wrap = false;
}

void layout_node_set_constraints(LayoutNode* node, const LayoutConstraints* constraints) {
    if (!node || !constraints) return;
    node->constraints = *constraints;
}

void layout_node_set_padding(LayoutNode* node, int top, int right, int bottom, int left) {
    if (!node) return;
    node->constraints.padding_top = top;
    node->constraints.padding_right = right;
    node->constraints.padding_bottom = bottom;
    node->constraints.padding_left = left;
}

void layout_node_set_margin(LayoutNode* node, int top, int right, int bottom, int left) {
    if (!node) return;
    node->constraints.margin_top = top;
    node->constraints.margin_right = right;
    node->constraints.margin_bottom = bottom;
    node->constraints.margin_left = left;
}

void layout_node_set_size(LayoutNode* node, int min_width, int min_height,
                          int max_width, int max_height) {
    if (!node) return;
    node->constraints.min_width = min_width;
    node->constraints.min_height = min_height;
    node->constraints.max_width = max_width;
    node->constraints.max_height = max_height;
}

// ============================================================================
// LAYOUT COMPUTATION
// ============================================================================

static void compute_grid_layout(LayoutNode* node) {
    if (!node || node->num_children == 0) return;
    
    GridConfig* grid = &node->config.grid;
    
    // Calculate available space (minus padding)
    int available_width = node->bounds.w - 
                         node->constraints.padding_left - 
                         node->constraints.padding_right;
    int available_height = node->bounds.h - 
                          node->constraints.padding_top - 
                          node->constraints.padding_bottom;
    
    // Calculate cell size
    int cell_width = (available_width - (grid->columns - 1) * grid->column_gap) / grid->columns;
    int cell_height = (available_height - (grid->rows - 1) * grid->row_gap) / grid->rows;
    
    // Position children in grid
    for (int i = 0; i < node->num_children && i < grid->rows * grid->columns; i++) {
        LayoutNode* child = node->children[i];
        if (!child->visible) continue;
        
        int row = i / grid->columns;
        int col = i % grid->columns;
        
        // Calculate position
        child->bounds.x = node->bounds.x + node->constraints.padding_left +
                         col * (cell_width + grid->column_gap) +
                         child->constraints.margin_left;
        child->bounds.y = node->bounds.y + node->constraints.padding_top +
                         row * (cell_height + grid->row_gap) +
                         child->constraints.margin_top;
        
        // Calculate size (respecting constraints)
        child->bounds.w = cell_width - child->constraints.margin_left - child->constraints.margin_right;
        child->bounds.h = cell_height - child->constraints.margin_top - child->constraints.margin_bottom;
        
        // Apply min/max constraints
        if (child->bounds.w < child->constraints.min_width) child->bounds.w = child->constraints.min_width;
        if (child->bounds.h < child->constraints.min_height) child->bounds.h = child->constraints.min_height;
        if (child->bounds.w > child->constraints.max_width) child->bounds.w = child->constraints.max_width;
        if (child->bounds.h > child->constraints.max_height) child->bounds.h = child->constraints.max_height;
        
        // Recursively compute child layout
        compute_node_layout(child);
    }
}

static void compute_flex_layout(LayoutNode* node) {
    if (!node || node->num_children == 0) return;
    
    FlexConfig* flex = &node->config.flex;
    
    // Calculate available space
    int available_width = node->bounds.w - 
                         node->constraints.padding_left - 
                         node->constraints.padding_right;
    int available_height = node->bounds.h - 
                          node->constraints.padding_top - 
                          node->constraints.padding_bottom;
    
    // Count visible children
    int visible_count = 0;
    for (int i = 0; i < node->num_children; i++) {
        if (node->children[i]->visible) visible_count++;
    }
    
    if (visible_count == 0) return;
    
    // Calculate total gap space
    int total_gap = (visible_count - 1) * flex->gap;
    
    if (flex->direction == FLEX_ROW) {
        // Horizontal layout
        int available_for_children = available_width - total_gap;
        int child_width = available_for_children / visible_count;
        
        int current_x = node->bounds.x + node->constraints.padding_left;
        
        for (int i = 0; i < node->num_children; i++) {
            LayoutNode* child = node->children[i];
            if (!child->visible) continue;
            
            child->bounds.x = current_x + child->constraints.margin_left;
            child->bounds.y = node->bounds.y + node->constraints.padding_top + child->constraints.margin_top;
            child->bounds.w = child_width - child->constraints.margin_left - child->constraints.margin_right;
            child->bounds.h = available_height - child->constraints.margin_top - child->constraints.margin_bottom;
            
            // Apply constraints
            if (child->bounds.w < child->constraints.min_width) child->bounds.w = child->constraints.min_width;
            if (child->bounds.h < child->constraints.min_height) child->bounds.h = child->constraints.min_height;
            if (child->bounds.w > child->constraints.max_width) child->bounds.w = child->constraints.max_width;
            if (child->bounds.h > child->constraints.max_height) child->bounds.h = child->constraints.max_height;
            
            current_x += child_width + flex->gap;
            
            compute_node_layout(child);
        }
    } else {
        // Vertical layout
        int available_for_children = available_height - total_gap;
        int child_height = available_for_children / visible_count;
        
        int current_y = node->bounds.y + node->constraints.padding_top;
        
        for (int i = 0; i < node->num_children; i++) {
            LayoutNode* child = node->children[i];
            if (!child->visible) continue;
            
            child->bounds.x = node->bounds.x + node->constraints.padding_left + child->constraints.margin_left;
            child->bounds.y = current_y + child->constraints.margin_top;
            child->bounds.w = available_width - child->constraints.margin_left - child->constraints.margin_right;
            child->bounds.h = child_height - child->constraints.margin_top - child->constraints.margin_bottom;
            
            // Apply constraints
            if (child->bounds.w < child->constraints.min_width) child->bounds.w = child->constraints.min_width;
            if (child->bounds.h < child->constraints.min_height) child->bounds.h = child->constraints.min_height;
            if (child->bounds.w > child->constraints.max_width) child->bounds.w = child->constraints.max_width;
            if (child->bounds.h > child->constraints.max_height) child->bounds.h = child->constraints.max_height;
            
            current_y += child_height + flex->gap;
            
            compute_node_layout(child);
        }
    }
}

static void compute_absolute_layout(LayoutNode* node) {
    if (!node) return;
    
    // For absolute layout, children are positioned manually
    // Just recursively compute their children
    for (int i = 0; i < node->num_children; i++) {
        compute_node_layout(node->children[i]);
    }
}

static void compute_node_layout(LayoutNode* node) {
    if (!node || !node->visible) return;
    
    switch (node->type) {
        case LAYOUT_GRID:
            compute_grid_layout(node);
            break;
        case LAYOUT_FLEX:
            compute_flex_layout(node);
            break;
        case LAYOUT_ABSOLUTE:
            compute_absolute_layout(node);
            break;
    }
}

void layout_compute(Layout* layout) {
    if (!layout || !layout->root) return;
    
    // Set root bounds to window size
    layout->root->bounds.x = 0;
    layout->root->bounds.y = 0;
    layout->root->bounds.w = layout->window_width;
    layout->root->bounds.h = layout->window_height;
    
    // Compute layout tree
    compute_node_layout(layout->root);
    
    layout->needs_recompute = false;
}

void layout_invalidate(Layout* layout) {
    if (!layout) return;
    layout->needs_recompute = true;
}

void layout_resize(Layout* layout, int new_width, int new_height) {
    if (!layout) return;
    
    layout->window_width = new_width;
    layout->window_height = new_height;
    layout->needs_recompute = true;
    
    layout_compute(layout);
}

// ============================================================================
// LAYOUT QUERIES
// ============================================================================

SDL_Rect layout_node_get_bounds(const LayoutNode* node) {
    if (!node) return (SDL_Rect){0, 0, 0, 0};
    return node->bounds;
}

bool layout_node_contains_point(const LayoutNode* node, int x, int y) {
    if (!node || !node->visible) return false;
    
    return x >= node->bounds.x && x < node->bounds.x + node->bounds.w &&
           y >= node->bounds.y && y < node->bounds.y + node->bounds.h;
}

LayoutNode* layout_find_node_at_point(Layout* layout, int x, int y) {
    if (!layout || !layout->root) return NULL;
    
    // Depth-first search for deepest node containing point
    LayoutNode* result = NULL;
    LayoutNode* stack[256];
    int stack_size = 0;
    
    stack[stack_size++] = layout->root;
    
    while (stack_size > 0) {
        LayoutNode* node = stack[--stack_size];
        
        if (layout_node_contains_point(node, x, y)) {
            result = node;
            
            // Add children to stack (they're deeper)
            for (int i = 0; i < node->num_children; i++) {
                if (stack_size < 256) {
                    stack[stack_size++] = node->children[i];
                }
            }
        }
    }
    
    return result;
}

bool layout_check_overlaps(Layout* layout) {
    if (!layout || !layout->root) return false;
    
    // Collect all visible nodes
    LayoutNode* nodes[256];
    int node_count = 0;
    
    LayoutNode* stack[256];
    int stack_size = 0;
    stack[stack_size++] = layout->root;
    
    while (stack_size > 0 && node_count < 256) {
        LayoutNode* node = stack[--stack_size];
        
        if (node->visible) {
            nodes[node_count++] = node;
            
            for (int i = 0; i < node->num_children; i++) {
                if (stack_size < 256) {
                    stack[stack_size++] = node->children[i];
                }
            }
        }
    }
    
    // Check for overlaps
    for (int i = 0; i < node_count; i++) {
        for (int j = i + 1; j < node_count; j++) {
            SDL_Rect* a = &nodes[i]->bounds;
            SDL_Rect* b = &nodes[j]->bounds;
            
            if (SDL_HasIntersection(a, b)) {
                printf("WARNING: Overlap detected between '%s' and '%s'\n",
                       nodes[i]->id, nodes[j]->id);
                return true;
            }
        }
    }
    
    return false;
}

// ============================================================================
// DEBUGGING
// ============================================================================

static void print_node_tree(const LayoutNode* node, int depth) {
    if (!node) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    printf("%s [%d,%d %dx%d] %s\n", 
           node->id,
           node->bounds.x, node->bounds.y,
           node->bounds.w, node->bounds.h,
           node->visible ? "visible" : "hidden");
    
    for (int i = 0; i < node->num_children; i++) {
        print_node_tree(node->children[i], depth + 1);
    }
}

void layout_print_tree(const Layout* layout) {
    if (!layout || !layout->root) {
        printf("Layout: (empty)\n");
        return;
    }
    
    printf("Layout: %dx%d\n", layout->window_width, layout->window_height);
    print_node_tree(layout->root, 0);
}

void layout_draw_debug(const Layout* layout, SDL_Renderer* renderer) {
    if (!layout || !layout->root || !renderer) return;
    
    // Draw bounds for all nodes
    LayoutNode* stack[256];
    int stack_size = 0;
    stack[stack_size++] = layout->root;
    
    while (stack_size > 0) {
        LayoutNode* node = stack[--stack_size];
        
        if (node->visible) {
            // Draw border
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
            SDL_RenderDrawRect(renderer, &node->bounds);
            
            // Add children
            for (int i = 0; i < node->num_children; i++) {
                if (stack_size < 256) {
                    stack[stack_size++] = node->children[i];
                }
            }
        }
    }
}