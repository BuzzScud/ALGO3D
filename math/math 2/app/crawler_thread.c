/**
 * Application Crawler Integration
 * 
 * This is a PURE consumer of libcrawler.so
 * It has NO shared code with the CLI tool.
 * Both CLI and GUI are independent implementations using the same library.
 */

#include "crawler_thread.h"
#include "crawler.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CrawlerState* g_crawler_state = NULL;
static pthread_mutex_t g_crawler_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Callback for crawler events - updates UI
 */
static void crawler_event_callback(const CrawlerEvent* event, void* user_data __attribute__((unused))) {
    // This callback is called from the crawler library
    // We can use it to update UI state or log events
    
    const char* event_type = "UNKNOWN";
    
    switch (event->type) {
        case CRAWLER_EVENT_PAGE_DOWNLOADED:
            event_type = "DOWNLOADED";
            break;
        case CRAWLER_EVENT_PAGE_PREPROCESSED:
            event_type = "PREPROCESSED";
            break;
        case CRAWLER_EVENT_PAGE_TOKENIZED:
            event_type = "TOKENIZED";
            break;
        case CRAWLER_EVENT_PAGE_TRAINED:
            event_type = "TRAINED";
            break;
        case CRAWLER_EVENT_ERROR:
            event_type = "ERROR";
            break;
        case CRAWLER_EVENT_STOPPED:
            event_type = "STOPPED";
            break;
    }
    
    printf("[CRAWLER %s] %s (Total: %d)\n", event_type, event->message, event->pages_crawled);
    fflush(stdout);
}

int start_crawler_thread(AppState* state, const char* start_url, ExtractionMode extraction_mode, const char* model_name) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    // Check if already running
    if (g_crawler_state != NULL) {
        CrawlerStatus status;
        crawler_get_status(g_crawler_state, &status);
        if (status.running) {
            printf("Crawler is already running\n");
            pthread_mutex_unlock(&g_crawler_mutex);
            return -1;
        }
        // Cleanup old state
        crawler_state_cleanup(g_crawler_state);
        g_crawler_state = NULL;
    }
    
    printf("\n=== Starting Crawler ===\n");
    if (start_url && start_url[0] != '\0') {
        printf("Start URL: %s\n", start_url);
    } else {
        printf("Using URLs from database (no start URL)\n");
    }
    printf("Data directory: %s\n", state->crawler_data_dir);
    
    // Initialize crawler (start_url can be NULL)
    g_crawler_state = crawler_state_init(state->crawler_data_dir, start_url, 0);
    if (!g_crawler_state) {
        fprintf(stderr, "ERROR: Failed to initialize crawler\n");
        pthread_mutex_unlock(&g_crawler_mutex);
        return -1;
    }
    
    // Set extraction mode
    crawler_set_extraction_mode(g_crawler_state, extraction_mode);
    
    // Set model name for training
    if (model_name && model_name[0] != '\0') {
        crawler_set_model_name(g_crawler_state, model_name);
        printf("✓ Crawler will use model: %s\n", model_name);
    } else {
        printf("⚠ No model specified, will use first available model\n");
    }
    
    // URL manager integration disabled for now
    // TODO: Re-enable when crawler tab provides URL manager
    printf("Note: Using file-based queue for URL management\n");
    
    // Set callback for events
    crawler_set_callback(g_crawler_state, crawler_event_callback, state);
    
    // Start crawler
    if (crawler_start(g_crawler_state) != 0) {
        fprintf(stderr, "ERROR: Failed to start crawler\n");
        crawler_state_cleanup(g_crawler_state);
        g_crawler_state = NULL;
        pthread_mutex_unlock(&g_crawler_mutex);
        return -1;
    }
    
    printf("Crawler started successfully!\n");
    printf("=======================\n\n");
    
    pthread_mutex_unlock(&g_crawler_mutex);
    return 0;
}

void stop_crawler_thread(void) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    if (g_crawler_state) {
        printf("\n=== Stopping Crawler ===\n");
        crawler_stop(g_crawler_state);
        
        // Print final stats
        CrawlerStatus status;
        crawler_get_status(g_crawler_state, &status);
        printf("Final stats:\n");
        printf("  Pages crawled: %d\n", status.pages_crawled);
        printf("  Pages preprocessed: %d\n", status.pages_preprocessed);
        printf("  Pages tokenized: %d\n", status.pages_tokenized);
        printf("  Pages trained: %d\n", status.pages_trained);
        printf("=======================\n\n");
        
        crawler_state_cleanup(g_crawler_state);
        g_crawler_state = NULL;
    }
    
    pthread_mutex_unlock(&g_crawler_mutex);
}

int is_crawler_running(void) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    int running = 0;
    if (g_crawler_state) {
        CrawlerStatus status;
        crawler_get_status(g_crawler_state, &status);
        running = status.running;
    }
    
    pthread_mutex_unlock(&g_crawler_mutex);
    return running;
}

void get_crawler_status(int* pages_crawled, int* queue_size) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    if (g_crawler_state) {
        CrawlerStatus status;
        crawler_get_status(g_crawler_state, &status);
        
        if (pages_crawled) *pages_crawled = status.pages_crawled;
        if (queue_size) *queue_size = status.pages_tokenized;
    } else {
        if (pages_crawled) *pages_crawled = 0;
        if (queue_size) *queue_size = 0;
    }
    
    pthread_mutex_unlock(&g_crawler_mutex);
}