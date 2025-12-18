// app/ui/loading_screen.c - Loading Screen Implementation
#include "loading_screen.h"
#include <stdio.h>
#include <string.h>

// Stage text descriptions
static const char* STAGE_TEXTS[] = {
    "Initializing SDL...",
    "Creating window...",
    "Initializing renderer...",
    "Loading fonts...",
    "Initializing input system...",
    "Initializing tabs...",
    "Starting background services...",
    "Complete"
};

// Stage progress ranges (0.0 to 1.0)
static const float STAGE_RANGES[][2] = {
    {0.0f, 0.20f},   // SDL
    {0.20f, 0.40f},  // Window
    {0.40f, 0.60f},  // Renderer
    {0.60f, 0.70f},  // Fonts
    {0.70f, 0.80f},  // Input
    {0.80f, 0.90f},  // Tabs
    {0.90f, 1.00f},  // Services
    {1.00f, 1.00f}   // Complete
};

void loading_screen_init(LoadingScreenState* state) {
    if (!state) return;
    
    state->current_stage = LOAD_STAGE_SDL;
    state->progress = 0.0f;
    state->stage_text = STAGE_TEXTS[0];
    state->error = false;
    state->error_message[0] = '\0';
}

void loading_screen_update(LoadingScreenState* state, LoadingStage stage, float progress) {
    if (!state) return;
    
    state->current_stage = stage;
    state->stage_text = loading_screen_get_stage_text(stage);
    
    // Calculate overall progress
    float stage_start = STAGE_RANGES[stage][0];
    float stage_end = STAGE_RANGES[stage][1];
    float stage_range = stage_end - stage_start;
    
    state->progress = stage_start + (progress * stage_range);
    
    // Clamp to [0.0, 1.0]
    if (state->progress < 0.0f) state->progress = 0.0f;
    if (state->progress > 1.0f) state->progress = 1.0f;
}

void loading_screen_set_error(LoadingScreenState* state, const char* error_msg) {
    if (!state) return;
    
    state->error = true;
    if (error_msg) {
        strncpy(state->error_message, error_msg, sizeof(state->error_message) - 1);
        state->error_message[sizeof(state->error_message) - 1] = '\0';
    } else {
        strcpy(state->error_message, "Unknown error");
    }
}

const char* loading_screen_get_stage_text(LoadingStage stage) {
    if (stage < 0 || stage > LOAD_STAGE_COMPLETE) {
        return "Unknown stage";
    }
    return STAGE_TEXTS[stage];
}

void loading_screen_render(SDL_Renderer* renderer, LoadingScreenState* state, uint8_t alpha) {
    if (!renderer || !state) return;
    
    // Get window size
    int window_width, window_height;
    SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
    
    // Clear background (dark blue-gray)
    SDL_SetRenderDrawColor(renderer, 10, 15, 20, 255);
    SDL_RenderClear(renderer);
    
    // Calculate center position
    int center_x = window_width / 2;
    int center_y = window_height / 2;
    
    // Draw title "CRYSTALLINE CLLM" (using simple rectangles as placeholder)
    // In production, this would use TTF_RenderText
    SDL_SetRenderDrawColor(renderer, 180, 200, 220, alpha);
    SDL_Rect title_rect = {center_x - 150, center_y - 100, 300, 40};
    SDL_RenderDrawRect(renderer, &title_rect);
    
    // Draw stage text
    SDL_SetRenderDrawColor(renderer, 140, 160, 180, alpha);
    SDL_Rect stage_rect = {center_x - 200, center_y - 40, 400, 20};
    SDL_RenderDrawRect(renderer, &stage_rect);
    
    // Draw progress bar background
    int bar_width = 400;
    int bar_height = 30;
    int bar_x = center_x - bar_width / 2;
    int bar_y = center_y;
    
    SDL_Rect bar_bg = {bar_x, bar_y, bar_width, bar_height};
    SDL_SetRenderDrawColor(renderer, 40, 45, 50, alpha);
    SDL_RenderFillRect(renderer, &bar_bg);
    
    // Draw progress bar border
    SDL_SetRenderDrawColor(renderer, 100, 120, 140, alpha);
    SDL_RenderDrawRect(renderer, &bar_bg);
    
    // Draw progress bar fill
    int fill_width = (int)((bar_width - 4) * state->progress);
    if (fill_width > 0) {
        SDL_Rect bar_fill = {bar_x + 2, bar_y + 2, fill_width, bar_height - 4};
        
        // Gradient effect (crystalline blue)
        SDL_SetRenderDrawColor(renderer, 60, 140, 220, alpha);
        SDL_RenderFillRect(renderer, &bar_fill);
    }
    
    // Draw progress percentage
    char progress_text[32];
    snprintf(progress_text, sizeof(progress_text), "%.0f%%", state->progress * 100.0f);
    // In production, render text here
    
    // Draw decorative crystalline brackets
    SDL_SetRenderDrawColor(renderer, 60, 140, 220, alpha / 2);
    
    // Top-left bracket
    SDL_RenderDrawLine(renderer, bar_x - 20, bar_y - 20, bar_x - 5, bar_y - 20);
    SDL_RenderDrawLine(renderer, bar_x - 20, bar_y - 20, bar_x - 20, bar_y - 5);
    
    // Top-right bracket
    SDL_RenderDrawLine(renderer, bar_x + bar_width + 5, bar_y - 20, bar_x + bar_width + 20, bar_y - 20);
    SDL_RenderDrawLine(renderer, bar_x + bar_width + 20, bar_y - 20, bar_x + bar_width + 20, bar_y - 5);
    
    // Bottom-left bracket
    SDL_RenderDrawLine(renderer, bar_x - 20, bar_y + bar_height + 5, bar_x - 20, bar_y + bar_height + 20);
    SDL_RenderDrawLine(renderer, bar_x - 20, bar_y + bar_height + 20, bar_x - 5, bar_y + bar_height + 20);
    
    // Bottom-right bracket
    SDL_RenderDrawLine(renderer, bar_x + bar_width + 20, bar_y + bar_height + 5, bar_x + bar_width + 20, bar_y + bar_height + 20);
    SDL_RenderDrawLine(renderer, bar_x + bar_width + 5, bar_y + bar_height + 20, bar_x + bar_width + 20, bar_y + bar_height + 20);
    
    // Draw error message if present
    if (state->error) {
        SDL_SetRenderDrawColor(renderer, 220, 60, 60, alpha);
        SDL_Rect error_rect = {center_x - 200, center_y + 50, 400, 30};
        SDL_RenderDrawRect(renderer, &error_rect);
        // In production, render error text here
    }
    
    SDL_RenderPresent(renderer);
}