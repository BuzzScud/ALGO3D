/**
 * HTML Preprocessor
 * 
 * Converts HTML pages to clean text suitable for training
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "content_filter.h"
#include "site_handlers.h"
#include "crawler_url_manager.h"

#define MAX_TEXT_SIZE (5 * 1024 * 1024)  // 5MB max text
#define MIN_TEXT_LENGTH 100

// Forward declarations for file processors
extern int process_pdf_file(const char* input_path, const char* output_path);
extern int process_image_file(const char* input_path, const char* output_path);
extern int process_office_file(const char* input_path, const char* output_path);

// Global URL manager for link extraction
CrawlerURLManager* g_crawler_url_manager = NULL;

/**
 * File type enumeration
 */
typedef enum {
    FILE_TYPE_HTML,
    FILE_TYPE_PDF,
    FILE_TYPE_IMAGE,
    FILE_TYPE_BINARY,
    FILE_TYPE_TEXT,
    FILE_TYPE_UNKNOWN
} FileType;

/**
 * Detect file type from magic bytes
 */
static FileType detect_file_type(const char* data, size_t size) {
    if (size < 4) return FILE_TYPE_UNKNOWN;
    
    // Check for common magic bytes
    unsigned char* bytes = (unsigned char*)data;
    
    // PDF: %PDF
    if (size >= 4 && bytes[0] == '%' && bytes[1] == 'P' && 
        bytes[2] == 'D' && bytes[3] == 'F') {
        return FILE_TYPE_PDF;
    }
    
    // PNG: \x89PNG
    if (size >= 4 && bytes[0] == 0x89 && bytes[1] == 'P' && 
        bytes[2] == 'N' && bytes[3] == 'G') {
        return FILE_TYPE_IMAGE;
    }
    
    // JPEG: \xFF\xD8\xFF
    if (size >= 3 && bytes[0] == 0xFF && bytes[1] == 0xD8 && bytes[2] == 0xFF) {
        return FILE_TYPE_IMAGE;
    }
    
    // GIF: GIF87a or GIF89a
    if (size >= 6 && bytes[0] == 'G' && bytes[1] == 'I' && bytes[2] == 'F') {
        return FILE_TYPE_IMAGE;
    }
    
    // ZIP/Office: PK\x03\x04 (DOCX, XLSX, PPTX, EPUB, etc.)
    if (size >= 4 && bytes[0] == 'P' && bytes[1] == 'K' && 
        bytes[2] == 0x03 && bytes[3] == 0x04) {
        // Could be Office document or other ZIP
        // Check for Office-specific markers
        if (size > 100) {
            // Look for Office XML namespace markers
            if (strstr(data, "word/") || strstr(data, "xl/") || strstr(data, "ppt/")) {
                return FILE_TYPE_BINARY;  // Office document
            }
        }
        return FILE_TYPE_BINARY;
    }
    
    // Legacy Office: OLE signature (DOC, XLS, PPT)
    if (size >= 8 && bytes[0] == 0xD0 && bytes[1] == 0xCF && 
        bytes[2] == 0x11 && bytes[3] == 0xE0) {
        return FILE_TYPE_BINARY;  // Legacy Office document
    }
    
    // Check for HTML markers
    if (size >= 15) {
        // Look for <!DOCTYPE, <html, or <HTML
        if (strncasecmp(data, "<!DOCTYPE", 9) == 0 ||
            strncasecmp(data, "<html", 5) == 0 ||
            strstr(data, "<head") != NULL ||
            strstr(data, "<body") != NULL) {
            return FILE_TYPE_HTML;
        }
    }
    
    // Check if mostly printable ASCII (text file)
    int printable = 0;
    int total = size > 1000 ? 1000 : size;
    for (int i = 0; i < total; i++) {
        if (isprint(bytes[i]) || isspace(bytes[i])) {
            printable++;
        }
    }
    
    if (printable > total * 0.95) {
        return FILE_TYPE_TEXT;
    }
    
    return FILE_TYPE_BINARY;
}

static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", tm_info);
}


