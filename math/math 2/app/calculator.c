// app/calculator_fixed.c - Complete calculator implementation with mini-map
// This replaces calculator.c with full functionality

#include "app_common.h"
#include <string.h>
#include <ctype.h>

// Calculator state helpers
void calc_clear(AppState* state) {
    state->calc_input[0] = '\0';
    state->calc_value1 = 0;
    state->calc_value2 = 0;
    state->calc_result = 0;
    strcpy(state->calc_operation, "");
    state->calc_has_value1 = false;
}

void calc_clear_entry(AppState* state) {
    state->calc_input[0] = '\0';
}

void calc_append_digit(AppState* state, char digit) {
    int len = strlen(state->calc_input);
    if (len < 63) {
        state->calc_input[len] = digit;
        state->calc_input[len + 1] = '\0';
    }
}

void calc_set_operation(AppState* state, const char* op) {
    if (state->calc_input[0] != '\0') {
        state->calc_value1 = atof(state->calc_input);
        state->calc_has_value1 = true;
        state->calc_input[0] = '\0';
    }
    strcpy(state->calc_operation, op);
}

void calc_equals(AppState* state) {
    if (!state->calc_has_value1 || state->calc_input[0] == '\0') return;
    
    state->calc_value2 = atof(state->calc_input);
    
    // Perform calculation using prime math functions
    if (strcmp(state->calc_operation, "+") == 0) {
        state->calc_result = state->calc_value1 + state->calc_value2;
    } else if (strcmp(state->calc_operation, "-") == 0) {
        state->calc_result = state->calc_value1 - state->calc_value2;
    } else if (strcmp(state->calc_operation, "*") == 0) {
        state->calc_result = state->calc_value1 * state->calc_value2;
    } else if (strcmp(state->calc_operation, "/") == 0) {
        state->calc_result = state->calc_value1 / state->calc_value2;
    }
    
    // Auto-analyze the result if it's an integer
    if (state->calc_result == (int)state->calc_result && state->calc_result > 0) {
        analyze_number(state, (int)state->calc_result);
        add_to_analysis_history(state, (int)state->calc_result);
    }
    
    // Show result in display
    snprintf(state->calc_input, sizeof(state->calc_input), "%.2f", state->calc_result);
    state->calc_has_value1 = false;
}

void calc_analyze_current(AppState* state) {
    if (state->calc_input[0] != '\0') {
        double val = atof(state->calc_input);
        if (val == (int)val && val > 0) {
            analyze_number(state, (int)val);
            add_to_analysis_history(state, (int)val);
        }
    }
}

// Handle calculator button click
void handle_calculator_button(AppState* state, int button_idx) {
    const char* buttons[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "=", "+",
        "C", "CE", "Analyze", ""
    };
    
    if (button_idx >= 19 || buttons[button_idx][0] == '\0') return;
    
    const char* btn = buttons[button_idx];
    
    // Set button as pressed for visual feedback
    state->calc_button_pressed = button_idx;
    state->calc_button_press_time = SDL_GetTicks();
    
    printf("Calculator button '%s' pressed\n", btn);
    
    // Handle different button types
    if (strcmp(btn, "C") == 0) {
        calc_clear(state);
    } else if (strcmp(btn, "CE") == 0) {
        calc_clear_entry(state);
    } else if (strcmp(btn, "=") == 0) {
        calc_equals(state);
    } else if (strcmp(btn, "Analyze") == 0) {
        calc_analyze_current(state);
    } else if (strcmp(btn, "+") == 0 || strcmp(btn, "-") == 0 || 
               strcmp(btn, "*") == 0 || strcmp(btn, "/") == 0) {
        calc_set_operation(state, btn);
    } else if (isdigit(btn[0]) || btn[0] == '.') {
        calc_append_digit(state, btn[0]);
    }
}

// Handle mode selector button click
void handle_mode_button(AppState* state, int mode_idx) {
    if (mode_idx >= 0 && mode_idx < MODE_COUNT) {
        // Set button as pressed for visual feedback
        state->mode_button_pressed = mode_idx;
        state->mode_button_press_time = SDL_GetTicks();
        
        state->mode = (VisualizationMode)mode_idx;
        printf("Switched to mode %d\n", mode_idx);
    }
}

