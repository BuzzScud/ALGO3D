// app/ui/left_sidebar.c - Left sidebar with vertical tabs
#include "../app_common.h"
#include <SDL2/SDL_ttf.h>

#define SIDEBAR_WIDTH 200
#define TAB_HEIGHT 50
#define ICON_SIZE 24
#define SUBMENU_HEIGHT 40

extern TTF_Font* get_global_font(void);
extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);

// Render main tab button
static void render_main_tab(SDL_Renderer* renderer, AppState* state, MainTab tab, const char* label, int y) {
    SDL_Rect button = {0, y, SIDEBAR_WIDTH, TAB_HEIGHT};
    
    // Highlight if active
    if (state->main_tab == tab) {
        SDL_SetRenderDrawColor(renderer, 60, 80, 120, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 40, 50, 65, 255);
    }
    
    SDL_RenderFillRect(renderer, &button);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 80, 100, 140, 255);
    SDL_RenderDrawRect(renderer, &button);
    
    // Label
    SDL_Color text_color = {255, 255, 255, 255};
    draw_text(renderer, label, 10, y + 15, text_color);
}

// Render left sidebar with vertical tabs
void render_left_sidebar(SDL_Renderer* renderer, AppState* state) {
    // Background
    SDL_SetRenderDrawColor(renderer, 30, 40, 50, 255);
    SDL_Rect sidebar = {0, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &sidebar);
    
    // Render main tabs
    int y = 10;
    
    // Visualization Tab (keep existing visualization tabs)
    render_main_tab(renderer, state, MAIN_TAB_VISUALIZATION, "[VIZ] Visualization", y);
    y += TAB_HEIGHT;
    
    // AI Tab
    render_main_tab(renderer, state, MAIN_TAB_AI, "[AI] AI", y);
    y += TAB_HEIGHT;
    
    // Models Tab
    render_main_tab(renderer, state, MAIN_TAB_MODELS, "[MOD] Models", y);
    y += TAB_HEIGHT;
    
    // System Tab
    render_main_tab(renderer, state, MAIN_TAB_SYSTEM, "[SYS] System", y);
    y += TAB_HEIGHT;
    
    // Data Tab
    render_main_tab(renderer, state, MAIN_TAB_DATA, "[DATA] Data", y);
    y += TAB_HEIGHT;
}

// Render submenu button
static void render_submenu_button(SDL_Renderer* renderer, const char* label, int x, int y, int width, bool active) {
    SDL_Rect button = {x, y, width, SUBMENU_HEIGHT};
    
    // Highlight if active
    if (active) {
        SDL_SetRenderDrawColor(renderer, 70, 90, 130, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 50, 60, 75, 255);
    }
    
    SDL_RenderFillRect(renderer, &button);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 90, 110, 150, 255);
    SDL_RenderDrawRect(renderer, &button);
    
    // Label
    SDL_Color text_color = {255, 255, 255, 255};
    draw_text(renderer, label, x + 10, y + 12, text_color);
}

// Render submenu for Visualization tab
static void render_visualization_submenu(SDL_Renderer* renderer, AppState* state) {
    int x = SIDEBAR_WIDTH + 10;
    int button_width = 120;
    int spacing = 5;
    
    render_submenu_button(renderer, "Prime Spiral", x, 0, button_width, 
                         state->sub_tab.viz_sub == VIZ_SUB_PRIME_SPIRAL);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Calculator", x, 0, button_width,
                         state->sub_tab.viz_sub == VIZ_SUB_CALCULATOR);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Spheres", x, 0, button_width,
                         state->sub_tab.viz_sub == VIZ_SUB_SPHERES);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Folding", x, 0, button_width,
                         state->sub_tab.viz_sub == VIZ_SUB_PRIME_FOLDING);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Video", x, 0, button_width,
                         state->sub_tab.viz_sub == VIZ_SUB_VIDEO_GENERATOR);
}

// Render submenu for AI tab
static void render_ai_submenu(SDL_Renderer* renderer, AppState* state) {
    int x = SIDEBAR_WIDTH + 10;
    int button_width = 120;
    int spacing = 5;
    
    render_submenu_button(renderer, "LLM Chat", x, 0, button_width,
                         state->sub_tab.ai_sub == AI_SUB_LLM);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Training", x, 0, button_width,
                         state->sub_tab.ai_sub == AI_SUB_TRAINING);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Research", x, 0, button_width,
                         state->sub_tab.ai_sub == AI_SUB_RESEARCH);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Crawler", x, 0, button_width,
                         state->sub_tab.ai_sub == AI_SUB_CRAWLER);
}

