<?php
/**
 * Crystalline Math - Clock Lattice Example
 * 
 * This example demonstrates the Babylonian clock lattice structure
 * with 12-fold symmetry using the new clock lattice functions.
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension is not loaded.\n" .
        "Please install it with: sudo make install\n");
}

echo "=== Crystalline Math - Clock Lattice ===\n\n";

// Initialize clock
clock_init();

// Example 1: Map primes to clock positions
echo "1. Prime to Clock Position Mapping:\n";
$primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47];
foreach ($primes as $prime) {
    $pos = clock_map_prime_to_position($prime);
    if ($pos !== false) {
        echo sprintf("   Prime %3d: Ring %2d, Position %2d, Angle %.4f rad, Radius %.2f\n",
            $prime, $pos['ring'], $pos['position'], $pos['angle'], $pos['radius']);
    }
}
echo "\n";

// Example 2: Reverse lookup - number to clock position
echo "2. Clock Reverse Lookup:\n";
$test_numbers = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71];
foreach ($test_numbers as $num) {
    $lookup = clock_reverse_lookup($num);
    if ($lookup !== false) {
        echo sprintf("   Number %3d: Ring %d, Position %2d, Magnitude %d\n",
            $num, $lookup['ring'], $lookup['position'], $lookup['magnitude']);
    }
}
echo "\n";

// Example 3: Validate clock positions
echo "3. Clock Position Validation:\n";
$test_positions = [
    ['ring' => 0, 'position' => 0],
    ['ring' => 0, 'position' => 5],
    ['ring' => 0, 'position' => 11],
    ['ring' => 0, 'position' => 12],  // Invalid
    ['ring' => 1, 'position' => 0],
    ['ring' => 1, 'position' => 30],
    ['ring' => 1, 'position' => 59],
    ['ring' => 1, 'position' => 60],  // Invalid
];
foreach ($test_positions as $pos) {
    $valid = clock_is_valid_position($pos);
    echo sprintf("   Ring %d, Position %2d: %s\n",
        $pos['ring'], $pos['position'], $valid ? "VALID" : "invalid");
}
echo "\n";

// Example 4: 12-fold symmetry demonstration
echo "4. 12-Fold Symmetry Analysis:\n";
rainbow_init(1000);
rainbow_populate_count(100);

$symmetry_groups = array_fill(0, 12, []);
for ($i = 1; $i <= 100; $i++) {
    $prime = rainbow_lookup_by_index($i);
    if ($prime !== false) {
        $pos = clock_map_prime_to_position($prime);
        if ($pos !== false) {
            $group = $pos['position'] % 12;
            $symmetry_groups[$group][] = $prime;
        }
    }
}

echo "   Distribution of first 100 primes across 12 positions:\n";
for ($i = 0; $i < 12; $i++) {
    $count = count($symmetry_groups[$i]);
    echo sprintf("   Position %2d: %3d primes", $i, $count);
    if ($count > 0 && $count <= 5) {
        echo " (" . implode(", ", array_slice($symmetry_groups[$i], 0, 5)) . ")";
    }
    echo "\n";
}
echo "\n";

// Example 5: Clock position to prime conversion
echo "5. Clock Position to Prime:\n";
$test_clock_positions = [
    ['ring' => 0, 'position' => 0],
    ['ring' => 0, 'position' => 1],
    ['ring' => 0, 'position' => 2],
    ['ring' => 0, 'position' => 3],
    ['ring' => 0, 'position' => 6],
    ['ring' => 0, 'position' => 9],
];
foreach ($test_clock_positions as $pos) {
    $prime = clock_position_to_prime($pos);
    echo sprintf("   Ring %d, Position %2d → Prime: %s\n",
        $pos['ring'], $pos['position'], $prime > 0 ? $prime : "composite");
}
echo "\n";

// Example 6: Prime generation using O(1) formula
echo "6. O(1) Prime Generation (Clock Positions 3, 6, 9):\n";
$positions = [3, 6, 9];
foreach ($positions as $pos) {
    echo "   Position $pos:\n";
    for ($mag = 0; $mag < 5; $mag++) {
        $prime = crystalline_prime_generate_o1($pos, $mag);
        echo sprintf("      Magnitude %d: %d\n", $mag, $prime);
    }
}
echo "\n";

// Example 7: Angle distribution
echo "7. Angle Distribution (Radians):\n";
$angles = [];
for ($i = 1; $i <= 50; $i++) {
    $prime = rainbow_lookup_by_index($i);
    if ($prime !== false) {
        $pos = clock_map_prime_to_position($prime);
        if ($pos !== false) {
            $angle_deg = rad2deg($pos['angle']);
            $bucket = floor($angle_deg / 30) * 30;
            if (!isset($angles[$bucket])) {
                $angles[$bucket] = 0;
            }
            $angles[$bucket]++;
        }
    }
}
ksort($angles);
echo "   Angle range distribution (first 50 primes):\n";
foreach ($angles as $bucket => $count) {
    echo sprintf("   %3d°-%3d°: %2d primes\n", $bucket, $bucket + 30, $count);
}
echo "\n";

// Example 8: Radius distribution
echo "8. Radius Distribution:\n";
$radii = [];
for ($i = 1; $i <= 50; $i++) {
    $prime = rainbow_lookup_by_index($i);
    if ($prime !== false) {
        $pos = clock_map_prime_to_position($prime);
        if ($pos !== false) {
            $radius_bucket = floor($pos['radius'] * 10) / 10;
            if (!isset($radii[$radius_bucket])) {
                $radii[$radius_bucket] = 0;
            }
            $radii[$radius_bucket]++;
        }
    }
}
ksort($radii);
echo "   Radius distribution (first 50 primes):\n";
foreach ($radii as $bucket => $count) {
    echo sprintf("   %.1f: %2d primes\n", $bucket, $count);
}
echo "\n";

echo "The clock lattice provides geometric structure to prime numbers!\n";
echo "12-fold symmetry emerges naturally from the modular arithmetic.\n";

rainbow_cleanup();
clock_cleanup();