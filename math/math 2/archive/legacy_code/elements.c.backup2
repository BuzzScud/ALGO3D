/*
 * Crystalline UI Elements - Implementation
 * 
 * Custom UI elements supporting both crystalline and rectangular styles
 * ALL calculations use prime_* functions
 */

#include "elements.h"
#include <stdlib.h>
#include <string.h>

/*
 * Helper Functions
 */

static SDL_Color get_element_color(CrystallineElementBase* base) {
    switch (base->state) {
        case CRYSTALLINE_STATE_HOVER:
            return base->hover_color;
        case CRYSTALLINE_STATE_ACTIVE:
            return base->active_color;
        case CRYSTALLINE_STATE_DISABLED:
            return base->disabled_color;
        case CRYSTALLINE_STATE_FOCUSED:
        case CRYSTALLINE_STATE_NORMAL:
        default:
            return base->color;
    }
}

/*
 * Button Implementation
 */

CrystallineButton* crystalline_button_create(CrystallineElementStyle style,
                                              float x, float y,
                                              float size_param1,
                                              float size_param2,
                                              const char* label,
                                              TTF_Font* font) {
    CrystallineButton* button = (CrystallineButton*)malloc(sizeof(CrystallineButton));
    if (!button) return NULL;
    
    memset(button, 0, sizeof(CrystallineButton));
    
    // Initialize base properties
    button->base.style = style;
    button->base.state = CRYSTALLINE_STATE_NORMAL;
    button->base.position = crystalline_point_cartesian(x, y);
    button->base.visible = true;
    button->base.enabled = true;
    
    // Set default colors (stronger contrast for visibility)
    button->base.color = crystalline_color_rgb(60, 100, 180);
    button->base.hover_color = crystalline_color_rgb(100, 150, 230);
    button->base.active_color = crystalline_color_rgb(40, 80, 160);
    button->base.active_color = crystalline_color_rgb(80, 130, 180);
    button->base.disabled_color = crystalline_color_rgb(100, 100, 100);
    
    // Set label and font
    if (label) {
        strncpy(button->label, label, sizeof(button->label) - 1);
    }
    button->font = font;
    
    // Set size based on style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        button->radius = size_param1;
        button->width = size_param1 * 2.0f;
        button->height = size_param1 * 2.0f;
        button->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        button->width = size_param1;
        button->height = size_param2;
        button->radius = prime_fminf(size_param1, size_param2) / 2.0f;
        button->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    // Initialize pulse animation
    button->pulse = crystalline_anim_pulse(2.0f, 0.1f);  // 2 Hz, 10% amplitude
    button->pulse.base_value = 1.0f;
    
    return button;
}

void crystalline_button_destroy(CrystallineButton* button) {
    if (button) {
        free(button);
    }
}

void crystalline_button_render(CrystallineButton* button, SDL_Renderer* renderer) {
    if (!button || !button->base.visible) return;
    
    SDL_Color color = get_element_color(&button->base);
    
    // Get pulse value for animation
    float pulse_scale = crystalline_pulse_current(&button->pulse);
    
    if (button->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Circular button rendering
        float radius = button->radius * pulse_scale;
        
        // Draw outer glow for all buttons (makes them stand out)
        float glow_radius = radius + 4.0f;
        int glow_alpha = 100;
        
        if (button->base.state == CRYSTALLINE_STATE_HOVER) {
            glow_radius = radius + 6.0f;
            glow_alpha = 200;
        }
        
        CrystallineDrawStyle glow_style = crystalline_draw_style_stroked(
            crystalline_color_rgba(255, 255, 255, glow_alpha),
            2.0f
        );
        crystalline_draw_circle(renderer, button->base.position, glow_radius, glow_style);
        
        // Draw button circle with white border for visibility
        CrystallineDrawStyle style = crystalline_draw_style_both(
            color,
            crystalline_color_rgb(255, 255, 255),  // White border
            3.0f  // Thicker border
        );
        crystalline_draw_circle(renderer, button->base.position, radius, style);
        
        // Draw additional glow effect on hover
        if (button->base.state == CRYSTALLINE_STATE_HOVER) {
            crystalline_draw_glow(renderer, button->base.position, radius, 
                                 button->base.hover_color, 0.5f);  // Stronger glow
        }
        
        // Draw label
        if (button->font && button->label[0]) {
            crystalline_draw_text_centered(renderer, button->label, 
                                          button->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          button->font);
        }
    } else {
        // Rectangular button rendering
        float width = button->width * pulse_scale;
        float height = button->height * pulse_scale;
        
        CrystallineRect rect = crystalline_rect_create(
            button->base.position.x,
            button->base.position.y,
            width,
            height
        );
        
        // Draw button rectangle
        CrystallineDrawStyle style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_rect(renderer, rect, style);
        
        // Draw label
        if (button->font && button->label[0]) {
            crystalline_draw_text_centered(renderer, button->label,
                                          button->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          button->font);
        }
    }
}

void crystalline_button_update(CrystallineButton* button, float delta_time) {
    if (!button) return;
    
    // Update pulse animation only when hovered or active
    if (button->base.state == CRYSTALLINE_STATE_HOVER || 
        button->base.state == CRYSTALLINE_STATE_ACTIVE) {
        if (button->pulse.base.state != CRYSTALLINE_ANIM_PLAYING) {
            crystalline_anim_start(&button->pulse.base);
        }
        crystalline_anim_pulse_update(&button->pulse, delta_time);
    } else {
        if (button->pulse.base.state == CRYSTALLINE_ANIM_PLAYING) {
            crystalline_anim_stop(&button->pulse.base);
        }
    }
}

bool crystalline_button_handle_mouse(CrystallineButton* button, SDL_Event* event) {
    if (!button || !button->base.visible || !button->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    
    bool inside = crystalline_rect_contains_point(button->base.bounds, 
                                                   crystalline_point_cartesian(mouse_x, mouse_y));
    
    switch (event->type) {
        case SDL_MOUSEMOTION:
            if (inside) {
                if (button->base.state == CRYSTALLINE_STATE_NORMAL) {
                    button->base.state = CRYSTALLINE_STATE_HOVER;
                }
            } else {
                if (button->base.state == CRYSTALLINE_STATE_HOVER) {
                    button->base.state = CRYSTALLINE_STATE_NORMAL;
                }
            }
            return inside;
            
        case SDL_MOUSEBUTTONDOWN:
            if (inside && event->button.button == SDL_BUTTON_LEFT) {
                button->base.state = CRYSTALLINE_STATE_ACTIVE;
                return true;
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (button->base.state == CRYSTALLINE_STATE_ACTIVE) {
                if (inside && event->button.button == SDL_BUTTON_LEFT) {
                    // Trigger click callback
                    if (button->on_click) {
                        button->on_click(button->base.user_data);
                    }
                }
                button->base.state = inside ? CRYSTALLINE_STATE_HOVER : CRYSTALLINE_STATE_NORMAL;
                return true;
            }
            break;
    }
    
    return false;
}

void crystalline_button_set_label(CrystallineButton* button, const char* label) {
    if (!button || !label) return;
    strncpy(button->label, label, sizeof(button->label) - 1);
    button->label[sizeof(button->label) - 1] = '\0';
}

void crystalline_button_set_callback(CrystallineButton* button, 
                                     void (*callback)(void* data), void* data) {
    if (!button) return;
    button->on_click = callback;
    button->base.user_data = data;
}

void crystalline_button_set_style(CrystallineButton* button, CrystallineElementStyle style) {
    if (!button) return;
    
    button->base.style = style;
    
    // Recalculate bounds based on new style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        button->base.bounds = crystalline_rect_circle(
            button->base.position.x,
            button->base.position.y,
            button->radius
        );
    } else {
        button->base.bounds = crystalline_rect_create(
            button->base.position.x,
            button->base.position.y,
            button->width,
            button->height
        );
    }
}

