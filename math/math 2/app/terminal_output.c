// app/terminal_output.c - Terminal Output Buffer Implementation
#include "terminal_output.h"
#include <string.h>
#include <stdio.h>

void terminal_buffer_init(TerminalBuffer* buffer) {
    if (!buffer) return;
    
    memset(buffer, 0, sizeof(TerminalBuffer));
    buffer->auto_scroll = true;
    buffer->scroll_offset = 0;
    buffer->line_count = 0;
    buffer->write_index = 0;
}

void terminal_buffer_add_line(TerminalBuffer* buffer, const char* text) {
    if (!buffer || !text) return;
    
    // Copy text to current write position
    strncpy(buffer->lines[buffer->write_index].text, text, TERMINAL_LINE_LENGTH - 1);
    buffer->lines[buffer->write_index].text[TERMINAL_LINE_LENGTH - 1] = '\0';
    buffer->lines[buffer->write_index].timestamp = time(NULL);
    
    // Advance write index (circular buffer)
    buffer->write_index = (buffer->write_index + 1) % TERMINAL_MAX_LINES;
    
    // Update line count (max TERMINAL_MAX_LINES)
    if (buffer->line_count < TERMINAL_MAX_LINES) {
        buffer->line_count++;
    }
    
    // Auto-scroll to bottom if enabled
    if (buffer->auto_scroll) {
        buffer->scroll_offset = 0;
    }
}

const char* terminal_buffer_get_line(TerminalBuffer* buffer, int index) {
    if (!buffer || index < 0 || index >= buffer->line_count) {
        return "";
    }
    
    // Calculate actual index in circular buffer
    int actual_index;
    if (buffer->line_count < TERMINAL_MAX_LINES) {
        // Buffer not full yet, simple indexing
        actual_index = index;
    } else {
        // Buffer full, account for wrap-around
        actual_index = (buffer->write_index + index) % TERMINAL_MAX_LINES;
    }
    
    return buffer->lines[actual_index].text;
}

int terminal_buffer_get_line_count(TerminalBuffer* buffer) {
    if (!buffer) return 0;
    return buffer->line_count;
}

void terminal_buffer_scroll(TerminalBuffer* buffer, int delta) {
    if (!buffer) return;
    
    buffer->scroll_offset += delta;
    
    // Clamp scroll offset
    int max_scroll = buffer->line_count - 1;
    if (buffer->scroll_offset < 0) {
        buffer->scroll_offset = 0;
    }
    if (buffer->scroll_offset > max_scroll) {
        buffer->scroll_offset = max_scroll;
    }
    
    // Disable auto-scroll when user scrolls up
    if (delta < 0) {
        buffer->auto_scroll = false;
    }
    
    // Re-enable auto-scroll when scrolled to bottom
    if (buffer->scroll_offset == 0) {
        buffer->auto_scroll = true;
    }
}

void terminal_buffer_clear(TerminalBuffer* buffer) {
    if (!buffer) return;
    
    buffer->line_count = 0;
    buffer->write_index = 0;
    buffer->scroll_offset = 0;
    buffer->auto_scroll = true;
}

int terminal_buffer_search(TerminalBuffer* buffer, const char* query, int* results, int max_results) {
    if (!buffer || !query || !results) return 0;
    
    int found_count = 0;
    
    for (int i = 0; i < buffer->line_count && found_count < max_results; i++) {
        const char* line = terminal_buffer_get_line(buffer, i);
        if (strstr(line, query) != NULL) {
            results[found_count++] = i;
        }
    }
    
    return found_count;
}