/**
 * Dynamic Link Management Implementation
 * 
 * In-memory link queue with priority support and duplicate detection.
 */

#include "link_management.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINKS 100000  // Maximum links in memory

/**
 * Link queue structure
 */
struct LinkQueue {
    CrawlerLink* links;
    int capacity;
    int count;
    char queue_file[1024];
};

/**
 * Create a new link queue
 */
LinkQueue* link_queue_create(const char* queue_file) {
    LinkQueue* queue = (LinkQueue*)calloc(1, sizeof(LinkQueue));
    if (!queue) return NULL;
    
    queue->capacity = MAX_LINKS;
    queue->links = (CrawlerLink*)calloc(queue->capacity, sizeof(CrawlerLink));
    if (!queue->links) {
        free(queue);
        return NULL;
    }
    
    queue->count = 0;
    
    if (queue_file) {
        strncpy(queue->queue_file, queue_file, sizeof(queue->queue_file) - 1);
        link_queue_load(queue);
    }
    
    return queue;
}

/**
 * Check if URL is already in queue
 */
bool link_queue_is_duplicate(LinkQueue* queue, const char* url) {
    if (!queue || !url) return false;
    
    for (int i = 0; i < queue->count; i++) {
        if (strcmp(queue->links[i].url, url) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 * Add a link to the queue
 */
int link_queue_add(LinkQueue* queue, const char* url, int priority, const char* source_url) {
    if (!queue || !url) return -1;
    
    // Check for duplicates
    if (link_queue_is_duplicate(queue, url)) {
        return 0;  // Already in queue, not an error
    }
    
    // Check capacity
    if (queue->count >= queue->capacity) {
        return -1;  // Queue full
    }
    
    // Add link
    CrawlerLink* link = &queue->links[queue->count];
    strncpy(link->url, url, sizeof(link->url) - 1);
    link->priority = priority;
    link->added_time = time(NULL);
    link->crawled = false;
    
    if (source_url) {
        strncpy(link->source_url, source_url, sizeof(link->source_url) - 1);
    } else {
        link->source_url[0] = '\0';
    }
    
    queue->count++;
    
    return 0;
}

/**
 * Add multiple links in batch
 */
int link_queue_add_batch(LinkQueue* queue, CrawlerLink* links, int count) {
    if (!queue || !links) return -1;
    
    int added = 0;
    for (int i = 0; i < count; i++) {
        if (link_queue_add(queue, links[i].url, links[i].priority, links[i].source_url) == 0) {
            added++;
        }
    }
    
    return added;
}

/**
 * Get next link to crawl (highest priority, uncrawled)
 */
int link_queue_get_next(LinkQueue* queue, char* url, size_t url_size) {
    if (!queue || !url) return -1;
    
    int best_index = -1;
    int best_priority = -1;
    
    // Find highest priority uncrawled link
    for (int i = 0; i < queue->count; i++) {
        if (!queue->links[i].crawled && queue->links[i].priority > best_priority) {
            best_priority = queue->links[i].priority;
            best_index = i;
        }
    }
    
    if (best_index < 0) {
        return -1;  // No uncrawled links
    }
    
    strncpy(url, queue->links[best_index].url, url_size - 1);
    url[url_size - 1] = '\0';
    
    return 0;
}

/**
 * Mark a link as crawled
 */
int link_queue_mark_crawled(LinkQueue* queue, const char* url) {
    if (!queue || !url) return -1;
    
    for (int i = 0; i < queue->count; i++) {
        if (strcmp(queue->links[i].url, url) == 0) {
            queue->links[i].crawled = true;
            return 0;
        }
    }
    
    return -1;  // URL not found
}

/**
 * Get queue size (uncrawled links)
 */
int link_queue_size(LinkQueue* queue) {
    if (!queue) return 0;
    
    int uncrawled = 0;
    for (int i = 0; i < queue->count; i++) {
        if (!queue->links[i].crawled) {
            uncrawled++;
        }
    }
    
    return uncrawled;
}

/**
 * Get total links
 */
int link_queue_total(LinkQueue* queue) {
    if (!queue) return 0;
    return queue->count;
}

/**
 * Clear all links
 */
int link_queue_clear(LinkQueue* queue) {
    if (!queue) return -1;
    
    queue->count = 0;
    memset(queue->links, 0, queue->capacity * sizeof(CrawlerLink));
    
    return 0;
}

/**
 * Save queue to file
 */
int link_queue_save(LinkQueue* queue) {
    if (!queue || queue->queue_file[0] == '\0') return -1;
    
    FILE* fp = fopen(queue->queue_file, "w");
    if (!fp) return -1;
    
    for (int i = 0; i < queue->count; i++) {
        CrawlerLink* link = &queue->links[i];
        fprintf(fp, "%s|%d|%ld|%s|%d\n",
                link->url,
                link->priority,
                (long)link->added_time,
                link->source_url,
                link->crawled ? 1 : 0);
    }
    
    fclose(fp);
    return 0;
}

/**
 * Load queue from file
 */
int link_queue_load(LinkQueue* queue) {
    if (!queue || queue->queue_file[0] == '\0') return -1;
    
    FILE* fp = fopen(queue->queue_file, "r");
    if (!fp) return 0;  // File doesn't exist yet, not an error
    
    char line[4096];
    while (fgets(line, sizeof(line), fp) && queue->count < queue->capacity) {
        CrawlerLink* link = &queue->links[queue->count];
        
        // Parse line: url|priority|time|source|crawled
        char* url = strtok(line, "|");
        char* priority_str = strtok(NULL, "|");
        char* time_str = strtok(NULL, "|");
        char* source = strtok(NULL, "|");
        char* crawled_str = strtok(NULL, "|\n");
        
        if (url && priority_str && time_str) {
            strncpy(link->url, url, sizeof(link->url) - 1);
            link->priority = atoi(priority_str);
            link->added_time = (time_t)atol(time_str);
            
            if (source) {
                strncpy(link->source_url, source, sizeof(link->source_url) - 1);
            }
            
            if (crawled_str) {
                link->crawled = (atoi(crawled_str) != 0);
            }
            
            queue->count++;
        }
    }
    
    fclose(fp);
    return 0;
}

/**
 * Destroy link queue
 */
void link_queue_destroy(LinkQueue* queue) {
    if (!queue) return;
    
    // Save before destroying
    link_queue_save(queue);
    
    if (queue->links) {
        free(queue->links);
    }
    
    free(queue);
}