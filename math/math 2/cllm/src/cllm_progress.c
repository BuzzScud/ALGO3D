#include "cllm_progress.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

// Get current time in milliseconds
static int64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// Initialize progress tracker
void cllm_progress_init(CLLMProgress* progress, const char* phase_name, size_t total) {
    if (!progress) return;
    
    progress->phase_name = phase_name;
    progress->current = 0;
    progress->total = total;
    progress->start_time = time(NULL);
    progress->last_update = 0;
    progress->update_interval_ms = 500;  // Update every 500ms to reduce flicker
    
    // Print initial header (no box, simpler format)
    printf("\n=== %s ===\n", phase_name);
    fflush(stdout);
}

// Format time duration
void cllm_format_duration(char* buffer, size_t buffer_size, double seconds) {
    if (seconds < 60) {
        snprintf(buffer, buffer_size, "%.0fs", seconds);
    } else if (seconds < 3600) {
        int mins = (int)(seconds / 60);
        int secs = (int)seconds % 60;
        snprintf(buffer, buffer_size, "%dm %ds", mins, secs);
    } else {
        int hours = (int)(seconds / 3600);
        int mins = (int)(seconds / 60) % 60;
        snprintf(buffer, buffer_size, "%dh %dm", hours, mins);
    }
}

// Calculate ETA
double cllm_calculate_eta(CLLMProgress* progress) {
    if (!progress || progress->current == 0) return 0.0;
    
    time_t now = time(NULL);
    double elapsed = difftime(now, progress->start_time);
    
    if (elapsed < 0.1) return 0.0;  // Not enough data yet
    
    double rate = (double)progress->current / elapsed;
    if (rate < 0.001) return 0.0;
    
    size_t remaining = progress->total - progress->current;
    return remaining / rate;
}

// Print progress bar with ETA
void cllm_progress_print(CLLMProgress* progress) {
    if (!progress) return;
    
    // Throttle updates
    int64_t now_ms = get_time_ms();
    if (now_ms - progress->last_update < progress->update_interval_ms) {
        return;
    }
    progress->last_update = now_ms;
    
    // Calculate progress percentage
    double percent = progress->total > 0 ? 
                     (100.0 * progress->current) / progress->total : 0.0;
    
    // Calculate elapsed time
    time_t now = time(NULL);
    double elapsed = difftime(now, progress->start_time);
    
    // Calculate ETA
    double eta_seconds = cllm_calculate_eta(progress);
    
    // Calculate speed
    double speed = elapsed > 0 ? progress->current / elapsed : 0.0;
    
    // Format times
    char elapsed_str[32];
    char eta_str[32];
    cllm_format_duration(elapsed_str, sizeof(elapsed_str), elapsed);
    
    if (eta_seconds > 0) {
        cllm_format_duration(eta_str, sizeof(eta_str), eta_seconds);
    } else {
        snprintf(eta_str, sizeof(eta_str), "calculating...");
    }
    
    // Create progress bar (50 characters wide)
    char bar[51];
    int filled = (int)(50.0 * percent / 100.0);
    for (int i = 0; i < 50; i++) {
        if (i < filled) {
            bar[i] = '=';
        } else if (i == filled && percent > 0 && percent < 100) {
            bar[i] = '>';
        } else {
            bar[i] = ' ';
        }
    }
    bar[50] = '\0';
    
    // Format speed
    char speed_str[32];
    if (speed >= 1000) {
        snprintf(speed_str, sizeof(speed_str), "%.1fk items/s", speed / 1000.0);
    } else if (speed >= 1) {
        snprintf(speed_str, sizeof(speed_str), "%.0f items/s", speed);
    } else {
        snprintf(speed_str, sizeof(speed_str), "%.2f items/s", speed);
    }
    
    // Simple single-line progress (no cursor movement to avoid conflicts)
    printf("\r[%s] %5.1f%% | %zu/%zu | Elapsed: %s | ETA: %s | Speed: %s   ", 
           bar, percent, progress->current, progress->total,
           elapsed_str, eta_str, speed_str);
    fflush(stdout);
}

// Update progress
void cllm_progress_update(CLLMProgress* progress, size_t current) {
    if (!progress) return;
    
    progress->current = current;
    cllm_progress_print(progress);
}

// Complete progress phase
void cllm_progress_complete(CLLMProgress* progress) {
    if (!progress) return;
    
    progress->current = progress->total;
    
    // Calculate final elapsed time
    time_t now = time(NULL);
    double elapsed = difftime(now, progress->start_time);
    
    // Format elapsed time
    char elapsed_str[32];
    cllm_format_duration(elapsed_str, sizeof(elapsed_str), elapsed);
    
    // Calculate final speed
    double speed = elapsed > 0 ? progress->total / elapsed : 0.0;
    char speed_str[32];
    if (speed >= 1000) {
        snprintf(speed_str, sizeof(speed_str), "%.1fk items/s", speed / 1000.0);
    } else if (speed >= 1) {
        snprintf(speed_str, sizeof(speed_str), "%.0f items/s", speed);
    } else {
        snprintf(speed_str, sizeof(speed_str), "%.2f items/s", speed);
    }
    
    // Print completion message (simple, no cursor movement)
    printf("\r[==================================================] 100.0%% | %zu/%zu | Completed in %s | Avg: %s   \n", 
           progress->total, progress->total, elapsed_str, speed_str);
    fflush(stdout);
}