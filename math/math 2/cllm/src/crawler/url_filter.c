/**
 * URL Filter System Implementation
 * 
 * Features:
 * - File type filtering
 * - Domain whitelist/blacklist
 * - URL pattern matching (regex)
 * - GET parameter handling
 */

#include "url_filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

// Filter structure
struct URLFilter {
    URLFilterConfig config;
    regex_t* compiled_patterns;  // Compiled regex patterns
};

/**
 * Initialize default configuration
 */
static void init_default_config(URLFilterConfig* config) {
    // Allow common file types by default
    config->allow_html = true;
    config->allow_pdf = true;
    config->allow_txt = true;
    config->allow_doc = true;
    config->allow_images = false;
    config->allow_video = false;
    config->allow_audio = false;
    config->allow_archives = false;
    config->allow_code = false;
    config->allow_data = false;
    
    // Empty lists
    config->domain_whitelist = NULL;
    config->whitelist_count = 0;
    config->domain_blacklist = NULL;
    config->blacklist_count = 0;
    config->url_patterns = NULL;
    config->pattern_count = 0;
    
    // Preserve query params by default
    config->preserve_query_params = true;
    config->strip_tracking_params = true;
    config->tracking_param_names = NULL;
    config->tracking_param_count = 0;
}

/**
 * Create filter
 */
URLFilter* url_filter_create(URLFilterConfig* config) {
    URLFilter* filter = (URLFilter*)calloc(1, sizeof(URLFilter));
    if (!filter) return NULL;
    
    if (config) {
        // Copy provided config
        memcpy(&filter->config, config, sizeof(URLFilterConfig));
        
        // Deep copy string arrays
        if (config->domain_whitelist && config->whitelist_count > 0) {
            filter->config.domain_whitelist = (char**)calloc(config->whitelist_count, sizeof(char*));
            for (int i = 0; i < config->whitelist_count; i++) {
                filter->config.domain_whitelist[i] = strdup(config->domain_whitelist[i]);
            }
        }
        
        if (config->domain_blacklist && config->blacklist_count > 0) {
            filter->config.domain_blacklist = (char**)calloc(config->blacklist_count, sizeof(char*));
            for (int i = 0; i < config->blacklist_count; i++) {
                filter->config.domain_blacklist[i] = strdup(config->domain_blacklist[i]);
            }
        }
        
        if (config->url_patterns && config->pattern_count > 0) {
            filter->config.url_patterns = (char**)calloc(config->pattern_count, sizeof(char*));
            filter->compiled_patterns = (regex_t*)calloc(config->pattern_count, sizeof(regex_t));
            
            for (int i = 0; i < config->pattern_count; i++) {
                filter->config.url_patterns[i] = strdup(config->url_patterns[i]);
                
                // Compile regex
                int ret = regcomp(&filter->compiled_patterns[i], config->url_patterns[i], REG_EXTENDED | REG_NOSUB);
                if (ret != 0) {
                    fprintf(stderr, "Warning: Failed to compile regex pattern: %s\n", config->url_patterns[i]);
                }
            }
        }
        
        if (config->tracking_param_names && config->tracking_param_count > 0) {
            filter->config.tracking_param_names = (char**)calloc(config->tracking_param_count, sizeof(char*));
            for (int i = 0; i < config->tracking_param_count; i++) {
                filter->config.tracking_param_names[i] = strdup(config->tracking_param_names[i]);
            }
        }
    } else {
        // Use defaults
        init_default_config(&filter->config);
    }
    
    return filter;
}

/**
 * Destroy filter
 */
void url_filter_destroy(URLFilter* filter) {
    if (!filter) return;
    
    // Free domain whitelist
    if (filter->config.domain_whitelist) {
        for (int i = 0; i < filter->config.whitelist_count; i++) {
            free(filter->config.domain_whitelist[i]);
        }
        free(filter->config.domain_whitelist);
    }
    
    // Free domain blacklist
    if (filter->config.domain_blacklist) {
        for (int i = 0; i < filter->config.blacklist_count; i++) {
            free(filter->config.domain_blacklist[i]);
        }
        free(filter->config.domain_blacklist);
    }
    
    // Free URL patterns
    if (filter->config.url_patterns) {
        for (int i = 0; i < filter->config.pattern_count; i++) {
            free(filter->config.url_patterns[i]);
            regfree(&filter->compiled_patterns[i]);
        }
        free(filter->config.url_patterns);
        free(filter->compiled_patterns);
    }
    
    // Free tracking param names
    if (filter->config.tracking_param_names) {
        for (int i = 0; i < filter->config.tracking_param_count; i++) {
            free(filter->config.tracking_param_names[i]);
        }
        free(filter->config.tracking_param_names);
    }
    
    free(filter);
}

