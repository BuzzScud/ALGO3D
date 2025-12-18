// app/io.c - Input/output and recording implementation
// CORRECTED to work with actual prime_math library

#define _POSIX_C_SOURCE 200809L

#include "app_common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

void start_recording(AppState* state) {
    if (state->is_recording) return;
    
    printf("\n=== VIDEO RECORDING ===\n");
    
    // Generate filename with timestamp
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char default_name[256];
    snprintf(default_name, sizeof(default_name), 
             "hyper_prime_%04d%02d%02d_%02d%02d%02d.mp4",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    // Get save path from user
    char* videos_dir = getenv("HOME");
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/Videos/%s", videos_dir, default_name);
    
    // Use the file dialog
    char* path = get_save_path_with_extension("Save Video As", full_path, ".mp4");
    if (!path || strlen(path) == 0) {
        printf("Recording cancelled\n");
        return;
    }
    
    strcpy(state->video_path, path);
    printf("Recording to: %s\n", state->video_path);
    printf("Video resolution: %dx%d (720p HD)\n", VIDEO_WIDTH, VIDEO_HEIGHT);
    
    // Prepare ffmpeg command for H.264 encoding
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
            "ffmpeg -y -f rawvideo -pix_fmt rgb24 -s %dx%d -r 30 -i - "
            "-c:v libx264 -preset ultrafast -crf 18 -pix_fmt yuv420p '%s' 2>/dev/null",
            VIDEO_WIDTH, VIDEO_HEIGHT, state->video_path);
    
    state->video_pipe = popen(cmd, "w");
    if (state->video_pipe) {
        state->frame_buffer = (uint8_t*)malloc(VIDEO_WIDTH * VIDEO_HEIGHT * 3);
        state->is_recording = true;
        state->recording_frames = 0;
        printf("✓ Recording started (720p HD format)\n");
        printf("Press 'R' again to stop recording\n\n");
    } else {
        printf("Failed to start recording (ffmpeg may not be installed)\n");
    }
}



void record_frame(AppState* state) {
    if (!state->is_recording || !state->video_pipe) return;
    
    // Check if recording renderer is available
    if (!state->recording_renderer || !state->recording_surface) {
        printf("\nWARNING: Recording system not available\n");
        printf("Stopping recording.\n\n");
        stop_recording(state);
        return;
    }
    
    // Render the current frame to the software surface using persistent renderer
    SDL_SetRenderDrawColor(state->recording_renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->recording_renderer);
    
    // Draw content based on current tab
    if (state->current_tab == TAB_CALCULATOR) {
        // Calculator tab with mini-map
        draw_calculator_with_minimap(state->recording_renderer, state);
    } else {
        // Visualization tabs
        draw_visualization(state->recording_renderer, state);
    }
    
    // Draw UI elements (tabs and control panel)
    draw_tabs(state->recording_renderer, state);
    draw_control_panel(state->recording_renderer, state);
    
    // Present the software renderer (commits to surface)
    SDL_RenderPresent(state->recording_renderer);
    
    // Lock surface and copy pixels
    if (SDL_MUSTLOCK(state->recording_surface)) {
        SDL_LockSurface(state->recording_surface);
    }
    
    // Convert ARGB8888 to RGB24 for ffmpeg
    uint32_t* src_pixels = (uint32_t*)state->recording_surface->pixels;
    uint8_t* dst_pixels = state->frame_buffer;
    
    for (int y = 0; y < VIDEO_HEIGHT; y++) {
        for (int x = 0; x < VIDEO_WIDTH; x++) {
            uint32_t pixel = src_pixels[y * VIDEO_WIDTH + x];
            int dst_idx = (y * VIDEO_WIDTH + x) * 3;
            
            // Extract RGB from ARGB (0xAARRGGBB)
            dst_pixels[dst_idx + 0] = (pixel >> 16) & 0xFF; // R
            dst_pixels[dst_idx + 1] = (pixel >> 8) & 0xFF;  // G
            dst_pixels[dst_idx + 2] = pixel & 0xFF;         // B
        }
    }
    
    if (SDL_MUSTLOCK(state->recording_surface)) {
        SDL_UnlockSurface(state->recording_surface);
    }
    
    // Write frame to ffmpeg
    size_t written = fwrite(state->frame_buffer, 1, VIDEO_WIDTH * VIDEO_HEIGHT * 3, state->video_pipe);
    if (written != VIDEO_WIDTH * VIDEO_HEIGHT * 3) {
        printf("WARNING: Incomplete frame write\n");
        return;
    }
    
    state->recording_frames++;
    
    // Progress indicator every 30 frames (1 second)
    if (state->recording_frames % 30 == 0) {
        printf("Recording: %d frames (%.1f seconds)\r", 
               state->recording_frames, state->recording_frames / 30.0);
        fflush(stdout);
    }
}

