#include "text_input.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void text_input_init(TextInput* input, const char* label, int x, int y, int w, int h) {
    if (!input) return;
    
    printf("DEBUG: text_input_init called for '%s', clearing text\n", label ? label : "NULL");
    memset(input->text, 0, sizeof(input->text));
    input->cursor_pos = 0;
    input->active = false;
    input->bounds.x = x;
    input->bounds.y = y;
    input->bounds.w = w;
    input->bounds.h = h;
    input->numeric_only = false;
    input->min_value = 0.0;
    input->max_value = 0.0;
    
    if (label) {
        strncpy(input->label, label, sizeof(input->label) - 1);
        input->label[sizeof(input->label) - 1] = '\0';
    } else {
        input->label[0] = '\0';
    }
}

void text_input_set_numeric(TextInput* input, double min, double max) {
    if (!input) return;
    input->numeric_only = true;
    input->min_value = min;
    input->max_value = max;
}

bool text_input_handle_event(TextInput* input, SDL_Event* event) {
    if (!input) return false;
    
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int mx = event->button.x;
        int my = event->button.y;
        
        // Check if click is inside bounds
        if (mx >= input->bounds.x && mx < input->bounds.x + input->bounds.w &&
            my >= input->bounds.y && my < input->bounds.y + input->bounds.h) {
            text_input_activate(input);
            return true;
        } else if (input->active) {
            printf("DEBUG: Mouse click outside input bounds, deactivating. input=%p, text='%s'\n", 
                   (void*)input, input->text);
            text_input_deactivate(input);
            return true;
        }
    }
    
    if (!input->active) return false;
    
    if (event->type == SDL_TEXTINPUT) {
        int len = strlen(input->text);
        char c = event->text.text[0];
        
        // Validate input
        if (input->numeric_only) {
            if (!isdigit(c) && c != '.' && c != '-') {
                return true; // Consume but ignore
            }
        }
        
        if (len < TEXT_INPUT_MAX_LENGTH - 1) {
            input->text[len] = c;
            input->text[len + 1] = '\0';
            input->cursor_pos = len + 1;
        }
        return true;
    }
    
    if (event->type == SDL_KEYDOWN) {
        SDL_Keymod mod = SDL_GetModState();
        
        // Handle Ctrl+V (Paste)
        if ((mod & KMOD_CTRL) && event->key.keysym.sym == SDLK_v) {
            if (SDL_HasClipboardText()) {
                char* clipboard = SDL_GetClipboardText();
                if (clipboard) {
                    // Clear current text and paste clipboard content
                    int remaining = TEXT_INPUT_MAX_LENGTH - 1;
                    strncpy(input->text, clipboard, remaining);
                    input->text[remaining] = '\0';
                    input->cursor_pos = strlen(input->text);
                    printf("Pasted from clipboard: %s\n", input->text);
                    printf("DEBUG: After paste, input=%p, input->text = '%s', length = %zu, active = %d\n", 
                           (void*)input, input->text, strlen(input->text), input->active);
                    SDL_free(clipboard);
                }
            }
            return true;
        }
        
        // Handle Ctrl+C (Copy)
        if ((mod & KMOD_CTRL) && event->key.keysym.sym == SDLK_c) {
            if (strlen(input->text) > 0) {
                SDL_SetClipboardText(input->text);
                printf("Copied to clipboard: %s\n", input->text);
            }
            return true;
        }
        
        // Handle Ctrl+X (Cut)
        if ((mod & KMOD_CTRL) && event->key.keysym.sym == SDLK_x) {
            if (strlen(input->text) > 0) {
                SDL_SetClipboardText(input->text);
                printf("Cut to clipboard: %s\n", input->text);
                memset(input->text, 0, TEXT_INPUT_MAX_LENGTH);
                input->cursor_pos = 0;
            }
            return true;
        }
        
        // Handle Ctrl+A (Select All - for now just move cursor to end)
        if ((mod & KMOD_CTRL) && event->key.keysym.sym == SDLK_a) {
            input->cursor_pos = strlen(input->text);
            return true;
        }
        
        switch (event->key.keysym.sym) {
            case SDLK_BACKSPACE:
                if (strlen(input->text) > 0) {
                    input->text[strlen(input->text) - 1] = '\0';
                    input->cursor_pos = strlen(input->text);
                }
                return true;
                
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                text_input_deactivate(input);
                return true;
                
            case SDLK_ESCAPE:
                text_input_deactivate(input);
                return true;
                
            case SDLK_LEFT:
                if (input->cursor_pos > 0) {
                    input->cursor_pos--;
                }
                return true;
                
            case SDLK_RIGHT:
                if (input->cursor_pos < (int)strlen(input->text)) {
                    input->cursor_pos++;
                }
                return true;
                
            case SDLK_HOME:
                input->cursor_pos = 0;
                return true;
                
            case SDLK_END:
                input->cursor_pos = strlen(input->text);
                return true;
        }
    }
    
    return false;
}