/**
 * Extract domain from URL
 */
static void extract_domain(const char* url, char* domain, size_t domain_size) {
    if (!url || !domain) return;
    
    domain[0] = '\0';
    
    // Find protocol end
    const char* proto_end = strstr(url, "://");
    if (!proto_end) return;
    
    const char* start = proto_end + 3;
    
    // Find path start
    const char* path_start = strchr(start, '/');
    
    size_t len;
    if (path_start) {
        len = path_start - start;
    } else {
        len = strlen(start);
    }
    
    if (len >= domain_size) len = domain_size - 1;
    
    strncpy(domain, start, len);
    domain[len] = '\0';
}

/**
 * Extract file type from URL
 */
static void extract_file_type(const char* url, char* file_type, size_t type_size) {
    if (!url || !file_type) return;
    
    strcpy(file_type, "html");  // Default
    
    // Find last dot in path (before query string)
    const char* query = strchr(url, '?');
    const char* path_end = query ? query : (url + strlen(url));
    
    const char* last_dot = NULL;
    for (const char* p = url; p < path_end; p++) {
        if (*p == '.') last_dot = p;
    }
    
    if (last_dot && last_dot < path_end) {
        const char* ext = last_dot + 1;
        size_t ext_len = path_end - ext;
        
        if (ext_len < type_size) {
            strncpy(file_type, ext, ext_len);
            file_type[ext_len] = '\0';
            
            // Convert to lowercase
            for (char* p = file_type; *p; p++) {
                if (*p >= 'A' && *p <= 'Z') {
                    *p = *p + ('a' - 'A');
                }
            }
        }
    }
}

/**
 * Check if file type is allowed
 */
bool url_filter_is_allowed_type(URLFilter* filter, const char* file_type) {
    if (!filter || !file_type) return false;
    
    if (strcmp(file_type, "html") == 0 || strcmp(file_type, "htm") == 0) {
        return filter->config.allow_html;
    } else if (strcmp(file_type, "pdf") == 0) {
        return filter->config.allow_pdf;
    } else if (strcmp(file_type, "txt") == 0) {
        return filter->config.allow_txt;
    } else if (strcmp(file_type, "doc") == 0 || strcmp(file_type, "docx") == 0) {
        return filter->config.allow_doc;
    } else if (strcmp(file_type, "jpg") == 0 || strcmp(file_type, "jpeg") == 0 || 
               strcmp(file_type, "png") == 0 || strcmp(file_type, "gif") == 0 ||
               strcmp(file_type, "webp") == 0 || strcmp(file_type, "svg") == 0) {
        return filter->config.allow_images;
    } else if (strcmp(file_type, "mp4") == 0 || strcmp(file_type, "avi") == 0 || 
               strcmp(file_type, "mkv") == 0 || strcmp(file_type, "webm") == 0) {
        return filter->config.allow_video;
    } else if (strcmp(file_type, "mp3") == 0 || strcmp(file_type, "wav") == 0 || 
               strcmp(file_type, "flac") == 0 || strcmp(file_type, "ogg") == 0) {
        return filter->config.allow_audio;
    } else if (strcmp(file_type, "zip") == 0 || strcmp(file_type, "tar") == 0 || 
               strcmp(file_type, "gz") == 0 || strcmp(file_type, "7z") == 0) {
        return filter->config.allow_archives;
    } else if (strcmp(file_type, "c") == 0 || strcmp(file_type, "h") == 0 || 
               strcmp(file_type, "cpp") == 0 || strcmp(file_type, "py") == 0 ||
               strcmp(file_type, "js") == 0 || strcmp(file_type, "java") == 0) {
        return filter->config.allow_code;
    } else if (strcmp(file_type, "json") == 0 || strcmp(file_type, "xml") == 0 || 
               strcmp(file_type, "csv") == 0 || strcmp(file_type, "sql") == 0) {
        return filter->config.allow_data;
    }
    
    // Unknown type - allow HTML by default
    return filter->config.allow_html;
}

