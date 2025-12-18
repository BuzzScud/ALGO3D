// app/ui/layout_manager.h - Flexible Layout Management System
#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../app_common.h"

/**
 * Layout Types
 * 
 * Defines how a tab should be laid out on screen
 */
typedef enum {
    LAYOUT_SPLIT,      // Visualization area + Control panel (legacy tabs)
    LAYOUT_FULL_WIDTH, // Full window width with columns
    LAYOUT_CUSTOM      // Tab-specific custom layout
} TabLayoutType;

/**
 * Column Configuration
 * 
 * Defines column layout for multi-column tabs
 */
typedef struct {
    int x;              // Column X position
    int y;              // Column Y position
    int width;          // Column width
    int height;         // Column height
    int padding;        // Internal padding
} ColumnLayout;

/**
 * Tab Layout Configuration
 * 
 * Complete layout information for a tab
 */
typedef struct {
    TabLayoutType type;
    
    // For LAYOUT_SPLIT
    SDL_Rect visualization_area;  // Left side for visualization
    SDL_Rect control_area;        // Right side for controls
    
    // For LAYOUT_FULL_WIDTH
    SDL_Rect content_area;        // Full content area
    int num_columns;              // Number of columns
    ColumnLayout columns[4];      // Up to 4 columns
    
    // Common
    int tab_bar_height;           // Height of tab bar (usually 60)
    int window_width;             // Total window width
    int window_height;            // Total window height
} TabLayout;

/**
 * Get layout configuration for a specific tab
 * 
 * @param tab The tab mode
 * @param window_width Current window width
 * @param window_height Current window height
 * @return TabLayout structure with calculated dimensions
 */
TabLayout get_tab_layout(TabMode tab, int window_width, int window_height);

/**
 * Get column layout for multi-column tabs
 * 
 * @param layout The tab layout
 * @param column_index Which column (0-based)
 * @return ColumnLayout for the specified column, or empty rect if invalid
 */
ColumnLayout get_column_layout(const TabLayout* layout, int column_index);

/**
 * Check if a point is inside a column
 * 
 * @param layout The tab layout
 * @param column_index Which column
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 * @return true if point is inside column
 */
bool is_point_in_column(const TabLayout* layout, int column_index, int x, int y);

/**
 * Get column index for a point
 * 
 * @param layout The tab layout
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 * @return Column index (0-based) or -1 if not in any column
 */
int get_column_at_point(const TabLayout* layout, int x, int y);

#endif // LAYOUT_MANAGER_H