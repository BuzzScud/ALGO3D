/**
 * UI Components Implementation
 */

#include "components.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool ui_point_in_rect(int x, int y, const SDL_Rect* rect) {
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

float ui_clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ============================================================================
// BUTTON COMPONENT
// ============================================================================

UIButton* ui_button_create(int x, int y, int w, int h, const char* label) {
    UIButton* button = calloc(1, sizeof(UIButton));
    if (!button) return NULL;
    
    button->bounds = (SDL_Rect){x, y, w, h};
    if (label) {
        strncpy(button->label, label, sizeof(button->label) - 1);
    }
    
    button->state = UI_STATE_NORMAL;
    button->visible = true;
    
    // Default colors
    button->bg_normal = (SDL_Color){60, 60, 60, 255};
    button->bg_hover = (SDL_Color){80, 80, 80, 255};
    button->bg_active = (SDL_Color){100, 100, 100, 255};
    button->bg_disabled = (SDL_Color){40, 40, 40, 255};
    button->text_color = (SDL_Color){220, 220, 220, 255};
    button->border_color = (SDL_Color){100, 100, 100, 255};
    
    return button;
}

void ui_button_destroy(UIButton* button) {
    free(button);
}

void ui_button_render(UIButton* button, SDL_Renderer* renderer) {
    if (!button || !button->visible || !renderer) return;
    
    // Select background color based on state
    SDL_Color bg;
    switch (button->state) {
        case UI_STATE_HOVER:
            bg = button->bg_hover;
            break;
        case UI_STATE_ACTIVE:
            bg = button->bg_active;
            break;
        case UI_STATE_DISABLED:
            bg = button->bg_disabled;
            break;
        default:
            bg = button->bg_normal;
            break;
    }
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &button->bounds);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 
                          button->border_color.r,
                          button->border_color.g,
                          button->border_color.b,
                          button->border_color.a);
    SDL_RenderDrawRect(renderer, &button->bounds);
    
    // Draw text (centered)
    if (button->label[0] != '\0') {
        int text_x = button->bounds.x + (button->bounds.w / 2) - (strlen(button->label) * 4);
        int text_y = button->bounds.y + (button->bounds.h / 2) - 6;
        draw_text(renderer, button->label, text_x, text_y, button->text_color);
    }
}

bool ui_button_handle_event(UIButton* button, SDL_Event* event) {
    if (!button || !button->visible || button->state == UI_STATE_DISABLED) {
        return false;
    }
    
    switch (event->type) {
        case SDL_MOUSEMOTION: {
            bool inside = ui_point_in_rect(event->motion.x, event->motion.y, &button->bounds);
            if (inside && button->state == UI_STATE_NORMAL) {
                button->state = UI_STATE_HOVER;
                return true;
            } else if (!inside && button->state == UI_STATE_HOVER) {
                button->state = UI_STATE_NORMAL;
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (ui_point_in_rect(event->button.x, event->button.y, &button->bounds)) {
                    button->state = UI_STATE_ACTIVE;
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (button->state == UI_STATE_ACTIVE) {
                    if (ui_point_in_rect(event->button.x, event->button.y, &button->bounds)) {
                        // Button clicked!
                        if (button->on_click) {
                            button->on_click(button->user_data);
                        }
                        button->state = UI_STATE_HOVER;
                    } else {
                        button->state = UI_STATE_NORMAL;
                    }
                    return true;
                }
            }
            break;
        }
    }
    
    return false;
}

void ui_button_set_callback(UIButton* button, ButtonCallback callback, void* user_data) {
    if (!button) return;
    button->on_click = callback;
    button->user_data = user_data;
}

void ui_button_set_enabled(UIButton* button, bool enabled) {
    if (!button) return;
    button->state = enabled ? UI_STATE_NORMAL : UI_STATE_DISABLED;
}

void ui_button_set_position(UIButton* button, int x, int y) {
    if (!button) return;
    button->bounds.x = x;
    button->bounds.y = y;
}

// ============================================================================
// TEXT INPUT COMPONENT
// ============================================================================

UITextInput* ui_text_input_create(int x, int y, int w, int h, const char* placeholder) {
    UITextInput* input = calloc(1, sizeof(UITextInput));
    if (!input) return NULL;
    
    input->bounds = (SDL_Rect){x, y, w, h};
    if (placeholder) {
        strncpy(input->placeholder, placeholder, sizeof(input->placeholder) - 1);
    }
    
    input->cursor_pos = 0;
    input->focused = false;
    input->visible = true;
    input->state = UI_STATE_NORMAL;
    
    // Default colors
    input->bg_color = (SDL_Color){30, 30, 30, 255};
    input->text_color = (SDL_Color){220, 220, 220, 255};
    input->placeholder_color = (SDL_Color){100, 100, 100, 255};
    input->border_color = (SDL_Color){80, 80, 80, 255};
    input->cursor_color = (SDL_Color){220, 220, 220, 255};
    
    return input;
}

void ui_text_input_destroy(UITextInput* input) {
    free(input);
}

void ui_text_input_render(UITextInput* input, SDL_Renderer* renderer) {
    if (!input || !input->visible || !renderer) return;
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, 
                          input->bg_color.r,
                          input->bg_color.g,
                          input->bg_color.b,
                          input->bg_color.a);
    SDL_RenderFillRect(renderer, &input->bounds);
    
    // Draw border (thicker if focused)
    SDL_Color border = input->border_color;
    if (input->focused) {
        border = (SDL_Color){120, 150, 200, 255};
    }
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, &input->bounds);
    
    // Draw text or placeholder
    int text_x = input->bounds.x + 5;
    int text_y = input->bounds.y + (input->bounds.h / 2) - 6;
    
    if (input->text[0] != '\0') {
        draw_text(renderer, input->text, text_x, text_y, input->text_color);
        
        // Draw cursor if focused
        if (input->focused) {
            int cursor_x = text_x + (input->cursor_pos * 8);
            SDL_SetRenderDrawColor(renderer, 
                                  input->cursor_color.r,
                                  input->cursor_color.g,
                                  input->cursor_color.b,
                                  input->cursor_color.a);
            SDL_RenderDrawLine(renderer, cursor_x, text_y, cursor_x, text_y + 12);
        }
    } else if (input->placeholder[0] != '\0') {
        draw_text(renderer, input->placeholder, text_x, text_y, input->placeholder_color);
    }
}

