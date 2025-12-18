// app/ui/loading_screen.h - Loading Screen Interface
#ifndef LOADING_SCREEN_H
#define LOADING_SCREEN_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Loading stages
typedef enum {
    LOAD_STAGE_SDL = 0,
    LOAD_STAGE_WINDOW,
    LOAD_STAGE_RENDERER,
    LOAD_STAGE_FONTS,
    LOAD_STAGE_INPUT,
    LOAD_STAGE_TABS,
    LOAD_STAGE_SERVICES,
    LOAD_STAGE_COMPLETE
} LoadingStage;

// Loading screen state
typedef struct {
    LoadingStage current_stage;
    float progress;  // 0.0 to 1.0
    const char* stage_text;
    bool error;
    char error_message[256];
} LoadingScreenState;

/**
 * Initialize loading screen state
 */
void loading_screen_init(LoadingScreenState* state);

/**
 * Update loading screen progress
 * 
 * @param state Loading screen state
 * @param stage Current loading stage
 * @param progress Progress within stage (0.0 to 1.0)
 */
void loading_screen_update(LoadingScreenState* state, LoadingStage stage, float progress);

/**
 * Set loading screen error
 * 
 * @param state Loading screen state
 * @param error_msg Error message to display
 */
void loading_screen_set_error(LoadingScreenState* state, const char* error_msg);

/**
 * Render loading screen
 * 
 * @param renderer SDL renderer
 * @param state Loading screen state
 * @param alpha Alpha value for fade effect (0-255)
 */
void loading_screen_render(SDL_Renderer* renderer, LoadingScreenState* state, uint8_t alpha);

/**
 * Get stage text for display
 * 
 * @param stage Loading stage
 * @return Text description of stage
 */
const char* loading_screen_get_stage_text(LoadingStage stage);

#endif // LOADING_SCREEN_H