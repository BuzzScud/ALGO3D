// preprocessor.h - HTML Preprocessor Interface
#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "content_filter.h"
#include <pthread.h>

/**
 * Preprocessor state structure
 */
struct PreprocessorState {
    char data_dir[1024];
    int running;
    int files_processed;
    ExtractionMode extraction_mode;
    pthread_mutex_t lock;
};

/**
 * Initialize preprocessor
 */
PreprocessorState* preprocessor_init(const char* data_dir);

/**
 * Cleanup preprocessor
 */
void preprocessor_cleanup(PreprocessorState* state);

/**
 * Preprocessor thread function
 */
void* preprocessor_thread_func(void* arg);

/**
 * Set extraction mode for content filtering
 */
void preprocessor_set_extraction_mode(PreprocessorState* state, ExtractionMode mode);

/**
 * Get current extraction mode
 */
ExtractionMode preprocessor_get_extraction_mode(PreprocessorState* state);

#endif // PREPROCESSOR_H