bool ui_text_input_handle_event(UITextInput* input, SDL_Event* event) {
    if (!input || !input->visible) return false;
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                bool inside = ui_point_in_rect(event->button.x, event->button.y, &input->bounds);
                input->focused = inside;
                return inside;
            }
            break;
        }
        
        case SDL_TEXTINPUT: {
            if (input->focused) {
                size_t len = strlen(input->text);
                if (len < sizeof(input->text) - 1) {
                    strcat(input->text, event->text.text);
                    input->cursor_pos = strlen(input->text);
                    
                    if (input->on_change) {
                        input->on_change(input->text, input->user_data);
                    }
                }
                return true;
            }
            break;
        }
        
        case SDL_KEYDOWN: {
            if (input->focused) {
                if (event->key.keysym.sym == SDLK_BACKSPACE) {
                    size_t len = strlen(input->text);
                    if (len > 0) {
                        input->text[len - 1] = '\0';
                        input->cursor_pos = len - 1;
                        
                        if (input->on_change) {
                            input->on_change(input->text, input->user_data);
                        }
                    }
                    return true;
                } else if (event->key.keysym.sym == SDLK_RETURN) {
                    if (input->on_submit) {
                        input->on_submit(input->text, input->user_data);
                    }
                    return true;
                }
            }
            break;
        }
    }
    
    return false;
}

void ui_text_input_set_text(UITextInput* input, const char* text) {
    if (!input) return;
    if (text) {
        strncpy(input->text, text, sizeof(input->text) - 1);
        input->text[sizeof(input->text) - 1] = '\0';
        input->cursor_pos = strlen(input->text);
    } else {
        input->text[0] = '\0';
        input->cursor_pos = 0;
    }
}

const char* ui_text_input_get_text(const UITextInput* input) {
    return input ? input->text : "";
}

void ui_text_input_set_callback(UITextInput* input, TextInputCallback on_change,
                                TextInputCallback on_submit, void* user_data) {
    if (!input) return;
    input->on_change = on_change;
    input->on_submit = on_submit;
    input->user_data = user_data;
}

void ui_text_input_set_validator(UITextInput* input, bool (*validator)(const char*)) {
    if (!input) return;
    input->validator = validator;
}

void ui_text_input_set_focus(UITextInput* input, bool focused) {
    if (!input) return;
    input->focused = focused;
}

// ============================================================================
// SLIDER COMPONENT
// ============================================================================

UISlider* ui_slider_create(int x, int y, int w, int h, float min, float max, const char* label) {
    UISlider* slider = calloc(1, sizeof(UISlider));
    if (!slider) return NULL;
    
    slider->bounds = (SDL_Rect){x, y, w, h};
    slider->value = 0.5f;
    slider->min_value = min;
    slider->max_value = max;
    slider->dragging = false;
    slider->visible = true;
    slider->state = UI_STATE_NORMAL;
    slider->show_value = true;
    
    if (label) {
        strncpy(slider->label, label, sizeof(slider->label) - 1);
    }
    
    // Default colors
    slider->track_color = (SDL_Color){60, 60, 60, 255};
    slider->fill_color = (SDL_Color){100, 150, 200, 255};
    slider->handle_color = (SDL_Color){220, 220, 220, 255};
    slider->text_color = (SDL_Color){220, 220, 220, 255};
    
    return slider;
}

void ui_slider_destroy(UISlider* slider) {
    free(slider);
}

