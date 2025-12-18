/**
 * UI Layout Analyzer and Simulator
 * 
 * A comprehensive tool for visualizing, testing, and debugging UI layouts.
 * 
 * Features:
 * - 2D ASCII visualization of UI elements
 * - Mouse click/movement simulation
 * - Keyboard input simulation
 * - Layout coordinate analysis
 * - Hit-testing verification
 * - Interactive debugging mode
 * - Export layout data for analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "math/math.h"

#define MAX_ELEMENTS 100
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 90
#define SCALE_X 10  // 1600px / 160 chars
#define SCALE_Y 10  // 900px / 90 chars

typedef enum {
    ELEM_PANEL,
    ELEM_BUTTON,
    ELEM_SLIDER,
    ELEM_INPUT,
    ELEM_TEXTAREA,
    ELEM_LABEL
} ElementType;

typedef struct {
    ElementType type;
    char name[64];
    
    // Position (CENTER coordinates)
    float center_x;
    float center_y;
    
    // Dimensions
    float width;
    float height;
    
    // Calculated bounds (TOP-LEFT coordinates)
    int x;
    int y;
    int w;
    int h;
    
    // State
    bool visible;
    bool enabled;
    bool hovered;
    bool active;
    
    // For sliders
    float value;
    float min_value;
    float max_value;
    
    // For inputs/textareas
    char text[256];
} UIElement;

typedef struct {
    UIElement elements[MAX_ELEMENTS];
    int element_count;
    
    // Screen buffer
    char screen[SCREEN_HEIGHT][SCREEN_WIDTH + 1];
    
    // Mouse state
    int mouse_x;
    int mouse_y;
    bool mouse_down;
    
    // Keyboard state
    char input_buffer[256];
    int input_focus;  // Index of focused element (-1 = none)
    
} UISimulator;

// Initialize simulator
void ui_sim_init(UISimulator* sim) {
    memset(sim, 0, sizeof(UISimulator));
    sim->input_focus = -1;
    
    // Clear screen
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        memset(sim->screen[y], ' ', SCREEN_WIDTH);
        sim->screen[y][SCREEN_WIDTH] = '\0';
    }
}

// Add element to simulator
int ui_sim_add_element(UISimulator* sim, ElementType type, const char* name,
                       float center_x, float center_y, float width, float height) {
    if (sim->element_count >= MAX_ELEMENTS) return -1;
    
    UIElement* elem = &sim->elements[sim->element_count];
    elem->type = type;
    strncpy(elem->name, name, sizeof(elem->name) - 1);
    
    elem->center_x = center_x;
    elem->center_y = center_y;
    elem->width = width;
    elem->height = height;
    
    // Calculate TOP-LEFT bounds
    elem->x = (int)(center_x - width / 2.0f);
    elem->y = (int)(center_y - height / 2.0f);
    elem->w = (int)width;
    elem->h = (int)height;
    
    elem->visible = true;
    elem->enabled = true;
    elem->hovered = false;
    elem->active = false;
    
    elem->value = 0.0f;
    elem->min_value = 0.0f;
    elem->max_value = 1.0f;
    elem->text[0] = '\0';
    
    return sim->element_count++;
}

// Check if point is inside element
bool ui_sim_point_in_element(UIElement* elem, int x, int y) {
    return (x >= elem->x && x < elem->x + elem->w &&
            y >= elem->y && y < elem->y + elem->h);
}

// Update hover states based on mouse position
void ui_sim_update_hover(UISimulator* sim) {
    for (int i = 0; i < sim->element_count; i++) {
        UIElement* elem = &sim->elements[i];
        elem->hovered = ui_sim_point_in_element(elem, sim->mouse_x, sim->mouse_y);
    }
}

// Handle mouse click
void ui_sim_mouse_click(UISimulator* sim, int x, int y) {
    sim->mouse_x = x;
    sim->mouse_y = y;
    sim->mouse_down = true;
    
    ui_sim_update_hover(sim);
    
    // Find clicked element
    for (int i = sim->element_count - 1; i >= 0; i--) {
        UIElement* elem = &sim->elements[i];
        if (elem->visible && elem->enabled && elem->hovered) {
            elem->active = true;
            
            // Set focus for inputs
            if (elem->type == ELEM_INPUT || elem->type == ELEM_TEXTAREA) {
                sim->input_focus = i;
            }
            
            printf("CLICK: Element '%s' at (%d, %d)\n", elem->name, x, y);
            printf("  Bounds: x=%d, y=%d, w=%d, h=%d\n", elem->x, elem->y, elem->w, elem->h);
            printf("  Center: cx=%.1f, cy=%.1f\n", elem->center_x, elem->center_y);
            break;
        }
    }
}

// Handle mouse release
void ui_sim_mouse_release(UISimulator* sim) {
    sim->mouse_down = false;
    
    for (int i = 0; i < sim->element_count; i++) {
        sim->elements[i].active = false;
    }
}

// Handle mouse movement
void ui_sim_mouse_move(UISimulator* sim, int x, int y) {
    sim->mouse_x = x;
    sim->mouse_y = y;
    ui_sim_update_hover(sim);
}

// Handle keyboard input
void ui_sim_key_input(UISimulator* sim, const char* text) {
    if (sim->input_focus >= 0 && sim->input_focus < sim->element_count) {
        UIElement* elem = &sim->elements[sim->input_focus];
        strncat(elem->text, text, sizeof(elem->text) - strlen(elem->text) - 1);
        printf("INPUT: '%s' -> Element '%s'\n", text, elem->name);
    }
}

// Draw element to screen buffer
void ui_sim_draw_element(UISimulator* sim, UIElement* elem) {
    if (!elem->visible) return;
    
    // Scale to screen coordinates
    int sx = elem->x / SCALE_X;
    int sy = elem->y / SCALE_Y;
    int sw = elem->w / SCALE_X;
    int sh = elem->h / SCALE_Y;
    
    // Clamp to screen bounds
    if (sx < 0) { sw += sx; sx = 0; }
    if (sy < 0) { sh += sy; sy = 0; }
    if (sx + sw > SCREEN_WIDTH) sw = SCREEN_WIDTH - sx;
    if (sy + sh > SCREEN_HEIGHT) sh = SCREEN_HEIGHT - sy;
    
    if (sw <= 0 || sh <= 0) return;
    
    // Choose character based on element type and state
    char ch = '?';
    switch (elem->type) {
        case ELEM_PANEL:     ch = elem->active ? '#' : (elem->hovered ? '+' : '.'); break;
        case ELEM_BUTTON:    ch = elem->active ? 'B' : (elem->hovered ? 'b' : 'o'); break;
        case ELEM_SLIDER:    ch = elem->active ? 'S' : (elem->hovered ? 's' : '-'); break;
        case ELEM_INPUT:     ch = elem->active ? 'I' : (elem->hovered ? 'i' : '_'); break;
        case ELEM_TEXTAREA:  ch = elem->active ? 'T' : (elem->hovered ? 't' : '~'); break;
        case ELEM_LABEL:     ch = 'L'; break;
    }
    
    // Draw border
    for (int x = sx; x < sx + sw; x++) {
        if (sy >= 0 && sy < SCREEN_HEIGHT) sim->screen[sy][x] = ch;
        if (sy + sh - 1 >= 0 && sy + sh - 1 < SCREEN_HEIGHT) sim->screen[sy + sh - 1][x] = ch;
    }
    for (int y = sy; y < sy + sh; y++) {
        if (y >= 0 && y < SCREEN_HEIGHT) {
            if (sx >= 0 && sx < SCREEN_WIDTH) sim->screen[y][sx] = ch;
            if (sx + sw - 1 >= 0 && sx + sw - 1 < SCREEN_WIDTH) sim->screen[y][sx + sw - 1] = ch;
        }
    }
    
    // Draw name in center
    int name_len = strlen(elem->name);
    int name_x = sx + (sw - name_len) / 2;
    int name_y = sy + sh / 2;
    
    if (name_y >= 0 && name_y < SCREEN_HEIGHT && name_x >= 0) {
        for (int i = 0; i < name_len && name_x + i < SCREEN_WIDTH; i++) {
            sim->screen[name_y][name_x + i] = elem->name[i];
        }
    }
}

// Render all elements
void ui_sim_render(UISimulator* sim) {
    // Clear screen
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        memset(sim->screen[y], ' ', SCREEN_WIDTH);
    }
    
    // Draw all elements
    for (int i = 0; i < sim->element_count; i++) {
        ui_sim_draw_element(sim, &sim->elements[i]);
    }
    
    // Draw mouse cursor
    int mx = sim->mouse_x / SCALE_X;
    int my = sim->mouse_y / SCALE_Y;
    if (mx >= 0 && mx < SCREEN_WIDTH && my >= 0 && my < SCREEN_HEIGHT) {
        sim->screen[my][mx] = sim->mouse_down ? 'X' : '*';
    }
}

// Print screen
void ui_sim_print(UISimulator* sim) {
    printf("\n");
    printf("╔");
    for (int i = 0; i < SCREEN_WIDTH; i++) printf("═");
    printf("╗\n");
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        printf("║%s║\n", sim->screen[y]);
    }
    
    printf("╚");
    for (int i = 0; i < SCREEN_WIDTH; i++) printf("═");
    printf("╝\n");
    
    printf("Mouse: (%d, %d) %s\n", sim->mouse_x, sim->mouse_y, 
           sim->mouse_down ? "[DOWN]" : "[UP]");
    if (sim->input_focus >= 0) {
        printf("Focus: %s\n", sim->elements[sim->input_focus].name);
    }
}

// Print element details
void ui_sim_print_elements(UISimulator* sim) {
    printf("\n=== UI ELEMENTS ===\n");
    for (int i = 0; i < sim->element_count; i++) {
        UIElement* elem = &sim->elements[i];
        printf("[%2d] %-20s ", i, elem->name);
        printf("Center:(%.1f, %.1f) ", elem->center_x, elem->center_y);
        printf("Size:(%.1f x %.1f) ", elem->width, elem->height);
        printf("Bounds:(%d, %d, %d, %d) ", elem->x, elem->y, elem->w, elem->h);
        printf("%s%s%s\n",
               elem->hovered ? "[HOVER]" : "",
               elem->active ? "[ACTIVE]" : "",
               !elem->visible ? "[HIDDEN]" : "");
    }
}

// Export layout data
void ui_sim_export(UISimulator* sim, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("ERROR: Cannot open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "# UI Layout Export\n");
    fprintf(f, "# Screen: %dx%d (scaled from 1600x900)\n\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    
    for (int i = 0; i < sim->element_count; i++) {
        UIElement* elem = &sim->elements[i];
        fprintf(f, "Element: %s\n", elem->name);
        fprintf(f, "  Type: %d\n", elem->type);
        fprintf(f, "  Center: (%.1f, %.1f)\n", elem->center_x, elem->center_y);
        fprintf(f, "  Size: %.1f x %.1f\n", elem->width, elem->height);
        fprintf(f, "  Bounds: (%d, %d) to (%d, %d)\n", 
                elem->x, elem->y, elem->x + elem->w, elem->y + elem->h);
        fprintf(f, "  Visible: %s, Enabled: %s\n", 
                elem->visible ? "yes" : "no", elem->enabled ? "yes" : "no");
        fprintf(f, "\n");
    }
    
    fclose(f);
    printf("Layout exported to %s\n", filename);
}

// Simulate LLM tab layout
void simulate_llm_tab(UISimulator* sim) {
    // Constants from app_common.h
    int WINDOW_WIDTH = 1600;
    int WINDOW_HEIGHT = 900;
    int SIDEBAR_WIDTH = 200;
    int CONTROL_PANEL_WIDTH = 320;
    int SUBMENU_HEIGHT = 40;
    
    int RENDER_WIDTH = WINDOW_WIDTH - CONTROL_PANEL_WIDTH - SIDEBAR_WIDTH;  // 1080
    int RENDER_OFFSET_X = SIDEBAR_WIDTH;  // 200
    int RENDER_OFFSET_Y = SUBMENU_HEIGHT;  // 40
    
    // LLM tab layout calculations
    int content_width = RENDER_WIDTH;  // 1080px
    int chat_width = (int)(content_width * 0.75f);     // 810px (75%)
    int control_width = content_width - chat_width;     // 270px (25%)
    
    int available_height = WINDOW_HEIGHT - RENDER_OFFSET_Y - 20;  // 840px
    
    // Chat area
    int chat_x = RENDER_OFFSET_X + 10;  // 210
    int chat_y = RENDER_OFFSET_Y + 10;  // 50
    int chat_w = chat_width - 20;       // 790
    int chat_h = available_height - 110; // 730
    
    float chat_center_x = chat_x + chat_w / 2.0f;  // 605
    float chat_center_y = chat_y + chat_h / 2.0f;  // 415
    
    ui_sim_add_element(sim, ELEM_TEXTAREA, "ChatArea", 
                       chat_center_x, chat_center_y, chat_w, chat_h);
    
    // Input field
    int input_x = RENDER_OFFSET_X + 10;  // 210
    int input_y = RENDER_OFFSET_Y + available_height - 90;  // 790
    int input_w = chat_width - 130;      // 680
    int input_h = 80;
    
    float input_center_x = input_x + input_w / 2.0f;  // 550
    float input_center_y = input_y + input_h / 2.0f;  // 830
    
    ui_sim_add_element(sim, ELEM_INPUT, "MessageInput",
                       input_center_x, input_center_y, input_w, input_h);
    
    // Send button
    int send_x = RENDER_OFFSET_X + chat_width - 110;  // 900
    int send_w = 100;
    
    float send_center_x = send_x + send_w / 2.0f;  // 950
    float send_center_y = input_center_y;           // 830
    
    ui_sim_add_element(sim, ELEM_BUTTON, "SendBtn",
                       send_center_x, send_center_y, send_w, input_h);
    
    // Control panel
    int ctrl_x = RENDER_OFFSET_X + chat_width + 10;  // 1020
    int ctrl_w = control_width - 20;                  // 250
    int ctrl_y = RENDER_OFFSET_Y + 10;                // 50
    float slider_center_x = ctrl_x + ctrl_w / 2.0f;  // 1145
    
    // Temperature slider
    ui_sim_add_element(sim, ELEM_SLIDER, "Temperature",
                       slider_center_x, ctrl_y, ctrl_w, 30);
    ctrl_y += 70;
    
    // Max tokens slider
    ui_sim_add_element(sim, ELEM_SLIDER, "MaxTokens",
                       slider_center_x, ctrl_y, ctrl_w, 30);
    ctrl_y += 70;
    
    // Top-K slider
    ui_sim_add_element(sim, ELEM_SLIDER, "TopK",
                       slider_center_x, ctrl_y, ctrl_w, 30);
    ctrl_y += 70;
    
    // Top-P slider
    ui_sim_add_element(sim, ELEM_SLIDER, "TopP",
                       slider_center_x, ctrl_y, ctrl_w, 30);
    ctrl_y += 90;
    
    // Browse Models button
    ui_sim_add_element(sim, ELEM_BUTTON, "BrowseModels",
                       slider_center_x, ctrl_y, ctrl_w, 40);
    ctrl_y += 60;
    
    // New Thread button
    ui_sim_add_element(sim, ELEM_BUTTON, "NewThread",
                       slider_center_x, ctrl_y, ctrl_w, 40);
    ctrl_y += 60;
    
    // Clear button
    ui_sim_add_element(sim, ELEM_BUTTON, "ClearChat",
                       slider_center_x, ctrl_y, ctrl_w, 40);
    
    printf("=== LLM TAB LAYOUT SIMULATION ===\n");
    printf("Window: %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    printf("Render Area: %dx%d at (%d, %d)\n", 
           RENDER_WIDTH, available_height, RENDER_OFFSET_X, RENDER_OFFSET_Y);
    printf("Chat Width: %d (75%%)\n", chat_width);
    printf("Control Width: %d (25%%)\n", control_width);
}

int main(int argc, char** argv) {
    UISimulator sim;
    ui_sim_init(&sim);
    
    // Simulate LLM tab
    simulate_llm_tab(&sim);
    
    // Print element details
    ui_sim_print_elements(&sim);
    
    // Render and display
    ui_sim_render(&sim);
    ui_sim_print(&sim);
    
    // Export layout
    ui_sim_export(&sim, "llm_tab_layout.txt");
    
    // Interactive mode
    if (argc > 1 && strcmp(argv[1], "-i") == 0) {
        printf("\n=== INTERACTIVE MODE ===\n");
        printf("Commands:\n");
        printf("  c X Y    - Click at (X, Y)\n");
        printf("  m X Y    - Move mouse to (X, Y)\n");
        printf("  r        - Release mouse\n");
        printf("  k TEXT   - Type text\n");
        printf("  d        - Display screen\n");
        printf("  l        - List elements\n");
        printf("  e FILE   - Export layout\n");
        printf("  q        - Quit\n\n");
        
        char cmd[256];
        while (1) {
            printf("> ");
            if (!fgets(cmd, sizeof(cmd), stdin)) break;
            
            if (cmd[0] == 'q') break;
            
            if (cmd[0] == 'c') {
                int x, y;
                if (sscanf(cmd + 1, "%d %d", &x, &y) == 2) {
                    ui_sim_mouse_click(&sim, x, y);
                    ui_sim_render(&sim);
                    ui_sim_print(&sim);
                }
            } else if (cmd[0] == 'm') {
                int x, y;
                if (sscanf(cmd + 1, "%d %d", &x, &y) == 2) {
                    ui_sim_mouse_move(&sim, x, y);
                    ui_sim_render(&sim);
                    ui_sim_print(&sim);
                }
            } else if (cmd[0] == 'r') {
                ui_sim_mouse_release(&sim);
                ui_sim_render(&sim);
                ui_sim_print(&sim);
            } else if (cmd[0] == 'k') {
                ui_sim_key_input(&sim, cmd + 2);
            } else if (cmd[0] == 'd') {
                ui_sim_render(&sim);
                ui_sim_print(&sim);
            } else if (cmd[0] == 'l') {
                ui_sim_print_elements(&sim);
            } else if (cmd[0] == 'e') {
                char filename[256];
                if (sscanf(cmd + 1, "%s", filename) == 1) {
                    ui_sim_export(&sim, filename);
                }
            }
        }
    }
    
    return 0;
}