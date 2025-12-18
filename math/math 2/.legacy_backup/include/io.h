#ifndef IO_H
#define IO_H

/*
 * io.h - Auto-generated header file
 * Source: io.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"

/* Local includes */
#include "app_common.h"

/* Function declarations */
void start_recording(AppState* state);
void record_frame(AppState* state);
void stop_recording(AppState* state);
void save_snapshot(AppState* state);
char* get_save_path_with_extension(const char* title, const char* default_name, const char* extension);
void ensure_file_extension(char* path, const char* extension);

#endif /* IO_H */