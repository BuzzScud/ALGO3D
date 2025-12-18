// app/ui/tabs/tab_downloaded_files_crystalline.c - Downloaded Files Viewer Tab (Crystalline UI)
#include "../../app_common.h"
#include "../../../src/crawler/crawler_url_manager.h"
#include "../../../src/crawler/url_database.h"
#include "../crystalline/elements.h"
#include "../crystalline/geometry.h"
#include "../crystalline/layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/color.h"
#include <SDL2/SDL_ttf.h>
#include <sys/stat.h>
#include <dirent.h>

extern TTF_Font* get_global_font(void);

// Use constants from app_common.h
#define SIDEBAR_WIDTH 200
#define SUBMENU_HEIGHT 40
#define BUTTON_HEIGHT 30
#define PADDING 10
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define MAX_FILES_DISPLAY 20
#define DOWNLOADS_DIR "data/crawler/downloads"

// File info structure
typedef struct {
    uint64_t url_id;
    char filename[256];
    char url[512];
    char domain[128];
    char file_type[32];
    time_t download_time;
    off_t file_size;
    bool file_exists;
} FileInfo;

// Downloaded Files state
typedef struct {
    CrawlerURLManager* url_manager;
    bool initialized;
    int selected_file_idx;
    int scroll_offset;
    char search_query[256];
    bool show_preview;
    
    // File list
    FileInfo* file_list;
    int file_count;
    bool file_list_dirty;
    
    // Preview
    char preview_content[4096];
    bool preview_loaded;
} DownloadedFilesState;

static DownloadedFilesState files_state = {0};

// Crystalline UI elements
static CrystallineButton* btn_open = NULL;
static CrystallineButton* btn_delete = NULL;
static CrystallineButton* btn_recrawl = NULL;
static CrystallinePanel* panel_browser = NULL;
static CrystallinePanel* panel_preview = NULL;
static CrystallineList* list_files = NULL;
static bool ui_initialized = false;

// Initialize state
static void init_downloaded_files_state(void) {
    if (files_state.initialized) return;
    
    // Create URL manager with database
    files_state.url_manager = crawler_url_manager_create("data/crawler");
    if (!files_state.url_manager) {
        fprintf(stderr, "ERROR: Failed to create URL manager for Downloaded Files tab\n");
        return;
    }
    
    files_state.initialized = true;
    files_state.selected_file_idx = -1;
    files_state.scroll_offset = 0;
    files_state.file_list_dirty = true;
    files_state.preview_loaded = false;
    
    // Create downloads directory if it doesn't exist
    mkdir("data", 0755);
    mkdir("data/crawler", 0755);
    mkdir(DOWNLOADS_DIR, 0755);
    
    printf("Downloaded Files tab: Initialized\n");
}

// Get file size
static off_t get_file_size(const char* filepath) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

