/**
 * Models Tab - Complete Rewrite Using Pure Crystalline UI System
 * 
 * This is a COMPLETE REWRITE using the Crystalline UI system.
 * NO legacy code, NO old component system, NO manual SDL rendering.
 * 
 * Layout:
 * - Left Panel (70%): Model list with scrolling
 * - Right Panel (30%): Control buttons and model info
 */

#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/global_layout.h"
#include "../button_sizes.h"
#include "cllm_model_registry.h"
#include <stdio.h>
#include <string.h>

// UI State
static struct {
    // Panels
    CrystallinePanel* list_panel;
    CrystallinePanel* control_panel;
    
    // List
    CrystallineList* model_list;
    
    // Buttons
    CrystallineButton* btn_load;
    CrystallineButton* btn_delete;
    CrystallineButton* btn_refresh;
    CrystallineButton* btn_create;
    
    // Text areas
    CrystallineTextArea* info_display;
    
    // State
    int selected_model_index;
    char selected_model_name[256];
    bool initialized;
    
} g_models_ui = {0};

/**
 * Update model info display
 */
static void update_model_info(void) {
    if (!g_models_ui.info_display) return;
    
    // Clear previous messages
    crystalline_textarea_clear(g_models_ui.info_display);
    
    if (g_models_ui.selected_model_index < 0) {
        crystalline_textarea_add_message(g_models_ui.info_display, 
            CRYSTALLINE_MESSAGE_SYSTEM, 
            "No model selected", 
            "");
        return;
    }
    
    // Get model metadata from registry
    const ModelMetadata* metadata = model_registry_get(g_models_ui.selected_model_name);
    if (!metadata) {
        crystalline_textarea_add_message(g_models_ui.info_display, 
            CRYSTALLINE_MESSAGE_SYSTEM, 
            "Model not found", 
            "");
        return;
    }
    
    // Format info text
    char info_text[2048];
    snprintf(info_text, sizeof(info_text),
        "Model: %s\n\n"
        "Path: %s\n"
        "Valid: %s\n\n"
        "Vocab Size: %u\n"
        "Embedding Dim: %u\n"
        "Num Layers: %u\n"
        "Num Heads: %u\n"
        "Max Seq Len: %u\n\n"
        "File Size: %llu bytes\n"
        "Created: %s"
        "Modified: %s",
        metadata->name,
        metadata->path,
        metadata->is_valid ? "Yes" : "No",
        metadata->vocab_size,
        metadata->embedding_dim,
        metadata->num_layers,
        metadata->num_heads,
        metadata->max_seq_len,
        (unsigned long long)metadata->file_size,
        ctime(&metadata->created_time),
        ctime(&metadata->modified_time)
    );
    
    crystalline_textarea_add_message(g_models_ui.info_display, 
        CRYSTALLINE_MESSAGE_SYSTEM, 
        info_text, 
        "");
}

/**
 * Refresh model list
 */
static void refresh_model_list(void) {
    if (!g_models_ui.model_list) return;
    
    // Scan models directory
    model_registry_scan();
    
    // Get model count from registry
    uint32_t model_count = model_registry_count();
    
    if (model_count == 0) {
        crystalline_list_set_items(g_models_ui.model_list, NULL, 0);
        printf("No models found\n");
        return;
    }
    
    // Create string array for list
    char** model_names = malloc(model_count * sizeof(char*));
    for (uint32_t i = 0; i < model_count; i++) {
        const ModelMetadata* metadata = model_registry_get_at_index(i);
        if (metadata) {
            model_names[i] = (char*)metadata->name;
        }
    }
    
    // Update list
    crystalline_list_set_items(g_models_ui.model_list, model_names, model_count);
    
    free(model_names);
    
    printf("Refreshed model list: %u models\n", model_count);
}

/**
 * Button callbacks
 */
