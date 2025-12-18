/**
 * Crawler URL Manager Implementation
 * 
 * Integrates all URL management systems
 */

#include "crawler_url_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// Manager structure
struct CrawlerURLManager {
    URLDatabase* database;
    URLFilter* filter;
    URLPriority* priority;
    URLBlocker* blocker;
    char data_dir[1024];
};

/**
 * Create crawler URL manager
 */
CrawlerURLManager* crawler_url_manager_create(const char* data_dir) {
    if (!data_dir) return NULL;
    
    CrawlerURLManager* manager = (CrawlerURLManager*)calloc(1, sizeof(CrawlerURLManager));
    if (!manager) return NULL;
    
    strncpy(manager->data_dir, data_dir, sizeof(manager->data_dir) - 1);
    
    // Ensure data directory exists
    struct stat st = {0};
    if (stat(data_dir, &st) == -1) {
        // Directory doesn't exist, create it
        if (mkdir(data_dir, 0755) != 0) {
            fprintf(stderr, "Failed to create data directory: %s\n", data_dir);
            free(manager);
            return NULL;
        }
    }
    
    // Create database path
    char db_path[2048];
    snprintf(db_path, sizeof(db_path), "%s/urls.db", data_dir);
    
    // Initialize database
    manager->database = url_db_open(db_path);
    if (!manager->database) {
        fprintf(stderr, "Failed to open URL database: %s\n", db_path);
        free(manager);
        return NULL;
    }
    
    // Initialize filter (with defaults)
    manager->filter = url_filter_create(NULL);
    if (!manager->filter) {
        fprintf(stderr, "Failed to create URL filter\n");
        url_db_close(manager->database);
        free(manager);
        return NULL;
    }
    
    // Load filter config if exists
    char filter_path[2048];
    snprintf(filter_path, sizeof(filter_path), "%s/url_filter.conf", data_dir);
    url_filter_load(manager->filter, filter_path);
    
    // Initialize priority calculator (with defaults)
    manager->priority = url_priority_create(NULL);
    if (!manager->priority) {
        fprintf(stderr, "Failed to create URL priority calculator\n");
        url_filter_destroy(manager->filter);
        url_db_close(manager->database);
        free(manager);
        return NULL;
    }
    
    // Initialize blocker
    char blocker_path[2048];
    snprintf(blocker_path, sizeof(blocker_path), "%s/url_blocker.txt", data_dir);
    manager->blocker = url_blocker_create(blocker_path);
    if (!manager->blocker) {
        fprintf(stderr, "Failed to create URL blocker\n");
        url_priority_destroy(manager->priority);
        url_filter_destroy(manager->filter);
        url_db_close(manager->database);
        free(manager);
        return NULL;
    }
    
    printf("✓ Crawler URL Manager initialized\n");
    printf("  Database: %s\n", db_path);
    printf("  Filter config: %s\n", filter_path);
    printf("  Blocker patterns: %s\n", blocker_path);
    
    return manager;
}

/**
 * Destroy crawler URL manager
 */
void crawler_url_manager_destroy(CrawlerURLManager* manager) {
    if (!manager) return;
    
    if (manager->blocker) {
        url_blocker_destroy(manager->blocker);
    }
    
    if (manager->priority) {
        url_priority_destroy(manager->priority);
    }
    
    if (manager->filter) {
        // Save filter config
        char filter_path[2048];
        snprintf(filter_path, sizeof(filter_path), "%s/url_filter.conf", manager->data_dir);
        url_filter_save(manager->filter, filter_path);
        
        url_filter_destroy(manager->filter);
    }
    
    if (manager->database) {
        url_db_close(manager->database);
    }
    
    free(manager);
}

/**
 * Check if URL should be crawled
 */
bool crawler_url_manager_should_crawl(CrawlerURLManager* manager, const char* url) {
    if (!manager || !url) return false;
    
    // Check if already exists
    if (url_db_exists(manager->database, url)) {
        return false;  // Already in database
    }
    
    // Check blocker
    if (url_blocker_is_blocked(manager->blocker, url)) {
        return false;  // Blocked
    }
    
    // Check filter
    if (!url_filter_should_crawl(manager->filter, url)) {
        return false;  // Filtered out
    }
    
    return true;
}

/**
 * Add URL to crawl queue
 */
int crawler_url_manager_add(CrawlerURLManager* manager, const char* url, const char* source_url) {
    if (!manager || !url) return -2;
    
    // Check if should crawl
    if (!crawler_url_manager_should_crawl(manager, url)) {
        return -1;  // Filtered or blocked
    }
    
    // Add to database
    int result = url_db_add(manager->database, url, source_url);
    if (result != 0) {
        return -2;  // Database error
    }
    
    // Get the entry to calculate priority
    URLEntry* entry = url_db_get_by_id(manager->database, url_db_count_total(manager->database));
    if (entry) {
        // Calculate and update priority
        int total_domains = url_priority_get_domain_count(manager->priority);
        int priority __attribute__((unused)) = url_priority_calculate(manager->priority, entry, total_domains);
        (void)priority; // Suppress unused variable warning
        (void)priority; // Suppress unused variable warning
        (void)priority; // Suppress unused variable warning
        
        // Update priority in database (we'll need to add this function)
        // For now, the priority is calculated on-the-fly during get_next
        
        url_db_free_entry(entry);
    }
    
    return 0;
}

