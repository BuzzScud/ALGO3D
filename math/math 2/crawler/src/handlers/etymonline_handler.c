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

static char* extract_word(const char* html) {
    char* word = find_between(html, "<h1", "</h1>");
    if (word) {
        char* clean = strip_html_tags(word);
        free(word);
        return clean;
    }
    return NULL;
}

static char* extract_etymology(const char* html) {
    char* etym = find_between(html, "<section", "</section>");
    if (etym) {
        char* clean = strip_html_tags(etym);
        free(etym);
        return clean;
    }
    return NULL;
}

static bool etymonline_can_handle(const char* url) {
    if (!url) return false;
    return url_matches_pattern(url, "etymonline.com");
}

static int etymonline_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    size_t buffer_size = 4096;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== Etymonline Word Etymology ===\n\n");
    char* word = extract_word(html);
    if (word) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Word: %s\n\n", word);
        free(word);
    }
    pos += snprintf(buffer + pos, buffer_size - pos, "URL: %s\n\n", url);
    char* etymology = extract_etymology(html);
    if (etymology) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Etymology:\n%s\n\n", etymology);
        free(etymology);
    }
    *output = buffer;
    *output_len = pos;
    return 0;
}

static void etymonline_cleanup(void) {
}

static const SiteHandler etymonline_handler = {
    .name = "etymonline",
    .url_pattern = "etymonline.com",
    .can_handle = etymonline_can_handle,
    .extract = etymonline_extract,
    .cleanup = etymonline_cleanup,
    .user_data = NULL
};

const SiteHandler* get_etymonline_handler(void) {
    return &etymonline_handler;
}
