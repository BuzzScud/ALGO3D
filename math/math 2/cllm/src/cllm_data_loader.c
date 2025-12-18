/**
 * CLLM Data Loader
 * 
 * Comprehensive data loading and preprocessing for CLLM training
 * Supports:
 * - Text files (.txt)
 * - JSON documents
 * - Web scraping results
 * - PDF extraction (via external tools)
 * - Batch processing
 * - Data augmentation
 */

#include "ai/cllm.h"
#include "ai/cllm_tokenizer.h"
#include "ai/cllm_progress.h"
#include "ai/cllm_global_progress.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_LINE_LENGTH 65536
#define MAX_DOCUMENT_SIZE (100 * 1024 * 1024)  // 100MB max per document

/**
 * Data Loader Structure
 */
typedef struct {
    CLLMTokenizer* tokenizer;
    char** documents;
    size_t num_documents;
    size_t capacity;
    
    // Statistics
    size_t total_chars;
    size_t total_tokens;
    size_t total_lines;
    
    // Configuration
    int min_token_length;
    int max_token_length;
    int lowercase;
    int remove_punctuation;
    int remove_numbers;
} CLLMDataLoader;

/**
 * Create Data Loader
 */
CLLMDataLoader* cllm_data_loader_create(CLLMTokenizer* tokenizer) {
    if (!tokenizer) return NULL;
    
    CLLMDataLoader* loader = (CLLMDataLoader*)calloc(1, sizeof(CLLMDataLoader));
    if (!loader) return NULL;
    
    loader->tokenizer = tokenizer;
    loader->capacity = 1000;
    loader->documents = (char**)calloc(loader->capacity, sizeof(char*));
    
    if (!loader->documents) {
        free(loader);
        return NULL;
    }
    
    // Default configuration
    loader->min_token_length = 1;
    loader->max_token_length = 50;
    loader->lowercase = 1;
    loader->remove_punctuation = 0;
    loader->remove_numbers = 0;
    
    return loader;
}

/**
 * Free Data Loader
 */
void cllm_data_loader_free(CLLMDataLoader* loader) {
    if (!loader) return;
    
    if (loader->documents) {
        for (size_t i = 0; i < loader->num_documents; i++) {
            free(loader->documents[i]);
        }
        free(loader->documents);
    }
    
    free(loader);
}

/**
 * Clean Text
 * Applies preprocessing rules
 */
static char* clean_text(const char* text, CLLMDataLoader* loader) {
    if (!text) return NULL;
    
    size_t len = strlen(text);
    char* cleaned = (char*)malloc(len + 1);
    if (!cleaned) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        
        // Convert to lowercase if enabled
        if (loader->lowercase) {
            c = tolower(c);
        }
        
        // Remove punctuation if enabled
        if (loader->remove_punctuation && ispunct(c)) {
            continue;
        }
        
        // Remove numbers if enabled
        if (loader->remove_numbers && isdigit(c)) {
            continue;
        }
        
        // Normalize whitespace
        if (isspace(c)) {
            // Skip multiple spaces
            if (j > 0 && cleaned[j-1] == ' ') {
                continue;
            }
            c = ' ';
        }
        
        cleaned[j++] = c;
    }
    
    cleaned[j] = '\0';
    return cleaned;
}

/**
 * Add Document
 */
int cllm_data_loader_add_document(CLLMDataLoader* loader, const char* text) {
    if (!loader || !text) return 0;
    
    // Expand capacity if needed
    if (loader->num_documents >= loader->capacity) {
        size_t new_capacity = loader->capacity * 2;
        char** new_docs = (char**)realloc(loader->documents, 
                                          new_capacity * sizeof(char*));
        if (!new_docs) return 0;
        
        loader->documents = new_docs;
        loader->capacity = new_capacity;
    }
    
    // Clean and store document
    char* cleaned = clean_text(text, loader);
    if (!cleaned) return 0;
    
    loader->documents[loader->num_documents++] = cleaned;
    loader->total_chars += strlen(cleaned);
    
    // Count lines
    for (const char* p = cleaned; *p; p++) {
        if (*p == '\n') loader->total_lines++;
    }
    
    return 1;
}

