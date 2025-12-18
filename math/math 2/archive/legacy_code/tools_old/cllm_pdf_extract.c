/**
 * CLLM PDF Text Extraction Tool
 * 
 * Extracts text from PDF files, with optional OCR support for scanned documents.
 */

#include "../include/cllm_pdf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <pdf_file>\n\n", program_name);
    printf("Extract text from PDF files.\n\n");
    printf("Options:\n");
    printf("  -o, --output FILE     Write output to FILE (default: stdout)\n");
    printf("  -r, --ocr             Enable OCR for scanned/image-based PDFs\n");
    printf("  -m, --metadata        Show PDF metadata\n");
    printf("  -p, --page NUM        Extract specific page only\n");
    printf("  -t, --type            Detect and show PDF type\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s document.pdf\n", program_name);
    printf("  %s -r scanned.pdf -o output.txt\n", program_name);
    printf("  %s -m -t document.pdf\n", program_name);
    printf("  %s -p 5 document.pdf\n", program_name);
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
    bool use_ocr = false;
    bool show_metadata = false;
    bool show_type = false;
    int specific_page = -1;

    // Parse command-line options
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"ocr", no_argument, 0, 'r'},
        {"metadata", no_argument, 0, 'm'},
        {"page", required_argument, 0, 'p'},
        {"type", no_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "o:rmp:th", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_path = optarg;
                break;
            case 'r':
                use_ocr = true;
                break;
            case 'm':
                show_metadata = true;
                break;
            case 'p':
                specific_page = atoi(optarg);
                break;
            case 't':
                show_type = true;
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

    // Show metadata if requested
    if (show_metadata || show_type) {
        PDFMetadata metadata;
        if (cllm_pdf_get_metadata(pdf_path, &metadata) == 0) {
            if (show_metadata) {
                printf("PDF Metadata:\n");
                printf("  Title: %s\n", metadata.title[0] ? metadata.title : "(none)");
                printf("  Author: %s\n", metadata.author[0] ? metadata.author : "(none)");
                printf("  Pages: %d\n", metadata.page_count);
            }
            if (show_type) {
                printf("  Type: %s\n", pdf_type_to_string(metadata.type));
            }
            if (show_metadata || show_type) {
                printf("\n");
            }
        } else {
            fprintf(stderr, "Error: Failed to read PDF metadata\n");
            return 1;
        }
    }

    // Extract text
    FILE* output_file = stdout;
    if (output_path) {
        output_file = fopen(output_path, "w");
        if (!output_file) {
            fprintf(stderr, "Error: Failed to open output file: %s\n", output_path);
            return 1;
        }
    }

    if (specific_page > 0) {
        // Extract specific page
        char* text = cllm_pdf_extract_page(pdf_path, specific_page, use_ocr);
        if (text) {
            fprintf(output_file, "%s", text);
            free(text);
        } else {
            fprintf(stderr, "Error: Failed to extract page %d\n", specific_page);
            if (output_file != stdout) fclose(output_file);
            return 1;
        }
    } else {
        // Extract entire document
        PDFExtractionResult result = cllm_pdf_extract_text(pdf_path, use_ocr);
        
        if (result.error_code == 0 && result.text) {
            fprintf(output_file, "%s", result.text);
            cllm_pdf_free_result(&result);
        } else {
            fprintf(stderr, "Error: %s\n", result.error_message);
            if (output_file != stdout) fclose(output_file);
            return 1;
        }
    }

    if (output_file != stdout) {
        fclose(output_file);
        printf("Text extracted to: %s\n", output_path);
    }

    return 0;
}
