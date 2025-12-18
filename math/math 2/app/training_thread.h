#ifndef TRAINING_THREAD_H
#define TRAINING_THREAD_H

#include "app_common.h"
#include <stdbool.h>

/**
 * Start training in separate thread
 * Returns 0 on success, -1 on error
 */
int start_training_thread(AppState* state);

/**
 * Stop training thread
 */
void stop_training_thread(AppState* state);

/**
 * Check if training thread is active
 */
bool is_training_thread_active(void);

/**
 * Get training state (thread-safe)
 */
void get_training_state(AppState* state, int* epoch, float* loss, bool* in_progress);

#endif // TRAINING_THREAD_H