/**
 * loss_functions.c - Loss Functions with Arbitrary Precision (Crystalline Abacus)
 * 
 * MIGRATED: Now uses NEW math library (Crystalline Abacus)
 * - Replaced BigFixed with CrystallineAbacus
 * - Uses NEW math library transcendental functions
 * - Supports ALL bases >= 2 (base-60 mathematics)
 * - No dependencies on OLD crystalline library
 */

#include "loss_functions.h"
#include "math/abacus.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>

/**
 * Cross-entropy loss with Crystalline Abacus (arbitrary precision)
 * 
 * Computes: -1/N * Σ log(softmax(logits)[target])
 */
void cross_entropy_loss(
    CrystallineAbacus** logits,
    uint32_t* targets,
    CrystallineAbacus* loss,
    int batch_size,
    int num_classes,
    int precision
) {
    if (!logits || !targets || !loss) return;
    
    (void)precision; // Abacus handles precision internally
    
    CrystallineAbacus* sum = abacus_from_uint64(0, 60);
    
    for (int i = 0; i < batch_size; i++) {
        uint32_t target = targets[i];
        if (target >= (uint32_t)num_classes) continue;
        
        CrystallineAbacus** logit_row = &logits[i * num_classes];
        
        // Find max for numerical stability
        CrystallineAbacus* max_logit = abacus_copy(logit_row[0]);
        
        for (int j = 1; j < num_classes; j++) {
            if (abacus_compare(logit_row[j], max_logit) > 0) {
                abacus_free(max_logit);
                max_logit = abacus_copy(logit_row[j]);
            }
        }
        
        // Compute exp sum
        CrystallineAbacus* exp_sum = abacus_from_uint64(0, 60);
        
        for (int j = 0; j < num_classes; j++) {
            CrystallineAbacus* diff = abacus_from_uint64(0, 60);
            CrystallineAbacus* exp_val = abacus_from_uint64(0, 60);
            
            abacus_sub(diff, logit_row[j], max_logit);
            
            // Convert to double for exp (will optimize later)
            double diff_val;
            abacus_to_double(diff, &diff_val);
            double exp_result = math_exp(diff_val);
            
            abacus_free(exp_val);
            exp_val = abacus_from_double(exp_result, 60, 10);
            
            CrystallineAbacus* new_sum = abacus_from_uint64(0, 60);
            abacus_add(new_sum, exp_sum, exp_val);
            abacus_free(exp_sum);
            exp_sum = new_sum;
            
            abacus_free(diff);
            abacus_free(exp_val);
        }
        
        // Compute log probability
        double exp_sum_val;
        abacus_to_double(exp_sum, &exp_sum_val);
        double log_sum_val = math_log(exp_sum_val);
        
        CrystallineAbacus* log_sum = abacus_from_double(log_sum_val, 60, 10);
        CrystallineAbacus* target_logit_norm = abacus_from_uint64(0, 60);
        CrystallineAbacus* log_prob = abacus_from_uint64(0, 60);
        CrystallineAbacus* neg_log_prob = abacus_from_uint64(0, 60);
        
        abacus_sub(target_logit_norm, logit_row[target], max_logit);
        abacus_sub(log_prob, target_logit_norm, log_sum);
        
        // Negate: neg_log_prob = -log_prob
        CrystallineAbacus* zero = abacus_from_uint64(0, 60);
        abacus_sub(neg_log_prob, zero, log_prob);
        abacus_free(zero);
        
        CrystallineAbacus* new_sum = abacus_from_uint64(0, 60);
        abacus_add(new_sum, sum, neg_log_prob);
        abacus_free(sum);
        sum = new_sum;
        
        abacus_free(max_logit);
        abacus_free(exp_sum);
        abacus_free(log_sum);
        abacus_free(target_logit_norm);
        abacus_free(log_prob);
        abacus_free(neg_log_prob);
    }
    
    // Average loss
    CrystallineAbacus* batch_size_abacus = abacus_from_uint64(batch_size, 60);
    CrystallineAbacus* remainder = abacus_from_uint64(0, 60);
    abacus_div(loss, remainder, sum, batch_size_abacus);
    
    abacus_free(sum);
    abacus_free(batch_size_abacus);
    abacus_free(remainder);
}

/**
 * Softmax with Crystalline Abacus (arbitrary precision)
 * 
 * Computes: softmax(x) = exp(x) / Σ exp(x)
 */
void softmax(
    CrystallineAbacus** logits,
    int size,
    int precision
) {
    if (!logits || size <= 0) return;
    
    (void)precision; // Abacus handles precision internally
    
    // Find max
    CrystallineAbacus* max_logit = abacus_copy(logits[0]);
    
    for (int i = 1; i < size; i++) {
        if (abacus_compare(logits[i], max_logit) > 0) {
            abacus_free(max_logit);
            max_logit = abacus_copy(logits[i]);
        }
    }
    
    // Compute exp and sum
    CrystallineAbacus* sum = abacus_from_uint64(0, 60);
    
    for (int i = 0; i < size; i++) {
        CrystallineAbacus* diff = abacus_from_uint64(0, 60);
        
        abacus_sub(diff, logits[i], max_logit);
        
        // Convert to double for exp (will optimize later)
        double diff_val;
        abacus_to_double(diff, &diff_val);
        double exp_result = math_exp(diff_val);
        
        CrystallineAbacus* exp_val = abacus_from_double(exp_result, 60, 10);
        
        abacus_free(logits[i]);
        logits[i] = exp_val;
        
        CrystallineAbacus* new_sum = abacus_from_uint64(0, 60);
        abacus_add(new_sum, sum, exp_val);
        abacus_free(sum);
        sum = new_sum;
        
        abacus_free(diff);
    }
    
    // Normalize
    for (int i = 0; i < size; i++) {
        CrystallineAbacus* normalized = abacus_from_uint64(0, 60);
        CrystallineAbacus* remainder = abacus_from_uint64(0, 60);
        abacus_div(normalized, remainder, logits[i], sum);
        abacus_free(logits[i]);
        abacus_free(remainder);
        logits[i] = normalized;
    }
    
    abacus_free(max_logit);
    abacus_free(sum);
}