#ifndef URL_FILTER_H
#define URL_FILTER_H

#include <stdbool.h>

/**
 * URL Filter System
 * 
 * Features:
 * - File type filtering (allow/block)
 * - Domain whitelist/blacklist
 * - URL pattern matching (regex)
 * - GET parameter handling
 */

// Filter configuration
typedef struct {
    // File type filters
    bool allow_html;
    bool allow_pdf;
    bool allow_txt;
    bool allow_doc;
    bool allow_images;
    bool allow_video;
    bool allow_audio;
    bool allow_archives;
    bool allow_code;
    bool allow_data;
    
    // Domain filters
    char** domain_whitelist;
    int whitelist_count;
    char** domain_blacklist;
    int blacklist_count;
    
    // Pattern filters (regex)
    char** url_patterns;
    int pattern_count;
    
    // GET parameter handling
    bool preserve_query_params;
    bool strip_tracking_params;
    char** tracking_param_names;
    int tracking_param_count;
} URLFilterConfig;

// Filter handle
typedef struct URLFilter URLFilter;

/**
 * Create filter with configuration
 * 
 * @param config Filter configuration (NULL for defaults)
 * @return Filter handle or NULL on error
 */
URLFilter* url_filter_create(URLFilterConfig* config);

/**
 * Destroy filter and free resources
 * 
 * @param filter Filter handle
 */
void url_filter_destroy(URLFilter* filter);

/**
 * Check if URL should be crawled
 * 
 * @param filter Filter handle
 * @param url URL to check
 * @return true if should crawl, false otherwise
 */
bool url_filter_should_crawl(URLFilter* filter, const char* url);

/**
 * Check if file type is allowed
 * 
 * @param filter Filter handle
 * @param file_type File type (html, pdf, txt, etc.)
 * @return true if allowed, false otherwise
 */
bool url_filter_is_allowed_type(URLFilter* filter, const char* file_type);

/**
 * Check if domain is allowed
 * 
 * @param filter Filter handle
 * @param domain Domain to check
 * @return true if allowed, false otherwise
 */
bool url_filter_is_allowed_domain(URLFilter* filter, const char* domain);

/**
 * Check if URL matches any pattern
 * 
 * @param filter Filter handle
 * @param url URL to check
 * @return true if matches, false otherwise
 */
bool url_filter_matches_pattern(URLFilter* filter, const char* url);

/**
 * Add domain to whitelist
 * 
 * @param filter Filter handle
 * @param domain Domain to add
 * @return 0 on success, -1 on error
 */
int url_filter_add_domain_whitelist(URLFilter* filter, const char* domain);

/**
 * Add domain to blacklist
 * 
 * @param filter Filter handle
 * @param domain Domain to add
 * @return 0 on success, -1 on error
 */
int url_filter_add_domain_blacklist(URLFilter* filter, const char* domain);

/**
 * Add URL pattern (regex)
 * 
 * @param filter Filter handle
 * @param pattern Regex pattern
 * @return 0 on success, -1 on error
 */
int url_filter_add_pattern(URLFilter* filter, const char* pattern);

/**
 * Remove domain from whitelist
 * 
 * @param filter Filter handle
 * @param domain Domain to remove
 * @return 0 on success, -1 on error
 */
int url_filter_remove_domain_whitelist(URLFilter* filter, const char* domain);

/**
 * Remove domain from blacklist
 * 
 * @param filter Filter handle
 * @param domain Domain to remove
 * @return 0 on success, -1 on error
 */
int url_filter_remove_domain_blacklist(URLFilter* filter, const char* domain);

/**
 * Remove URL pattern
 * 
 * @param filter Filter handle
 * @param pattern Pattern to remove
 * @return 0 on success, -1 on error
 */
int url_filter_remove_pattern(URLFilter* filter, const char* pattern);

/**
 * Get filter configuration
 * 
 * @param filter Filter handle
 * @return Configuration (do not free)
 */
URLFilterConfig* url_filter_get_config(URLFilter* filter);

/**
 * Set file type filter
 * 
 * @param filter Filter handle
 * @param file_type File type (html, pdf, txt, etc.)
 * @param allowed true to allow, false to block
 * @return 0 on success, -1 on error
 */
int url_filter_set_type(URLFilter* filter, const char* file_type, bool allowed);

/**
 * Save filter configuration to file
 * 
 * @param filter Filter handle
 * @param file_path Output file path
 * @return 0 on success, -1 on error
 */
int url_filter_save(URLFilter* filter, const char* file_path);

/**
 * Load filter configuration from file
 * 
 * @param filter Filter handle
 * @param file_path Input file path
 * @return 0 on success, -1 on error
 */
int url_filter_load(URLFilter* filter, const char* file_path);

#endif // URL_FILTER_H