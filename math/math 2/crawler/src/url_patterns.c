/**
 * Advanced URL Pattern Detection Implementation
 * 
 * Extracts URLs from various sources in HTML content.
 */

#include "url_patterns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Initialize pattern configuration with defaults
 */
void url_pattern_config_init_default(URLPatternConfig* config) {
    if (!config) return;
    
    config->enable_href = true;
    config->enable_onclick = true;
    config->enable_data_attr = true;
    config->enable_meta_refresh = false;
    config->enable_js_location = false;
    config->enable_ajax = false;
    config->enable_api = false;
    config->enable_sitemap = false;
    config->enable_feed = false;
    config->enable_canonical = false;
    config->enable_opengraph = false;
}

/**
 * Enable all URL patterns
 */
void url_pattern_config_enable_all(URLPatternConfig* config) {
    if (!config) return;
    
    config->enable_href = true;
    config->enable_onclick = true;
    config->enable_data_attr = true;
    config->enable_meta_refresh = true;
    config->enable_js_location = true;
    config->enable_ajax = true;
    config->enable_api = true;
    config->enable_sitemap = true;
    config->enable_feed = true;
    config->enable_canonical = true;
    config->enable_opengraph = true;
}

/**
 * Get pattern type name
 */
const char* url_pattern_name(URLPatternType pattern) {
    switch (pattern) {
        case URL_PATTERN_HREF: return "href";
        case URL_PATTERN_ONCLICK: return "onclick";
        case URL_PATTERN_DATA_ATTR: return "data-attr";
        case URL_PATTERN_META_REFRESH: return "meta-refresh";
        case URL_PATTERN_JS_LOCATION: return "js-location";
        case URL_PATTERN_AJAX: return "ajax";
        case URL_PATTERN_API: return "api";
        case URL_PATTERN_SITEMAP: return "sitemap";
        case URL_PATTERN_FEED: return "feed";
        case URL_PATTERN_CANONICAL: return "canonical";
        case URL_PATTERN_OPENGRAPH: return "opengraph";
        case URL_PATTERN_ALL: return "all";
        default: return "unknown";
    }
}

/**
 * Check if URL is valid
 */
static bool is_valid_url(const char* url) {
    if (!url || url[0] == '\0') return false;
    if (url[0] == '#') return false;
    if (strncmp(url, "javascript:", 11) == 0) return false;
    if (strncmp(url, "mailto:", 7) == 0) return false;
    if (strncmp(url, "tel:", 4) == 0) return false;
    if (strncmp(url, "data:", 5) == 0) return false;
    return true;
}

/**
 * Write URL to file
 */
static void write_url(FILE* fp, const char* url) {
    if (fp && url && is_valid_url(url)) {
        fprintf(fp, "%s\n", url);
    }
}

/**
 * Extract standard href URLs
 */
static int extract_href_urls(const char* html, const char* base_url __attribute__((unused)), FILE* output) {
    int count = 0;
    const char* p = html;
    
    while ((p = strstr(p, "href=")) != NULL) {
        p += 5;  // Skip "href="
        
        // Skip whitespace
        while (*p && isspace(*p)) p++;
        
        // Determine quote type
        char quote = 0;
        if (*p == '"' || *p == '\'') {
            quote = *p;
            p++;
        }
        
        // Find end of URL
        const char* url_start = p;
        const char* url_end = NULL;
        
        if (quote) {
            url_end = strchr(p, quote);
        } else {
            while (*p && !isspace(*p) && *p != '>') p++;
            url_end = p;
        }
        
        if (!url_end || url_end == url_start) {
            if (quote) p = url_end + 1;
            continue;
        }
        
        // Extract URL
        size_t url_len = url_end - url_start;
        if (url_len >= 2048) {
            p = url_end + 1;
            continue;
        }
        
        char url[2048];
        strncpy(url, url_start, url_len);
        url[url_len] = '\0';
        
        if (is_valid_url(url)) {
            write_url(output, url);
            count++;
        }
        
        p = url_end + 1;
    }
    
    return count;
}

/**
 * Extract onclick URLs
 */
static int extract_onclick_urls(const char* html, const char* base_url __attribute__((unused)), FILE* output) {
    int count = 0;
    const char* p = html;
    
    while ((p = strstr(p, "onclick=")) != NULL) {
        p += 8;  // Skip "onclick="
        
        // Look for URLs in onclick handlers
        // Common patterns: window.location='url', location.href='url'
        const char* loc = strstr(p, "location");
        if (loc && loc < p + 200) {
            const char* quote_start = strchr(loc, '\'');
            if (!quote_start) quote_start = strchr(loc, '"');
            
            if (quote_start) {
                char quote = *quote_start;
                const char* url_start = quote_start + 1;
                const char* url_end = strchr(url_start, quote);
                
                if (url_end && url_end > url_start) {
                    size_t url_len = url_end - url_start;
                    if (url_len < 2048) {
                        char url[2048];
                        strncpy(url, url_start, url_len);
                        url[url_len] = '\0';
                        
                        if (is_valid_url(url)) {
                            write_url(output, url);
                            count++;
                        }
                    }
                }
            }
        }
        
        p++;
    }
    
    return count;
}

