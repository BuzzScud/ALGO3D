/**
 * Multi-Format File Processor for Crawler
 * 
 * Supports: HTML, PDF, DOC, DOCX, XLSX, PPTX, ODT, ODS, ODP, EPUB,
 *           TXT, JSON, XML, CSV, MD, YAML, TOML, SQL, LaTeX, EML,
 *           Images (OCR), Archives, RTF, and more
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_TEXT_SIZE (50 * 1024 * 1024)  // 50MB max extracted text

typedef enum {
    FILE_TYPE_HTML,
    FILE_TYPE_PDF,
    FILE_TYPE_DOC,
    FILE_TYPE_DOCX,
    FILE_TYPE_XLSX,
    FILE_TYPE_PPTX,
    FILE_TYPE_ODT,
    FILE_TYPE_ODS,
    FILE_TYPE_ODP,
    FILE_TYPE_EPUB,
    FILE_TYPE_TXT,
    FILE_TYPE_JSON,
    FILE_TYPE_XML,
    FILE_TYPE_CSV,
    FILE_TYPE_CODE,
    FILE_TYPE_MARKDOWN,
    FILE_TYPE_YAML,
    FILE_TYPE_TOML,
    FILE_TYPE_SQL,
    FILE_TYPE_LATEX,
    FILE_TYPE_EML,
    FILE_TYPE_IMAGE,
    FILE_TYPE_ARCHIVE,
    FILE_TYPE_RTF,
    FILE_TYPE_UNKNOWN
} FileType;

/**
 * Detect file type from URL and content type
 */
FileType detect_file_type(const char* url, const char* content_type) {
    // Check URL extension first
    const char* ext = strrchr(url, '.');
    if (ext) {
        ext++;  // Skip the dot
        
        // Microsoft Office
        if (strcasecmp(ext, "docx") == 0) return FILE_TYPE_DOCX;
        if (strcasecmp(ext, "xlsx") == 0) return FILE_TYPE_XLSX;
        if (strcasecmp(ext, "pptx") == 0) return FILE_TYPE_PPTX;
        if (strcasecmp(ext, "doc") == 0) return FILE_TYPE_DOC;
        
        // LibreOffice/OpenDocument
        if (strcasecmp(ext, "odt") == 0) return FILE_TYPE_ODT;
        if (strcasecmp(ext, "ods") == 0) return FILE_TYPE_ODS;
        if (strcasecmp(ext, "odp") == 0) return FILE_TYPE_ODP;
        
        // eBooks
        if (strcasecmp(ext, "epub") == 0) return FILE_TYPE_EPUB;
        
        // Documents
        if (strcasecmp(ext, "pdf") == 0) return FILE_TYPE_PDF;
        if (strcasecmp(ext, "rtf") == 0) return FILE_TYPE_RTF;
        if (strcasecmp(ext, "txt") == 0) return FILE_TYPE_TXT;
        
        // Structured data
        if (strcasecmp(ext, "json") == 0) return FILE_TYPE_JSON;
        if (strcasecmp(ext, "xml") == 0) return FILE_TYPE_XML;
        if (strcasecmp(ext, "csv") == 0) return FILE_TYPE_CSV;
        if (strcasecmp(ext, "yaml") == 0 || strcasecmp(ext, "yml") == 0) return FILE_TYPE_YAML;
        if (strcasecmp(ext, "toml") == 0) return FILE_TYPE_TOML;
        
        // Documentation
        if (strcasecmp(ext, "md") == 0 || strcasecmp(ext, "markdown") == 0) return FILE_TYPE_MARKDOWN;
        if (strcasecmp(ext, "tex") == 0 || strcasecmp(ext, "latex") == 0) return FILE_TYPE_LATEX;
        
        // Code/SQL
        if (strcasecmp(ext, "sql") == 0) return FILE_TYPE_SQL;
        if (strcasecmp(ext, "c") == 0 || strcasecmp(ext, "h") == 0 ||
            strcasecmp(ext, "cpp") == 0 || strcasecmp(ext, "py") == 0 ||
            strcasecmp(ext, "js") == 0 || strcasecmp(ext, "java") == 0 ||
            strcasecmp(ext, "go") == 0 || strcasecmp(ext, "rs") == 0) {
            return FILE_TYPE_CODE;
        }
        
        // Email
        if (strcasecmp(ext, "eml") == 0 || strcasecmp(ext, "msg") == 0) return FILE_TYPE_EML;
        
        // Images
        if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0 ||
            strcasecmp(ext, "png") == 0 || strcasecmp(ext, "gif") == 0 ||
            strcasecmp(ext, "webp") == 0 || strcasecmp(ext, "bmp") == 0 ||
            strcasecmp(ext, "tiff") == 0 || strcasecmp(ext, "tif") == 0) {
            return FILE_TYPE_IMAGE;
        }
        
        // Archives
        if (strcasecmp(ext, "zip") == 0 || strcasecmp(ext, "tar") == 0 ||
            strcasecmp(ext, "gz") == 0 || strcasecmp(ext, "tgz") == 0 ||
            strcasecmp(ext, "bz2") == 0 || strcasecmp(ext, "xz") == 0) {
            return FILE_TYPE_ARCHIVE;
        }
    }
    
    // Check Content-Type header
    if (content_type) {
        if (strstr(content_type, "application/pdf")) return FILE_TYPE_PDF;
        if (strstr(content_type, "application/vnd.openxmlformats-officedocument.wordprocessingml")) return FILE_TYPE_DOCX;
        if (strstr(content_type, "application/vnd.openxmlformats-officedocument.spreadsheetml")) return FILE_TYPE_XLSX;
        if (strstr(content_type, "application/vnd.openxmlformats-officedocument.presentationml")) return FILE_TYPE_PPTX;
        if (strstr(content_type, "application/vnd.oasis.opendocument.text")) return FILE_TYPE_ODT;
        if (strstr(content_type, "application/vnd.oasis.opendocument.spreadsheet")) return FILE_TYPE_ODS;
        if (strstr(content_type, "application/vnd.oasis.opendocument.presentation")) return FILE_TYPE_ODP;
        if (strstr(content_type, "application/epub+zip")) return FILE_TYPE_EPUB;
        if (strstr(content_type, "application/json")) return FILE_TYPE_JSON;
        if (strstr(content_type, "application/xml") || strstr(content_type, "text/xml")) return FILE_TYPE_XML;
        if (strstr(content_type, "text/csv")) return FILE_TYPE_CSV;
        if (strstr(content_type, "text/plain")) return FILE_TYPE_TXT;
        if (strstr(content_type, "text/html")) return FILE_TYPE_HTML;
        if (strstr(content_type, "text/markdown")) return FILE_TYPE_MARKDOWN;
        if (strstr(content_type, "image/")) return FILE_TYPE_IMAGE;
        if (strstr(content_type, "application/zip")) return FILE_TYPE_ARCHIVE;
        if (strstr(content_type, "application/x-tar")) return FILE_TYPE_ARCHIVE;
    }
    
    return FILE_TYPE_HTML;  // Default
}

