/**
 * Global Button Size Constants
 * 
 * Consistent button sizing across all tabs
 */

#ifndef BUTTON_SIZES_H
#define BUTTON_SIZES_H

// Circular button radii (for Crystalline UI style)
#define BUTTON_RADIUS_PRIMARY 40    // Main actions (START, TRAIN)
#define BUTTON_RADIUS_SECONDARY 25  // Secondary actions (SCAN, SELECT, LOAD) - Reduced from 30
#define BUTTON_RADIUS_TERTIARY 20   // Supporting actions (PAUSE, SAVE, STOP) - Reduced from 25
#define BUTTON_RADIUS_TINY 15       // Utility buttons (2D/3D toggle, settings) - Reduced from 20

// Rectangular button sizes
#define BUTTON_RECT_WIDTH_SMALL 80
#define BUTTON_RECT_WIDTH_MEDIUM 100
#define BUTTON_RECT_WIDTH_LARGE 120
#define BUTTON_RECT_HEIGHT 30

// Slider sizes
#define SLIDER_TRACK_HEIGHT 20
#define SLIDER_HANDLE_SIZE 12
#define SLIDER_LABEL_SPACING 25

// Panel spacing
#define PANEL_PADDING 15
#define ELEMENT_SPACING 10
#define SECTION_SPACING 20

#endif /* BUTTON_SIZES_H */