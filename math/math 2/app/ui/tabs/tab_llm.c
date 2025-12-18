/**
 * LLM Tab - Complete Rewrite with Crystalline UI System
 * 
 * COMPLETE REWRITE using the Training Tab as reference.
 * Uses PURE Crystalline UI - NO legacy SDL_Rect code.
 * 
 * Layout:
 * - Left Area (75%): Chat interface with message history and input
 * - Right Area (25%): Control panel with model selection and parameters
 * 
 * Reference: tab_training.c (the CORRECT implementation)
 */

#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/global_layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/geometry.h"
#include "../button_sizes.h"
#include "../../cllm_integration.h"
#include "../../../include/cllm_model_registry.h"
#include "../../../include/cllm_utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_CHAT_MESSAGES 100
#define MAX_MESSAGE_LENGTH 2048

// Chat message structure
typedef struct {
    char text[MAX_MESSAGE_LENGTH];
    bool is_user;
    time_t timestamp;
} ChatMessage;

// Chat history
static ChatMessage chat_history[MAX_CHAT_MESSAGES];
static int chat_message_count = 0;

// LLM Tab State - Owns its model independently
typedef struct {
    // Model ownership (OWNED BY THIS TAB)
    CLLMModel* model;                // NULL when not loaded
    char model_path[512];
    char model_name[256];
    bool model_loaded;
    
    // Inference state
    bool is_generating;
    pthread_t inference_thread;
    bool should_stop;
    
    // Inference parameters
    float temperature;
    int max_tokens;
    int top_k;
    float top_p;
    
    // Inference statistics
    struct {
        uint64_t total_tokens_generated;
        uint64_t total_inferences;
        float avg_tokens_per_second;
        time_t last_inference_time;
    } stats;
} LLMTabState;

// UI State - Pure Crystalline UI
static struct {
    // Main interface
    CrystallineTextArea* chat_area;
    CrystallineInput* message_input;
    CrystallineButton* btn_send;
    CrystallineButton* btn_clear;
    
    // Control panel
    CrystallineDropdown* model_dropdown;
    CrystallineSlider* slider_temperature;
    CrystallineSlider* slider_tokens;
    CrystallineSlider* slider_top_k;
    CrystallineSlider* slider_top_p;
    
    // Control panel buttons
    CrystallineButton* btn_browse_models;
    CrystallineButton* btn_new_thread;
    
    // State
    bool initialized;
    char selected_model[256];
    
    // NEW: LLM tab state (owns model independently)
    LLMTabState tab_state;
    
} llm_ui = {0};

/**
 * Model Management Functions
 */

// Forward declarations
static void llm_tab_unload_model(void);

/**
 * Load a model for inference
 */
static bool llm_tab_load_model(const char* model_name) {
    if (!model_name || !model_name[0]) {
        fprintf(stderr, "No model name provided\n");
        return false;
    }
    
    // Unload existing model if any
    llm_tab_unload_model();
    
    // Build model path
    char model_path[512];
    extern bool model_registry_get_path(const char* name, char* path_out);
    if (!model_registry_get_path(model_name, model_path)) {
        fprintf(stderr, "Failed to get path for model: %s\n", model_name);
        return false;
    }
    
    // Load model
    printf("Loading model for inference: %s\n", model_path);
    extern CLLMModel* cllm_read_model(const char* path);
    CLLMModel* model = cllm_read_model(model_path);
    if (!model) {
        fprintf(stderr, "Failed to load model: %s\n", model_path);
        return false;
    }
    
    // Store in tab state
    llm_ui.tab_state.model = model;
    snprintf(llm_ui.tab_state.model_path, 512, "%s", model_path);
    snprintf(llm_ui.tab_state.model_name, 256, "%s", model_name);
    llm_ui.tab_state.model_loaded = true;
    
    printf("✓ Model loaded for inference: %s (%zu vocab, %zu dim, %u layers)\n",
           model_name, model->vocab_size, model->embedding_dim, model->num_layers);
    
    return true;
}

/**
 * Unload the current model
 */
