#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include <SDL2/SDL.h>
#include <stdbool.h>

/**
 * UI Layout System
 * 
 * Provides dynamic layout management for UI elements to avoid hardcoded positions
 * and overlapping elements. Follows proper UI design principles.
 */

// Layout container types
typedef enum {
    LAYOUT_VERTICAL,    // Stack elements vertically
    LAYOUT_HORIZONTAL,  // Stack elements horizontally
    LAYOUT_GRID         // Grid layout
} LayoutType;

// Layout alignment
typedef enum {
    ALIGN_START,   // Top/Left
    ALIGN_CENTER,  // Center
    ALIGN_END      // Bottom/Right
} LayoutAlign;

// UI Element types
typedef enum {
    UI_LABEL,
    UI_BUTTON,
    UI_TEXT_INPUT,
    UI_SLIDER,
    UI_CHECKBOX,
    UI_LIST,
    UI_PROGRESS_BAR,
    UI_SEPARATOR
} UIElementType;

// Layout container
typedef struct {
    SDL_Rect bounds;
    LayoutType type;
    LayoutAlign align;
    int padding;
    int spacing;
    int current_y;  // For vertical layout
    int current_x;  // For horizontal layout
} LayoutContainer;

// UI Element
typedef struct {
    UIElementType type;
    SDL_Rect bounds;
    char label[128];
    void* data;  // Type-specific data
    bool visible;
    bool enabled;
} UIElement;

/**
 * Initialize a layout container
 */
void layout_init(LayoutContainer* layout, SDL_Rect bounds, LayoutType type, int padding, int spacing);

/**
 * Add an element to the layout and get its calculated bounds
 */
SDL_Rect layout_add_element(LayoutContainer* layout, int width, int height);

/**
 * Add a label to the layout
 */
SDL_Rect layout_add_label(LayoutContainer* layout, const char* text, int height);

/**
 * Add a button to the layout
 */
SDL_Rect layout_add_button(LayoutContainer* layout, const char* text, int width, int height);

/**
 * Add a text input to the layout
 */
SDL_Rect layout_add_text_input(LayoutContainer* layout, const char* label, int width, int height);

/**
 * Add a slider to the layout
 */
SDL_Rect layout_add_slider(LayoutContainer* layout, const char* label, int width, int height);

/**
 * Add a separator (horizontal line)
 */
void layout_add_separator(LayoutContainer* layout, int height);

/**
 * Add spacing
 */
void layout_add_spacing(LayoutContainer* layout, int height);

/**
 * Reset layout to start position
 */
void layout_reset(LayoutContainer* layout);

/**
 * Get remaining height in layout
 */
int layout_get_remaining_height(LayoutContainer* layout);

/**
 * Check if point is inside rect
 */
bool rect_contains_point(SDL_Rect rect, int x, int y);

#endif // UI_LAYOUT_H