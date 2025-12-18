#ifndef CRAWLER_H
#define CRAWLER_H

#include <pthread.h>
#include <stdbool.h>
#include "../src/crawler/content_filter.h"

// Forward declarations for internal component states
typedef struct CrawlerStateInternal CrawlerStateInternal;
typedef struct PreprocessorState PreprocessorState;
typedef struct TokenizerState TokenizerState;
typedef struct ContinuousTrainingState ContinuousTrainingState;

// High-level API state (opaque to users)
typedef struct CrawlerState CrawlerState;

// ============================================================================
// STATUS AND EVENTS
// ============================================================================

/**
 * Crawler status structure (thread-safe access via crawler_get_status)
 */
typedef struct {
    int running;                    // 1 if crawler is running, 0 otherwise
    int pages_crawled;              // Number of pages downloaded
    int pages_preprocessed;         // Number of pages preprocessed
    int pages_tokenized;            // Number of pages tokenized
    int pages_trained;              // Number of pages trained on
    char current_url[512];          // Current URL being processed
    char last_error[512];           // Last error message (if any)
} CrawlerStatus;

/**
 * Event types for callbacks
 */
typedef enum {
    CRAWLER_EVENT_PAGE_DOWNLOADED,
    CRAWLER_EVENT_PAGE_PREPROCESSED,
    CRAWLER_EVENT_PAGE_TOKENIZED,
    CRAWLER_EVENT_PAGE_TRAINED,
    CRAWLER_EVENT_ERROR,
    CRAWLER_EVENT_STOPPED
} CrawlerEventType;

/**
 * Event structure passed to callbacks
 */
typedef struct {
    CrawlerEventType type;
    char message[512];
    int pages_crawled;
} CrawlerEvent;

/**
 * Callback function type
 */
typedef void (*CrawlerCallback)(const CrawlerEvent* event, void* user_data);

// ============================================================================
// MAIN CRAWLER API
// ============================================================================

/**
 * Initialize crawler state
 * @param data_dir Directory for storing crawler data
 * @param start_url Starting URL for crawling
 /**
 * Initialize crawler with custom thread count
 * @param data_dir Directory for storing crawler data
 * @param start_url Starting URL for crawling
 * @param max_pages Maximum pages to crawl (0 = unlimited)
 * @param num_threads Number of threads per stage (0 = auto-detect cores-1)
 * @return Crawler state or NULL on error
 */
CrawlerState* crawler_state_init(const char* data_dir, const char* start_url, 
                                 int max_pages, int num_threads);

/**
 * Crawler rate limiting configuration
 */
typedef struct {
    int min_delay_seconds;      // Minimum delay between requests (default: 2)
    int max_delay_seconds;      // Maximum delay for randomization (default: 5)
    int delay_minutes;          // Optional: delay in minutes (0 = use seconds)
    bool use_random_delay;      // Randomize delays (default: true)
    float requests_per_minute;  // Alternative rate limit (0 = use delay_seconds)
} CrawlerRateConfig;

/**
 * Set crawler rate limiting
 * @param state Crawler state
 * @param min_seconds Minimum delay in seconds
 * @param max_seconds Maximum delay in seconds (for randomization)
 */
void crawler_set_rate_limit(CrawlerState* state, int min_seconds, int max_seconds);

/**
 * Set crawler rate limiting by requests per minute
 * @param state Crawler state
 * @param rpm Requests per minute (e.g., 10.0 = 1 request every 6 seconds)
 */
void crawler_set_rate_limit_rpm(CrawlerState* state, float rpm);

/**
 * Set crawler rate limiting in minutes
 * @param state Crawler state
 * @param minutes Delay in minutes between requests
 */
void crawler_set_rate_limit_minutes(CrawlerState* state, int minutes);

/**
 * Start the crawler (spawns all threads internally)
 * @param state Crawler state
 * @return 0 on success, -1 on error
 */
int crawler_start(CrawlerState* state);

/**
 * Pause the crawler (can be resumed)
 * @param state Crawler state
 */
void crawler_pause(CrawlerState* state);

/**
 * Resume the crawler from paused state
 * @param state Crawler state
 */
void crawler_resume(CrawlerState* state);

/**
 * Stop the crawler (stops all threads)
 * @param state Crawler state
 */
void crawler_stop(CrawlerState* state);

/**
 * Get current crawler status (thread-safe)
 * @param state Crawler state
 * @param status Output status structure
 */
void crawler_get_status(CrawlerState* state, CrawlerStatus* status);

/**
 * Set callback for crawler events
 * @param state Crawler state
 * @param callback Callback function
 * @param user_data User data passed to callback
 */
void crawler_set_callback(CrawlerState* state, CrawlerCallback callback, void* user_data);

void crawler_set_extraction_mode(CrawlerState* state, ExtractionMode mode);

/**
 * Set the model name to use for training
 */
void crawler_set_model_name(CrawlerState* state, const char* model_name);

/**
 * Set URL manager for database integration
 * @param state Crawler state
 * @param url_manager CrawlerURLManager pointer (void* to avoid circular dependency)
 */
void crawler_set_url_manager(CrawlerState* state, void* url_manager);

/**
 * Cleanup crawler state
 * @param state Crawler state
 */
void crawler_state_cleanup(CrawlerState* state);

// ============================================================================
// INTERNAL FUNCTIONS (for library use only - not for application use)
// ============================================================================

// Internal component initialization
CrawlerStateInternal* crawler_internal_init(const char* data_dir, const char* start_url, int max_pages);
void crawler_internal_set_url_manager(CrawlerStateInternal* state, void* url_manager);
void crawler_internal_cleanup(CrawlerStateInternal* state);
void* crawler_thread_func(void* arg);

PreprocessorState* preprocessor_init(const char* data_dir);
void preprocessor_cleanup(PreprocessorState* state);
void* preprocessor_thread_func(void* arg);

TokenizerState* tokenizer_init(const char* data_dir);
void tokenizer_cleanup(TokenizerState* state);
void* tokenizer_thread_func(void* arg);

ContinuousTrainingState* continuous_training_init(const char* data_dir, const char* model_path, 
                                                   void* model, int num_threads, void* app_state);
int continuous_training_start(ContinuousTrainingState* state, pthread_t* threads);
void continuous_training_stop(ContinuousTrainingState* state, pthread_t* threads);
void continuous_training_cleanup(ContinuousTrainingState* state);

#endif // CRAWLER_H