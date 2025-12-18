// app/ui/tabs/tab_url_manager_crystalline.c - URL Manager Tab (Crystalline UI)
#include "../../app_common.h"
#include "../../../src/crawler/crawler_url_manager.h"
#include "../../../src/crawler/url_database.h"
#include "../crystalline/elements.h"
#include "../crystalline/geometry.h"
#include "../crystalline/layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/color.h"
#include "../../input_manager.h"
#include <SDL2/SDL_ttf.h>

extern TTF_Font* get_global_font(void);
extern InputManager* g_input_manager;

#define SIDEBAR_WIDTH 200
#define SUBMENU_HEIGHT 40
#define PADDING 10
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define MAX_URLS_DISPLAY 30

// URL Manager state
typedef struct {
    CrawlerURLManager* url_manager;
    bool initialized;
    int selected_url_idx;
    int scroll_offset;
    char add_url_input[512];
    bool add_url_active;
    
    // URL list
    URLEntry** url_list;
    int url_list_count;
    bool url_list_dirty;
    
    // Filter state
    char filter_status[32];
    char filter_domain[128];
} URLManagerState;

static URLManagerState url_state = {0};

// Crystalline UI elements
static CrystallineButton* btn_add = NULL;
static CrystallineButton* btn_remove = NULL;
static CrystallineButton* btn_block = NULL;
static CrystallineButton* btn_export = NULL;
static CrystallineButton* btn_import = NULL;
static CrystallinePanel* panel_urls = NULL;
static CrystallinePanel* panel_actions = NULL;
static CrystallinePanel* panel_filters = NULL;
static CrystallineList* list_urls = NULL;
static CrystallineInput* input_add_url = NULL;
static bool ui_initialized = false;

// Initialize state
static void init_url_manager_state(void) {
    if (url_state.initialized) return;
    
    url_state.url_manager = crawler_url_manager_create("data/crawler");
    if (!url_state.url_manager) {
        fprintf(stderr, "ERROR: Failed to create URL manager\n");
        return;
    }
    
    url_state.initialized = true;
    url_state.selected_url_idx = -1;
    url_state.scroll_offset = 0;
    url_state.url_list_dirty = true;
    url_state.add_url_active = false;
    url_state.add_url_input[0] = '\0';
    strcpy(url_state.filter_status, "all");
    url_state.filter_domain[0] = '\0';
    
    printf("URL Manager tab: Initialized\n");
}

// Refresh URL list
static void refresh_url_list(void) {
    if (!url_state.url_manager) return;
    
    if (url_state.url_list) {
        url_db_free_entries(url_state.url_list, url_state.url_list_count);
        url_state.url_list = NULL;
    }
    
    URLDatabase* db = crawler_url_manager_get_database(url_state.url_manager);
    if (!db) return;
    
    // Build query based on filters
    char query[512] = "";
    if (strcmp(url_state.filter_status, "all") != 0) {
        snprintf(query, sizeof(query), "status='%s'", url_state.filter_status);
    }
    
    url_state.url_list = url_db_query(db, query[0] ? query : NULL, &url_state.url_list_count);
    url_state.url_list_dirty = false;
    
    printf("URL Manager: Loaded %d URLs\n", url_state.url_list_count);
}

