#include "input_manager.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Global input manager instance
InputManager* g_input_manager = NULL;

// Create input manager
InputManager* input_manager_create(void) {
    InputManager* manager = (InputManager*)calloc(1, sizeof(InputManager));
    if (!manager) return NULL;
    
    manager->input_count = 0;
    manager->focused_input = NULL;
    manager->current_tab = 0;
    
    printf("InputManager: Created\n");
    return manager;
}

// Destroy input manager
void input_manager_destroy(InputManager* manager) {
    if (!manager) return;
    
    printf("InputManager: Destroyed (%d inputs)\n", manager->input_count);
    free(manager);
}

// Register a new input
ManagedInput* input_manager_register(
    InputManager* manager,
    const char* id,
    int tab_id,
    InputType type,
    SDL_Rect bounds
) {
    if (!manager || !id) return NULL;
    if (manager->input_count >= MAX_INPUTS) {
        printf("InputManager: ERROR - Max inputs reached\n");
        return NULL;
    }
    
       // CRITICAL: Validate bounds to prevent segfault
       if (bounds.w <= 0 || bounds.h <= 0) {
           printf("InputManager: ERROR - Invalid input bounds for '%s': w=%d, h=%d\n", 
                  id, bounds.w, bounds.h);
           return NULL;
       }
       if (bounds.x < 0 || bounds.y < 0) {
           printf("InputManager: ERROR - Negative input position for '%s': x=%d, y=%d\n", 
                  id, bounds.x, bounds.y);
           return NULL;
       }

    // Check for duplicate ID
    for (int i = 0; i < manager->input_count; i++) {
        if (strcmp(manager->inputs[i].id, id) == 0) {
            printf("InputManager: WARNING - Input '%s' already registered\n", id);
            return &manager->inputs[i];
        }
    }
    
    ManagedInput* input = &manager->inputs[manager->input_count++];
    strncpy(input->id, id, sizeof(input->id) - 1);
    input->id[sizeof(input->id) - 1] = '\0';
    input->text[0] = '\0';
    input->bounds = bounds;
    input->active = false;
    input->visible = true;
    input->tab_id = tab_id;
    input->type = type;
    input->cursor_pos = 0;
    input->on_change = NULL;
    input->on_submit = NULL;
    input->user_data = NULL;
    
    // Log registration
    printf("InputManager: Registered '%s' tab=%d bounds=(%d,%d,%d,%d)\n",
           id, tab_id, bounds.x, bounds.y, bounds.w, bounds.h);
    
    // Silent registration
    return input;
}

// Get input by ID
ManagedInput* input_manager_get(InputManager* manager, const char* id) {
    if (!manager || !id) return NULL;
    
    for (int i = 0; i < manager->input_count; i++) {
        if (strcmp(manager->inputs[i].id, id) == 0) {
            return &manager->inputs[i];
        }
    }
    
    return NULL;
}

// Set input text
void input_manager_set_text(InputManager* manager, const char* id, const char* text) {
    ManagedInput* input = input_manager_get(manager, id);
    if (!input || !text) return;
    
    strncpy(input->text, text, MAX_INPUT_TEXT - 1);
    input->text[MAX_INPUT_TEXT - 1] = '\0';
    input->cursor_pos = strlen(input->text);
    
    if (input->on_change) {
        input->on_change(input->text, input->user_data);
    }
}

// Get input text
const char* input_manager_get_text(InputManager* manager, const char* id) {
    ManagedInput* input = input_manager_get(manager, id);
    return input ? input->text : "";
}

// Set input bounds
void input_manager_set_bounds(InputManager* manager, const char* id, SDL_Rect bounds) {
    ManagedInput* input = input_manager_get(manager, id);
    if (!input) return;
    
    input->bounds = bounds;
}

// Set input visibility
void input_manager_set_visible(InputManager* manager, const char* id, bool visible) {
    ManagedInput* input = input_manager_get(manager, id);
    if (!input) return;
    
    input->visible = visible;
}

