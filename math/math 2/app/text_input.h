#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#ifndef TEXT_INPUT_MAX_LENGTH
#define TEXT_INPUT_MAX_LENGTH 256
#endif

typedef struct {
    char text[TEXT_INPUT_MAX_LENGTH];
    int cursor_pos;
    bool active;
    SDL_Rect bounds;
    char label[64];
    bool numeric_only;
    double min_value;
    double max_value;
} TextInput;

// Initialize a text input field
void text_input_init(TextInput* input, const char* label, int x, int y, int w, int h);

// Set numeric constraints
void text_input_set_numeric(TextInput* input, double min, double max);

// Handle SDL events
bool text_input_handle_event(TextInput* input, SDL_Event* event);

// Render the text input field
void text_input_render(TextInput* input, SDL_Renderer* renderer, TTF_Font* font);

// Get the text value
const char* text_input_get_text(TextInput* input);

// Get numeric value (if numeric_only is true)
double text_input_get_number(TextInput* input);

// Set text value
void text_input_set_text(TextInput* input, const char* text);

// Clear the input
void text_input_clear(TextInput* input);

// Check if input is active
bool text_input_is_active(TextInput* input);

// Activate/deactivate input
void text_input_activate(TextInput* input);
void text_input_deactivate(TextInput* input);

#endif // TEXT_INPUT_H