void stop_recording(AppState* state) {
    if (!state->is_recording) return;
    
    if (state->video_pipe) {
        fflush(state->video_pipe);
        pclose(state->video_pipe);
        state->video_pipe = NULL;
    }
    
    if (state->frame_buffer) {
        free(state->frame_buffer);
        state->frame_buffer = NULL;
    }
    
    state->is_recording = false;
    printf("\n=== RECORDING STOPPED ===\n");
    printf("Saved: %s\n", state->video_path);
    printf("Total frames: %d\n", state->recording_frames);
    printf("Duration: ~%.1f seconds\n\n", state->recording_frames / 30.0);
}

void save_snapshot(AppState* state) {
    printf("\n=== SAVE SNAPSHOT ===\n");
    
    // Generate filename with timestamp
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char default_name[256];
    snprintf(default_name, sizeof(default_name), 
             "hyper_prime_%04d%02d%02d_%02d%02d%02d.png",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    char* path = get_save_path_with_extension("Save Image As", default_name, ".png");
    
    // Create surface from renderer
    SDL_Surface* surface = SDL_CreateRGBSurface(0, RENDER_WIDTH, WINDOW_HEIGHT, 32,
                                                0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    
    if (!surface) {
        printf("Failed to create surface\n");
        return;
    }
    
    // Read pixels from renderer
    SDL_Rect render_area = {0, 0, RENDER_WIDTH, WINDOW_HEIGHT};
    if (SDL_RenderReadPixels(state->renderer, &render_area, SDL_PIXELFORMAT_ARGB8888, 
                             surface->pixels, surface->pitch) != 0) {
        printf("Failed to read pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    
    // Save as BMP first
    char bmp_path[512];
    strcpy(bmp_path, path);
    char* ext = strstr(bmp_path, ".png");
    if (ext) {
        strcpy(ext, ".bmp");
    }
    
    if (SDL_SaveBMP(surface, bmp_path) == 0) {
        // Try to convert to PNG using ImageMagick
        char cmd[2048];  // Increased buffer size
        snprintf(cmd, sizeof(cmd), "convert '%s' '%s' && rm '%s' 2>/dev/null", 
                bmp_path, path, bmp_path);
        int ret = system(cmd);
        
        if (ret == 0) {
            printf("✓ Saved: %s\n\n", path);
        } else {
            // Conversion failed, keep BMP
            printf("✓ Saved: %s (BMP format)\n", bmp_path);
            printf("Note: Install ImageMagick for PNG support\n\n");
        }
    } else {
        printf("Failed to save image: %s\n", SDL_GetError());
    }
    
    SDL_FreeSurface(surface);
}

char* get_save_path_with_extension(const char* title, const char* default_name, const char* extension) {
    static char path[512];
    
    // Try using zenity for file dialog
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
            "zenity --file-selection --save --title='%s' --filename='%s' 2>/dev/null",
            title, default_name);
    
    FILE* fp = popen(cmd, "r");
    if (fp) {
        if (fgets(path, sizeof(path), fp)) {
            pclose(fp);
            path[strcspn(path, "\n")] = 0;  // Remove newline
            ensure_file_extension(path, extension);
            return path;
        }
        pclose(fp);
    }
    
    // Fallback: use default name in current directory
    strcpy(path, default_name);
    ensure_file_extension(path, extension);
    return path;
}

void ensure_file_extension(char* path, const char* extension) {
    // Check if path already has the extension
    size_t path_len = strlen(path);
    size_t ext_len = strlen(extension);
    
    if (path_len < ext_len || strcmp(path + path_len - ext_len, extension) != 0) {
        // Add extension
        strcat(path, extension);
    }
}