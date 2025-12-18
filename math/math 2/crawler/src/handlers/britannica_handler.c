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
        if (*src == '<') {
            in_tag = true;
        } else if (*src == '>') {
            in_tag = false;
        } else if (!in_tag) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
    return clean;
}

static char* extract_title(const char* html) {
    char* title = find_between(html, "<h1", "</h1>");
    if (title) {
        char* clean = strip_html_tags(title);
        free(title);
        return clean;
    }
    return NULL;
}

static char* extract_article_body(const char* html) {
    char* body = find_between(html, "<article>", "</article>");
    if (body) {
        char* clean = strip_html_tags(body);
        free(body);
        return clean;
    }
    return NULL;
}

static bool britannica_can_handle(const char* url) {
    if (!url) return false;
    return url_matches_pattern(url, "britannica.com");
}

static int britannica_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    size_t buffer_size = 8192;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== Britannica Encyclopedia ===\n\n");
    char* title = extract_title(html);
    if (title) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Title: %s\n\n", title);
        free(title);
    }
    pos += snprintf(buffer + pos, buffer_size - pos, "URL: %s\n\n", url);
    char* body = extract_article_body(html);
    if (body) {
        size_t body_len = strlen(body);
        size_t max_body = buffer_size - pos - 100;
        if (body_len > max_body) {
            body[max_body] = '\0';
        }
        pos += snprintf(buffer + pos, buffer_size - pos, "Content:\n%s\n\n", body);
        free(body);
    }
    *output = buffer;
    *output_len = pos;
    return 0;
}

static void britannica_cleanup(void) {
}

static const SiteHandler britannica_handler = {
    .name = "britannica",
    .url_pattern = "britannica.com",
    .can_handle = britannica_can_handle,
    .extract = britannica_extract,
    .cleanup = britannica_cleanup,
    .user_data = NULL
};

const SiteHandler* get_britannica_handler(void) {
    return &britannica_handler;
}
