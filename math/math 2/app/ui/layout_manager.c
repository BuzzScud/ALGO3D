// app/ui/layout_manager.c - Flexible Layout Management System Implementation
#include "layout_manager.h"
#include <string.h>

/**
 * Calculate split layout (visualization + control panel)
 * Used by: Prime Spiral, Calculator, Spheres, Folding
 */
static TabLayout calculate_split_layout(int window_width, int window_height) {
    TabLayout layout = {0};
    layout.type = LAYOUT_SPLIT;
    layout.tab_bar_height = 40;  // Submenu height
    layout.window_width = window_width;
    layout.window_height = window_height;
    
    // Account for left sidebar (200px) and submenu (40px)
    int sidebar_width = 200;
    int content_width = window_width - sidebar_width;
    int content_height = window_height - layout.tab_bar_height;
    
    // Visualization area (left side of content area)
    layout.visualization_area.x = sidebar_width;
    layout.visualization_area.y = layout.tab_bar_height;
    layout.visualization_area.w = content_width - CONTROL_PANEL_WIDTH;
    layout.visualization_area.h = content_height;
    
    // Control panel (right side of content area)
    layout.control_area.x = window_width - CONTROL_PANEL_WIDTH;
    layout.control_area.y = layout.tab_bar_height;
    layout.control_area.w = CONTROL_PANEL_WIDTH;
    layout.control_area.h = content_height;
    
    layout.num_columns = 1;
    layout.columns[0].x = layout.control_area.x + 10;
    layout.columns[0].y = layout.control_area.y + 10;
    layout.columns[0].width = layout.control_area.w - 20;
    layout.columns[0].height = layout.control_area.h - 20;
    layout.columns[0].padding = 10;
    
    return layout;
}

/**
 * Calculate full-width layout with 2 columns
 * Used by: LLM, Training, Research
 */
static TabLayout calculate_full_width_2col(int window_width, int window_height, 
                                           float col1_percent) {
    TabLayout layout = {0};
    layout.type = LAYOUT_FULL_WIDTH;
    layout.tab_bar_height = 40;  // Submenu height
    layout.window_width = window_width;
    layout.window_height = window_height;
    
    // Full content area
    int margin = 20;
    layout.content_area.x = 200 + margin;  // Sidebar width + margin
    layout.content_area.y = layout.tab_bar_height;
    layout.content_area.w = window_width - 200 - (margin * 2);
    layout.content_area.h = window_height - layout.tab_bar_height;
    
    layout.num_columns = 2;
    int spacing = 20;
    int total_width = layout.content_area.w - spacing;
    
    // Column 1 (left)
    layout.columns[0].x = layout.content_area.x;
    layout.columns[0].y = layout.content_area.y;
    layout.columns[0].width = (int)(total_width * col1_percent);
    layout.columns[0].height = layout.content_area.h;
    layout.columns[0].padding = 10;
    
    // Column 2 (right)
    layout.columns[1].x = layout.columns[0].x + layout.columns[0].width + spacing;
    layout.columns[1].y = layout.content_area.y;
    layout.columns[1].width = total_width - layout.columns[0].width;
    layout.columns[1].height = layout.content_area.h;
    layout.columns[1].padding = 10;
    
    return layout;
}

/**
 * Calculate full-width layout with 3 columns
 * Used by: Crawler
 */
static TabLayout calculate_full_width_3col(int window_width, int window_height) {
    TabLayout layout = {0};
    layout.type = LAYOUT_FULL_WIDTH;
    layout.tab_bar_height = 40;  // Submenu height
    layout.window_width = window_width;
    layout.window_height = window_height;
    
    // Full content area
    int margin = 20;
    layout.content_area.x = 200 + margin;  // Sidebar width + margin
    layout.content_area.y = layout.tab_bar_height;
    layout.content_area.w = window_width - 200 - (margin * 2);
    layout.content_area.h = window_height - layout.tab_bar_height;
    
    layout.num_columns = 3;
    int spacing = 20;
    int total_width = layout.content_area.w - (spacing * 2);
    int col_width = total_width / 3;
    
    // Column 1 (left)
    layout.columns[0].x = layout.content_area.x;
    layout.columns[0].y = layout.content_area.y;
    layout.columns[0].width = col_width;
    layout.columns[0].height = layout.content_area.h;
    layout.columns[0].padding = 10;
    
    // Column 2 (middle)
    layout.columns[1].x = layout.columns[0].x + col_width + spacing;
    layout.columns[1].y = layout.content_area.y;
    layout.columns[1].width = col_width;
    layout.columns[1].height = layout.content_area.h;
    layout.columns[1].padding = 10;
    
    // Column 3 (right)
    layout.columns[2].x = layout.columns[1].x + col_width + spacing;
    layout.columns[2].y = layout.content_area.y;
    layout.columns[2].width = col_width;
    layout.columns[2].height = layout.content_area.h;
    layout.columns[2].padding = 10;
    
    return layout;
}

/**
 * Get layout configuration for a specific tab
 */
TabLayout get_tab_layout(TabMode tab, int window_width, int window_height) {
    switch (tab) {
        // Split layout tabs (visualization + control panel)
        case TAB_PRIME_SPIRAL:
        case TAB_CALCULATOR:
        case TAB_SPHERES:
        case TAB_PRIME_FOLDING:
            return calculate_split_layout(window_width, window_height);
        
        // Full-width 3-column layout
        case TAB_CRAWLER:
            return calculate_full_width_3col(window_width, window_height);
        
        // Full-width 2-column layout (70% / 30%)
        case TAB_LLM:
            return calculate_full_width_2col(window_width, window_height, 0.70f);
        
        // Full-width 2-column layout (60% / 40%)
        case TAB_TRAINING:
            return calculate_full_width_2col(window_width, window_height, 0.60f);
        
        // Full-width 2-column layout (40% / 60%)
        case TAB_RESEARCH:
            return calculate_full_width_2col(window_width, window_height, 0.40f);
        
        // Video tab - full width single column
        case TAB_VIDEO_GENERATOR:
            return calculate_full_width_2col(window_width, window_height, 0.70f);
        
        // Default to split layout
        default:
            return calculate_split_layout(window_width, window_height);
    }
}

/**
 * Get column layout for multi-column tabs
 */
ColumnLayout get_column_layout(const TabLayout* layout, int column_index) {
    if (!layout || column_index < 0 || column_index >= layout->num_columns) {
        ColumnLayout empty = {0};
        return empty;
    }
    
    return layout->columns[column_index];
}

/**
 * Check if a point is inside a column
 */
bool is_point_in_column(const TabLayout* layout, int column_index, int x, int y) {
    if (!layout || column_index < 0 || column_index >= layout->num_columns) {
        return false;
    }
    
    const ColumnLayout* col = &layout->columns[column_index];
    return (x >= col->x && x < col->x + col->width &&
            y >= col->y && y < col->y + col->height);
}

/**
 * Get column index for a point
 */
int get_column_at_point(const TabLayout* layout, int x, int y) {
    if (!layout) {
        return -1;
    }
    
    for (int i = 0; i < layout->num_columns; i++) {
        if (is_point_in_column(layout, i, x, y)) {
            return i;
        }
    }
    
    return -1;
}