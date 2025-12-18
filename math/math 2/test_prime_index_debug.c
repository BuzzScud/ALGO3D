#include <stdio.h>
#include "math/prime.h"

int main() {
    printf("Testing prime_index():\n\n");
    
    uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    uint64_t expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    for (int i = 0; i < 10; i++) {
        uint64_t idx = prime_index(primes[i]);
        printf("prime_index(%lu) = %lu (expected %lu) %s\n", 
               primes[i], idx, expected[i], 
               (idx == expected[i]) ? "✓" : "✗");
    }
    
    return 0;
}
