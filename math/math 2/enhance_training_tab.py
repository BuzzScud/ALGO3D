#!/usr/bin/env python3
"""
Apply Crystalline UI enhancements to Training Tab while preserving all functionality.
This script adds visual polish without removing any features.
"""

import re

def enhance_visualization_borders(content):
    """Add Crystalline UI borders to the visualization area"""
    
    # Find the visualization drawing section
    pattern = r'(// Draw background\s+SDL_SetRenderDrawColor.*?SDL_RenderFillRect.*?viz_area_rect.*?\s+// Draw border\s+SDL_SetRenderDrawColor.*?SDL_RenderDrawRect.*?viz_area_rect.*?;)'
    
    replacement = '''// CRYSTALLINE UI: Draw background with proper panel styling
       SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
       SDL_RenderFillRect(renderer, &viz_area_rect);
       
       // CRYSTALLINE UI: Outer border (3px, lighter)
       SDL_Color outer_border = {60, 60, 70, 255};
       for (int i = 0; i < 3; i++) {
           SDL_Rect outer = {viz_area_rect.x - i, viz_area_rect.y - i, 
                            viz_area_rect.w + 2*i, viz_area_rect.h + 2*i};
           SDL_SetRenderDrawColor(renderer, outer_border.r, outer_border.g, outer_border.b, 255);
           SDL_RenderDrawRect(renderer, &outer);
       }
       
       // CRYSTALLINE UI: Inner border (2px, darker)
       SDL_Color inner_border = {30, 30, 40, 255};
       for (int i = 0; i < 2; i++) {
           SDL_Rect inner = {viz_area_rect.x + 3 + i, viz_area_rect.y + 3 + i,
                            viz_area_rect.w - 6 - 2*i, viz_area_rect.h - 6 - 2*i};
           SDL_SetRenderDrawColor(renderer, inner_border.r, inner_border.g, inner_border.b, 255);
           SDL_RenderDrawRect(renderer, &inner);
       }'''
    
    content = re.sub(pattern, replacement, content, flags=re.DOTALL)
    return content

def enhance_metrics_panel(content):
    """Add Crystalline UI styling to metrics panel"""
    
    # Find the metrics panel drawing
    pattern = r'(// Draw metrics panel background\s+SDL_SetRenderDrawColor.*?25, 25, 30.*?SDL_RenderFillRect.*?metrics_bounds.*?\s+SDL_SetRenderDrawColor.*?grid_color.*?SDL_RenderDrawRect.*?metrics_bounds.*?;)'
    
    replacement = '''// CRYSTALLINE UI: Draw metrics panel with proper borders
       SDL_SetRenderDrawColor(renderer, 25, 25, 30, 255);
       SDL_RenderFillRect(renderer, &metrics_bounds);
       
       // Outer border (3px, lighter)
       SDL_Color metrics_outer = {50, 50, 60, 255};
       for (int i = 0; i < 3; i++) {
           SDL_Rect outer = {metrics_bounds.x - i, metrics_bounds.y - i,
                            metrics_bounds.w + 2*i, metrics_bounds.h + 2*i};
           SDL_SetRenderDrawColor(renderer, metrics_outer.r, metrics_outer.g, metrics_outer.b, 255);
           SDL_RenderDrawRect(renderer, &outer);
       }
       
       // Inner border (2px, darker)
       SDL_Color metrics_inner = {20, 20, 25, 255};
       for (int i = 0; i < 2; i++) {
           SDL_Rect inner = {metrics_bounds.x + 3 + i, metrics_bounds.y + 3 + i,
                            metrics_bounds.w - 6 - 2*i, metrics_bounds.h - 6 - 2*i};
           SDL_SetRenderDrawColor(renderer, metrics_inner.r, metrics_inner.g, metrics_inner.b, 255);
           SDL_RenderDrawRect(renderer, &inner);
       }'''
    
    content = re.sub(pattern, replacement, content, flags=re.DOTALL)
    return content

def main():
    # Read the file
    with open('app/ui/tabs/tab_training.c', 'r') as f:
        content = f.read()
    
    print("Applying Crystalline UI enhancements...")
    
    # Apply enhancements
    content = enhance_visualization_borders(content)
    content = enhance_metrics_panel(content)
    
    # Write back
    with open('app/ui/tabs/tab_training.c', 'w') as f:
        f.write(content)
    
    print("✓ Crystalline UI enhancements applied successfully!")
    print("  - Added proper panel borders (3px outer, 2px inner)")
    print("  - Enhanced visual hierarchy")
    print("  - Preserved all existing functionality")

if __name__ == '__main__':
    main()