/**
 * Load Text File
 */
int cllm_data_loader_load_file(CLLMDataLoader* loader, const char* filename) {
    if (!loader || !filename) return 0;
    
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 0;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size > MAX_DOCUMENT_SIZE) {
        fprintf(stderr, "File too large: %s (%ld bytes)\n", filename, file_size);
        fclose(f);
        return 0;
    }
    
    // Read entire file
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(f);
        return 0;
    }
    
    size_t bytes_read = fread(content, 1, file_size, f);
    content[bytes_read] = '\0';
    fclose(f);
    
    // Add as document
    int result = cllm_data_loader_add_document(loader, content);
    free(content);
    
    if (result) {
        printf("Loaded: %s (%ld bytes)\n", filename, file_size);
    }
    
    return result;
}

/**
 * Load Directory
 * Recursively loads all .txt files from directory
 */
int cllm_data_loader_load_directory(CLLMDataLoader* loader, const char* dirname) {
    if (!loader || !dirname) return 0;
    
    DIR* dir = opendir(dirname);
    if (!dir) {
        fprintf(stderr, "Failed to open directory: %s\n", dirname);
        return 0;
    }
    
    int count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Build full path
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        
        // Check if it's a directory
        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recursively load subdirectory
                count += cllm_data_loader_load_directory(loader, path);
            } else if (S_ISREG(st.st_mode)) {
                // Load ALL file types (not just .txt)
                // Skip binary files and hidden files
                if (entry->d_name[0] != '.') {
                    // Skip common binary extensions
                    const char* ext = strrchr(entry->d_name, '.');
                    int is_binary = 0;
                    if (ext) {
                        if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                            strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                            strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                            strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                            strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                            is_binary = 1;
                        }
                    }
                    if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                        count++;
                    }
                }
            }
        }
    }
    
    closedir(dir);
    return count;
}

/**
 * Build Vocabulary from Loaded Documents
 */
// Forward declarations for thread-safe tokenizer
extern uint32_t cllm_add_token_threadsafe(CLLMTokenizer* tokenizer, const char* token);
extern void cllm_consolidate_vocabulary(CLLMTokenizer* tokenizer);

/**
 * Vocabulary Building with 12-Fold Symmetry Thread-Safe Tokenizer
 * 
 * MASTER PLAN COMPLIANCE:
 * - Uses 12-fold symmetry structure (12 vocabulary partitions)
 * - Each sphere adds tokens to hash-partitioned vocabulary
 * - Per-partition locks allow 12 concurrent additions
 * - Consolidates partitions after parallel phase
 */

typedef struct {
    CLLMTokenizer* tokenizer;
    const char** documents;
    size_t start_idx;
    size_t end_idx;
    int symmetry_group;  // 0-11
    _Atomic size_t* progress_counter;
} VocabSphereContext;

