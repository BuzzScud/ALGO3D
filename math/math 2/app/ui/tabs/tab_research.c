// app/ui/tabs/tab_research_crystalline.c - Research Data Browser Tab (Crystalline UI)
#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/geometry.h"
#include "../crystalline/layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/color.h"
#include "../../input_manager.h"
#include "../model_selector.h"
#include "../../../include/cllm_model_registry.h"
#include <SDL2/SDL_ttf.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

extern TTF_Font* get_global_font(void);
extern InputManager* g_input_manager;

#ifndef DT_REG
#define DT_REG 8
#endif

#define SIDEBAR_WIDTH 200
#define SUBMENU_HEIGHT 40
#define PADDING 10
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define MAX_FILES 200
#define MAX_FILENAME_LENGTH 256
#define MAX_CONTENT_LENGTH 100000

typedef enum {
    SORT_NAME,
    SORT_SIZE,
    SORT_DATE,
    SORT_TYPE
} SortMode;

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char full_path[1024];
    long file_size;
    time_t modified_time;
    char file_type[32];
} ResearchFile;

// Research state
typedef struct {
    ResearchFile files[MAX_FILES];
    int file_count;
    int selected_file;
    int file_list_scroll;
    int content_scroll;
    SortMode sort_mode;
    bool sort_ascending;
    char current_directory[512];
    char search_query[128];
    char file_content[MAX_CONTENT_LENGTH];
    ModelSelector* model_selector;
    char selected_model_name[256];
} ResearchState;

static ResearchState research_state = {
    .file_count = 0,
    .selected_file = -1,
    .file_list_scroll = 0,
    .content_scroll = 0,
    .sort_mode = SORT_NAME,
    .sort_ascending = true,
    .current_directory = "docs/research",
    .search_query = "",
    .model_selector = NULL,
    .selected_model_name = ""
};

// Crystalline UI elements
static CrystallinePanel* panel_files = NULL;
static CrystallinePanel* panel_viewer = NULL;
static CrystallinePanel* panel_controls = NULL;
static CrystallineList* list_files = NULL;
static CrystallineInput* input_search = NULL;
static CrystallineButton* btn_scan = NULL;
static CrystallineButton* btn_refresh = NULL;
static CrystallineButton* btn_up = NULL;
static CrystallineButton* btn_sort_name = NULL;
static CrystallineButton* btn_sort_size = NULL;
static CrystallineButton* btn_sort_date = NULL;
static CrystallineButton* btn_sort_type = NULL;
static bool ui_initialized = false;

// Helper functions
static const char* get_file_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "Unknown";
    
    if (strcmp(ext, ".txt") == 0) return "Text";
    if (strcmp(ext, ".md") == 0) return "Markdown";
    if (strcmp(ext, ".json") == 0) return "JSON";
    if (strcmp(ext, ".csv") == 0) return "CSV";
    if (strcmp(ext, ".log") == 0) return "Log";
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) return "C Code";
    if (strcmp(ext, ".py") == 0) return "Python";
    if (strcmp(ext, ".sh") == 0) return "Shell";
    
    return "Other";
}

static int compare_files(const void* a, const void* b) {
    ResearchFile* f1 = (ResearchFile*)a;
    ResearchFile* f2 = (ResearchFile*)b;
    
    int result = 0;
    switch (research_state.sort_mode) {
        case SORT_NAME:
            result = strcmp(f1->filename, f2->filename);
            break;
        case SORT_SIZE:
            result = (f1->file_size > f2->file_size) - (f1->file_size < f2->file_size);
            break;
        case SORT_DATE:
            result = (f1->modified_time > f2->modified_time) - (f1->modified_time < f2->modified_time);
            break;
        case SORT_TYPE:
            result = strcmp(f1->file_type, f2->file_type);
            break;
    }
    
    return research_state.sort_ascending ? result : -result;
}