void text_input_render(TextInput* input, SDL_Renderer* renderer, TTF_Font* font) {
    if (!input || !renderer) return;
    
    // Draw background
    SDL_Color bg_color = input->active ? 
        (SDL_Color){255, 255, 255, 255} : 
        (SDL_Color){200, 200, 200, 255};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, &input->bounds);
    
    // Draw border
    SDL_Color border_color = input->active ? 
        (SDL_Color){0, 120, 215, 255} : 
        (SDL_Color){100, 100, 100, 255};
    SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
    SDL_RenderDrawRect(renderer, &input->bounds);
    
    // Draw label if exists
    if (font && strlen(input->label) > 0) {
        SDL_Color text_color = {0, 0, 0, 255};
        SDL_Surface* label_surface = TTF_RenderText_Blended(font, input->label, text_color);
        if (label_surface) {
            SDL_Texture* label_texture = SDL_CreateTextureFromSurface(renderer, label_surface);
            if (label_texture) {
                SDL_Rect label_rect = {
                    input->bounds.x,
                    input->bounds.y - 20,
                    label_surface->w,
                    label_surface->h
                };
                SDL_RenderCopy(renderer, label_texture, NULL, &label_rect);
                SDL_DestroyTexture(label_texture);
            }
            SDL_FreeSurface(label_surface);
        }
    }
    
    // Draw text
    if (font && strlen(input->text) > 0) {
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
        if (font && strlen(input->text) > 0) {
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

const char* text_input_get_text(TextInput* input) {
    return input ? input->text : "";
}

double text_input_get_number(TextInput* input) {
    if (!input || !input->numeric_only) return 0.0;
    
    double value = atof(input->text);
    
    // Clamp to min/max if set
    if (input->max_value > input->min_value) {
        if (value < input->min_value) value = input->min_value;
        if (value > input->max_value) value = input->max_value;
    }
    
    return value;
}

void text_input_set_text(TextInput* input, const char* text) {
    if (!input || !text) return;
    
    printf("DEBUG: text_input_set_text called, old='%s', new='%s'\n", 
           input->text, text);
    strncpy(input->text, text, TEXT_INPUT_MAX_LENGTH - 1);
    input->text[TEXT_INPUT_MAX_LENGTH - 1] = '\0';
    input->cursor_pos = strlen(input->text);
    printf("DEBUG: After set_text, text='%s'\n", input->text);
}

void text_input_clear(TextInput* input) {
    if (!input) return;
    
    printf("DEBUG: text_input_clear called! text was='%s'\n", input->text);
    memset(input->text, 0, TEXT_INPUT_MAX_LENGTH);
    input->cursor_pos = 0;
}

bool text_input_is_active(TextInput* input) {
    return input ? input->active : false;
}

void text_input_activate(TextInput* input) {
    if (!input) return;
    
    printf("DEBUG: text_input_activate called, input=%p, text='%s'\n", 
           (void*)input, input->text);
    printf("DEBUG: Activation stack trace - check where this was called from\n");
    input->active = true;
    SDL_StartTextInput();
}

void text_input_deactivate(TextInput* input) {
    if (!input) return;
    
    printf("DEBUG: text_input_deactivate called, input=%p, text='%s', length=%zu\n", 
           (void*)input, input->text, strlen(input->text));
    input->active = false;
    SDL_StopTextInput();
    printf("DEBUG: After deactivate, input=%p, text='%s', length=%zu\n", 
           (void*)input, input->text, strlen(input->text));
}