/*
 * Slider Implementation
 */

CrystallineSlider* crystalline_slider_create(CrystallineElementStyle style,
                                              float x, float y,
                                              float size_param1,
                                              float size_param2,
                                              float min_value,
                                              float max_value) {
    CrystallineSlider* slider = (CrystallineSlider*)malloc(sizeof(CrystallineSlider));
    if (!slider) return NULL;
    
    memset(slider, 0, sizeof(CrystallineSlider));
    
    // Initialize base properties
    slider->base.style = style;
    slider->base.state = CRYSTALLINE_STATE_NORMAL;
    slider->base.position = crystalline_point_cartesian(x, y);
    slider->base.visible = true;
    slider->base.enabled = true;
    
    // Set default colors
    slider->base.color = crystalline_color_rgb(100, 150, 200);
    slider->base.hover_color = crystalline_color_rgb(120, 170, 220);
    slider->base.active_color = crystalline_color_rgb(80, 130, 180);
    slider->base.disabled_color = crystalline_color_rgb(100, 100, 100);
    
    // Set value range
    slider->min_value = min_value;
    slider->max_value = max_value;
    slider->value = 0.5f;  // Start at middle
    slider->step = 0.0f;   // Continuous by default
    
    // Set size based on style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        slider->radius = size_param1;
        slider->start_angle = -M_PI * 0.75f;  // Start at 7:30
        slider->end_angle = M_PI * 0.75f;     // End at 4:30
        slider->base.bounds = crystalline_rect_circle(x, y, size_param1 + 20.0f);
    } else {
        slider->width = size_param1;
        slider->height = size_param2;
        slider->handle_size = prime_fminf(size_param1 * 0.1f, size_param2);
        slider->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    slider->dragging = false;
    
    return slider;
}

void crystalline_slider_destroy(CrystallineSlider* slider) {
    if (slider) {
        free(slider);
    }
}

void crystalline_slider_render(CrystallineSlider* slider, SDL_Renderer* renderer) {
    if (!slider || !slider->base.visible) return;
    
    SDL_Color color = get_element_color(&slider->base);
    
    if (slider->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Circular slider rendering (arc)
        
        // Draw track arc
        CrystallineDrawStyle track_style = crystalline_draw_style_stroked(
            crystalline_color_brightness(color, 0.5f), 4.0f
        );
        crystalline_draw_arc(renderer, slider->base.position, slider->radius,
                            slider->start_angle, slider->end_angle, track_style);
        
        // Draw filled arc up to current value
        float value_angle = slider->start_angle + 
                           (slider->end_angle - slider->start_angle) * slider->value;
        CrystallineDrawStyle value_style = crystalline_draw_style_stroked(color, 6.0f);
        crystalline_draw_arc(renderer, slider->base.position, slider->radius,
                            slider->start_angle, value_angle, value_style);
        
        // Draw handle at current position
        CrystallinePoint handle_pos = crystalline_point_polar(value_angle, slider->radius);
        handle_pos = crystalline_point_add(handle_pos, slider->base.position);
        
        CrystallineDrawStyle handle_style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_circle(renderer, handle_pos, 8.0f, handle_style);
        
        // Draw glow on hover/active
        if (slider->base.state == CRYSTALLINE_STATE_HOVER || 
            slider->base.state == CRYSTALLINE_STATE_ACTIVE) {
            crystalline_draw_glow(renderer, handle_pos, 8.0f, color, 0.5f);
        }
    } else {
        // Rectangular slider rendering (horizontal bar)
        
        // Draw track
        CrystallineRect track = crystalline_rect_create(
            slider->base.position.x,
            slider->base.position.y,
            slider->width,
            slider->height
        );
        CrystallineDrawStyle track_style = crystalline_draw_style_both(
            crystalline_color_brightness(color, 0.3f),
            crystalline_color_brightness(color, 0.5f),
            1.0f
        );
        crystalline_draw_rect(renderer, track, track_style);
        
        // Draw filled portion
        float filled_width = slider->width * slider->value;
        CrystallineRect filled = crystalline_rect_create(
            slider->base.position.x - slider->width / 2.0f + filled_width / 2.0f,
            slider->base.position.y,
            filled_width,
            slider->height
        );
        CrystallineDrawStyle filled_style = crystalline_draw_style_filled(color);
        crystalline_draw_rect(renderer, filled, filled_style);
        
        // Draw handle
        float handle_x = slider->base.position.x - slider->width / 2.0f + 
                        slider->width * slider->value;
        CrystallinePoint handle_pos = crystalline_point_cartesian(
            handle_x, slider->base.position.y
        );
        
        CrystallineDrawStyle handle_style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_circle(renderer, handle_pos, slider->handle_size, handle_style);
    }
    
    // Draw label if present
    if (slider->font && slider->label[0]) {
        CrystallinePoint label_pos = crystalline_point_cartesian(
            slider->base.position.x,
            slider->base.position.y - slider->radius - 20.0f
        );
        crystalline_draw_text_centered(renderer, slider->label, label_pos,
                                      crystalline_color_rgb(255, 255, 255),
                                      slider->font);
    }
}

void crystalline_slider_update(CrystallineSlider* slider, float delta_time) {
    if (!slider) return;
    // Slider doesn't need continuous updates (yet)
    (void)delta_time;
}

bool crystalline_slider_handle_mouse(CrystallineSlider* slider, SDL_Event* event) {
    if (!slider || !slider->base.visible || !slider->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    CrystallinePoint mouse_pos = crystalline_point_cartesian(mouse_x, mouse_y);
    
    switch (event->type) {
        case SDL_MOUSEMOTION: {
            if (slider->dragging) {
                // Update value based on mouse position
                if (slider->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
                    // Calculate angle from center to mouse
                    float angle = crystalline_angle_between(slider->base.position, mouse_pos);
                    
                    // Clamp to slider range
                    float range = slider->end_angle - slider->start_angle;
                    float normalized = (angle - slider->start_angle) / range;
                    slider->value = crystalline_clamp(normalized, 0.0f, 1.0f);
                } else {
                    // Calculate position along track
                    float track_start = slider->base.position.x - slider->width / 2.0f;
                    float normalized = (mouse_x - track_start) / slider->width;
                    slider->value = crystalline_clamp(normalized, 0.0f, 1.0f);
                }
                
                // Apply step if set
                if (slider->step > 0.0f) {
                    float range = slider->max_value - slider->min_value;
                    float steps = range / slider->step;
                    slider->value = prime_round(slider->value * steps) / steps;
                }
                
                // Trigger callback
                if (slider->on_change) {
                    float actual_value = slider->min_value + 
                                       (slider->max_value - slider->min_value) * slider->value;
                    slider->on_change(actual_value, slider->base.user_data);
                }
                
                return true;
            } else {
                // Check for hover
                bool inside = crystalline_rect_contains_point(slider->base.bounds, mouse_pos);
                slider->base.state = inside ? CRYSTALLINE_STATE_HOVER : CRYSTALLINE_STATE_NORMAL;
                return inside;
            }
        }
            
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                bool inside = crystalline_rect_contains_point(slider->base.bounds, mouse_pos);
                if (inside) {
                    slider->dragging = true;
                    slider->base.state = CRYSTALLINE_STATE_ACTIVE;
                    return true;
                }
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT && slider->dragging) {
                slider->dragging = false;
                bool inside = crystalline_rect_contains_point(slider->base.bounds, mouse_pos);
                slider->base.state = inside ? CRYSTALLINE_STATE_HOVER : CRYSTALLINE_STATE_NORMAL;
                return true;
            }
            break;
    }
    
    return false;
}