static void scan_research_directory(const char* dir_path) {
    research_state.file_count = 0;
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Could not open directory: %s\n", dir_path);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && research_state.file_count < MAX_FILES) {
        if (entry->d_name[0] == '.') continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            // Apply search filter
            if (strlen(research_state.search_query) > 0) {
                if (strstr(entry->d_name, research_state.search_query) == NULL) continue;
            }
            
            ResearchFile* file = &research_state.files[research_state.file_count];
            snprintf(file->filename, MAX_FILENAME_LENGTH, "%s", entry->d_name);
            snprintf(file->full_path, sizeof(file->full_path), "%s", full_path);
            file->file_size = st.st_size;
            file->modified_time = st.st_mtime;
            snprintf(file->file_type, sizeof(file->file_type), "%s", get_file_type(entry->d_name));
            research_state.file_count++;
        }
    }
    closedir(dir);
    
    // Sort files
    qsort(research_state.files, research_state.file_count, sizeof(ResearchFile), compare_files);
    
    printf("Found %d research files in %s\n", research_state.file_count, dir_path);
}

static void load_file_content(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        snprintf(research_state.file_content, sizeof(research_state.file_content), 
                "Error: Could not open file\n%s", filepath);
        return;
    }
    
    size_t bytes_read = fread(research_state.file_content, 1, MAX_CONTENT_LENGTH - 1, f);
    research_state.file_content[bytes_read] = '\0';
    fclose(f);
    research_state.content_scroll = 0;
    
    printf("Loaded file: %s (%zu bytes)\n", filepath, bytes_read);
}

static const char* format_file_size(long size) {
    static char buffer[32];
    if (size < 1024) {
        snprintf(buffer, sizeof(buffer), "%ld B", size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, sizeof(buffer), "%.1f KB", size / 1024.0);
    } else {
        snprintf(buffer, sizeof(buffer), "%.1f MB", size / (1024.0 * 1024.0));
    }
    return buffer;
}

static const char* format_time(time_t time) {
    static char buffer[64];
    struct tm* tm_info = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm_info);
    return buffer;
}

// Model selector callback
static void on_research_model_selected(const char* model_name, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state || !model_name) return;
    
    printf("Research tab: Model '%s' selected (not loaded yet)\n", model_name);
    
    strncpy(research_state.selected_model_name, model_name, sizeof(research_state.selected_model_name) - 1);
    research_state.selected_model_name[sizeof(research_state.selected_model_name) - 1] = '\0';
}