static void* vocab_sphere_worker(void* arg) {
    VocabSphereContext* ctx = (VocabSphereContext*)arg;
    
    // Debug output wrapped in CLLM_DEBUG (suppressed during normal operation)
    #ifdef CLLM_DEBUG
    printf("[Sphere %d] Processing documents %zu to %zu\n", 
           ctx->symmetry_group, ctx->start_idx, ctx->end_idx);
    #else
    (void)ctx->symmetry_group;  // Suppress unused warning
    #endif
    
    // Pre-allocate a large buffer for tokenization to reduce malloc contention
    // This is thread-local, so no contention between threads
    // Base buffer size aligned with mathematical framework's vector culmination
    // 144,000 bytes = 3 × 12³ × (250/9) from the lattice formula
    size_t buffer_size = 144000;  // Vector culmination point
    char* thread_buffer = (char*)malloc(buffer_size);
    if (!thread_buffer) {
        fprintf(stderr, "[Sphere %d] Failed to allocate thread buffer\n", ctx->symmetry_group);
        return NULL;
    }
    
    // Process assigned documents
    for (size_t i = ctx->start_idx; i < ctx->end_idx; i++) {
        const char* doc = ctx->documents[i];
        if (!doc) continue;
        
        // Copy document to thread-local buffer (avoid strdup malloc contention)
        size_t doc_len = strlen(doc);
        if (doc_len >= buffer_size) {
            // Document too large - use recursive 3-growth aligned with mathematical framework
            // Formula: 3^d growth (self-similarity from 3)
            size_t new_size = buffer_size;
            while (new_size < doc_len + 1024) {
                new_size *= 3;  // Recursive 3-growth: 3^d = 3^(d-1) · 3
            }
            
            char* new_buffer = (char*)realloc(thread_buffer, new_size);
            if (!new_buffer) {
                fprintf(stderr, "[Sphere %d] Document %zu too large (%zu bytes), failed to reallocate to %zu bytes, skipping\n", 
                        ctx->symmetry_group, i, doc_len, new_size);
                continue;
            }
            thread_buffer = new_buffer;
            buffer_size = new_size;
            // Suppress verbose output during progress tracking
            // fprintf(stderr, "[Sphere %d] Reallocated buffer using 3^d growth to %zu bytes for document %zu\n",
            //         ctx->symmetry_group, buffer_size, i);
        }
        
        memcpy(thread_buffer, doc, doc_len + 1);
        
        char* token = strtok(thread_buffer, " \t\n\r");
        
        while (token) {
            // Convert to lowercase
            for (char* p = token; *p; p++) {
                *p = tolower(*p);
            }
            
            // Add to thread-safe partitioned vocabulary
            // Hash determines partition, each partition has its own lock
            // NOTE: strdup is still called inside, but much less frequently
            cllm_add_token_threadsafe(ctx->tokenizer, token);
            
            token = strtok(NULL, " \t\n\r");
        }
        
        // Update progress
        atomic_fetch_add(ctx->progress_counter, 1);
    }
    
    free(thread_buffer);
    
    // Suppress verbose output during progress tracking
    // printf("[Sphere %d] Completed processing\n", ctx->symmetry_group);
    return NULL;
}

/**
 * Build vocabulary using 12-fold symmetry with thread-safe tokenizer
 * 
 * MASTER PLAN COMPLIANCE:
 * - Creates 12 sphere workers (one per symmetry group)
 * - Each sphere adds tokens to hash-partitioned vocabulary
 * - 12 partitions allow 12 concurrent additions
 * - Consolidates partitions into single vocabulary after parallel phase
 */
void cllm_data_loader_build_vocab(CLLMDataLoader* loader) {
    if (!loader || !loader->tokenizer) return;
    
    printf("Building vocabulary from %zu documents...\n", loader->num_documents);
    printf("Using 12-fold symmetry thread-safe tokenizer\n");
    
    // Determine number of active spheres (up to 12)
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus < 1) num_cpus = 1;
    
    // MASTER PLAN: Use 12-fold symmetry structure
    int num_active_spheres = (num_cpus > 12) ? 12 : num_cpus;
    
    printf("Activating %d of 12 symmetry spheres\n", num_active_spheres);
    
    // Start vocabulary building phase in global progress
    cllm_global_progress_start_phase(CLLM_PHASE_BUILDING_VOCAB, "Building Vocabulary", loader->num_documents);
    
    // Create progress counter
    _Atomic size_t progress_counter = 0;
    
    // Create sphere contexts (12-fold symmetry)
    VocabSphereContext contexts[12];
    pthread_t threads[12];
    
    // Distribute documents across active spheres
    size_t docs_per_sphere = loader->num_documents / num_active_spheres;
    size_t remainder = loader->num_documents % num_active_spheres;
    
    size_t current_idx = 0;
    for (int i = 0; i < num_active_spheres; i++) {
        contexts[i].tokenizer = loader->tokenizer;
        contexts[i].documents = (const char**)loader->documents;
        contexts[i].start_idx = current_idx;
        contexts[i].end_idx = current_idx + docs_per_sphere + (i < (int)remainder ? 1 : 0);
        contexts[i].symmetry_group = i;  // 0-11
        contexts[i].progress_counter = &progress_counter;
        
        pthread_create(&threads[i], NULL, vocab_sphere_worker, &contexts[i]);
        
        current_idx = contexts[i].end_idx;
    }
    
    // Monitor progress
    size_t last_progress = 0;
    while (atomic_load(&progress_counter) < loader->num_documents) {
        size_t current_progress = atomic_load(&progress_counter);
        if (current_progress != last_progress) {
            cllm_global_progress_update(current_progress);
            last_progress = current_progress;
        }
        usleep(100000);  // 100ms
    }
    
    // Wait for all spheres to complete
    for (int i = 0; i < num_active_spheres; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Consolidate 12 partitions into single vocabulary
    cllm_consolidate_vocabulary(loader->tokenizer);
    
    loader->total_tokens = 0;
    for (uint32_t i = 0; i < loader->tokenizer->vocab_size; i++) {
        loader->total_tokens += loader->tokenizer->token_counts[i];
    }
    
    // Complete vocabulary building phase
    cllm_global_progress_complete_phase();
}

