#include <stdio.h>
#include <stdint.h>
#include "clock_lattice.h"

int main(void) {
    uint64_t prime = 37;
    
    printf("Debugging prime 37 validation\n");
    printf("Prime value: %lu\n", prime);
    printf("37 mod 12 = %lu\n", prime % 12);
    
    // Estimate index
    uint32_t index = estimate_prime_index(prime);
    printf("Estimated index: %u (should be 12)\n", index);
    
    // Map to clock position
    BabylonianClockPosition pos = map_prime_index_to_clock(index);
    printf("Clock position: ring=%d, position=%d, angle=%f, radius=%f\n",
           pos.ring, pos.position, pos.angle, pos.radius);
    
    // Check position validity
    printf("\nValidation checks:\n");
    printf("Ring in bounds [0,7]: %s\n", (pos.ring >= 0 && pos.ring <= 7) ? "YES" : "NO");
    
    if (pos.ring == 0) {
        printf("Ring 0 position in bounds [1,12]: %s\n", 
               (pos.position >= 1 && pos.position <= 12) ? "YES" : "NO");
    }
    
    double expected_radius = (pos.ring == 0) ? 0.25 : 0.0;
    printf("Expected radius: %f\n", expected_radius);
    printf("Actual radius: %f\n", pos.radius);
    printf("Radius match (tolerance 0.01): %s\n",
           (pos.radius >= expected_radius - 0.01 && pos.radius <= expected_radius + 0.01) ? "YES" : "NO");
    
    bool valid = is_valid_clock_position(pos);
    printf("\nPosition valid: %s\n", valid ? "YES" : "NO");
    
    return 0;
}
