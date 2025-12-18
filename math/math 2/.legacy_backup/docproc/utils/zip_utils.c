/**
 * ZIP Utilities Implementation
 * 
 * Uses libzip for ZIP file operations.
 */

#include "zip_utils.h"
#include <zip.h>
#include <string.h>
#include <stdio.h>

/**
 * Extract a file from ZIP archive to memory
 */
int zip_extract_file(const char* zip_path, const char* file_path, 
                     char* buffer, size_t buffer_size) {
    int err;
    zip_t* archive = zip_open(zip_path, ZIP_RDONLY, &err);
    if (!archive) {
        return -1;
    }
    
    // Find file in archive
    zip_int64_t index = zip_name_locate(archive, file_path, 0);
    if (index < 0) {
        zip_close(archive);
        return -1;
    }
    
    // Open file
    zip_file_t* file = zip_fopen_index(archive, index, 0);
    if (!file) {
        zip_close(archive);
        return -1;
    }
    
    // Read file
    zip_int64_t bytes_read = zip_fread(file, buffer, buffer_size - 1);
    if (bytes_read < 0) {
        zip_fclose(file);
        zip_close(archive);
        return -1;
    }
    
    buffer[bytes_read] = '\0';
    
    zip_fclose(file);
    zip_close(archive);
    
    return (int)bytes_read;
}

/**
 * Check if file exists in ZIP archive
 */
bool zip_file_exists(const char* zip_path, const char* file_path) {
    int err;
    zip_t* archive = zip_open(zip_path, ZIP_RDONLY, &err);
    if (!archive) {
        return false;
    }
    
    zip_int64_t index = zip_name_locate(archive, file_path, 0);
    zip_close(archive);
    
    return index >= 0;
}

/**
 * List all files in ZIP archive
 */
int zip_list_files(const char* zip_path, 
                   void (*callback)(const char* filename, void* user_data),
                   void* user_data) {
    int err;
    zip_t* archive = zip_open(zip_path, ZIP_RDONLY, &err);
    if (!archive) {
        return -1;
    }
    
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char* name = zip_get_name(archive, i, 0);
        if (name && callback) {
            callback(name, user_data);
        }
    }
    
    zip_close(archive);
    return 0;
}