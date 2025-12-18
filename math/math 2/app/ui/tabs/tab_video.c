/**
 * Video Tab - Crystalline UI Version
 * 
 * Features:
 * - Radial layout with 12-fold symmetry
 * - Circular Start/Stop button with pulse animation
 * - Status displayed in circular progress ring
 * - Information arranged using golden ratio
 * - Sacred geometry borders
 */

#include "../../app_common.h"
#include "../../ui_layout.h"
#include "../crystalline/elements.h"
#include "../crystalline/geometry.h"
#include "../crystalline/layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/color.h"
#include <stdio.h>
#include <string.h>

extern TTF_Font* get_global_font(void);

// Crystalline UI elements
static CrystallineButton* btn_record = NULL;
static CrystallinePanel* panel_status = NULL;
static CrystallinePanel* panel_requirements = NULL;
static CrystallineProgress* progress_recording = NULL;
static bool ui_initialized = false;

/**
 * Initialize video tab UI with crystalline elements
 */
void init_video_tab(void) {
    if (ui_initialized) return;
    
    // Calculate center of render area
    float center_x = RENDER_OFFSET_X + RENDER_WIDTH / 2.0f;
    float center_y = RENDER_OFFSET_Y + WINDOW_HEIGHT / 2.0f;
    
    // Create central record button (circular style)
    btn_record = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        center_x,
        center_y - 100.0f,
        80.0f,   // radius
        0.0f,    // unused for circular
        "START",
        get_global_font()
    );
    
    // Create status panel (circular style with Flower of Life border)
    panel_status = crystalline_panel_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        center_x,
        center_y - 250.0f,
        400.0f,  // width
        180.0f,  // height
        "RECORDING STATUS",
        get_global_font()
    );
    
    // Create requirements panel (circular style)
    panel_requirements = crystalline_panel_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        center_x,
        center_y + 150.0f,
        400.0f,  // width
        150.0f,  // height
        "REQUIREMENTS",
        get_global_font()
    );
    
    // Create circular progress indicator
    progress_recording = crystalline_progress_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        center_x,
        center_y - 100.0f,
        120.0f,  // radius (outer ring around button)
        0.0f     // unused for circular
    );
    
    ui_initialized = true;
}

/**
 * Draw video tab with crystalline UI
 */
