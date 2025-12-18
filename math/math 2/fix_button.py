#!/usr/bin/env python3
import re

with open('app/ui/tabs/tab_training.c', 'r') as f:
    content = f.read()

old_section = '''       // Check for 2D/3D toggle button click
       // Calculate sphere visualization bounds (same as in draw function)
       int content_x = RENDER_OFFSET_X + 20;
       int content_y = RENDER_OFFSET_Y + 20;
       int content_w = RENDER_WIDTH - 40;
       int content_h = WINDOW_HEIGHT - RENDER_OFFSET_Y - 40;
       
       // CRITICAL FIX: Add the same 45 pixel offset that's in the draw function!
       content_y += 45;  // Account for progress bar height
       
       int sphere_viz_width = (content_w * 6) / 10;
       int sphere_viz_height = (content_h * 7) / 10;
       if (sphere_viz_height < 500) sphere_viz_height = 500;
       
       // Toggle button bounds
       int toggle_x = content_x + sphere_viz_width - 110;
       int toggle_y = content_y + 10;
       int toggle_w = 100;
       int toggle_h = 30;'''

new_section = '''       // Check for 2D/3D toggle button click
       // CRITICAL: Use viz_area_rect coordinates (same as draw function)
       int content_x = viz_area_rect.x + 20;
       int content_y = viz_area_rect.y + 20;
       int content_w = viz_area_rect.w - 40;
       int content_h = viz_area_rect.h - 40;
       
       // Account for title
       content_y += 30;
       
       // Account for status message if shown
       if (state->training_in_progress && state->training_status_message[0] != '\\0') {
           content_y += 25;
           
           // Account for preprocessing progress bar if shown
           if (state->training_preprocessing_progress > 0.0f && 
               state->training_preprocessing_progress < 1.0f) {
               content_y += 35;
           }
       }
       
       // Account for progress bar
       content_y += 45;
       
       int sphere_viz_width = (content_w * 6) / 10;
       int sphere_viz_height = (content_h * 7) / 10;
       if (sphere_viz_height < 500) sphere_viz_height = 500;
       
       // Toggle button bounds
       int toggle_x = content_x + sphere_viz_width - 110;
       int toggle_y = content_y + 10;
       int toggle_w = 100;
       int toggle_h = 30;'''

if old_section in content:
    content = content.replace(old_section, new_section)
    with open('app/ui/tabs/tab_training.c', 'w') as f:
        f.write(content)
    print("✓ Fixed button click detection")
else:
    print("✗ Section not found")