static void llm_tab_unload_model(void) {
    if (!llm_ui.tab_state.model_loaded) {
        return;
    }
    
    // Stop generation if active
    if (llm_ui.tab_state.is_generating) {
        llm_ui.tab_state.should_stop = true;
        if (llm_ui.tab_state.inference_thread) {
            pthread_join(llm_ui.tab_state.inference_thread, NULL);
        }
    }
    
    // Free model
    if (llm_ui.tab_state.model) {
        printf("Unloading model: %s\n", llm_ui.tab_state.model_name);
        extern void cllm_free_model(CLLMModel* model);
        cllm_free_model(llm_ui.tab_state.model);
        llm_ui.tab_state.model = NULL;
    }
    
    // Clear state
    llm_ui.tab_state.model_loaded = false;
    llm_ui.tab_state.model_name[0] = '\0';
    llm_ui.tab_state.model_path[0] = '\0';
    
    printf("✓ Model unloaded\n");
    
    // NOTE: Inference state cleanup happens in on_model_selected() and cleanup_llm_tab()
    // where AppState is available. This is a design issue that should be fixed by
    // moving inference state into LLMTabState instead of global AppState.
}

/**
 * Add message to chat history
 */
static void add_chat_message(const char* text, bool is_user) {
    if (!text || chat_message_count >= MAX_CHAT_MESSAGES) return;
    
    ChatMessage* msg = &chat_history[chat_message_count++];
    strncpy(msg->text, text, MAX_MESSAGE_LENGTH - 1);
    msg->text[MAX_MESSAGE_LENGTH - 1] = '\0';
    msg->is_user = is_user;
    msg->timestamp = time(NULL);
    
    // Add to Crystalline TextArea
    if (llm_ui.chat_area) {
        CrystallineMessageType msg_type = is_user ? 
            CRYSTALLINE_MESSAGE_USER : CRYSTALLINE_MESSAGE_ASSISTANT;
        
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
        
        crystalline_textarea_add_message(llm_ui.chat_area, msg_type, text, timestamp);
    }
}

/**
 * Clear chat history
 */
void clear_chat_history(void) {
    chat_message_count = 0;
    
    if (llm_ui.chat_area) {
        crystalline_textarea_clear(llm_ui.chat_area);
    }
}

/**
 * Model dropdown callback
 */
static void on_model_selected(int index, void* data) {
    printf("=== LLM MODEL SELECTION CALLBACK: index=%d ===\n", index);
    AppState* state = (AppState*)data;
    if (!state || !llm_ui.model_dropdown) return;
    
    // Get selected model name from model registry
    const ModelMetadata* metadata = model_registry_get_at_index((uint32_t)index);
    const char* model_name = metadata ? metadata->name : NULL;
    if (model_name) {
        strncpy(llm_ui.selected_model, model_name, sizeof(llm_ui.selected_model) - 1);
        llm_ui.selected_model[sizeof(llm_ui.selected_model) - 1] = '\0';
        printf("LLM MODEL SELECTED: '%s' (index %d)\n", llm_ui.selected_model, index);
        
        // Load the model using tab state
        if (llm_tab_load_model(model_name)) {
            // Clean up previous inference context
            if (state->cllm_inference) {
                extern void cllm_inference_cleanup(CLLMInference* inference);
                cllm_inference_cleanup(state->cllm_inference);
                state->cllm_inference = NULL;
            }
            
            // Create inference context
            extern CLLMInference* cllm_inference_init(CLLMModel* model);
            state->cllm_inference = cllm_inference_init(llm_ui.tab_state.model);
            
            if (state->cllm_inference) {
                // Set parameters from sliders
                extern void cllm_set_temperature(CLLMInference* inference, float temperature);
                extern void cllm_set_max_tokens(CLLMInference* inference, int max_tokens);
                cllm_set_temperature(state->cllm_inference, state->llm_temperature);
                cllm_set_max_tokens(state->cllm_inference, state->llm_max_tokens);
                
                printf("✓ Model loaded for inference\n");
                add_chat_message("Model loaded successfully. You can now start chatting!", false);
            } else {
                printf("ERROR: Failed to create inference context\n");
                add_chat_message("Error: Failed to create inference context.", false);
                llm_tab_unload_model();
            }
        } else {
            printf("ERROR: Failed to load model\n");
            add_chat_message("Error: Failed to load model.", false);
        }
    } else {
        printf("ERROR: Could not get model metadata for index %d\n", index);
    }
}