/**
 * Get file type name for logging
 */
const char* get_file_type_name(FileType type) {
    switch (type) {
        case FILE_TYPE_HTML: return "HTML";
        case FILE_TYPE_PDF: return "PDF";
        case FILE_TYPE_DOC: return "DOC";
        case FILE_TYPE_DOCX: return "DOCX";
        case FILE_TYPE_XLSX: return "XLSX";
        case FILE_TYPE_PPTX: return "PPTX";
        case FILE_TYPE_ODT: return "ODT";
        case FILE_TYPE_ODS: return "ODS";
        case FILE_TYPE_ODP: return "ODP";
        case FILE_TYPE_EPUB: return "EPUB";
        case FILE_TYPE_TXT: return "TXT";
        case FILE_TYPE_JSON: return "JSON";
        case FILE_TYPE_XML: return "XML";
        case FILE_TYPE_CSV: return "CSV";
        case FILE_TYPE_CODE: return "CODE";
        case FILE_TYPE_MARKDOWN: return "MARKDOWN";
        case FILE_TYPE_YAML: return "YAML";
        case FILE_TYPE_TOML: return "TOML";
        case FILE_TYPE_SQL: return "SQL";
        case FILE_TYPE_LATEX: return "LATEX";
        case FILE_TYPE_EML: return "EML";
        case FILE_TYPE_IMAGE: return "IMAGE";
        case FILE_TYPE_ARCHIVE: return "ARCHIVE";
        case FILE_TYPE_RTF: return "RTF";
        default: return "UNKNOWN";
    }
}

/**
 * Extract text from PDF using pdftotext
 */
int extract_text_from_pdf(const char* filepath, char* output_text, size_t output_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "pdftotext -layout -nopgbrk '%s' -", filepath);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to run pdftotext\n");
        return -1;
    }
    
    size_t bytes_read = fread(output_text, 1, output_size - 1, fp);
    output_text[bytes_read] = '\0';
    
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "pdftotext failed with status %d\n", status);
        return -1;
    }
    
    return bytes_read;
}

/**
 * Extract text from DOC using antiword
 */
int extract_text_from_doc(const char* filepath, char* output_text, size_t output_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "antiword '%s'", filepath);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return -1;
    
    size_t bytes_read = fread(output_text, 1, output_size - 1, fp);
    output_text[bytes_read] = '\0';
    pclose(fp);
    
    return bytes_read;
}

/**
 * Extract text from RTF using unrtf
 */
int extract_text_from_rtf(const char* filepath, char* output_text, size_t output_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "unrtf --text '%s'", filepath);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return -1;
    
    size_t bytes_read = fread(output_text, 1, output_size - 1, fp);
    output_text[bytes_read] = '\0';
    pclose(fp);
    
    return bytes_read;
}

/**
 * Extract text using Python universal extractor
 */