static void on_load_clicked(void* data) {
    (void)data;
    
    if (g_models_ui.selected_model_index < 0) {
        printf("No model selected\n");
        return;
    }
    
    printf("Load button clicked for model: %s\n", g_models_ui.selected_model_name);
    printf("Note: Models Tab is for viewing only. Use Training or LLM tabs to load models.\n");
    
    // Just refresh the display
    update_model_info();
}

static void on_delete_clicked(void* data) {
    (void)data;
    
    if (g_models_ui.selected_model_index < 0) {
        printf("No model selected\n");
        return;
    }
    
    printf("Deleting model: %s\n", g_models_ui.selected_model_name);
    
    // Delete model file and remove from registry
    if (model_registry_delete(g_models_ui.selected_model_name)) {
        printf("Model deleted successfully\n");
        g_models_ui.selected_model_index = -1;
        g_models_ui.selected_model_name[0] = '\0';
        refresh_model_list();
        update_model_info();
    } else {
        printf("Failed to delete model\n");
    }
}

static void on_refresh_clicked(void* data) {
    (void)data;
    printf("Refreshing model list\n");
    refresh_model_list();
}

static void on_create_clicked(void* data) {
    (void)data;
    printf("Create model button clicked (not implemented yet)\n");
    // TODO: Implement model creation dialog
}

/**
 * List selection callback
 */
static void on_model_selected(int index, void* data) {
    (void)data;
    
    g_models_ui.selected_model_index = index;
    
    // Get model metadata from registry
    const ModelMetadata* metadata = model_registry_get_at_index((uint32_t)index);
    
    if (metadata && index >= 0) {
        strncpy(g_models_ui.selected_model_name, metadata->name, 
                sizeof(g_models_ui.selected_model_name) - 1);
        printf("Selected model: %s\n", g_models_ui.selected_model_name);
        update_model_info();
    }
}

/**
 * Initialize Models Tab
 */
