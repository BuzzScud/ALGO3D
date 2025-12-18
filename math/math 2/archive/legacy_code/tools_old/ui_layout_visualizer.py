#!/usr/bin/env python3
"""
UI Layout Visualizer - High-resolution pixel-perfect visualization
Uses matplotlib to create actual graphical output of UI layouts
"""

import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.patches import Rectangle
import numpy as np

class UIElement:
    def __init__(self, name, elem_type, center_x, center_y, width, height):
        self.name = name
        self.type = elem_type
        self.center_x = center_x
        self.center_y = center_y
        self.width = width
        self.height = height
        
        # Calculate TOP-LEFT bounds
        self.x = center_x - width / 2
        self.y = center_y - height / 2
        
    def get_bounds(self):
        return (self.x, self.y, self.width, self.height)

class UIVisualizer:
    def __init__(self, width=1600, height=900):
        self.width = width
        self.height = height
        self.elements = []
        
        # Create figure with exact pixel dimensions
        dpi = 100
        self.fig, self.ax = plt.subplots(figsize=(width/dpi, height/dpi), dpi=dpi)
        self.ax.set_xlim(0, width)
        self.ax.set_ylim(0, height)
        self.ax.invert_yaxis()  # Y increases downward like screen coordinates
        self.ax.set_aspect('equal')
        
        # Add grid
        self.ax.grid(True, alpha=0.3, linewidth=0.5)
        self.ax.set_axisbelow(True)
        
    def add_element(self, name, elem_type, center_x, center_y, width, height):
        elem = UIElement(name, elem_type, center_x, center_y, width, height)
        self.elements.append(elem)
        return elem
        
    def draw(self):
        # Color scheme
        colors = {
            'panel': '#1a1a2e',
            'button': '#4a5568',
            'slider': '#2d3748',
            'input': '#2c3e50',
            'textarea': '#16213e',
            'border': '#4a5568',
            'text': '#ffffff',
            'hover': '#667eea',
            'active': '#764ba2'
        }
        
        # Draw window background
        bg = Rectangle((0, 0), self.width, self.height, 
                      facecolor='#0f0f1e', edgecolor='none')
        self.ax.add_patch(bg)
        
        # Draw sidebar
        sidebar = Rectangle((0, 0), 200, self.height,
                           facecolor='#16213e', edgecolor='#2d3748', linewidth=2)
        self.ax.add_patch(sidebar)
        self.ax.text(100, 50, 'SIDEBAR', ha='center', va='center',
                    color='white', fontsize=10, weight='bold')
        
        # Draw submenu
        submenu = Rectangle((200, 0), 1400, 40,
                           facecolor='#1a1a2e', edgecolor='#2d3748', linewidth=2)
        self.ax.add_patch(submenu)
        self.ax.text(900, 20, 'LLM Chat | Training | Research | Crawler', 
                    ha='center', va='center', color='white', fontsize=9)
        
        # Draw control panel
        control_panel = Rectangle((1280, 40), 320, 860,
                                 facecolor='#16213e', edgecolor='#2d3748', linewidth=2)
        self.ax.add_patch(control_panel)
        self.ax.text(1440, 60, 'CONTROL PANEL', ha='center', va='center',
                    color='white', fontsize=8, weight='bold')
        
        # Draw each element
        for elem in self.elements:
            x, y, w, h = elem.get_bounds()
            
            # Choose color based on type
            if 'button' in elem.type.lower() or elem.type == 'button':
                color = colors['button']
            elif 'slider' in elem.type.lower() or elem.type == 'slider':
                color = colors['slider']
            elif 'input' in elem.type.lower() or elem.type == 'input':
                color = colors['input']
            elif 'textarea' in elem.type.lower() or elem.type == 'textarea':
                color = colors['textarea']
            else:
                color = colors['panel']
            
            # Draw element rectangle
            rect = Rectangle((x, y), w, h,
                           facecolor=color, edgecolor=colors['border'], 
                           linewidth=2, alpha=0.9)
            self.ax.add_patch(rect)
            
            # Draw element name
            self.ax.text(elem.center_x, elem.center_y, elem.name,
                        ha='center', va='center', color=colors['text'],
                        fontsize=8, weight='bold')
            
            # Draw center point
            self.ax.plot(elem.center_x, elem.center_y, 'r+', markersize=8, markeredgewidth=1)
            
            # Add coordinate labels
            coord_text = f"({int(x)}, {int(y)})\n{int(w)}x{int(h)}"
            self.ax.text(x + 5, y + 5, coord_text,
                        fontsize=6, color='yellow', va='top')
        
        # Add measurement lines for gaps
        self.add_measurements()
        
        # Add title
        self.ax.set_title('LLM Tab Layout - Pixel Perfect Visualization', 
                         fontsize=14, weight='bold', color='white', pad=20)
        
        # Style
        self.ax.set_facecolor('#0a0a0f')
        self.fig.patch.set_facecolor('#0a0a0f')
        self.ax.tick_params(colors='white')
        for spine in self.ax.spines.values():
            spine.set_edgecolor('white')
            spine.set_linewidth(0.5)
        
    def add_measurements(self):
        """Add measurement lines showing gaps and spacing"""
        # Find rightmost element
        if not self.elements:
            return
            
        max_right = max(elem.x + elem.width for elem in self.elements)
        
        # Draw line showing gap to control panel
        control_panel_x = 1280
        if max_right < control_panel_x:
            gap = control_panel_x - max_right
            mid_y = 450
            
            # Draw measurement line
            self.ax.plot([max_right, control_panel_x], [mid_y, mid_y],
                        'r-', linewidth=2, alpha=0.7)
            self.ax.plot([max_right, max_right], [mid_y-10, mid_y+10],
                        'r-', linewidth=2, alpha=0.7)
            self.ax.plot([control_panel_x, control_panel_x], [mid_y-10, mid_y+10],
                        'r-', linewidth=2, alpha=0.7)
            
            # Add gap label
            self.ax.text((max_right + control_panel_x) / 2, mid_y - 20,
                        f'GAP: {int(gap)}px', ha='center', va='bottom',
                        color='red', fontsize=10, weight='bold',
                        bbox=dict(boxstyle='round', facecolor='black', alpha=0.8))
        
        # Find bottom-most element
        max_bottom = max(elem.y + elem.height for elem in self.elements)
        window_bottom = self.height
        
        if max_bottom < window_bottom - 50:
            gap = window_bottom - max_bottom
            mid_x = 900
            
            # Draw measurement line
            self.ax.plot([mid_x, mid_x], [max_bottom, window_bottom],
                        'r-', linewidth=2, alpha=0.7)
            self.ax.plot([mid_x-10, mid_x+10], [max_bottom, max_bottom],
                        'r-', linewidth=2, alpha=0.7)
            self.ax.plot([mid_x-10, mid_x+10], [window_bottom, window_bottom],
                        'r-', linewidth=2, alpha=0.7)
            
            # Add gap label
            self.ax.text(mid_x + 20, (max_bottom + window_bottom) / 2,
                        f'GAP: {int(gap)}px', ha='left', va='center',
                        color='red', fontsize=10, weight='bold',
                        bbox=dict(boxstyle='round', facecolor='black', alpha=0.8))
    
    def save(self, filename='llm_tab_layout.png'):
        plt.tight_layout()
        plt.savefig(filename, dpi=100, facecolor='#0a0a0f', edgecolor='none')
        print(f"Saved visualization to {filename}")
        
    def show(self):
        plt.tight_layout()
        plt.show()
        
    def print_analysis(self):
        """Print detailed analysis of layout"""
        print("\n" + "="*80)
        print("UI LAYOUT ANALYSIS")
        print("="*80)
        
        print(f"\nWindow: {self.width}x{self.height}")
        print(f"Sidebar: 0-200px")
        print(f"Submenu: 0-40px (height)")
        print(f"Control Panel: 1280-1600px")
        print(f"Render Area: 200-1280px (width), 40-900px (height)")
        print(f"Render Area Size: 1080x860px")
        
        print(f"\n{'Element':<20} {'Center':<15} {'Size':<12} {'Bounds':<25}")
        print("-" * 80)
        
        for elem in self.elements:
            center = f"({int(elem.center_x)}, {int(elem.center_y)})"
            size = f"{int(elem.width)}x{int(elem.height)}"
            bounds = f"({int(elem.x)}, {int(elem.y)}) to ({int(elem.x + elem.width)}, {int(elem.y + elem.height)})"
            print(f"{elem.name:<20} {center:<15} {size:<12} {bounds:<25}")
        
        # Check for gaps
        print("\n" + "="*80)
        print("GAP ANALYSIS")
        print("="*80)
        
        # Right side gap
        max_right = max(elem.x + elem.width for elem in self.elements)
        control_panel_x = 1280
        right_gap = control_panel_x - max_right
        print(f"\nRight side gap: {int(right_gap)}px")
        print(f"  Rightmost element ends at: {int(max_right)}px")
        print(f"  Control panel starts at: {control_panel_x}px")
        if right_gap > 20:
            print(f"  ⚠️  WARNING: Excessive gap! Should be ~10px")
        
        # Bottom gap
        max_bottom = max(elem.y + elem.height for elem in self.elements)
        window_bottom = self.height
        bottom_gap = window_bottom - max_bottom
        print(f"\nBottom gap: {int(bottom_gap)}px")
        print(f"  Bottom-most element ends at: {int(max_bottom)}px")
        print(f"  Window bottom at: {window_bottom}px")
        if bottom_gap > 30:
            print(f"  ⚠️  WARNING: Excessive gap!")