void crystalline_slider_set_value(CrystallineSlider* slider, float value) {
    if (!slider) return;
    
    // Normalize to [0, 1]
    slider->value = (value - slider->min_value) / (slider->max_value - slider->min_value);
    slider->value = crystalline_clamp(slider->value, 0.0f, 1.0f);
}

void crystalline_slider_set_range(CrystallineSlider* slider, float min, float max) {
    if (!slider) return;
    slider->min_value = min;
    slider->max_value = max;
}

void crystalline_slider_set_callback(CrystallineSlider* slider,
                                     void (*callback)(float value, void* data), void* data) {
    if (!slider) return;
    slider->on_change = callback;
    slider->base.user_data = data;
}

float crystalline_slider_get_value(CrystallineSlider* slider) {
    if (!slider) return 0.0f;
    return slider->min_value + (slider->max_value - slider->min_value) * slider->value;
}

void crystalline_slider_set_style(CrystallineSlider* slider, CrystallineElementStyle style) {
    if (!slider) return;
    slider->base.style = style;
    
    // Recalculate bounds
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        slider->base.bounds = crystalline_rect_circle(
            slider->base.position.x,
            slider->base.position.y,
            slider->radius + 20.0f
        );
    } else {
        slider->base.bounds = crystalline_rect_create(
            slider->base.position.x,
            slider->base.position.y,
            slider->width,
            slider->height
        );
    }
}

/*
 * Utility Functions
 */

bool crystalline_element_contains_point(CrystallineElementBase* base, float x, float y) {
    if (!base) return false;
    CrystallinePoint point = crystalline_point_cartesian(x, y);
    return crystalline_rect_contains_point(base->bounds, point);
}

void crystalline_element_set_state(CrystallineElementBase* base, CrystallineElementState state) {
    if (!base) return;
    base->state = state;
}

void crystalline_element_set_enabled(CrystallineElementBase* base, bool enabled) {
    if (!base) return;
    base->enabled = enabled;
    if (!enabled) {
        base->state = CRYSTALLINE_STATE_DISABLED;
    }
}

void crystalline_element_set_visible(CrystallineElementBase* base, bool visible) {
    if (!base) return;
    base->visible = visible;
}

void crystalline_element_set_colors(CrystallineElementBase* base,
                                    SDL_Color normal,
                                    SDL_Color hover,
                                    SDL_Color active,
                                    SDL_Color disabled) {
    if (!base) return;
    base->color = normal;
    base->hover_color = hover;
    base->active_color = active;
    base->disabled_color = disabled;
}
/*
 * Progress Bar Implementation
 */

CrystallineProgress* crystalline_progress_create(CrystallineElementStyle style,
                                                  float x, float y,
                                                  float size_param1,
                                                  float size_param2) {
    CrystallineProgress* progress = (CrystallineProgress*)malloc(sizeof(CrystallineProgress));
    if (!progress) return NULL;
    
    memset(progress, 0, sizeof(CrystallineProgress));
    
    // Initialize base properties
    progress->base.style = style;
    progress->base.state = CRYSTALLINE_STATE_NORMAL;
    progress->base.position = crystalline_point_cartesian(x, y);
    progress->base.visible = true;
    progress->base.enabled = true;
    
    // Set default colors
    progress->base.color = crystalline_color_rgb(100, 200, 100);
    progress->base.hover_color = progress->base.color;
    progress->base.active_color = progress->base.color;
    progress->base.disabled_color = crystalline_color_rgb(100, 100, 100);
    
    // Initialize progress
    progress->progress = 0.0f;
    progress->show_percentage = true;
    
    // Set size based on style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        progress->radius = size_param1;
        progress->thickness = size_param2 > 0 ? size_param2 : 10.0f;
        progress->start_angle = -M_PI / 2.0f;  // Start at top
        progress->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        progress->width = size_param1;
        progress->height = size_param2;
        progress->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    // Initialize color animation
    progress->color_anim = crystalline_anim_color(
        crystalline_color_activity(0.0f),
        crystalline_color_activity(1.0f),
        1.0f
    );
    
    return progress;
}

void crystalline_progress_destroy(CrystallineProgress* progress) {
    if (progress) {
        free(progress);
    }
}

void crystalline_progress_render(CrystallineProgress* progress, SDL_Renderer* renderer) {
    if (!progress || !progress->base.visible) return;
    
    // Get color based on progress (activity-based)
    SDL_Color color = crystalline_color_activity(progress->progress);
    
    if (progress->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Circular progress (ring)
        
        // Draw background ring
        CrystallineDrawStyle bg_style = crystalline_draw_style_stroked(
            crystalline_color_brightness(color, 0.3f), progress->thickness
        );
        crystalline_draw_circle(renderer, progress->base.position, progress->radius, bg_style);
        
        // Draw progress arc
        float end_angle = progress->start_angle + CRYSTALLINE_TWO_PI * progress->progress;
        CrystallineDrawStyle progress_style = crystalline_draw_style_stroked(
            color, progress->thickness
        );
        crystalline_draw_arc(renderer, progress->base.position, progress->radius,
                            progress->start_angle, end_angle, progress_style);
        
        // Draw percentage text
        if (progress->show_percentage && progress->font) {
            char text[32];
            snprintf(text, sizeof(text), "%.0f%%", progress->progress * 100.0f);
            crystalline_draw_text_centered(renderer, text, progress->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          progress->font);
        }
    } else {
        // Rectangular progress bar
        
        // Draw background
        CrystallineDrawStyle bg_style = crystalline_draw_style_filled(
            crystalline_color_brightness(color, 0.3f)
        );
        crystalline_draw_rect(renderer, progress->base.bounds, bg_style);
        
        // Draw progress fill
        float filled_width = progress->width * progress->progress;
        CrystallineRect filled = crystalline_rect_create(
            progress->base.position.x - progress->width / 2.0f + filled_width / 2.0f,
            progress->base.position.y,
            filled_width,
            progress->height
        );
        CrystallineDrawStyle fill_style = crystalline_draw_style_filled(color);
        crystalline_draw_rect(renderer, filled, fill_style);
        
        // Draw border
        CrystallineDrawStyle border_style = crystalline_draw_style_stroked(
            crystalline_color_brightness(color, 0.8f), 2.0f
        );
        crystalline_draw_rect(renderer, progress->base.bounds, border_style);
        
        // Draw percentage text
        if (progress->show_percentage && progress->font) {
            char text[32];
            snprintf(text, sizeof(text), "%.0f%%", progress->progress * 100.0f);
            crystalline_draw_text_centered(renderer, text, progress->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          progress->font);
        }
    }
}

void crystalline_progress_update(CrystallineProgress* progress, float delta_time) {
    if (!progress) return;
    
    // Update color animation based on progress
    crystalline_anim_color_update(&progress->color_anim, delta_time);
}

void crystalline_progress_set_value(CrystallineProgress* progress, float value) {
    if (!progress) return;
    progress->progress = crystalline_clamp(value, 0.0f, 1.0f);
}

