#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <stddef.h>

// File type enumeration
typedef enum {
    FILE_TYPE_HTML = 0,
    FILE_TYPE_PDF = 1,
    FILE_TYPE_DOC = 2,
    FILE_TYPE_DOCX = 3,
    FILE_TYPE_XLSX = 4,
    FILE_TYPE_PPTX = 5,
    FILE_TYPE_ODT = 6,
    FILE_TYPE_ODS = 7,
    FILE_TYPE_ODP = 8,
    FILE_TYPE_EPUB = 9,
    FILE_TYPE_TXT = 10,
    FILE_TYPE_JSON = 11,
    FILE_TYPE_XML = 12,
    FILE_TYPE_CSV = 13,
    FILE_TYPE_CODE = 14,
    FILE_TYPE_MARKDOWN = 15,
    FILE_TYPE_YAML = 16,
    FILE_TYPE_TOML = 17,
    FILE_TYPE_SQL = 18,
    FILE_TYPE_LATEX = 19,
    FILE_TYPE_EML = 20,
    FILE_TYPE_IMAGE = 21,
    FILE_TYPE_ARCHIVE = 22,
    FILE_TYPE_RTF = 23,
    FILE_TYPE_UNKNOWN = 24
} FileType;

/**
 * Detect file type from URL and content type
 */
FileType detect_file_type(const char* url, const char* content_type);

/**
 * Get file type name for logging
 */
const char* get_file_type_name(FileType type);

/**
 * Process file and extract text
 * Returns number of bytes extracted, or -1 on error
 */
int process_file_by_type(const char* filepath, FileType type, 
                        char* output_text, size_t output_size);

#endif // FILE_PROCESSOR_H