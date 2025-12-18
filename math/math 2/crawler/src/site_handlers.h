#ifndef SITE_HANDLERS_H
#define SITE_HANDLERS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Site Handler Framework
 * 
 * Provides a plugin-based system for site-specific content extraction.
 * Each site can have a custom handler that knows how to extract content
 * from that specific site's HTML structure.
 */

// Forward declaration
typedef struct SiteHandler SiteHandler;

/**
 * Handler callback function types
 */

// Check if this handler can process the given URL
typedef bool (*CanHandleFunc)(const char* url);

// Extract content from HTML
// Returns 0 on success, -1 on error
// output should be allocated by the handler and freed by caller
typedef int (*ExtractFunc)(const char* html, const char* url, char** output, size_t* output_len);

// Cleanup handler resources (optional)
typedef void (*CleanupFunc)(void);

/**
 * Site Handler Structure
 * 
 * Defines a handler for a specific website or group of websites.
 */
struct SiteHandler {
    const char* name;              // Handler name (e.g., "twitter", "britannica")
    const char* url_pattern;       // URL pattern for matching (e.g., "twitter.com", "x.com")
    CanHandleFunc can_handle;      // Function to check if handler applies
    ExtractFunc extract;           // Function to extract content
    CleanupFunc cleanup;           // Optional cleanup function
    void* user_data;               // Optional handler-specific data
};

/**
 * Handler Registry
 * 
 * Manages the collection of registered site handlers.
 */

// Initialize the handler registry
void site_handlers_init(void);

// Register a new site handler
// Returns 0 on success, -1 on error
int site_handlers_register(const SiteHandler* handler);

// Find a handler for the given URL
// Returns handler pointer or NULL if no handler found
const SiteHandler* site_handlers_find(const char* url);

// Get handler by name
// Returns handler pointer or NULL if not found
const SiteHandler* site_handlers_get(const char* name);

// List all registered handlers
// Returns number of handlers, fills handlers array up to max_handlers
int site_handlers_list(const SiteHandler** handlers, int max_handlers);

// Cleanup all handlers and free registry
void site_handlers_cleanup(void);

/**
 * Utility Functions
 */

// Check if URL matches pattern (simple substring match)
bool url_matches_pattern(const char* url, const char* pattern);

// Extract domain from URL
// Returns 0 on success, -1 on error
int extract_domain(const char* url, char* domain, size_t domain_size);

#endif // SITE_HANDLERS_H