/**
 * Add multiple URLs in batch
 */
int crawler_url_manager_add_batch(CrawlerURLManager* manager, char** urls, int count, const char* source_url) {
    if (!manager || !urls || count <= 0) return 0;
    
    int added = 0;
    for (int i = 0; i < count; i++) {
        if (crawler_url_manager_add(manager, urls[i], source_url) == 0) {
            added++;
        }
    }
    
    return added;
}

/**
 * Get next URL to crawl
 */
URLEntry* crawler_url_manager_get_next(CrawlerURLManager* manager) {
    if (!manager) return NULL;
    
    // Get all pending URLs
    int count = 0;
    URLEntry** entries = url_db_query(manager->database, "status = 'pending' AND blocked = 0", &count);
    
    if (!entries || count == 0) {
        if (entries) free(entries);
        return NULL;
    }
    
    // Calculate priorities for all
    int total_domains = url_priority_get_domain_count(manager->priority);
    int best_idx = 0;
    int best_priority = url_priority_calculate(manager->priority, entries[0], total_domains);
    
    for (int i = 1; i < count; i++) {
        int priority = url_priority_calculate(manager->priority, entries[i], total_domains);
        if (priority > best_priority) {
            best_priority = priority;
            best_idx = i;
        }
    }
    
    // Get the best entry
    URLEntry* result = (URLEntry*)malloc(sizeof(URLEntry));
    if (result) {
        memcpy(result, entries[best_idx], sizeof(URLEntry));
        
        // CRITICAL FIX: Mark URL as 'crawling' immediately to prevent re-selection
        url_db_mark_crawling(manager->database, result->id);
    }
    
    // Free the query results
    url_db_free_entries(entries, count);
    
    return result;
}

/**
 * Mark URL as crawled
 */
int crawler_url_manager_mark_crawled(CrawlerURLManager* manager, uint64_t url_id) {
    if (!manager) return -1;
    
    // Get URL entry to update domain stats
    URLEntry* entry = url_db_get_by_id(manager->database, url_id);
    if (entry) {
        // Update domain statistics
        url_priority_update_domain_stats(manager->priority, entry->domain);
        url_db_free_entry(entry);
    }
    
    // Mark as crawled in database
    return url_db_mark_crawled(manager->database, url_id);
}

/**
 * Mark URL as failed
 */
int crawler_url_manager_mark_failed(CrawlerURLManager* manager, uint64_t url_id) {
    if (!manager) return -1;
    
    return url_db_mark_failed(manager->database, url_id);
}

/**
 * Get statistics
 */
void crawler_url_manager_get_stats(CrawlerURLManager* manager, int* total, int* pending, int* crawled, int* blocked) {
    if (!manager) return;
    
    if (total) *total = url_db_count_total(manager->database);
    if (pending) *pending = url_db_count_pending(manager->database);
    if (crawled) *crawled = url_db_count_crawled(manager->database);
    if (blocked) *blocked = url_db_count_blocked(manager->database);
}

/**
 * Get URL database
 */
URLDatabase* crawler_url_manager_get_database(CrawlerURLManager* manager) {
    if (!manager) return NULL;
    return manager->database;
}

/**
 * Get URL filter
 */
URLFilter* crawler_url_manager_get_filter(CrawlerURLManager* manager) {
    if (!manager) return NULL;
    return manager->filter;
}

/**
 * Get URL priority calculator
 */
URLPriority* crawler_url_manager_get_priority(CrawlerURLManager* manager) {
    if (!manager) return NULL;
    return manager->priority;
}

/**
 * Get URL blocker
 */
URLBlocker* crawler_url_manager_get_blocker(CrawlerURLManager* manager) {
    if (!manager) return NULL;
    return manager->blocker;
}

/**
 * Recalculate priorities for all pending URLs
 */
int crawler_url_manager_recalculate_priorities(CrawlerURLManager* manager) {
    if (!manager) return -1;
    
    // Get all pending URLs
    int count = 0;
    URLEntry** entries = url_db_query(manager->database, "status = 'pending'", &count);
    
    if (!entries || count == 0) {
        if (entries) free(entries);
        return 0;
    }
    
    // Recalculate priorities
    int total_domains = url_priority_get_domain_count(manager->priority);
    
    for (int i = 0; i < count; i++) {
        int priority = url_priority_calculate(manager->priority, entries[i], total_domains);
        // Note: We would need to add a function to update priority in database
        // For now, priorities are calculated on-the-fly
        (void)priority;  // Suppress unused warning
    }
    
    url_db_free_entries(entries, count);
    
    return count;
}

/**
 * Export URLs to file
 */
int crawler_url_manager_export(CrawlerURLManager* manager, const char* file_path) {
    if (!manager || !file_path) return -1;
    
    return url_db_export(manager->database, file_path);
}

/**
 * Import URLs from file
 */
int crawler_url_manager_import(CrawlerURLManager* manager, const char* file_path) {
    if (!manager || !file_path) return -1;
    
    return url_db_import(manager->database, file_path);
}

/**
 * Reset all URLs to pending status (for recrawling)
 */
int crawler_url_manager_reset_all(CrawlerURLManager* manager) {
    if (!manager || !manager->database) return -1;
    
    return url_db_reset_all_to_pending(manager->database);
}
