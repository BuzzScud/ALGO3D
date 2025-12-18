// content_filter.h - Smart content extraction for web crawler
#ifndef CONTENT_FILTER_H
#define CONTENT_FILTER_H

#include <stddef.h>

/**
 * Extraction modes for content filtering
 */
typedef enum {
    EXTRACT_ALL,           // Extract everything (current behavior)
    EXTRACT_HUMAN_TEXT,    // Extract main content only (articles, paragraphs)
    EXTRACT_METADATA,      // Extract metadata only (titles, dates, authors)
    EXTRACT_MIXED          // Extract content + metadata, skip boilerplate
} ExtractionMode;

/**
 * Content type classification
 */
typedef enum {
    CONTENT_MAIN,          // Main article content
    CONTENT_NAVIGATION,    // Nav, menus
    CONTENT_BOILERPLATE,   // Headers, footers
    CONTENT_SIDEBAR,       // Aside, sidebars
    CONTENT_METADATA,      // Titles, dates, authors
    CONTENT_UNKNOWN        // Unclassified
} ContentType;

/**
 * Classify HTML element based on tag, class, and ID
 */
ContentType classify_html_element(const char* tag, const char* class_attr, const char* id_attr);

/**
 * Extract content from HTML based on extraction mode
 * 
 * @param html Input HTML content
 * @param output Output buffer for extracted text
 * @param output_size Size of output buffer
 * @param mode Extraction mode
 * @return 0 on success, -1 on error
 */
int extract_content_smart(const char* html, char* output, size_t output_size, ExtractionMode mode);

/**
 * Check if a class or ID name indicates boilerplate content
 */
int is_boilerplate_name(const char* name);

/**
 * Check if a class or ID name indicates navigation
 */
int is_navigation_name(const char* name);

/**
 * Check if a class or ID name indicates metadata
 */
int is_metadata_name(const char* name);

#endif // CONTENT_FILTER_H
