/**
 * Simple Dialog System
 * 
 * Basic modal dialogs for user prompts
 */

#include "simple_dialog.h"
#include "app_common.h"
#include <string.h>

void dialog_init(SimpleDialog* dialog) {
    if (!dialog) return;
    
    memset(dialog, 0, sizeof(SimpleDialog));
    dialog->active = false;
    dialog->result = DIALOG_RESULT_NONE;
}

void dialog_show(SimpleDialog* dialog, const char* title, const char* message,
                const char* btn1, const char* btn2, const char* btn3) {
    if (!dialog) return;
    
    dialog->active = true;
    dialog->result = DIALOG_RESULT_NONE;
    
    strncpy(dialog->title, title, sizeof(dialog->title) - 1);
    strncpy(dialog->message, message, sizeof(dialog->message) - 1);
    
    if (btn1) strncpy(dialog->button1_text, btn1, sizeof(dialog->button1_text) - 1);
    if (btn2) strncpy(dialog->button2_text, btn2, sizeof(dialog->button2_text) - 1);
    if (btn3) strncpy(dialog->button3_text, btn3, sizeof(dialog->button3_text) - 1);
    
    // Center dialog on screen
    int dialog_width = 500;
    int dialog_height = 200;
    dialog->dialog_rect.x = (WINDOW_WIDTH - dialog_width) / 2;
    dialog->dialog_rect.y = (WINDOW_HEIGHT - dialog_height) / 2;
    dialog->dialog_rect.w = dialog_width;
    dialog->dialog_rect.h = dialog_height;
    
    // Position buttons
    int button_width = 140;
    int button_height = 35;
    int button_y = dialog->dialog_rect.y + dialog_height - button_height - 20;
    int button_spacing = 10;
    
    if (btn1 && btn2 && btn3) {
        // Three buttons
        int total_width = button_width * 3 + button_spacing * 2;
        int start_x = dialog->dialog_rect.x + (dialog_width - total_width) / 2;
        
        dialog->button1_rect = (SDL_Rect){start_x, button_y, button_width, button_height};
        dialog->button2_rect = (SDL_Rect){start_x + button_width + button_spacing, button_y, button_width, button_height};
        dialog->button3_rect = (SDL_Rect){start_x + (button_width + button_spacing) * 2, button_y, button_width, button_height};
    } else if (btn1 && btn2) {
        // Two buttons
        int total_width = button_width * 2 + button_spacing;
        int start_x = dialog->dialog_rect.x + (dialog_width - total_width) / 2;
        
        dialog->button1_rect = (SDL_Rect){start_x, button_y, button_width, button_height};
        dialog->button2_rect = (SDL_Rect){start_x + button_width + button_spacing, button_y, button_width, button_height};
    } else if (btn1) {
        // One button
        int start_x = dialog->dialog_rect.x + (dialog_width - button_width) / 2;
        dialog->button1_rect = (SDL_Rect){start_x, button_y, button_width, button_height};
    }
}

void dialog_draw(SimpleDialog* dialog, SDL_Renderer* renderer) {
    if (!dialog || !dialog->active || !renderer) return;
    
    // Draw overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Draw dialog background
    SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
    SDL_RenderFillRect(renderer, &dialog->dialog_rect);
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &dialog->dialog_rect);
    
    // Draw title
    SDL_Color title_color = {220, 220, 255, 255};
    draw_text(renderer, dialog->title, dialog->dialog_rect.x + 20, dialog->dialog_rect.y + 15, title_color);
    
    // Draw message (word-wrapped)
    SDL_Color text_color = {200, 200, 200, 255};
    int text_y = dialog->dialog_rect.y + 50;
    int chars_per_line = 60;
    for (size_t i = 0; i < strlen(dialog->message); i += chars_per_line) {
        char line[128];
        int len = (int)strlen(dialog->message + i);
        if (len > chars_per_line) len = chars_per_line;
        memcpy(line, dialog->message + i, (size_t)len);
        line[len] = '\0';
        draw_text(renderer, line, dialog->dialog_rect.x + 20, text_y, text_color);
        text_y += 18;
    }
    
    // Draw buttons
    SDL_Color button_color = {70, 100, 180, 255};
    SDL_Color button_text_color = {255, 255, 255, 255};
    
    if (dialog->button1_text[0]) {
        SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
        SDL_RenderFillRect(renderer, &dialog->button1_rect);
        SDL_SetRenderDrawColor(renderer, 100, 130, 200, 255);
        SDL_RenderDrawRect(renderer, &dialog->button1_rect);
        int text_x = dialog->button1_rect.x + (dialog->button1_rect.w - strlen(dialog->button1_text) * 7) / 2;
        draw_text(renderer, dialog->button1_text, text_x, dialog->button1_rect.y + 10, button_text_color);
    }
    
    if (dialog->button2_text[0]) {
        SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
        SDL_RenderFillRect(renderer, &dialog->button2_rect);
        SDL_SetRenderDrawColor(renderer, 100, 130, 200, 255);
        SDL_RenderDrawRect(renderer, &dialog->button2_rect);
        int text_x = dialog->button2_rect.x + (dialog->button2_rect.w - strlen(dialog->button2_text) * 7) / 2;
        draw_text(renderer, dialog->button2_text, text_x, dialog->button2_rect.y + 10, button_text_color);
    }
    
    if (dialog->button3_text[0]) {
        SDL_Color cancel_color = {100, 60, 60, 255};
        SDL_SetRenderDrawColor(renderer, cancel_color.r, cancel_color.g, cancel_color.b, 255);
        SDL_RenderFillRect(renderer, &dialog->button3_rect);
        SDL_SetRenderDrawColor(renderer, 150, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &dialog->button3_rect);
        int text_x = dialog->button3_rect.x + (dialog->button3_rect.w - strlen(dialog->button3_text) * 7) / 2;
        draw_text(renderer, dialog->button3_text, text_x, dialog->button3_rect.y + 10, button_text_color);
    }
}

DialogResult dialog_handle_click(SimpleDialog* dialog, int x, int y) {
    if (!dialog || !dialog->active) return DIALOG_RESULT_NONE;
    
    // Check button 1
    if (dialog->button1_text[0] &&
        x >= dialog->button1_rect.x && x <= dialog->button1_rect.x + dialog->button1_rect.w &&
        y >= dialog->button1_rect.y && y <= dialog->button1_rect.y + dialog->button1_rect.h) {
        dialog->result = DIALOG_RESULT_CREATE_NEW;
        dialog->active = false;
        return dialog->result;
    }
    
    // Check button 2
    if (dialog->button2_text[0] &&
        x >= dialog->button2_rect.x && x <= dialog->button2_rect.x + dialog->button2_rect.w &&
        y >= dialog->button2_rect.y && y <= dialog->button2_rect.y + dialog->button2_rect.h) {
        dialog->result = DIALOG_RESULT_OVERWRITE;
        dialog->active = false;
        return dialog->result;
    }
    
    // Check button 3
    if (dialog->button3_text[0] &&
        x >= dialog->button3_rect.x && x <= dialog->button3_rect.x + dialog->button3_rect.w &&
        y >= dialog->button3_rect.y && y <= dialog->button3_rect.y + dialog->button3_rect.h) {
        dialog->result = DIALOG_RESULT_CANCEL_TRAINING;
        dialog->active = false;
        return dialog->result;
    }
    
    return DIALOG_RESULT_NONE;
}

bool dialog_is_active(SimpleDialog* dialog) {
    return dialog && dialog->active;
}

void dialog_close(SimpleDialog* dialog) {
    if (dialog) {
        dialog->active = false;
    }
}