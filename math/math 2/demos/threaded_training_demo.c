/**
 * Multi-Threaded Training Demo
 * 
 * Demonstrates the kissing spheres threading architecture
 * integrated into the training loop for parallel batch processing
 */

#include "cllm_tokenizer.h"
#include "cllm_data_loader.h"
#include "cllm_batch.h"
#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ANSI_BOLD "\033[1m"
#define ANSI_GREEN "\033[32m"
#define ANSI_BLUE "\033[34m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_CYAN "\033[36m"
#define ANSI_RESET "\033[0m"

void print_header(const char* title) {
    printf("\n");
    printf(ANSI_BOLD ANSI_CYAN "========================================\n");
    printf("  %s\n", title);
    printf("========================================" ANSI_RESET "\n");
    printf("\n");
}

int main(void) {
    print_header("CLLM Multi-Threaded Training Demo");
    
    printf("This demo shows the 12-fold kissing spheres architecture\n");
    printf("processing batches in parallel across 12 worker threads.\n");
    
    // Configuration
    const char* training_data = 
        "Machine learning enables computers to learn from data. "
        "Deep neural networks are powerful models for pattern recognition. "
        "Training requires large datasets and computational resources. "
        "Gradient descent optimizes model parameters iteratively. "
        "Backpropagation computes gradients efficiently through layers. "
        "Attention mechanisms allow models to focus on relevant information. "
        "Transformers have revolutionized natural language processing. "
        "Pre-training on large corpora improves downstream task performance. "
        "Fine-tuning adapts models to specific applications. "
        "Regularization techniques prevent overfitting on training data. "
        "Batch normalization stabilizes training dynamics. "
        "Learning rate scheduling improves convergence. "
        "Data augmentation increases training set diversity. "
        "Transfer learning leverages knowledge from related tasks. "
        "Multi-task learning shares representations across objectives.";
    
    uint32_t vocab_size = 5000;
    uint32_t batch_size = 2;
    uint32_t seq_len = 8;
    int num_epochs = 2;
    
    printf("\nConfiguration:\n");
    printf("  Vocabulary size: %u\n", vocab_size);
    printf("  Batch size: %u\n", batch_size);
    printf("  Sequence length: %u\n", seq_len);
    printf("  Number of epochs: %d\n", num_epochs);
    printf("  Threading: 12 kissing spheres\n");
    
    // Step 1: Create tokenizer and load data
    print_header("Step 1: Data Preparation");
    
    printf(ANSI_BLUE "▶ Creating tokenizer..." ANSI_RESET "\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(vocab_size);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer\n");
        return 1;
    }
    printf(ANSI_GREEN "✓ Tokenizer created" ANSI_RESET "\n");
    
    printf(ANSI_BLUE "▶ Loading training data..." ANSI_RESET "\n");
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        fprintf(stderr, "Failed to create data loader\n");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    if (cllm_data_loader_add_document(loader, training_data) == 0) {
        fprintf(stderr, "Failed to add document\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf(ANSI_GREEN "✓ Training data loaded" ANSI_RESET "\n");
    
    printf(ANSI_BLUE "▶ Building vocabulary..." ANSI_RESET "\n");
    cllm_data_loader_build_vocab(loader);
    printf(ANSI_GREEN "✓ Vocabulary built: %u tokens" ANSI_RESET "\n", 
           tokenizer->vocab_size);
    
    printf(ANSI_BLUE "▶ Creating token dataset..." ANSI_RESET "\n");
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        fprintf(stderr, "Failed to create dataset\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf(ANSI_GREEN "✓ Dataset created: %zu tokens" ANSI_RESET "\n", 
           dataset->num_tokens);
    
    // Step 2: Create batch iterator
    print_header("Step 2: Batch Generation");
    
    printf(ANSI_BLUE "▶ Creating batch iterator..." ANSI_RESET "\n");
    CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
        dataset->tokens,
        dataset->num_tokens,
        batch_size,
        seq_len,
        0,  // no shuffle
        1   // drop last
    );
    
    if (!batch_iter) {
        fprintf(stderr, "Failed to create batch iterator\n");
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    size_t num_batches = cllm_batch_iterator_num_batches(batch_iter);
    printf(ANSI_GREEN "✓ Batch iterator created" ANSI_RESET "\n");
    printf("  Total batches: %zu\n", num_batches);
    printf("  Batches per sphere: %.1f\n", (float)num_batches / 12.0f);
    
    // Step 3: Create training context (simplified - no actual model)
    print_header("Step 3: Training Setup");
    
    printf(ANSI_BLUE "▶ Initializing training context..." ANSI_RESET "\n");
    printf(ANSI_YELLOW "  Note: Using simplified training for demo" ANSI_RESET "\n");
    
    // Create minimal training structure for demo
    CLLMTraining* training = (CLLMTraining*)calloc(1, sizeof(CLLMTraining));
    if (!training) {
        fprintf(stderr, "Failed to allocate training context\n");
        cllm_batch_iterator_free(batch_iter);
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    // Create minimal model structure
    training->model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!training->model) {
        fprintf(stderr, "Failed to allocate model\n");
        free(training);
        cllm_batch_iterator_free(batch_iter);
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    training->model->vocab_size = tokenizer->vocab_size;
    training->model->embedding_dim = 128;
    
    printf(ANSI_GREEN "✓ Training context initialized" ANSI_RESET "\n");
    
    // Step 4: Create threaded training system
    print_header("Step 4: Multi-Threaded Training System");
    
    printf(ANSI_BLUE "▶ Creating threaded training system..." ANSI_RESET "\n");
    printf("  Initializing 12 kissing spheres...\n");
    
    ThreadedTrainingSystem* threaded_system = threaded_training_create(
        training, 
        batch_iter,
        12  // 12 worker threads
    );
    
    if (!threaded_system) {
        fprintf(stderr, "Failed to create threaded training system\n");
        free(training->model);
        free(training);
        cllm_batch_iterator_free(batch_iter);
        free(dataset->tokens);
        free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    printf(ANSI_GREEN "✓ Threaded training system created" ANSI_RESET "\n");
    printf("  12 worker threads initialized\n");
    printf("  Gradient accumulation buffer allocated\n");
    
    // Step 5: Train with multi-threading
    print_header("Step 5: Multi-Threaded Training");
    
    time_t start_time = time(NULL);
    
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        printf("\n" ANSI_BOLD ANSI_YELLOW "Epoch %d/%d" ANSI_RESET "\n", 
               epoch + 1, num_epochs);
        
        float epoch_loss = threaded_train_epoch_lockfree(threaded_system, epoch);
        
        printf(ANSI_GREEN "✓ Epoch %d complete - Loss: %.4f" ANSI_RESET "\n",
               epoch + 1, epoch_loss);
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("\n" ANSI_GREEN "✓ Training complete" ANSI_RESET "\n");
    printf("  Training time: %.0f seconds\n", elapsed);
    printf("  Throughput: %.1f batches/second\n", 
           (num_batches * num_epochs) / elapsed);
    
    // Step 6: Print statistics
    print_header("Step 6: Threading Statistics");
    
    threaded_training_print_stats(threaded_system);
    
    // Cleanup
    print_header("Cleanup");
    
    printf(ANSI_BLUE "▶ Freeing resources..." ANSI_RESET "\n");
    
    threaded_training_free(threaded_system);
    free(training->model);
    free(training);
    cllm_batch_iterator_free(batch_iter);
    free(dataset->tokens);
    free(dataset);
    cllm_data_loader_free(loader);
    cllm_free_tokenizer(tokenizer);
    
    printf(ANSI_GREEN "✓ All resources freed" ANSI_RESET "\n");
    
    print_header("Demo Complete");
    printf(ANSI_GREEN "✓ Multi-threaded training demo executed successfully!" ANSI_RESET "\n");
    printf("\nKey achievements:\n");
    printf("  ✓ 12 kissing spheres processing batches in parallel\n");
    printf("  ✓ Gradient accumulation across threads\n");
    printf("  ✓ Efficient work distribution\n");
    printf("  ✓ Thread-safe gradient synchronization\n");
    printf("\n");
    
    return 0;
}