void ui_slider_render(UISlider* slider, SDL_Renderer* renderer) {
    if (!slider || !slider->visible || !renderer) return;
    
    // Draw label
    if (slider->label[0] != '\0') {
        draw_text(renderer, slider->label, slider->bounds.x, slider->bounds.y - 20, slider->text_color);
    }
    
    // Draw track
    SDL_Rect track = {
        slider->bounds.x,
        slider->bounds.y + slider->bounds.h / 2 - 2,
        slider->bounds.w,
        4
    };
    SDL_SetRenderDrawColor(renderer, 
                          slider->track_color.r,
                          slider->track_color.g,
                          slider->track_color.b,
                          slider->track_color.a);
    SDL_RenderFillRect(renderer, &track);
    
    // Draw fill
    SDL_Rect fill = track;
    fill.w = (int)(slider->value * slider->bounds.w);
    SDL_SetRenderDrawColor(renderer,
                          slider->fill_color.r,
                          slider->fill_color.g,
                          slider->fill_color.b,
                          slider->fill_color.a);
    SDL_RenderFillRect(renderer, &fill);
    
    // Draw handle
    int handle_x = slider->bounds.x + (int)(slider->value * slider->bounds.w);
    int handle_y = slider->bounds.y + slider->bounds.h / 2;
    int handle_size = 12;
    
    SDL_Rect handle = {
        handle_x - handle_size / 2,
        handle_y - handle_size / 2,
        handle_size,
        handle_size
    };
    
    SDL_SetRenderDrawColor(renderer,
                          slider->handle_color.r,
                          slider->handle_color.g,
                          slider->handle_color.b,
                          slider->handle_color.a);
    SDL_RenderFillRect(renderer, &handle);
    
    // Draw value
    if (slider->show_value) {
        char value_str[32];
        float actual_value = slider->min_value + slider->value * (slider->max_value - slider->min_value);
        snprintf(value_str, sizeof(value_str), "%.1f", actual_value);
        draw_text(renderer, value_str, slider->bounds.x + slider->bounds.w + 10, 
                 slider->bounds.y + slider->bounds.h / 2 - 6, slider->text_color);
    }
}

bool ui_slider_handle_event(UISlider* slider, SDL_Event* event) {
    if (!slider || !slider->visible) return false;
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (ui_point_in_rect(event->button.x, event->button.y, &slider->bounds)) {
                    slider->dragging = true;
                    
                    // Update value
                    float new_value = (float)(event->button.x - slider->bounds.x) / slider->bounds.w;
                    slider->value = ui_clamp(new_value, 0.0f, 1.0f);
                    
                    if (slider->on_change) {
                        float actual_value = slider->min_value + slider->value * (slider->max_value - slider->min_value);
                        slider->on_change(actual_value, slider->user_data);
                    }
                    
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (slider->dragging) {
                    slider->dragging = false;
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEMOTION: {
            if (slider->dragging) {
                float new_value = (float)(event->motion.x - slider->bounds.x) / slider->bounds.w;
                slider->value = ui_clamp(new_value, 0.0f, 1.0f);
                
                if (slider->on_change) {
                    float actual_value = slider->min_value + slider->value * (slider->max_value - slider->min_value);
                    slider->on_change(actual_value, slider->user_data);
                }
                
                return true;
            }
            break;
        }
    }
    
    return false;
}

void ui_slider_set_value(UISlider* slider, float value) {
    if (!slider) return;
    
    // Convert actual value to 0-1 range
    float normalized = (value - slider->min_value) / (slider->max_value - slider->min_value);
    slider->value = ui_clamp(normalized, 0.0f, 1.0f);
}

float ui_slider_get_value(const UISlider* slider) {
    if (!slider) return 0.0f;
    return slider->min_value + slider->value * (slider->max_value - slider->min_value);
}

int ui_slider_get_value_int(const UISlider* slider) {
    return (int)ui_slider_get_value(slider);
}

void ui_slider_set_callback(UISlider* slider, SliderCallback callback, void* user_data) {
    if (!slider) return;
    slider->on_change = callback;
    slider->user_data = user_data;
}

// ============================================================================
// PROGRESS BAR COMPONENT
// ============================================================================

UIProgressBar* ui_progress_bar_create(int x, int y, int w, int h) {
    UIProgressBar* bar = calloc(1, sizeof(UIProgressBar));
    if (!bar) return NULL;
    
    bar->bounds = (SDL_Rect){x, y, w, h};
    bar->progress = 0.0f;
    bar->visible = true;
    bar->show_percentage = true;
    
    // Default colors
    bar->bg_color = (SDL_Color){40, 40, 40, 255};
    bar->fill_color = (SDL_Color){100, 200, 100, 255};
    bar->text_color = (SDL_Color){220, 220, 220, 255};
    bar->border_color = (SDL_Color){80, 80, 80, 255};
    
    return bar;
}

void ui_progress_bar_destroy(UIProgressBar* bar) {
    free(bar);
}

void ui_progress_bar_render(UIProgressBar* bar, SDL_Renderer* renderer) {
    if (!bar || !bar->visible || !renderer) return;
    
    // Draw background
    SDL_SetRenderDrawColor(renderer,
                          bar->bg_color.r,
                          bar->bg_color.g,
                          bar->bg_color.b,
                          bar->bg_color.a);
    SDL_RenderFillRect(renderer, &bar->bounds);
    
    // Draw fill
    SDL_Rect fill = bar->bounds;
    fill.w = (int)(bar->progress * bar->bounds.w);
    SDL_SetRenderDrawColor(renderer,
                          bar->fill_color.r,
                          bar->fill_color.g,
                          bar->fill_color.b,
                          bar->fill_color.a);
    SDL_RenderFillRect(renderer, &fill);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer,
                          bar->border_color.r,
                          bar->border_color.g,
                          bar->border_color.b,
                          bar->border_color.a);
    SDL_RenderDrawRect(renderer, &bar->bounds);
    
    // Draw text
    char text[256];
    if (bar->show_percentage) {
        snprintf(text, sizeof(text), "%s %.1f%%", bar->label, bar->progress * 100.0f);
    } else {
        strncpy(text, bar->label, sizeof(text) - 1);
    }
    
    if (text[0] != '\0') {
        int text_x = bar->bounds.x + (bar->bounds.w / 2) - (strlen(text) * 4);
        int text_y = bar->bounds.y + (bar->bounds.h / 2) - 6;
        draw_text(renderer, text, text_x, text_y, bar->text_color);
    }
}

