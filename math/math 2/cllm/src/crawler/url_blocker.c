/**
 * URL Blocker System Implementation
 * 
 * Supports multiple blocking strategies with regex patterns
 */

#include "url_blocker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_PATTERNS 1000

// Blocker structure
struct URLBlocker {
    BlockPattern* patterns;
    regex_t* compiled_regex;  // Compiled regex patterns
    int pattern_count;
    int pattern_capacity;
    int next_id;
    char patterns_file[1024];
};

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
 * Extract path from URL
 */
static void extract_path(const char* url, char* path, size_t path_size) {
    if (!url || !path) return;
    
    path[0] = '\0';
    
    // Find protocol end
    const char* proto_end = strstr(url, "://");
    if (!proto_end) return;
    
    // Find path start
    const char* path_start = strchr(proto_end + 3, '/');
    if (!path_start) return;
    
    // Copy path (before query string)
    const char* query = strchr(path_start, '?');
    size_t len;
    if (query) {
        len = query - path_start;
    } else {
        len = strlen(path_start);
    }
    
    if (len >= path_size) len = path_size - 1;
    
    strncpy(path, path_start, len);
    path[len] = '\0';
}

/**
 * Create blocker
 */
URLBlocker* url_blocker_create(const char* patterns_file) {
    URLBlocker* blocker = (URLBlocker*)calloc(1, sizeof(URLBlocker));
    if (!blocker) return NULL;
    
    blocker->pattern_capacity = 100;
    blocker->patterns = (BlockPattern*)calloc(blocker->pattern_capacity, sizeof(BlockPattern));
    blocker->compiled_regex = (regex_t*)calloc(blocker->pattern_capacity, sizeof(regex_t));
    
    if (!blocker->patterns || !blocker->compiled_regex) {
        if (blocker->patterns) free(blocker->patterns);
        if (blocker->compiled_regex) free(blocker->compiled_regex);
        free(blocker);
        return NULL;
    }
    
    blocker->pattern_count = 0;
    blocker->next_id = 1;
    
    if (patterns_file) {
        strncpy(blocker->patterns_file, patterns_file, sizeof(blocker->patterns_file) - 1);
        url_blocker_load(blocker);
    }
    
    return blocker;
}

/**
 * Destroy blocker
 */
void url_blocker_destroy(URLBlocker* blocker) {
    if (!blocker) return;
    
    // Free compiled regex patterns
    if (blocker->compiled_regex) {
        for (int i = 0; i < blocker->pattern_count; i++) {
            if (blocker->patterns[i].type == BLOCK_REGEX_PATTERN) {
                regfree(&blocker->compiled_regex[i]);
            }
        }
        free(blocker->compiled_regex);
    }
    
    if (blocker->patterns) {
        free(blocker->patterns);
    }
    
    free(blocker);
}

/**
 * Add block pattern
 */
int url_blocker_add_pattern(URLBlocker* blocker, BlockPatternType type, 
                            const char* pattern, const char* description) {
    if (!blocker || !pattern) return -1;
    
    // Check capacity
    if (blocker->pattern_count >= blocker->pattern_capacity) {
        int new_capacity = blocker->pattern_capacity * 2;
        if (new_capacity > MAX_PATTERNS) new_capacity = MAX_PATTERNS;
        
        if (blocker->pattern_count >= MAX_PATTERNS) {
            return -1;  // Too many patterns
        }
        
        BlockPattern* new_patterns = (BlockPattern*)realloc(blocker->patterns, 
                                                            new_capacity * sizeof(BlockPattern));
        regex_t* new_regex = (regex_t*)realloc(blocker->compiled_regex, 
                                               new_capacity * sizeof(regex_t));
        
        if (!new_patterns || !new_regex) {
            if (new_patterns) free(new_patterns);
            if (new_regex) free(new_regex);
            return -1;
        }
        
        blocker->patterns = new_patterns;
        blocker->compiled_regex = new_regex;
        blocker->pattern_capacity = new_capacity;
    }
    
    // Add pattern
    BlockPattern* p = &blocker->patterns[blocker->pattern_count];
    p->id = blocker->next_id++;
    p->type = type;
    strncpy(p->pattern, pattern, sizeof(p->pattern) - 1);
    p->pattern[sizeof(p->pattern) - 1] = '\0';
    
    if (description) {
        strncpy(p->description, description, sizeof(p->description) - 1);
        p->description[sizeof(p->description) - 1] = '\0';
    } else {
        p->description[0] = '\0';
    }
    
    p->added_time = time(NULL);
    p->enabled = true;
    
    // Compile regex if needed
    if (type == BLOCK_REGEX_PATTERN) {
        int ret = regcomp(&blocker->compiled_regex[blocker->pattern_count], 
                         pattern, REG_EXTENDED | REG_NOSUB);
        if (ret != 0) {
            fprintf(stderr, "Warning: Failed to compile regex pattern: %s\n", pattern);
            return -1;
        }
    }
    
    blocker->pattern_count++;
    
    // Auto-save if file is set
    if (blocker->patterns_file[0] != '\0') {
        url_blocker_save(blocker);
    }
    
    return p->id;
}