/**
 * Extract data attribute URLs
 */
static int extract_data_attr_urls(const char* html, const char* base_url __attribute__((unused)), FILE* output) {
    int count = 0;
    const char* patterns[] = {"data-href=", "data-url=", "data-link=", NULL};
    
    for (int i = 0; patterns[i] != NULL; i++) {
        const char* p = html;
        size_t pattern_len = strlen(patterns[i]);
        
        while ((p = strstr(p, patterns[i])) != NULL) {
            p += pattern_len;
            
            // Skip whitespace
            while (*p && isspace(*p)) p++;
            
            // Get quote type
            char quote = 0;
            if (*p == '"' || *p == '\'') {
                quote = *p;
                p++;
            }
            
            const char* url_start = p;
            const char* url_end = quote ? strchr(p, quote) : NULL;
            
            if (!url_end) {
                while (*p && !isspace(*p) && *p != '>') p++;
                url_end = p;
            }
            
            if (url_end > url_start) {
                size_t url_len = url_end - url_start;
                if (url_len < 2048) {
                    char url[2048];
                    strncpy(url, url_start, url_len);
                    url[url_len] = '\0';
                    
                    if (is_valid_url(url)) {
                        write_url(output, url);
                        count++;
                    }
                }
            }
            
            p = url_end + 1;
        }
    }
    
    return count;
}

/**
 * Extract meta refresh URLs
 */
static int extract_meta_refresh_urls(const char* html, const char* base_url __attribute__((unused)), FILE* output) {
    int count = 0;
    const char* p = html;
    
    while ((p = strstr(p, "<meta")) != NULL) {
        // Check if it's a refresh meta tag
        const char* refresh = strstr(p, "http-equiv");
        if (refresh && refresh < p + 200) {
            const char* url_marker = strstr(refresh, "url=");
            if (url_marker && url_marker < refresh + 200) {
                const char* url_start = url_marker + 4;
                const char* url_end = strchr(url_start, '"');
                if (!url_end) url_end = strchr(url_start, '\'');
                if (!url_end) url_end = strchr(url_start, '>');
                
                if (url_end && url_end > url_start) {
                    size_t url_len = url_end - url_start;
                    if (url_len < 2048) {
                        char url[2048];
                        strncpy(url, url_start, url_len);
                        url[url_len] = '\0';
                        
                        if (is_valid_url(url)) {
                            write_url(output, url);
                            count++;
                        }
                    }
                }
            }
        }
        
        p++;
    }
    
    return count;
}

/**
 * Extract URLs from specific pattern type
 */
int extract_urls_pattern(const char* html, const char* base_url,
                        URLPatternType pattern,
                        const char* output_file) {
    if (!html || !output_file) return -1;
    
    FILE* fp = fopen(output_file, "a");
    if (!fp) return -1;
    
    int count = 0;
    
    switch (pattern) {
        case URL_PATTERN_HREF:
            count = extract_href_urls(html, base_url, fp);
            break;
        case URL_PATTERN_ONCLICK:
            count = extract_onclick_urls(html, base_url, fp);
            break;
        case URL_PATTERN_DATA_ATTR:
            count = extract_data_attr_urls(html, base_url, fp);
            break;
        case URL_PATTERN_META_REFRESH:
            count = extract_meta_refresh_urls(html, base_url, fp);
            break;
        default:
            break;
    }
    
    fclose(fp);
    return count;
}

/**
 * Extract URLs using configured patterns
 */
int extract_urls_advanced(const char* html, const char* base_url,
                          const URLPatternConfig* config,
                          const char* output_file) {
    if (!html || !output_file) return -1;
    
    URLPatternConfig default_config;
    if (!config) {
        url_pattern_config_init_default(&default_config);
        config = &default_config;
    }
    
    int total_count = 0;
    
    if (config->enable_href) {
        int count = extract_urls_pattern(html, base_url, URL_PATTERN_HREF, output_file);
        if (count > 0) total_count += count;
    }
    
    if (config->enable_onclick) {
        int count = extract_urls_pattern(html, base_url, URL_PATTERN_ONCLICK, output_file);
        if (count > 0) total_count += count;
    }
    
    if (config->enable_data_attr) {
        int count = extract_urls_pattern(html, base_url, URL_PATTERN_DATA_ATTR, output_file);
        if (count > 0) total_count += count;
    }
    
    if (config->enable_meta_refresh) {
        int count = extract_urls_pattern(html, base_url, URL_PATTERN_META_REFRESH, output_file);
        if (count > 0) total_count += count;
    }
    
    return total_count;
}
