/**
 * Document Processing Core
 * 
 * Core initialization, cleanup, and extraction dispatch.
 */

#include "../docproc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Library initialization state
static bool g_initialized = false;

/**
 * Initialize document processing library
 */
DocProcStatus docproc_init(void) {
    if (g_initialized) {
        return DOCPROC_SUCCESS;
    }
    
    // Initialize any global state here
    // For now, just mark as initialized
    g_initialized = true;
    
    return DOCPROC_SUCCESS;
}

/**
 * Cleanup document processing library
 */
void docproc_cleanup(void) {
    if (!g_initialized) {
        return;
    }
    
    // Cleanup any global state here
    g_initialized = false;
}

/**
 * Get default extraction options
 */
void docproc_default_options(DocProcOptions* options) {
    if (!options) return;
    
    options->extract_metadata = true;
    options->preserve_structure = true;
    options->extract_images = false;
    options->use_ocr = false;
    options->clean_text = true;
    options->min_text_length = 100;
    options->max_text_size = 50 * 1024 * 1024;  // 50MB
}

/**
 * Extract text from document
 */
DocProcStatus docproc_extract(const char* filepath, 
                              const DocProcOptions* options,
                              DocProcResult* result) {
    if (!filepath || !result) {
        return DOCPROC_ERROR_INVALID_PARAMETER;
    }
    
    // Detect format
    DocProcFormat format = docproc_detect_format(filepath, NULL);
    
    return docproc_extract_format(filepath, format, options, result);
}

/**
 * Extract text from document with explicit format
 */
DocProcStatus docproc_extract_format(const char* filepath,
                                     DocProcFormat format,
                                     const DocProcOptions* options,
                                     DocProcResult* result) {
    if (!filepath || !result) {
        return DOCPROC_ERROR_INVALID_PARAMETER;
    }
    
    // Initialize result
    memset(result, 0, sizeof(DocProcResult));
    result->format = format;
    
    // Use default options if not provided
    DocProcOptions default_opts;
    if (!options) {
        docproc_default_options(&default_opts);
        options = &default_opts;
    }
    
    // Allocate text buffer
    result->text = (char*)malloc(options->max_text_size);
    if (!result->text) {
        result->status = DOCPROC_ERROR_OUT_OF_MEMORY;
        snprintf(result->error_message, sizeof(result->error_message),
                "Failed to allocate text buffer");
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    // Dispatch to format-specific extractor
    DocProcStatus status;
    
    switch (format) {
        case DOCPROC_FORMAT_PDF:
            status = docproc_extract_pdf(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_DOCX:
            status = docproc_extract_docx(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_XLSX:
            status = docproc_extract_xlsx(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_PPTX:
            status = docproc_extract_pptx(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_ODT:
            status = docproc_extract_odt(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_ODS:
            status = docproc_extract_ods(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_ODP:
            status = docproc_extract_odp(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_EPUB:
            status = docproc_extract_epub(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_HTML:
            status = docproc_extract_html(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_MARKDOWN:
            status = docproc_extract_markdown(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_YAML:
            status = docproc_extract_yaml(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_TOML:
            status = docproc_extract_toml(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_JSON:
            status = docproc_extract_json(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_CSV:
            status = docproc_extract_csv(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_SQL:
            status = docproc_extract_sql(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_EML:
            status = docproc_extract_eml(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_IMAGE:
            if (options->use_ocr) {
                status = docproc_extract_image_ocr(filepath, result->text, options->max_text_size);
            } else {
                status = DOCPROC_ERROR_UNSUPPORTED_FORMAT;
                snprintf(result->error_message, sizeof(result->error_message),
                        "OCR not enabled for image extraction");
            }
            break;
            
        case DOCPROC_FORMAT_ARCHIVE:
            status = docproc_extract_archive(filepath, result->text, options->max_text_size);
            break;
            
        case DOCPROC_FORMAT_TXT:
        case DOCPROC_FORMAT_CODE:
        case DOCPROC_FORMAT_XML:
        default:
            // Direct file read for plain text formats
            FILE* fp = fopen(filepath, "r");
            if (!fp) {
                status = DOCPROC_ERROR_FILE_NOT_FOUND;
                snprintf(result->error_message, sizeof(result->error_message),
                        "Failed to open file: %s", filepath);
                break;
            }
            
            size_t bytes_read = fread(result->text, 1, options->max_text_size - 1, fp);
            result->text[bytes_read] = '\0';
            fclose(fp);
            
            status = DOCPROC_SUCCESS;
            break;
    }
    
    // Update result
    result->status = status;
    if (status == DOCPROC_SUCCESS) {
        result->text_length = strlen(result->text);
        
        // Check minimum length
        if (result->text_length < options->min_text_length) {
            status = DOCPROC_ERROR_EXTRACTION_FAILED;
            snprintf(result->error_message, sizeof(result->error_message),
                    "Extracted text too short: %zu bytes (minimum: %zu)",
                    result->text_length, options->min_text_length);
            result->status = status;
        }
    } else if (result->error_message[0] == '\0') {
        snprintf(result->error_message, sizeof(result->error_message),
                "Extraction failed for format: %s", docproc_format_name(format));
    }
    
    return status;
}

/**
 * Free extraction result
 */
void docproc_free_result(DocProcResult* result) {
    if (!result) return;
    
    if (result->text) {
        free(result->text);
        result->text = NULL;
    }
    
    result->text_length = 0;
}

/**
 * Get library version
 */
const char* docproc_version(void) {
    return "1.0.0";
}

/**
 * Get list of supported file extensions
 */
int docproc_get_supported_extensions(const char** extensions, int max_count) {
    static const char* supported[] = {
        "html", "htm", "pdf", "doc", "docx", "xlsx", "pptx",
        "odt", "ods", "odp", "epub", "txt", "md", "markdown",
        "json", "xml", "csv", "yaml", "yml", "toml", "sql",
        "tex", "latex", "eml", "rtf", "c", "h", "cpp", "py",
        "js", "java", "go", "rs", "jpg", "jpeg", "png", "gif",
        "webp", "bmp", "tiff", "tif", "zip", "tar", "gz", "tgz",
        "bz2", "xz"
    };
    
    int count = sizeof(supported) / sizeof(supported[0]);
    if (count > max_count) count = max_count;
    
    for (int i = 0; i < count; i++) {
        extensions[i] = supported[i];
    }
    
    return count;
}