void draw_url_manager_tab(SDL_Renderer* renderer, AppState* state) {
    (void)state;
    
    init_url_manager_state();
    
    if (url_state.url_list_dirty) {
        refresh_url_list();
    }
    
    // Initialize UI elements if needed
    if (!ui_initialized) {
        int content_x = SIDEBAR_WIDTH;
        int content_y = SUBMENU_HEIGHT;
        int content_width = RENDER_WIDTH;  // Use RENDER_WIDTH to account for control panel
        int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
        
        // Create URL list panel (left, 50%)
        int col1_width = (content_width * 50) / 100;
        float panel1_width = (float)col1_width - 20.0f;
        float panel1_height = (float)content_height - 20.0f;
        panel_urls = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            content_x + panel1_width / 2.0f + 10.0f,
            content_y + panel1_height / 2.0f + 10.0f,
            panel1_width,
            panel1_height,
            "URL LIST",
            get_global_font()
        );
        
        // Create actions panel (middle, 25%)
        int col2_width = (content_width * 25) / 100;
        int col2_x = content_x + col1_width + PADDING;
        float panel2_width = (float)col2_width - 20.0f;
        float panel2_height = (float)content_height - 20.0f;
        panel_actions = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            col2_x + panel2_width / 2.0f,
            content_y + panel2_height / 2.0f + 10.0f,
            panel2_width,
            panel2_height,
            "ACTIONS",
            get_global_font()
        );
        
        // Create filters panel (right, 25%)
        int col3_width = (content_width * 25) / 100;
        int col3_x = col2_x + col2_width + PADDING;
        float panel3_width = (float)col3_width - 20.0f;
        float panel3_height = (float)content_height - 20.0f;
        panel_filters = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            col3_x + panel3_width / 2.0f,
            content_y + panel3_height / 2.0f + 10.0f,
            panel3_width,
            panel3_height,
            "FILTERS",
            get_global_font()
        );
        
        // Create URL list
        // For RECTANGULAR: x,y is CENTER, not top-left
        float url_list_width = (float)col1_width - 40.0f;
        float url_list_display_height = 30.0f * 10;  // item_height * visible_items
        list_urls = crystalline_list_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            content_x + url_list_width / 2.0f + PADDING,
            content_y + url_list_display_height / 2.0f + 80.0f,
            url_list_width,
            30.0f,  // item_height
            get_global_font()
        );
        
        // Create input field for adding URLs
        input_add_url = crystalline_input_create(
            CRYSTALLINE_STYLE_RECTANGULAR,  // Standard rectangle
            col2_x + ((float)col2_width - 40.0f) / 2.0f + 20.0f,
            content_y + 80.0f + 20.0f,
            (float)col2_width - 40.0f,
            40.0f,  // Increased from 30 to 40 for better usability
            "Enter URL...",
            get_global_font()
        );
        
        // Create action buttons (5 buttons in vertical layout)
        float button_x = col2_x + col2_width / 2.0f;
        float button_start_y = content_y + 130.0f;
        float button_spacing = 60.0f;
        
        btn_add = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_x, button_start_y,
            50.0f, 0.0f, "ADD", get_global_font()
        );
        
        btn_remove = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_x, button_start_y + button_spacing,
            50.0f, 0.0f, "REMOVE", get_global_font()
        );
        
        btn_block = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_x, button_start_y + button_spacing * 2,
            50.0f, 0.0f, "BLOCK", get_global_font()
        );
        
        btn_export = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_x, button_start_y + button_spacing * 3,
            50.0f, 0.0f, "EXPORT", get_global_font()
        );
        
        btn_import = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_x, button_start_y + button_spacing * 4,
            50.0f, 0.0f, "IMPORT", get_global_font()
        );
        
        ui_initialized = true;
    }
    
    // Colors
    SDL_Color text_color = crystalline_color_from_frequency(432.0f);
    SDL_Color accent_color = crystalline_color_from_frequency(639.0f);
    
    int content_x = SIDEBAR_WIDTH;
    int content_y = SUBMENU_HEIGHT;
    int content_width = RENDER_WIDTH;  // Use RENDER_WIDTH to account for control panel
    
    // Draw title
    float center_x = content_x + content_width / 2.0f;
    CrystallinePoint title_pos = crystalline_point_cartesian(center_x, content_y + 20.0f);
    crystalline_draw_text_centered(renderer, "URL MANAGER", title_pos, text_color, NULL);
    
    // Draw panels
    crystalline_panel_render(panel_urls, renderer);
    crystalline_panel_render(panel_actions, renderer);
    crystalline_panel_render(panel_filters, renderer);
    
    // Update URL list
    crystalline_list_clear(list_urls);
    if (url_state.url_list && url_state.url_list_count > 0) {
        for (int i = 0; i < url_state.url_list_count && i < MAX_URLS_DISPLAY; i++) {
            URLEntry* entry = url_state.url_list[i];
            if (!entry) continue;
            
            char url_display[1024];
            char truncated_url[256];
            strncpy(truncated_url, entry->url, sizeof(truncated_url) - 1);
            truncated_url[sizeof(truncated_url) - 1] = '\0';
            snprintf(url_display, sizeof(url_display),
                     "%s | %s | %s",
                     entry->domain,
                     entry->status,
                     truncated_url);
            
            crystalline_list_add_item(list_urls, url_display);
        }
    }
    
    // Draw URL list
    crystalline_list_render(list_urls, renderer);
    
    // Draw input field
    crystalline_input_render(input_add_url, renderer);
    
    // Draw buttons
    crystalline_button_render(btn_add, renderer);
    crystalline_button_render(btn_remove, renderer);
    crystalline_button_render(btn_block, renderer);
    crystalline_button_render(btn_export, renderer);
    crystalline_button_render(btn_import, renderer);
    
    // Draw URL count
    char count_text[128];
    snprintf(count_text, sizeof(count_text), "Total URLs: %d", url_state.url_list_count);
    CrystallinePoint count_pos = crystalline_point_cartesian(center_x, content_y + WINDOW_HEIGHT - 30.0f);
    crystalline_draw_text_centered(renderer, count_text, count_pos, accent_color, NULL);
    
    // Render inputs through InputManager
    if (g_input_manager) {
        input_manager_render(g_input_manager, renderer, get_global_font(), TAB_URL_MANAGER);
    }
}

