#ifndef URL_DATABASE_H
#define URL_DATABASE_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/**
 * Comprehensive URL Database for Crawler
 * 
 * Features:
 * - SQLite backend for robust storage
 * - Preserves GET parameters (critical!)
 * - Timestamp tracking for prioritization
 * - Status tracking (pending, crawled, failed, blocked)
 * - Domain and file type categorization
 * - Export/import functionality
 */

// URL entry in database
typedef struct {
    uint64_t id;
    char url[4096];              // Full URL with GET parameters
    char domain[256];
    char path[2048];
    char query_string[1024];     // GET parameters preserved
    char file_type[32];          // html, pdf, txt, etc.
    time_t first_seen;
    time_t last_crawled;
    int crawl_count;
    int priority;
    char status[32];             // pending, crawled, failed, blocked
    char source_url[4096];
    bool blocked;
} URLEntry;

// URL database handle
typedef struct URLDatabase URLDatabase;

/**
 * Create/open database
 * 
 * @param db_path Path to SQLite database file
 * @return Database handle or NULL on error
 */
URLDatabase* url_db_open(const char* db_path);

/**
 * Close database and free resources
 * 
 * @param db Database handle
 */
void url_db_close(URLDatabase* db);

/**
 * Add URL to database
 * 
 * @param db Database handle
 * @param url Full URL (with GET parameters)
 * @param source_url URL that linked to this (can be NULL)
 * @return 0 on success, -1 on error
 */
int url_db_add(URLDatabase* db, const char* url, const char* source_url);

/**
 * Remove URL from database
 * 
 * @param db Database handle
 * @param id URL ID
 * @return 0 on success, -1 on error
 */
int url_db_remove(URLDatabase* db, uint64_t id);

/**
 * Block URL
 * 
 * @param db Database handle
 * @param id URL ID
 * @return 0 on success, -1 on error
 */
int url_db_block(URLDatabase* db, uint64_t id);

/**
 * Unblock URL
 * 
 * @param db Database handle
 * @param id URL ID
 * @return 0 on success, -1 on error
 */
int url_db_unblock(URLDatabase* db, uint64_t id);

/**
 * Mark URL as crawled
 * 
 * @param db Database handle
 * @param id URL ID
 * @return 0 on success, -1 on error
 */
int url_db_mark_crawled(URLDatabase* db, uint64_t id);

/**
 * Mark URL as failed
 * 
 * @param db Database handle
 * @param id URL ID
 * @return 0 on success, -1 on error
 */
int url_db_mark_failed(URLDatabase* db, uint64_t id);

/**
 * Mark URL as currently being crawled
 * 
 * @param db Database handle
 * @param id URL ID
 * @return 0 on success, -1 on error
 */
int url_db_mark_crawling(URLDatabase* db, uint64_t id);

/**
 * Get next URL to crawl (highest priority, uncrawled)
 * 
 * @param db Database handle
 * @return URL entry or NULL if no URLs available
 */
URLEntry* url_db_get_next(URLDatabase* db);

/**
 * Query URLs with filter
 * 
 * @param db Database handle
 * @param filter SQL WHERE clause (can be NULL for all)
 * @param count Output: number of results
 * @return Array of URL entries (must be freed by caller)
 */
URLEntry** url_db_query(URLDatabase* db, const char* filter, int* count);

/**
 * Get URL by ID
 * 
 * @param db Database handle
 * @param id URL ID
 * @return URL entry or NULL if not found
 */
URLEntry* url_db_get_by_id(URLDatabase* db, uint64_t id);

/**
 * Check if URL exists in database
 * 
 * @param db Database handle
 * @param url Full URL to check
 * @return true if exists, false otherwise
 */
bool url_db_exists(URLDatabase* db, const char* url);

/**
 * Get total URL count
 * 
 * @param db Database handle
 * @return Total number of URLs
 */
int url_db_count_total(URLDatabase* db);

/**
 * Get pending URL count
 * 
 * @param db Database handle
 * @return Number of pending URLs
 */
int url_db_count_pending(URLDatabase* db);

/**
 * Get crawled URL count
 * 
 * @param db Database handle
 * @return Number of crawled URLs
 */
int url_db_count_crawled(URLDatabase* db);

/**
 * Get blocked URL count
 * 
 * @param db Database handle
 * @return Number of blocked URLs
 */
int url_db_count_blocked(URLDatabase* db);

/**
 * Export URLs to file
 * 
 * @param db Database handle
 * @param file_path Output file path
 * @return 0 on success, -1 on error
 */
int url_db_export(URLDatabase* db, const char* file_path);

/**
 * Import URLs from file
 * 
 * @param db Database handle
 * @param file_path Input file path
 * @return Number of URLs imported, -1 on error
 */
int url_db_import(URLDatabase* db, const char* file_path);

/**
 * Reset all URLs to pending status (for recrawling)
 * 
 * @param db Database handle
 * @return Number of URLs reset, -1 on error
 */
int url_db_reset_all_to_pending(URLDatabase* db);

/**
 * Free URL entry
 * 
 * @param entry URL entry to free
 */
void url_db_free_entry(URLEntry* entry);

/**
 * Free URL entry array
 * 
 * @param entries Array of URL entries
 * @param count Number of entries
 */
void url_db_free_entries(URLEntry** entries, int count);

#endif // URL_DATABASE_H