/**
 * Remove block pattern
 */
int url_blocker_remove_pattern(URLBlocker* blocker, int pattern_id) {
    if (!blocker) return -1;
    
    for (int i = 0; i < blocker->pattern_count; i++) {
        if (blocker->patterns[i].id == pattern_id) {
            // Free regex if needed
            if (blocker->patterns[i].type == BLOCK_REGEX_PATTERN) {
                regfree(&blocker->compiled_regex[i]);
            }
            
            // Shift remaining patterns
            for (int j = i; j < blocker->pattern_count - 1; j++) {
                blocker->patterns[j] = blocker->patterns[j + 1];
                if (blocker->patterns[j].type == BLOCK_REGEX_PATTERN) {
                    blocker->compiled_regex[j] = blocker->compiled_regex[j + 1];
                }
            }
            
            blocker->pattern_count--;
            
            // Auto-save
            if (blocker->patterns_file[0] != '\0') {
                url_blocker_save(blocker);
            }
            
            return 0;
        }
    }
    
    return -1;  // Not found
}

/**
 * Enable/disable pattern
 */
int url_blocker_set_pattern_enabled(URLBlocker* blocker, int pattern_id, bool enabled) {
    if (!blocker) return -1;
    
    for (int i = 0; i < blocker->pattern_count; i++) {
        if (blocker->patterns[i].id == pattern_id) {
            blocker->patterns[i].enabled = enabled;
            
            // Auto-save
            if (blocker->patterns_file[0] != '\0') {
                url_blocker_save(blocker);
            }
            
            return 0;
        }
    }
    
    return -1;  // Not found
}

/**
 * Check if URL matches pattern
 */
static bool matches_pattern(URLBlocker* blocker, int pattern_idx, const char* url) {
    BlockPattern* p = &blocker->patterns[pattern_idx];
    
    if (!p->enabled) return false;
    
    switch (p->type) {
        case BLOCK_EXACT_URL:
            return strcmp(url, p->pattern) == 0;
            
        case BLOCK_DOMAIN: {
            char domain[256];
            extract_domain(url, domain, sizeof(domain));
            return strcmp(domain, p->pattern) == 0;
        }
        
        case BLOCK_PATH_PREFIX: {
            char path[2048];
            extract_path(url, path, sizeof(path));
            return strncmp(path, p->pattern, strlen(p->pattern)) == 0;
        }
        
        case BLOCK_REGEX_PATTERN: {
            int ret = regexec(&blocker->compiled_regex[pattern_idx], url, 0, NULL, 0);
            return (ret == 0);
        }
        
        default:
            return false;
    }
}

/**
 * Check if URL is blocked
 */
bool url_blocker_is_blocked(URLBlocker* blocker, const char* url) {
    if (!blocker || !url) return false;
    
    for (int i = 0; i < blocker->pattern_count; i++) {
        if (matches_pattern(blocker, i, url)) {
            return true;
        }
    }
    
    return false;
}

/**
 * Get all patterns
 */
BlockPattern** url_blocker_get_patterns(URLBlocker* blocker, int* count) {
    if (!blocker || !count) return NULL;
    
    *count = blocker->pattern_count;
    
    if (blocker->pattern_count == 0) return NULL;
    
    BlockPattern** patterns = (BlockPattern**)malloc(blocker->pattern_count * sizeof(BlockPattern*));
    if (!patterns) return NULL;
    
    for (int i = 0; i < blocker->pattern_count; i++) {
        patterns[i] = &blocker->patterns[i];
    }
    
    return patterns;
}

/**
 * Get pattern by ID
 */
BlockPattern* url_blocker_get_pattern(URLBlocker* blocker, int pattern_id) {
    if (!blocker) return NULL;
    
    for (int i = 0; i < blocker->pattern_count; i++) {
        if (blocker->patterns[i].id == pattern_id) {
            return &blocker->patterns[i];
        }
    }
    
    return NULL;
}

/**
 * Test pattern against URL
 */
