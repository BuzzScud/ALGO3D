<?php
/**
 * Crystalline Math - Rainbow Table Example
 * 
 * This example demonstrates rainbow table operations for
 * fast prime lookups using the new rainbow table functions.
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension is not loaded.\n" .
        "Please install it with: sudo make install\n");
}

echo "=== Crystalline Math - Rainbow Table ===\n\n";

// Example 1: Initialize rainbow table
echo "1. Initialize Rainbow Table:\n";
$result = rainbow_init(10000);
if ($result) {
    echo "   Rainbow table initialized successfully\n";
    echo "   Initial size: " . rainbow_size() . " primes\n";
} else {
    echo "   Failed to initialize rainbow table\n";
}
echo "\n";

// Example 2: Populate with primes
echo "2. Populate Rainbow Table:\n";
$count = 1000;
$result = rainbow_populate_count($count);
if ($result) {
    echo "   Populated with $count primes\n";
    echo "   Table size: " . rainbow_size() . " primes\n";
    echo "   Max prime: " . rainbow_max_prime() . "\n";
} else {
    echo "   Failed to populate rainbow table\n";
}
echo "\n";

// Example 3: Lookup primes by index
echo "3. Lookup Primes by Index:\n";
for ($i = 1; $i <= 20; $i++) {
    $prime = rainbow_lookup_by_index($i);
    if ($prime !== false) {
        echo sprintf("   Index %3d: %5d\n", $i, $prime);
    }
}
echo "\n";

// Example 4: Lookup index by prime
echo "4. Lookup Index by Prime:\n";
$test_primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47];
foreach ($test_primes as $prime) {
    $index = rainbow_lookup_index($prime);
    if ($index !== false) {
        echo sprintf("   Prime %3d → Index %3d\n", $prime, $index);
    }
}
echo "\n";

// Example 5: Next and Previous Prime
echo "5. Next and Previous Prime:\n";
$test_values = [10, 50, 100, 200, 500];
foreach ($test_values as $val) {
    $next = rainbow_next_prime($val);
    $prev = rainbow_prev_prime($val);
    echo sprintf("   Around %3d: prev=%3d, next=%3d\n", $val, $prev, $next);
}
echo "\n";

// Example 6: Contains Check
echo "6. Check if Primes are in Table:\n";
$test_numbers = [2, 4, 7, 10, 17, 100, 541, 1000];
foreach ($test_numbers as $num) {
    $contains = rainbow_contains($num);
    echo sprintf("   %4d: %s\n", $num, $contains ? "YES" : "NO");
}
echo "\n";

// Example 7: Performance Test - Rainbow Table Lookup
echo "7. Performance Test - Rainbow Table Lookup:\n";
$iterations = 10000;
$start = microtime(true);
for ($i = 0; $i < $iterations; $i++) {
    $index = rand(1, rainbow_size());
    rainbow_lookup_by_index($index);
}
$elapsed = microtime(true) - $start;
$rate = $iterations / $elapsed;
echo "   Lookups: $iterations\n";
echo "   Time: " . number_format($elapsed, 4) . " seconds\n";
echo "   Rate: " . number_format($rate, 0) . " lookups/second\n\n";

// Example 8: Performance Test - Index Lookup
echo "8. Performance Test - Index Lookup:\n";
$iterations = 10000;
$start = microtime(true);
for ($i = 0; $i < $iterations; $i++) {
    $prime = rainbow_lookup_by_index(rand(1, 100));
    if ($prime !== false) {
        rainbow_lookup_index($prime);
    }
}
$elapsed = microtime(true) - $start;
$rate = $iterations / $elapsed;
echo "   Lookups: $iterations\n";
echo "   Time: " . number_format($elapsed, 4) . " seconds\n";
echo "   Rate: " . number_format($rate, 0) . " lookups/second\n\n";

// Example 9: Populate to specific prime
echo "9. Populate to Specific Prime:\n";
rainbow_cleanup();
rainbow_init(10000);
$target_prime = 10000;
$result = rainbow_populate_to_prime($target_prime);
if ($result) {
    echo "   Populated up to prime $target_prime\n";
    echo "   Table size: " . rainbow_size() . " primes\n";
    echo "   Max prime: " . rainbow_max_prime() . "\n";
}
echo "\n";

// Example 10: Prime Gaps Analysis
echo "10. Prime Gaps Analysis (using rainbow table):\n";
$gaps = [];
for ($i = 1; $i < 100; $i++) {
    $p1 = rainbow_lookup_by_index($i);
    $p2 = rainbow_lookup_by_index($i + 1);
    if ($p1 !== false && $p2 !== false) {
        $gap = $p2 - $p1;
        if (!isset($gaps[$gap])) {
            $gaps[$gap] = 0;
        }
        $gaps[$gap]++;
    }
}
ksort($gaps);
echo "   Gap distribution (first 100 primes):\n";
foreach ($gaps as $gap => $count) {
    echo sprintf("   Gap %2d: %3d occurrences\n", $gap, $count);
}
echo "\n";

echo "Rainbow table provides O(1) prime lookups!\n";
echo "Perfect for applications needing fast prime access.\n";

rainbow_cleanup();