void handle_url_manager_click(AppState* state, int x, int y) {
    if (!state) return;
    
    init_url_manager_state();
    if (!url_state.url_manager || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Pass BUTTONDOWN event to all interactive elements
    // Actions will be triggered on BUTTONUP
    if (list_urls) crystalline_list_handle_mouse(list_urls, &event);
    if (input_add_url) crystalline_input_handle_mouse(input_add_url, &event);
    if (btn_add) crystalline_button_handle_mouse(btn_add, &event);
    if (btn_remove) crystalline_button_handle_mouse(btn_remove, &event);
    if (btn_block) crystalline_button_handle_mouse(btn_block, &event);
    if (btn_export) crystalline_button_handle_mouse(btn_export, &event);
    if (btn_import) crystalline_button_handle_mouse(btn_import, &event);
}

// Alias for main.c compatibility
void handle_url_manager_tab_mouse_down(AppState* state, int x, int y) {
    handle_url_manager_click(state, x, y);
}

/**
 * Handle URL manager tab mouse motion for hover states
 */
void handle_url_manager_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state) return;
    
    init_url_manager_state();
    if (!url_state.url_manager || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Update hover states
    if (list_urls) crystalline_list_handle_mouse(list_urls, &event);
    if (input_add_url) crystalline_input_handle_mouse(input_add_url, &event);
    if (btn_add) crystalline_button_handle_mouse(btn_add, &event);
    if (btn_remove) crystalline_button_handle_mouse(btn_remove, &event);
    if (btn_block) crystalline_button_handle_mouse(btn_block, &event);
    if (btn_export) crystalline_button_handle_mouse(btn_export, &event);
    if (btn_import) crystalline_button_handle_mouse(btn_import, &event);
}

/**
 * Handle URL manager tab mouse up for click callbacks
 */
