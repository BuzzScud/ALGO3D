#ifndef LINK_MANAGEMENT_H
#define LINK_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/**
 * Dynamic Link Management for Crawler
 * 
 * Manages a queue of links to crawl with priority support,
 * duplicate detection, and dynamic addition during crawling.
 */

/**
 * Link entry in the queue
 */
typedef struct {
    char url[2048];
    int priority;              // Higher = more important
    time_t added_time;
    char source_url[2048];     // URL that linked to this
    bool crawled;
} CrawlerLink;

/**
 * Link queue manager
 */
typedef struct LinkQueue LinkQueue;

/**
 * Create a new link queue
 * 
 * @param queue_file Path to file for persistent storage
 * @return Link queue or NULL on error
 */
LinkQueue* link_queue_create(const char* queue_file);

/**
 * Add a link to the queue
 * 
 * @param queue Link queue
 * @param url URL to add
 * @param priority Priority (higher = more important)
 * @param source_url Source URL (can be NULL)
 * @return 0 on success, -1 on error
 */
int link_queue_add(LinkQueue* queue, const char* url, int priority, const char* source_url);

/**
 * Add multiple links in batch
 * 
 * @param queue Link queue
 * @param links Array of links
 * @param count Number of links
 * @return Number of links added
 */
int link_queue_add_batch(LinkQueue* queue, CrawlerLink* links, int count);

/**
 * Get next link to crawl (highest priority)
 * 
 * @param queue Link queue
 * @param url Output buffer for URL
 * @param url_size Size of output buffer
 * @return 0 on success, -1 if queue is empty
 */
int link_queue_get_next(LinkQueue* queue, char* url, size_t url_size);

/**
 * Mark a link as crawled
 * 
 * @param queue Link queue
 * @param url URL that was crawled
 * @return 0 on success, -1 on error
 */
int link_queue_mark_crawled(LinkQueue* queue, const char* url);

/**
 * Check if URL is already in queue or crawled
 * 
 * @param queue Link queue
 * @param url URL to check
 * @return true if duplicate, false otherwise
 */
bool link_queue_is_duplicate(LinkQueue* queue, const char* url);

/**
 * Get queue size (uncrawled links)
 * 
 * @param queue Link queue
 * @return Number of uncrawled links
 */
int link_queue_size(LinkQueue* queue);

/**
 * Get total links (crawled + uncrawled)
 * 
 * @param queue Link queue
 * @return Total number of links
 */
int link_queue_total(LinkQueue* queue);

/**
 * Clear all links from queue
 * 
 * @param queue Link queue
 * @return 0 on success, -1 on error
 */
int link_queue_clear(LinkQueue* queue);

/**
 * Save queue to file
 * 
 * @param queue Link queue
 * @return 0 on success, -1 on error
 */
int link_queue_save(LinkQueue* queue);

/**
 * Load queue from file
 * 
 * @param queue Link queue
 * @return 0 on success, -1 on error
 */
int link_queue_load(LinkQueue* queue);

/**
 * Destroy link queue and free memory
 * 
 * @param queue Link queue
 */
void link_queue_destroy(LinkQueue* queue);

#endif // LINK_MANAGEMENT_H