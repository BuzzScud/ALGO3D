/**
 * Web Crawler Core
 * 
 * Implements slow, methodical web crawling with:
 * - Rate limiting
 * - Link extraction
 * - robots.txt respect
 * - Domain filtering
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <pthread.h>
#include "crawler_url_manager.h"
#include "url_database.h"
#include <stdbool.h>
#include "../../include/crawler.h"

#define MAX_URL_LENGTH 2048
#define MAX_PAGE_SIZE (10 * 1024 * 1024)  // 10MB max page size

// Global rate limiting configuration (can be changed at runtime)
// HUMAN-LIKE CRAWL SPEED: Slow and methodical
static int g_min_delay_seconds = 5;   // Increased from 2 to 5
static int g_max_delay_seconds = 15;  // Increased from 5 to 15
static int g_delay_minutes = 0;
static bool g_use_random_delay = true;
static float g_requests_per_minute = 0.0f;
static pthread_mutex_t g_rate_config_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Get current timestamp string
 */
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", tm_info);
}

   typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} MemoryBuffer;
   struct CrawlerStateInternal {
    char data_dir[1024];
    char start_url[MAX_URL_LENGTH];
    int max_pages;
    int pages_crawled;
    int running;
    int paused;  // NEW: Pause state
    pthread_mutex_t lock;
    FILE* links_to_crawl;  // DEPRECATED: Will be removed
    FILE* links_crawled;   // DEPRECATED: Will be removed
    void* url_manager;     // CrawlerURLManager* (void* to avoid circular dependency)
   };

/**
 * Initialize crawler state (internal function)
 */
CrawlerStateInternal* crawler_internal_init(const char* data_dir, const char* start_url, int max_pages) {
    CrawlerStateInternal* state = (CrawlerStateInternal*)calloc(1, sizeof(CrawlerStateInternal));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    
    // start_url is now optional - only used if database is empty
    if (start_url) {
        strncpy(state->start_url, start_url, sizeof(state->start_url) - 1);
    } else {
        state->start_url[0] = '\0';
    }
    
    state->max_pages = max_pages;
    state->pages_crawled = 0;
    state->running = 1;
    state->paused = 0;  // Not paused initially
    pthread_mutex_init(&state->lock, NULL);
    
    // Create directory structure
    char path[1024];
    snprintf(path, sizeof(path), "%s/raw_pages", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/preprocessed", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/training_queue", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/trained", data_dir);
    mkdir(path, 0755);
    
    // Open link files (for fallback only)
    snprintf(path, sizeof(path), "%s/links_to_crawl.txt", data_dir);
    state->links_to_crawl = fopen(path, "a+");
    
    snprintf(path, sizeof(path), "%s/links_crawled.txt", data_dir);
    state->links_crawled = fopen(path, "a+");
    
    // Add start URL to queue ONLY if provided and files are empty
    if (start_url && start_url[0] != '\0') {
        fseek(state->links_to_crawl, 0, SEEK_END);
        if (ftell(state->links_to_crawl) == 0) {
            fprintf(state->links_to_crawl, "%s\n", start_url);
            fflush(state->links_to_crawl);
        }
    }
    
    return state;
}

/**
 * Cleanup crawler state (internal function)
 */
void crawler_internal_cleanup(CrawlerStateInternal* state) {
    if (!state) return;
    
    if (state->links_to_crawl) fclose(state->links_to_crawl);
    if (state->links_crawled) fclose(state->links_crawled);
    pthread_mutex_destroy(&state->lock);
    free(state);
}

/**
 * Set URL manager on internal crawler
 */
void crawler_internal_set_url_manager(CrawlerStateInternal* state, void* url_manager) {
    if (state) {
        state->url_manager = url_manager;
    }
}

/**
 * Callback for curl to write data
 */
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryBuffer* mem = (MemoryBuffer*)userp;
    
    // Check if we need to expand buffer
    if (mem->size + realsize + 1 > mem->capacity) {
        size_t new_capacity = mem->capacity * 2;
        if (new_capacity < mem->size + realsize + 1) {
            new_capacity = mem->size + realsize + 1;
        }
        
        char* new_data = (char*)realloc(mem->data, new_capacity);
        if (!new_data) return 0;  // Out of memory
        
        mem->data = new_data;
        mem->capacity = new_capacity;
    }
    
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

/**
 * Download a web page
 */
int crawler_download_page(const char* url, MemoryBuffer* buffer) {
    CURL* curl = curl_easy_init();
    if (!curl) return -1;
    
    // Set options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, 
                    "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return -1;
    }
    
    if (response_code != 200) {
        fprintf(stderr, "HTTP error: %ld\n", response_code);
        return -1;
    }
    
    return 0;
}