int extract_text_with_python(const char* filepath, char* output_text, size_t output_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "python3 src/crawler/universal_extractor.py '%s'", filepath);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return -1;
    
    size_t bytes_read = fread(output_text, 1, output_size - 1, fp);
    output_text[bytes_read] = '\0';
    
    int status = pclose(fp);
    if (status != 0) {
        return -1;
    }
    
    return bytes_read;
}

/**
 * Extract text from image using OCR
 */
int extract_text_from_image_ocr(const char* filepath, char* output_text, size_t output_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "tesseract '%s' stdout 2>/dev/null", filepath);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return -1;
    
    size_t bytes_read = fread(output_text, 1, output_size - 1, fp);
    output_text[bytes_read] = '\0';
    pclose(fp);
    
    return bytes_read;
}

/**
 * Process archive (extract and process contents)
 */
int process_archive(const char* filepath, char* output_text, size_t output_size) {
    // Create temp directory
    char temp_dir[1024];
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/crawler_extract_%d", getpid());
    mkdir(temp_dir, 0755);
    
    // Extract based on type
    char cmd[2048];
    if (strstr(filepath, ".zip")) {
        snprintf(cmd, sizeof(cmd), "unzip -q '%s' -d '%s' 2>/dev/null", filepath, temp_dir);
    } else if (strstr(filepath, ".tar.gz") || strstr(filepath, ".tgz")) {
        snprintf(cmd, sizeof(cmd), "tar -xzf '%s' -C '%s' 2>/dev/null", filepath, temp_dir);
    } else if (strstr(filepath, ".tar")) {
        snprintf(cmd, sizeof(cmd), "tar -xf '%s' -C '%s' 2>/dev/null", filepath, temp_dir);
    } else {
        return -1;  // Unsupported archive type
    }
    
    int status = system(cmd);
    if (status != 0) {
        snprintf(cmd, sizeof(cmd), "rm -rf '%s'", temp_dir);
        int cleanup_ret = system(cmd);
        (void)cleanup_ret; // Ignore cleanup result
        return -1;
    }
    
    // Process all text files in extracted directory
    snprintf(cmd, sizeof(cmd), "find '%s' -type f \\( -name '*.txt' -o -name '*.md' -o -name '*.c' -o -name '*.h' -o -name '*.py' \\) -exec cat {} \\;", temp_dir);
    
    FILE* fp = popen(cmd, "r");
    if (fp) {
        size_t bytes_read = fread(output_text, 1, output_size - 1, fp);
        output_text[bytes_read] = '\0';
        pclose(fp);
    }
    
    // Cleanup
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", temp_dir);
    int cleanup_ret = system(cmd);
    (void)cleanup_ret; // Suppress unused result warning
    
    return strlen(output_text);
}

/**
 * Process JSON file (extract text values)
 */
int process_json(const char* filepath, char* output_text, size_t output_size) {
    // Use jq to extract all string values
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "jq -r '.. | strings' '%s' 2>/dev/null", filepath);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return -1;
    
    size_t bytes_read = fread(output_text, 1, output_size - 1, fp);
    output_text[bytes_read] = '\0';
    pclose(fp);
    
    return bytes_read;
}

/**
 * Main file processor - dispatches to appropriate handler
 */
int process_file_by_type(const char* filepath, FileType type, char* output_text, size_t output_size) {
    printf("Processing file type: %s\n", get_file_type_name(type));
    
    switch (type) {
        case FILE_TYPE_PDF:
            return extract_text_from_pdf(filepath, output_text, output_size);
            
        case FILE_TYPE_DOC:
            return extract_text_from_doc(filepath, output_text, output_size);
            
        case FILE_TYPE_RTF:
            return extract_text_from_rtf(filepath, output_text, output_size);
            
        case FILE_TYPE_IMAGE:
            return extract_text_from_image_ocr(filepath, output_text, output_size);
            
        case FILE_TYPE_ARCHIVE:
            return process_archive(filepath, output_text, output_size);
            
        case FILE_TYPE_JSON:
            return process_json(filepath, output_text, output_size);
            
        // Use Python extractor for Office and other formats
        case FILE_TYPE_DOCX:
        case FILE_TYPE_XLSX:
        case FILE_TYPE_PPTX:
        case FILE_TYPE_ODT:
        case FILE_TYPE_ODS:
        case FILE_TYPE_ODP:
        case FILE_TYPE_EPUB:
        case FILE_TYPE_MARKDOWN:
        case FILE_TYPE_YAML:
        case FILE_TYPE_TOML:
        case FILE_TYPE_SQL:
        case FILE_TYPE_LATEX:
        case FILE_TYPE_EML:
            return extract_text_with_python(filepath, output_text, output_size);
            
        case FILE_TYPE_TXT:
        case FILE_TYPE_CODE:
        case FILE_TYPE_XML:
        case FILE_TYPE_CSV:
        case FILE_TYPE_HTML:
        default:
            // Just read the file directly
            FILE* fp = fopen(filepath, "r");
            if (!fp) return -1;
            size_t bytes = fread(output_text, 1, output_size - 1, fp);
            output_text[bytes] = '\0';
            fclose(fp);
            return bytes;
    }
}