typedef struct {
    char data_dir[1024];
    int running;
    int files_processed;
    ExtractionMode extraction_mode;
    bool handlers_initialized;  // Track if handlers are registered  // NEW: Content filtering mode
    pthread_mutex_t lock;
} PreprocessorState;

/**
 * Remove HTML tags from text
 */
static void remove_html_tags(const char* html, char* text, size_t text_size) {
    const char* p = html;
    char* out = text;
    char* out_end = text + text_size - 1;
    int in_tag = 0;
    int in_script = 0;
    int in_style = 0;
    
    while (*p && out < out_end) {
        // Check for script/style tags
        if (strncasecmp(p, "<script", 7) == 0) {
            in_script = 1;
            in_tag = 1;
        } else if (strncasecmp(p, "</script>", 9) == 0) {
            in_script = 0;
            p += 9;
            continue;
        } else if (strncasecmp(p, "<style", 6) == 0) {
            in_style = 1;
            in_tag = 1;
        } else if (strncasecmp(p, "</style>", 8) == 0) {
            in_style = 0;
            p += 8;
            continue;
        }
        
        // Skip script/style content
        if (in_script || in_style) {
            p++;
            continue;
        }
        
        // Handle tags
        if (*p == '<') {
            in_tag = 1;
            p++;
            continue;
        } else if (*p == '>') {
            in_tag = 0;
            p++;
            // Add space after tag
            if (out > text && *(out-1) != ' ' && *(out-1) != '\n') {
                *out++ = ' ';
            }
            continue;
        }
        
        // Skip tag content
        if (in_tag) {
            p++;
            continue;
        }
        
        // Copy text content
        *out++ = *p++;
    }
    
    *out = '\0';
}

/**
 * Clean and normalize text
 */
static void clean_text(char* text) {
    char* src = text;
    char* dst = text;
    int last_was_space = 1;
    
    while (*src) {
        // Convert HTML entities (simple version)
        if (strncmp(src, "&nbsp;", 6) == 0) {
            *dst++ = ' ';
            src += 6;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&lt;", 4) == 0) {
            *dst++ = '<';
            src += 4;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&gt;", 4) == 0) {
            *dst++ = '>';
            src += 4;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&amp;", 5) == 0) {
            *dst++ = '&';
            src += 5;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&quot;", 6) == 0) {
            *dst++ = '"';
            src += 6;
            last_was_space = 0;
            continue;
        }
        
        // Normalize whitespace
        if (isspace(*src)) {
            if (!last_was_space) {
                *dst++ = ' ';
                last_was_space = 1;
            }
            src++;
        } else {
            *dst++ = *src++;
            last_was_space = 0;
        }
    }
    
    *dst = '\0';
    
    // Trim leading whitespace (move content to start of buffer)
    char* start = text;
    while (*start && isspace(*start)) start++;
    
    if (start != text) {
        // Move trimmed content to beginning
        char* write = text;
        while (*start) {
            *write++ = *start++;
        }
        *write = '\0';
    }
    
    // Trim trailing whitespace
    size_t len = strlen(text);
    while (len > 0 && isspace(text[len-1])) {
        text[len-1] = '\0';
        len--;
    }
}


/**
 * Extract links from HTML and add to crawl queue
 */
