/**
 * Document Format Detection
 * 
 * Detects document format from file extension and content-type headers.
 * Pure C implementation with no external dependencies.
 */

#include "../docproc.h"
#include <string.h>
#include <strings.h>
#include <ctype.h>

/**
 * Get file extension from path
 */
static const char* get_extension(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (ext && ext != filepath) {
        return ext + 1;  // Skip the dot
    }
    return NULL;
}

/**
 * Case-insensitive string comparison
 */
static int strcasecmp_safe(const char* s1, const char* s2) {
    if (!s1 || !s2) return -1;
    return strcasecmp(s1, s2);
}

/**
 * Detect format from file extension
 */
static DocProcFormat detect_from_extension(const char* ext) {
    if (!ext) return DOCPROC_FORMAT_UNKNOWN;
    
    // Microsoft Office
    if (strcasecmp_safe(ext, "docx") == 0) return DOCPROC_FORMAT_DOCX;
    if (strcasecmp_safe(ext, "xlsx") == 0) return DOCPROC_FORMAT_XLSX;
    if (strcasecmp_safe(ext, "pptx") == 0) return DOCPROC_FORMAT_PPTX;
    if (strcasecmp_safe(ext, "doc") == 0) return DOCPROC_FORMAT_DOC;
    
    // OpenDocument
    if (strcasecmp_safe(ext, "odt") == 0) return DOCPROC_FORMAT_ODT;
    if (strcasecmp_safe(ext, "ods") == 0) return DOCPROC_FORMAT_ODS;
    if (strcasecmp_safe(ext, "odp") == 0) return DOCPROC_FORMAT_ODP;
    
    // eBooks
    if (strcasecmp_safe(ext, "epub") == 0) return DOCPROC_FORMAT_EPUB;
    
    // Documents
    if (strcasecmp_safe(ext, "pdf") == 0) return DOCPROC_FORMAT_PDF;
    if (strcasecmp_safe(ext, "rtf") == 0) return DOCPROC_FORMAT_RTF;
    if (strcasecmp_safe(ext, "txt") == 0) return DOCPROC_FORMAT_TXT;
    
    // Markup
    if (strcasecmp_safe(ext, "html") == 0) return DOCPROC_FORMAT_HTML;
    if (strcasecmp_safe(ext, "htm") == 0) return DOCPROC_FORMAT_HTML;
    if (strcasecmp_safe(ext, "xml") == 0) return DOCPROC_FORMAT_XML;
    if (strcasecmp_safe(ext, "md") == 0) return DOCPROC_FORMAT_MARKDOWN;
    if (strcasecmp_safe(ext, "markdown") == 0) return DOCPROC_FORMAT_MARKDOWN;
    
    // Data formats
    if (strcasecmp_safe(ext, "json") == 0) return DOCPROC_FORMAT_JSON;
    if (strcasecmp_safe(ext, "csv") == 0) return DOCPROC_FORMAT_CSV;
    if (strcasecmp_safe(ext, "yaml") == 0) return DOCPROC_FORMAT_YAML;
    if (strcasecmp_safe(ext, "yml") == 0) return DOCPROC_FORMAT_YAML;
    if (strcasecmp_safe(ext, "toml") == 0) return DOCPROC_FORMAT_TOML;
    
    // Code and SQL
    if (strcasecmp_safe(ext, "sql") == 0) return DOCPROC_FORMAT_SQL;
    if (strcasecmp_safe(ext, "c") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "h") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "cpp") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "hpp") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "py") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "js") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "java") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "go") == 0) return DOCPROC_FORMAT_CODE;
    if (strcasecmp_safe(ext, "rs") == 0) return DOCPROC_FORMAT_CODE;
    
    // LaTeX
    if (strcasecmp_safe(ext, "tex") == 0) return DOCPROC_FORMAT_LATEX;
    if (strcasecmp_safe(ext, "latex") == 0) return DOCPROC_FORMAT_LATEX;
    
    // Email
    if (strcasecmp_safe(ext, "eml") == 0) return DOCPROC_FORMAT_EML;
    if (strcasecmp_safe(ext, "msg") == 0) return DOCPROC_FORMAT_EML;
    
    // Images
    if (strcasecmp_safe(ext, "jpg") == 0) return DOCPROC_FORMAT_IMAGE;
    if (strcasecmp_safe(ext, "jpeg") == 0) return DOCPROC_FORMAT_IMAGE;
    if (strcasecmp_safe(ext, "png") == 0) return DOCPROC_FORMAT_IMAGE;
    if (strcasecmp_safe(ext, "gif") == 0) return DOCPROC_FORMAT_IMAGE;
    if (strcasecmp_safe(ext, "webp") == 0) return DOCPROC_FORMAT_IMAGE;
    if (strcasecmp_safe(ext, "bmp") == 0) return DOCPROC_FORMAT_IMAGE;
    if (strcasecmp_safe(ext, "tiff") == 0) return DOCPROC_FORMAT_IMAGE;
    if (strcasecmp_safe(ext, "tif") == 0) return DOCPROC_FORMAT_IMAGE;
    
    // Archives
    if (strcasecmp_safe(ext, "zip") == 0) return DOCPROC_FORMAT_ARCHIVE;
    if (strcasecmp_safe(ext, "tar") == 0) return DOCPROC_FORMAT_ARCHIVE;
    if (strcasecmp_safe(ext, "gz") == 0) return DOCPROC_FORMAT_ARCHIVE;
    if (strcasecmp_safe(ext, "tgz") == 0) return DOCPROC_FORMAT_ARCHIVE;
    if (strcasecmp_safe(ext, "bz2") == 0) return DOCPROC_FORMAT_ARCHIVE;
    if (strcasecmp_safe(ext, "xz") == 0) return DOCPROC_FORMAT_ARCHIVE;
    
    return DOCPROC_FORMAT_UNKNOWN;
}