/**
 * Button Callbacks
 */
static void on_send_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state || !llm_ui.message_input) return;
    
    const char* input_text = crystalline_input_get_text(llm_ui.message_input);
    if (!input_text || strlen(input_text) == 0) return;
    
    // CRITICAL FIX: Copy input text before clearing the input field
    // The input_text pointer becomes invalid after clearing
    char input_copy[MAX_MESSAGE_LENGTH];
    strncpy(input_copy, input_text, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0';
    
    printf("=== SEND BUTTON CLICKED ===\n");
    printf("Input: %s\n", input_copy);
    
    // Add user message to chat
    add_chat_message(input_copy, true);
    
    // Clear input
    crystalline_input_set_text(llm_ui.message_input, "");
    
    // Generate AI response
    if (state->cllm_inference) {
        char response[MAX_MESSAGE_LENGTH];
        extern int app_generate_text(AppState* state, const char* prompt, char* output, size_t output_size);
        int result = app_generate_text(state, input_copy, response, sizeof(response));
        
        if (result > 0) {
            add_chat_message(response, false);
        } else {
            add_chat_message("Error: Failed to generate response. Make sure a model is loaded.", false);
        }
    } else {
        add_chat_message("Error: No model loaded. Please load a model first.", false);
    }
}

static void on_clear_clicked(void* data) {
    (void)data;
    printf("=== CLEAR BUTTON CLICKED ===\n");
    clear_chat_history();
}

static void on_browse_models_clicked(void* data) {
    (void)data;
    printf("=== BROWSE MODELS BUTTON CLICKED ===\n");
    // TODO: Show model browser dialog
}

static void on_new_thread_clicked(void* data) {
    (void)data;
    printf("=== NEW THREAD BUTTON CLICKED ===\n");
    clear_chat_history();
}

/**
 * Slider Callbacks
 */
static void on_temperature_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    state->llm_temperature = value;
    printf("Temperature: %.2f\n", value);
}

static void on_tokens_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    state->llm_max_tokens = (int)value;
    printf("Max Tokens: %d\n", (int)value);
}

/**
 * Initialize LLM Tab
 * EXACT pattern from training tab - using TOP-LEFT coordinates
 */