void ui_progress_bar_set_progress(UIProgressBar* bar, float progress) {
    if (!bar) return;
    bar->progress = ui_clamp(progress, 0.0f, 1.0f);
}

void ui_progress_bar_set_label(UIProgressBar* bar, const char* label) {
    if (!bar) return;
    if (label) {
        strncpy(bar->label, label, sizeof(bar->label) - 1);
        bar->label[sizeof(bar->label) - 1] = '\0';
    } else {
        bar->label[0] = '\0';
    }
}

// ============================================================================
// LABEL COMPONENT
// ============================================================================

UILabel* ui_label_create(int x, int y, int w, int h, const char* text) {
    UILabel* label = calloc(1, sizeof(UILabel));
    if (!label) return NULL;
    
    label->bounds = (SDL_Rect){x, y, w, h};
    if (text) {
        strncpy(label->text, text, sizeof(label->text) - 1);
    }
    
    label->visible = true;
    label->text_color = (SDL_Color){220, 220, 220, 255};
    label->bg_color = (SDL_Color){0, 0, 0, 0};
    label->has_background = false;
    label->align = LABEL_ALIGN_LEFT;
    
    return label;
}

void ui_label_destroy(UILabel* label) {
    free(label);
}

void ui_label_render(UILabel* label, SDL_Renderer* renderer) {
    if (!label || !label->visible || !renderer) return;
    
    // Draw background if enabled
    if (label->has_background) {
        SDL_SetRenderDrawColor(renderer,
                              label->bg_color.r,
                              label->bg_color.g,
                              label->bg_color.b,
                              label->bg_color.a);
        SDL_RenderFillRect(renderer, &label->bounds);
    }
    
    // Draw text
    if (label->text[0] != '\0') {
        int text_x = label->bounds.x;
        
        // Apply alignment
        if (label->align == LABEL_ALIGN_CENTER) {
            text_x = label->bounds.x + (label->bounds.w / 2) - (strlen(label->text) * 4);
        } else if (label->align == LABEL_ALIGN_RIGHT) {
            text_x = label->bounds.x + label->bounds.w - (strlen(label->text) * 8);
        }
        
        int text_y = label->bounds.y + (label->bounds.h / 2) - 6;
        draw_text(renderer, label->text, text_x, text_y, label->text_color);
    }
}

void ui_label_set_text(UILabel* label, const char* text) {
    if (!label) return;
    if (text) {
        strncpy(label->text, text, sizeof(label->text) - 1);
        label->text[sizeof(label->text) - 1] = '\0';
    } else {
        label->text[0] = '\0';
    }
}

void ui_label_set_color(UILabel* label, SDL_Color color) {
    if (!label) return;
    label->text_color = color;
}

// ============================================================================
// PANEL COMPONENT
// ============================================================================

UIPanel* ui_panel_create(int x, int y, int w, int h, const char* title) {
    UIPanel* panel = calloc(1, sizeof(UIPanel));
    if (!panel) return NULL;
    
    panel->bounds = (SDL_Rect){x, y, w, h};
    if (title) {
        strncpy(panel->title, title, sizeof(panel->title) - 1);
    }
    
    panel->visible = true;
    panel->collapsible = false;
    panel->collapsed = false;
    
    // Default colors
    panel->bg_color = (SDL_Color){30, 30, 30, 255};
    panel->border_color = (SDL_Color){80, 80, 80, 255};
    panel->title_bg_color = (SDL_Color){50, 50, 50, 255};
    panel->title_text_color = (SDL_Color){200, 220, 255, 255};
    
    return panel;
}

void ui_panel_destroy(UIPanel* panel) {
    free(panel);
}

