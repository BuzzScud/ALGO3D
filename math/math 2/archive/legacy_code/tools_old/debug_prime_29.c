#include <stdio.h>
#include <stdint.h>
#include "../include/clock_lattice.h"

int main(void) {
    uint64_t prime = 29;
    
    printf("Debugging prime 29 validation\n");
    printf("Prime value: %lu\n", prime);
    printf("29 mod 12 = %lu\n", prime % 12);
    
    // Check 12-fold symmetry
    uint64_t mod12 = prime % 12;
    bool symmetry_ok = (mod12 == 1 || mod12 == 5 || mod12 == 7 || mod12 == 11);
    printf("12-fold symmetry check: %s (mod12=%lu)\n", symmetry_ok ? "PASS" : "FAIL", mod12);
    
    // Estimate index
    uint32_t index = estimate_prime_index(prime);
    printf("Estimated index: %u\n", index);
    
    // Map to clock position
    BabylonianClockPosition pos = map_prime_index_to_clock(index);
    printf("Clock position: ring=%d, position=%d, angle=%f, radius=%f\n",
           pos.ring, pos.position, pos.angle, pos.radius);
    
    // Validate position
    bool valid = is_valid_clock_position(pos);
    printf("Position valid: %s\n", valid ? "YES" : "NO");
    
    // Full validation
    bool full_valid = validate_prime_by_clock_position(prime);
    printf("Full validation: %s\n", full_valid ? "PASS" : "FAIL");
    
    return 0;
}