void init_llm_tab(AppState* state) {
    if (llm_ui.initialized) return;
    if (!state) return;
    
    printf("=== INITIALIZING LLM TAB ===\n");
    
    extern TTF_Font* get_global_font(void);
    TTF_Font* font = get_global_font();
    
    // Chat area uses RENDER_WIDTH, control panel uses CONTROL_PANEL area
    int chat_width = RENDER_WIDTH - 10;  // 1070px (full render width minus padding)
    int control_width = CONTROL_PANEL_WIDTH - 20;  // 300px (control panel minus padding)
    
    // Calculate available height (same as training tab)
    int available_height = WINDOW_HEIGHT - RENDER_OFFSET_Y - 20;  // Full render area
    
    printf("LLM TAB LAYOUT DEBUG:\n");
    printf("  WINDOW_HEIGHT=%d, RENDER_OFFSET_Y=%d\n", WINDOW_HEIGHT, RENDER_OFFSET_Y);
    printf("  available_height=%d\n", available_height);
    printf("  chat_width=%d, control_width=%d\n", 
           chat_width, control_width);
    
    // === CHAT AREA (LEFT SIDE) ===
    
    // Chat display area (top part) - leave 100px at bottom for input
    int chat_x = RENDER_OFFSET_X + 10;
    int chat_y = RENDER_OFFSET_Y + 10;
    int chat_w = chat_width - 20;
    int chat_h = available_height - 110;  // Leave room for input field at bottom
    
    printf("  CHAT: x=%d, y=%d, w=%d, h=%d\n", chat_x, chat_y, chat_w, chat_h);
    
    llm_ui.chat_area = crystalline_textarea_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        chat_x + chat_w / 2.0f,  // TOP-LEFT + width/2 = CENTER X
        chat_y + chat_h / 2.0f,  // TOP-LEFT + height/2 = CENTER Y
        chat_w,
        chat_h,
        font
    );
    
    // Input field (bottom part) - positioned at bottom of render area
    int input_x = RENDER_OFFSET_X + 10;
    int input_y = RENDER_OFFSET_Y + available_height - 90;  // 90px from bottom
    int input_w = chat_width - 130;  // Leave room for send button
    int input_h = 80;
    
    printf("  INPUT: x=%d, y=%d, w=%d, h=%d\n", input_x, input_y, input_w, input_h);
    
    llm_ui.message_input = crystalline_input_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        input_x + input_w / 2.0f,  // TOP-LEFT + width/2 = CENTER X
        input_y + input_h / 2.0f,  // TOP-LEFT + height/2 = CENTER Y
        input_w,
        input_h,
        "Type your message...",
        font
    );
    crystalline_input_set_callbacks(llm_ui.message_input, NULL, NULL, state);
    
    // Send button (next to input)
    int send_x = RENDER_OFFSET_X + chat_width - 110;
    int send_w = 100;
    
    llm_ui.btn_send = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        send_x + send_w / 2.0f,  // TOP-LEFT + width/2 = CENTER X
        input_y + input_h / 2.0f,
        send_w,
        input_h,
        "SEND",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_send, on_send_clicked, state);
    
    // === CONTROL PANEL (RIGHT SIDE) ===
    // Position in the ACTUAL control panel area (1280-1600)
    int ctrl_x = WINDOW_WIDTH - CONTROL_PANEL_WIDTH + 10;  // 1290
    int ctrl_w = control_width;  // 300
    int ctrl_y = RENDER_OFFSET_Y + 10;  // Start at same Y as chat area
    float slider_center_x = ctrl_x + ctrl_w / 2.0f;  // 1440
    
    printf("  CONTROL PANEL: x=%d, y=%d, w=%d, slider_center_x=%.1f\n", 
           ctrl_x, ctrl_y, ctrl_w, slider_center_x);
    
    // Model dropdown (TOP of control panel)
    float dropdown_height = 40.0f;
    llm_ui.model_dropdown = crystalline_dropdown_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y + dropdown_height / 2.0f,
        ctrl_w,
        dropdown_height,
        font
    );
    crystalline_dropdown_set_callback(llm_ui.model_dropdown, on_model_selected, state);
    ctrl_y += 70;  // Move down for next element
    

    // Temperature slider (TOP JUSTIFIED - fixed spacing)
    llm_ui.slider_temperature = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y,
        ctrl_w,
        30,
        0.0f,
        2.0f
    );
    crystalline_slider_set_value(llm_ui.slider_temperature, state->llm_temperature);
    crystalline_slider_set_callback(llm_ui.slider_temperature, on_temperature_changed, state);
    ctrl_y += 70;
    
    // Max tokens slider
    llm_ui.slider_tokens = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y,
        ctrl_w,
        30,
        1.0f,
        2048.0f
    );
    crystalline_slider_set_value(llm_ui.slider_tokens, (float)state->llm_max_tokens);
    crystalline_slider_set_callback(llm_ui.slider_tokens, on_tokens_changed, state);
    ctrl_y += 70;
    
    // Top-K slider
    llm_ui.slider_top_k = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y,
        ctrl_w,
        30,
        1.0f,
        100.0f
    );
    crystalline_slider_set_value(llm_ui.slider_top_k, 50.0f);
    ctrl_y += 70;
    
    // Top-P slider
    llm_ui.slider_top_p = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y,
        ctrl_w,
        30,
        0.0f,
        1.0f
    );
    crystalline_slider_set_value(llm_ui.slider_top_p, 0.9f);
    ctrl_y += 90;
    
    // Browse Models button
    llm_ui.btn_browse_models = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y,
        ctrl_w,
        40,
        "Browse Models",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_browse_models, on_browse_models_clicked, state);
    ctrl_y += 60;
    
    // New Thread button
    llm_ui.btn_new_thread = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y,
        ctrl_w,
        40,
        "New Thread",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_new_thread, on_new_thread_clicked, state);
    ctrl_y += 60;
    
    // Clear button
    llm_ui.btn_clear = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x,
        ctrl_y,
        ctrl_w,
        40,
        "Clear Chat",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_clear, on_clear_clicked, state);
    
    llm_ui.initialized = true;
    printf("=== LLM TAB INITIALIZED ===\n");
}

