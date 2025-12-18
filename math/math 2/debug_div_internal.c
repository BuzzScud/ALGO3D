#include "math/abacus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_beads(const CrystallineAbacus* a, const char* label) {
    printf("%s: ", label);
    for (size_t i = 0; i < a->num_beads; i++) {
        printf("[%u@%d] ", a->beads[i].value, a->beads[i].weight_exponent);
    }
    double val;
    abacus_to_double(a, &val);
    printf(" = %.10f\n", val);
}

int main() {
    printf("=== Internal Division Debug ===\n\n");
    
    // Create 6.333...
    CrystallineAbacus* dividend = abacus_from_double(6.333333, 10, 6);
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    
    print_beads(dividend, "Dividend");
    print_beads(two, "Divisor");
    
    // Separate integer and fractional parts
    CrystallineAbacus* a_int = abacus_new(10);
    CrystallineAbacus* a_frac = abacus_new(10);
    
    size_t int_count = 0;
    size_t frac_count = 0;
    
    for (size_t i = 0; i < dividend->num_beads; i++) {
        if (dividend->beads[i].weight_exponent >= 0) {
            a_int->beads[int_count++] = dividend->beads[i];
        } else {
            a_frac->beads[frac_count++] = dividend->beads[i];
        }
    }
    
    a_int->num_beads = int_count;
    a_int->min_exponent = 0;
    a_frac->num_beads = frac_count;
    a_frac->min_exponent = dividend->min_exponent;
    
    printf("\nSeparated:\n");
    print_beads(a_int, "Integer part");
    print_beads(a_frac, "Fractional part");
    
    // Divide integer part
    CrystallineAbacus* q_int = abacus_new(10);
    CrystallineAbacus* remainder = abacus_new(10);
    
    abacus_div(q_int, remainder, a_int, two);
    
    printf("\nAfter integer division:\n");
    print_beads(q_int, "Quotient");
    print_beads(remainder, "Remainder");
    
    // Add fractional part to remainder
    CrystallineAbacus* temp_remainder = abacus_new(10);
    abacus_add(temp_remainder, remainder, a_frac);
    
    printf("\nAfter adding fractional part:\n");
    print_beads(temp_remainder, "Total remainder");
    
    printf("\nNow we need to divide this remainder by 2 to get fractional quotient...\n");
    
    abacus_free(dividend);
    abacus_free(two);
    abacus_free(a_int);
    abacus_free(a_frac);
    abacus_free(q_int);
    abacus_free(remainder);
    abacus_free(temp_remainder);
    
    return 0;
}
