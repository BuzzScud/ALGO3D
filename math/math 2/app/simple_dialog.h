#ifndef SIMPLE_DIALOG_H
#define SIMPLE_DIALOG_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef enum {
    DIALOG_RESULT_NONE = 0,
    DIALOG_RESULT_YES = 1,
    DIALOG_RESULT_NO = 2,
    DIALOG_RESULT_CANCEL = 3,
    DIALOG_RESULT_CREATE_NEW = 4,
    DIALOG_RESULT_OVERWRITE = 5,
    DIALOG_RESULT_CANCEL_TRAINING = 6
} DialogResult;

typedef struct {
    bool active;
    char title[128];
    char message[512];
    DialogResult result;
    SDL_Rect dialog_rect;
    SDL_Rect button1_rect;
    SDL_Rect button2_rect;
    SDL_Rect button3_rect;
    char button1_text[32];
    char button2_text[32];
    char button3_text[32];
} SimpleDialog;

// Initialize dialog
void dialog_init(SimpleDialog* dialog);

// Show dialog with message and buttons
void dialog_show(SimpleDialog* dialog, const char* title, const char* message,
                const char* btn1, const char* btn2, const char* btn3);

// Draw dialog
void dialog_draw(SimpleDialog* dialog, SDL_Renderer* renderer);

// Handle click
DialogResult dialog_handle_click(SimpleDialog* dialog, int x, int y);

// Check if dialog is active
bool dialog_is_active(SimpleDialog* dialog);

// Close dialog
void dialog_close(SimpleDialog* dialog);

#endif // SIMPLE_DIALOG_H