void crystalline_progress_set_label(CrystallineProgress* progress, const char* label) {
    if (!progress || !label) return;
    strncpy(progress->label, label, sizeof(progress->label) - 1);
    progress->label[sizeof(progress->label) - 1] = '\0';
}

float crystalline_progress_get_value(CrystallineProgress* progress) {
    if (!progress) return 0.0f;
    return progress->progress;
}

void crystalline_progress_set_style(CrystallineProgress* progress, CrystallineElementStyle style) {
    if (!progress) return;
    progress->base.style = style;
    
    // Recalculate bounds
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        progress->base.bounds = crystalline_rect_circle(
            progress->base.position.x,
            progress->base.position.y,
            progress->radius
        );
    } else {
        progress->base.bounds = crystalline_rect_create(
            progress->base.position.x,
            progress->base.position.y,
            progress->width,
            progress->height
        );
    }
}

/*
 * Input Field Implementation (Stub - Full implementation would be extensive)
 */

CrystallineInput* crystalline_input_create(CrystallineElementStyle style,
                                            float x, float y,
                                            float size_param1,
                                            float size_param2,
                                            const char* placeholder,
                                            TTF_Font* font) {
    CrystallineInput* input = (CrystallineInput*)malloc(sizeof(CrystallineInput));
    if (!input) return NULL;
    
    memset(input, 0, sizeof(CrystallineInput));
    
    input->base.style = style;
    input->base.state = CRYSTALLINE_STATE_NORMAL;
    input->base.position = crystalline_point_cartesian(x, y);
    input->base.visible = true;
    input->base.enabled = true;
    
    input->base.color = crystalline_color_rgb(80, 80, 100);
    input->base.hover_color = crystalline_color_rgb(100, 100, 120);
    input->base.active_color = crystalline_color_rgb(120, 120, 140);
    input->base.disabled_color = crystalline_color_rgb(60, 60, 60);
    
    if (placeholder) {
        strncpy(input->placeholder, placeholder, sizeof(input->placeholder) - 1);
    }
    input->font = font;
    
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        input->radius = size_param1;
        input->sides = 12;  // Dodecagon
        input->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        input->width = size_param1;
        input->height = size_param2;
        input->padding = 5.0f;
        input->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    input->cursor_anim = crystalline_anim_pulse(1.0f, 1.0f);
    
    return input;
}

void crystalline_input_destroy(CrystallineInput* input) {
    if (input) {
        free(input);
    }
}

void crystalline_input_render(CrystallineInput* input, SDL_Renderer* renderer) {
    if (!input || !input->base.visible) return;
    
    SDL_Color color = get_element_color(&input->base);
    
    if (input->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Draw dodecagon border
        CrystallineDrawStyle style = crystalline_draw_style_stroked(color, 2.0f);
        crystalline_draw_polygon(renderer, input->base.position, input->radius, 
                                input->sides, 0.0f, style);
    } else {
        // Draw rectangular border
        CrystallineDrawStyle bg_style = crystalline_draw_style_filled(
            crystalline_color_brightness(color, 0.2f)
        );
        crystalline_draw_rect(renderer, input->base.bounds, bg_style);
        
        CrystallineDrawStyle border_style = crystalline_draw_style_stroked(color, 2.0f);
        crystalline_draw_rect(renderer, input->base.bounds, border_style);
    }
    
    // Draw text or placeholder
    const char* display_text = input->text[0] ? input->text : input->placeholder;
    if (input->font && display_text[0]) {
        SDL_Color text_color = input->text[0] ? 
            crystalline_color_rgb(255, 255, 255) : 
            crystalline_color_rgb(150, 150, 150);
        crystalline_draw_text_centered(renderer, display_text, input->base.position,
                                      text_color, input->font);
    }
}

void crystalline_input_update(CrystallineInput* input, float delta_time) {
    if (!input) return;
    crystalline_anim_pulse_update(&input->cursor_anim, delta_time);
}

