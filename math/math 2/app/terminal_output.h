// app/terminal_output.h - Terminal Output Buffer for UI Display
#ifndef TERMINAL_OUTPUT_H
#define TERMINAL_OUTPUT_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#define TERMINAL_MAX_LINES 500
#define TERMINAL_LINE_LENGTH 256

typedef struct {
    char text[TERMINAL_LINE_LENGTH];
    time_t timestamp;
} TerminalLine;

typedef struct TerminalBuffer {
    TerminalLine lines[TERMINAL_MAX_LINES];
    int line_count;
    int scroll_offset;
    bool auto_scroll;
    int write_index;  // Circular buffer write position
} TerminalBuffer;

// Initialize terminal buffer
void terminal_buffer_init(TerminalBuffer* buffer);

// Add a line to the terminal buffer
void terminal_buffer_add_line(TerminalBuffer* buffer, const char* text);

// Get a line from the buffer (accounting for circular buffer)
const char* terminal_buffer_get_line(TerminalBuffer* buffer, int index);

// Get the number of visible lines
int terminal_buffer_get_line_count(TerminalBuffer* buffer);

// Scroll the terminal view
void terminal_buffer_scroll(TerminalBuffer* buffer, int delta);

// Clear the terminal buffer
void terminal_buffer_clear(TerminalBuffer* buffer);

// Search for text in the buffer
int terminal_buffer_search(TerminalBuffer* buffer, const char* query, int* results, int max_results);

#endif // TERMINAL_OUTPUT_H