// Format file size for display
static void format_file_size(off_t size, char* buffer, size_t buffer_size) {
    if (size < 1024) {
        snprintf(buffer, buffer_size, "%ld B", (long)size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f KB", size / 1024.0);
    } else if (size < 1024 * 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f MB", size / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buffer_size, "%.1f GB", size / (1024.0 * 1024.0 * 1024.0));
    }
}

// Refresh file list from database
static void refresh_file_list(void) {
    if (!files_state.url_manager) return;
    
    // Free old list if exists
    if (files_state.file_list) {
        free(files_state.file_list);
        files_state.file_list = NULL;
    }
    
    // Get database from manager
    URLDatabase* db = crawler_url_manager_get_database(files_state.url_manager);
    if (!db) return;
    
    // Query crawled URLs only
    int url_count = 0;
    URLEntry** urls = url_db_query(db, "status='crawled'", &url_count);
    
    if (!urls || url_count == 0) {
        files_state.file_count = 0;
        files_state.file_list_dirty = false;
        if (urls) url_db_free_entries(urls, url_count);
        return;
    }
    
    // Allocate file list
    files_state.file_list = (FileInfo*)calloc(url_count, sizeof(FileInfo));
    files_state.file_count = 0;
    
    // Build file list
    for (int i = 0; i < url_count; i++) {
        URLEntry* entry = urls[i];
        if (!entry) continue;
        
        FileInfo* file = &files_state.file_list[files_state.file_count];
        
        // Extract filename from URL (last part of path)
        const char* last_slash = strrchr(entry->path, '/');
        if (last_slash && last_slash[1] != '\0') {
            strncpy(file->filename, last_slash + 1, sizeof(file->filename) - 1);
        } else {
            snprintf(file->filename, sizeof(file->filename), "index.%s", entry->file_type);
        }
        
        // Copy URL info
        file->url_id = entry->id;
        strncpy(file->url, entry->url, sizeof(file->url) - 1);
        strncpy(file->domain, entry->domain, sizeof(file->domain) - 1);
        strncpy(file->file_type, entry->file_type, sizeof(file->file_type) - 1);
        file->download_time = entry->last_crawled;
        
        // Check if file exists and get size
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", DOWNLOADS_DIR, file->filename);
        file->file_size = get_file_size(filepath);
        file->file_exists = (file->file_size > 0);
        
        files_state.file_count++;
    }
    
    url_db_free_entries(urls, url_count);
    files_state.file_list_dirty = false;
    
    printf("Downloaded Files: Loaded %d files\n", files_state.file_count);
}

void draw_downloaded_files_tab(SDL_Renderer* renderer, AppState* state) {
    (void)state;
    
    // Initialize if needed
    init_downloaded_files_state();
    
    // Refresh file list if dirty
    if (files_state.file_list_dirty) {
        refresh_file_list();
    }
    
    // Initialize UI elements if needed
    if (!ui_initialized) {
        int content_x = SIDEBAR_WIDTH;
        int content_y = SUBMENU_HEIGHT;
        int content_width = RENDER_WIDTH;  // Use RENDER_WIDTH to account for control panel
        int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
        
        float center_x __attribute__((unused)) = content_x + content_width / 2.0f;
        float center_y __attribute__((unused)) = content_y + content_height / 2.0f;
        
        // Create file browser panel (left, Flower of Life pattern)
        int col1_width = (content_width * 40) / 100;
        float browser_width = (float)col1_width - 20.0f;
        float browser_height = (float)content_height - 20.0f;
        panel_browser = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            content_x + browser_width / 2.0f + 10.0f,
            content_y + browser_height / 2.0f + 10.0f,
            browser_width,
            browser_height,
            "DOWNLOADED FILES",
            get_global_font()
        );
        
        // Create preview panel (right)
        int col2_width = (content_width * 60) / 100;
        int col2_x = content_x + col1_width + PADDING;
        float preview_width = (float)col2_width - 40.0f;
        float preview_height = (float)content_height - 20.0f;
        panel_preview = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            col2_x + preview_width / 2.0f,
            content_y + preview_height / 2.0f + 10.0f,
            preview_width,
            preview_height,
            "FILE PREVIEW",
            get_global_font()
        );
        
        // Create file list (clock lattice - 60 positions)
        // For RECTANGULAR: x,y is CENTER, not top-left
        float file_list_width = (float)col1_width - 40.0f;
        float file_list_display_height = 30.0f * 10;  // item_height * visible_items
        list_files = crystalline_list_create(
            CRYSTALLINE_STYLE_RECTANGULAR,  // Use rectangular for file list
            content_x + file_list_width / 2.0f + PADDING,
            content_y + file_list_display_height / 2.0f + 100.0f,
            file_list_width,
            30.0f,  // item_height
            get_global_font()
        );
        
        // Create buttons in radial layout (3 buttons, 120° apart)
        float button_y = content_y + content_height - 80.0f;
        float button_center_x = col2_x + col2_width / 2.0f;
        
        btn_open = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_center_x - 140.0f,
            button_y,
            50.0f,   // radius
            0.0f,
            "OPEN",
            get_global_font()
        );
        
        btn_delete = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_center_x,
            button_y,
            50.0f,   // radius
            0.0f,
            "DELETE",
            get_global_font()
        );
        
        btn_recrawl = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            button_center_x + 140.0f,
            button_y,
            50.0f,   // radius
            0.0f,
            "RECRAWL",
            get_global_font()
        );
        
        ui_initialized = true;
    }
    
    // Colors
    SDL_Color text_color = crystalline_color_from_frequency(432.0f);
    SDL_Color accent_color = crystalline_color_from_frequency(639.0f);
    SDL_Color success_color = crystalline_color_from_frequency(528.0f);
    SDL_Color warning_color = crystalline_color_from_frequency(741.0f);
    
    int content_x = SIDEBAR_WIDTH;
    int content_y = SUBMENU_HEIGHT;
    int content_width = RENDER_WIDTH;  // Use RENDER_WIDTH to account for control panel
    
    // Draw title
    float center_x = content_x + content_width / 2.0f;
    CrystallinePoint title_pos = crystalline_point_cartesian(center_x, content_y + 20.0f);
    crystalline_draw_text_centered(renderer, "DOWNLOADED FILES", title_pos, text_color, NULL);
    
    // Draw browser panel
    crystalline_panel_render(panel_browser, renderer);
    
    // Update file list
    crystalline_list_clear(list_files);
    if (files_state.file_list && files_state.file_count > 0) {
        for (int i = 0; i < files_state.file_count && i < MAX_FILES_DISPLAY; i++) {
            FileInfo* file = &files_state.file_list[i];
            
            char file_display[512];
            char size_str[32];
            format_file_size(file->file_size, size_str, sizeof(size_str));
            
            snprintf(file_display, sizeof(file_display),
                     "%s | %s | %s",
                     file->filename,
                     file->domain,
                     size_str);
            
            crystalline_list_add_item(list_files, file_display);
        }
    }
    
    // Draw file list
    crystalline_list_render(list_files, renderer);
    
    // Draw preview panel
    crystalline_panel_render(panel_preview, renderer);
    
    // Draw preview content
    if (files_state.selected_file_idx >= 0 && 
        files_state.selected_file_idx < files_state.file_count) {
        
        FileInfo* file = &files_state.file_list[files_state.selected_file_idx];
        
        int col2_width = (content_width * 60) / 100;
        int col2_x = content_x + (content_width * 40) / 100 + PADDING;
        float preview_x = col2_x + col2_width / 2.0f;
        float preview_y = content_y + 100.0f;
        
        // File info
        char info_text[512];
        snprintf(info_text, sizeof(info_text), "File: %s", file->filename);
        CrystallinePoint info_pos = crystalline_point_cartesian(preview_x, preview_y);
        crystalline_draw_text_centered(renderer, info_text, info_pos, text_color, NULL);
        
        snprintf(info_text, sizeof(info_text), "Domain: %s", file->domain);
        CrystallinePoint domain_pos = crystalline_point_cartesian(preview_x, preview_y + 25.0f);
        crystalline_draw_text_centered(renderer, info_text, domain_pos, accent_color, NULL);
        
        char size_str[32];
        format_file_size(file->file_size, size_str, sizeof(size_str));
        snprintf(info_text, sizeof(info_text), "Size: %s | Type: %s", size_str, file->file_type);
        CrystallinePoint size_pos = crystalline_point_cartesian(preview_x, preview_y + 50.0f);
        crystalline_draw_text_centered(renderer, info_text, size_pos, text_color, NULL);
        
        // Status
        SDL_Color status_color = file->file_exists ? success_color : warning_color;
        const char* status_text = file->file_exists ? "File exists" : "File not found";
        CrystallinePoint status_pos = crystalline_point_cartesian(preview_x, preview_y + 75.0f);
        crystalline_draw_text_centered(renderer, status_text, status_pos, status_color, NULL);
    } else {
        // No file selected
        int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
        int col2_width = (content_width * 60) / 100;
        int col2_x = content_x + (content_width * 40) / 100 + PADDING;
        float preview_x = col2_x + col2_width / 2.0f;
        float preview_y = content_y + content_height / 2.0f;
        
        CrystallinePoint msg_pos = crystalline_point_cartesian(preview_x, preview_y);
        crystalline_draw_text_centered(renderer, "Select a file to preview", msg_pos, accent_color, NULL);
    }
    
    // Draw buttons
    crystalline_button_render(btn_open, renderer);
    crystalline_button_render(btn_delete, renderer);
    crystalline_button_render(btn_recrawl, renderer);
    
    // Draw file count
    char count_text[128];
    snprintf(count_text, sizeof(count_text), "Total Files: %d", files_state.file_count);
    CrystallinePoint count_pos = crystalline_point_cartesian(center_x, content_y + WINDOW_HEIGHT - 30.0f);
    crystalline_draw_text_centered(renderer, count_text, count_pos, text_color, NULL);
}