/**
 * Draw LLM Tab
 * Pattern copied from draw_training_tab()
 */
void draw_llm_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    // Initialize on first draw
    if (!llm_ui.initialized) {
        init_llm_tab(state);
    }
    
    // Render all Crystalline UI elements
    // NO manual SDL_RenderFillRect calls!
    
    if (llm_ui.chat_area) {
        crystalline_textarea_render(llm_ui.chat_area, renderer);
    }
    
    if (llm_ui.message_input) {
        crystalline_input_render(llm_ui.message_input, renderer);
    }
    
    if (llm_ui.btn_send) {
        crystalline_button_render(llm_ui.btn_send, renderer);
    }
    
    if (llm_ui.btn_clear) {
        crystalline_button_render(llm_ui.btn_clear, renderer);
    }
    
    // Render sliders BEFORE dropdown (so dropdown appears on top when expanded)
    if (llm_ui.slider_temperature) {
        crystalline_slider_render(llm_ui.slider_temperature, renderer);
    }
    
    if (llm_ui.slider_tokens) {
        crystalline_slider_render(llm_ui.slider_tokens, renderer);
    }
    
    if (llm_ui.slider_top_k) {
        crystalline_slider_render(llm_ui.slider_top_k, renderer);
    }
    
    if (llm_ui.slider_top_p) {
        crystalline_slider_render(llm_ui.slider_top_p, renderer);
    }
    
    if (llm_ui.btn_browse_models) {
        crystalline_button_render(llm_ui.btn_browse_models, renderer);
    }
    
    if (llm_ui.btn_new_thread) {
        crystalline_button_render(llm_ui.btn_new_thread, renderer);
    }
    
    // Populate model dropdown once (registry already scanned at startup)
    static bool dropdown_populated = false;
    if (!dropdown_populated && llm_ui.model_dropdown) {
        uint32_t model_count = model_registry_count();
        
        if (model_count > 0) {
            char** model_names = malloc(model_count * sizeof(char*));
            if (model_names) {
                for (uint32_t i = 0; i < model_count; i++) {
                    const ModelMetadata* metadata = model_registry_get_at_index(i);
                    model_names[i] = metadata ? (char*)metadata->name : "";
                }
                crystalline_dropdown_set_options(llm_ui.model_dropdown, model_names, (int)model_count);
                printf("LLM MODEL DROPDOWN: Populated with %u models\n", model_count);
                free(model_names);
                dropdown_populated = true;
            }
        }
    }
    
    
    // Render model dropdown LAST (so it appears on top when expanded)

    if (llm_ui.model_dropdown) {
        crystalline_dropdown_render(llm_ui.model_dropdown, renderer);
    }
    
    
    // Draw labels for dropdown and sliders
    extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
    SDL_Color text_color = {220, 220, 220, 255};
    
    // Calculate label positions (same as init)
    int ctrl_x = WINDOW_WIDTH - CONTROL_PANEL_WIDTH + 10;  // 1290
    int label_x = ctrl_x + 5;
    int label_y = RENDER_OFFSET_Y + 40;
    
    char label[64];
    
    // Model dropdown label
    draw_text(renderer, "Model:", label_x, label_y, text_color);
    label_y += 70;
    
    // Temperature label
    snprintf(label, sizeof(label), "Temperature: %.2f", 
             crystalline_slider_get_value(llm_ui.slider_temperature));
    draw_text(renderer, label, label_x, label_y, text_color);
    label_y += 70;
    
    // Max tokens label
    snprintf(label, sizeof(label), "Max Tokens: %d", 
             (int)crystalline_slider_get_value(llm_ui.slider_tokens));
    draw_text(renderer, label, label_x, label_y, text_color);
    label_y += 70;
    
    // Top-K label
    snprintf(label, sizeof(label), "Top-K: %d", 
             (int)crystalline_slider_get_value(llm_ui.slider_top_k));
    draw_text(renderer, label, label_x, label_y, text_color);
    label_y += 70;
    
    // Top-P label
    snprintf(label, sizeof(label), "Top-P: %.2f", 
             crystalline_slider_get_value(llm_ui.slider_top_p));
    draw_text(renderer, label, label_x, label_y, text_color);
}