static int extract_links(const char* html, const char* base_url, const char* queue_file) {
    // Try to use URL manager if available (passed via global)
    extern CrawlerURLManager* g_crawler_url_manager;
    
    FILE* queue = NULL;
    if (!g_crawler_url_manager) {
        // Fallback to file-based system
        queue = fopen(queue_file, "a");
        if (!queue) {
            fprintf(stderr, "Failed to open queue file: %s\n", queue_file);
            return -1;
        }
    }
    
    int links_found = 0;
    const char* p = html;
    
    // Look for href attributes
    while ((p = strstr(p, "href=")) != NULL) {
        p += 5; // Skip "href="
        
        // Skip whitespace
        while (*p && isspace(*p)) p++;
        
        // Determine quote type
        char quote = 0;
        if (*p == '"' || *p == '\'') {
            quote = *p;
            p++;
        }
        
        // Find end of URL
        const char* url_start = p;
        const char* url_end = NULL;
        
        if (quote) {
            url_end = strchr(p, quote);
        } else {
            // No quotes - find whitespace or >
            while (*p && !isspace(*p) && *p != '>') p++;
            url_end = p;
        }
        
        if (!url_end || url_end == url_start) {
            if (quote) p = url_end + 1;
            continue;
        }
        
        // Extract URL
        size_t url_len = url_end - url_start;
        if (url_len >= 2048) {
            p = url_end + 1;
            continue;
        }
        
        char url[2048];
        strncpy(url, url_start, url_len);
        url[url_len] = '\0';
        
        // Skip invalid URLs
        if (url[0] == '#' || 
            strncmp(url, "javascript:", 11) == 0 ||
            strncmp(url, "mailto:", 7) == 0 ||
            strncmp(url, "tel:", 4) == 0 ||
            strncmp(url, "data:", 5) == 0) {
            p = url_end + 1;
            continue;
        }
        
        // Handle relative URLs
        if (url[0] == '/') {
            // Extract domain from base_url
            const char* domain_start = strstr(base_url, "://");
            if (domain_start) {
                domain_start += 3;
                const char* domain_end = strchr(domain_start, '/');
                if (!domain_end) domain_end = domain_start + strlen(domain_start);
                
                size_t domain_len = domain_end - domain_start;
                char full_url[2048];
                
                // Determine protocol
                const char* protocol = "https://";
                if (strncmp(base_url, "http://", 7) == 0) {
                    protocol = "http://";
                }
                
                int url_len = snprintf(full_url, sizeof(full_url), "%s%.*s%s", 
                        protocol, (int)domain_len, domain_start, url);
                if (url_len >= (int)sizeof(full_url)) {
                    // URL too long, skip it
                    continue;
                }
                
                // Add to database or file
                if (g_crawler_url_manager) {
                    crawler_url_manager_add(g_crawler_url_manager, full_url, base_url);
                } else if (queue) {
                    fprintf(queue, "%s\n", full_url);
                }
                links_found++;
            }
        } else if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
            // Absolute URL
            if (g_crawler_url_manager) {
                crawler_url_manager_add(g_crawler_url_manager, url, base_url);
            } else if (queue) {
                fprintf(queue, "%s\n", url);
            }
            links_found++;
        }
        
        p = url_end + 1;
    }
    
    if (queue) {
        fclose(queue);
    }
    return links_found;
}

/**
 * Extract base URL from HTML metadata comment
 */
static int extract_base_url(const char* html, char* base_url, size_t size) {
    // Look for <!-- URL: ... --> comment
    const char* marker = "<!-- URL: ";
    const char* p = strstr(html, marker);
    if (!p) return -1;
    
    p += strlen(marker);
    const char* end = strstr(p, " -->");
    if (!end) return -1;
    
    size_t len = end - p;
    if (len >= size) return -1;
    
    strncpy(base_url, p, len);
    base_url[len] = '\0';
    return 0;
}

/**
 * Process one HTML file
 */