void draw_research_tab(SDL_Renderer* renderer, AppState* state) {
    (void)state;
    
    int content_x = SIDEBAR_WIDTH;
    int content_y = SUBMENU_HEIGHT;
    int content_width = RENDER_WIDTH;  // Use RENDER_WIDTH to account for control panel
    int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
    
    // Initialize UI elements if needed
    if (!ui_initialized) {
        // Golden ratio division: 61.8% viewer, 38.2% controls
        float golden_ratio = 1.618f;
        int viewer_width = (int)(content_width / golden_ratio);
        int controls_width = content_width - viewer_width - PADDING;
        
        // Create viewer panel (left, golden ratio)
        // For RECTANGULAR: x,y is CENTER, not top-left
        float viewer_panel_width = (float)viewer_width - 20.0f;
        float viewer_panel_height = (float)content_height - 20.0f;
        panel_viewer = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            content_x + viewer_panel_width / 2.0f + 10.0f,
            content_y + viewer_panel_height / 2.0f + 10.0f,
            viewer_panel_width,
            viewer_panel_height,
            "CONTENT",
            get_global_font()
        );
        
        // Create controls panel (right)
        int controls_x = content_x + viewer_width + PADDING;
        float controls_panel_width = (float)controls_width - 20.0f;
        float controls_panel_height = (float)content_height - 20.0f;
        panel_controls = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            controls_x + controls_panel_width / 2.0f,
            content_y + controls_panel_height / 2.0f + 10.0f,
            controls_panel_width,
            controls_panel_height,
            "CONTROLS",
            get_global_font()
        );
        
        // Create file list panel within controls
        int list_y_start = content_y + 230;
        int list_height = content_height - 250;
        float files_panel_width = (float)controls_width - 40.0f;
        float files_panel_height = (float)list_height;
        panel_files = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            controls_x + files_panel_width / 2.0f + 10.0f,
            list_y_start + files_panel_height / 2.0f,
            files_panel_width,
            files_panel_height,
            "FILES",
            get_global_font()
        );
           
           // Visual hierarchy: Set different border widths and colors for nested panels
           if (panel_viewer) {
               panel_viewer->border_width = 3.0f;
               panel_viewer->base.color = crystalline_color_rgb(100, 120, 140);
           }
           if (panel_controls) {
               panel_controls->border_width = 3.0f;
               panel_controls->base.color = crystalline_color_rgb(100, 120, 140);
           }
           if (panel_files) {
               panel_files->border_width = 2.0f;
               panel_files->base.color = crystalline_color_rgb(80, 100, 120);
           }
        
        // Create file list
        // For RECTANGULAR: x,y is CENTER, not top-left
        float list_width = (float)controls_width - 60.0f;
        float list_display_height = 30.0f * 10;  // item_height * visible_items
        list_files = crystalline_list_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            controls_x + list_width / 2.0f + 20.0f,
            list_y_start + list_display_height / 2.0f + 50.0f,
            list_width,
            30.0f,  // item_height
            get_global_font()
        );
        
        // Create search input
        input_search = crystalline_input_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            controls_x + ((float)controls_width - 60.0f) / 2.0f + 30.0f,
            content_y + 60.0f + 20.0f,
            (float)controls_width - 60.0f,
            40.0f,  // Increased from 30 to 40 for better usability
            "Search files...",
            get_global_font()
        );
        
        // Create control buttons (3 in a row)
        float btn_y = content_y + 120.0f;
        float btn_spacing = (controls_width - 60.0f) / 3.0f;
        float btn_start_x = controls_x + 30.0f;
        
        btn_scan = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + btn_spacing * 0.5f, btn_y,
            32.0f, 0.0f, "SCAN", get_global_font()
        );
        
        btn_refresh = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + btn_spacing * 1.5f, btn_y,
            32.0f, 0.0f, "REFRESH", get_global_font()
        );
        
        btn_up = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + btn_spacing * 2.5f, btn_y,
            32.0f, 0.0f, "UP DIR", get_global_font()
        );
        
        // Create sort buttons (4 in a row)
        float sort_y = content_y + 175.0f;
        float sort_spacing = (controls_width - 60.0f) / 4.0f;
        
        btn_sort_name = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + sort_spacing * 0.5f, sort_y,
            40.0f, 0.0f, "NAME", get_global_font()
        );
        
        btn_sort_size = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + sort_spacing * 1.5f, sort_y,
            40.0f, 0.0f, "SIZE", get_global_font()
        );
        
        btn_sort_date = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + sort_spacing * 2.5f, sort_y,
            40.0f, 0.0f, "DATE", get_global_font()
        );
        
        btn_sort_type = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + sort_spacing * 3.5f, sort_y,
            40.0f, 0.0f, "TYPE", get_global_font()
        );
        
        // Initialize model selector
        research_state.model_selector = model_selector_create(
            controls_x + 20, content_y + 10, controls_width - 40, 30
        );
        model_selector_update_list(research_state.model_selector);
        model_selector_set_callback(research_state.model_selector, on_research_model_selected, state);
        
        // Initial scan
        scan_research_directory(research_state.current_directory);
        
        ui_initialized = true;
    }
    
    // Colors
    SDL_Color text_color = crystalline_color_from_frequency(432.0f);
    SDL_Color accent_color = crystalline_color_from_frequency(639.0f);
    SDL_Color content_color = crystalline_color_from_frequency(528.0f);
    
    // Draw panels
    crystalline_panel_render(panel_viewer, renderer);
    crystalline_panel_render(panel_controls, renderer);
    crystalline_panel_render(panel_files, renderer);
    
    // Update file list
    crystalline_list_clear(list_files);
    for (int i = 0; i < research_state.file_count; i++) {
        char display[256];
        snprintf(display, sizeof(display), "%.30s | %s | %s",
                research_state.files[i].filename,
                format_file_size(research_state.files[i].file_size),
                research_state.files[i].file_type);
        crystalline_list_add_item(list_files, display);
    }
    
    // Set selected item
    if (research_state.selected_file >= 0 && research_state.selected_file < research_state.file_count) {
        crystalline_list_set_selected(list_files, research_state.selected_file);
    }
    
    // Draw file list
    crystalline_list_render(list_files, renderer);
    
    // Draw search input
    crystalline_input_render(input_search, renderer);
    
    // Draw buttons
    crystalline_button_render(btn_scan, renderer);
    crystalline_button_render(btn_refresh, renderer);
    crystalline_button_render(btn_up, renderer);
    crystalline_button_render(btn_sort_name, renderer);
    crystalline_button_render(btn_sort_size, renderer);
    crystalline_button_render(btn_sort_date, renderer);
    crystalline_button_render(btn_sort_type, renderer);
    
    // Draw model selector
    if (research_state.model_selector) {
        model_selector_render(research_state.model_selector, renderer);
    }
    
    // Draw file viewer content
    if (research_state.selected_file >= 0 && research_state.selected_file < research_state.file_count) {
        int viewer_x = content_x + 20;
        int viewer_y = content_y + 60;
        int viewer_width __attribute__((unused)) = (int)(content_width / 1.618f) - 40;
        
        // File header
        ResearchFile* file = &research_state.files[research_state.selected_file];
        CrystallinePoint header_pos = crystalline_point_cartesian(viewer_x + 10.0f, viewer_y);
        crystalline_draw_text(renderer, file->filename, header_pos, text_color, NULL);
        
        // File info
        char info[256];
        snprintf(info, sizeof(info), "%s | %s | %s",
                file->file_type,
                format_file_size(file->file_size),
                format_time(file->modified_time));
        CrystallinePoint info_pos = crystalline_point_cartesian(viewer_x + 10.0f, viewer_y + 20.0f);
        crystalline_draw_text(renderer, info, info_pos, accent_color, NULL);
        
        // File content
        if (strlen(research_state.file_content) > 0) {
            int line_y = viewer_y + 50;
            int line_height = 14;
            int max_lines = (content_height - 110) / line_height;
            
            const char* ptr = research_state.file_content;
            int current_line = 0;
            int lines_drawn = 0;
            
            while (*ptr && lines_drawn < max_lines) {
                // Skip lines before scroll offset
                if (current_line < research_state.content_scroll) {
                    while (*ptr && *ptr != '\n') ptr++;
                    if (*ptr == '\n') ptr++;
                    current_line++;
                    continue;
                }
                
                // Draw line
                char line[256];
                int len = 0;
                while (*ptr && *ptr != '\n' && len < 200) {
                    line[len++] = *ptr++;
                }
                if (*ptr == '\n') ptr++;
                line[len] = '\0';
                
                CrystallinePoint line_pos = crystalline_point_cartesian(viewer_x + 10.0f, line_y);
                crystalline_draw_text(renderer, line, line_pos, content_color, NULL);
                
                line_y += line_height;
                current_line++;
                lines_drawn++;
            }
        }
    } else {
        // No file selected message
        int viewer_x = content_x + 20;
        int viewer_y = content_y + content_height / 2;
        CrystallinePoint msg_pos = crystalline_point_cartesian(viewer_x + 100.0f, viewer_y);
        crystalline_draw_text(renderer, "Select a file to view", msg_pos, text_color, NULL);
    }
    
       // Draw current directory and file count
       int controls_x = content_x + (int)(content_width / 1.618f) + PADDING;
       
       // Current directory path
       char dir_text[512];
       char truncated_dir[480];
       strncpy(truncated_dir, research_state.current_directory, sizeof(truncated_dir) - 1);
       truncated_dir[sizeof(truncated_dir) - 1] = '\0';
       snprintf(dir_text, sizeof(dir_text), "Directory: %s", truncated_dir);
       CrystallinePoint dir_pos = crystalline_point_cartesian(controls_x + 20.0f, content_y + 225.0f);
       crystalline_draw_text(renderer, dir_text, dir_pos, text_color, NULL);
       
       // File count
       char count_text[64];
       snprintf(count_text, sizeof(count_text), "Files: %d", research_state.file_count);
       CrystallinePoint count_pos = crystalline_point_cartesian(controls_x + 20.0f, content_y + content_height - 20.0f);
       crystalline_draw_text(renderer, count_text, count_pos, accent_color, NULL);
    
    // Render inputs through InputManager
    if (g_input_manager) {
        input_manager_render(g_input_manager, renderer, get_global_font(), TAB_RESEARCH);
    }
}

