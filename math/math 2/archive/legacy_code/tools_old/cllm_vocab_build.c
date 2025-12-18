/**
 * CLLM Vocabulary Builder Tool
 * 
 * Builds vocabulary from text corpus using CLLMTokenizer.
 * Updated to use current API.
 */

#include "../include/cllm_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <input_path>\n\n", program_name);
    printf("Build vocabulary from text corpus.\n\n");
    printf("Options:\n");
    printf("  -o, --output FILE     Output vocabulary file (default: vocab.txt)\n");
    printf("  -s, --size NUM        Maximum vocabulary size (default: 50000)\n");
    printf("  -r, --recursive       Process directories recursively\n");
    printf("  -e, --ext EXT         File extension filter (e.g., .txt)\n");
    printf("  -v, --verbose         Show processing details\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Input can be:\n");
    printf("  - Single text file\n");
    printf("  - Directory of text files\n");
    printf("  - Multiple files (space-separated)\n\n");
    printf("Examples:\n");
    printf("  %s corpus.txt\n", program_name);
    printf("  %s -r -e .txt data/ -o vocab.txt\n", program_name);
    printf("  %s file1.txt file2.txt file3.txt -s 10000\n", program_name);
}

static bool is_text_file(const char* path, const char* ext_filter) {
    if (!path) return false;
    
    struct stat st;
    if (stat(path, &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    
    if (ext_filter) {
        const char* ext = strrchr(path, '.');
        if (!ext || strcmp(ext, ext_filter) != 0) {
            return false;
        }
    }
    
    return true;
}

static int process_file(const char* path, CLLMTokenizer* tokenizer, bool verbose) {
    if (verbose) {
        printf("Processing: %s\n", path);
    }
    
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Warning: Failed to open %s\n", path);
        return -1;
    }
    
    // Read entire file
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* content = malloc(size + 1);
    if (!content) {
        fclose(f);
        return -1;
    }
    
    size_t read = fread(content, 1, size, f);
    content[read] = '\0';
    fclose(f);
    
    // Build vocabulary from this text
    cllm_build_vocab(tokenizer, content);
    
    free(content);
    
    if (verbose) {
        printf("  Processed %ld bytes\n", size);
    }
    
    return 0;
}

static int process_directory(const char* path, CLLMTokenizer* tokenizer,
                            bool recursive, const char* ext_filter, bool verbose) {
    DIR* dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Error: Failed to open directory %s\n", path);
        return -1;
    }
    
    struct dirent* entry;
    int file_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) != 0) continue;
        
        if (S_ISDIR(st.st_mode)) {
            if (recursive) {
                int count = process_directory(full_path, tokenizer, recursive, ext_filter, verbose);
                if (count > 0) file_count += count;
            }
        } else if (S_ISREG(st.st_mode)) {
            if (is_text_file(full_path, ext_filter)) {
                if (process_file(full_path, tokenizer, verbose) == 0) {
                    file_count++;
                }
            }
        }
    }
    
    closedir(dir);
    return file_count;
}

int main(int argc, char* argv[]) {
    const char* output_path = "vocab.txt";
    const char* ext_filter = NULL;
    uint32_t vocab_size = 50000;
    bool recursive = false;
    bool verbose = false;

    // Parse command-line options
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"size", required_argument, 0, 's'},
        {"recursive", no_argument, 0, 'r'},
        {"ext", required_argument, 0, 'e'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "o:s:re:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_path = optarg;
                break;
            case 's':
                vocab_size = (uint32_t)atoi(optarg);
                break;
            case 'r':
                recursive = true;
                break;
            case 'e':
                ext_filter = optarg;
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

    // Validate input
    if (optind >= argc) {
        fprintf(stderr, "Error: Input path required\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Create tokenizer
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(vocab_size);
    if (!tokenizer) {
        fprintf(stderr, "Error: Failed to create tokenizer\n");
        return 1;
    }

    if (verbose) {
        printf("Building vocabulary (max size: %u)\n", vocab_size);
        printf("Output: %s\n\n", output_path);
    }

    // Process all input paths
    int total_files = 0;
    for (int i = optind; i < argc; i++) {
        const char* input_path = argv[i];
        
        struct stat st;
        if (stat(input_path, &st) != 0) {
            fprintf(stderr, "Error: Cannot access %s\n", input_path);
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            int count = process_directory(input_path, tokenizer, recursive, ext_filter, verbose);
            if (count > 0) total_files += count;
        } else if (S_ISREG(st.st_mode)) {
            if (process_file(input_path, tokenizer, verbose) == 0) {
                total_files++;
            }
        }
    }

    if (total_files == 0) {
        fprintf(stderr, "Error: No files processed\n");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }

    // Save vocabulary
    if (verbose) {
        printf("\nSaving vocabulary to %s\n", output_path);
    }

    if (cllm_save_vocab(tokenizer, output_path) == 0) {
        fprintf(stderr, "Error: Failed to save vocabulary\n");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }

    // Print statistics
    uint32_t final_vocab_size = cllm_get_vocab_size(tokenizer);
    
    if (verbose) {
        printf("\n=== Vocabulary Statistics ===\n");
        printf("Files processed: %d\n", total_files);
        printf("Final vocabulary size: %u\n", final_vocab_size);
        printf("Output file: %s\n", output_path);
        
        cllm_print_vocab_stats(tokenizer);
    } else {
        printf("Vocabulary built: %u tokens from %d files\n", final_vocab_size, total_files);
        printf("Saved to: %s\n", output_path);
    }

    cllm_free_tokenizer(tokenizer);
    return 0;
}