static int preprocess_file(const char* input_path, const char* output_path, const char* queue_file) {
    // Read input file in binary mode for magic byte detection
    FILE* f = fopen(input_path, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open: %s\n", input_path);
        return -1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* html = (char*)malloc(size + 1);
    if (!html) {
        fclose(f);
        return -1;
    }
    
    size_t bytes_read = fread(html, 1, size, f);
    if (bytes_read != (size_t)size) {
        fprintf(stderr, "Warning: Expected to read %ld bytes, got %zu bytes from %s\n", 
                size, bytes_read, input_path);
    }
    html[bytes_read] = '\0';
    fclose(f);
    
    // Detect file type
    FileType file_type = detect_file_type(html, size);
    
    // Debug: Show file type and size
    const char* type_names[] = {"HTML", "PDF", "IMAGE", "BINARY", "TEXT", "UNKNOWN"};
    printf("  File type: %s, Size: %ld bytes\n", type_names[file_type], size);
    
    // Route to appropriate processor based on file type
    char* text = NULL;
    
    switch (file_type) {
        case FILE_TYPE_PDF:
            printf("  Processing PDF file...\n");
            free(html);
            // Call PDF processor
            return process_pdf_file(input_path, output_path);
            
        case FILE_TYPE_IMAGE:
            printf("  Processing image with OCR...\n");
            free(html);
            // Call image OCR processor
            return process_image_file(input_path, output_path);
            
        case FILE_TYPE_BINARY:
            printf("  Processing binary file (Office document)...\n");
            free(html);
            // Try Office document processor
            int office_result = process_office_file(input_path, output_path);
            if (office_result == 0) {
                return 0;  // Successfully processed
            }
            // If Office processing failed, create marker
            FILE* bin_marker = fopen(output_path, "w");
            if (bin_marker) {
                fprintf(bin_marker, "<!-- Binary file - Office processing failed or unsupported -->\n");
                fclose(bin_marker);
            }
            return -1;
            
        case FILE_TYPE_HTML:
        case FILE_TYPE_TEXT:
        case FILE_TYPE_UNKNOWN:
            // Process as HTML/text (existing code path)
            text = (char*)malloc(MAX_TEXT_SIZE);
            if (!text) {
                free(html);
                return -1;
            }
            break;
    }
    
    // If we got here, we're processing HTML/text
    if (!text) {
        free(html);
        return -1;
    }
    

    // Extract base URL from metadata
    char base_url[2048] = {0};
    extract_base_url(html, base_url, sizeof(base_url));
    
    // Extract links and add to queue
    int links_found = 0;
    if (base_url[0]) {
        links_found = extract_links(html, base_url, queue_file);
        if (links_found > 0) {
            char timestamp[32];
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s   Extracted %d links\n", timestamp, links_found);
        }
    }
    
    // Debug: Show raw HTML size
    printf("  Raw HTML: %ld bytes\n", size);
    
    // Use smart extraction if mode is set, otherwise use legacy method
    // For now, default to EXTRACT_ALL to maintain backward compatibility
    ExtractionMode mode = EXTRACT_ALL;  // TODO: Get from state parameter
    
    if (mode == EXTRACT_ALL) {
        // Legacy extraction - remove all tags
        remove_html_tags(html, text, MAX_TEXT_SIZE);
    } else {
        // Smart extraction - filter based on content type
        extract_content_smart(html, text, MAX_TEXT_SIZE, mode);
    }
    free(html);
    
    // Debug: Show text after tag removal
    size_t text_after_tags = strlen(text);
    printf("  After tag removal: %zu chars\n", text_after_tags);
    
    // Clean text
    clean_text(text);
    
    // Debug: Show text after cleaning
    size_t text_after_clean = strlen(text);
    printf("  After cleaning: %zu chars\n", text_after_clean);
    
    // Check minimum length
    if (text_after_clean < MIN_TEXT_LENGTH) {
        printf("  Skipped (too short): %zu chars (min: %d)\n", text_after_clean, MIN_TEXT_LENGTH);
        
        // CRITICAL FIX: Create empty marker file to prevent infinite loop
        FILE* marker = fopen(output_path, "w");
        if (marker) {
            fprintf(marker, "<!-- SKIPPED: Too short (%zu chars) -->\n", text_after_clean);
            fclose(marker);
        }
        
        free(text);
        return -1;
    }
    
    // Write output
    FILE* out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "Failed to write: %s\n", output_path);
        free(text);
        return -1;
    }
    
    fprintf(out, "%s\n", text);
    fclose(out);
    free(text);
    
    return 0;
}

/**
 * Preprocessor thread
 */
/**
 * Get current timestamp string
 */


