// app/ui/layout_engine.h - Comprehensive Layout Engine
#ifndef LAYOUT_ENGINE_H
#define LAYOUT_ENGINE_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Forward declarations
typedef struct LayoutElement LayoutElement;
typedef struct LayoutContainer LayoutContainer;

// Layout types
typedef enum {
    LAYOUT_CONTAINER,  // Base container
    LAYOUT_VBOX,       // Vertical box (stack vertically)
    LAYOUT_HBOX,       // Horizontal box (stack horizontally)
    LAYOUT_GRID        // Grid layout (rows x columns)
} LayoutType;

// Spacing constants
typedef enum {
    SPACING_NONE = 0,
    SPACING_SMALL = 5,
    SPACING_MEDIUM = 10,
    SPACING_LARGE = 20,
    SPACING_XLARGE = 40
} LayoutSpacing;

// Alignment options
typedef enum {
    ALIGN_START,   // Top/Left
    ALIGN_CENTER,  // Center
    ALIGN_END,     // Bottom/Right
    ALIGN_STRETCH  // Fill available space
} LayoutAlignment;

// Layout element (base for all UI elements)
struct LayoutElement {
    int x, y;              // Position
    int width, height;     // Size
    int min_width, min_height;  // Minimum size
    bool visible;          // Visibility flag
    bool enabled;          // Enabled flag
    void* user_data;       // User data pointer
    
    // Callbacks
    void (*render)(LayoutElement* elem, SDL_Renderer* renderer);
    bool (*handle_input)(LayoutElement* elem, SDL_Event* event);
    void (*destroy)(LayoutElement* elem);
};

// Layout container
struct LayoutContainer {
    LayoutElement base;    // Base element
    LayoutType type;       // Container type
    
    // Children
    LayoutElement** children;
    int child_count;
    int child_capacity;
    
    // Layout properties
    int padding;           // Internal padding
    int spacing;           // Spacing between children
    LayoutAlignment h_align;  // Horizontal alignment
    LayoutAlignment v_align;  // Vertical alignment
    
    // Grid-specific
    int grid_rows;
    int grid_cols;
    
    // Scrolling support
    int scroll_offset_x;
    int scroll_offset_y;
    int content_width;
    int content_height;
};

/**
 * Create a layout container
 * 
 * @param type Container type
 * @param x X position
 * @param y Y position
 * @param width Width
 * @param height Height
 * @return New container or NULL on failure
 */
LayoutContainer* layout_container_create(LayoutType type, int x, int y, int width, int height);

/**
 * Destroy a layout container
 * 
 * @param container Container to destroy
 */
void layout_container_destroy(LayoutContainer* container);

/**
 * Add a child element to container
 * 
 * @param container Parent container
 * @param child Child element to add
 * @return true on success, false on failure
 */
bool layout_container_add_child(LayoutContainer* container, LayoutElement* child);

/**
 * Remove a child element from container
 * 
 * @param container Parent container
 * @param child Child element to remove
 * @return true on success, false on failure
 */
bool layout_container_remove_child(LayoutContainer* container, LayoutElement* child);

/**
 * Set container padding
 * 
 * @param container Container
 * @param padding Padding in pixels
 */
void layout_container_set_padding(LayoutContainer* container, int padding);

/**
 * Set container spacing
 * 
 * @param container Container
 * @param spacing Spacing in pixels
 */
void layout_container_set_spacing(LayoutContainer* container, int spacing);

/**
 * Set container alignment
 * 
 * @param container Container
 * @param h_align Horizontal alignment
 * @param v_align Vertical alignment
 */
void layout_container_set_alignment(LayoutContainer* container, LayoutAlignment h_align, LayoutAlignment v_align);

/**
 * Set grid dimensions (for LAYOUT_GRID only)
 * 
 * @param container Container
 * @param rows Number of rows
 * @param cols Number of columns
 */
void layout_container_set_grid(LayoutContainer* container, int rows, int cols);

/**
 * Calculate layout (position all children)
 * 
 * @param container Container
 */
void layout_container_calculate(LayoutContainer* container);

/**
 * Render container and all children
 * 
 * @param container Container
 * @param renderer SDL renderer
 */
void layout_container_render(LayoutContainer* container, SDL_Renderer* renderer);

/**
 * Handle input for container and children
 * 
 * @param container Container
 * @param event SDL event
 * @return true if event was handled
 */
bool layout_container_handle_input(LayoutContainer* container, SDL_Event* event);

/**
 * Create a layout element (base)
 * 
 * @param x X position
 * @param y Y position
 * @param width Width
 * @param height Height
 * @return New element or NULL on failure
 */
LayoutElement* layout_element_create(int x, int y, int width, int height);

/**
 * Destroy a layout element
 * 
 * @param element Element to destroy
 */
void layout_element_destroy(LayoutElement* element);

#endif // LAYOUT_ENGINE_H