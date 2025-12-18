#include "../site_handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* find_between(const char* html, const char* start_marker, const char* end_marker) {
    if (!html || !start_marker || !end_marker) return NULL;
    const char* start = strstr(html, start_marker);
    if (!start) return NULL;
    start += strlen(start_marker);
    const char* end = strstr(start, end_marker);
    if (!end) return NULL;
    size_t len = end - start;
    char* result = malloc(len + 1);
    if (!result) return NULL;
    strncpy(result, start, len);
    result[len] = '\0';
    return result;
}

static char* strip_html_tags(const char* html) {
    if (!html) return NULL;
    char* clean = malloc(strlen(html) + 1);
    if (!clean) return NULL;
    const char* src = html;
    char* dst = clean;
    bool in_tag = false;
    while (*src) {
        if (*src == '<') in_tag = true;
        else if (*src == '>') in_tag = false;
        else if (!in_tag) *dst++ = *src;
        src++;
    }
    *dst = '\0';
    return clean;
}

static char* extract_original_url(const char* url) {
    // Extract original URL from Wayback Machine URL
    // Format: https://web.archive.org/web/TIMESTAMP/ORIGINAL_URL
    const char* web_prefix = "web.archive.org/web/";
    const char* start = strstr(url, web_prefix);
    if (!start) return NULL;
    
    start += strlen(web_prefix);
    // Skip timestamp (14 digits)
    while (*start && isdigit(*start)) start++;
    if (*start == '/') start++;
    
    if (*start) {
        char* original = strdup(start);
        return original;
    }
    return NULL;
}

static char* extract_archive_date(const char* url) {
    // Extract date from Wayback Machine URL
    // Format: https://web.archive.org/web/YYYYMMDDHHMMSS/URL
    const char* web_prefix = "web.archive.org/web/";
    const char* start = strstr(url, web_prefix);
    if (!start) return NULL;
    
    start += strlen(web_prefix);
    char* date = malloc(15);
    if (!date) return NULL;
    
    int i = 0;
    while (*start && isdigit(*start) && i < 14) {
        date[i++] = *start++;
    }
    date[i] = '\0';
    
    if (i >= 8) {
        // Format as YYYY-MM-DD
        char* formatted = malloc(20);
        if (formatted) {
            snprintf(formatted, 20, "%.4s-%.2s-%.2s", date, date+4, date+6);
            free(date);
            return formatted;
        }
    }
    
    return date;
}

static char* extract_page_title(const char* html) {
    char* title = find_between(html, "<title>", "</title>");
    if (title) {
        char* clean = strip_html_tags(title);
        free(title);
        return clean;
    }
    return NULL;
}

static char* extract_page_content(const char* html) {
    char* content = find_between(html, "<body", "</body>");
    if (content) {
        char* clean = strip_html_tags(content);
        free(content);
        return clean;
    }
    return NULL;
}

static bool archive_can_handle(const char* url) {
    if (!url) return false;
    return url_matches_pattern(url, "archive.org") ||
           url_matches_pattern(url, "web.archive.org");
}

static int archive_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    
    size_t buffer_size = 16384;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== Archive.org Archived Page ===\n\n");
    
    char* original_url = extract_original_url(url);
    if (original_url) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Original URL: %s\n", original_url);
        free(original_url);
    }
    
    char* archive_date = extract_archive_date(url);
    if (archive_date) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Archive Date: %s\n", archive_date);
        free(archive_date);
    }
    
    pos += snprintf(buffer + pos, buffer_size - pos, "Archive URL: %s\n\n", url);
    
    char* title = extract_page_title(html);
    if (title) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Page Title: %s\n\n", title);
        free(title);
    }
    
    char* content = extract_page_content(html);
    if (content) {
        size_t content_len = strlen(content);
        size_t max_content = buffer_size - pos - 100;
        if (content_len > max_content) {
            content[max_content] = '\0';
            pos += snprintf(buffer + pos, buffer_size - pos, "Content:\n%s\n[Truncated]\n\n", content);
        } else {
            pos += snprintf(buffer + pos, buffer_size - pos, "Content:\n%s\n\n", content);
        }
        free(content);
    }
    
    pos += snprintf(buffer + pos, buffer_size - pos, 
        "Note: This is an archived snapshot from the Wayback Machine.\n");
    
    *output = buffer;
    *output_len = pos;
    return 0;
}

static void archive_cleanup(void) {
}

static const SiteHandler archive_handler = {
    .name = "archive",
    .url_pattern = "archive.org",
    .can_handle = archive_can_handle,
    .extract = archive_extract,
    .cleanup = archive_cleanup,
    .user_data = NULL
};

const SiteHandler* get_archive_handler(void) {
    return &archive_handler;
}