/**
 * Event Handlers
 * EXACT pattern from training tab - handles DOWN, UP, and MOTION separately
 */
void handle_llm_tab_mouse_down(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONDOWN;
    dummy_event.button.x = x;
    dummy_event.button.y = y;
    dummy_event.button.button = SDL_BUTTON_LEFT;
    
    // Handle all elements - dropdown, buttons, input, sliders
    if (llm_ui.model_dropdown) crystalline_dropdown_handle_mouse(llm_ui.model_dropdown, &dummy_event);
    if (llm_ui.btn_send) crystalline_button_handle_mouse(llm_ui.btn_send, &dummy_event);
    if (llm_ui.btn_clear) crystalline_button_handle_mouse(llm_ui.btn_clear, &dummy_event);
    if (llm_ui.btn_browse_models) crystalline_button_handle_mouse(llm_ui.btn_browse_models, &dummy_event);
    if (llm_ui.btn_new_thread) crystalline_button_handle_mouse(llm_ui.btn_new_thread, &dummy_event);
    if (llm_ui.message_input) crystalline_input_handle_mouse(llm_ui.message_input, &dummy_event);
    if (llm_ui.slider_temperature) crystalline_slider_handle_mouse(llm_ui.slider_temperature, &dummy_event);
    if (llm_ui.slider_tokens) crystalline_slider_handle_mouse(llm_ui.slider_tokens, &dummy_event);
    if (llm_ui.slider_top_k) crystalline_slider_handle_mouse(llm_ui.slider_top_k, &dummy_event);
    if (llm_ui.slider_top_p) crystalline_slider_handle_mouse(llm_ui.slider_top_p, &dummy_event);
}

void handle_llm_tab_mouse_up(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONUP;
    dummy_event.button.x = x;
    dummy_event.button.y = y;
    dummy_event.button.button = SDL_BUTTON_LEFT;
    
    // CRITICAL: Handle dropdown and buttons FIRST - this triggers callbacks!
    if (llm_ui.model_dropdown) crystalline_dropdown_handle_mouse(llm_ui.model_dropdown, &dummy_event);
    if (llm_ui.btn_send) crystalline_button_handle_mouse(llm_ui.btn_send, &dummy_event);
    if (llm_ui.btn_clear) crystalline_button_handle_mouse(llm_ui.btn_clear, &dummy_event);
    if (llm_ui.btn_browse_models) crystalline_button_handle_mouse(llm_ui.btn_browse_models, &dummy_event);
    if (llm_ui.btn_new_thread) crystalline_button_handle_mouse(llm_ui.btn_new_thread, &dummy_event);
    
    // Handle sliders
    if (llm_ui.slider_temperature) crystalline_slider_handle_mouse(llm_ui.slider_temperature, &dummy_event);
    if (llm_ui.slider_tokens) crystalline_slider_handle_mouse(llm_ui.slider_tokens, &dummy_event);
    if (llm_ui.slider_top_k) crystalline_slider_handle_mouse(llm_ui.slider_top_k, &dummy_event);
    if (llm_ui.slider_top_p) crystalline_slider_handle_mouse(llm_ui.slider_top_p, &dummy_event);
}

