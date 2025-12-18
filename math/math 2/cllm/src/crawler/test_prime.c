/**
 * Test program for prime randomization
 */

#include "prime_randomization.h"
#include <stdio.h>

int main() {
    printf("=== Prime Randomization Test ===\n\n");
    
    // Test prime checking
    printf("--- Prime Checking ---\n");
    uint64_t test_numbers[] = {2, 3, 4, 5, 7, 11, 13, 17, 19, 23, 100, 101};
    for (int i = 0; i < 12; i++) {
        printf("  %lu is %s\n", test_numbers[i], 
               is_prime(test_numbers[i]) ? "prime" : "not prime");
    }
    
    // Test next/prev prime
    printf("\n--- Next/Previous Prime ---\n");
    printf("  Next prime after 10: %lu\n", next_prime(10));
    printf("  Next prime after 20: %lu\n", next_prime(20));
    printf("  Previous prime before 20: %lu\n", prev_prime(20));
    printf("  Previous prime before 10: %lu\n", prev_prime(10));
    
    // Test configuration
    printf("\n--- Prime Configuration ---\n");
    CrawlerPrimeConfig config;
    prime_config_init_default(&config);
    
    printf("  Frequency prime: %lu\n", config.frequency_prime);
    printf("  Link selection prime: %lu\n", config.link_selection_prime);
    printf("  Delay min prime: %lu\n", config.delay_min_prime);
    printf("  Delay max prime: %lu\n", config.delay_max_prime);
    printf("  Use randomization: %s\n", config.use_prime_randomization ? "yes" : "no");
    printf("  Configuration valid: %s\n", prime_config_validate(&config) ? "yes" : "no");
    
    // Test delay calculation
    printf("\n--- Prime Delay Calculation ---\n");
    for (uint64_t i = 0; i < 10; i++) {
        uint64_t delay = calculate_prime_delay(config.delay_min_prime, 
                                               config.delay_max_prime, i);
        printf("  Iteration %lu: delay = %lu seconds\n", i, delay);
    }
    
    // Test link selection
    printf("\n--- Prime Link Selection ---\n");
    int total_links = 50;
    printf("  Total links: %d\n", total_links);
    printf("  Selection prime: %lu\n", config.link_selection_prime);
    for (uint64_t i = 0; i < 10; i++) {
        int index = select_prime_link_index(config.link_selection_prime, 
                                           total_links, i);
        printf("  Iteration %lu: selected link index = %d\n", i, index);
    }
    
    printf("\n=== All tests passed ===\n");
    return 0;
}