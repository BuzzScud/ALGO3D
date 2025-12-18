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

static char* extract_headline(const char* html) {
    char* headline = find_between(html, "<h1", "</h1>");
    if (headline) {
        char* clean = strip_html_tags(headline);
        free(headline);
        return clean;
    }
    return NULL;
}

static char* extract_article_body(const char* html) {
    char* body = find_between(html, "<article", "</article>");
    if (body) {
        char* clean = strip_html_tags(body);
        free(body);
        return clean;
    }
    return NULL;
}

static bool news_can_handle(const char* url) {
    if (!url) return false;
    const char* domains[] = {
        "cnn.com", "bbc.com", "reuters.com", "apnews.com",
        "nytimes.com", "washingtonpost.com", "theguardian.com",
        NULL
    };
    for (int i = 0; domains[i]; i++) {
        if (url_matches_pattern(url, domains[i])) return true;
    }
    return false;
}

static int news_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    
    size_t buffer_size = 16384;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== News Article ===\n\n");
    
    char* headline = extract_headline(html);
    if (headline) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Headline: %s\n\n", headline);
        free(headline);
    }
    
    pos += snprintf(buffer + pos, buffer_size - pos, "URL: %s\n\n", url);
    
    char* body = extract_article_body(html);
    if (body) {
        size_t body_len = strlen(body);
        size_t max_body = buffer_size - pos - 100;
        if (body_len > max_body) {
            body[max_body] = '\0';
            pos += snprintf(buffer + pos, buffer_size - pos, "Article: %s [Truncated]\n\n", body);
        } else {
            pos += snprintf(buffer + pos, buffer_size - pos, "Article: %s\n\n", body);
        }
        free(body);
    }
    
    *output = buffer;
    *output_len = pos;
    return 0;
}

static void news_cleanup(void) {
}

static const SiteHandler news_handler = {
    .name = "news",
    .url_pattern = "news|cnn|bbc|reuters",
    .can_handle = news_can_handle,
    .extract = news_extract,
    .cleanup = news_cleanup,
    .user_data = NULL
};

const SiteHandler* get_news_handler(void) {
    return &news_handler;
}
