/**
 * CLLM Crawler CLI Tool
 * 
 * This is a PURE consumer of libcrawler.so
 * It has NO shared code with the GUI application.
 * Both CLI and GUI are independent implementations using the same library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "../include/crawler.h"

static CrawlerState* g_crawler_state = NULL;
static volatile int g_running = 1;

/**
 * Signal handler for graceful shutdown
 */
static void signal_handler(int signum) {
    printf("\n\nReceived signal %d, shutting down...\n", signum);
    g_running = 0;
}

/**
 * Callback for crawler events
 */
static void crawler_event_callback(const CrawlerEvent* event, void* user_data __attribute__((unused))) {
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
    
    printf("[%s] %s (Total pages: %d)\n", event_type, event->message, event->pages_crawled);
    fflush(stdout);
}

/**
 * Print usage information
 */
static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\nOptions:\n");
    printf("  --start-url URL      Starting URL for crawling (required)\n");
    printf("  --data-dir DIR       Directory for storing data (default: ./crawler_data)\n");
    printf("  --max-pages N        Maximum pages to crawl (0 = unlimited, default: 0)\n");
    printf("  --help               Show this help message\n");
    printf("\nExample:\n");
    printf("  %s --start-url https://example.com --max-pages 100\n", program_name);
}

int main(int argc, char* argv[]) {
    // Default configuration
    const char* start_url = NULL;
    const char* data_dir = "./crawler_data";
    int max_pages = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--start-url") == 0 && i + 1 < argc) {
            start_url = argv[++i];
        } else if (strcmp(argv[i], "--data-dir") == 0 && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (strcmp(argv[i], "--max-pages") == 0 && i + 1 < argc) {
            max_pages = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Validate required arguments
    if (!start_url) {
        fprintf(stderr, "Error: --start-url is required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("=== CLLM Crawler ===\n");
    printf("Start URL: %s\n", start_url);
    printf("Data directory: %s\n", data_dir);
    printf("Max pages: %s\n\n", max_pages == 0 ? "unlimited" : "");
    if (max_pages > 0) {
        printf("Max pages: %d\n\n", max_pages);
    }
    
    // Initialize crawler
    printf("Initializing crawler...\n");
    g_crawler_state = crawler_state_init(data_dir, start_url, max_pages);
    if (!g_crawler_state) {
        fprintf(stderr, "Error: Failed to initialize crawler\n");
        return 1;
    }
    
    // Set callback
    crawler_set_callback(g_crawler_state, crawler_event_callback, NULL);
    
    // Start crawler
    printf("Starting crawler...\n");
    if (crawler_start(g_crawler_state) != 0) {
        fprintf(stderr, "Error: Failed to start crawler\n");
        crawler_state_cleanup(g_crawler_state);
        return 1;
    }
    
    printf("Crawler started successfully!\n");
    printf("Press Ctrl+C to stop\n\n");
    
    // Main loop - print status periodically
    while (g_running) {
        sleep(10);
        
        CrawlerStatus status;
        crawler_get_status(g_crawler_state, &status);
        
        if (status.running) {
            printf("\n--- Status Update ---\n");
            printf("Pages crawled: %d\n", status.pages_crawled);
            printf("Pages preprocessed: %d\n", status.pages_preprocessed);
            printf("Pages tokenized: %d\n", status.pages_tokenized);
            printf("Pages trained: %d\n", status.pages_trained);
            if (status.last_error[0] != '\0') {
                printf("Last error: %s\n", status.last_error);
            }
            printf("--------------------\n\n");
        } else {
            printf("Crawler has stopped\n");
            break;
        }
    }
    
    // Stop crawler
    printf("\nStopping crawler...\n");
    crawler_stop(g_crawler_state);
    
    // Print final statistics
    CrawlerStatus final_status;
    crawler_get_status(g_crawler_state, &final_status);
    
    printf("\n=== Final Statistics ===\n");
    printf("Total pages crawled: %d\n", final_status.pages_crawled);
    printf("Total pages preprocessed: %d\n", final_status.pages_preprocessed);
    printf("Total pages tokenized: %d\n", final_status.pages_tokenized);
    printf("Total pages trained: %d\n", final_status.pages_trained);
    printf("=======================\n");
    
    // Cleanup
    crawler_state_cleanup(g_crawler_state);
    
    printf("\nCrawler shutdown complete\n");
    return 0;
}
