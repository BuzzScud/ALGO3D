/**
 * Proper Layout System for Crystalline CLLM UI
 * 
 * Provides automatic positioning, responsive layouts, and overlap prevention
 */

#ifndef LAYOUT_SYSTEM_H
#define LAYOUT_SYSTEM_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Forward declarations
typedef struct LayoutNode LayoutNode;
typedef struct Layout Layout;

// Layout types
typedef enum {
    LAYOUT_GRID,      // Grid layout (rows/columns)
    LAYOUT_FLEX,      // Flexible box layout
    LAYOUT_ABSOLUTE   // Manual positioning
} LayoutType;

// Flex direction
typedef enum {
    FLEX_ROW,         // Horizontal
    FLEX_COLUMN       // Vertical
} FlexDirection;

// Alignment
typedef enum {
    ALIGN_START,      // Top/Left
    ALIGN_CENTER,     // Center
    ALIGN_END,        // Bottom/Right
    ALIGN_STRETCH     // Fill available space
} Alignment;

// Layout constraints
typedef struct {
    int min_width;
    int min_height;
    int max_width;
    int max_height;
    int padding_top;
    int padding_right;
    int padding_bottom;
    int padding_left;
    int margin_top;
    int margin_right;
    int margin_bottom;
    int margin_left;
} LayoutConstraints;

// Grid layout configuration
typedef struct {
    int rows;
    int columns;
    int row_gap;
    int column_gap;
    Alignment row_align;
    Alignment column_align;
} GridConfig;

// Flex layout configuration
typedef struct {
    FlexDirection direction;
    Alignment main_align;    // Along main axis
    Alignment cross_align;   // Along cross axis
    int gap;                 // Space between items
    bool wrap;               // Allow wrapping
} FlexConfig;

// Layout node (represents a UI element)
struct LayoutNode {
    // Identity
    char id[64];
    
    // Computed bounds (after layout)
    SDL_Rect bounds;
    
    // Constraints
    LayoutConstraints constraints;
    
    // Layout configuration (if this node is a container)
    LayoutType type;
    union {
        GridConfig grid;
        FlexConfig flex;
    } config;
    
    // Children
    LayoutNode** children;
    int num_children;
    int children_capacity;
    
    // Parent
    LayoutNode* parent;
    
    // Visibility
    bool visible;
    
    // User data
    void* user_data;
};

// Main layout manager
struct Layout {
    LayoutNode* root;
    int window_width;
    int window_height;
    bool needs_recompute;
};

// ============================================================================
// LAYOUT CREATION & DESTRUCTION
// ============================================================================

/**
 * Create a new layout manager
 */
Layout* layout_create(int window_width, int window_height);

/**
 * Destroy layout manager
 */
void layout_destroy(Layout* layout);

/**
 * Create a layout node
 */
LayoutNode* layout_node_create(const char* id, LayoutType type);

/**
 * Destroy a layout node and all its children
 */
void layout_node_destroy(LayoutNode* node);

// ============================================================================
// LAYOUT TREE MANIPULATION
// ============================================================================

/**
 * Set root node
 */
void layout_set_root(Layout* layout, LayoutNode* root);

/**
 * Add child to node
 */
void layout_node_add_child(LayoutNode* parent, LayoutNode* child);

/**
 * Remove child from node
 */
void layout_node_remove_child(LayoutNode* parent, LayoutNode* child);

/**
 * Find node by ID
 */
LayoutNode* layout_find_node(Layout* layout, const char* id);

// ============================================================================
// LAYOUT CONFIGURATION
// ============================================================================

/**
 * Configure grid layout
 */
void layout_node_configure_grid(LayoutNode* node, int rows, int columns, 
                                 int row_gap, int column_gap);

/**
 * Configure flex layout
 */
void layout_node_configure_flex(LayoutNode* node, FlexDirection direction,
                                 Alignment main_align, Alignment cross_align, int gap);

/**
 * Set node constraints
 */
void layout_node_set_constraints(LayoutNode* node, const LayoutConstraints* constraints);

/**
 * Set node padding (convenience function)
 */
void layout_node_set_padding(LayoutNode* node, int top, int right, int bottom, int left);

/**
 * Set node margin (convenience function)
 */
void layout_node_set_margin(LayoutNode* node, int top, int right, int bottom, int left);

/**
 * Set node size constraints
 */
void layout_node_set_size(LayoutNode* node, int min_width, int min_height,
                          int max_width, int max_height);

// ============================================================================
// LAYOUT COMPUTATION
// ============================================================================

/**
 * Compute layout (call after changes or window resize)
 */
void layout_compute(Layout* layout);

/**
 * Mark layout as needing recomputation
 */
void layout_invalidate(Layout* layout);

/**
 * Handle window resize
 */
void layout_resize(Layout* layout, int new_width, int new_height);

// ============================================================================
// LAYOUT QUERIES
// ============================================================================

/**
 * Get computed bounds for a node
 */
SDL_Rect layout_node_get_bounds(const LayoutNode* node);

/**
 * Check if point is inside node bounds
 */
bool layout_node_contains_point(const LayoutNode* node, int x, int y);

/**
 * Find node at point (returns deepest node)
 */
LayoutNode* layout_find_node_at_point(Layout* layout, int x, int y);

/**
 * Check for overlapping nodes (for debugging)
 */
bool layout_check_overlaps(Layout* layout);

// ============================================================================
// DEBUGGING
// ============================================================================

/**
 * Print layout tree (for debugging)
 */
void layout_print_tree(const Layout* layout);

/**
 * Draw layout bounds (for debugging)
 */
void layout_draw_debug(const Layout* layout, SDL_Renderer* renderer);

#endif // LAYOUT_SYSTEM_H