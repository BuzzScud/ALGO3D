#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define MAX_INPUTS 32
#define MAX_INPUT_TEXT 256

// Forward declarations
typedef struct InputManager InputManager;
typedef struct ManagedInput ManagedInput;

// Input types
typedef enum {
    INPUT_TYPE_TEXT,
    INPUT_TYPE_NUMBER,
    INPUT_TYPE_URL
} InputType;

// Managed input structure
typedef struct ManagedInput {
    char id[64];                    // Unique identifier
    char text[MAX_INPUT_TEXT];      // Input text
    SDL_Rect bounds;                // Exact bounds for rendering and click detection
    bool active;                    // Is this input focused?
    bool visible;                   // Should this input be rendered?
    int tab_id;                     // Which tab owns this input
    InputType type;                 // Input type for validation
    int cursor_pos;                 // Cursor position
    
    // Callbacks
    void (*on_change)(const char* text, void* user_data);
    void (*on_submit)(const char* text, void* user_data);
    void* user_data;
} ManagedInput;

// Input manager structure
typedef struct InputManager {
    ManagedInput inputs[MAX_INPUTS];
    int input_count;
    ManagedInput* focused_input;
    int current_tab;
} InputManager;

// Global input manager instance
extern InputManager* g_input_manager;

// Lifecycle functions
InputManager* input_manager_create(void);
void input_manager_destroy(InputManager* manager);

// Input registration
ManagedInput* input_manager_register(
    InputManager* manager,
    const char* id,
    int tab_id,
    InputType type,
    SDL_Rect bounds
);

// Input management
ManagedInput* input_manager_get(InputManager* manager, const char* id);
void input_manager_set_text(InputManager* manager, const char* id, const char* text);
const char* input_manager_get_text(InputManager* manager, const char* id);
void input_manager_set_bounds(InputManager* manager, const char* id, SDL_Rect bounds);
void input_manager_set_visible(InputManager* manager, const char* id, bool visible);

// Focus management
void input_manager_focus(InputManager* manager, const char* id);
void input_manager_unfocus(InputManager* manager);
ManagedInput* input_manager_get_focused(InputManager* manager);

// Event handling
bool input_manager_handle_event(InputManager* manager, SDL_Event* event);

// Rendering
void input_manager_render(InputManager* manager, SDL_Renderer* renderer, TTF_Font* font, int tab_id);

// Tab management
void input_manager_set_tab(InputManager* manager, int tab_id);
void input_manager_hide_tab_inputs(InputManager* manager, int tab_id);
void input_manager_show_tab_inputs(InputManager* manager, int tab_id);

#endif // INPUT_MANAGER_H