void draw_video_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    // Initialize if needed
    if (!ui_initialized) {
        init_video_tab();
    }
    
    // Colors using crystalline color system
    SDL_Color text_color = crystalline_color_from_frequency(432.0f);  // Base frequency
    SDL_Color recording_color = crystalline_color_from_frequency(963.0f);  // High frequency (red)
    SDL_Color success_color = crystalline_color_from_frequency(528.0f);  // Love frequency (green)
    
    // Calculate center
    float center_x = RENDER_OFFSET_X + RENDER_WIDTH / 2.0f;
    float center_y __attribute__((unused)) = RENDER_OFFSET_Y + WINDOW_HEIGHT / 2.0f;
    
    // Draw title using golden ratio positioning
    CrystallinePoint title_pos = crystalline_point_cartesian(center_x, RENDER_OFFSET_Y + 40.0f);
    crystalline_draw_text_centered(renderer, "VIDEO RECORDING", title_pos, text_color, NULL);
    
    // Update button label based on recording state
    if (state->is_recording) {
        crystalline_button_set_label(btn_record, "STOP");
    } else {
        crystalline_button_set_label(btn_record, "START");
    }
    
    // Update progress indicator
    if (state->is_recording && state->recording_frames > 0) {
        // Calculate progress (arbitrary max of 3600 frames = 2 minutes at 30fps)
        float progress = (float)state->recording_frames / 3600.0f;
        if (progress > 1.0f) progress = 1.0f;
        crystalline_progress_set_value(progress_recording, progress);
    } else {
        crystalline_progress_set_value(progress_recording, 0.0f);
    }
    
    // Draw status panel
    crystalline_panel_render(panel_status, renderer);
    
    // Draw status information inside panel using radial layout
    float panel_center_x = panel_status->base.position.x;
    float panel_center_y = panel_status->base.position.y;
    float info_y = panel_center_y - 60.0f;
    
    if (state->is_recording) {
        // Recording status
        CrystallinePoint status_pos = crystalline_point_cartesian(panel_center_x, info_y);
        crystalline_draw_text_centered(renderer, "STATUS: RECORDING", status_pos, recording_color, NULL);
        info_y += 25.0f;
        
        // Blinking indicator using crystalline pulse
        if ((SDL_GetTicks() / 500) % 2 == 0) {
            CrystallinePoint indicator_center = crystalline_point_cartesian(panel_center_x - 40.0f, info_y);
            CrystallineDrawStyle indicator_style = crystalline_draw_style_filled(recording_color);
            crystalline_draw_circle(renderer, indicator_center, 6.0f, indicator_style);
        }
        
        CrystallinePoint rec_text_pos = crystalline_point_cartesian(panel_center_x, info_y);
        crystalline_draw_text_centered(renderer, "REC", rec_text_pos, recording_color, NULL);
        info_y += 30.0f;
        
        // Frame counter
        char frame_info[128];
        snprintf(frame_info, sizeof(frame_info), "Frames: %d", state->recording_frames);
        CrystallinePoint frame_pos = crystalline_point_cartesian(panel_center_x, info_y);
        crystalline_draw_text_centered(renderer, frame_info, frame_pos, text_color, NULL);
        info_y += 20.0f;
        
        // Duration
        int duration_seconds = state->recording_frames / 30;
        int minutes = duration_seconds / 60;
        int seconds = duration_seconds % 60;
        char duration_info[128];
        snprintf(duration_info, sizeof(duration_info), "Duration: %02d:%02d", minutes, seconds);
        CrystallinePoint duration_pos = crystalline_point_cartesian(panel_center_x, info_y);
        crystalline_draw_text_centered(renderer, duration_info, duration_pos, text_color, NULL);
        info_y += 25.0f;
        
        // Settings info arranged using 12-fold symmetry
        const char* settings[] = {
            "1280x720 @ 30 FPS",
            "H.264 Codec"
        };
        for (int i = 0; i < 2; i++) {
            CrystallinePoint setting_pos = crystalline_point_cartesian(panel_center_x, info_y);
            crystalline_draw_text_centered(renderer, settings[i], setting_pos, text_color, NULL);
            info_y += 18.0f;
        }
        
    } else {
        // Ready status
        CrystallinePoint status_pos = crystalline_point_cartesian(panel_center_x, info_y);
        crystalline_draw_text_centered(renderer, "STATUS: READY", status_pos, success_color, NULL);
        info_y += 30.0f;
        
        CrystallinePoint ready_pos = crystalline_point_cartesian(panel_center_x, info_y);
        crystalline_draw_text_centered(renderer, "Press START to begin", ready_pos, text_color, NULL);
        info_y += 30.0f;
        
        // Settings arranged using golden ratio spacing
        const char* settings[] = {
            "Resolution: 1280x720",
            "Frame Rate: 30 FPS",
            "Codec: H.264",
            "Format: MP4"
        };
        for (int i = 0; i < 4; i++) {
            CrystallinePoint setting_pos = crystalline_point_cartesian(panel_center_x, info_y);
            crystalline_draw_text_centered(renderer, settings[i], setting_pos, text_color, NULL);
            info_y += 18.0f;
        }
    }
    
    // Draw circular progress ring (around button)
    crystalline_progress_render(progress_recording, renderer);
    
    // Draw central record button with pulse animation
    crystalline_button_render(btn_record, renderer);
    
    // Draw requirements panel
    crystalline_panel_render(panel_requirements, renderer);
    
    // Draw requirements info
    float req_center_x = panel_requirements->base.position.x;
    float req_center_y = panel_requirements->base.position.y;
    float req_y = req_center_y - 40.0f;
    
    // Check ffmpeg
    static int ffmpeg_checked = 0;
    static bool ffmpeg_available = false;
    
    if (ffmpeg_checked == 0) {
        ffmpeg_available = (system("which ffmpeg > /dev/null 2>&1") == 0);
        ffmpeg_checked = 1;
    }
    
    if (ffmpeg_available) {
        CrystallinePoint ffmpeg_pos = crystalline_point_cartesian(req_center_x, req_y);
        crystalline_draw_text_centered(renderer, "[OK] FFmpeg: Installed", ffmpeg_pos, success_color, NULL);
    } else {
        CrystallinePoint ffmpeg_pos = crystalline_point_cartesian(req_center_x, req_y);
        crystalline_draw_text_centered(renderer, "[X] FFmpeg: Not Found", ffmpeg_pos, recording_color, NULL);
        req_y += 20.0f;
        CrystallinePoint install_pos = crystalline_point_cartesian(req_center_x, req_y);
        crystalline_draw_text_centered(renderer, "Install: sudo apt-get install ffmpeg", install_pos, text_color, NULL);
    }
    req_y += 30.0f;
    
    // Notes arranged using 12-fold symmetry
    const char* notes[] = {
        "Captures main visualization area",
        "Switch tabs while recording",
        "May impact performance slightly"
    };
    for (int i = 0; i < 3; i++) {
        CrystallinePoint note_pos = crystalline_point_cartesian(req_center_x, req_y);
        crystalline_draw_text_centered(renderer, notes[i], note_pos, text_color, NULL);
        req_y += 18.0f;
    }
}

/**
 * Handle video tab mouse click with crystalline UI
 */
/**
 * Handle video tab mouse down
 */
void handle_video_tab_mouse_down(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    // Create SDL event for button handling
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check button click
    crystalline_button_handle_mouse(btn_record, &event);
}

// Legacy function for compatibility
void handle_video_tab_click(AppState* state, int x, int y) {
    handle_video_tab_mouse_down(state, x, y);
}

/**
 * Handle video tab mouse motion for hover states
 */
void handle_video_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    // Create SDL event for hover handling
    SDL_Event event;
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Update button hover state
    crystalline_button_handle_mouse(btn_record, &event);
}

/**
 * Handle video tab mouse up for click callbacks
 */
void handle_video_tab_mouse_up(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    // Create SDL event for button release
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check button release and trigger callback
    if (crystalline_button_handle_mouse(btn_record, &event)) {
        // Button was clicked - toggle recording
        if (state->is_recording) {
            printf("Stopping recording...\n");
            extern void stop_recording(AppState* state);
            stop_recording(state);
        } else {
            printf("Starting recording...\n");
            extern void start_recording(AppState* state);
            start_recording(state);
        }
    }
}

/**
 * Cleanup video tab resources
 */
void cleanup_video_tab(void) {
    if (btn_record) {
        crystalline_button_destroy(btn_record);
        btn_record = NULL;
    }
    if (panel_status) {
        crystalline_panel_destroy(panel_status);
        panel_status = NULL;
    }
    if (panel_requirements) {
        crystalline_panel_destroy(panel_requirements);
        panel_requirements = NULL;
    }
    if (progress_recording) {
        crystalline_progress_destroy(progress_recording);
        progress_recording = NULL;
    }
    ui_initialized = false;
}