/**
 * Create Training Dataset
 * Tokenizes all documents and creates training sequences
 */
typedef struct {
    uint32_t* tokens;
    size_t num_tokens;
    size_t capacity;
} TokenDataset;

// Thread-local token buffer for parallel tokenization
typedef struct {
    uint32_t* tokens;
    size_t num_tokens;
    size_t capacity;
} ThreadTokenBuffer;

// Worker context for parallel tokenization
typedef struct {
    CLLMDataLoader* loader;
    ThreadTokenBuffer* buffer;
    size_t start_doc;
    size_t end_doc;
    int thread_id;
    _Atomic size_t* progress_counter;
} TokenizeWorkerContext;

// Worker function for parallel tokenization
static void* tokenize_worker(void* arg) {
    TokenizeWorkerContext* ctx = (TokenizeWorkerContext*)arg;
    
    for (size_t i = ctx->start_doc; i < ctx->end_doc; i++) {
        uint32_t num_tokens;
        uint32_t* doc_tokens = cllm_tokenizer_encode(ctx->loader->tokenizer,
                                                     ctx->loader->documents[i],
                                                     &num_tokens);
        
        if (doc_tokens) {
            // Expand buffer if needed
            while (ctx->buffer->num_tokens + num_tokens > ctx->buffer->capacity) {
                ctx->buffer->capacity *= 2;
                uint32_t* new_tokens = (uint32_t*)realloc(ctx->buffer->tokens,
                                                          ctx->buffer->capacity * sizeof(uint32_t));
                if (!new_tokens) {
                    free(doc_tokens);
                    return NULL;
                }
                ctx->buffer->tokens = new_tokens;
            }
            
            // Copy tokens to thread buffer
            memcpy(ctx->buffer->tokens + ctx->buffer->num_tokens, doc_tokens,
                   num_tokens * sizeof(uint32_t));
            ctx->buffer->num_tokens += num_tokens;
            
            free(doc_tokens);
        }
        
        // Update progress
        size_t current = atomic_fetch_add(ctx->progress_counter, 1) + 1;
        cllm_global_progress_update(current);
    }
    
    return NULL;
}