bool url_blocker_test_pattern(URLBlocker* blocker, const char* pattern, 
                              BlockPatternType type, const char* test_url) {
    if (!blocker || !pattern || !test_url) return false;
    
    // Create temporary pattern
    BlockPattern temp;
    temp.type = type;
    strncpy(temp.pattern, pattern, sizeof(temp.pattern) - 1);
    temp.enabled = true;
    
    regex_t temp_regex;
    bool regex_compiled = false;
    
    if (type == BLOCK_REGEX_PATTERN) {
        int ret = regcomp(&temp_regex, pattern, REG_EXTENDED | REG_NOSUB);
        if (ret != 0) {
            return false;  // Invalid regex
        }
        regex_compiled = true;
    }
    
    // Test match
    bool result = false;
    
    switch (type) {
        case BLOCK_EXACT_URL:
            result = (strcmp(test_url, pattern) == 0);
            break;
            
        case BLOCK_DOMAIN: {
            char domain[256];
            extract_domain(test_url, domain, sizeof(domain));
            result = (strcmp(domain, pattern) == 0);
            break;
        }
        
        case BLOCK_PATH_PREFIX: {
            char path[2048];
            extract_path(test_url, path, sizeof(path));
            result = (strncmp(path, pattern, strlen(pattern)) == 0);
            break;
        }
        
        case BLOCK_REGEX_PATTERN:
            if (regex_compiled) {
                int ret = regexec(&temp_regex, test_url, 0, NULL, 0);
                result = (ret == 0);
            }
            break;
    }
    
    if (regex_compiled) {
        regfree(&temp_regex);
    }
    
    return result;
}

/**
 * Get pattern type name
 */
const char* url_blocker_get_type_name(BlockPatternType type) {
    switch (type) {
        case BLOCK_EXACT_URL: return "exact";
        case BLOCK_DOMAIN: return "domain";
        case BLOCK_PATH_PREFIX: return "prefix";
        case BLOCK_REGEX_PATTERN: return "regex";
        default: return "unknown";
    }
}

/**
 * Parse pattern type from string
 */
int url_blocker_parse_type(const char* type_str) {
    if (!type_str) return -1;
    
    if (strcmp(type_str, "exact") == 0) return BLOCK_EXACT_URL;
    if (strcmp(type_str, "domain") == 0) return BLOCK_DOMAIN;
    if (strcmp(type_str, "prefix") == 0) return BLOCK_PATH_PREFIX;
    if (strcmp(type_str, "regex") == 0) return BLOCK_REGEX_PATTERN;
    
    return -1;
}

/**
 * Save patterns to file
 */
int url_blocker_save(URLBlocker* blocker) {
    if (!blocker || blocker->patterns_file[0] == '\0') return -1;
    
    FILE* fp = fopen(blocker->patterns_file, "w");
    if (!fp) return -1;
    
    for (int i = 0; i < blocker->pattern_count; i++) {
        BlockPattern* p = &blocker->patterns[i];
        fprintf(fp, "%d|%s|%s|%s|%ld|%d\n",
                p->id,
                url_blocker_get_type_name(p->type),
                p->pattern,
                p->description,
                (long)p->added_time,
                p->enabled ? 1 : 0);
    }
    
    fclose(fp);
    return 0;
}

/**
 * Load patterns from file
 */
int url_blocker_load(URLBlocker* blocker) {
    if (!blocker || blocker->patterns_file[0] == '\0') return -1;
    
    FILE* fp = fopen(blocker->patterns_file, "r");
    if (!fp) return 0;  // File doesn't exist yet
    
    char line[4096];
    int loaded = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // Parse line: id|type|pattern|description|time|enabled
        char* id_str = strtok(line, "|");
        char* type_str = strtok(NULL, "|");
        char* pattern = strtok(NULL, "|");
        char* description = strtok(NULL, "|");
        char* time_str = strtok(NULL, "|");
        char* enabled_str = strtok(NULL, "|\n");
        
        if (id_str && type_str && pattern) {
            int type = url_blocker_parse_type(type_str);
            if (type >= 0) {
                int id = url_blocker_add_pattern(blocker, (BlockPatternType)type, 
                                                pattern, description);
                if (id >= 0) {
                    // Update metadata
                    BlockPattern* p = url_blocker_get_pattern(blocker, id);
                    if (p) {
                        if (time_str) p->added_time = (time_t)atol(time_str);
                        if (enabled_str) p->enabled = (atoi(enabled_str) != 0);
                    }
                    loaded++;
                }
            }
        }
    }
    
    fclose(fp);
    return loaded;
}

/**
 * Clear all patterns
 */
int url_blocker_clear(URLBlocker* blocker) {
    if (!blocker) return -1;
    
    // Free regex patterns
    for (int i = 0; i < blocker->pattern_count; i++) {
        if (blocker->patterns[i].type == BLOCK_REGEX_PATTERN) {
            regfree(&blocker->compiled_regex[i]);
        }
    }
    
    blocker->pattern_count = 0;
    blocker->next_id = 1;
    
    // Auto-save
    if (blocker->patterns_file[0] != '\0') {
        url_blocker_save(blocker);
    }
    
    return 0;
}

/**
 * Get total pattern count
 */
int url_blocker_get_count(URLBlocker* blocker) {
    if (!blocker) return 0;
    return blocker->pattern_count;
}

/**
 * Get enabled pattern count
 */
int url_blocker_get_enabled_count(URLBlocker* blocker) {
    if (!blocker) return 0;
    
    int count = 0;
    for (int i = 0; i < blocker->pattern_count; i++) {
        if (blocker->patterns[i].enabled) {
            count++;
        }
    }
    
    return count;
}