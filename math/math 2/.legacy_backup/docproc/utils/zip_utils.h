#ifndef ZIP_UTILS_H
#define ZIP_UTILS_H

#include <stddef.h>
#include <stdbool.h>

/**
 * ZIP Utilities for Document Processing
 * 
 * Wrapper around libzip for extracting files from ZIP archives.
 */

/**
 * Extract a file from ZIP archive to memory
 * 
 * @param zip_path Path to ZIP file
 * @param file_path Path of file inside ZIP
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of bytes read, or -1 on error
 */
int zip_extract_file(const char* zip_path, const char* file_path, 
                     char* buffer, size_t buffer_size);

/**
 * Check if file exists in ZIP archive
 * 
 * @param zip_path Path to ZIP file
 * @param file_path Path of file inside ZIP
 * @return true if file exists, false otherwise
 */
bool zip_file_exists(const char* zip_path, const char* file_path);

/**
 * List all files in ZIP archive
 * 
 * @param zip_path Path to ZIP file
 * @param callback Callback function called for each file
 * @param user_data User data passed to callback
 * @return 0 on success, -1 on error
 */
int zip_list_files(const char* zip_path, 
                   void (*callback)(const char* filename, void* user_data),
                   void* user_data);

#endif // ZIP_UTILS_H