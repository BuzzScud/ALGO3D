#ifndef CRAWLER_THREAD_H
#define CRAWLER_THREAD_H

#include "app_common.h"
#include "../src/crawler/content_filter.h"

/**
 * Start the crawler thread
 * Returns 0 on success, -1 on error
 */
int start_crawler_thread(AppState* state, const char* start_url, ExtractionMode extraction_mode, const char* model_name);

/**
 * Stop the crawler thread
 */
void stop_crawler_thread(void);

/**
 * Check if crawler is running
 */
int is_crawler_running(void);

/**
 * Get crawler status information
 */
void get_crawler_status(int* pages_crawled, int* queue_size);

#endif // CRAWLER_THREAD_H