def simulate_llm_tab():
    """Simulate the current LLM tab layout"""
    
    # Constants
    WINDOW_WIDTH = 1600
    WINDOW_HEIGHT = 900
    SIDEBAR_WIDTH = 200
    CONTROL_PANEL_WIDTH = 320
    SUBMENU_HEIGHT = 40
    
    RENDER_WIDTH = WINDOW_WIDTH - CONTROL_PANEL_WIDTH - SIDEBAR_WIDTH  # 1080
    RENDER_OFFSET_X = SIDEBAR_WIDTH  # 200
    RENDER_OFFSET_Y = SUBMENU_HEIGHT  # 40
    
    # Create visualizer
    viz = UIVisualizer(WINDOW_WIDTH, WINDOW_HEIGHT)
    
    # LLM tab layout - Chat uses RENDER area, control uses CONTROL_PANEL area
    chat_width = RENDER_WIDTH - 10  # 1070 (full render width minus padding)
    control_width = CONTROL_PANEL_WIDTH - 20  # 300 (control panel minus padding)
    
    available_height = WINDOW_HEIGHT - RENDER_OFFSET_Y - 20  # 840
    
    # Chat area
    chat_x = RENDER_OFFSET_X + 10  # 210
    chat_y = RENDER_OFFSET_Y + 10  # 50
    chat_w = chat_width - 20  # 790
    chat_h = available_height - 110  # 730
    
    chat_center_x = chat_x + chat_w / 2
    chat_center_y = chat_y + chat_h / 2
    
    viz.add_element("ChatArea", "textarea", chat_center_x, chat_center_y, chat_w, chat_h)
    
    # Input field
    input_x = RENDER_OFFSET_X + 10
    input_y = RENDER_OFFSET_Y + available_height - 90
    input_w = chat_width - 130
    input_h = 80
    
    input_center_x = input_x + input_w / 2
    input_center_y = input_y + input_h / 2
    
    viz.add_element("MessageInput", "input", input_center_x, input_center_y, input_w, input_h)
    
    # Send button
    send_x = RENDER_OFFSET_X + chat_width - 110
    send_w = 100
    
    send_center_x = send_x + send_w / 2
    send_center_y = input_center_y
    
    viz.add_element("SendBtn", "button", send_center_x, send_center_y, send_w, input_h)
    
    # Control panel - Position in ACTUAL control panel area (1280-1600)
    ctrl_x = WINDOW_WIDTH - CONTROL_PANEL_WIDTH + 10  # 1290
    ctrl_w = control_width  # 300
    ctrl_y = RENDER_OFFSET_Y + 10
    slider_center_x = ctrl_x + ctrl_w / 2  # 1440
    
    # Temperature slider
    viz.add_element("Temperature", "slider", slider_center_x, ctrl_y, ctrl_w, 30)
    ctrl_y += 70
    
    # Max tokens slider
    viz.add_element("MaxTokens", "slider", slider_center_x, ctrl_y, ctrl_w, 30)
    ctrl_y += 70
    
    # Top-K slider
    viz.add_element("TopK", "slider", slider_center_x, ctrl_y, ctrl_w, 30)
    ctrl_y += 70
    
    # Top-P slider
    viz.add_element("TopP", "slider", slider_center_x, ctrl_y, ctrl_w, 30)
    ctrl_y += 90
    
    # Browse Models button
    viz.add_element("BrowseModels", "button", slider_center_x, ctrl_y, ctrl_w, 40)
    ctrl_y += 60
    
    # New Thread button
    viz.add_element("NewThread", "button", slider_center_x, ctrl_y, ctrl_w, 40)
    ctrl_y += 60
    
    # Clear button
    viz.add_element("ClearChat", "button", slider_center_x, ctrl_y, ctrl_w, 40)
    
    return viz

if __name__ == "__main__":
    viz = simulate_llm_tab()
    viz.draw()
    viz.print_analysis()
    viz.save('llm_tab_layout.png')
    print("\n✓ Visualization saved to llm_tab_layout.png")
    print("✓ Open the image to see pixel-perfect layout")