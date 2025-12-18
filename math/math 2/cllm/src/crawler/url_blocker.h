#ifndef URL_BLOCKER_H
#define URL_BLOCKER_H

#include <stdbool.h>
#include <time.h>

/**
 * URL Blocker System
 * 
 * Features:
 * - Block exact URLs
 * - Block entire domains
 * - Block path prefixes
 * - Block using regex patterns
 * - Pattern testing
 * - Persistent storage
 */

// Block pattern types
typedef enum {
    BLOCK_EXACT_URL,        // Block exact URL match
    BLOCK_DOMAIN,           // Block entire domain
    BLOCK_PATH_PREFIX,      // Block URLs starting with path
    BLOCK_REGEX_PATTERN     // Block using regex pattern
} BlockPatternType;

// Block pattern
typedef struct {
    int id;
    BlockPatternType type;
    char pattern[2048];
    char description[256];
    time_t added_time;
    bool enabled;
} BlockPattern;

// Blocker handle
typedef struct URLBlocker URLBlocker;

/**
 * Create blocker with patterns file
 * 
 * @param patterns_file Path to patterns file (NULL for no persistence)
 * @return Blocker handle or NULL on error
 */
URLBlocker* url_blocker_create(const char* patterns_file);

/**
 * Destroy blocker and free resources
 * 
 * @param blocker Blocker handle
 */
void url_blocker_destroy(URLBlocker* blocker);

/**
 * Add block pattern
 * 
 * @param blocker Blocker handle
 * @param type Pattern type
 * @param pattern Pattern string
 * @param description Human-readable description
 * @return Pattern ID on success, -1 on error
 */
int url_blocker_add_pattern(URLBlocker* blocker, BlockPatternType type, 
                            const char* pattern, const char* description);

/**
 * Remove block pattern
 * 
 * @param blocker Blocker handle
 * @param pattern_id Pattern ID to remove
 * @return 0 on success, -1 on error
 */
int url_blocker_remove_pattern(URLBlocker* blocker, int pattern_id);

/**
 * Enable/disable pattern
 * 
 * @param blocker Blocker handle
 * @param pattern_id Pattern ID
 * @param enabled true to enable, false to disable
 * @return 0 on success, -1 on error
 */
int url_blocker_set_pattern_enabled(URLBlocker* blocker, int pattern_id, bool enabled);

/**
 * Check if URL is blocked
 * 
 * @param blocker Blocker handle
 * @param url URL to check
 * @return true if blocked, false otherwise
 */
bool url_blocker_is_blocked(URLBlocker* blocker, const char* url);

/**
 * Get all patterns
 * 
 * @param blocker Blocker handle
 * @param count Output: number of patterns
 * @return Array of patterns (do not free individual patterns, free array only)
 */
BlockPattern** url_blocker_get_patterns(URLBlocker* blocker, int* count);

/**
 * Get pattern by ID
 * 
 * @param blocker Blocker handle
 * @param pattern_id Pattern ID
 * @return Pattern or NULL if not found (do not free)
 */
BlockPattern* url_blocker_get_pattern(URLBlocker* blocker, int pattern_id);

/**
 * Test pattern against URL
 * 
 * Tests if a pattern would block a URL without actually blocking it
 * 
 * @param blocker Blocker handle
 * @param pattern Pattern string
 * @param type Pattern type
 * @param test_url URL to test against
 * @return true if pattern would block URL, false otherwise
 */
bool url_blocker_test_pattern(URLBlocker* blocker, const char* pattern, 
                              BlockPatternType type, const char* test_url);

/**
 * Get pattern type name
 * 
 * @param type Pattern type
 * @return Type name string
 */
const char* url_blocker_get_type_name(BlockPatternType type);

/**
 * Parse pattern type from string
 * 
 * @param type_str Type string ("exact", "domain", "prefix", "regex")
 * @return Pattern type or -1 if invalid
 */
int url_blocker_parse_type(const char* type_str);

/**
 * Save patterns to file
 * 
 * @param blocker Blocker handle
 * @return 0 on success, -1 on error
 */
int url_blocker_save(URLBlocker* blocker);

/**
 * Load patterns from file
 * 
 * @param blocker Blocker handle
 * @return Number of patterns loaded, -1 on error
 */
int url_blocker_load(URLBlocker* blocker);

/**
 * Clear all patterns
 * 
 * @param blocker Blocker handle
 * @return 0 on success, -1 on error
 */
int url_blocker_clear(URLBlocker* blocker);

/**
 * Get total pattern count
 * 
 * @param blocker Blocker handle
 * @return Number of patterns
 */
int url_blocker_get_count(URLBlocker* blocker);

/**
 * Get enabled pattern count
 * 
 * @param blocker Blocker handle
 * @return Number of enabled patterns
 */
int url_blocker_get_enabled_count(URLBlocker* blocker);

#endif // URL_BLOCKER_H