// Handle mini-map click (select analyzed numbers)
void handle_minimap_click(AppState* state, int click_x, int click_y, int map_x, int map_y, int map_width, int map_height) {
    if (state->analysis_history.count == 0) return;
    
    // Calculate scaling factor
    double scale_x = (map_width > 0 && RENDER_WIDTH > 0) ? (double)map_width / RENDER_WIDTH : 1.0;
    double scale_y = (map_height > 0 && WINDOW_HEIGHT > 0) ? (double)map_height / WINDOW_HEIGHT : 1.0;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale <= 0) scale = 1.0;
    
    // Center point in mini-map
    int cx = map_x + map_width / 2;
    int cy = map_y + map_height / 2;
    
    // Check each analyzed number
    for (int i = 0; i < state->analysis_history.count; i++) {
        int num = state->analysis_history.numbers[i];
        
        // Get position in mini-map
        double angle = get_exact_angle(num);
        double radius = get_radial_distance(num, state->zoom) * scale;
        
        int num_x = cx + (int)(radius * math_cos(angle));
        int num_y = cy + (int)(radius * math_sin(angle));
        
        // Check if click is within 10 pixels of this number
        int dx = click_x - num_x;
        int dy = click_y - num_y;
        int dist_sq = dx * dx + dy * dy;
        
        if (dist_sq <= 100) {  // 10 pixel radius
            // Select this number
            state->analysis_history.selected_index = i;
            // Re-analyze this number to update the overlay
            analyze_number(state, num);
            printf("Selected analyzed number from mini-map: %d (index %d)\n", num, i);
            return;
        }
    }
}

