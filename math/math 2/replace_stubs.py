#!/usr/bin/env python3
import re

# Read the current file
with open('app/ui/crystalline/elements.c', 'r') as f:
    content = f.read()

# Find where stubs start (after Panel implementation)
stub_start = content.find('// Stub implementations for List, Toggle, Dropdown')

if stub_start == -1:
    print("Could not find stub section")
    exit(1)

# Keep everything before stubs
before_stubs = content[:stub_start]

# New complete implementations
complete_implementations = '''
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
            float list_height = visible_options * dropdown->item_height;
            
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
'''

# Write the new file
with open('app/ui/crystalline/elements.c', 'w') as f:
    f.write(before_stubs)
    f.write(complete_implementations)

print("Successfully replaced stub implementations with complete versions")
print(f"New file size: {len(before_stubs) + len(complete_implementations)} characters")