// Handle click events
void handle_downloaded_files_click(AppState* state, int x, int y) {
    if (!state) return;
    
    init_downloaded_files_state();
    if (!files_state.url_manager || !ui_initialized) return;
    
    // Create SDL event for button handling
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Pass BUTTONDOWN event to all interactive elements
    // Actions will be triggered on BUTTONUP
    if (list_files) crystalline_list_handle_mouse(list_files, &event);
    if (btn_open) crystalline_button_handle_mouse(btn_open, &event);
    if (btn_delete) crystalline_button_handle_mouse(btn_delete, &event);
    if (btn_recrawl) crystalline_button_handle_mouse(btn_recrawl, &event);
}

// Alias for main.c compatibility
void handle_downloaded_files_tab_mouse_down(AppState* state, int x, int y) {
    handle_downloaded_files_click(state, x, y);
}

/**
 * Handle downloaded files tab mouse motion for hover states
 */
void handle_downloaded_files_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state) return;
    
    init_downloaded_files_state();
    if (!files_state.url_manager || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Update hover states
    if (list_files) crystalline_list_handle_mouse(list_files, &event);
    if (btn_open) crystalline_button_handle_mouse(btn_open, &event);
    if (btn_delete) crystalline_button_handle_mouse(btn_delete, &event);
    if (btn_recrawl) crystalline_button_handle_mouse(btn_recrawl, &event);
}

