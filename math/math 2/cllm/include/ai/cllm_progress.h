#ifndef CLLM_PROGRESS_H
#define CLLM_PROGRESS_H

#include <stddef.h>
#include <time.h>
#include <stdint.h>

// Progress tracking structure
typedef struct {
    const char* phase_name;
    size_t current;
    size_t total;
    time_t start_time;
    time_t last_update;
    int update_interval_ms;  // Minimum milliseconds between updates
} CLLMProgress;

// Initialize progress tracker
void cllm_progress_init(CLLMProgress* progress, const char* phase_name, size_t total);

// Update progress (thread-safe)
void cllm_progress_update(CLLMProgress* progress, size_t current);

// Complete progress phase
void cllm_progress_complete(CLLMProgress* progress);

// Print progress bar with ETA
void cllm_progress_print(CLLMProgress* progress);

// Format time duration (seconds to human-readable)
void cllm_format_duration(char* buffer, size_t buffer_size, double seconds);

// Calculate ETA
double cllm_calculate_eta(CLLMProgress* progress);

#endif // CLLM_PROGRESS_H