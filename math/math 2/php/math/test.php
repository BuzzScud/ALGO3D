<?php
/**
 * Crystalline Math PHP Extension Test Suite
 */

echo "=== Crystalline Math PHP Extension Tests ===\n\n";

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("ERROR: crystalline_math extension is not loaded!\n");
}

echo "✓ Extension loaded successfully\n\n";

// Test 1: GCD
echo "Test 1: GCD Calculation\n";
$gcd1 = crystalline_gcd(48, 18);
echo "  GCD(48, 18) = $gcd1 (expected: 6)\n";
assert($gcd1 === 6, "GCD test failed");

$gcd2 = crystalline_gcd(100, 50);
echo "  GCD(100, 50) = $gcd2 (expected: 50)\n";
assert($gcd2 === 50, "GCD test failed");
echo "  ✓ GCD tests passed\n\n";

// Test 2: Prime Check
echo "Test 2: Prime Number Check\n";
$primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29];
$non_primes = [4, 6, 8, 9, 10, 12, 14, 15, 16, 18];

foreach ($primes as $p) {
    $is_prime = crystalline_is_prime($p);
    echo "  is_prime($p) = " . ($is_prime ? 'true' : 'false') . "\n";
    assert($is_prime === true, "Prime check failed for $p");
}

foreach ($non_primes as $n) {
    $is_prime = crystalline_is_prime($n);
    assert($is_prime === false, "Prime check failed for $n");
}
echo "  ✓ Prime check tests passed\n\n";

// Test 3: Next Prime
echo "Test 3: Next Prime\n";
$next = crystalline_next_prime(10);
echo "  next_prime(10) = $next (expected: 11)\n";
assert($next === 11, "Next prime test failed");

$next = crystalline_next_prime(20);
echo "  next_prime(20) = $next (expected: 23)\n";
assert($next === 23, "Next prime test failed");
echo "  ✓ Next prime tests passed\n\n";

// Test 4: Prime Factorization
echo "Test 4: Prime Factorization\n";
$factors = crystalline_prime_factors(60);
echo "  prime_factors(60) = [" . implode(", ", $factors) . "] (expected: [2, 2, 3, 5])\n";
assert($factors === [2, 2, 3, 5], "Prime factorization test failed");

$factors = crystalline_prime_factors(100);
echo "  prime_factors(100) = [" . implode(", ", $factors) . "] (expected: [2, 2, 5, 5])\n";
assert($factors === [2, 2, 5, 5], "Prime factorization test failed");
echo "  ✓ Prime factorization tests passed\n\n";

// Test 5: Prime Sieve
echo "Test 5: Prime Sieve\n";
$primes = crystalline_sieve(30);
echo "  sieve(30) = [" . implode(", ", $primes) . "]\n";
echo "  Found " . count($primes) . " primes up to 30\n";
assert(count($primes) === 10, "Sieve test failed");
echo "  ✓ Sieve tests passed\n\n";

// Test 6: Mathematical Functions
echo "Test 6: Mathematical Functions\n";

$sqrt = crystalline_sqrt(16);
echo "  sqrt(16) = $sqrt (expected: 4.0)\n";
assert(abs($sqrt - 4.0) < 0.0001, "Sqrt test failed");

$pow = crystalline_pow(2, 10);
echo "  pow(2, 10) = $pow (expected: 1024.0)\n";
assert(abs($pow - 1024.0) < 0.0001, "Pow test failed");

$sin = crystalline_sin(0);
echo "  sin(0) = $sin (expected: 0.0)\n";
assert(abs($sin) < 0.0001, "Sin test failed");

$cos = crystalline_cos(0);
echo "  cos(0) = $cos (expected: 1.0)\n";
assert(abs($cos - 1.0) < 0.0001, "Cos test failed");

$log = crystalline_log(M_E);
echo "  log(e) = $log (expected: 1.0)\n";
assert(abs($log - 1.0) < 0.0001, "Log test failed");

$exp = crystalline_exp(0);
echo "  exp(0) = $exp (expected: 1.0)\n";
assert(abs($exp - 1.0) < 0.0001, "Exp test failed");

echo "  ✓ Mathematical function tests passed\n\n";

// Performance Test
echo "Test 7: Performance\n";
$start = microtime(true);
for ($i = 0; $i < 1000; $i++) {
    crystalline_gcd(12345, 67890);
}
$end = microtime(true);
$time = ($end - $start) * 1000;
echo "  1000 GCD calculations: " . number_format($time, 2) . " ms\n";

$start = microtime(true);
for ($i = 0; $i < 1000; $i++) {
    crystalline_is_prime(104729); // Large prime
}
$end = microtime(true);
$time = ($end - $start) * 1000;
echo "  1000 prime checks: " . number_format($time, 2) . " ms\n";

$start = microtime(true);
$primes = crystalline_sieve(10000);
$end = microtime(true);
$time = ($end - $start) * 1000;
echo "  Sieve up to 10000: " . number_format($time, 2) . " ms (" . count($primes) . " primes found)\n";
echo "  ✓ Performance tests complete\n\n";

echo "=== All Tests Passed! ===\n";
echo "✓ Crystalline Math PHP Extension is working correctly\n";