void init_models_tab(AppState* state) {
    if (g_models_ui.initialized) return;
    
    printf("Initializing Models Tab with Crystalline UI\n");
    
    // Get global font
    extern TTF_Font* get_global_font();
    TTF_Font* font = get_global_font();
    if (!font) {
        printf("ERROR: Failed to get global font\n");
        return;
    }
    
    // Calculate layout using RENDER_WIDTH (not WINDOW_WIDTH - SIDEBAR_WIDTH)
    int content_width = RENDER_WIDTH;  // 1080px (accounts for sidebar + control panel)
    int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
    
    // Split into left (70%) and right (30%)
    int list_width = (int)(content_width * 0.70f);
    int control_width = content_width - list_width;
    
    // Calculate TOP-LEFT positions first
    int list_x = RENDER_OFFSET_X;
    int list_y = SUBMENU_HEIGHT;
    int list_w = list_width - 20;
    int list_h = content_height - 20;
    
    int control_x = RENDER_OFFSET_X + list_width + 10;
    int control_y = SUBMENU_HEIGHT;
    int control_w = control_width - 30;
    int control_h = content_height - 20;
    
    // Convert to CENTER coordinates for Crystalline UI
    g_models_ui.list_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        list_x + list_w / 2.0f,
        list_y + list_h / 2.0f,
        list_w,
        list_h,
        "Available Models",
        font
    );
    
    g_models_ui.control_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        control_x + control_w / 2.0f,
        control_y + control_h / 2.0f,
        control_w,
        control_h,
        "Controls",
        font
    );
    
    // Create model list inside left panel
    int list_content_x = list_x + 10;
    int list_content_y = list_y + 40;
    int list_content_w = list_w - 20;
    int list_content_h = list_h - 50;
    
    g_models_ui.model_list = crystalline_list_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        list_content_x + list_content_w / 2.0f,
        list_content_y + list_content_h / 2.0f,
        list_content_w,
        40,  // Item height
        font
    );
    
    crystalline_list_set_callback(g_models_ui.model_list, on_model_selected, state);
    
    // Create control elements inside right panel (top-justified with fixed spacing)
    int btn_x = control_x + 10;
    int btn_w = control_w - 20;
    int btn_h = 50;
    int btn_spacing = 70;
    
    int btn_y = control_y + 40;
    
    // Load button
    g_models_ui.btn_load = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        btn_x + btn_w / 2.0f,
        btn_y + btn_h / 2.0f,
        btn_w,
        btn_h,
        "Load Model",
        font
    );
    crystalline_button_set_callback(g_models_ui.btn_load, on_load_clicked, state);
    btn_y += btn_spacing;
    
    // Delete button
    g_models_ui.btn_delete = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        btn_x + btn_w / 2.0f,
        btn_y + btn_h / 2.0f,
        btn_w,
        btn_h,
        "Delete Model",
        font
    );
    crystalline_button_set_callback(g_models_ui.btn_delete, on_delete_clicked, state);
    btn_y += btn_spacing;
    
    // Refresh button
    g_models_ui.btn_refresh = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        btn_x + btn_w / 2.0f,
        btn_y + btn_h / 2.0f,
        btn_w,
        btn_h,
        "Refresh List",
        font
    );
    crystalline_button_set_callback(g_models_ui.btn_refresh, on_refresh_clicked, state);
    btn_y += btn_spacing;
    
    // Create button
    g_models_ui.btn_create = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        btn_x + btn_w / 2.0f,
        btn_y + btn_h / 2.0f,
        btn_w,
        btn_h,
        "Create New",
        font
    );
    crystalline_button_set_callback(g_models_ui.btn_create, on_create_clicked, state);
    btn_y += btn_spacing + 20;
    
    // Info display
    int info_h = control_h - (btn_y - control_y) - 20;
    g_models_ui.info_display = crystalline_textarea_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        btn_x + btn_w / 2.0f,
        btn_y + info_h / 2.0f,
        btn_w,
        info_h,
        font
    );
    
    // Add initial message to textarea
    crystalline_textarea_add_message(g_models_ui.info_display, 
        CRYSTALLINE_MESSAGE_SYSTEM, 
        "No model selected", 
        "");
    
    // Initialize state
    g_models_ui.selected_model_index = -1;
    g_models_ui.selected_model_name[0] = '\0';
    g_models_ui.initialized = true;
    
    // Load initial model list
    refresh_model_list();
    
    printf("Models Tab initialized successfully\n");
}

/**
 * Cleanup Models Tab
 */
void cleanup_models_tab(void) {
    if (!g_models_ui.initialized) return;
    
    // Cleanup Crystalline UI elements
    if (g_models_ui.list_panel) crystalline_panel_destroy(g_models_ui.list_panel);
    if (g_models_ui.control_panel) crystalline_panel_destroy(g_models_ui.control_panel);
    if (g_models_ui.model_list) crystalline_list_destroy(g_models_ui.model_list);
    if (g_models_ui.btn_load) crystalline_button_destroy(g_models_ui.btn_load);
    if (g_models_ui.btn_delete) crystalline_button_destroy(g_models_ui.btn_delete);
    if (g_models_ui.btn_refresh) crystalline_button_destroy(g_models_ui.btn_refresh);
    if (g_models_ui.btn_create) crystalline_button_destroy(g_models_ui.btn_create);
    if (g_models_ui.info_display) crystalline_textarea_destroy(g_models_ui.info_display);
    
    memset(&g_models_ui, 0, sizeof(g_models_ui));
}

/**
 * Render Models Tab
 */