bool crystalline_input_handle_mouse(CrystallineInput* input, SDL_Event* event) {
    if (!input || !input->base.visible || !input->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    CrystallinePoint mouse_pos = crystalline_point_cartesian(mouse_x, mouse_y);
    
    bool inside = crystalline_rect_contains_point(input->base.bounds, mouse_pos);
    
    if (event->type == SDL_MOUSEBUTTONDOWN && inside) {
        input->base.state = CRYSTALLINE_STATE_FOCUSED;
        return true;
    }
    
    return false;
}

bool crystalline_input_handle_keyboard(CrystallineInput* input, SDL_Event* event) {
    if (!input || input->base.state != CRYSTALLINE_STATE_FOCUSED) return false;
    
    if (event->type == SDL_KEYDOWN) {
        // Basic text input handling (simplified)
        if (event->key.keysym.sym == SDLK_BACKSPACE && input->cursor_pos > 0) {
            input->text[--input->cursor_pos] = '\0';
            if (input->on_change) {
                input->on_change(input->text, input->base.user_data);
            }
            return true;
        } else if (event->key.keysym.sym == SDLK_RETURN) {
            if (input->on_submit) {
                input->on_submit(input->text, input->base.user_data);
            }
            return true;
        }
    } else if (event->type == SDL_TEXTINPUT) {
        // Add character
        if (input->cursor_pos < (int)sizeof(input->text) - 1) {
            input->text[input->cursor_pos++] = event->text.text[0];
            input->text[input->cursor_pos] = '\0';
            if (input->on_change) {
                input->on_change(input->text, input->base.user_data);
            }
            return true;
        }
    }
    
    return false;
}

void crystalline_input_set_text(CrystallineInput* input, const char* text) {
    if (!input || !text) return;
    strncpy(input->text, text, sizeof(input->text) - 1);
    input->text[sizeof(input->text) - 1] = '\0';
    input->cursor_pos = strlen(input->text);
}

void crystalline_input_set_placeholder(CrystallineInput* input, const char* placeholder) {
    if (!input || !placeholder) return;
    strncpy(input->placeholder, placeholder, sizeof(input->placeholder) - 1);
    input->placeholder[sizeof(input->placeholder) - 1] = '\0';
}

void crystalline_input_set_password_mode(CrystallineInput* input, bool enabled) {
    if (!input) return;
    input->password_mode = enabled;
}

void crystalline_input_set_callbacks(CrystallineInput* input,
                                     void (*on_change)(const char* text, void* data),
                                     void (*on_submit)(const char* text, void* data),
                                     void* data) {
    if (!input) return;
    input->on_change = on_change;
    input->on_submit = on_submit;
    input->base.user_data = data;
}

const char* crystalline_input_get_text(CrystallineInput* input) {
    if (!input) return "";
    return input->text;
}

void crystalline_input_set_style(CrystallineInput* input, CrystallineElementStyle style) {
    if (!input) return;
    input->base.style = style;
}

/*
 * Stub implementations for remaining elements
 * (Panel, List, Toggle, Dropdown - to be fully implemented)
 */

CrystallinePanel* crystalline_panel_create(CrystallineElementStyle style,
                                            float x, float y,
                                            float size_param1,
                                            float size_param2,
                                            const char* title,
                                            TTF_Font* title_font) {
    CrystallinePanel* panel = (CrystallinePanel*)malloc(sizeof(CrystallinePanel));
    if (!panel) return NULL;
    memset(panel, 0, sizeof(CrystallinePanel));
    
    panel->base.style = style;
    panel->base.position = crystalline_point_cartesian(x, y);
    panel->base.visible = true;
    panel->base.enabled = true;
    
    if (title) strncpy(panel->title, title, sizeof(panel->title) - 1);
    panel->title_font = title_font;
    
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        panel->radius = size_param1;
        panel->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        panel->width = size_param1;
        panel->height = size_param2;
        panel->corner_radius = 5.0f;
        panel->border_width = 2.0f;
        panel->padding = 10.0f;
        panel->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    return panel;
}

void crystalline_panel_destroy(CrystallinePanel* panel) {
    if (panel) free(panel);
}

void crystalline_panel_render(CrystallinePanel* panel, SDL_Renderer* renderer) {
    if (!panel || !panel->base.visible) return;
    
    SDL_Color color = crystalline_color_rgb(40, 40, 50);
    SDL_Color border_color = crystalline_color_rgb(150, 150, 180);  // Brighter border
    
    if (panel->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        CrystallineDrawStyle style = crystalline_draw_style_both(color, border_color, 3.0f);  // Thicker border
        crystalline_draw_circle(renderer, panel->base.position, panel->radius, style);
        
        if (panel->show_flower_border) {
            CrystallineDrawStyle flower_style = crystalline_draw_style_stroked(border_color, 1.0f);
            crystalline_draw_flower_of_life(renderer, panel->base.position, 
                                           panel->radius / 3.0f, 1, flower_style);
        }
    } else {
        CrystallineDrawStyle bg_style = crystalline_draw_style_filled(color);
        crystalline_draw_rect(renderer, panel->base.bounds, bg_style);
        
        CrystallineDrawStyle border_style = crystalline_draw_style_stroked(border_color, panel->border_width);
        crystalline_draw_rect(renderer, panel->base.bounds, border_style);
    }
    
    // Draw title
    if (panel->title_font && panel->title[0]) {
        CrystallinePoint title_pos = crystalline_point_cartesian(
            panel->base.position.x,
            panel->base.position.y - panel->height / 2.0f + 20.0f
        );
        crystalline_draw_text_centered(renderer, panel->title, title_pos,
                                      crystalline_color_rgb(255, 255, 255),
                                      panel->title_font);
    }
}

void crystalline_panel_set_style(CrystallinePanel* panel, CrystallineElementStyle style) {
    if (!panel) return;
    panel->base.style = style;
}


/*
 * List Implementation - COMPLETE
 */

CrystallineList* crystalline_list_create(CrystallineElementStyle style, float x, float y,
                                          float size_param1, float size_param2, TTF_Font* font) {
    CrystallineList* list = (CrystallineList*)malloc(sizeof(CrystallineList));
    if (!list) return NULL;
    
    memset(list, 0, sizeof(CrystallineList));
    
    list->base.style = style;
    list->base.state = CRYSTALLINE_STATE_NORMAL;
    list->base.position = crystalline_point_cartesian(x, y);
    list->base.visible = true;
    list->base.enabled = true;
    
    list->base.color = crystalline_color_rgb(60, 60, 80);
    list->base.hover_color = crystalline_color_rgb(80, 80, 100);
    list->base.active_color = crystalline_color_rgb(100, 150, 200);
    list->base.disabled_color = crystalline_color_rgb(40, 40, 40);
    
    list->font = font;
    list->selected_index = -1;
    list->hover_index = -1;
    list->items = NULL;
    list->item_count = 0;
    
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        list->radius = size_param1;
        list->use_clock_lattice = true;
        list->ring = 1;
        list->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        list->width = size_param1;
        list->item_height = size_param2 > 0 ? size_param2 : 30.0f;
        list->visible_items = 10;
        list->base.bounds = crystalline_rect_create(x, y, size_param1, 
                                                     list->item_height * list->visible_items);
    }
    
    list->scroll_offset = 0.0f;
    list->scrollable = true;
    
       
       // Initialize checkbox support
       list->item_checked = NULL;
       list->show_checkboxes = false;
       list->checkbox_size = 12.0f;
    return list;
}

void crystalline_list_destroy(CrystallineList* list) {
    if (list) {
        if (list->items) {
            for (int i = 0; i < list->item_count; i++) {
                if (list->items[i]) free(list->items[i]);
            }
            free(list->items);
        }
        free(list);
    }
}

           if (list->item_checked) free(list->item_checked);
void crystalline_list_render(CrystallineList* list, SDL_Renderer* renderer) {
    if (!list || !list->base.visible || !list->font) return;
    
    if (list->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Circular list - items on clock lattice
        int positions = crystalline_clock_ring_positions(list->ring);
        int items_to_show = list->item_count < positions ? list->item_count : positions;
        
        for (int i = 0; i < items_to_show; i++) {
            if (!list->items[i]) continue;
            
            // Calculate position on clock lattice
            CrystallinePoint item_pos = crystalline_point_clock(list->ring, i + 1, list->radius);
            item_pos = crystalline_point_add(item_pos, list->base.position);
            
            // Determine color based on state
            SDL_Color color;
            if (i == list->selected_index) {
                color = list->base.active_color;
            } else if (i == list->hover_index) {
                color = list->base.hover_color;
            } else {
                color = list->base.color;
            }
            
            // Draw item background circle
            CrystallineDrawStyle bg_style = crystalline_draw_style_filled(color);
            crystalline_draw_circle(renderer, item_pos, 15.0f, bg_style);
            
            // Draw item text
            crystalline_draw_text_centered(renderer, list->items[i], item_pos,
                                          crystalline_color_rgb(255, 255, 255), list->font);
        }
    } else {
        // Rectangular list
        float item_y = list->base.position.y - (list->visible_items * list->item_height) / 2.0f;
        
        int start_index = (int)(list->scroll_offset / list->item_height);
        int end_index = start_index + list->visible_items;
        if (end_index > list->item_count) end_index = list->item_count;
        
        for (int i = start_index; i < end_index; i++) {
            if (!list->items[i]) continue;
            
            float y = item_y + (i - start_index) * list->item_height;
            CrystallinePoint item_pos = crystalline_point_cartesian(list->base.position.x, y);
            
            // Determine color
            SDL_Color color;
            if (i == list->selected_index) {
                color = list->base.active_color;
            } else if (i == list->hover_index) {
                color = list->base.hover_color;
            } else {
                color = list->base.color;
            }
            
            // Draw item background
            CrystallineRect item_rect = crystalline_rect_create(
                list->base.position.x, y, list->width, list->item_height
            );
            CrystallineDrawStyle bg_style = crystalline_draw_style_filled(color);
            crystalline_draw_rect(renderer, item_rect, bg_style);
            
            // Draw item text
            crystalline_draw_text_centered(renderer, list->items[i], item_pos,
                                          crystalline_color_rgb(255, 255, 255), list->font);
        }
        
        // Draw border
        CrystallineDrawStyle border_style = crystalline_draw_style_stroked(
            crystalline_color_rgb(100, 100, 120), 2.0f
        );
        crystalline_draw_rect(renderer, list->base.bounds, border_style);
    }
}