void ui_panel_render(UIPanel* panel, SDL_Renderer* renderer) {
    if (!panel || !panel->visible || !renderer) return;
    
    int title_height = 30;
    
    // Draw title bar
    SDL_Rect title_rect = {panel->bounds.x, panel->bounds.y, panel->bounds.w, title_height};
    SDL_SetRenderDrawColor(renderer,
                          panel->title_bg_color.r,
                          panel->title_bg_color.g,
                          panel->title_bg_color.b,
                          panel->title_bg_color.a);
    SDL_RenderFillRect(renderer, &title_rect);
    
    // Draw title text
    if (panel->title[0] != '\0') {
        draw_text(renderer, panel->title, panel->bounds.x + 10, panel->bounds.y + 8, 
                 panel->title_text_color);
    }
    
    // Draw collapse indicator if collapsible
    if (panel->collapsible) {
        const char* indicator = panel->collapsed ? "+" : "-";
        draw_text(renderer, indicator, panel->bounds.x + panel->bounds.w - 20, 
                 panel->bounds.y + 8, panel->title_text_color);
    }
    
    // Draw content area if not collapsed
    if (!panel->collapsed) {
        SDL_Rect content_rect = {
            panel->bounds.x,
            panel->bounds.y + title_height,
            panel->bounds.w,
            panel->bounds.h - title_height
        };
        
        SDL_SetRenderDrawColor(renderer,
                              panel->bg_color.r,
                              panel->bg_color.g,
                              panel->bg_color.b,
                              panel->bg_color.a);
        SDL_RenderFillRect(renderer, &content_rect);
    }
    
    // Draw border
    SDL_SetRenderDrawColor(renderer,
                          panel->border_color.r,
                          panel->border_color.g,
                          panel->border_color.b,
                          panel->border_color.a);
    SDL_RenderDrawRect(renderer, &panel->bounds);
}

bool ui_panel_handle_event(UIPanel* panel, SDL_Event* event) {
    if (!panel || !panel->visible || !panel->collapsible) return false;
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        SDL_Rect title_rect = {panel->bounds.x, panel->bounds.y, panel->bounds.w, 30};
        if (ui_point_in_rect(event->button.x, event->button.y, &title_rect)) {
            panel->collapsed = !panel->collapsed;
            return true;
        }
    }
    
    return false;
}

void ui_panel_set_collapsible(UIPanel* panel, bool collapsible) {
    if (!panel) return;
    panel->collapsible = collapsible;
}

void ui_panel_set_collapsed(UIPanel* panel, bool collapsed) {
    if (!panel) return;
    panel->collapsed = collapsed;
}

bool ui_panel_is_collapsed(const UIPanel* panel) {
    return panel ? panel->collapsed : false;
}

SDL_Rect ui_panel_get_content_bounds(const UIPanel* panel) {
    if (!panel || panel->collapsed) {
        return (SDL_Rect){0, 0, 0, 0};
    }
    
    return (SDL_Rect){
        panel->bounds.x + 5,
        panel->bounds.y + 35,
        panel->bounds.w - 10,
        panel->bounds.h - 40
    };
}

// ============================================================================
// DROPDOWN COMPONENT
// ============================================================================

UIDropdown* ui_dropdown_create(int x, int y, int w, int h) {
    UIDropdown* dropdown = calloc(1, sizeof(UIDropdown));
    if (!dropdown) return NULL;
    
    dropdown->bounds = (SDL_Rect){x, y, w, h};
    dropdown->items = NULL;
    dropdown->num_items = 0;
    dropdown->selected_index = -1;
    dropdown->open = false;
    dropdown->visible = true;
    dropdown->state = UI_STATE_NORMAL;
    dropdown->scroll_offset = 0;
    dropdown->hover_index = -1;
    
    // Default colors
    dropdown->bg_color = (SDL_Color){40, 40, 40, 255};
    dropdown->text_color = (SDL_Color){220, 220, 220, 255};
    dropdown->hover_color = (SDL_Color){60, 60, 80, 255};
    dropdown->border_color = (SDL_Color){100, 100, 100, 255};
    
    return dropdown;
}

void ui_dropdown_destroy(UIDropdown* dropdown) {
    if (!dropdown) return;
    
    // Free all items
    for (int i = 0; i < dropdown->num_items; i++) {
        free(dropdown->items[i]);
    }
    free(dropdown->items);
    free(dropdown);
}