void* preprocessor_thread_func(void* arg) {
    PreprocessorState* state = (PreprocessorState*)arg;
    char timestamp[32];
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === PREPROCESSOR STARTED ===\n", timestamp);
    
    char raw_dir[2048];
    char preprocessed_dir[2048];
    char queue_file[2048];
    snprintf(raw_dir, sizeof(raw_dir), "%s/raw_pages", state->data_dir);
    snprintf(preprocessed_dir, sizeof(preprocessed_dir), "%s/preprocessed", state->data_dir);
    snprintf(queue_file, sizeof(queue_file), "%s/links_to_crawl.txt", state->data_dir);
    
    while (state->running) {
        DIR* dir = opendir(raw_dir);
        if (!dir) {
            sleep(5);
            continue;
        }
        
        struct dirent* entry;
        int found_file = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (strstr(entry->d_name, ".html") == NULL) continue;
            
            // Check if already processed
            char preprocessed_path[2048];
            char* base = strdup(entry->d_name);
            char* dot = strrchr(base, '.');
            if (dot) *dot = '\0';
            
            int path_len = snprintf(preprocessed_path, sizeof(preprocessed_path), 
                    "%s/%s.txt", preprocessed_dir, base);
            if (path_len >= (int)sizeof(preprocessed_path)) {
                fprintf(stderr, "Preprocessed path too long, skipping: %s\n", base);
                free(base);
                continue;
            }
            
            // Check if output exists
            if (access(preprocessed_path, F_OK) == 0) {
                free(base);
                continue;
            }
            
            // Process file
            char input_path[2048];
            int input_len = snprintf(input_path, sizeof(input_path), "%s/%s", raw_dir, entry->d_name);
            if (input_len >= (int)sizeof(input_path)) {
                fprintf(stderr, "Input path too long, skipping: %s\n", entry->d_name);
                free(base);
                continue;
            }
            
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s Preprocessing: %s\n", timestamp, entry->d_name);
            
            if (preprocess_file(input_path, preprocessed_path, queue_file) == 0) {
                get_timestamp(timestamp, sizeof(timestamp));
                printf("%s ✓ Preprocessed: %s\n", timestamp, base);
                pthread_mutex_lock(&state->lock);
                state->files_processed++;
                pthread_mutex_unlock(&state->lock);
            }
            
            free(base);
            found_file = 1;
            break;  // Process one file at a time
        }
        
        closedir(dir);
        
        if (!found_file) {
            sleep(5);  // Wait for new files
        } else {
            sleep(1);  // Small delay between files
        }
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === PREPROCESSOR STOPPED ===\n", timestamp);
    return NULL;
}

/**
 * Initialize preprocessor
 */
PreprocessorState* preprocessor_init(const char* data_dir) {
    PreprocessorState* state = (PreprocessorState*)calloc(1, sizeof(PreprocessorState));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    state->running = 1;
    state->files_processed = 0;
    state->extraction_mode = EXTRACT_ALL;  // Default to extract everything
    
    // Initialize site handlers (only once)
    if (!state->handlers_initialized) {
        extern void register_all_handlers(void);
        register_all_handlers();
        state->handlers_initialized = true;
    }
    
    pthread_mutex_init(&state->lock, NULL);
    
    return state;
}

/**
 * Cleanup preprocessor
 */
void preprocessor_cleanup(PreprocessorState* state) {
    if (!state) return;
    pthread_mutex_destroy(&state->lock);
    free(state);
}
/**
 * Set extraction mode for preprocessor
 */
void preprocessor_set_extraction_mode(PreprocessorState* state, ExtractionMode mode) {
    if (!state) return;
    
    pthread_mutex_lock(&state->lock);
    state->extraction_mode = mode;
    pthread_mutex_unlock(&state->lock);
    
    const char* mode_names[] = {"ALL", "HUMAN_TEXT", "METADATA", "MIXED"};
    printf("Extraction mode set to: %s\n", mode_names[mode]);
}

/**
 * Get current extraction mode
 */
ExtractionMode preprocessor_get_extraction_mode(PreprocessorState* state) {
    if (!state) return EXTRACT_ALL;
    
    pthread_mutex_lock(&state->lock);
    ExtractionMode mode = state->extraction_mode;
    pthread_mutex_unlock(&state->lock);
    
    return mode;
}