void handle_url_manager_tab_mouse_up(AppState* state, int x, int y) {
    if (!state) return;
    
    init_url_manager_state();
    if (!url_state.url_manager || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check URL list click
    if (crystalline_list_handle_mouse(list_urls, &event)) {
        int selected = crystalline_list_get_selected(list_urls);
        if (selected >= 0 && selected < url_state.url_list_count) {
            url_state.selected_url_idx = selected;
            printf("Selected URL: %s\n", url_state.url_list[selected]->url);
        }
        return;
    }
    
    // Check input field click
    if (crystalline_input_handle_mouse(input_add_url, &event)) {
        url_state.add_url_active = true;
        return;
    }
    
    // Check Add button
    if (crystalline_button_handle_mouse(btn_add, &event)) {
        const char* url_text = crystalline_input_get_text(input_add_url);
        if (url_text && url_text[0] != '\0') {
            crawler_url_manager_add(url_state.url_manager, url_text, "manual");
            printf("Added URL: %s\n", url_text);
            crystalline_input_set_text(input_add_url, "");
            url_state.url_list_dirty = true;
        }
        return;
    }
    
    // Check Remove button
    if (crystalline_button_handle_mouse(btn_remove, &event)) {
        if (url_state.selected_url_idx >= 0 && url_state.selected_url_idx < url_state.url_list_count) {
            URLEntry* entry = url_state.url_list[url_state.selected_url_idx];
            URLDatabase* db = crawler_url_manager_get_database(url_state.url_manager);
            if (db) {
                url_db_remove(db, entry->id);
                printf("Removed URL: %s\n", entry->url);
                url_state.url_list_dirty = true;
                url_state.selected_url_idx = -1;
            }
        }
        return;
    }
    
    // Check Block button
    if (crystalline_button_handle_mouse(btn_block, &event)) {
        if (url_state.selected_url_idx >= 0 && url_state.selected_url_idx < url_state.url_list_count) {
            URLEntry* entry = url_state.url_list[url_state.selected_url_idx];
            URLDatabase* db = crawler_url_manager_get_database(url_state.url_manager);
            if (db) {
                url_db_block(db, entry->id);
                printf("Blocked URL: %s\n", entry->url);
                url_state.url_list_dirty = true;
            }
        }
        return;
    }
    
    // Check Export button
    if (crystalline_button_handle_mouse(btn_export, &event)) {
        printf("Export functionality not yet implemented\n");
        return;
    }
    
    // Check Import button
    if (crystalline_button_handle_mouse(btn_import, &event)) {
        printf("Import functionality not yet implemented\n");
        return;
    }
}

void handle_url_manager_keyboard(AppState* state, SDL_Event* event) {
    if (!state || !event || !ui_initialized) return;
    
    // Handle input field keyboard events
    crystalline_input_handle_keyboard(input_add_url, event);
}

void cleanup_url_manager_tab(void) {
    if (url_state.url_list) {
        url_db_free_entries(url_state.url_list, url_state.url_list_count);
        url_state.url_list = NULL;
    }
    
    if (url_state.url_manager) {
        crawler_url_manager_destroy(url_state.url_manager);
        url_state.url_manager = NULL;
    }
    
    if (btn_add) crystalline_button_destroy(btn_add);
    if (btn_remove) crystalline_button_destroy(btn_remove);
    if (btn_block) crystalline_button_destroy(btn_block);
    if (btn_export) crystalline_button_destroy(btn_export);
    if (btn_import) crystalline_button_destroy(btn_import);
    if (panel_urls) crystalline_panel_destroy(panel_urls);
    if (panel_actions) crystalline_panel_destroy(panel_actions);
    if (panel_filters) crystalline_panel_destroy(panel_filters);
    if (list_urls) crystalline_list_destroy(list_urls);
    if (input_add_url) crystalline_input_destroy(input_add_url);
    
    btn_add = btn_remove = btn_block = btn_export = btn_import = NULL;
    panel_urls = panel_actions = panel_filters = NULL;
    list_urls = NULL;
    input_add_url = NULL;
    
    url_state.initialized = false;
    ui_initialized = false;
}// Legacy function for compatibility
void handle_url_manager_tab_click(AppState* state, int x, int y) {
    handle_url_manager_tab_mouse_down(state, x, y);
}
