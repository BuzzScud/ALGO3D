#ifndef CLLM_PHONETIC_VALUES_H
#define CLLM_PHONETIC_VALUES_H

/**
 * Phonetic Value System - ν(λ) Implementation
 * 
 * Maps phonetic/cultural layers (λ) to numeric values for the lattice formula.
 * This implements the ν(λ) term in the complete unabridged formula:
 * L(n,d,k,λ,ω,ψ) = 3^O × ∏cos(θ×φᵢ) × Γ(k) × ν(λ) × (ω) × Ψ(ψ) × Γ(n,d)
 * 
 * Based on linguistic analysis and phonetic degradation patterns.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Get phonetic value for a given phonetic/cultural layer
 * 
 * @param lambda Phonetic string (e.g., "dub", "knbt", "k'anchay", "kub")
 * @return Numeric value for the phonetic layer (default: 3.0)
 * 
 * Known mappings:
 * - "dub" → 3.0
 * - "knbt" → 3.0
 * - "k'anchay" → 3.0
 * - "kub" → 3.0
 * - Unknown → 3.0 (default)
 */
double get_phonetic_value(const char* lambda);

/**
 * Check if a phonetic value is registered in the system
 * 
 * @param lambda Phonetic string to check
 * @return true if registered, false otherwise
 */
bool is_phonetic_registered(const char* lambda);

/**
 * Get the number of registered phonetic values
 * 
 * @return Count of registered phonetic mappings
 */
size_t get_phonetic_count(void);

/**
 * Get phonetic name by index (for iteration)
 * 
 * @param index Index in phonetic table
 * @return Phonetic name or NULL if index out of bounds
 */
const char* get_phonetic_name(size_t index);

/**
 * Get phonetic value by index (for iteration)
 * 
 * @param index Index in phonetic table
 * @return Phonetic value or 0.0 if index out of bounds
 */
double get_phonetic_value_by_index(size_t index);

#endif // CLLM_PHONETIC_VALUES_H