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
    bool in_bracket = false;
    
    while (*src) {
        if (*src == '<') {
            in_tag = true;
        } else if (*src == '>') {
            in_tag = false;
        } else if (*src == '[') {
            in_bracket = true;
        } else if (*src == ']') {
            in_bracket = false;
        } else if (!in_tag && !in_bracket) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
    return clean;
}

static char* extract_title(const char* html) {
    char* title = find_between(html, "<h1", "</h1>");
    if (!title) {
        title = find_between(html, "<title>", " - Wikipedia");
    }
    if (title) {
        char* clean = strip_html_tags(title);
        free(title);
        return clean;
    }
    return NULL;
}

static char* extract_content(const char* html) {
    char* content = find_between(html, "<article>", "</article>");
    if (!content) {
        content = find_between(html, "<p>", "</p>");
    }
    if (content) {
        char* clean = strip_html_tags(content);
        free(content);
        return clean;
    }
    return NULL;
}

static bool wikipedia_can_handle(const char* url) {
    if (!url) return false;
    return url_matches_pattern(url, "wikipedia.org");
}

static int wikipedia_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    
    size_t buffer_size = 16384;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== Wikipedia Article ===\n\n");
    
    char* title = extract_title(html);
    if (title) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Title: %s\n\n", title);
        free(title);
    }
    
    pos += snprintf(buffer + pos, buffer_size - pos, "URL: %s\n\n", url);
    
    char* content = extract_content(html);
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
    
    *output = buffer;
    *output_len = pos;
    return 0;
}

static void wikipedia_cleanup(void) {
}

static const SiteHandler wikipedia_handler = {
    .name = "wikipedia",
    .url_pattern = "wikipedia.org",
    .can_handle = wikipedia_can_handle,
    .extract = wikipedia_extract,
    .cleanup = wikipedia_cleanup,
    .user_data = NULL
};

const SiteHandler* get_wikipedia_handler(void) {
    return &wikipedia_handler;
}
