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

static char* extract_tweet_text(const char* html) {
    char* text = find_between(html, "data-testid=", "</div>");
    if (!text) {
        text = find_between(html, "class=", "</div>");
    }
    
    if (text) {
        char* clean = malloc(strlen(text) + 1);
        if (!clean) {
            free(text);
            return NULL;
        }
        
        char* src = text;
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
        
        free(text);
        return clean;
    }
    
    return NULL;
}

static char* extract_username(const char* html) {
    char* username = find_between(html, "twitter.com/", "/status/");
    if (!username) {
        username = find_between(html, "x.com/", "/status/");
    }
    return username;
}

static char* extract_timestamp(const char* html) {
    char* timestamp = find_between(html, "<time datetime=", ">");
    return timestamp;
}

static bool twitter_can_handle(const char* url) {
    if (!url) return false;
    return url_matches_pattern(url, "twitter.com") || 
           url_matches_pattern(url, "x.com");
}

static int twitter_extract(const char* html, const char* url, char** output, size_t* output_len) {
    if (!html || !url || !output || !output_len) return -1;
    
    size_t buffer_size = 4096;
    char* buffer = malloc(buffer_size);
    if (!buffer) return -1;
    
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "=== Twitter/X.com Content ===\n\n");
    
    char* username = extract_username(html);
    if (username) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Author: @%s\n", username);
        free(username);
    }
    
    char* timestamp = extract_timestamp(html);
    if (timestamp) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Posted: %s\n", timestamp);
        free(timestamp);
    }
    
    pos += snprintf(buffer + pos, buffer_size - pos, "URL: %s\n\n", url);
    
    char* tweet_text = extract_tweet_text(html);
    if (tweet_text) {
        pos += snprintf(buffer + pos, buffer_size - pos, "Tweet:\n%s\n\n", tweet_text);
        free(tweet_text);
    } else {
        pos += snprintf(buffer + pos, buffer_size - pos, "Tweet: [Could not extract]\n\n");
    }
    
    *output = buffer;
    *output_len = pos;
    
    return 0;
}

static void twitter_cleanup(void) {
}

static const SiteHandler twitter_handler = {
    .name = "twitter",
    .url_pattern = "twitter.com|x.com",
    .can_handle = twitter_can_handle,
    .extract = twitter_extract,
    .cleanup = twitter_cleanup,
    .user_data = NULL
};

const SiteHandler* get_twitter_handler(void) {
    return &twitter_handler;
}