// Draw mini-map visualization (scaled down version)
void draw_minimap_visualization(SDL_Renderer* renderer, AppState* state, int x, int y, int width, int height) {
    // Set clip rect for mini-map area
    SDL_Rect clip = {x, y, width, height};
    SDL_RenderSetClipRect(renderer, &clip);
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, 15, 20, 30, 255);
    SDL_RenderFillRect(renderer, &clip);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
    SDL_RenderDrawRect(renderer, &clip);
    
    // Calculate scaling factor
    double scale_x = (width > 0 && RENDER_WIDTH > 0) ? (double)width / RENDER_WIDTH : 1.0;
    double scale_y = (height > 0 && WINDOW_HEIGHT > 0) ? (double)height / WINDOW_HEIGHT : 1.0;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    
    // Safety check
    if (scale <= 0) scale = 1.0;
    
    // Center point in mini-map
    int cx = x + width / 2;
    int cy = y + height / 2;
    
    // Draw based on current mode
    if (state->mode == MODE_CLOCK_SUDOKU) {
        // Draw simplified clock sudoku
        // 3 rings
        int ring_radii[] = {50, 100, 150};
        SDL_Color ring_colors[] = {
            {100, 150, 255, 255},
            {100, 255, 150, 255},
            {255, 150, 100, 255}
        };
        
        for (int i = 0; i < 3; i++) {
            SDL_SetRenderDrawColor(renderer, ring_colors[i].r, ring_colors[i].g, ring_colors[i].b, 255);
            int r = (int)(ring_radii[i] * scale * state->zoom);
            
            // Draw circle
            for (int angle = 0; angle < 360; angle += 5) {
                double rad = angle * MATH_PI / 180.0;
                int px = cx + (int)(r * math_cos(rad));
                int py = cy + (int)(r * math_sin(rad));
                if (px >= x && px < x + width && py >= y && py < y + height) {
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }
        
        // Draw 12 spokes
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        for (int i = 0; i < 12; i++) {
            double angle = i * 30.0 * MATH_PI / 180.0;
            int x2 = cx + (int)(200 * scale * state->zoom * math_cos(angle));
            int y2 = cy + (int)(200 * scale * state->zoom * math_sin(angle));
            SDL_RenderDrawLine(renderer, cx, cy, x2, y2);
        }
    } else if (state->mode == MODE_NATURAL_PRIME_LATTICE) {
        // Natural lattice - just primes, no overlay
        // Will be drawn below with analyzed numbers
    } else {
        // For other modes, draw mode-specific patterns
        SDL_SetRenderDrawColor(renderer, 80, 100, 120, 255);
        
        switch (state->mode) {
            case MODE_ULAM_SPIRAL:
                // Ulam spiral pattern
                for (int i = 0; i < 200; i++) {
                    double angle = i * 0.5;
                    double r = i * 0.5 * scale * state->zoom;
                    int px = cx + (int)(r * math_cos(angle));
                    int py = cy + (int)(r * math_sin(angle));
                    if (px >= x && px < x + width && py >= y && py < y + height) {
                        SDL_RenderDrawPoint(renderer, px, py);
                    }
                }
                break;
                
            case MODE_RECURSIVE_FRACTAL:
                // Recursive circles
                for (int level = 1; level <= 3; level++) {
                    int r = (int)(50 * level * scale * state->zoom);
                    for (int angle = 0; angle < 360; angle += 10) {
                        double rad = angle * MATH_PI / 180.0;
                        int px = cx + (int)(r * math_cos(rad));
                        int py = cy + (int)(r * math_sin(rad));
                        if (px >= x && px < x + width && py >= y && py < y + height) {
                            SDL_RenderDrawPoint(renderer, px, py);
                        }
                    }
                }
                break;
                
            case MODE_HARMONIC_RESONANCE:
                // Wave pattern
                for (int i = 0; i < 360; i += 5) {
                    double angle = i * MATH_PI / 180.0;
                    double r = 100 * scale * state->zoom * (1.0 + 0.3 * math_sin(angle * 3));
                    int px = cx + (int)(r * math_cos(angle));
                    int py = cy + (int)(r * math_sin(angle));
                    if (px >= x && px < x + width && py >= y && py < y + height) {
                        SDL_RenderDrawPoint(renderer, px, py);
                    }
                }
                break;
                
            case MODE_HYPERDIM_PROJECTION:
                // 3D cube projection
                for (int i = -1; i <= 1; i += 2) {
                    for (int j = -1; j <= 1; j += 2) {
                        int r = (int)(80 * scale * state->zoom);
                        int px = cx + i * r;
                        int py = cy + j * r;
                        if (px >= x && px < x + width && py >= y && py < y + height) {
                            SDL_RenderDrawPoint(renderer, px, py);
                        }
                    }
                }
                break;
                
            case MODE_SELF_SIMILAR_LAYERS:
                // Concentric circles
                for (int i = 1; i <= 5; i++) {
                    int r = (int)(30 * i * scale * state->zoom);
                    for (int angle = 0; angle < 360; angle += 10) {
                        double rad = angle * MATH_PI / 180.0;
                        int px = cx + (int)(r * math_cos(rad));
                        int py = cy + (int)(r * math_sin(rad));
                        if (px >= x && px < x + width && py >= y && py < y + height) {
                            SDL_RenderDrawPoint(renderer, px, py);
                        }
                    }
                }
                break;
                
            case MODE_FOLDED_ORIGAMI:
                // Triangle pattern
                for (int i = 0; i < 3; i++) {
                    double angle = i * 120.0 * MATH_PI / 180.0;
                    int r = (int)(120 * scale * state->zoom);
                    int px = cx + (int)(r * math_cos(angle));
                    int py = cy + (int)(r * math_sin(angle));
                    SDL_RenderDrawLine(renderer, cx, cy, px, py);
                }
                break;
                
            default:
                break;
        }
    }
    
    // Draw analyzed numbers (for ALL modes)
    for (int h = 0; h < state->analysis_history.count; h++) {
        int num = state->analysis_history.numbers[h];
        SDL_Color color = state->analysis_history.colors[h];
        bool is_selected = (h == state->analysis_history.selected_index);
        
        double angle = get_exact_angle(num);
        double radius = get_radial_distance(num, state->zoom) * scale;
        
        int px = cx + (int)(radius * math_cos(angle));
        int py = cy + (int)(radius * math_sin(angle));
        
        // Draw small dot (larger if selected)
        int dot_size = is_selected ? 3 : 2;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        for (int dy = -dot_size; dy <= dot_size; dy++) {
            for (int dx = -dot_size; dx <= dot_size; dx++) {
                if (dx*dx + dy*dy <= dot_size*dot_size) {
                    int dpx = px + dx;
                    int dpy = py + dy;
                    if (dpx >= x && dpx < x + width && dpy >= y && dpy < y + height) {
                        SDL_RenderDrawPoint(renderer, dpx, dpy);
                    }
                }
            }
        }
        
        // Draw white outline if selected
        if (is_selected) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int angle = 0; angle < 360; angle += 30) {
                double rad = angle * MATH_PI / 180.0;
                int ox = px + (int)(4 * math_cos(rad));
                int oy = py + (int)(4 * math_sin(rad));
                if (ox >= x && ox < x + width && oy >= y && oy < y + height) {
                    SDL_RenderDrawPoint(renderer, ox, oy);
                }
            }
        }
    }
    
    // Draw factor lines if enabled
    if (state->show_factors && state->analysis_history.count > 0) {
        // Determine which numbers to show factors for
        int start_idx = 0;
        int end_idx = state->analysis_history.count;
        
        if (state->analysis_history.selected_index >= 0) {
            // Show only selected number's factors
            start_idx = state->analysis_history.selected_index;
            end_idx = start_idx + 1;
        }
        
        // Draw factors for each number in range
        for (int h = start_idx; h < end_idx; h++) {
            int num = state->analysis_history.numbers[h];
            SDL_Color color = state->analysis_history.colors[h];
            
            // Get number position
            double num_angle = get_exact_angle(num);
            double num_radius = get_radial_distance(num, state->zoom) * scale;
            int num_x = cx + (int)(num_radius * math_cos(num_angle));
            int num_y = cy + (int)(num_radius * math_sin(num_angle));
            
            // Find factors
            for (int i = 2; i < num; i++) {
                if (num % i == 0) {
                    // This is a factor - draw line to it
                    double factor_angle = get_exact_angle(i);
                    double factor_radius = get_radial_distance(i, state->zoom) * scale;
                    int factor_x = cx + (int)(factor_radius * math_cos(factor_angle));
                    int factor_y = cy + (int)(factor_radius * math_sin(factor_angle));
                    
                    // Draw thin line
                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 100);
                    SDL_RenderDrawLine(renderer, num_x, num_y, factor_x, factor_y);
                }
            }
        }
    }
    
    // Clear clip rect
    SDL_RenderSetClipRect(renderer, NULL);
}