void render_models_tab(SDL_Renderer* renderer, AppState* state) {
    (void)state;  // Unused in current implementation
    if (!g_models_ui.initialized) return;
    
    // Render panels
    if (g_models_ui.list_panel) {
        crystalline_panel_render(g_models_ui.list_panel, renderer);
    }
    if (g_models_ui.control_panel) {
        crystalline_panel_render(g_models_ui.control_panel, renderer);
    }
    
    // Render list
    if (g_models_ui.model_list) {
        crystalline_list_render(g_models_ui.model_list, renderer);
    }
    
    // Render buttons
    if (g_models_ui.btn_load) {
        crystalline_button_render(g_models_ui.btn_load, renderer);
    }
    if (g_models_ui.btn_delete) {
        crystalline_button_render(g_models_ui.btn_delete, renderer);
    }
    if (g_models_ui.btn_refresh) {
        crystalline_button_render(g_models_ui.btn_refresh, renderer);
    }
    if (g_models_ui.btn_create) {
        crystalline_button_render(g_models_ui.btn_create, renderer);
    }
    
    // Render info display
    if (g_models_ui.info_display) {
        crystalline_textarea_render(g_models_ui.info_display, renderer);
    }
}

/**
 * Event handlers following standardized pattern
 */
void handle_models_tab_mouse_down(SDL_MouseButtonEvent* event, AppState* state) {
    (void)state;  // Unused in current implementation
    if (!g_models_ui.initialized) return;
    
    SDL_Event sdl_event = {0};
    sdl_event.type = SDL_MOUSEBUTTONDOWN;
    sdl_event.button = *event;
    
    // Handle list
    if (g_models_ui.model_list) {
        crystalline_list_handle_mouse(g_models_ui.model_list, &sdl_event);
    }
    
    // Handle buttons
    if (g_models_ui.btn_load) {
        crystalline_button_handle_mouse(g_models_ui.btn_load, &sdl_event);
    }
    if (g_models_ui.btn_delete) {
        crystalline_button_handle_mouse(g_models_ui.btn_delete, &sdl_event);
    }
    if (g_models_ui.btn_refresh) {
        crystalline_button_handle_mouse(g_models_ui.btn_refresh, &sdl_event);
    }
    if (g_models_ui.btn_create) {
        crystalline_button_handle_mouse(g_models_ui.btn_create, &sdl_event);
    }
}

void handle_models_tab_mouse_up(SDL_MouseButtonEvent* event, AppState* state) {
    (void)state;  // Unused in current implementation
    if (!g_models_ui.initialized) return;
    
    SDL_Event sdl_event = {0};
    sdl_event.type = SDL_MOUSEBUTTONUP;
    sdl_event.button = *event;
    
    // Handle list
    if (g_models_ui.model_list) {
        crystalline_list_handle_mouse(g_models_ui.model_list, &sdl_event);
    }
    
    // Handle buttons
    if (g_models_ui.btn_load) {
        crystalline_button_handle_mouse(g_models_ui.btn_load, &sdl_event);
    }
    if (g_models_ui.btn_delete) {
        crystalline_button_handle_mouse(g_models_ui.btn_delete, &sdl_event);
    }
    if (g_models_ui.btn_refresh) {
        crystalline_button_handle_mouse(g_models_ui.btn_refresh, &sdl_event);
    }
    if (g_models_ui.btn_create) {
        crystalline_button_handle_mouse(g_models_ui.btn_create, &sdl_event);
    }
}

void handle_models_tab_mouse_motion(SDL_MouseMotionEvent* event, AppState* state) {
    (void)state;  // Unused in current implementation
    if (!g_models_ui.initialized) return;
    
    SDL_Event sdl_event = {0};
    sdl_event.type = SDL_MOUSEMOTION;
    sdl_event.motion = *event;
    
    // Handle list
    if (g_models_ui.model_list) {
        crystalline_list_handle_mouse(g_models_ui.model_list, &sdl_event);
    }
    
    // Handle buttons
    if (g_models_ui.btn_load) {
        crystalline_button_handle_mouse(g_models_ui.btn_load, &sdl_event);
    }
    if (g_models_ui.btn_delete) {
        crystalline_button_handle_mouse(g_models_ui.btn_delete, &sdl_event);
    }
    if (g_models_ui.btn_refresh) {
        crystalline_button_handle_mouse(g_models_ui.btn_refresh, &sdl_event);
    }
    if (g_models_ui.btn_create) {
        crystalline_button_handle_mouse(g_models_ui.btn_create, &sdl_event);
    }
}