void handle_research_tab_mouse_down(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Pass BUTTONDOWN event to all interactive elements
    // Actions will be triggered on BUTTONUP
    if (research_state.model_selector) model_selector_handle_button_down(research_state.model_selector, x, y);
    if (list_files) crystalline_list_handle_mouse(list_files, &event);
    if (input_search) crystalline_input_handle_mouse(input_search, &event);
    if (btn_scan) crystalline_button_handle_mouse(btn_scan, &event);
    if (btn_refresh) crystalline_button_handle_mouse(btn_refresh, &event);
    if (btn_up) crystalline_button_handle_mouse(btn_up, &event);
    if (btn_sort_name) crystalline_button_handle_mouse(btn_sort_name, &event);
    if (btn_sort_size) crystalline_button_handle_mouse(btn_sort_size, &event);
    if (btn_sort_date) crystalline_button_handle_mouse(btn_sort_date, &event);
    if (btn_sort_type) crystalline_button_handle_mouse(btn_sort_type, &event);
}

bool handle_research_tab_event(AppState* state, SDL_Event* event) {
    if (!state || !event || !ui_initialized) return false;
    
    // Handle search input events
    if (crystalline_input_handle_keyboard(input_search, event)) {
        const char* query = crystalline_input_get_text(input_search);
        if (query) {
            strncpy(research_state.search_query, query, sizeof(research_state.search_query) - 1);
            research_state.search_query[sizeof(research_state.search_query) - 1] = '\0';
            scan_research_directory(research_state.current_directory);
        }
        return true;
    }
    
    return false;
}