void ui_dropdown_render(UIDropdown* dropdown, SDL_Renderer* renderer) {
    if (!dropdown || !dropdown->visible || !renderer) return;
    
    // Draw main button
    SDL_Color bg = (dropdown->state == UI_STATE_HOVER) ? dropdown->hover_color : dropdown->bg_color;
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &dropdown->bounds);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 
                          dropdown->border_color.r,
                          dropdown->border_color.g,
                          dropdown->border_color.b,
                          dropdown->border_color.a);
    SDL_RenderDrawRect(renderer, &dropdown->bounds);
    
    // Draw selected item text
    if (dropdown->selected_index >= 0 && dropdown->selected_index < dropdown->num_items) {
        draw_text(renderer, dropdown->items[dropdown->selected_index],
                 dropdown->bounds.x + 5, dropdown->bounds.y + 8, dropdown->text_color);
    } else {
        SDL_Color placeholder = {150, 150, 150, 255};
        draw_text(renderer, "Select...", dropdown->bounds.x + 5, dropdown->bounds.y + 8, placeholder);
    }
    
    // Draw arrow
    const char* arrow = dropdown->open ? "▲" : "▼";
    draw_text(renderer, arrow, dropdown->bounds.x + dropdown->bounds.w - 20, 
             dropdown->bounds.y + 8, dropdown->text_color);
    
    // Draw dropdown list if open
    if (dropdown->open && dropdown->num_items > 0) {
        int item_height = 25;
        int max_visible = 10;
        int visible_items = (dropdown->num_items < max_visible) ? dropdown->num_items : max_visible;
        int list_height = visible_items * item_height;
        
        SDL_Rect list_rect = {
            dropdown->bounds.x,
            dropdown->bounds.y + dropdown->bounds.h,
            dropdown->bounds.w,
            list_height
        };
        
        // Draw list background
        SDL_SetRenderDrawColor(renderer, 
                              dropdown->bg_color.r,
                              dropdown->bg_color.g,
                              dropdown->bg_color.b,
                              dropdown->bg_color.a);
        SDL_RenderFillRect(renderer, &list_rect);
        
        // Draw list border
        SDL_SetRenderDrawColor(renderer,
                              dropdown->border_color.r,
                              dropdown->border_color.g,
                              dropdown->border_color.b,
                              dropdown->border_color.a);
        SDL_RenderDrawRect(renderer, &list_rect);
        
        // Draw items
        for (int i = 0; i < visible_items; i++) {
            int item_index = i + dropdown->scroll_offset;
            if (item_index >= dropdown->num_items) break;
            
            SDL_Rect item_rect = {
                list_rect.x,
                list_rect.y + i * item_height,
                list_rect.w,
                item_height
            };
            
            // Highlight hovered item
            if (item_index == dropdown->hover_index) {
                SDL_SetRenderDrawColor(renderer,
                                      dropdown->hover_color.r,
                                      dropdown->hover_color.g,
                                      dropdown->hover_color.b,
                                      dropdown->hover_color.a);
                SDL_RenderFillRect(renderer, &item_rect);
            }
            
            // Draw item text
            draw_text(renderer, dropdown->items[item_index],
                     item_rect.x + 5, item_rect.y + 5, dropdown->text_color);
        }
    }
}