// Render submenu for System tab
static void render_system_submenu(SDL_Renderer* renderer, AppState* state) {
    int x = SIDEBAR_WIDTH + 10;
    int button_width = 120;
    
    render_submenu_button(renderer, "Benchmark", x, 0, button_width,
                         state->sub_tab.system_sub == SYSTEM_SUB_BENCHMARK);
}

// Render submenu for Data tab
static void render_data_submenu(SDL_Renderer* renderer, AppState* state) {
    int x = SIDEBAR_WIDTH + 10;
    int button_width = 150;
    int spacing = 5;
    
    render_submenu_button(renderer, "URL Manager", x, 0, button_width,
                         state->sub_tab.data_sub == DATA_SUB_URL_MANAGER);
    x += button_width + spacing;
    
    render_submenu_button(renderer, "Downloaded Files", x, 0, button_width,
                         state->sub_tab.data_sub == DATA_SUB_DOWNLOADED_FILES);
}

// Render horizontal submenu bar
void render_submenu(SDL_Renderer* renderer, AppState* state) {
    // Background
    SDL_SetRenderDrawColor(renderer, 40, 50, 60, 255);
    SDL_Rect submenu_bar = {SIDEBAR_WIDTH, 0, WINDOW_WIDTH - SIDEBAR_WIDTH, SUBMENU_HEIGHT};
    SDL_RenderFillRect(renderer, &submenu_bar);
    
    // Render submenu items based on main tab
    switch (state->main_tab) {
        case MAIN_TAB_VISUALIZATION:
            render_visualization_submenu(renderer, state);
            break;
        case MAIN_TAB_AI:
            render_ai_submenu(renderer, state);
            break;
        case MAIN_TAB_MODELS:
            // Models tab has no submenu (single tab)
            break;
        case MAIN_TAB_SYSTEM:
            render_system_submenu(renderer, state);
            break;
        case MAIN_TAB_DATA:
            render_data_submenu(renderer, state);
            break;
        case MAIN_TAB_COUNT:
            break;
    }
}

// Handle sidebar click
void handle_sidebar_click(AppState* state, int x, int y) {
    if (x > SIDEBAR_WIDTH) return;
    
    int tab_index = (y - 10) / TAB_HEIGHT;
    
    switch (tab_index) {
        case 0:
            state->main_tab = MAIN_TAB_VISUALIZATION;
            state->sub_tab.viz_sub = VIZ_SUB_PRIME_SPIRAL;
            break;
        case 1:
            state->main_tab = MAIN_TAB_AI;
            state->sub_tab.ai_sub = AI_SUB_LLM;
            break;
        case 2:
            state->main_tab = MAIN_TAB_MODELS;
            break;
        case 3:
            state->main_tab = MAIN_TAB_SYSTEM;
            state->sub_tab.system_sub = SYSTEM_SUB_BENCHMARK;
            break;
        case 4:
            state->main_tab = MAIN_TAB_DATA;
            state->sub_tab.data_sub = DATA_SUB_URL_MANAGER;
            break;
    }
}

// Handle submenu click
void handle_submenu_click(AppState* state, int x, int y) {
    if (y > SUBMENU_HEIGHT) return;
    if (x < SIDEBAR_WIDTH) return;
    
    int button_width = 120;
    int spacing = 5;
    int button_index = (x - SIDEBAR_WIDTH - 10) / (button_width + spacing);
    
    switch (state->main_tab) {
        case MAIN_TAB_VISUALIZATION:
            if (button_index >= 0 && button_index < VIZ_SUB_COUNT) {
                state->sub_tab.viz_sub = (VisualizationSubTab)button_index;
            }
            break;
        case MAIN_TAB_AI:
            if (button_index >= 0 && button_index < AI_SUB_COUNT) {
                state->sub_tab.ai_sub = (AISubTab)button_index;
            }
            break;
        case MAIN_TAB_SYSTEM:
            if (button_index >= 0 && button_index < SYSTEM_SUB_COUNT) {
                state->sub_tab.system_sub = (SystemSubTab)button_index;
            }
            break;
        case MAIN_TAB_DATA:
            if (button_index >= 0 && button_index < DATA_SUB_COUNT) {
                state->sub_tab.data_sub = (DataSubTab)button_index;
            }
            break;
        default:
            break;
    }
}