void handle_research_tab_scroll(AppState* state, int scroll_y) {
    (void)state;
    
    if (!ui_initialized) return;
    
    if (research_state.selected_file >= 0) {
        // Scroll file content
        research_state.content_scroll -= scroll_y;
        if (research_state.content_scroll < 0) research_state.content_scroll = 0;
        
        // Count total lines
        int total_lines = 1;
        for (size_t i = 0; i < strlen(research_state.file_content); i++) {
            if (research_state.file_content[i] == '\n') total_lines++;
        }
        
        int max_lines = (WINDOW_HEIGHT - 150) / 14;
        int max_scroll = total_lines - max_lines;
        if (max_scroll < 0) max_scroll = 0;
        if (research_state.content_scroll > max_scroll) research_state.content_scroll = max_scroll;
    }
}

void handle_research_tab_text_input(AppState* state, const char* text) {
    (void)state;
    (void)text;
    // Handled by crystalline input
}

void handle_research_tab_keydown(AppState* state, SDL_Keycode key) {
    (void)state;
    (void)key;
    // Handled by crystalline input
}

/**
 * Handle research tab mouse motion for hover states
 */
void handle_research_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Update hover states for all interactive elements
    if (list_files) crystalline_list_handle_mouse(list_files, &event);
    if (input_search) crystalline_input_handle_mouse(input_search, &event);
    if (btn_scan) crystalline_button_handle_mouse(btn_scan, &event);
    if (btn_refresh) crystalline_button_handle_mouse(btn_refresh, &event);
    if (btn_up) crystalline_button_handle_mouse(btn_up, &event);
    if (btn_sort_name) crystalline_button_handle_mouse(btn_sort_name, &event);
    if (btn_sort_size) crystalline_button_handle_mouse(btn_sort_size, &event);
    if (btn_sort_date) crystalline_button_handle_mouse(btn_sort_date, &event);
    if (btn_sort_type) crystalline_button_handle_mouse(btn_sort_type, &event);
}