bool ui_dropdown_handle_event(UIDropdown* dropdown, SDL_Event* event) {
    if (!dropdown || !dropdown->visible) return false;
    
    switch (event->type) {
        case SDL_MOUSEMOTION: {
            // Check if hovering over main button
            bool over_button = ui_point_in_rect(event->motion.x, event->motion.y, &dropdown->bounds);
            dropdown->state = over_button ? UI_STATE_HOVER : UI_STATE_NORMAL;
            
            // Check if hovering over list items
            if (dropdown->open) {
                int item_height = 25;
                int max_visible = 10;
                int visible_items = (dropdown->num_items < max_visible) ? dropdown->num_items : max_visible;
                
                SDL_Rect list_rect = {
                    dropdown->bounds.x,
                    dropdown->bounds.y + dropdown->bounds.h,
                    dropdown->bounds.w,
                    visible_items * item_height
                };
                
                if (ui_point_in_rect(event->motion.x, event->motion.y, &list_rect)) {
                    int relative_y = event->motion.y - list_rect.y;
                    int item_index = (relative_y / item_height) + dropdown->scroll_offset;
                    dropdown->hover_index = item_index;
                    return true;
                } else {
                    dropdown->hover_index = -1;
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                // Check if clicking main button
                if (ui_point_in_rect(event->button.x, event->button.y, &dropdown->bounds)) {
                    dropdown->open = !dropdown->open;
                    return true;
                }
                
                // Check if clicking list item
                if (dropdown->open) {
                    int item_height = 25;
                    int max_visible = 10;
                    int visible_items = (dropdown->num_items < max_visible) ? dropdown->num_items : max_visible;
                    
                    SDL_Rect list_rect = {
                        dropdown->bounds.x,
                        dropdown->bounds.y + dropdown->bounds.h,
                        dropdown->bounds.w,
                        visible_items * item_height
                    };
                    
                    if (ui_point_in_rect(event->button.x, event->button.y, &list_rect)) {
                        int relative_y = event->button.y - list_rect.y;
                        int item_index = (relative_y / item_height) + dropdown->scroll_offset;
                        
                        if (item_index >= 0 && item_index < dropdown->num_items) {
                            dropdown->selected_index = item_index;
                            dropdown->open = false;
                            
                            if (dropdown->on_select) {
                                dropdown->on_select(item_index, dropdown->items[item_index], 
                                                   dropdown->user_data);
                            }
                            return true;
                        }
                    } else {
                        // Clicked outside - close dropdown
                        dropdown->open = false;
                        return true;
                    }
                }
            }
            break;
        }
        
        case SDL_MOUSEWHEEL: {
            if (dropdown->open) {
                dropdown->scroll_offset -= event->wheel.y;
                if (dropdown->scroll_offset < 0) dropdown->scroll_offset = 0;
                
                int max_visible = 10;
                int max_scroll = dropdown->num_items - max_visible;
                if (max_scroll < 0) max_scroll = 0;
                if (dropdown->scroll_offset > max_scroll) dropdown->scroll_offset = max_scroll;
                
                return true;
            }
            break;
        }
    }
    
    return false;
}

void ui_dropdown_add_item(UIDropdown* dropdown, const char* item) {
    if (!dropdown || !item) return;
    
    // Resize items array
    dropdown->items = realloc(dropdown->items, (dropdown->num_items + 1) * sizeof(char*));
    dropdown->items[dropdown->num_items] = strdup(item);
    dropdown->num_items++;
}

void ui_dropdown_clear_items(UIDropdown* dropdown) {
    if (!dropdown) return;
    
    for (int i = 0; i < dropdown->num_items; i++) {
        free(dropdown->items[i]);
    }
    free(dropdown->items);
    
    dropdown->items = NULL;
    dropdown->num_items = 0;
    dropdown->selected_index = -1;
}

void ui_dropdown_set_selected(UIDropdown* dropdown, int index) {
    if (!dropdown) return;
    if (index >= -1 && index < dropdown->num_items) {
        dropdown->selected_index = index;
    }
}

int ui_dropdown_get_selected(const UIDropdown* dropdown) {
    return dropdown ? dropdown->selected_index : -1;
}

const char* ui_dropdown_get_selected_text(const UIDropdown* dropdown) {
    if (!dropdown || dropdown->selected_index < 0 || dropdown->selected_index >= dropdown->num_items) {
        return NULL;
    }
    return dropdown->items[dropdown->selected_index];
}

void ui_dropdown_set_callback(UIDropdown* dropdown, DropdownCallback callback, void* user_data) {
    if (!dropdown) return;
    dropdown->on_select = callback;
    dropdown->user_data = user_data;
}

// ============================================================================
// DIALOG COMPONENT
// ============================================================================

static void dialog_button_callback(void* user_data);

UIDialog* ui_dialog_create(int x, int y, int w, int h, const char* title,
                           const char* message, DialogType type) {
    UIDialog* dialog = calloc(1, sizeof(UIDialog));
    if (!dialog) return NULL;
    
    dialog->bounds = (SDL_Rect){x, y, w, h};
    if (title) {
        strncpy(dialog->title, title, sizeof(dialog->title) - 1);
    }
    if (message) {
        strncpy(dialog->message, message, sizeof(dialog->message) - 1);
    }
    
    dialog->type = type;
    dialog->result = DIALOG_RESULT_NONE;
    dialog->visible = false;
    dialog->modal = true;
    dialog->num_buttons = 0;
    
    // Default colors
    dialog->bg_color = (SDL_Color){40, 40, 40, 255};
    dialog->title_bg_color = (SDL_Color){60, 60, 80, 255};
    dialog->title_text_color = (SDL_Color){220, 220, 220, 255};
    dialog->text_color = (SDL_Color){200, 200, 200, 255};
    dialog->border_color = (SDL_Color){100, 100, 100, 255};
    
    // Create buttons based on type
    int button_width = 100;
    int button_height = 30;
    int button_spacing = 10;
    int button_y = y + h - button_height - 20;
    
    switch (type) {
        case DIALOG_OK: {
            int button_x = x + (w - button_width) / 2;
            dialog->buttons[0] = ui_button_create(button_x, button_y, button_width, button_height, "OK");
            dialog->num_buttons = 1;
            break;
        }
        
        case DIALOG_OK_CANCEL: {
            int total_width = 2 * button_width + button_spacing;
            int start_x = x + (w - total_width) / 2;
            dialog->buttons[0] = ui_button_create(start_x, button_y, button_width, button_height, "OK");
            dialog->buttons[1] = ui_button_create(start_x + button_width + button_spacing, button_y, 
                                                  button_width, button_height, "Cancel");
            dialog->num_buttons = 2;
            break;
        }
        
        case DIALOG_YES_NO: {
            int total_width = 2 * button_width + button_spacing;
            int start_x = x + (w - total_width) / 2;
            dialog->buttons[0] = ui_button_create(start_x, button_y, button_width, button_height, "Yes");
            dialog->buttons[1] = ui_button_create(start_x + button_width + button_spacing, button_y,
                                                  button_width, button_height, "No");
            dialog->num_buttons = 2;
            break;
        }
        
        case DIALOG_YES_NO_CANCEL: {
            int total_width = 3 * button_width + 2 * button_spacing;
            int start_x = x + (w - total_width) / 2;
            dialog->buttons[0] = ui_button_create(start_x, button_y, button_width, button_height, "Yes");
            dialog->buttons[1] = ui_button_create(start_x + button_width + button_spacing, button_y,
                                                  button_width, button_height, "No");
            dialog->buttons[2] = ui_button_create(start_x + 2 * (button_width + button_spacing), button_y,
                                                  button_width, button_height, "Cancel");
            dialog->num_buttons = 3;
            break;
        }
    }
    
    // Set button callbacks
    for (int i = 0; i < dialog->num_buttons; i++) {
        ui_button_set_callback(dialog->buttons[i], dialog_button_callback, dialog);
    }
    
    return dialog;
}

void ui_dialog_destroy(UIDialog* dialog) {
    if (!dialog) return;
    
    for (int i = 0; i < dialog->num_buttons; i++) {
        ui_button_destroy(dialog->buttons[i]);
    }
    
    free(dialog);
}

void ui_dialog_render(UIDialog* dialog, SDL_Renderer* renderer) {
    if (!dialog || !dialog->visible || !renderer) return;
    
    // Draw semi-transparent overlay if modal
    if (dialog->modal) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_Rect screen = {0, 0, 1920, 1080}; // TODO: Get actual screen size
        SDL_RenderFillRect(renderer, &screen);
    }
    
    // Draw dialog background
    SDL_SetRenderDrawColor(renderer,
                          dialog->bg_color.r,
                          dialog->bg_color.g,
                          dialog->bg_color.b,
                          dialog->bg_color.a);
    SDL_RenderFillRect(renderer, &dialog->bounds);
    
    // Draw title bar
    SDL_Rect title_rect = {dialog->bounds.x, dialog->bounds.y, dialog->bounds.w, 40};
    SDL_SetRenderDrawColor(renderer,
                          dialog->title_bg_color.r,
                          dialog->title_bg_color.g,
                          dialog->title_bg_color.b,
                          dialog->title_bg_color.a);
    SDL_RenderFillRect(renderer, &title_rect);
    
    // Draw title text
    if (dialog->title[0] != '\0') {
        draw_text(renderer, dialog->title, dialog->bounds.x + 10, dialog->bounds.y + 12,
                 dialog->title_text_color);
    }
    
    // Draw message
    if (dialog->message[0] != '\0') {
        // Simple word wrapping (basic implementation)
        int text_x = dialog->bounds.x + 20;
        int text_y = dialog->bounds.y + 60;
        int line_height = 20;
        
        char* message_copy = strdup(dialog->message);
        char* line = strtok(message_copy, "\n");
        
        while (line) {
            draw_text(renderer, line, text_x, text_y, dialog->text_color);
            text_y += line_height;
            line = strtok(NULL, "\n");
        }
        
        free(message_copy);
    }
    
    // Draw border
    SDL_SetRenderDrawColor(renderer,
                          dialog->border_color.r,
                          dialog->border_color.g,
                          dialog->border_color.b,
                          dialog->border_color.a);
    SDL_RenderDrawRect(renderer, &dialog->bounds);
    
    // Draw buttons
    for (int i = 0; i < dialog->num_buttons; i++) {
        ui_button_render(dialog->buttons[i], renderer);
    }
}

