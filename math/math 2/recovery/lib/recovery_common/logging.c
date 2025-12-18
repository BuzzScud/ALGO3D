#include "recovery_common/recovery_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

/*
 * Logging Module
 * 
 * Provides logging functionality with multiple levels:
 * - DEBUG: Detailed debugging information
 * - INFO: General informational messages
 * - WARN: Warning messages
 * - ERROR: Error messages
 * 
 * Supports both file and console logging.
 */

// Global logging state
static FILE* g_log_file = NULL;
static LogLevel g_log_level = LOG_INFO;
static bool g_console_logging = true;

// ============================================================================
// LOGGING INITIALIZATION
// ============================================================================

void init_logging(const char* log_file, LogLevel level) {
    g_log_level = level;
    
    // Close existing log file if open
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
    
    // Open new log file if specified
    if (log_file) {
        g_log_file = fopen(log_file, "a");
        if (!g_log_file) {
            fprintf(stderr, "Warning: Could not open log file: %s\n", log_file);
        }
    }
}

// ============================================================================
// LOGGING FUNCTIONS
// ============================================================================

static const char* get_level_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        default:        return "?????";
    }
}

static const char* get_level_color(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "\033[36m"; // Cyan
        case LOG_INFO:  return "\033[32m"; // Green
        case LOG_WARN:  return "\033[33m"; // Yellow
        case LOG_ERROR: return "\033[31m"; // Red
        default:        return "\033[0m";  // Reset
    }
}

void log_message(LogLevel level, const char* format, ...) {
    // Check if this level should be logged
    if (level < g_log_level) {
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Format the message
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Log to file
    if (g_log_file) {
        fprintf(g_log_file, "[%s] [%s] %s\n",
                time_buffer, get_level_string(level), message);
        fflush(g_log_file);
    }
    
    // Log to console
    if (g_console_logging) {
        fprintf(stderr, "%s[%s] [%s]%s %s\n",
                get_level_color(level), time_buffer, get_level_string(level),
                "\033[0m", message);
    }
}

// ============================================================================
// LOGGING CLEANUP
// ============================================================================

void close_logging() {
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}