/**
 * Handle research tab mouse up for click callbacks
 */
void handle_research_tab_mouse_up(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check model selector
    if (research_state.model_selector && model_selector_handle_button_up(research_state.model_selector, x, y)) {
        return;
    }
    
    // Check file list
    if (crystalline_list_handle_mouse(list_files, &event)) {
        int selected = crystalline_list_get_selected(list_files);
        if (selected >= 0 && selected < research_state.file_count) {
            research_state.selected_file = selected;
            load_file_content(research_state.files[selected].full_path);
        }
        return;
    }
    
    // Check search input
    if (crystalline_input_handle_mouse(input_search, &event)) {
        return;
    }
    
    // Check buttons
    if (crystalline_button_handle_mouse(btn_scan, &event)) {
        scan_research_directory(research_state.current_directory);
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_refresh, &event)) {
        scan_research_directory(research_state.current_directory);
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_up, &event)) {
        char* last_slash = strrchr(research_state.current_directory, '/');
        if (last_slash && last_slash != research_state.current_directory) {
            *last_slash = '\0';
            scan_research_directory(research_state.current_directory);
        }
        return;
    }
    
    // Check sort buttons
    if (crystalline_button_handle_mouse(btn_sort_name, &event)) {
        if (research_state.sort_mode == SORT_NAME) {
            research_state.sort_ascending = !research_state.sort_ascending;
        } else {
            research_state.sort_mode = SORT_NAME;
            research_state.sort_ascending = true;
        }
        scan_research_directory(research_state.current_directory);
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_sort_size, &event)) {
        if (research_state.sort_mode == SORT_SIZE) {
            research_state.sort_ascending = !research_state.sort_ascending;
        } else {
            research_state.sort_mode = SORT_SIZE;
            research_state.sort_ascending = true;
        }
        scan_research_directory(research_state.current_directory);
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_sort_date, &event)) {
        if (research_state.sort_mode == SORT_DATE) {
            research_state.sort_ascending = !research_state.sort_ascending;
        } else {
            research_state.sort_mode = SORT_DATE;
            research_state.sort_ascending = true;
        }
        scan_research_directory(research_state.current_directory);
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_sort_type, &event)) {
        if (research_state.sort_mode == SORT_TYPE) {
            research_state.sort_ascending = !research_state.sort_ascending;
        } else {
            research_state.sort_mode = SORT_TYPE;
            research_state.sort_ascending = true;
        }
        scan_research_directory(research_state.current_directory);
        return;
    }
}

void cleanup_research_tab(void) {
    if (research_state.model_selector) {
        model_selector_destroy(research_state.model_selector);
        research_state.model_selector = NULL;
    }
    
    if (panel_viewer) crystalline_panel_destroy(panel_viewer);
    if (panel_controls) crystalline_panel_destroy(panel_controls);
    if (panel_files) crystalline_panel_destroy(panel_files);
    if (list_files) crystalline_list_destroy(list_files);
    if (input_search) crystalline_input_destroy(input_search);
    if (btn_scan) crystalline_button_destroy(btn_scan);
    if (btn_refresh) crystalline_button_destroy(btn_refresh);
    if (btn_up) crystalline_button_destroy(btn_up);
    if (btn_sort_name) crystalline_button_destroy(btn_sort_name);
    if (btn_sort_size) crystalline_button_destroy(btn_sort_size);
    if (btn_sort_date) crystalline_button_destroy(btn_sort_date);
    if (btn_sort_type) crystalline_button_destroy(btn_sort_type);
    
    panel_viewer = panel_controls = panel_files = NULL;
    list_files = NULL;
    input_search = NULL;
    btn_scan = btn_refresh = btn_up = NULL;
    btn_sort_name = btn_sort_size = btn_sort_date = btn_sort_type = NULL;
    
    ui_initialized = false;
}// Legacy function for compatibility
void handle_research_tab_click(AppState* state, int x, int y) {
    handle_research_tab_mouse_down(state, x, y);
}
