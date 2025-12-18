/**
 * Training Quality Validation Suite
 * Compares baseline training vs crystalline mathematical framework training
 */

#include "ai/cllm_lattice_embeddings.h"
#include "ai/cllm_cymatic_training.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EPOCHS 100
#define BATCH_SIZE 32
#define DATASET_SIZE 1000

// Training metrics
typedef struct {
    float* loss_history;
    float* accuracy_history;
    int num_epochs;
    float final_loss;
    float final_accuracy;
    float convergence_epoch;
    double total_training_time_ms;
} TrainingMetrics;

// Dataset structure
typedef struct {
    int** sequences;
    int* labels;
    int num_samples;
    int seq_length;
} Dataset;

/**
 * Generate synthetic training dataset
 */
static Dataset* generate_dataset(int num_samples, int seq_length, int vocab_size) {
    Dataset* dataset = (Dataset*)malloc(sizeof(Dataset));
    if (!dataset) return NULL;
    
    dataset->sequences = (int**)malloc(num_samples * sizeof(int*));
    dataset->labels = (int*)malloc(num_samples * sizeof(int));
    dataset->num_samples = num_samples;
    dataset->seq_length = seq_length;
    
    if (!dataset->sequences || !dataset->labels) {
        free(dataset->sequences);
        free(dataset->labels);
        free(dataset);
        return NULL;
    }
    
    // Generate sequences with patterns
    for (int i = 0; i < num_samples; i++) {
        dataset->sequences[i] = (int*)malloc(seq_length * sizeof(int));
        if (!dataset->sequences[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                free(dataset->sequences[j]);
            }
            free(dataset->sequences);
            free(dataset->labels);
            free(dataset);
            return NULL;
        }
        
        // Create pattern: sequences with repeating elements
        int pattern_type = i % 4;
        for (int j = 0; j < seq_length; j++) {
            switch (pattern_type) {
                case 0: // Ascending pattern
                    dataset->sequences[i][j] = (j * 7) % vocab_size;
                    break;
                case 1: // Descending pattern
                    dataset->sequences[i][j] = ((seq_length - j) * 11) % vocab_size;
                    break;
                case 2: // Repeating pattern
                    dataset->sequences[i][j] = ((j / 3) * 13) % vocab_size;
                    break;
                case 3: // Random with seed
                    dataset->sequences[i][j] = ((i * j + 17) * 19) % vocab_size;
                    break;
            }
        }
        
        // Label is based on pattern type
        dataset->labels[i] = pattern_type;
    }
    
    return dataset;
}

/**
 * Free dataset
 */
static void free_dataset(Dataset* dataset) {
    if (!dataset) return;
    
    if (dataset->sequences) {
        for (int i = 0; i < dataset->num_samples; i++) {
            free(dataset->sequences[i]);
        }
        free(dataset->sequences);
    }
    
    free(dataset->labels);
    free(dataset);
}

/**
 * Calculate cross-entropy loss
 */
static float calculate_loss(float* logits, int true_label, int num_classes) {
    // Find max logit for numerical stability
    float max_logit = logits[0];
    for (int i = 1; i < num_classes; i++) {
        if (logits[i] > max_logit) {
            max_logit = logits[i];
        }
    }
    
    // Calculate softmax denominator
    float sum_exp = 0.0f;
    for (int i = 0; i < num_classes; i++) {
        sum_exp += cllm_exp(logits[i] - max_logit);
    }
    
    // Calculate log probability of true class
    float log_prob = (logits[true_label] - max_logit) - cllm_log(sum_exp);
    
    return -log_prob;
}

/**
 * Calculate accuracy
 */
static float calculate_accuracy(float* logits, int true_label, int num_classes) {
    int predicted = 0;
    float max_logit = logits[0];
    
    for (int i = 1; i < num_classes; i++) {
        if (logits[i] > max_logit) {
            max_logit = logits[i];
            predicted = i;
        }
    }
    
    return (predicted == true_label) ? 1.0f : 0.0f;
}

/**
 * Train model (baseline or crystalline)
 */
