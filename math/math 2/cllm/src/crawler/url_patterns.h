#ifndef URL_PATTERNS_H
#define URL_PATTERNS_H

#include <stdbool.h>

/**
 * Advanced URL Pattern Detection
 * 
 * Detects URLs from various sources beyond simple href attributes.
 */

/**
 * URL pattern types
 */
typedef enum {
    URL_PATTERN_HREF,           // Standard <a href="...">
    URL_PATTERN_ONCLICK,        // JavaScript onclick handlers
    URL_PATTERN_DATA_ATTR,      // Data attributes (data-href, data-url)
    URL_PATTERN_META_REFRESH,   // <meta http-equiv="refresh">
    URL_PATTERN_JS_LOCATION,    // window.location assignments
    URL_PATTERN_AJAX,           // AJAX endpoint URLs
    URL_PATTERN_API,            // API endpoint URLs
    URL_PATTERN_SITEMAP,        // Sitemap.xml references
    URL_PATTERN_FEED,           // RSS/Atom feed links
    URL_PATTERN_CANONICAL,      // Canonical URLs
    URL_PATTERN_OPENGRAPH,      // Open Graph URLs
    URL_PATTERN_ALL             // All patterns
} URLPatternType;

/**
 * URL pattern configuration
 */
typedef struct {
    bool enable_href;
    bool enable_onclick;
    bool enable_data_attr;
    bool enable_meta_refresh;
    bool enable_js_location;
    bool enable_ajax;
    bool enable_api;
    bool enable_sitemap;
    bool enable_feed;
    bool enable_canonical;
    bool enable_opengraph;
} URLPatternConfig;

/**
 * Initialize pattern configuration with defaults
 * 
 * @param config Output configuration
 */
void url_pattern_config_init_default(URLPatternConfig* config);

/**
 * Enable all URL patterns
 * 
 * @param config Configuration to modify
 */
void url_pattern_config_enable_all(URLPatternConfig* config);

/**
 * Extract URLs from HTML using configured patterns
 * 
 * @param html HTML content
 * @param base_url Base URL for resolving relative URLs
 * @param config Pattern configuration
 * @param output_file File to write extracted URLs
 * @return Number of URLs extracted, or -1 on error
 */
int extract_urls_advanced(const char* html, const char* base_url,
                          const URLPatternConfig* config,
                          const char* output_file);

/**
 * Extract URLs from specific pattern type
 * 
 * @param html HTML content
 * @param base_url Base URL for resolving relative URLs
 * @param pattern Pattern type to extract
 * @param output_file File to write extracted URLs
 * @return Number of URLs extracted, or -1 on error
 */
int extract_urls_pattern(const char* html, const char* base_url,
                        URLPatternType pattern,
                        const char* output_file);

/**
 * Get pattern type name
 * 
 * @param pattern Pattern type
 * @return Pattern name string
 */
const char* url_pattern_name(URLPatternType pattern);

#endif // URL_PATTERNS_H