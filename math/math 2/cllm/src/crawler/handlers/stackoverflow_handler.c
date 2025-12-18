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

static char* extract_question_title(const char* html) {
    char* title = find_between(html, "<h1", "</h1>");
    if (title) {
        char* clean = strip_html_tags(title);
        free(title);
        return clean;
    }
    return NULL;
}

static bool stackoverflow_can_handle(const char* url) {
    if (!url) return false;
    return url_matches_pattern(url, "stackoverflow.com") ||
           url_matches_pattern(url, "stackexchange.com");
}

static int stackoverflow_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    
    size_t buffer_size = 12288;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== Stack Overflow Question ===\n\n");
    
    char* title = extract_question_title(html);
    if (title) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Question: %s\n\n", title);
        free(title);
    }
    
    pos += snprintf(buffer + pos, buffer_size - pos, "URL: %s\n\n", url);
    
    *output = buffer;
    *output_len = pos;
    return 0;
}

static void stackoverflow_cleanup(void) {
}

static const SiteHandler stackoverflow_handler = {
    .name = "stackoverflow",
    .url_pattern = "stackoverflow.com|stackexchange.com",
    .can_handle = stackoverflow_can_handle,
    .extract = stackoverflow_extract,
    .cleanup = stackoverflow_cleanup,
    .user_data = NULL
};

const SiteHandler* get_stackoverflow_handler(void) {
    return &stackoverflow_handler;
}
