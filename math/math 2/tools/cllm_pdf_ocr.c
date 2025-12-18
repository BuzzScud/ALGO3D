/**
 * CLLM PDF+OCR Combined Tool
 * 
 * Intelligently extracts text from PDFs, automatically applying OCR when needed.
 */

#include "cllm_pdf.h"
#include "cllm_ocr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <pdf_file>\n\n", program_name);
    printf("Intelligently extract text from PDFs with automatic OCR.\n\n");
    printf("Options:\n");
    printf("  -o, --output FILE     Write output to FILE (default: stdout)\n");
    printf("  -l, --lang LANG       OCR language for scanned pages (default: eng)\n");
    printf("  -f, --force-ocr       Force OCR even for text-based PDFs\n");
    printf("  -m, --metadata        Show PDF metadata and type detection\n");
    printf("  -v, --verbose         Show processing details\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("This tool automatically:\n");
    printf("  - Detects if PDF contains text or images\n");
    printf("  - Extracts text directly from text-based PDFs\n");
    printf("  - Applies OCR to image-based (scanned) PDFs\n");
    printf("  - Handles mixed PDFs with both text and images\n\n");
    printf("Examples:\n");
    printf("  %s document.pdf\n", program_name);
    printf("  %s -v scanned.pdf -o output.txt\n", program_name);
    printf("  %s -l fra -m french_doc.pdf\n", program_name);
    printf("  %s --force-ocr mixed.pdf\n", program_name);
}

static const char* pdf_type_to_string(PDFType type) {
    switch (type) {
        case PDF_TYPE_TEXT: return "Text-based PDF";
        case PDF_TYPE_IMAGE: return "Image-based PDF (scanned)";
        case PDF_TYPE_MIXED: return "Mixed PDF (text + images)";
        default: return "Unknown";
    }
}

int main(int argc, char* argv[]) {
    const char* pdf_path = NULL;
    const char* output_path = NULL;
    char ocr_language[64] = "eng";
    bool force_ocr = false;
    bool show_metadata = false;
    bool verbose = false;

    // Parse command-line options
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"lang", required_argument, 0, 'l'},
        {"force-ocr", no_argument, 0, 'f'},
        {"metadata", no_argument, 0, 'm'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "o:l:fmvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_path = optarg;
                break;
            case 'l':
                strncpy(ocr_language, optarg, sizeof(ocr_language) - 1);
                break;
            case 'f':
                force_ocr = true;
                break;
            case 'm':
                show_metadata = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Get PDF file path
    if (optind >= argc) {
        fprintf(stderr, "Error: PDF file path required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    pdf_path = argv[optind];

    // Check if OCR is available
    if (!cllm_ocr_is_available()) {
        fprintf(stderr, "Warning: Tesseract OCR is not available\n");
        fprintf(stderr, "Image-based PDFs cannot be processed without OCR\n");
        if (!force_ocr) {
            fprintf(stderr, "Continuing with text extraction only...\n\n");
        }
    }

    // Get and display metadata
    PDFMetadata metadata;
    if (cllm_pdf_get_metadata(pdf_path, &metadata) != 0) {
        fprintf(stderr, "Error: Failed to read PDF metadata\n");
        return 1;
    }

    if (show_metadata || verbose) {
        printf("=== PDF Information ===\n");
        printf("File: %s\n", pdf_path);
        printf("Title: %s\n", metadata.title[0] ? metadata.title : "(none)");
        printf("Author: %s\n", metadata.author[0] ? metadata.author : "(none)");
        printf("Pages: %d\n", metadata.page_count);
        printf("Type: %s\n", pdf_type_to_string(metadata.type));
        printf("\n");
    }

    // Determine processing strategy
    bool use_ocr = force_ocr || 
                   (metadata.type == PDF_TYPE_IMAGE) || 
                   (metadata.type == PDF_TYPE_MIXED);

    if (verbose) {
        printf("=== Processing Strategy ===\n");
        if (force_ocr) {
            printf("Mode: Forced OCR\n");
        } else if (metadata.type == PDF_TYPE_TEXT) {
            printf("Mode: Direct text extraction\n");
        } else if (metadata.type == PDF_TYPE_IMAGE) {
            printf("Mode: OCR (scanned document)\n");
        } else if (metadata.type == PDF_TYPE_MIXED) {
            printf("Mode: Hybrid (text extraction + OCR)\n");
        }
        
        if (use_ocr) {
            printf("OCR Language: %s\n", ocr_language);
        }
        printf("\n");
    }

    // Extract text
    if (verbose) {
        printf("=== Extracting Text ===\n");
    }

    PDFExtractionResult result = cllm_pdf_extract_text(pdf_path, use_ocr);

    if (result.error_code != 0) {
        fprintf(stderr, "Error: %s\n", result.error_message);
        return 1;
    }

    if (!result.text || result.text_length == 0) {
        fprintf(stderr, "Warning: No text extracted from PDF\n");
        cllm_pdf_free_result(&result);
        return 1;
    }

    if (verbose) {
        printf("Extracted %zu characters\n", result.text_length);
        printf("\n");
    }

    // Write output
    FILE* output_file = stdout;
    if (output_path) {
        output_file = fopen(output_path, "w");
        if (!output_file) {
            fprintf(stderr, "Error: Failed to open output file: %s\n", output_path);
            cllm_pdf_free_result(&result);
            return 1;
        }
    }

    fprintf(output_file, "%s", result.text);

    if (output_file != stdout) {
        fclose(output_file);
        printf("✓ Text extracted successfully\n");
        printf("  Output: %s\n", output_path);
        printf("  Size: %zu characters\n", result.text_length);
    }

    cllm_pdf_free_result(&result);
    return 0;
}