// Focus an input
void input_manager_focus(InputManager* manager, const char* id) {
    if (!manager || !id) {
        printf("InputManager: ERROR - Cannot focus: manager=%p, id=%p\n", (void*)manager, (void*)id);
        return;
    }
    
    // Unfocus current
    if (manager->focused_input) {
        manager->focused_input->active = false;
    }
    
    // Focus new
    ManagedInput* input = input_manager_get(manager, id);
    if (input) {
        // CRITICAL: Validate bounds before focusing
        if (input->bounds.w <= 0 || input->bounds.h <= 0) {
            printf("InputManager: ERROR - Cannot focus input '%s' with invalid bounds: w=%d, h=%d\n",
                   id, input->bounds.w, input->bounds.h);
            return;
        }
        input->active = true;
        manager->focused_input = input;
        SDL_StartTextInput();
        printf("InputManager: Focused input '%s'\n", id);
    } else {
        printf("InputManager: ERROR - Input '%s' not found\n", id);
    }
}

// Unfocus current input
void input_manager_unfocus(InputManager* manager) {
    if (!manager || !manager->focused_input) return;
    
    // Silent unfocus
    manager->focused_input->active = false;
    manager->focused_input = NULL;
    SDL_StopTextInput();
}

// Get focused input
ManagedInput* input_manager_get_focused(InputManager* manager) {
    return manager ? manager->focused_input : NULL;
}

// Handle event
bool input_manager_handle_event(InputManager* manager, SDL_Event* event) {
    if (!manager || !event) return false;
    
    // Handle mouse clicks
        if (event->type == SDL_MOUSEBUTTONDOWN) {
            int mx = event->button.x;
            int my = event->button.y;
            
            // DEBUG: Uncomment for input debugging
            // printf("InputManager: Click at (%d, %d), current_tab=%d, input_count=%d\n",
            //        mx, my, manager->current_tab, manager->input_count);
            
            // Check if click is on any visible input in current tab
            bool clicked_input = false;
            for (int i = 0; i < manager->input_count; i++) {
                ManagedInput* input = &manager->inputs[i];
                
                // CRITICAL: Validate input pointer
                if (!input) {
                    printf("InputManager: ERROR - NULL input at index %d\n", i);
                    continue;
                }
                
                if (!input->visible || input->tab_id != manager->current_tab) {
                    // DEBUG: Uncomment for input debugging
                    // if (input->tab_id != manager->current_tab) {
                    //     printf("  Input '%s': tab_id=%d (skipped - wrong tab)\n", input->id, input->tab_id);
                    // }
                    continue;
                }
                
                // DEBUG: Uncomment for input debugging
                // printf("  Checking input '%s': bounds=(%d,%d,%d,%d)\n",
                //        input->id, input->bounds.x, input->bounds.y, input->bounds.w, input->bounds.h);
                
                // CRITICAL: Validate bounds before checking
                if (input->bounds.w <= 0 || input->bounds.h <= 0) {
                    printf("InputManager: WARNING - Invalid bounds for input '%s': w=%d, h=%d\n",
                           input->id, input->bounds.w, input->bounds.h);
                    continue;
                }
                
                // Check if click is inside bounds
                if (mx >= input->bounds.x && mx < input->bounds.x + input->bounds.w &&
                    my >= input->bounds.y && my < input->bounds.y + input->bounds.h) {
                    // DEBUG: Uncomment for input debugging
                    // printf("InputManager: Click on input '%s' at (%d, %d)\n", input->id, mx, my);
                    input_manager_focus(manager, input->id);
                    clicked_input = true;
                    return true;
                }
            }
            
            // Click outside all inputs - unfocus but let click pass through to buttons
            if (!clicked_input && manager->focused_input) {
                input_manager_unfocus(manager);
                // Don't return true - let the click reach button handlers
            }
        }

    
    // Handle keyboard input for focused input
    if (manager->focused_input && manager->focused_input->active) {
        ManagedInput* input = manager->focused_input;
        
        if (event->type == SDL_TEXTINPUT) {
            int len = strlen(input->text);
            if (len < MAX_INPUT_TEXT - 1) {
                char c = event->text.text[0];
                input->text[len] = c;
                input->text[len + 1] = '\0';
                input->cursor_pos = len + 1;
                
                if (input->on_change) {
                    input->on_change(input->text, input->user_data);
                }
            }
            return true;
        }
        
        if (event->type == SDL_KEYDOWN) {
            SDL_Keymod mod = SDL_GetModState();
            
            // Ctrl+V - Paste
            if ((mod & KMOD_CTRL) && event->key.keysym.sym == SDLK_v) {
                if (SDL_HasClipboardText()) {
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard) {
                        strncpy(input->text, clipboard, MAX_INPUT_TEXT - 1);
                        input->text[MAX_INPUT_TEXT - 1] = '\0';
                        input->cursor_pos = strlen(input->text);
                        SDL_free(clipboard);
                        
                        // Silent paste
                        
                        if (input->on_change) {
                            input->on_change(input->text, input->user_data);
                        }
                    }
                }
                return true;
            }
            
            // Backspace
            if (event->key.keysym.sym == SDLK_BACKSPACE) {
                int len = strlen(input->text);
                if (len > 0) {
                    input->text[len - 1] = '\0';
                    input->cursor_pos = len - 1;
                    
                    if (input->on_change) {
                        input->on_change(input->text, input->user_data);
                    }
                }
                return true;
            }
            
            // Enter - Submit
            if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER) {
                if (input->on_submit) {
                    input->on_submit(input->text, input->user_data);
                }
                input_manager_unfocus(manager);
                return true;
            }
            
            // Escape - Cancel
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                input_manager_unfocus(manager);
                return true;
            }
            
            // CRITICAL FIX: Block ALL other keydown events when input is focused
            // This prevents hotkeys from triggering while typing in input fields
            // Without this, keys like 'e', 'm', 'f', 's', 'r' would trigger hotkeys
            return true;
        }
    }
    
    return false;
}