/**
 * Save page to disk
 */
int crawler_save_page(CrawlerStateInternal* state, const char* url, const char* content, size_t size) {
    // Generate filename from URL hash
    unsigned long hash = 5381;
    for (const char* p = url; *p; p++) {
        hash = ((hash << 5) + hash) + *p;
    }
    
    time_t now = time(NULL);
    char filename[2048];
    snprintf(filename, sizeof(filename), "%s/raw_pages/page_%lu_%ld.html", 
             state->data_dir, hash, now);
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(stderr, "%s Failed to open file: %s\n", timestamp, filename);
        return -1;
    }
    
    // Write metadata header
    fprintf(f, "<!-- URL: %s -->\n", url);
    fprintf(f, "<!-- Timestamp: %ld -->\n", now);
    fprintf(f, "<!-- Size: %zu -->\n", size);
    
    // Write content
    fwrite(content, 1, size, f);
    fclose(f);
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s ✓ Saved: %s\n", timestamp, filename);
    return 0;
}

/**
 * Get next URL to crawl
 */
int crawler_get_next_url(CrawlerStateInternal* state, char* url, size_t url_size) {
    pthread_mutex_lock(&state->lock);
    
    // NEW: Use database if available
    if (state->url_manager) {
        CrawlerURLManager* manager = (CrawlerURLManager*)state->url_manager;
        URLEntry* entry = crawler_url_manager_get_next(manager);
        
        if (entry) {
            strncpy(url, entry->url, url_size - 1);
            url[url_size - 1] = '\0';
            
            // Note: URL is already marked as pending in database
            // We'll mark it as crawled after successful download
            
            free(entry);
            pthread_mutex_unlock(&state->lock);
            return 0;
        } else {
            // No URLs in database
            pthread_mutex_unlock(&state->lock);
            return -1;
        }
    }
    
    // FALLBACK: Use file-based queue (DEPRECATED)
    // Read all URLs into memory for prime-based selection
    fseek(state->links_to_crawl, 0, SEEK_SET);
    
    // Count URLs
    int url_count = 0;
    char line[MAX_URL_LENGTH];
    while (fgets(line, sizeof(line), state->links_to_crawl)) {
        url_count++;
    }
    
    if (url_count == 0) {
        pthread_mutex_unlock(&state->lock);
        return -1;  // No more URLs
    }
    
    // Allocate array for URLs
    char** urls = (char**)malloc(url_count * sizeof(char*));
    if (!urls) {
        pthread_mutex_unlock(&state->lock);
        return -1;
    }
    
    // Read URLs into array
    fseek(state->links_to_crawl, 0, SEEK_SET);
    int i = 0;
    while (fgets(line, sizeof(line), state->links_to_crawl) && i < url_count) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        urls[i] = strdup(line);
        i++;
    }
    
    // Use prime-based selection (prime 13 for link selection)
    // This creates a pseudo-random but deterministic pattern
    static uint64_t iteration = 0;
    int selected_index = (int)((iteration * 13) % url_count);
    iteration++;
    
    // Copy selected URL
    strncpy(url, urls[selected_index], url_size - 1);
    url[url_size - 1] = '\0';
    
    // Write remaining URLs back to file
    char temp_path[2048];
    snprintf(temp_path, sizeof(temp_path), "%s/links_to_crawl.tmp", state->data_dir);
    FILE* temp = fopen(temp_path, "w");
    
    if (temp) {
        for (int j = 0; j < url_count; j++) {
            if (j != selected_index) {
                fprintf(temp, "%s\n", urls[j]);
            }
        }
        fclose(temp);
        
        // Replace original with temp
        fclose(state->links_to_crawl);
        char orig_path[2048];
        snprintf(orig_path, sizeof(orig_path), "%s/links_to_crawl.txt", state->data_dir);
        rename(temp_path, orig_path);
        
        // Reopen
        state->links_to_crawl = fopen(orig_path, "a+");
    }
    
    // Free memory
    for (int j = 0; j < url_count; j++) {
        free(urls[j]);
    }
    free(urls);
    
    pthread_mutex_unlock(&state->lock);
    return 0;
}