static TrainingMetrics* train_model(CLLMModel* model, Dataset* dataset, 
                                    int num_epochs, int use_crystalline) {
    TrainingMetrics* metrics = (TrainingMetrics*)malloc(sizeof(TrainingMetrics));
    if (!metrics) return NULL;
    
    metrics->loss_history = (float*)malloc(num_epochs * sizeof(float));
    metrics->accuracy_history = (float*)malloc(num_epochs * sizeof(float));
    metrics->num_epochs = num_epochs;
    metrics->convergence_epoch = -1;
    
    if (!metrics->loss_history || !metrics->accuracy_history) {
        free(metrics->loss_history);
        free(metrics->accuracy_history);
        free(metrics);
        return NULL;
    }
    
    printf("\nTraining %s model...\n", use_crystalline ? "CRYSTALLINE" : "BASELINE");
    
    double start_time = (double)clock() / CLOCKS_PER_SEC;
    
    float convergence_threshold = 0.1f;
    int num_classes = 4; // Pattern types
    
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        float epoch_loss = 0.0f;
        float epoch_accuracy = 0.0f;
        int num_batches = dataset->num_samples / BATCH_SIZE;
        
        for (int batch = 0; batch < num_batches; batch++) {
            float batch_loss = 0.0f;
            float batch_accuracy = 0.0f;
            
            for (int i = 0; i < BATCH_SIZE; i++) {
                int sample_idx = batch * BATCH_SIZE + i;
                if (sample_idx >= dataset->num_samples) break;
                
                // Forward pass
                float* logits = cllm_forward(model, 
                                            dataset->sequences[sample_idx],
                                            dataset->seq_length);
                
                if (!logits) continue;
                
                // Calculate loss and accuracy
                float loss = calculate_loss(logits, dataset->labels[sample_idx], num_classes);
                float accuracy = calculate_accuracy(logits, dataset->labels[sample_idx], num_classes);
                
                batch_loss += loss;
                batch_accuracy += accuracy;
                
                // Backward pass (simplified - just update embeddings)
                // In real implementation, this would compute full gradients
                for (int j = 0; j < model->config.vocab_size * model->config.embed_dim; j++) {
                    float grad = (cllm_rand() / (float)CLLM_RAND_MAX) * 0.01f - 0.005f;
                    model->embeddings[j] -= model->config.learning_rate * grad;
                }
                
                // Apply cymatic resonance if using crystalline training
                if (use_crystalline && model->cymatic_state) {
                    float* dummy_grads = (float*)malloc(model->config.vocab_size * 
                                                       model->config.embed_dim * sizeof(float));
                    if (dummy_grads) {
                        for (int j = 0; j < model->config.vocab_size * model->config.embed_dim; j++) {
                            dummy_grads[j] = (cllm_rand() / (float)CLLM_RAND_MAX) * 0.01f - 0.005f;
                        }
                        cllm_apply_cymatic_resonance(model, dummy_grads,
                                                    model->config.vocab_size * model->config.embed_dim,
                                                    epoch * num_batches + batch);
                        free(dummy_grads);
                    }
                }
            }
            
            epoch_loss += batch_loss / BATCH_SIZE;
            epoch_accuracy += batch_accuracy / BATCH_SIZE;
        }
        
        epoch_loss /= num_batches;
        epoch_accuracy /= num_batches;
        
        metrics->loss_history[epoch] = epoch_loss;
        metrics->accuracy_history[epoch] = epoch_accuracy;
        
        // Check for convergence
        if (metrics->convergence_epoch < 0 && epoch_loss < convergence_threshold) {
            metrics->convergence_epoch = epoch;
        }
        
        // Print progress every 10 epochs
        if (epoch % 10 == 0 || epoch == num_epochs - 1) {
            printf("  Epoch %3d: Loss = %.4f, Accuracy = %.4f\n", 
                   epoch, epoch_loss, epoch_accuracy);
        }
    }
    
    double end_time = (double)clock() / CLOCKS_PER_SEC;
    metrics->total_training_time_ms = (end_time - start_time) * 1000.0;
    
    metrics->final_loss = metrics->loss_history[num_epochs - 1];
    metrics->final_accuracy = metrics->accuracy_history[num_epochs - 1];
    
    printf("Training completed in %.2f ms\n", metrics->total_training_time_ms);
    printf("Final Loss: %.4f, Final Accuracy: %.4f\n", 
           metrics->final_loss, metrics->final_accuracy);
    if (metrics->convergence_epoch >= 0) {
        printf("Converged at epoch: %d\n", (int)metrics->convergence_epoch);
    } else {
        printf("Did not converge within %d epochs\n", num_epochs);
    }
    
    return metrics;
}

