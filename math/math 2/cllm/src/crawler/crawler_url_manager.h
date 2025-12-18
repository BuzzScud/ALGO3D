#ifndef CRAWLER_URL_MANAGER_H
#define CRAWLER_URL_MANAGER_H

#include "url_database.h"
#include "url_filter.h"
#include "url_priority.h"
#include "url_blocker.h"
#include <stdbool.h>

/**
 * Crawler URL Manager
 * 
 * Integrates all URL management systems with the crawler:
 * - URL Database (storage with GET params)
 * - URL Filter (file types, domains, patterns)
 * - URL Priority (prime-based, diversity)
 * - URL Blocker (4 blocking strategies)
 */

// Crawler URL manager handle
typedef struct CrawlerURLManager CrawlerURLManager;

/**
 * Create crawler URL manager
 * 
 * @param data_dir Data directory for databases
 * @return Manager handle or NULL on error
 */
CrawlerURLManager* crawler_url_manager_create(const char* data_dir);

/**
 * Destroy crawler URL manager
 * 
 * @param manager Manager handle
 */
void crawler_url_manager_destroy(CrawlerURLManager* manager);

/**
 * Add URL to crawl queue
 * 
 * Applies filtering and blocking before adding
 * 
 * @param manager Manager handle
 * @param url URL to add (with GET parameters preserved)
 * @param source_url Source URL (can be NULL)
 * @return 0 on success, -1 if filtered/blocked, -2 on error
 */
int crawler_url_manager_add(CrawlerURLManager* manager, const char* url, const char* source_url);

/**
 * Add multiple URLs in batch
 * 
 * @param manager Manager handle
 * @param urls Array of URLs
 * @param count Number of URLs
 * @param source_url Source URL for all (can be NULL)
 * @return Number of URLs added
 */
int crawler_url_manager_add_batch(CrawlerURLManager* manager, char** urls, int count, const char* source_url);

/**
 * Get next URL to crawl
 * 
 * Uses priority system to select best URL
 * 
 * @param manager Manager handle
 * @return URL entry or NULL if no URLs available (must be freed by caller)
 */
URLEntry* crawler_url_manager_get_next(CrawlerURLManager* manager);

/**
 * Mark URL as crawled
 * 
 * Updates statistics and domain tracking
 * 
 * @param manager Manager handle
 * @param url_id URL ID
 * @return 0 on success, -1 on error
 */
int crawler_url_manager_mark_crawled(CrawlerURLManager* manager, uint64_t url_id);

/**
 * Mark URL as failed
 * 
 * @param manager Manager handle
 * @param url_id URL ID
 * @return 0 on success, -1 on error
 */
int crawler_url_manager_mark_failed(CrawlerURLManager* manager, uint64_t url_id);

/**
 * Check if URL should be crawled
 * 
 * Checks filters and blockers
 * 
 * @param manager Manager handle
 * @param url URL to check
 * @return true if should crawl, false otherwise
 */
bool crawler_url_manager_should_crawl(CrawlerURLManager* manager, const char* url);

/**
 * Get statistics
 * 
 * @param manager Manager handle
 * @param total Output: total URLs
 * @param pending Output: pending URLs
 * @param crawled Output: crawled URLs
 * @param blocked Output: blocked URLs
 */
void crawler_url_manager_get_stats(CrawlerURLManager* manager, int* total, int* pending, int* crawled, int* blocked);

/**
 * Get URL database
 * 
 * @param manager Manager handle
 * @return URL database handle (do not free)
 */
URLDatabase* crawler_url_manager_get_database(CrawlerURLManager* manager);

/**
 * Reset all URLs to pending status (for recrawling)
 * 
 * @param manager Manager handle
 * @return Number of URLs reset, -1 on error
 */
int crawler_url_manager_reset_all(CrawlerURLManager* manager);

/**
 * Get URL filter
 * 
 * @param manager Manager handle
 * @return URL filter handle (do not free)
 */
URLFilter* crawler_url_manager_get_filter(CrawlerURLManager* manager);

/**
 * Get URL priority calculator
 * 
 * @param manager Manager handle
 * @return URL priority handle (do not free)
 */
URLPriority* crawler_url_manager_get_priority(CrawlerURLManager* manager);

/**
 * Get URL blocker
 * 
 * @param manager Manager handle
 * @return URL blocker handle (do not free)
 */
URLBlocker* crawler_url_manager_get_blocker(CrawlerURLManager* manager);

/**
 * Recalculate priorities for all pending URLs
 * 
 * Call this after changing priority factors or domain stats
 * 
 * @param manager Manager handle
 * @return Number of URLs updated, -1 on error
 */
int crawler_url_manager_recalculate_priorities(CrawlerURLManager* manager);

/**
 * Export URLs to file
 * 
 * @param manager Manager handle
 * @param file_path Output file path
 * @return 0 on success, -1 on error
 */
int crawler_url_manager_export(CrawlerURLManager* manager, const char* file_path);

/**
 * Import URLs from file
 * 
 * @param manager Manager handle
 * @param file_path Input file path
 * @return Number of URLs imported, -1 on error
 */
int crawler_url_manager_import(CrawlerURLManager* manager, const char* file_path);

#endif // CRAWLER_URL_MANAGER_H