/**
 * Reusable UI Component System for Crystalline CLLM
 * 
 * Provides consistent, reusable UI components with proper event handling
 */

#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct UIButton UIButton;
typedef struct UITextInput UITextInput;
typedef struct UISlider UISlider;
typedef struct UIProgressBar UIProgressBar;
typedef struct UIDropdown UIDropdown;
typedef struct UIDialog UIDialog;
typedef struct UILabel UILabel;
typedef struct UIPanel UIPanel;

// Callback types
typedef void (*ButtonCallback)(void* user_data);
typedef void (*TextInputCallback)(const char* text, void* user_data);
typedef void (*SliderCallback)(float value, void* user_data);
typedef void (*DropdownCallback)(int index, const char* item, void* user_data);

// Component states
typedef enum {
    UI_STATE_NORMAL,
    UI_STATE_HOVER,
    UI_STATE_ACTIVE,
    UI_STATE_DISABLED
} UIState;

// ============================================================================
// BUTTON COMPONENT
// ============================================================================

struct UIButton {
    SDL_Rect bounds;
    char label[128];
    UIState state;
    bool visible;
    
    // Styling
    SDL_Color bg_normal;
    SDL_Color bg_hover;
    SDL_Color bg_active;
    SDL_Color bg_disabled;
    SDL_Color text_color;
    SDL_Color border_color;
    
    // Callback
    ButtonCallback on_click;
    void* user_data;
};

UIButton* ui_button_create(int x, int y, int w, int h, const char* label);
void ui_button_destroy(UIButton* button);
void ui_button_render(UIButton* button, SDL_Renderer* renderer);
bool ui_button_handle_event(UIButton* button, SDL_Event* event);
void ui_button_set_callback(UIButton* button, ButtonCallback callback, void* user_data);
void ui_button_set_enabled(UIButton* button, bool enabled);
void ui_button_set_position(UIButton* button, int x, int y);

// ============================================================================
// TEXT INPUT COMPONENT
// ============================================================================

struct UITextInput {
    SDL_Rect bounds;
    char text[256];
    char placeholder[128];
    int cursor_pos;
    bool focused;
    bool visible;
    UIState state;
    
    // Styling
    SDL_Color bg_color;
    SDL_Color text_color;
    SDL_Color placeholder_color;
    SDL_Color border_color;
    SDL_Color cursor_color;
    
    // Validation
    bool (*validator)(const char* text);
    
    // Callback
    TextInputCallback on_change;
    TextInputCallback on_submit;
    void* user_data;
};

UITextInput* ui_text_input_create(int x, int y, int w, int h, const char* placeholder);
void ui_text_input_destroy(UITextInput* input);
void ui_text_input_render(UITextInput* input, SDL_Renderer* renderer);
bool ui_text_input_handle_event(UITextInput* input, SDL_Event* event);
void ui_text_input_set_text(UITextInput* input, const char* text);
const char* ui_text_input_get_text(const UITextInput* input);
void ui_text_input_set_callback(UITextInput* input, TextInputCallback on_change, 
                                TextInputCallback on_submit, void* user_data);
void ui_text_input_set_validator(UITextInput* input, bool (*validator)(const char*));
void ui_text_input_set_focus(UITextInput* input, bool focused);

// ============================================================================
// SLIDER COMPONENT
// ============================================================================

struct UISlider {
    SDL_Rect bounds;
    float value;        // Current value (0.0 to 1.0)
    float min_value;    // Minimum value
    float max_value;    // Maximum value
    bool dragging;
    bool visible;
    UIState state;
    
    // Display
    char label[64];
    bool show_value;
    
    // Styling
    SDL_Color track_color;
    SDL_Color fill_color;
    SDL_Color handle_color;
    SDL_Color text_color;
    
    // Callback
    SliderCallback on_change;
    void* user_data;
};

UISlider* ui_slider_create(int x, int y, int w, int h, float min, float max, const char* label);
void ui_slider_destroy(UISlider* slider);
void ui_slider_render(UISlider* slider, SDL_Renderer* renderer);
bool ui_slider_handle_event(UISlider* slider, SDL_Event* event);
void ui_slider_set_value(UISlider* slider, float value);
float ui_slider_get_value(const UISlider* slider);
int ui_slider_get_value_int(const UISlider* slider);
void ui_slider_set_callback(UISlider* slider, SliderCallback callback, void* user_data);

// ============================================================================
// PROGRESS BAR COMPONENT
// ============================================================================

struct UIProgressBar {
    SDL_Rect bounds;
    float progress;     // 0.0 to 1.0
    bool visible;
    
    // Display
    char label[128];
    bool show_percentage;
    
    // Styling
    SDL_Color bg_color;
    SDL_Color fill_color;
    SDL_Color text_color;
    SDL_Color border_color;
};

UIProgressBar* ui_progress_bar_create(int x, int y, int w, int h);
void ui_progress_bar_destroy(UIProgressBar* bar);
void ui_progress_bar_render(UIProgressBar* bar, SDL_Renderer* renderer);
void ui_progress_bar_set_progress(UIProgressBar* bar, float progress);
void ui_progress_bar_set_label(UIProgressBar* bar, const char* label);