/**
 * Handle downloaded files tab mouse up for click callbacks
 */
void handle_downloaded_files_tab_mouse_up(AppState* state, int x, int y) {
    if (!state) return;
    
    init_downloaded_files_state();
    if (!files_state.url_manager || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check file list click
    if (crystalline_list_handle_mouse(list_files, &event)) {
        int selected = crystalline_list_get_selected(list_files);
        if (selected >= 0 && selected < files_state.file_count) {
            files_state.selected_file_idx = selected;
            files_state.preview_loaded = false;
            printf("Selected file: %s\n", files_state.file_list[selected].filename);
        }
        return;
    }
    
    // Check Open button
    if (crystalline_button_handle_mouse(btn_open, &event)) {
        if (files_state.selected_file_idx >= 0 && 
            files_state.selected_file_idx < files_state.file_count) {
            FileInfo* file = &files_state.file_list[files_state.selected_file_idx];
            
            // Build file path
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", DOWNLOADS_DIR, file->filename);
            
            // Try to open file with system default application
            #ifdef __linux__
            char cmd[2048];
            snprintf(cmd, sizeof(cmd), "xdg-open '%s' 2>/dev/null &", filepath);
            int result = system(cmd);
            if (result == 0) {
                printf("Opened file: %s\n", filepath);
            } else {
                printf("Failed to open file: %s\n", filepath);
            }
            #else
            printf("Open file not implemented for this platform: %s\n", filepath);
            #endif
        } else {
            printf("No file selected\n");
        }
        return;
    }
    
    // Check Delete button
    if (crystalline_button_handle_mouse(btn_delete, &event)) {
        if (files_state.selected_file_idx >= 0 && 
            files_state.selected_file_idx < files_state.file_count) {
            FileInfo* file = &files_state.file_list[files_state.selected_file_idx];
            
            // Build file path
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", DOWNLOADS_DIR, file->filename);
            
            // Delete the file
            if (remove(filepath) == 0) {
                printf("Deleted file: %s\n", filepath);
                
                // Refresh file list
                files_state.file_list_dirty = true;
                files_state.selected_file_idx = -1;
                files_state.preview_loaded = false;
            } else {
                printf("Failed to delete file: %s\n", filepath);
            }
        } else {
            printf("No file selected\n");
        }
        return;
    }
    
    // Check Re-crawl button
    if (crystalline_button_handle_mouse(btn_recrawl, &event)) {
        if (files_state.selected_file_idx >= 0 && 
            files_state.selected_file_idx < files_state.file_count) {
            FileInfo* file = &files_state.file_list[files_state.selected_file_idx];
            
            // Re-crawl: First unblock if blocked, then mark as pending
            URLDatabase* db = crawler_url_manager_get_database(files_state.url_manager);
            if (db) {
                // Unblock first (in case it was blocked)
                url_db_unblock(db, file->url_id);
                
                // Remove the URL and re-add it to mark as pending
                char url_copy[512];
                strncpy(url_copy, file->url, sizeof(url_copy) - 1);
                url_copy[sizeof(url_copy) - 1] = '\0';
                
                url_db_remove(db, file->url_id);
                crawler_url_manager_add(files_state.url_manager, url_copy, "recrawl");
                
                printf("Marked URL for re-crawl: %s\n", file->url);
                
                // Refresh file list
                files_state.file_list_dirty = true;
                files_state.selected_file_idx = -1;
            }
        } else {
            printf("No file selected\n");
        }
        return;
    }
}

// Cleanup function
void cleanup_downloaded_files_tab(void) {
    if (files_state.file_list) {
        free(files_state.file_list);
        files_state.file_list = NULL;
    }
    
    if (files_state.url_manager) {
        crawler_url_manager_destroy(files_state.url_manager);
        files_state.url_manager = NULL;
    }
    
    if (btn_open) {
        crystalline_button_destroy(btn_open);
        btn_open = NULL;
    }
    if (btn_delete) {
        crystalline_button_destroy(btn_delete);
        btn_delete = NULL;
    }
    if (btn_recrawl) {
        crystalline_button_destroy(btn_recrawl);
        btn_recrawl = NULL;
    }
    if (panel_browser) {
        crystalline_panel_destroy(panel_browser);
        panel_browser = NULL;
    }
    if (panel_preview) {
        crystalline_panel_destroy(panel_preview);
        panel_preview = NULL;
    }
    if (list_files) {
        crystalline_list_destroy(list_files);
        list_files = NULL;
    }
    
    files_state.initialized = false;
    ui_initialized = false;
}// Legacy function for compatibility
void handle_downloaded_files_tab_click(AppState* state, int x, int y) {
    handle_downloaded_files_tab_mouse_down(state, x, y);
}