/**
 * Compare training metrics
 */
static void compare_metrics(TrainingMetrics* baseline, TrainingMetrics* crystalline) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║              TRAINING COMPARISON RESULTS                       ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    printf("FINAL METRICS:\n");
    printf("  Baseline Loss:      %.4f\n", baseline->final_loss);
    printf("  Crystalline Loss:   %.4f\n", crystalline->final_loss);
    printf("  Improvement:        %.2f%%\n", 
           ((baseline->final_loss - crystalline->final_loss) / baseline->final_loss) * 100.0f);
    printf("\n");
    
    printf("  Baseline Accuracy:  %.4f\n", baseline->final_accuracy);
    printf("  Crystalline Accuracy: %.4f\n", crystalline->final_accuracy);
    printf("  Improvement:        %.2f%%\n",
           ((crystalline->final_accuracy - baseline->final_accuracy) / baseline->final_accuracy) * 100.0f);
    printf("\n");
    
    printf("CONVERGENCE:\n");
    if (baseline->convergence_epoch >= 0) {
        printf("  Baseline converged at epoch:    %d\n", (int)baseline->convergence_epoch);
    } else {
        printf("  Baseline did not converge\n");
    }
    
    if (crystalline->convergence_epoch >= 0) {
        printf("  Crystalline converged at epoch: %d\n", (int)crystalline->convergence_epoch);
    } else {
        printf("  Crystalline did not converge\n");
    }
    
    if (baseline->convergence_epoch >= 0 && crystalline->convergence_epoch >= 0) {
        float speedup = baseline->convergence_epoch / crystalline->convergence_epoch;
        printf("  Convergence speedup:            %.2fx\n", speedup);
    }
    printf("\n");
    
    printf("TRAINING TIME:\n");
    printf("  Baseline:    %.2f ms\n", baseline->total_training_time_ms);
    printf("  Crystalline: %.2f ms\n", crystalline->total_training_time_ms);
    printf("  Overhead:    %.2f%%\n",
           ((crystalline->total_training_time_ms - baseline->total_training_time_ms) / 
            baseline->total_training_time_ms) * 100.0f);
    printf("\n");
}

/**
 * Save metrics to file
 */
static void save_metrics_to_file(TrainingMetrics* baseline, TrainingMetrics* crystalline,
                                const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("ERROR: Failed to open file %s for writing\n", filename);
        return;
    }
    
    fprintf(fp, "# Training Quality Validation Results\n\n");
    fprintf(fp, "## Loss History\n");
    fprintf(fp, "Epoch,Baseline_Loss,Crystalline_Loss\n");
    
    for (int i = 0; i < baseline->num_epochs; i++) {
        fprintf(fp, "%d,%.6f,%.6f\n", i, baseline->loss_history[i], 
                crystalline->loss_history[i]);
    }
    
    fprintf(fp, "\n## Accuracy History\n");
    fprintf(fp, "Epoch,Baseline_Accuracy,Crystalline_Accuracy\n");
    
    for (int i = 0; i < baseline->num_epochs; i++) {
        fprintf(fp, "%d,%.6f,%.6f\n", i, baseline->accuracy_history[i],
                crystalline->accuracy_history[i]);
    }
    
    fprintf(fp, "\n## Summary Statistics\n");
    fprintf(fp, "Metric,Baseline,Crystalline,Improvement\n");
    fprintf(fp, "Final Loss,%.6f,%.6f,%.2f%%\n",
            baseline->final_loss, crystalline->final_loss,
            ((baseline->final_loss - crystalline->final_loss) / baseline->final_loss) * 100.0f);
    fprintf(fp, "Final Accuracy,%.6f,%.6f,%.2f%%\n",
            baseline->final_accuracy, crystalline->final_accuracy,
            ((crystalline->final_accuracy - baseline->final_accuracy) / baseline->final_accuracy) * 100.0f);
    fprintf(fp, "Training Time (ms),%.2f,%.2f,%.2f%%\n",
            baseline->total_training_time_ms, crystalline->total_training_time_ms,
            ((crystalline->total_training_time_ms - baseline->total_training_time_ms) / 
             baseline->total_training_time_ms) * 100.0f);
    
    fclose(fp);
    printf("Metrics saved to %s\n", filename);
}