// ============================================================================
// DROPDOWN COMPONENT
// ============================================================================

struct UIDropdown {
    SDL_Rect bounds;
    char** items;
    int num_items;
    int selected_index;
    bool open;
    bool visible;
    UIState state;
    int scroll_offset;
    int hover_index;
    
    // Styling
    SDL_Color bg_color;
    SDL_Color text_color;
    SDL_Color hover_color;
    SDL_Color border_color;
    
    // Callback
    DropdownCallback on_select;
    void* user_data;
};

UIDropdown* ui_dropdown_create(int x, int y, int w, int h);
void ui_dropdown_destroy(UIDropdown* dropdown);
void ui_dropdown_render(UIDropdown* dropdown, SDL_Renderer* renderer);
bool ui_dropdown_handle_event(UIDropdown* dropdown, SDL_Event* event);
void ui_dropdown_add_item(UIDropdown* dropdown, const char* item);
void ui_dropdown_clear_items(UIDropdown* dropdown);
void ui_dropdown_set_selected(UIDropdown* dropdown, int index);
int ui_dropdown_get_selected(const UIDropdown* dropdown);
const char* ui_dropdown_get_selected_text(const UIDropdown* dropdown);
void ui_dropdown_set_callback(UIDropdown* dropdown, DropdownCallback callback, void* user_data);

// ============================================================================
// LABEL COMPONENT
// ============================================================================

struct UILabel {
    SDL_Rect bounds;
    char text[256];
    bool visible;
    
    // Styling
    SDL_Color text_color;
    SDL_Color bg_color;
    bool has_background;
    
    // Alignment
    enum {
        LABEL_ALIGN_LEFT,
        LABEL_ALIGN_CENTER,
        LABEL_ALIGN_RIGHT
    } align;
};

UILabel* ui_label_create(int x, int y, int w, int h, const char* text);
void ui_label_destroy(UILabel* label);
void ui_label_render(UILabel* label, SDL_Renderer* renderer);
void ui_label_set_text(UILabel* label, const char* text);
void ui_label_set_color(UILabel* label, SDL_Color color);

// ============================================================================
// PANEL COMPONENT
// ============================================================================

struct UIPanel {
    SDL_Rect bounds;
    char title[128];
    bool visible;
    bool collapsible;
    bool collapsed;
    
    // Styling
    SDL_Color bg_color;
    SDL_Color border_color;
    SDL_Color title_bg_color;
    SDL_Color title_text_color;
    
    // Children (managed externally, panel just provides container)
    void** children;
    int num_children;
};

UIPanel* ui_panel_create(int x, int y, int w, int h, const char* title);
void ui_panel_destroy(UIPanel* panel);
void ui_panel_render(UIPanel* panel, SDL_Renderer* renderer);
bool ui_panel_handle_event(UIPanel* panel, SDL_Event* event);
void ui_panel_set_collapsible(UIPanel* panel, bool collapsible);
void ui_panel_set_collapsed(UIPanel* panel, bool collapsed);
bool ui_panel_is_collapsed(const UIPanel* panel);
SDL_Rect ui_panel_get_content_bounds(const UIPanel* panel);

// ============================================================================
// DIALOG COMPONENT
// ============================================================================

typedef enum {
    DIALOG_OK,
    DIALOG_OK_CANCEL,
    DIALOG_YES_NO,
    DIALOG_YES_NO_CANCEL
} DialogType;

typedef enum {
    DIALOG_RESULT_NONE,
    DIALOG_RESULT_OK,
    DIALOG_RESULT_CANCEL,
    DIALOG_RESULT_YES,
    DIALOG_RESULT_NO
} DialogResult;

struct UIDialog {
    SDL_Rect bounds;
    char title[128];
    char message[512];
    DialogType type;
    DialogResult result;
    bool visible;
    bool modal;
    
    // Buttons
    UIButton* buttons[3];
    int num_buttons;
    
    // Styling
    SDL_Color bg_color;
    SDL_Color title_bg_color;
    SDL_Color title_text_color;
    SDL_Color text_color;
    SDL_Color border_color;
    
    // Callback
    void (*on_close)(DialogResult result, void* user_data);
    void* user_data;
};

UIDialog* ui_dialog_create(int x, int y, int w, int h, const char* title, 
                           const char* message, DialogType type);
void ui_dialog_destroy(UIDialog* dialog);
void ui_dialog_render(UIDialog* dialog, SDL_Renderer* renderer);
bool ui_dialog_handle_event(UIDialog* dialog, SDL_Event* event);
void ui_dialog_show(UIDialog* dialog);
void ui_dialog_hide(UIDialog* dialog);
bool ui_dialog_is_visible(const UIDialog* dialog);
DialogResult ui_dialog_get_result(const UIDialog* dialog);
void ui_dialog_set_callback(UIDialog* dialog, 
                            void (*on_close)(DialogResult, void*), void* user_data);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Draw text (external function from app)
 */
extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);

/**
 * Check if point is inside rect
 */
bool ui_point_in_rect(int x, int y, const SDL_Rect* rect);

/**
 * Clamp value between min and max
 */
float ui_clamp(float value, float min, float max);

#endif // UI_COMPONENTS_H