// Render all inputs for current tab
void input_manager_render(InputManager* manager, SDL_Renderer* renderer, TTF_Font* font, int tab_id) {
    if (!manager || !renderer || !font) return;
    
    for (int i = 0; i < manager->input_count; i++) {
        ManagedInput* input = &manager->inputs[i];
        
        if (!input->visible || input->tab_id != tab_id) continue;
        
        // Draw background
        SDL_Color bg_color = input->active ? 
            (SDL_Color){255, 255, 255, 255} : (SDL_Color){200, 200, 200, 255};
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
        SDL_RenderFillRect(renderer, &input->bounds);
        
        // Draw border
        SDL_Color border_color = input->active ?
            (SDL_Color){0, 120, 215, 255} : (SDL_Color){100, 100, 100, 255};
        SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, 255);
        SDL_RenderDrawRect(renderer, &input->bounds);
        
        // Draw text
        if (strlen(input->text) > 0) {
            SDL_Color text_color = {0, 0, 0, 255};
            SDL_Surface* text_surface = TTF_RenderText_Blended(font, input->text, text_color);
            if (text_surface) {
                SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                if (text_texture) {
                    SDL_Rect text_rect = {
                        input->bounds.x + 5,
                        input->bounds.y + (input->bounds.h - text_surface->h) / 2,
                        text_surface->w,
                        text_surface->h
                    };
                    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
                    SDL_DestroyTexture(text_texture);
                }
                SDL_FreeSurface(text_surface);
            }
        }
        
        // Draw cursor if active
        if (input->active) {
            int cursor_x = input->bounds.x + 5;
            if (strlen(input->text) > 0) {
                int text_w = 0;
                TTF_SizeText(font, input->text, &text_w, NULL);
                cursor_x += text_w;
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawLine(renderer,
                cursor_x, input->bounds.y + 5,
                cursor_x, input->bounds.y + input->bounds.h - 5);
        }
    }
}

// Set current tab
void input_manager_set_tab(InputManager* manager, int tab_id) {
    if (!manager) return;
    
    // Unfocus when switching tabs
    if (manager->focused_input && manager->focused_input->tab_id != tab_id) {
        input_manager_unfocus(manager);
    }
    
    manager->current_tab = tab_id;
}

// Hide tab inputs
void input_manager_hide_tab_inputs(InputManager* manager, int tab_id) {
    if (!manager) return;
    
    for (int i = 0; i < manager->input_count; i++) {
        if (manager->inputs[i].tab_id == tab_id) {
            manager->inputs[i].visible = false;
        }
    }
}

// Show tab inputs
void input_manager_show_tab_inputs(InputManager* manager, int tab_id) {
    if (!manager) return;
    
    for (int i = 0; i < manager->input_count; i++) {
        if (manager->inputs[i].tab_id == tab_id) {
            manager->inputs[i].visible = true;
        }
    }
}