void handle_llm_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEMOTION;
    dummy_event.motion.x = x;
    dummy_event.motion.y = y;
    
    // Handle hover states for dropdown and buttons
    if (llm_ui.model_dropdown) crystalline_dropdown_handle_mouse(llm_ui.model_dropdown, &dummy_event);
    if (llm_ui.btn_send) crystalline_button_handle_mouse(llm_ui.btn_send, &dummy_event);
    if (llm_ui.btn_clear) crystalline_button_handle_mouse(llm_ui.btn_clear, &dummy_event);
    if (llm_ui.btn_browse_models) crystalline_button_handle_mouse(llm_ui.btn_browse_models, &dummy_event);
    if (llm_ui.btn_new_thread) crystalline_button_handle_mouse(llm_ui.btn_new_thread, &dummy_event);
    
    // Handle slider dragging
    if (llm_ui.slider_temperature) crystalline_slider_handle_mouse(llm_ui.slider_temperature, &dummy_event);
    if (llm_ui.slider_tokens) crystalline_slider_handle_mouse(llm_ui.slider_tokens, &dummy_event);
    if (llm_ui.slider_top_k) crystalline_slider_handle_mouse(llm_ui.slider_top_k, &dummy_event);
    if (llm_ui.slider_top_p) crystalline_slider_handle_mouse(llm_ui.slider_top_p, &dummy_event);
}

// Legacy handler for compatibility
void handle_llm_tab_click(AppState* state, int x, int y) {
    handle_llm_tab_mouse_down(state, x, y);
}

void handle_llm_tab_keydown(AppState* state, int key) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_KEYDOWN;
    dummy_event.key.keysym.sym = key;
    
    if (llm_ui.message_input) {
        crystalline_input_handle_keyboard(llm_ui.message_input, &dummy_event);
    }
}

void handle_llm_tab_key(AppState* state, int key) {
    handle_llm_tab_keydown(state, key);
}

void handle_llm_tab_text_input(AppState* state, const char* text) {
    if (!state || !text) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_TEXTINPUT;
    strncpy(dummy_event.text.text, text, sizeof(dummy_event.text.text) - 1);
    
    if (llm_ui.message_input) {
        crystalline_input_handle_keyboard(llm_ui.message_input, &dummy_event);
    }
}

void handle_llm_tab_scroll(AppState* state, int wheel_y) {
    if (!state) return;
    
    // TODO: Scroll chat area
    (void)wheel_y;
}

// Input callbacks for compatibility
void llm_input_on_change(const char* text, void* user_data) {
    (void)text;
    (void)user_data;
}

void llm_input_on_submit(const char* text, void* user_data) {
    if (!text || !user_data) return;
    
    // Trigger send button
    on_send_clicked(user_data);
}

/**
 * Cleanup LLM Tab
 */
void cleanup_llm_tab(void) {
    if (!llm_ui.initialized) return;
    
    // Unload model if loaded
    llm_tab_unload_model();
    
    // Cleanup Crystalline UI elements
    if (llm_ui.chat_area) crystalline_textarea_destroy(llm_ui.chat_area);
    if (llm_ui.message_input) crystalline_input_destroy(llm_ui.message_input);
    if (llm_ui.btn_send) crystalline_button_destroy(llm_ui.btn_send);
    if (llm_ui.btn_clear) crystalline_button_destroy(llm_ui.btn_clear);
    if (llm_ui.model_dropdown) crystalline_dropdown_destroy(llm_ui.model_dropdown);
    if (llm_ui.slider_temperature) crystalline_slider_destroy(llm_ui.slider_temperature);
    if (llm_ui.slider_tokens) crystalline_slider_destroy(llm_ui.slider_tokens);
    if (llm_ui.slider_top_k) crystalline_slider_destroy(llm_ui.slider_top_k);
    if (llm_ui.slider_top_p) crystalline_slider_destroy(llm_ui.slider_top_p);
    if (llm_ui.btn_browse_models) crystalline_button_destroy(llm_ui.btn_browse_models);
    if (llm_ui.btn_new_thread) crystalline_button_destroy(llm_ui.btn_new_thread);
    
    memset(&llm_ui, 0, sizeof(llm_ui));
    
    printf("✓ LLM tab cleaned up\n");
}