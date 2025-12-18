#ifndef TIME_FORMAT_H
#define TIME_FORMAT_H

#include <time.h>
#include <stdio.h>

/**
 * Format elapsed time in human-readable format
 * Examples: "5s", "2m 35s", "1h 15m"
 */
static inline void format_time_elapsed(double seconds, char* buffer, size_t size) {
    if (seconds < 0) {
        snprintf(buffer, size, "Elapsed: --");
        return;
    }
    
    int hours = (int)(seconds / 3600);
    int minutes = (int)((seconds - hours * 3600) / 60);
    int secs = (int)(seconds - hours * 3600 - minutes * 60);
    
    if (hours > 0) {
        snprintf(buffer, size, "Elapsed: %dh %dm %ds", hours, minutes, secs);
    } else if (minutes > 0) {
        snprintf(buffer, size, "Elapsed: %dm %ds", minutes, secs);
    } else {
        snprintf(buffer, size, "Elapsed: %ds", secs);
    }
}

/**
 * Format remaining time estimate
 * Examples: "~5s", "~2m 35s", "~1h 15m"
 */
static inline void format_time_remaining(double seconds, char* buffer, size_t size) {
    if (seconds < 0 || seconds > 86400) {  // > 24 hours or invalid
        snprintf(buffer, size, "Remaining: calculating...");
        return;
    }
    
    int hours = (int)(seconds / 3600);
    int minutes = (int)((seconds - hours * 3600) / 60);
    int secs = (int)(seconds - hours * 3600 - minutes * 60);
    
    if (hours > 0) {
        snprintf(buffer, size, "Remaining: ~%dh %dm", hours, minutes);
    } else if (minutes > 0) {
        snprintf(buffer, size, "Remaining: ~%dm %ds", minutes, secs);
    } else {
        snprintf(buffer, size, "Remaining: ~%ds", secs);
    }
}

/**
 * Format ETA (estimated time of arrival)
 * Example: "ETA: 14:23:45"
 */
static inline void format_eta(double seconds_remaining, char* buffer, size_t size) {
    if (seconds_remaining < 0 || seconds_remaining > 86400) {
        snprintf(buffer, size, "ETA: calculating...");
        return;
    }
    
    time_t now = time(NULL);
    time_t eta = now + (time_t)seconds_remaining;
    struct tm* eta_tm = localtime(&eta);
    
    snprintf(buffer, size, "ETA: %02d:%02d:%02d", 
             eta_tm->tm_hour, eta_tm->tm_min, eta_tm->tm_sec);
}

#endif // TIME_FORMAT_H