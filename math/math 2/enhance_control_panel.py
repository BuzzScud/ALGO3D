#!/usr/bin/env python3
"""
Apply Crystalline UI enhancements to the control panel.
"""

import re

def enhance_control_panel(content):
    """Add Crystalline UI borders to control panel"""
    
    # Find the control panel background drawing
    pattern = r'(// Draw panel background\s+SDL_SetRenderDrawColor\(renderer, bg_color\.r, bg_color\.g, bg_color\.b, 255\);\s+SDL_Rect panel_rect = \{panel_x, panel_y, panel_width - 20, panel_height\};.*?\s+SDL_RenderFillRect\(renderer, &panel_rect\);)'
    
    replacement = '''// CRYSTALLINE UI: Draw control panel with proper borders
       SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
       SDL_Rect panel_rect = {panel_x, panel_y, panel_width - 20, panel_height};  // Leave space for scrollbar
       SDL_RenderFillRect(renderer, &panel_rect);
       
       // Outer border (3px, lighter)
       SDL_Color panel_outer = {70, 70, 90, 255};
       for (int i = 0; i < 3; i++) {
           SDL_Rect outer = {panel_rect.x - i, panel_rect.y - i,
                            panel_rect.w + 2*i, panel_rect.h + 2*i};
           SDL_SetRenderDrawColor(renderer, panel_outer.r, panel_outer.g, panel_outer.b, 255);
           SDL_RenderDrawRect(renderer, &outer);
       }
       
       // Inner border (2px, darker)
       SDL_Color panel_inner = {30, 30, 40, 255};
       for (int i = 0; i < 2; i++) {
           SDL_Rect inner = {panel_rect.x + 3 + i, panel_rect.y + 3 + i,
                            panel_rect.w - 6 - 2*i, panel_rect.h - 6 - 2*i};
           SDL_SetRenderDrawColor(renderer, panel_inner.r, panel_inner.g, panel_inner.b, 255);
           SDL_RenderDrawRect(renderer, &inner);
       }'''
    
    content = re.sub(pattern, replacement, content, flags=re.DOTALL)
    return content

def main():
    # Read the file
    with open('app/ui/tabs/tab_training.c', 'r') as f:
        content = f.read()
    
    print("Enhancing control panel...")
    
    # Apply enhancements
    content = enhance_control_panel(content)
    
    # Write back
    with open('app/ui/tabs/tab_training.c', 'w') as f:
        f.write(content)
    
    print("✓ Control panel enhanced with Crystalline UI borders!")

if __name__ == '__main__':
    main()