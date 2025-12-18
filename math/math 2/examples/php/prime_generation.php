<?php
/**
 * Crystalline Math - Prime Generation Example
 * 
 * This example demonstrates O(1) deterministic prime generation
 * using the Crystalline Math PHP extension with rainbow table and clock lattice.
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension is not loaded.\n" .
        "Please install it with: sudo make install\n");
}

echo "=== Crystalline Math - Prime Generation ===\n\n";
echo "Extension loaded: crystalline_math\n\n";

// Example 1: Generate primes using O(1) formula
echo "1. O(1) Prime Generation:\n";
echo "   Position 3, Magnitude 0: " . crystalline_prime_generate_o1(3, 0) . "\n";
echo "   Position 3, Magnitude 1: " . crystalline_prime_generate_o1(3, 1) . "\n";
echo "   Position 3, Magnitude 2: " . crystalline_prime_generate_o1(3, 2) . "\n";
echo "   Position 6, Magnitude 0: " . crystalline_prime_generate_o1(6, 0) . "\n";
echo "   Position 6, Magnitude 1: " . crystalline_prime_generate_o1(6, 1) . "\n";
echo "   Position 9, Magnitude 0: " . crystalline_prime_generate_o1(9, 0) . "\n";
echo "   Position 9, Magnitude 1: " . crystalline_prime_generate_o1(9, 1) . "\n\n";

// Example 2: Check if numbers are prime
echo "2. Primality Testing:\n";
$test_numbers = [2, 3, 4, 5, 17, 100, 157, 997, 1009, 10007];
foreach ($test_numbers as $n) {
    $is_prime = prime_is_prime($n);
    echo "   $n is " . ($is_prime ? "PRIME" : "composite") . "\n";
}
echo "\n";

// Example 3: First 20 primes using prime_is_prime
echo "3. First 20 Primes (using prime_is_prime):\n   ";
$count = 0;
$n = 2;
$primes = [];
while ($count < 20) {
    if (prime_is_prime($n)) {
        $primes[] = $n;
        $count++;
    }
    $n++;
}
echo implode(", ", $primes) . "\n\n";

// Example 4: Generate primes in all 12 positions
echo "4. Primes in All 12 Clock Positions (Magnitude 0):\n";
for ($pos = 0; $pos < 12; $pos++) {
    $prime = crystalline_prime_generate_o1($pos, 0);
    if ($prime > 0) {
        echo "   Position $pos: $prime\n";
    } else {
        echo "   Position $pos: composite\n";
    }
}
echo "\n";

// Example 5: Rainbow Table Operations
echo "5. Rainbow Table Operations:\n";
rainbow_init(1000);
rainbow_populate_count(100);
echo "   Initialized rainbow table with " . rainbow_size() . " primes\n";
echo "   Max prime in table: " . rainbow_max_prime() . "\n";
echo "   10th prime: " . rainbow_lookup_by_index(10) . "\n";
echo "   Index of prime 29: " . rainbow_lookup_index(29) . "\n";
echo "   Next prime after 29: " . rainbow_next_prime(29) . "\n";
echo "   Previous prime before 29: " . rainbow_prev_prime(29) . "\n";
echo "   Contains 31: " . (rainbow_contains(31) ? "yes" : "no") . "\n\n";

// Example 6: Clock Lattice Mapping
echo "6. Clock Lattice Mapping:\n";
clock_init();
$test_primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29];
foreach ($test_primes as $prime) {
    $pos = clock_map_prime_to_position($prime);
    echo "   Prime $prime → Ring " . $pos['ring'] . ", Position " . $pos['position'] . "\n";
}
echo "\n";

// Example 7: Reverse Lookup
echo "7. Clock Reverse Lookup:\n";
$numbers = [5, 17, 29, 100, 157];
foreach ($numbers as $num) {
    $lookup = clock_reverse_lookup($num);
    if ($lookup !== false) {
        echo "   Number $num → Ring " . $lookup['ring'] . ", Position " . $lookup['position'] . 
             ", Magnitude " . $lookup['magnitude'] . "\n";
    } else {
        echo "   Number $num → Not a valid prime position\n";
    }
}
echo "\n";

// Example 8: Prime Totient and Index
echo "8. Prime Totient and Index:\n";
$test_nums = [12, 30, 100];
foreach ($test_nums as $n) {
    echo "   φ($n) = " . prime_totient($n) . "\n";
}
echo "\n";
// Note: prime_index() currently only works for primes with mod 12 = 5, 7, or 11
// This is a known limitation of the clock lattice implementation
$test_primes2 = [2, 3, 5, 7, 11, 17, 19, 23, 29];  // Removed 13 (mod 12 = 1)
foreach ($test_primes2 as $p) {
    $index = @prime_index($p);  // Suppress warning for unsupported primes
    if ($index !== false && $index !== null && $index > 0) {
        echo "   Index of prime $p: " . $index . "\n";
    }
}
echo "\n";

// Example 9: Performance Test
echo "9. Performance Test (Primality Testing):\n";
$start = microtime(true);
$count = 0;
for ($i = 0; $i < 1000; $i++) {
    prime_is_prime(rand(1, 10000));
    $count++;
}
$elapsed = microtime(true) - $start;
$rate = $count / $elapsed;
echo "   Checked $count numbers in " . number_format($elapsed, 4) . " seconds\n";
echo "   Rate: " . number_format($rate, 0) . " checks/second\n\n";

// Example 10: Twin Primes
echo "10. Twin Primes (first 10 pairs):\n";
$twin_count = 0;
$p = 3;
while ($twin_count < 10) {
    if (prime_is_prime($p) && prime_is_prime($p + 2)) {
        echo "   ($p, " . ($p + 2) . ")\n";
        $twin_count++;
    }
    $p += 2;
}
echo "\n";

echo "Use prime_is_prime() in a loop as shown above to find the nth prime.\n";
echo "Use rainbow table for fast lookups of cached primes.\n";
echo "Use clock lattice for geometric prime analysis.\n";

rainbow_cleanup();
clock_cleanup();