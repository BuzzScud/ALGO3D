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

static char* extract_post_title(const char* html) {
    char* title = find_between(html, "<h1", "</h1>");
    if (title) {
        char* clean = strip_html_tags(title);
        free(title);
        return clean;
    }
    return NULL;
}

static char* extract_subreddit(const char* url) {
    char* subreddit = find_between(url, "/r/", "/");
    return subreddit;
}

static bool reddit_can_handle(const char* url) {
    if (!url) return false;
    return url_matches_pattern(url, "reddit.com");
}

static int reddit_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    
    size_t buffer_size = 8192;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== Reddit Post ===\n\n");
    
    char* subreddit = extract_subreddit(url);
    if (subreddit) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Subreddit: r/%s\n", subreddit);
        free(subreddit);
    }
    
    pos += snprintf(buffer + pos, buffer_size - pos, "URL: %s\n\n", url);
    
    char* title = extract_post_title(html);
    if (title) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Title: %s\n\n", title);
        free(title);
    }
    
    *output = buffer;
    *output_len = pos;
    return 0;
}

static void reddit_cleanup(void) {
}

static const SiteHandler reddit_handler = {
    .name = "reddit",
    .url_pattern = "reddit.com",
    .can_handle = reddit_can_handle,
    .extract = reddit_extract,
    .cleanup = reddit_cleanup,
    .user_data = NULL
};

const SiteHandler* get_reddit_handler(void) {
    return &reddit_handler;
}