TokenDataset* cllm_data_loader_create_dataset(CLLMDataLoader* loader) {
    if (!loader || !loader->tokenizer) return NULL;
    
    TokenDataset* dataset = (TokenDataset*)calloc(1, sizeof(TokenDataset));
    if (!dataset) return NULL;
    
    // Estimate capacity
    dataset->capacity = loader->total_chars / 4;  // Rough estimate
    dataset->tokens = (uint32_t*)malloc(dataset->capacity * sizeof(uint32_t));
    
    if (!dataset->tokens) {
        free(dataset);
        return NULL;
    }
    
    printf("Creating training dataset...\n");
    
    // Determine number of threads (up to 12 for 12-fold symmetry)
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus < 1) num_cpus = 1;
    if (num_cpus > 12) num_cpus = 12;
    
    // Check if parallel processing is worthwhile
    if (loader->num_documents < 100 || num_cpus == 1) {
        // Single-threaded for small datasets
        printf("Using single-threaded tokenization (%zu documents)\n", loader->num_documents);
        
        for (size_t i = 0; i < loader->num_documents; i++) {
            uint32_t num_tokens;
            uint32_t* doc_tokens = cllm_tokenizer_encode(loader->tokenizer, 
                                                         loader->documents[i], 
                                                         &num_tokens);
            
            if (doc_tokens) {
                // Expand capacity if needed
                while (dataset->num_tokens + num_tokens > dataset->capacity) {
                    dataset->capacity *= 2;
                    uint32_t* new_tokens = (uint32_t*)realloc(dataset->tokens,
                                                              dataset->capacity * sizeof(uint32_t));
                    if (!new_tokens) {
                        free(doc_tokens);
                        free(dataset->tokens);
                        free(dataset);
                        return NULL;
                    }
                    dataset->tokens = new_tokens;
                }
                
                // Copy tokens
                memcpy(dataset->tokens + dataset->num_tokens, doc_tokens, 
                       num_tokens * sizeof(uint32_t));
                dataset->num_tokens += num_tokens;
                
                free(doc_tokens);
            }
            
            if ((i + 1) % 100 == 0) {
                printf("  Processed %zu/%zu documents\n", i + 1, loader->num_documents);
            }
        }
    } else {
        // Parallel tokenization using 12-fold symmetry
        printf("Using %d-thread parallel tokenization (%zu documents)\n", num_cpus, loader->num_documents);
        
        // Start tokenization phase in global progress
        cllm_global_progress_start_phase(CLLM_PHASE_TOKENIZING, "Tokenizing Documents", loader->num_documents);
        
        // Create per-thread token buffers
        ThreadTokenBuffer* thread_buffers = (ThreadTokenBuffer*)calloc(num_cpus, sizeof(ThreadTokenBuffer));
        if (!thread_buffers) {
            free(dataset->tokens);
            free(dataset);
            return NULL;
        }
        
        // Initialize thread buffers
        for (int t = 0; t < num_cpus; t++) {
            thread_buffers[t].capacity = loader->total_chars / (4 * num_cpus);
            thread_buffers[t].tokens = (uint32_t*)malloc(thread_buffers[t].capacity * sizeof(uint32_t));
            if (!thread_buffers[t].tokens) {
                for (int j = 0; j < t; j++) free(thread_buffers[j].tokens);
                free(thread_buffers);
                free(dataset->tokens);
                free(dataset);
                return NULL;
            }
            thread_buffers[t].num_tokens = 0;
        }
        
        // Thread worker contexts
        _Atomic size_t progress_counter = 0;
        pthread_t* threads = (pthread_t*)malloc(num_cpus * sizeof(pthread_t));
        TokenizeWorkerContext* contexts = (TokenizeWorkerContext*)malloc(num_cpus * sizeof(TokenizeWorkerContext));
        
        // Launch threads
        size_t docs_per_thread = loader->num_documents / num_cpus;
        for (int t = 0; t < num_cpus; t++) {
            contexts[t].loader = loader;
            contexts[t].buffer = &thread_buffers[t];
            contexts[t].start_doc = t * docs_per_thread;
            contexts[t].end_doc = (t == num_cpus - 1) ? loader->num_documents : (t + 1) * docs_per_thread;
            contexts[t].thread_id = t;
            contexts[t].progress_counter = &progress_counter;
            
            pthread_create(&threads[t], NULL, tokenize_worker, &contexts[t]);
        }
        
        // Wait for completion
        for (int t = 0; t < num_cpus; t++) {
            pthread_join(threads[t], NULL);
        }
        
        // Complete tokenization phase
        cllm_global_progress_complete_phase();
        
        // Merge thread buffers into final dataset
        
        // Calculate total tokens
        size_t total_tokens = 0;
        for (int t = 0; t < num_cpus; t++) {
            total_tokens += thread_buffers[t].num_tokens;
        }
        
        // Allocate final buffer
        free(dataset->tokens);
        dataset->tokens = (uint32_t*)malloc(total_tokens * sizeof(uint32_t));
        if (!dataset->tokens) {
            for (int t = 0; t < num_cpus; t++) free(thread_buffers[t].tokens);
            free(thread_buffers);
            free(threads);
            free(contexts);
            free(dataset);
            return NULL;
        }
        
        // Copy from thread buffers
        dataset->num_tokens = 0;
        for (int t = 0; t < num_cpus; t++) {
            memcpy(dataset->tokens + dataset->num_tokens, thread_buffers[t].tokens,
                   thread_buffers[t].num_tokens * sizeof(uint32_t));
            dataset->num_tokens += thread_buffers[t].num_tokens;
            free(thread_buffers[t].tokens);
        }
        
        free(thread_buffers);
        free(threads);
        free(contexts);
    }
    
    printf("Dataset created: %zu tokens\n", dataset->num_tokens);
    return dataset;
}

