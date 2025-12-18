/**
 * Phonetic Value System Implementation
 * 
 * Implements the ν(λ) phonetic value mapping for the lattice formula.
 * Maps phonetic/cultural layers to numeric values based on linguistic analysis.
 */

#include "../include/phonetic_values.h"
#include <string.h>
#include <stdio.h>

/**
 * Phonetic mapping structure
 */
typedef struct {
    const char* phoneme;
    double value;
} PhoneticMapping;

/**
 * Phonetic value table
 * 
 * Based on linguistic analysis and phonetic degradation patterns.
 * All current mappings use value 3.0 (the triad core - seed of all).
 * 
 * Future expansion may include:
 * - Additional phonetic layers from various languages
 * - Cultural-specific mappings
 * - Frequency-based adjustments
 */
static const PhoneticMapping PHONETIC_TABLE[] = {
    {"dub", 3.0},        // English phonetic layer
    {"knbt", 3.0},       // Ancient phonetic layer
    {"k'anchay", 3.0},   // Quechua phonetic layer
    {"kub", 3.0},        // Germanic phonetic layer
    
    // Additional phonetic layers can be added here
    // Format: {"phoneme", value}
};

/**
 * Number of registered phonetic mappings
 */
static const size_t PHONETIC_TABLE_SIZE = sizeof(PHONETIC_TABLE) / sizeof(PhoneticMapping);

/**
 * Default phonetic value (used for unknown phonemes)
 * Set to 3.0 (the triad core)
 */
static const double DEFAULT_PHONETIC_VALUE = 3.0;

/**
 * Get phonetic value for a given phonetic/cultural layer
 * 
 * Uses simple linear search for now. For larger tables, consider:
 * - Hash table for O(1) lookup
 * - Binary search for O(log n) lookup
 * - Trie structure for prefix matching
 * 
 * @param lambda Phonetic string
 * @return Numeric value for the phonetic layer
 */
double get_phonetic_value(const char* lambda) {
    // Handle NULL input
    if (!lambda) {
        return DEFAULT_PHONETIC_VALUE;
    }
    
    // Search phonetic table
    for (size_t i = 0; i < PHONETIC_TABLE_SIZE; i++) {
        if (strcmp(lambda, PHONETIC_TABLE[i].phoneme) == 0) {
            return PHONETIC_TABLE[i].value;
        }
    }
    
    // Return default value for unknown phonemes
    return DEFAULT_PHONETIC_VALUE;
}

/**
 * Check if a phonetic value is registered in the system
 * 
 * @param lambda Phonetic string to check
 * @return true if registered, false otherwise
 */
bool is_phonetic_registered(const char* lambda) {
    if (!lambda) {
        return false;
    }
    
    for (size_t i = 0; i < PHONETIC_TABLE_SIZE; i++) {
        if (strcmp(lambda, PHONETIC_TABLE[i].phoneme) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 * Get the number of registered phonetic values
 * 
 * @return Count of registered phonetic mappings
 */
size_t get_phonetic_count(void) {
    return PHONETIC_TABLE_SIZE;
}

/**
 * Get phonetic name by index (for iteration)
 * 
 * @param index Index in phonetic table
 * @return Phonetic name or NULL if index out of bounds
 */
const char* get_phonetic_name(size_t index) {
    if (index >= PHONETIC_TABLE_SIZE) {
        return NULL;
    }
    return PHONETIC_TABLE[index].phoneme;
}

/**
 * Get phonetic value by index (for iteration)
 * 
 * @param index Index in phonetic table
 * @return Phonetic value or 0.0 if index out of bounds
 */
double get_phonetic_value_by_index(size_t index) {
    if (index >= PHONETIC_TABLE_SIZE) {
        return 0.0;
    }
    return PHONETIC_TABLE[index].value;
}