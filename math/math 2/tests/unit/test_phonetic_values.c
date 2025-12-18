/**
 * Unit tests for phonetic value system
 */

#include "../../include/phonetic_values.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

#define MATH_EPSILON 0.0001

void test_known_phonemes() {
    printf("Testing known phonemes...\n");
    
    // Test known phonetic values
    assert(fabs(get_phonetic_value("dub") - 3.0) < MATH_EPSILON);
    assert(fabs(get_phonetic_value("knbt") - 3.0) < MATH_EPSILON);
    assert(fabs(get_phonetic_value("k'anchay") - 3.0) < MATH_EPSILON);
    assert(fabs(get_phonetic_value("kub") - 3.0) < MATH_EPSILON);
    
    printf("✓ All known phonemes return correct values\n");
}

void test_unknown_phonemes() {
    printf("Testing unknown phonemes...\n");
    
    // Test unknown phonemes return default value (3.0)
    assert(fabs(get_phonetic_value("unknown") - 3.0) < MATH_EPSILON);
    assert(fabs(get_phonetic_value("xyz") - 3.0) < MATH_EPSILON);
    assert(fabs(get_phonetic_value("") - 3.0) < MATH_EPSILON);
    
    printf("✓ Unknown phonemes return default value (3.0)\n");
}

void test_null_input() {
    printf("Testing NULL input...\n");
    
    // Test NULL input returns default value
    assert(fabs(get_phonetic_value(NULL) - 3.0) < MATH_EPSILON);
    
    printf("✓ NULL input returns default value (3.0)\n");
}

void test_phonetic_registration() {
    printf("Testing phonetic registration...\n");
    
    // Test registration checks
    assert(is_phonetic_registered("dub") == true);
    assert(is_phonetic_registered("knbt") == true);
    assert(is_phonetic_registered("k'anchay") == true);
    assert(is_phonetic_registered("kub") == true);
    assert(is_phonetic_registered("unknown") == false);
    assert(is_phonetic_registered(NULL) == false);
    
    printf("✓ Phonetic registration checks work correctly\n");
}

void test_phonetic_count() {
    printf("Testing phonetic count...\n");
    
    // Test count
    size_t count = get_phonetic_count();
    assert(count == 4);  // We have 4 registered phonemes
    
    printf("✓ Phonetic count is correct: %zu\n", count);
}

void test_phonetic_iteration() {
    printf("Testing phonetic iteration...\n");
    
    // Test iteration
    size_t count = get_phonetic_count();
    for (size_t i = 0; i < count; i++) {
        const char* name = get_phonetic_name(i);
        double value = get_phonetic_value_by_index(i);
        
        assert(name != NULL);
        assert(fabs(value - 3.0) < MATH_EPSILON);  // All current values are 3.0
        
        printf("  [%zu] %s = %.1f\n", i, name, value);
    }
    
    // Test out of bounds
    assert(get_phonetic_name(count) == NULL);
    assert(fabs(get_phonetic_value_by_index(count) - 0.0) < MATH_EPSILON);
    
    printf("✓ Phonetic iteration works correctly\n");
}

int main() {
    printf("\n=== Phonetic Value System Unit Tests ===\n\n");
    
    test_known_phonemes();
    test_unknown_phonemes();
    test_null_input();
    test_phonetic_registration();
    test_phonetic_count();
    test_phonetic_iteration();
    
    printf("\n✅ All phonetic value tests passed!\n\n");
    
    return 0;
}