/**
 * Free Token Dataset
 */
void cllm_token_dataset_free(TokenDataset* dataset) {
    if (!dataset) return;
    free(dataset->tokens);
    free(dataset);
}

/**
 * Save Dataset to File
 */
int cllm_token_dataset_save(TokenDataset* dataset, const char* filename) {
    if (!dataset || !filename) return 0;
    
    FILE* f = fopen(filename, "wb");
    if (!f) return 0;
    
    // Write header
    fwrite(&dataset->num_tokens, sizeof(size_t), 1, f);
    
    // Write tokens
    fwrite(dataset->tokens, sizeof(uint32_t), dataset->num_tokens, f);
    
    fclose(f);
    
    printf("Dataset saved to: %s (%zu tokens)\n", filename, dataset->num_tokens);
    return 1;
}

/**
 * Load Dataset from File
 */
TokenDataset* cllm_token_dataset_load(const char* filename) {
    if (!filename) return NULL;
    
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    TokenDataset* dataset = (TokenDataset*)calloc(1, sizeof(TokenDataset));
    if (!dataset) {
        fclose(f);
        return NULL;
    }
    
    // Read header
    if (fread(&dataset->num_tokens, sizeof(size_t), 1, f) != 1) {
        free(dataset);
        fclose(f);
        return NULL;
    }
    
    // Allocate and read tokens
    dataset->capacity = dataset->num_tokens;
    dataset->tokens = (uint32_t*)malloc(dataset->capacity * sizeof(uint32_t));
    
    if (!dataset->tokens) {
        free(dataset);
        fclose(f);
        return NULL;
    }
    
    if (fread(dataset->tokens, sizeof(uint32_t), dataset->num_tokens, f) != dataset->num_tokens) {
        free(dataset->tokens);
        free(dataset);
        fclose(f);
        return NULL;
    }
    
    fclose(f);
    
    printf("Dataset loaded from: %s (%zu tokens)\n", filename, dataset->num_tokens);
    return dataset;
}

/**
 * Print Dataset Statistics
 */
void cllm_data_loader_print_stats(CLLMDataLoader* loader) {
    if (!loader) return;
    
    printf("\n=== Data Loader Statistics ===\n");
    printf("Documents: %zu\n", loader->num_documents);
    printf("Total characters: %zu\n", loader->total_chars);
    printf("Total lines: %zu\n", loader->total_lines);
    printf("Total tokens: %zu\n", loader->total_tokens);
    
    if (loader->num_documents > 0) {
        printf("Avg chars per document: %.1f\n", 
               (double)loader->total_chars / loader->num_documents);
    }
    
    if (loader->tokenizer) {
        printf("Vocabulary size: %u\n", loader->tokenizer->vocab_size);
    }
    
    printf("==============================\n\n");
}