// Main calculator drawing function with mini-map
void draw_calculator_with_minimap(SDL_Renderer* renderer, AppState* state) {
    // Layout:
    // Top: Mini-map visualization (centered in render area)
    // Bottom: Calculator UI (centered in render area)
    
    int minimap_height = 300;
    int minimap_width = RENDER_WIDTH - 100;
    int calc_width = 800;
    
    // Center minimap in render area
    int minimap_x = RENDER_OFFSET_X + (RENDER_WIDTH - minimap_width) / 2;
    int minimap_y = RENDER_OFFSET_Y + 10;
    
    // Center calculator in render area
    int x_base = RENDER_OFFSET_X + (RENDER_WIDTH - calc_width) / 2;
    int calc_start_y = minimap_y + minimap_height + 50;
    int y_pos = calc_start_y;
    
    // Draw mini-map at top (centered)
    draw_minimap_visualization(renderer, state, minimap_x, minimap_y, minimap_width, minimap_height);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {100, 255, 100, 255};
    SDL_Color yellow = {255, 255, 100, 255};
    
    // Title
    draw_text(renderer, "PRIME MATHEMATICS CALCULATOR", x_base + 200, y_pos, green);
    y_pos += 30;
    
    // Display
    SDL_Rect display = {x_base, y_pos, 800, 50};
    SDL_SetRenderDrawColor(renderer, 40, 50, 60, 255);
    SDL_RenderFillRect(renderer, &display);
    SDL_SetRenderDrawColor(renderer, 120, 150, 180, 255);
    SDL_RenderDrawRect(renderer, &display);
    
    // Display value
    char display_text[128];
    if (state->calc_input[0] != '\0') {
        snprintf(display_text, sizeof(display_text), "%s", state->calc_input);
    } else if (state->calc_has_value1) {
        snprintf(display_text, sizeof(display_text), "%.2f %s", state->calc_value1, state->calc_operation);
    } else {
        snprintf(display_text, sizeof(display_text), "%.2f", state->calc_result);
    }
    draw_text(renderer, display_text, x_base + 10, y_pos + 15, yellow);
    y_pos += 70;
    
    // Calculator buttons (4x5 grid)
    const char* buttons[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "=", "+",
        "C", "CE", "Analyze", ""
    };
    
    int btn_width = 180;
    int btn_height = 60;
    int btn_spacing = 10;
    
    // Store button positions for click detection
    state->calc_button_x = x_base;
    state->calc_button_y = y_pos;
    state->calc_button_width = btn_width;
    state->calc_button_height = btn_height;
    state->calc_button_spacing = btn_spacing;
    
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            int idx = row * 4 + col;
            if (idx >= 19 || buttons[idx][0] == '\0') continue;
            
            int btn_x = x_base + col * (btn_width + btn_spacing);
            int btn_y = y_pos + row * (btn_height + btn_spacing);
            
            SDL_Rect btn = {btn_x, btn_y, btn_width, btn_height};
            
            // Check if this button is currently pressed (within 200ms)
            bool is_pressed = false;
            if (state->calc_button_pressed == idx) {
                uint32_t current_time = SDL_GetTicks();
                if (current_time - state->calc_button_press_time < 200) {
                    is_pressed = true;
                } else {
                    state->calc_button_pressed = -1;  // Reset after 200ms
                }
            }
            
            // Button color (brighter if pressed)
            if (is_pressed) {
                // Bright color when pressed
                SDL_SetRenderDrawColor(renderer, 200, 220, 255, 255);
            } else if (strcmp(buttons[idx], "=") == 0) {
                SDL_SetRenderDrawColor(renderer, 80, 120, 80, 255);
            } else if (strcmp(buttons[idx], "Analyze") == 0) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 120, 255);
            } else if (strcmp(buttons[idx], "C") == 0 || strcmp(buttons[idx], "CE") == 0) {
                SDL_SetRenderDrawColor(renderer, 120, 60, 60, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 60, 80, 100, 255);
            }
            
            SDL_RenderFillRect(renderer, &btn);
            SDL_SetRenderDrawColor(renderer, 120, 150, 180, 255);
            SDL_RenderDrawRect(renderer, &btn);
            
            // Button text
            int text_x = btn_x + (btn_width - strlen(buttons[idx]) * 8) / 2;
            int text_y = btn_y + (btn_height - 16) / 2;
            draw_text(renderer, buttons[idx], text_x, text_y, white);
        }
    }
    
    // Mode selector on the right
    int mode_x = x_base + 4 * (btn_width + btn_spacing) + 50;
    int mode_y = y_pos;
    
    draw_text(renderer, "VISUALIZATION MODE", mode_x, mode_y - 30, green);
    
    const char* mode_names[] = {
        "Ulam Spiral",
        "Recursive Fractal",
        "Harmonic Resonance",
        "Hyperdim Projection",
        "Self-Similar",
        "Clock Sudoku",
        "Natural Prime Lattice",
        "Folded Origami"
    };
    
    int mode_btn_width = 200;
    int mode_btn_height = 40;
    
    // Store mode button positions
    state->mode_button_x = mode_x;
    state->mode_button_y = mode_y;
    state->mode_button_width = mode_btn_width;
    state->mode_button_height = mode_btn_height;
    
    for (int i = 0; i < MODE_COUNT; i++) {
        int btn_y = mode_y + i * (mode_btn_height + 5);
        
        SDL_Rect mode_btn = {mode_x, btn_y, mode_btn_width, mode_btn_height};
        
        // Check if this mode button is currently pressed (within 200ms)
        bool is_mode_pressed = false;
        if (state->mode_button_pressed == i) {
            uint32_t current_time = SDL_GetTicks();
            if (current_time - state->mode_button_press_time < 200) {
                is_mode_pressed = true;
            } else {
                state->mode_button_pressed = -1;  // Reset after 200ms
            }
        }
        
        if (is_mode_pressed) {
            // Bright color when pressed
            SDL_SetRenderDrawColor(renderer, 200, 220, 255, 255);
        } else if ((int)state->mode == i) {
            SDL_SetRenderDrawColor(renderer, 80, 120, 80, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 50, 70, 90, 255);
        }
        
        SDL_RenderFillRect(renderer, &mode_btn);
        SDL_SetRenderDrawColor(renderer, 120, 150, 180, 255);
        SDL_RenderDrawRect(renderer, &mode_btn);
        
        SDL_Color mode_color = ((int)state->mode == i) ? green : white;
        draw_text(renderer, mode_names[i], mode_x + 10, btn_y + 12, mode_color);
    }
}