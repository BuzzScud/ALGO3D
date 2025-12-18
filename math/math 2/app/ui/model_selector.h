#ifndef MODEL_SELECTOR_H
#define MODEL_SELECTOR_H

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ModelSelector ModelSelector;

/**
 * Create model selector
 */
ModelSelector* model_selector_create(int x, int y, int width, int height);

/**
 * Update model list from model manager
 */
void model_selector_update_list(ModelSelector* selector);

/**
 * Render model selector
 */
void model_selector_render(ModelSelector* selector, SDL_Renderer* renderer);

/**
 * Handle click (legacy - delegates to button_down)
 * Returns 1 if click was handled, 0 otherwise
 */
int model_selector_handle_click(ModelSelector* selector, int mouse_x, int mouse_y);

/**
 * Handle mouse button down (for opening dropdown)
 * Returns 1 if handled, 0 otherwise
 */
int model_selector_handle_button_down(ModelSelector* selector, int mouse_x, int mouse_y);

/**
 * Handle mouse button up (for selecting items)
 * Returns 1 if handled, 0 otherwise
 */
int model_selector_handle_button_up(ModelSelector* selector, int mouse_x, int mouse_y);

/**
 * Handle mouse motion (for hover effects)
 */
void model_selector_handle_motion(ModelSelector* selector, int mouse_x, int mouse_y);

/**
 * Get selected model
 */
const char* model_selector_get_selected(ModelSelector* selector);

/**
 * Set selected model
 */
void model_selector_set_selected(ModelSelector* selector, const char* model_name);

/**
 * Set change callback
 * Called when user selects a different model
 */
void model_selector_set_callback(ModelSelector* selector, 
                                void (*callback)(const char* model_name, void* user_data), 
                                void* user_data);

/**
 * Destroy model selector
 */
void model_selector_destroy(ModelSelector* selector);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_SELECTOR_H */