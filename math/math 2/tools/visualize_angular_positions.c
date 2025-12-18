/**
 * Visualize Angular Positions Tool
 * 
 * Displays angular positions θ(n,k,λ,ω,ψ) for tokens in a model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "cllm_format.h"
#include "cllm_training.h"
#include "cllm_angular_position.h"

void print_usage(const char* program_name) {
    printf("Usage: %s <model.cllm> [options]\n", program_name);
    printf("\n");
    printf("Visualize angular positions for tokens in a CLLM model.\n");
    printf("\n");
    printf("Options:\n");
    printf("  --max-tokens N  Maximum number of tokens to display (default: 100)\n");
    printf("  --dimension N   Dimension for angular calculation (default: 0)\n");
    printf("  --help          Show this help message\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s model.cllm --max-tokens 50\n", program_name);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* model_path = argv[1];
    uint32_t max_tokens = 100;
    int dimension = 0;
    
    // Parse arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--max-tokens") == 0 && i + 1 < argc) {
            max_tokens = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--dimension") == 0 && i + 1 < argc) {
            dimension = atoi(argv[++i]);
        }
    }
    
    printf("\n=== Angular Position Visualizer ===\n\n");
    
    // Load model
    printf("Loading model from %s...\n", model_path);
    CLLMModel* model = cllm_read_model(model_path);
    if (!model) {
        fprintf(stderr, "ERROR: Failed to load model from %s\n", model_path);
        return 1;
    }
    
    printf("Model loaded successfully.\n");
    printf("  Vocabulary size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Dimension: %d\n", dimension);
    printf("  Max tokens to display: %u\n", max_tokens);
    printf("\n");
    
    // Limit tokens to display
    uint32_t num_display = (max_tokens < model->vocab_size) ? max_tokens : model->vocab_size;
    
    // Display angular positions
    printf("Angular Positions θ(n,k,λ,ω,ψ):\n");
    printf("%-8s %-12s %-10s %-15s %-15s %-10s %-10s\n",
           "Token", "Prime", "Group", "θ (radians)", "θ (degrees)", "Clock", "Near 144k");
    printf("%-8s %-12s %-10s %-15s %-15s %-10s %-10s\n",
           "--------", "------------", "----------", "---------------", "---------------", 
           "----------", "----------");
    
    for (uint32_t i = 0; i < num_display; i++) {
        CLLMToken* token = &model->tokens[i];
        
        // Compute angular position
        AngularPosition pos;
        double wavelength = get_phonetic_wavelength(token->token_str[0]);
        angular_position_calculate(
            token->prime_encoding,
            i,
            dimension,
            wavelength,
            &pos
        );
        
        // Convert to degrees
        double theta_degrees = pos.theta * 180.0 / 3.14159265358979323846;
        
        // Format clock position
        char clock_str[20];
        snprintf(clock_str, sizeof(clock_str), "%d:%02d", 
                 pos.clock_hour, (int)pos.clock_minute);
        
        printf("%-8u %-12lu %-10u %-15.6f %-15.2f %-10s %-10s\n",
               i,
               (unsigned long)token->prime_encoding,
               token->symmetry_group,
               pos.theta,
               theta_degrees,
               clock_str,
               pos.is_near_144000 ? "Yes" : "No");
    }
    
    printf("\n");
    
    // Statistics by symmetry group
    printf("Statistics by Symmetry Group:\n");
    printf("%-10s %-10s %-15s %-15s\n", "Group", "Count", "Avg θ", "Std Dev θ");
    printf("%-10s %-10s %-15s %-15s\n", "----------", "----------", "---------------", "---------------");
    
    for (int g = 0; g < 12; g++) {
        int count = 0;
        double sum_theta = 0.0;
        double sum_theta_sq = 0.0;
        
        for (uint32_t i = 0; i < model->vocab_size; i++) {
            if (model->tokens[i].symmetry_group == (uint32_t)g) {
                AngularPosition pos;
                double wavelength = get_phonetic_wavelength(model->tokens[i].token_str[0]);
                angular_position_calculate(
                    model->tokens[i].prime_encoding,
                    i,
                    dimension,
                    wavelength,
                    &pos
                );
                
                sum_theta += pos.theta;
                sum_theta_sq += pos.theta * pos.theta;
                count++;
            }
        }
        
        if (count > 0) {
            double avg = sum_theta / count;
            double variance = (sum_theta_sq / count) - (avg * avg);
            double stddev = (variance > 0) ? math_sqrt(variance) : 0.0;
            
            printf("%-10d %-10d %-15.6f %-15.6f\n", g, count, avg, stddev);
        }
    }
    
    printf("\n✓ Visualization complete!\n\n");
    
    // Cleanup
    free(model);
    
    return 0;
}