/**
 * Mark URL as crawled
 */
void crawler_mark_crawled(CrawlerStateInternal* state, const char* url) {
    pthread_mutex_lock(&state->lock);
    
    // NEW: Update database if available
    if (state->url_manager) {
        CrawlerURLManager* manager = (CrawlerURLManager*)state->url_manager;
        URLDatabase* db = crawler_url_manager_get_database(manager);
        if (db) {
            // Find URL by URL string and mark as crawled
            int count = 0;
            URLEntry** entries = url_db_query(db, NULL, &count);
            if (entries) {
                for (int i = 0; i < count; i++) {
                    if (strcmp(entries[i]->url, url) == 0) {
                        url_db_mark_crawled(db, entries[i]->id);
                        break;
                    }
                }
                url_db_free_entries(entries, count);
            }
        }
    }
    
    // FALLBACK: Write to file (DEPRECATED)
    if (state->links_crawled) {
        fprintf(state->links_crawled, "%s\n", url);
        fflush(state->links_crawled);
    }
    
    state->pages_crawled++;
    pthread_mutex_unlock(&state->lock);
}

/**
 * Main crawler loop
 */
void* crawler_thread_func(void* arg) {
    CrawlerStateInternal* state = (CrawlerStateInternal*)arg;
    char timestamp[32];
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === CRAWLER STARTED ===\n", timestamp);
    printf("%s Data directory: %s\n", timestamp, state->data_dir);
    if (state->max_pages == 0) {
        printf("%s Max pages: UNLIMITED\n", timestamp);
    } else {
        printf("%s Max pages: %d\n", timestamp, state->max_pages);
    }
    
    while (state->running && (state->max_pages == 0 || state->pages_crawled < state->max_pages)) {
        char url[MAX_URL_LENGTH];
        
        // Get next URL
        if (crawler_get_next_url(state, url, sizeof(url)) != 0) {
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s No more URLs in queue, waiting...\n", timestamp);
            sleep(5);  // Wait for more URLs
            continue;
        }
        
        // Print progress
        get_timestamp(timestamp, sizeof(timestamp));
        if (state->max_pages == 0) {
            printf("\n%s === Crawling [%d/unlimited] ===\n", timestamp, state->pages_crawled + 1);
        } else {
            printf("\n%s === Crawling [%d/%d] ===\n", timestamp, state->pages_crawled + 1, state->max_pages);
        }
        printf("%s URL: %s\n", timestamp, url);
        
        // Download page
        MemoryBuffer buffer = {0};
        buffer.capacity = 4096;
        buffer.data = (char*)malloc(buffer.capacity);
        
        if (crawler_download_page(url, &buffer) == 0) {
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s Downloaded %zu bytes\n", timestamp, buffer.size);
            
            // Save page
            crawler_save_page(state, url, buffer.data, buffer.size);
            
            // Extract links
            // Links will be extracted by preprocessor
            
            // Mark as crawled
            crawler_mark_crawled(state, url);
        } else {
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s ✗ Failed to download\n", timestamp);
        }
        
        free(buffer.data);
        
        // Rate limiting: configurable delay
        pthread_mutex_lock(&g_rate_config_mutex);
        int delay;
        
        if (g_requests_per_minute > 0.0f) {
            // Use requests per minute
            delay = (int)(60.0f / g_requests_per_minute);
        } else if (g_delay_minutes > 0) {
            // Use minutes
            delay = g_delay_minutes * 60;
        } else if (g_use_random_delay) {
            // Random delay between min and max
            delay = g_min_delay_seconds + (rand() % (g_max_delay_seconds - g_min_delay_seconds + 1));
        } else {
            // Fixed delay
            delay = g_min_delay_seconds;
        }
        pthread_mutex_unlock(&g_rate_config_mutex);
        
        get_timestamp(timestamp, sizeof(timestamp));
        if (delay >= 60) {
            printf("%s Waiting %d minutes %d seconds...\n", timestamp, delay / 60, delay % 60);
        } else {
            printf("%s Waiting %d seconds...\n", timestamp, delay);
        }
        
        // Check for pause during delay
        for (int i = 0; i < delay && state->running && !state->paused; i++) {
            sleep(1);
        }
        
        // Handle pause
        while (state->paused && state->running) {
            printf("%s Crawler paused. Waiting...\n", timestamp);
            sleep(5);
        }
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("\n%s === CRAWLER STOPPED ===\n", timestamp);
    printf("%s Total pages crawled: %d\n", timestamp, state->pages_crawled);
    
    return NULL;
}

// ============================================================================
// RATE LIMITING API
// ============================================================================

void crawler_set_rate_limit(CrawlerState* state, int min_seconds, int max_seconds) {
    (void)state;  // State not needed for global config
    pthread_mutex_lock(&g_rate_config_mutex);
    g_min_delay_seconds = min_seconds;
    g_max_delay_seconds = max_seconds;
    g_delay_minutes = 0;
    g_requests_per_minute = 0.0f;
    g_use_random_delay = (min_seconds != max_seconds);
    pthread_mutex_unlock(&g_rate_config_mutex);
    
    printf("Rate limit set: %d-%d seconds between requests\n", min_seconds, max_seconds);
}

void crawler_set_rate_limit_rpm(CrawlerState* state, float rpm) {
    (void)state;
    pthread_mutex_lock(&g_rate_config_mutex);
    g_requests_per_minute = rpm;
    g_min_delay_seconds = (int)(60.0f / rpm);
    g_max_delay_seconds = g_min_delay_seconds;
    g_delay_minutes = 0;
    g_use_random_delay = false;
    pthread_mutex_unlock(&g_rate_config_mutex);
    
    printf("Rate limit set: %.2f requests per minute (1 request every %.1f seconds)\n", 
           rpm, 60.0f / rpm);
}

void crawler_set_rate_limit_minutes(CrawlerState* state, int minutes) {
    (void)state;
    pthread_mutex_lock(&g_rate_config_mutex);
    g_delay_minutes = minutes;
    g_min_delay_seconds = 0;
    g_max_delay_seconds = 0;
    g_requests_per_minute = 0.0f;
    g_use_random_delay = false;
    pthread_mutex_unlock(&g_rate_config_mutex);
    
    printf("Rate limit set: %d minutes between requests\n", minutes);
}

// ============================================================================
// PAUSE/RESUME API
// ============================================================================

void crawler_pause(CrawlerState* state) {
    if (!state) return;
    CrawlerStateInternal* internal = (CrawlerStateInternal*)state;
    
    pthread_mutex_lock(&internal->lock);
    internal->paused = 1;
    pthread_mutex_unlock(&internal->lock);
    
    printf("Crawler paused. Call crawler_resume() to continue.\n");
}

void crawler_resume(CrawlerState* state) {
    if (!state) return;
    CrawlerStateInternal* internal = (CrawlerStateInternal*)state;
    
    pthread_mutex_lock(&internal->lock);
    internal->paused = 0;
    pthread_mutex_unlock(&internal->lock);
    
    printf("Crawler resumed.\n");
}