/**
 * Check if domain is allowed
 */
bool url_filter_is_allowed_domain(URLFilter* filter, const char* domain) {
    if (!filter || !domain) return false;
    
    // If whitelist exists, domain must be in it
    if (filter->config.domain_whitelist && filter->config.whitelist_count > 0) {
        bool found = false;
        for (int i = 0; i < filter->config.whitelist_count; i++) {
            if (strcmp(filter->config.domain_whitelist[i], domain) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    
    // Check blacklist
    if (filter->config.domain_blacklist && filter->config.blacklist_count > 0) {
        for (int i = 0; i < filter->config.blacklist_count; i++) {
            if (strcmp(filter->config.domain_blacklist[i], domain) == 0) {
                return false;
            }
        }
    }
    
    return true;
}

/**
 * Check if URL matches any pattern
 */
bool url_filter_matches_pattern(URLFilter* filter, const char* url) {
    if (!filter || !url) return false;
    
    if (!filter->config.url_patterns || filter->config.pattern_count == 0) {
        return false;
    }
    
    for (int i = 0; i < filter->config.pattern_count; i++) {
        int ret = regexec(&filter->compiled_patterns[i], url, 0, NULL, 0);
        if (ret == 0) {
            return true;  // Match found
        }
    }
    
    return false;
}

/**
 * Check if URL should be crawled
 */
bool url_filter_should_crawl(URLFilter* filter, const char* url) {
    if (!filter || !url) return false;
    
    // Extract domain
    char domain[256];
    extract_domain(url, domain, sizeof(domain));
    
    // Check domain filter
    if (!url_filter_is_allowed_domain(filter, domain)) {
        return false;
    }
    
    // Extract file type
    char file_type[32];
    extract_file_type(url, file_type, sizeof(file_type));
    
    // Check file type filter
    if (!url_filter_is_allowed_type(filter, file_type)) {
        return false;
    }
    
    // Check patterns (if any match, block the URL)
    if (url_filter_matches_pattern(filter, url)) {
        return false;
    }
    
    return true;
}

/**
 * Add domain to whitelist
 */
int url_filter_add_domain_whitelist(URLFilter* filter, const char* domain) {
    if (!filter || !domain) return -1;
    
    // Check if already exists
    for (int i = 0; i < filter->config.whitelist_count; i++) {
        if (strcmp(filter->config.domain_whitelist[i], domain) == 0) {
            return 0;  // Already exists
        }
    }
    
    // Expand array
    char** new_list = (char**)realloc(filter->config.domain_whitelist, 
                                      (filter->config.whitelist_count + 1) * sizeof(char*));
    if (!new_list) return -1;
    
    filter->config.domain_whitelist = new_list;
    filter->config.domain_whitelist[filter->config.whitelist_count] = strdup(domain);
    filter->config.whitelist_count++;
    
    return 0;
}

/**
 * Add domain to blacklist
 */
int url_filter_add_domain_blacklist(URLFilter* filter, const char* domain) {
    if (!filter || !domain) return -1;
    
    // Check if already exists
    for (int i = 0; i < filter->config.blacklist_count; i++) {
        if (strcmp(filter->config.domain_blacklist[i], domain) == 0) {
            return 0;  // Already exists
        }
    }
    
    // Expand array
    char** new_list = (char**)realloc(filter->config.domain_blacklist, 
                                      (filter->config.blacklist_count + 1) * sizeof(char*));
    if (!new_list) return -1;
    
    filter->config.domain_blacklist = new_list;
    filter->config.domain_blacklist[filter->config.blacklist_count] = strdup(domain);
    filter->config.blacklist_count++;
    
    return 0;
}

/**
 * Add URL pattern
 */
int url_filter_add_pattern(URLFilter* filter, const char* pattern) {
    if (!filter || !pattern) return -1;
    
    // Expand arrays
    char** new_patterns = (char**)realloc(filter->config.url_patterns, 
                                          (filter->config.pattern_count + 1) * sizeof(char*));
    if (!new_patterns) return -1;
    
    regex_t* new_compiled = (regex_t*)realloc(filter->compiled_patterns, 
                                              (filter->config.pattern_count + 1) * sizeof(regex_t));
    if (!new_compiled) {
        free(new_patterns);
        return -1;
    }
    
    filter->config.url_patterns = new_patterns;
    filter->compiled_patterns = new_compiled;
    
    // Add pattern
    filter->config.url_patterns[filter->config.pattern_count] = strdup(pattern);
    
    // Compile regex
    int ret = regcomp(&filter->compiled_patterns[filter->config.pattern_count], 
                     pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        fprintf(stderr, "Warning: Failed to compile regex pattern: %s\n", pattern);
        free(filter->config.url_patterns[filter->config.pattern_count]);
        return -1;
    }
    
    filter->config.pattern_count++;
    return 0;
}

/**
 * Remove domain from whitelist
 */
int url_filter_remove_domain_whitelist(URLFilter* filter, const char* domain) {
    if (!filter || !domain) return -1;
    
    for (int i = 0; i < filter->config.whitelist_count; i++) {
        if (strcmp(filter->config.domain_whitelist[i], domain) == 0) {
            free(filter->config.domain_whitelist[i]);
            
            // Shift remaining elements
            for (int j = i; j < filter->config.whitelist_count - 1; j++) {
                filter->config.domain_whitelist[j] = filter->config.domain_whitelist[j + 1];
            }
            
            filter->config.whitelist_count--;
            return 0;
        }
    }
    
    return -1;  // Not found
}

/**
 * Remove domain from blacklist
 */
int url_filter_remove_domain_blacklist(URLFilter* filter, const char* domain) {
    if (!filter || !domain) return -1;
    
    for (int i = 0; i < filter->config.blacklist_count; i++) {
        if (strcmp(filter->config.domain_blacklist[i], domain) == 0) {
            free(filter->config.domain_blacklist[i]);
            
            // Shift remaining elements
            for (int j = i; j < filter->config.blacklist_count - 1; j++) {
                filter->config.domain_blacklist[j] = filter->config.domain_blacklist[j + 1];
            }
            
            filter->config.blacklist_count--;
            return 0;
        }
    }
    
    return -1;  // Not found
}

/**
 * Remove URL pattern
 */
int url_filter_remove_pattern(URLFilter* filter, const char* pattern) {
    if (!filter || !pattern) return -1;
    
    for (int i = 0; i < filter->config.pattern_count; i++) {
        if (strcmp(filter->config.url_patterns[i], pattern) == 0) {
            free(filter->config.url_patterns[i]);
            regfree(&filter->compiled_patterns[i]);
            
            // Shift remaining elements
            for (int j = i; j < filter->config.pattern_count - 1; j++) {
                filter->config.url_patterns[j] = filter->config.url_patterns[j + 1];
                filter->compiled_patterns[j] = filter->compiled_patterns[j + 1];
            }
            
            filter->config.pattern_count--;
            return 0;
        }
    }
    
    return -1;  // Not found
}

/**
 * Get filter configuration
 */
URLFilterConfig* url_filter_get_config(URLFilter* filter) {
    if (!filter) return NULL;
    return &filter->config;
}

/**
 * Set file type filter
 */
int url_filter_set_type(URLFilter* filter, const char* file_type, bool allowed) {
    if (!filter || !file_type) return -1;
    
    if (strcmp(file_type, "html") == 0) {
        filter->config.allow_html = allowed;
    } else if (strcmp(file_type, "pdf") == 0) {
        filter->config.allow_pdf = allowed;
    } else if (strcmp(file_type, "txt") == 0) {
        filter->config.allow_txt = allowed;
    } else if (strcmp(file_type, "doc") == 0) {
        filter->config.allow_doc = allowed;
    } else if (strcmp(file_type, "images") == 0) {
        filter->config.allow_images = allowed;
    } else if (strcmp(file_type, "video") == 0) {
        filter->config.allow_video = allowed;
    } else if (strcmp(file_type, "audio") == 0) {
        filter->config.allow_audio = allowed;
    } else if (strcmp(file_type, "archives") == 0) {
        filter->config.allow_archives = allowed;
    } else if (strcmp(file_type, "code") == 0) {
        filter->config.allow_code = allowed;
    } else if (strcmp(file_type, "data") == 0) {
        filter->config.allow_data = allowed;
    } else {
        return -1;  // Unknown type
    }
    
    return 0;
}

/**
 * Save filter configuration to file
 */
int url_filter_save(URLFilter* filter, const char* file_path) {
    if (!filter || !file_path) return -1;
    
    FILE* fp = fopen(file_path, "w");
    if (!fp) return -1;
    
    // Save file type filters
    fprintf(fp, "[file_types]\n");
    fprintf(fp, "html=%d\n", filter->config.allow_html);
    fprintf(fp, "pdf=%d\n", filter->config.allow_pdf);
    fprintf(fp, "txt=%d\n", filter->config.allow_txt);
    fprintf(fp, "doc=%d\n", filter->config.allow_doc);
    fprintf(fp, "images=%d\n", filter->config.allow_images);
    fprintf(fp, "video=%d\n", filter->config.allow_video);
    fprintf(fp, "audio=%d\n", filter->config.allow_audio);
    fprintf(fp, "archives=%d\n", filter->config.allow_archives);
    fprintf(fp, "code=%d\n", filter->config.allow_code);
    fprintf(fp, "data=%d\n", filter->config.allow_data);
    
    // Save domain whitelist
    fprintf(fp, "\n[domain_whitelist]\n");
    for (int i = 0; i < filter->config.whitelist_count; i++) {
        fprintf(fp, "%s\n", filter->config.domain_whitelist[i]);
    }
    
    // Save domain blacklist
    fprintf(fp, "\n[domain_blacklist]\n");
    for (int i = 0; i < filter->config.blacklist_count; i++) {
        fprintf(fp, "%s\n", filter->config.domain_blacklist[i]);
    }
    
    // Save URL patterns
    fprintf(fp, "\n[url_patterns]\n");
    for (int i = 0; i < filter->config.pattern_count; i++) {
        fprintf(fp, "%s\n", filter->config.url_patterns[i]);
    }
    
    // Save query param settings
    fprintf(fp, "\n[query_params]\n");
    fprintf(fp, "preserve=%d\n", filter->config.preserve_query_params);
    fprintf(fp, "strip_tracking=%d\n", filter->config.strip_tracking_params);
    
    fclose(fp);
    return 0;
}

/**
 * Load filter configuration from file
 */
int url_filter_load(URLFilter* filter, const char* file_path) {
    if (!filter || !file_path) return -1;
    
    FILE* fp = fopen(file_path, "r");
    if (!fp) return -1;
    
    char line[1024];
    char section[64] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Skip empty lines
        if (line[0] == '\0') continue;
        
        // Check for section header
        if (line[0] == '[') {
            char* end = strchr(line, ']');
            if (end) {
                *end = '\0';
                strncpy(section, line + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0'; // Ensure null termination
                section[sizeof(section) - 1] = '\0'; // Ensure null termination
                section[sizeof(section) - 1] = '\0'; // Ensure null termination
                section[sizeof(section) - 1] = '\0'; // Ensure null termination
            }
            continue;
        }
        
        // Parse based on section
        if (strcmp(section, "file_types") == 0) {
            char* eq = strchr(line, '=');
            if (eq) {
                *eq = '\0';
                char* key = line;
                int value = atoi(eq + 1);
                url_filter_set_type(filter, key, value != 0);
            }
        } else if (strcmp(section, "domain_whitelist") == 0) {
            url_filter_add_domain_whitelist(filter, line);
        } else if (strcmp(section, "domain_blacklist") == 0) {
            url_filter_add_domain_blacklist(filter, line);
        } else if (strcmp(section, "url_patterns") == 0) {
            url_filter_add_pattern(filter, line);
        } else if (strcmp(section, "query_params") == 0) {
            char* eq = strchr(line, '=');
            if (eq) {
                *eq = '\0';
                char* key = line;
                int value = atoi(eq + 1);
                
                if (strcmp(key, "preserve") == 0) {
                    filter->config.preserve_query_params = (value != 0);
                } else if (strcmp(key, "strip_tracking") == 0) {
                    filter->config.strip_tracking_params = (value != 0);
                }
            }
        }
    }
    
    fclose(fp);
    return 0;
}