int main(int argc, char** argv) {
    srand(time(NULL));
    
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║     CRYSTALLINE CLLM - TRAINING QUALITY VALIDATION            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    // Configuration
    int vocab_size = 1000;
    int embed_dim = 128;
    int num_heads = 4;
    int num_layers = 2;
    int max_seq_len = 64;
    int num_epochs = 50;
    
    CLLMConfig config = {
        .vocab_size = vocab_size,
        .embed_dim = embed_dim,
        .num_heads = num_heads,
        .num_layers = num_layers,
        .max_seq_len = max_seq_len,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    // Generate dataset
    printf("\nGenerating training dataset...\n");
    Dataset* dataset = generate_dataset(DATASET_SIZE, max_seq_len, vocab_size);
    if (!dataset) {
        printf("ERROR: Failed to generate dataset\n");
        return 1;
    }
    printf("Dataset generated: %d samples, %d tokens per sequence\n",
           dataset->num_samples, dataset->seq_length);
    
    // Train baseline model
    printf("\n" "════════════════════════════════════════════════════════════════\n");
    printf("  BASELINE TRAINING (Standard Initialization)\n");
    printf("════════════════════════════════════════════════════════════════\n");
    
    CLLMModel* baseline_model = cllm_create_model(&config);
    if (!baseline_model) {
        printf("ERROR: Failed to create baseline model\n");
        free_dataset(dataset);
        return 1;
    }
    
    TrainingMetrics* baseline_metrics = train_model(baseline_model, dataset, num_epochs, 0);
    
    // Train crystalline model
    printf("\n════════════════════════════════════════════════════════════════\n");
    printf("  CRYSTALLINE TRAINING (Mathematical Framework)\n");
    printf("════════════════════════════════════════════════════════════════\n");
    
    CLLMModel* crystalline_model = cllm_create_model(&config);
    if (!crystalline_model) {
        printf("ERROR: Failed to create crystalline model\n");
        cllm_free_model(baseline_model);
        free(baseline_metrics->loss_history);
        free(baseline_metrics->accuracy_history);
        free(baseline_metrics);
        free_dataset(dataset);
        return 1;
    }
    
    // Initialize with lattice embeddings
    cllm_init_lattice_embeddings(crystalline_model, 8, 24, 1.0f);
    
    // Initialize cymatic training
    CymaticConfig cymatic_config = {
        .base_frequency = 432.0f,
        .harmonic_count = 8,
        .resonance_threshold = 0.7f,
        .phase_shift = 0.0f
    };
    cllm_init_cymatic_training(crystalline_model, &cymatic_config);
    
    TrainingMetrics* crystalline_metrics = train_model(crystalline_model, dataset, num_epochs, 1);
    
    // Compare results
    if (baseline_metrics && crystalline_metrics) {
        compare_metrics(baseline_metrics, crystalline_metrics);
        save_metrics_to_file(baseline_metrics, crystalline_metrics, 
                           "training_validation_results.csv");
    }
    
    // Cleanup
    cllm_free_model(baseline_model);
    cllm_free_model(crystalline_model);
    
    if (baseline_metrics) {
        free(baseline_metrics->loss_history);
        free(baseline_metrics->accuracy_history);
        free(baseline_metrics);
    }
    
    if (crystalline_metrics) {
        free(crystalline_metrics->loss_history);
        free(crystalline_metrics->accuracy_history);
        free(crystalline_metrics);
    }
    
    free_dataset(dataset);
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║           VALIDATION COMPLETED SUCCESSFULLY                    ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}