bool crystalline_list_handle_mouse(CrystallineList* list, SDL_Event* event) {
    if (!list || !list->base.visible || !list->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    CrystallinePoint mouse_pos = crystalline_point_cartesian(mouse_x, mouse_y);
    
    if (event->type == SDL_MOUSEMOTION) {
        // Update hover index
        list->hover_index = -1;
        
        if (list->base.style == CRYSTALLINE_STYLE_RECTANGULAR) {
            float item_y = list->base.position.y - (list->visible_items * list->item_height) / 2.0f;
            int start_index = (int)(list->scroll_offset / list->item_height);
            
            for (int i = 0; i < list->visible_items && (start_index + i) < list->item_count; i++) {
                float y = item_y + i * list->item_height;
                CrystallineRect item_rect = crystalline_rect_create(
                    list->base.position.x, y, list->width, list->item_height
                );
                
                if (crystalline_rect_contains_point(item_rect, mouse_pos)) {
                    list->hover_index = start_index + i;
                    return true;
                }
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (list->hover_index >= 0) {
            list->selected_index = list->hover_index;
            if (list->on_select) {
                list->on_select(list->selected_index, list->base.user_data);
            }
            return true;
        }
    }
    
    return false;
}

bool crystalline_list_handle_keyboard(CrystallineList* list, SDL_Event* event) {
    if (!list || !list->base.enabled || list->item_count == 0) return false;
    
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                if (list->selected_index > 0) {
                    list->selected_index--;
                    if (list->on_select) {
                        list->on_select(list->selected_index, list->base.user_data);
                    }
                    return true;
                }
                break;
            case SDLK_DOWN:
                if (list->selected_index < list->item_count - 1) {
                    list->selected_index++;
                    if (list->on_select) {
                        list->on_select(list->selected_index, list->base.user_data);
                    }
                    return true;
                }
                break;
        }
    }
    
    return false;
}

void crystalline_list_set_items(CrystallineList* list, char** items, int count) {
    if (!list) return;
    
    // Free existing items
    if (list->items) {
        for (int i = 0; i < list->item_count; i++) {
            if (list->items[i]) free(list->items[i]);
        }
        free(list->items);
    }
    
    // Allocate new items
    list->items = (char**)malloc(sizeof(char*) * count);
    list->item_count = count;
    
    for (int i = 0; i < count; i++) {
        list->items[i] = strdup(items[i]);
    }
}

void crystalline_list_add_item(CrystallineList* list, const char* item) {
    if (!list || !item) return;
    
    list->items = (char**)realloc(list->items, sizeof(char*) * (list->item_count + 1));
    list->items[list->item_count] = strdup(item);
    list->item_count++;
}

void crystalline_list_remove_item(CrystallineList* list, int index) {
    if (!list || index < 0 || index >= list->item_count) return;
    
    free(list->items[index]);
    
    for (int i = index; i < list->item_count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    
    list->item_count--;
}

void crystalline_list_clear(CrystallineList* list) {
    if (!list) return;
    
    for (int i = 0; i < list->item_count; i++) {
        if (list->items[i]) free(list->items[i]);
    }
    free(list->items);
    
    list->items = NULL;
    list->item_count = 0;
    list->selected_index = -1;
    list->hover_index = -1;
}

void crystalline_list_set_selected(CrystallineList* list, int index) {
    if (!list) return;
    list->selected_index = index;
}

void crystalline_list_set_callback(CrystallineList* list, void (*callback)(int, void*), void* data) {
    if (!list) return;
    list->on_select = callback;
    list->base.user_data = data;
}

int crystalline_list_get_selected(CrystallineList* list) {
    return list ? list->selected_index : -1;
}

void crystalline_list_set_style(CrystallineList* list, CrystallineElementStyle style) {
    if (!list) return;
    list->base.style = style;
}

/*
 * Toggle Implementation - COMPLETE
 */

CrystallineToggle* crystalline_toggle_create(CrystallineElementStyle style, float x, float y,
                                              float size_param1, float size_param2,
                                              const char* label, TTF_Font* font) {
    CrystallineToggle* toggle = (CrystallineToggle*)malloc(sizeof(CrystallineToggle));
    if (!toggle) return NULL;
    
    memset(toggle, 0, sizeof(CrystallineToggle));
    
    toggle->base.style = style;
    toggle->base.state = CRYSTALLINE_STATE_NORMAL;
    toggle->base.position = crystalline_point_cartesian(x, y);
    toggle->base.visible = true;
    toggle->base.enabled = true;
    
    toggle->base.color = crystalline_color_rgb(100, 100, 120);
    toggle->base.hover_color = crystalline_color_rgb(120, 120, 140);
    toggle->base.active_color = crystalline_color_rgb(100, 200, 100);
    toggle->base.disabled_color = crystalline_color_rgb(60, 60, 60);
    
    if (label) strncpy(toggle->label, label, sizeof(toggle->label) - 1);
    toggle->font = font;
    toggle->value = false;
    
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        toggle->radius = size_param1;
        toggle->use_yinyang = true;
        toggle->base.bounds = crystalline_rect_circle(x, y, size_param1);
        
        // Initialize rotation animation
        toggle->rotation = crystalline_anim_rotation(0.0f, M_PI, 0.3f);
    } else {
        toggle->width = size_param1;
        toggle->height = size_param2;
        toggle->handle_size = size_param2 * 0.8f;
        toggle->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
        
        // Initialize handle animation
        CrystallinePoint off_pos = crystalline_point_cartesian(
            x - size_param1 / 2.0f + toggle->handle_size / 2.0f, y
        );
        CrystallinePoint on_pos = crystalline_point_cartesian(
            x + size_param1 / 2.0f - toggle->handle_size / 2.0f, y
        );
        toggle->handle_anim = crystalline_anim_position(off_pos, on_pos, 0.3f);
    }
    
    return toggle;
}

void crystalline_toggle_destroy(CrystallineToggle* toggle) {
    if (toggle) free(toggle);
}

void crystalline_toggle_render(CrystallineToggle* toggle, SDL_Renderer* renderer) {
    if (!toggle || !toggle->base.visible) return;
    
    SDL_Color off_color = toggle->base.color;
    SDL_Color on_color = toggle->base.active_color;
    SDL_Color current_color = toggle->value ? on_color : off_color;
    
    if (toggle->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Yin-yang style toggle
        float rotation = toggle->value ? M_PI : 0.0f;
        
        // Draw outer circle
        CrystallineDrawStyle outer_style = crystalline_draw_style_both(
            current_color,
            crystalline_color_brightness(current_color, 0.8f),
            2.0f
        );
        crystalline_draw_circle(renderer, toggle->base.position, toggle->radius, outer_style);
        
        // Draw yin-yang pattern (simplified)
        if (toggle->use_yinyang) {
            // Draw two half circles
            CrystallineDrawStyle yin_style = crystalline_draw_style_filled(
                crystalline_color_rgb(40, 40, 50)
            );
            CrystallineDrawStyle yang_style = crystalline_draw_style_filled(
                crystalline_color_rgb(240, 240, 250)
            );
            
            // Left half (yin)
            crystalline_draw_arc(renderer, toggle->base.position, toggle->radius,
                                rotation - M_PI / 2.0f, rotation + M_PI / 2.0f, yin_style);
            
            // Right half (yang)
            crystalline_draw_arc(renderer, toggle->base.position, toggle->radius,
                                rotation + M_PI / 2.0f, rotation + M_PI * 1.5f, yang_style);
        }
    } else {
        // Rectangular toggle switch
        
        // Draw track
        CrystallineDrawStyle track_style = crystalline_draw_style_both(
            current_color,
            crystalline_color_brightness(current_color, 0.8f),
            2.0f
        );
        crystalline_draw_rect(renderer, toggle->base.bounds, track_style);
        
        // Draw handle
        CrystallinePoint handle_pos;
        if (toggle->value) {
            handle_pos = crystalline_point_cartesian(
                toggle->base.position.x + toggle->width / 2.0f - toggle->handle_size / 2.0f,
                toggle->base.position.y
            );
        } else {
            handle_pos = crystalline_point_cartesian(
                toggle->base.position.x - toggle->width / 2.0f + toggle->handle_size / 2.0f,
                toggle->base.position.y
            );
        }
        
        CrystallineDrawStyle handle_style = crystalline_draw_style_both(
            crystalline_color_rgb(255, 255, 255),
            crystalline_color_rgb(200, 200, 200),
            2.0f
        );
        crystalline_draw_circle(renderer, handle_pos, toggle->handle_size / 2.0f, handle_style);
    }
    
    // Draw label
    if (toggle->font && toggle->label[0]) {
        CrystallinePoint label_pos = crystalline_point_cartesian(
            toggle->base.position.x,
            toggle->base.position.y + toggle->radius + 20.0f
        );
        crystalline_draw_text_centered(renderer, toggle->label, label_pos,
                                      crystalline_color_rgb(255, 255, 255), toggle->font);
    }
}

void crystalline_toggle_update(CrystallineToggle* toggle, float delta_time) {
    if (!toggle) return;
    
    if (toggle->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        crystalline_anim_rotation_update(&toggle->rotation, delta_time);
    } else {
        crystalline_anim_position_update(&toggle->handle_anim, delta_time);
    }
}

bool crystalline_toggle_handle_mouse(CrystallineToggle* toggle, SDL_Event* event) {
    if (!toggle || !toggle->base.visible || !toggle->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    CrystallinePoint mouse_pos = crystalline_point_cartesian(mouse_x, mouse_y);
    
    bool inside = crystalline_rect_contains_point(toggle->base.bounds, mouse_pos);
    
    if (event->type == SDL_MOUSEMOTION) {
        toggle->base.state = inside ? CRYSTALLINE_STATE_HOVER : CRYSTALLINE_STATE_NORMAL;
        return inside;
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT && inside) {
        toggle->value = !toggle->value;
        
        // Start animations
        if (toggle->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
            crystalline_anim_start(&toggle->rotation.base);
        } else {
            crystalline_anim_start(&toggle->handle_anim.base);
        }
        
        if (toggle->on_change) {
            toggle->on_change(toggle->value, toggle->base.user_data);
        }
        return true;
    }
    
    return false;
}

void crystalline_toggle_set_value(CrystallineToggle* toggle, bool value) {
    if (!toggle) return;
    toggle->value = value;
}

void crystalline_toggle_set_callback(CrystallineToggle* toggle, void (*callback)(bool, void*), void* data) {
    if (!toggle) return;
    toggle->on_change = callback;
    toggle->base.user_data = data;
}

bool crystalline_toggle_get_value(CrystallineToggle* toggle) {
    return toggle ? toggle->value : false;
}

void crystalline_toggle_set_style(CrystallineToggle* toggle, CrystallineElementStyle style) {
    if (!toggle) return;
    toggle->base.style = style;
}

/*
 * Dropdown Implementation - COMPLETE
 */

CrystallineDropdown* crystalline_dropdown_create(CrystallineElementStyle style, float x, float y,
                                                  float size_param1, float size_param2, TTF_Font* font) {
    CrystallineDropdown* dropdown = (CrystallineDropdown*)malloc(sizeof(CrystallineDropdown));
    if (!dropdown) return NULL;
    
    memset(dropdown, 0, sizeof(CrystallineDropdown));
    
    dropdown->base.style = style;
    dropdown->base.state = CRYSTALLINE_STATE_NORMAL;
    dropdown->base.position = crystalline_point_cartesian(x, y);
    dropdown->base.visible = true;
    dropdown->base.enabled = true;
    
    dropdown->base.color = crystalline_color_rgb(80, 80, 100);
    dropdown->base.hover_color = crystalline_color_rgb(100, 100, 120);
    dropdown->base.active_color = crystalline_color_rgb(120, 150, 200);
    dropdown->base.disabled_color = crystalline_color_rgb(60, 60, 60);
    
    dropdown->font = font;
    dropdown->selected_index = -1;
    dropdown->hover_index = -1;
    dropdown->expanded = false;
    dropdown->options = NULL;
    dropdown->option_count = 0;
    
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        dropdown->radius = size_param1;
        dropdown->use_radial_menu = true;
        dropdown->base.bounds = crystalline_rect_circle(x, y, size_param1);
        
        // Initialize spiral animation
        dropdown->expand_anim = crystalline_anim_spiral(0.0f, size_param1, 1.0f, 0.3f);
    } else {
        dropdown->width = size_param1;
        dropdown->item_height = size_param2 > 0 ? size_param2 : 30.0f;
        dropdown->max_height = dropdown->item_height * 5;  // Show max 5 items
        dropdown->base.bounds = crystalline_rect_create(x, y, size_param1, dropdown->item_height);
    }
    
    return dropdown;
}

void crystalline_dropdown_destroy(CrystallineDropdown* dropdown) {
    if (dropdown) {
        if (dropdown->options) {
            for (int i = 0; i < dropdown->option_count; i++) {
                if (dropdown->options[i]) free(dropdown->options[i]);
            }
            free(dropdown->options);
        }
        free(dropdown);
    }
}

void crystalline_dropdown_render(CrystallineDropdown* dropdown, SDL_Renderer* renderer) {
    if (!dropdown || !dropdown->base.visible || !dropdown->font) return;
    
    SDL_Color color = get_element_color(&dropdown->base);
    
    if (dropdown->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Radial menu (12-fold)
        
        // Draw center button
        CrystallineDrawStyle center_style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_circle(renderer, dropdown->base.position, 30.0f, center_style);
        
        // Draw selected option text
        if (dropdown->selected_index >= 0 && dropdown->selected_index < dropdown->option_count) {
            crystalline_draw_text_centered(renderer, dropdown->options[dropdown->selected_index],
                                          dropdown->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          dropdown->font);
        } else {
            crystalline_draw_text_centered(renderer, "Select",
                                          dropdown->base.position,
                                          crystalline_color_rgb(200, 200, 200),
                                          dropdown->font);
        }
        
        // Draw options in radial layout when expanded
        if (dropdown->expanded && dropdown->option_count > 0) {
            int options_to_show = dropdown->option_count < 12 ? dropdown->option_count : 12;
            
            for (int i = 0; i < options_to_show; i++) {
                float angle = crystalline_twelve_fold_angle(i);
                CrystallinePoint option_pos = crystalline_point_polar(angle, dropdown->radius);
                option_pos = crystalline_point_add(option_pos, dropdown->base.position);
                
                // Determine color
                SDL_Color opt_color;
                if (i == dropdown->hover_index) {
                    opt_color = dropdown->base.hover_color;
                } else if (i == dropdown->selected_index) {
                    opt_color = dropdown->base.active_color;
                } else {
                    opt_color = color;
                }
                
                // Draw option circle
                CrystallineDrawStyle opt_style = crystalline_draw_style_both(
                    opt_color,
                    crystalline_color_brightness(opt_color, 0.8f),
                    2.0f
                );
                crystalline_draw_circle(renderer, option_pos, 25.0f, opt_style);
                
                // Draw option text
                crystalline_draw_text_centered(renderer, dropdown->options[i], option_pos,
                                              crystalline_color_rgb(255, 255, 255),
                                              dropdown->font);
            }
        }
    } else {
        // Rectangular dropdown
        
        // Draw main button
        CrystallineDrawStyle button_style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_rect(renderer, dropdown->base.bounds, button_style);
        
        // Draw selected option or placeholder
        if (dropdown->selected_index >= 0 && dropdown->selected_index < dropdown->option_count) {
            crystalline_draw_text_centered(renderer, dropdown->options[dropdown->selected_index],
                                          dropdown->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          dropdown->font);
        } else {
            crystalline_draw_text_centered(renderer, "Select...",
                                          dropdown->base.position,
                                          crystalline_color_rgb(150, 150, 150),
                                          dropdown->font);
        }
        
        // Draw dropdown list when expanded
        if (dropdown->expanded && dropdown->option_count > 0) {
            int visible_options = dropdown->option_count < 5 ? dropdown->option_count : 5;
            float list_height __attribute__((unused)) = visible_options * dropdown->item_height;
            
            for (int i = 0; i < visible_options && i < dropdown->option_count; i++) {
                float y = dropdown->base.position.y + dropdown->item_height / 2.0f + 
                         (i + 1) * dropdown->item_height;
                CrystallinePoint opt_pos = crystalline_point_cartesian(dropdown->base.position.x, y);
                
                // Determine color
                SDL_Color opt_color;
                if (i == dropdown->hover_index) {
                    opt_color = dropdown->base.hover_color;
                } else if (i == dropdown->selected_index) {
                    opt_color = dropdown->base.active_color;
                } else {
                    opt_color = color;
                }
                
                // Draw option background
                CrystallineRect opt_rect = crystalline_rect_create(
                    dropdown->base.position.x, y, dropdown->width, dropdown->item_height
                );
                CrystallineDrawStyle opt_bg_style = crystalline_draw_style_filled(opt_color);
                crystalline_draw_rect(renderer, opt_rect, opt_bg_style);
                
                // Draw option border
                CrystallineDrawStyle opt_border_style = crystalline_draw_style_stroked(
                    crystalline_color_brightness(opt_color, 0.8f), 1.0f
                );
                crystalline_draw_rect(renderer, opt_rect, opt_border_style);
                
                // Draw option text
                crystalline_draw_text_centered(renderer, dropdown->options[i], opt_pos,
                                              crystalline_color_rgb(255, 255, 255),
                                              dropdown->font);
            }
        }
    }
}

void crystalline_dropdown_update(CrystallineDropdown* dropdown, float delta_time) {
    if (!dropdown) return;
    
    if (dropdown->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        crystalline_anim_spiral_update(&dropdown->expand_anim, delta_time);
    }
}

bool crystalline_dropdown_handle_mouse(CrystallineDropdown* dropdown, SDL_Event* event) {
    if (!dropdown || !dropdown->base.visible || !dropdown->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    CrystallinePoint mouse_pos = crystalline_point_cartesian(mouse_x, mouse_y);
    
    if (event->type == SDL_MOUSEMOTION) {
        if (dropdown->expanded) {
            // Check hover over options
            dropdown->hover_index = -1;
            
            if (dropdown->base.style == CRYSTALLINE_STYLE_RECTANGULAR) {
                int visible_options = dropdown->option_count < 5 ? dropdown->option_count : 5;
                
                for (int i = 0; i < visible_options; i++) {
                    float y = dropdown->base.position.y + dropdown->item_height / 2.0f + 
                             (i + 1) * dropdown->item_height;
                    CrystallineRect opt_rect = crystalline_rect_create(
                        dropdown->base.position.x, y, dropdown->width, dropdown->item_height
                    );
                    
                    if (crystalline_rect_contains_point(opt_rect, mouse_pos)) {
                        dropdown->hover_index = i;
                        return true;
                    }
                }
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (dropdown->expanded && dropdown->hover_index >= 0) {
            // Select option
            dropdown->selected_index = dropdown->hover_index;
            dropdown->expanded = false;
            
            if (dropdown->on_select) {
                dropdown->on_select(dropdown->selected_index, dropdown->base.user_data);
            }
            return true;
        } else if (crystalline_rect_contains_point(dropdown->base.bounds, mouse_pos)) {
            // Toggle expanded state
            dropdown->expanded = !dropdown->expanded;
            
            if (dropdown->base.style == CRYSTALLINE_STYLE_CIRCULAR && dropdown->expanded) {
                crystalline_anim_start(&dropdown->expand_anim.base);
            }
            return true;
        } else if (dropdown->expanded) {
            // Click outside - collapse
            dropdown->expanded = false;
            return true;
        }
    }
    
    return false;
}

bool crystalline_dropdown_handle_keyboard(CrystallineDropdown* dropdown, SDL_Event* event) {
    if (!dropdown || !dropdown->base.enabled || !dropdown->expanded) return false;
    
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                if (dropdown->hover_index > 0) {
                    dropdown->hover_index--;
                    return true;
                }
                break;
            case SDLK_DOWN:
                if (dropdown->hover_index < dropdown->option_count - 1) {
                    dropdown->hover_index++;
                    return true;
                }
                break;
            case SDLK_RETURN:
                if (dropdown->hover_index >= 0) {
                    dropdown->selected_index = dropdown->hover_index;
                    dropdown->expanded = false;
                    if (dropdown->on_select) {
                        dropdown->on_select(dropdown->selected_index, dropdown->base.user_data);
                    }
                    return true;
                }
                break;
            case SDLK_ESCAPE:
                dropdown->expanded = false;
                return true;
        }
    }
    
    return false;
}

void crystalline_dropdown_set_options(CrystallineDropdown* dropdown, char** options, int count) {
    if (!dropdown) return;
    
    // Free existing options
    if (dropdown->options) {
        for (int i = 0; i < dropdown->option_count; i++) {
            if (dropdown->options[i]) free(dropdown->options[i]);
        }
        free(dropdown->options);
    }
    
    // Allocate new options
    dropdown->options = (char**)malloc(sizeof(char*) * count);
    dropdown->option_count = count;
    
    for (int i = 0; i < count; i++) {
        dropdown->options[i] = strdup(options[i]);
    }
}

void crystalline_dropdown_set_selected(CrystallineDropdown* dropdown, int index) {
    if (!dropdown) return;
    dropdown->selected_index = index;
}

void crystalline_dropdown_set_callback(CrystallineDropdown* dropdown, void (*callback)(int, void*), void* data) {
    if (!dropdown) return;
    dropdown->on_select = callback;
    dropdown->base.user_data = data;
}

int crystalline_dropdown_get_selected(CrystallineDropdown* dropdown) {
    return dropdown ? dropdown->selected_index : -1;
}

void crystalline_dropdown_set_style(CrystallineDropdown* dropdown, CrystallineElementStyle style) {
    if (!dropdown) return;
    dropdown->base.style = style;
}

/*
 * List checkbox functions
 */

void crystalline_list_enable_checkboxes(CrystallineList* list, float checkbox_size) {
    if (!list) return;
    
    list->show_checkboxes = true;
    list->checkbox_size = checkbox_size;
    
    // Allocate checkbox array if not already allocated
    if (!list->item_checked && list->item_count > 0) {
        list->item_checked = (bool*)calloc(list->item_count, sizeof(bool));
    }
}

void crystalline_list_set_item_checked(CrystallineList* list, int index, bool checked) {
    if (!list || !list->item_checked || index < 0 || index >= list->item_count) return;
    list->item_checked[index] = checked;
}

bool crystalline_list_get_item_checked(CrystallineList* list, int index) {
    if (!list || !list->item_checked || index < 0 || index >= list->item_count) return false;
    return list->item_checked[index];
}

void crystalline_list_set_check_callback(CrystallineList* list,
                                         void (*callback)(int index, bool checked, void* data), 
                                         void* data) {
    if (!list) return;
    list->on_check = callback;
    list->base.user_data = data;
}
