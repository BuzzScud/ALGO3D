#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <assert.h>
#include "clock_lattice.h"

int main() {
    printf("\n=== Clock Lattice Mapping Test ===\n");
    
    int test_indices[] = {0, 10, 50, 100, 232, 500, 1000, 10000, 100000, 1000000};
    int num_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int prime_index = test_indices[i];
        BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
        
        printf("  Prime %d → Ring %d, Position %d, Angle %.2f, Radius %.2f\n",
               prime_index, pos.ring, pos.position, pos.angle, pos.radius);
        
        // Verify ring is bounded
        assert(pos.ring >= 0 && pos.ring <= 7);
        
        // Calculate O
        double positions_in_ring;
        if (pos.ring == 0) positions_in_ring = 12.0;
        else if (pos.ring == 1 || pos.ring == 2) positions_in_ring = 60.0;
        else if (pos.ring == 3) positions_in_ring = 100.0;
        else positions_in_ring = 1000.0;
        
        double O = (double)pos.ring + ((double)pos.position / positions_in_ring);
        printf("    O = %.3f (3^O would be %.2f)\n", O, pow(3.0, O));
        
        assert(O >= 0.0 && O <= 10.0);
        assert(pow(3.0, O) < 100000.0);
    }
    
    printf("\n✓ ALL TESTS PASSED\n\n");
    return 0;
}