bool ui_dialog_handle_event(UIDialog* dialog, SDL_Event* event) {
    if (!dialog || !dialog->visible) return false;
    
    // Handle button events
    for (int i = 0; i < dialog->num_buttons; i++) {
        if (ui_button_handle_event(dialog->buttons[i], event)) {
            return true;
        }
    }
    
    // Block events from reaching underlying UI if modal
    if (dialog->modal) {
        return true;
    }
    
    return false;
}

static void dialog_button_callback(void* user_data) {
    UIDialog* dialog = (UIDialog*)user_data;
    if (!dialog) return;
    
    // Determine which button was clicked
    for (int i = 0; i < dialog->num_buttons; i++) {
        if (dialog->buttons[i]->state == UI_STATE_ACTIVE) {
            // Map button to result
            const char* label = dialog->buttons[i]->label;
            
            if (strcmp(label, "OK") == 0) {
                dialog->result = DIALOG_RESULT_OK;
            } else if (strcmp(label, "Cancel") == 0) {
                dialog->result = DIALOG_RESULT_CANCEL;
            } else if (strcmp(label, "Yes") == 0) {
                dialog->result = DIALOG_RESULT_YES;
            } else if (strcmp(label, "No") == 0) {
                dialog->result = DIALOG_RESULT_NO;
            }
            
            // Hide dialog
            dialog->visible = false;
            
            // Call user callback
            if (dialog->on_close) {
                dialog->on_close(dialog->result, dialog->user_data);
            }
            
            break;
        }
    }
}

void ui_dialog_show(UIDialog* dialog) {
    if (!dialog) return;
    dialog->visible = true;
    dialog->result = DIALOG_RESULT_NONE;
}

void ui_dialog_hide(UIDialog* dialog) {
    if (!dialog) return;
    dialog->visible = false;
}

bool ui_dialog_is_visible(const UIDialog* dialog) {
    return dialog ? dialog->visible : false;
}

DialogResult ui_dialog_get_result(const UIDialog* dialog) {
    return dialog ? dialog->result : DIALOG_RESULT_NONE;
}

void ui_dialog_set_callback(UIDialog* dialog,
                            void (*on_close)(DialogResult, void*), void* user_data) {
    if (!dialog) return;
    dialog->on_close = on_close;
    dialog->user_data = user_data;
}