/**
 * Detect format from content-type header
 */
static DocProcFormat detect_from_content_type(const char* content_type) {
    if (!content_type) return DOCPROC_FORMAT_UNKNOWN;
    
    // Microsoft Office
    if (strstr(content_type, "application/vnd.openxmlformats-officedocument.wordprocessingml"))
        return DOCPROC_FORMAT_DOCX;
    if (strstr(content_type, "application/vnd.openxmlformats-officedocument.spreadsheetml"))
        return DOCPROC_FORMAT_XLSX;
    if (strstr(content_type, "application/vnd.openxmlformats-officedocument.presentationml"))
        return DOCPROC_FORMAT_PPTX;
    if (strstr(content_type, "application/msword"))
        return DOCPROC_FORMAT_DOC;
    
    // OpenDocument
    if (strstr(content_type, "application/vnd.oasis.opendocument.text"))
        return DOCPROC_FORMAT_ODT;
    if (strstr(content_type, "application/vnd.oasis.opendocument.spreadsheet"))
        return DOCPROC_FORMAT_ODS;
    if (strstr(content_type, "application/vnd.oasis.opendocument.presentation"))
        return DOCPROC_FORMAT_ODP;
    
    // eBooks
    if (strstr(content_type, "application/epub+zip"))
        return DOCPROC_FORMAT_EPUB;
    
    // Documents
    if (strstr(content_type, "application/pdf"))
        return DOCPROC_FORMAT_PDF;
    if (strstr(content_type, "application/rtf"))
        return DOCPROC_FORMAT_RTF;
    if (strstr(content_type, "text/plain"))
        return DOCPROC_FORMAT_TXT;
    
    // Markup
    if (strstr(content_type, "text/html"))
        return DOCPROC_FORMAT_HTML;
    if (strstr(content_type, "application/xml") || strstr(content_type, "text/xml"))
        return DOCPROC_FORMAT_XML;
    if (strstr(content_type, "text/markdown"))
        return DOCPROC_FORMAT_MARKDOWN;
    
    // Data formats
    if (strstr(content_type, "application/json"))
        return DOCPROC_FORMAT_JSON;
    if (strstr(content_type, "text/csv"))
        return DOCPROC_FORMAT_CSV;
    
    // Images
    if (strstr(content_type, "image/"))
        return DOCPROC_FORMAT_IMAGE;
    
    // Archives
    if (strstr(content_type, "application/zip"))
        return DOCPROC_FORMAT_ARCHIVE;
    if (strstr(content_type, "application/x-tar"))
        return DOCPROC_FORMAT_ARCHIVE;
    if (strstr(content_type, "application/gzip"))
        return DOCPROC_FORMAT_ARCHIVE;
    
    return DOCPROC_FORMAT_UNKNOWN;
}

/**
 * Detect document format
 */
DocProcFormat docproc_detect_format(const char* filepath, const char* content_type) {
    if (!filepath) return DOCPROC_FORMAT_UNKNOWN;
    
    // Try extension first
    const char* ext = get_extension(filepath);
    DocProcFormat format = detect_from_extension(ext);
    
    if (format != DOCPROC_FORMAT_UNKNOWN) {
        return format;
    }
    
    // Try content-type if extension didn't work
    format = detect_from_content_type(content_type);
    
    if (format != DOCPROC_FORMAT_UNKNOWN) {
        return format;
    }
    
    // Default to HTML if nothing else matches
    return DOCPROC_FORMAT_HTML;
}

/**
 * Get format name as string
 */
const char* docproc_format_name(DocProcFormat format) {
    switch (format) {
        case DOCPROC_FORMAT_HTML: return "HTML";
        case DOCPROC_FORMAT_PDF: return "PDF";
        case DOCPROC_FORMAT_DOC: return "DOC";
        case DOCPROC_FORMAT_DOCX: return "DOCX";
        case DOCPROC_FORMAT_XLSX: return "XLSX";
        case DOCPROC_FORMAT_PPTX: return "PPTX";
        case DOCPROC_FORMAT_ODT: return "ODT";
        case DOCPROC_FORMAT_ODS: return "ODS";
        case DOCPROC_FORMAT_ODP: return "ODP";
        case DOCPROC_FORMAT_EPUB: return "EPUB";
        case DOCPROC_FORMAT_TXT: return "TXT";
        case DOCPROC_FORMAT_JSON: return "JSON";
        case DOCPROC_FORMAT_XML: return "XML";
        case DOCPROC_FORMAT_CSV: return "CSV";
        case DOCPROC_FORMAT_CODE: return "CODE";
        case DOCPROC_FORMAT_MARKDOWN: return "MARKDOWN";
        case DOCPROC_FORMAT_YAML: return "YAML";
        case DOCPROC_FORMAT_TOML: return "TOML";
        case DOCPROC_FORMAT_SQL: return "SQL";
        case DOCPROC_FORMAT_LATEX: return "LATEX";
        case DOCPROC_FORMAT_EML: return "EML";
        case DOCPROC_FORMAT_IMAGE: return "IMAGE";
        case DOCPROC_FORMAT_ARCHIVE: return "ARCHIVE";
        case DOCPROC_FORMAT_RTF: return "RTF";
        default: return "UNKNOWN";
    }
}

/**
 * Check if format is supported
 */
bool docproc_is_format_supported(DocProcFormat format) {
    return format != DOCPROC_FORMAT_UNKNOWN;
}

/**
 * Check if file extension is supported
 */
bool docproc_is_extension_supported(const char* extension) {
    if (!extension) return false;
    DocProcFormat format = detect_from_extension(extension);
    return format != DOCPROC_FORMAT_UNKNOWN;
}