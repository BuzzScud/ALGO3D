<?php
/**
 * Complete Examples for All Crystalline Math Functions
 * 
 * This file demonstrates every available function in the Crystalline Math library.
 */

echo "=== CRYSTALLINE MATH LIBRARY - COMPLETE FUNCTION EXAMPLES ===\n\n";

// ============================================================================
// ARITHMETIC OPERATIONS
// ============================================================================
echo "--- ARITHMETIC OPERATIONS ---\n";

// Addition
$sum = math_add(10.5, 20.3);
echo "math_add(10.5, 20.3) = $sum\n";

// Subtraction
$diff = math_sub(50.0, 15.5);
echo "math_sub(50.0, 15.5) = $diff\n";

// Multiplication
$product = math_mul(7.5, 4.0);
echo "math_mul(7.5, 4.0) = $product\n";

// Division
$quotient = math_div(100.0, 8.0);
echo "math_div(100.0, 8.0) = $quotient\n";

// Modulo
$remainder = math_mod(17.0, 5.0);
echo "math_mod(17.0, 5.0) = $remainder\n";

// Absolute value
$abs_val = math_abs(-42.5);
echo "math_abs(-42.5) = $abs_val\n";

// Minimum
$min_val = math_min(15.0, 23.0);
echo "math_min(15.0, 23.0) = $min_val\n";

// Maximum
$max_val = math_max(15.0, 23.0);
echo "math_max(15.0, 23.0) = $max_val\n";

// Clamp
$clamped = math_clamp(150.0, 0.0, 100.0);
echo "math_clamp(150.0, 0.0, 100.0) = $clamped\n";

// Sign
$sign = math_sign(-5.5);
echo "math_sign(-5.5) = $sign\n";

// Floor
$floor_val = math_floor(3.7);
echo "math_floor(3.7) = $floor_val\n";

// Ceiling
$ceil_val = math_ceil(3.2);
echo "math_ceil(3.2) = $ceil_val\n";

// Round
$rounded = math_round(3.6);
echo "math_round(3.6) = $rounded\n";

// Truncate
$truncated = math_trunc(3.9);
echo "math_trunc(3.9) = $truncated\n";

echo "\n";

// ============================================================================
// TRANSCENDENTAL FUNCTIONS
// ============================================================================
echo "--- TRANSCENDENTAL FUNCTIONS ---\n";

// Square root
$sqrt_val = math_sqrt(16.0);
echo "math_sqrt(16.0) = $sqrt_val\n";

// Cube root
$cbrt_val = math_cbrt(27.0);
echo "math_cbrt(27.0) = $cbrt_val\n";

// Power
$pow_val = math_pow(2.0, 10.0);
echo "math_pow(2.0, 10.0) = $pow_val\n";

// Exponential
$exp_val = math_exp(1.0);
echo "math_exp(1.0) = $exp_val (e)\n";

// Natural logarithm
$log_val = math_log(2.718281828);
echo "math_log(2.718281828) = $log_val\n";

// Base-10 logarithm
$log10_val = math_log10(100.0);
echo "math_log10(100.0) = $log10_val\n";

// Base-2 logarithm
$log2_val = math_log2(8.0);
echo "math_log2(8.0) = $log2_val\n";

// Sine
$sin_val = math_sin(M_PI / 2);
echo "math_sin(π/2) = $sin_val\n";

// Cosine
$cos_val = math_cos(0.0);
echo "math_cos(0) = $cos_val\n";

// Tangent
$tan_val = math_tan(M_PI / 4);
echo "math_tan(π/4) = $tan_val\n";

// Arcsine
$asin_val = math_asin(1.0);
echo "math_asin(1.0) = $asin_val\n";

// Arccosine
$acos_val = math_acos(0.0);
echo "math_acos(0.0) = $acos_val\n";

// Arctangent
$atan_val = math_atan(1.0);
echo "math_atan(1.0) = $atan_val\n";

// Two-argument arctangent
$atan2_val = math_atan2(1.0, 1.0);
echo "math_atan2(1.0, 1.0) = $atan2_val\n";

// Hyperbolic sine
$sinh_val = math_sinh(1.0);
echo "math_sinh(1.0) = $sinh_val\n";

// Hyperbolic cosine
$cosh_val = math_cosh(0.0);
echo "math_cosh(0.0) = $cosh_val\n";

// Hyperbolic tangent
$tanh_val = math_tanh(0.5);
echo "math_tanh(0.5) = $tanh_val\n";

echo "\n";

// ============================================================================
// PRIME NUMBER OPERATIONS
// ============================================================================
echo "--- PRIME NUMBER OPERATIONS ---\n";

// Get nth prime
$prime_10 = prime_nth(10);
echo "prime_nth(10) = $prime_10 (10th prime)\n";

// Next prime
$next_prime = prime_next(100);
echo "prime_next(100) = $next_prime\n";

// Previous prime
$prev_prime = prime_prev(100);
echo "prime_prev(100) = $prev_prime\n";

// Check if prime
$is_prime_17 = prime_is_prime(17);
$is_prime_18 = prime_is_prime(18);
echo "prime_is_prime(17) = " . ($is_prime_17 ? "true" : "false") . "\n";
echo "prime_is_prime(18) = " . ($is_prime_18 ? "true" : "false") . "\n";

// Count primes below n
$count = prime_count_below(100);
echo "prime_count_below(100) = $count\n";

// Prime factorization
$factors = prime_factor(60);
echo "prime_factor(60) = ";
print_r($factors);

echo "\n";

// ============================================================================
// COMPLEX NUMBER OPERATIONS
// ============================================================================
echo "--- COMPLEX NUMBER OPERATIONS ---\n";

// Create complex numbers
$z1 = complex_create(3.0, 4.0);
$z2 = complex_create(1.0, 2.0);
echo "z1 = 3 + 4i\n";
echo "z2 = 1 + 2i\n";

// Add complex numbers
$z_sum = complex_add($z1, $z2);
echo "z1 + z2 = " . $z_sum['real'] . " + " . $z_sum['imag'] . "i\n";

// Subtract complex numbers
$z_diff = complex_sub($z1, $z2);
echo "z1 - z2 = " . $z_diff['real'] . " + " . $z_diff['imag'] . "i\n";

// Multiply complex numbers
$z_prod = complex_mul($z1, $z2);
echo "z1 * z2 = " . $z_prod['real'] . " + " . $z_prod['imag'] . "i\n";

// Divide complex numbers
$z_quot = complex_div($z1, $z2);
echo "z1 / z2 = " . $z_quot['real'] . " + " . $z_quot['imag'] . "i\n";

// Magnitude
$mag = complex_magnitude($z1);
echo "|z1| = $mag\n";

// Phase
$phase = complex_phase($z1);
echo "arg(z1) = $phase radians\n";

// Conjugate
$conj = complex_conjugate($z1);
echo "conj(z1) = " . $conj['real'] . " + " . $conj['imag'] . "i\n";

// Exponential
$z_exp = complex_exp($z1);
echo "exp(z1) = " . $z_exp['real'] . " + " . $z_exp['imag'] . "i\n";

echo "\n";

// ============================================================================
// ARBITRARY PRECISION (ABACUS) OPERATIONS
// ============================================================================
echo "--- ARBITRARY PRECISION (ABACUS) OPERATIONS ---\n";

// Create abacus numbers
$a1 = abacus_create();
$a2 = abacus_create();
abacus_from_uint64($a1, 12345);
abacus_from_uint64($a2, 67890);
echo "a1 = 12345\n";
echo "a2 = 67890\n";

// Add
$a_sum = abacus_create();
abacus_add($a_sum, $a1, $a2);
$sum_str = abacus_to_string($a_sum);
echo "a1 + a2 = $sum_str\n";

// Subtract
$a_diff = abacus_create();
abacus_sub($a_diff, $a2, $a1);
$diff_str = abacus_to_string($a_diff);
echo "a2 - a1 = $diff_str\n";

// Multiply
$a_prod = abacus_create();
abacus_mul($a_prod, $a1, $a2);
$prod_str = abacus_to_string($a_prod);
echo "a1 * a2 = $prod_str\n";

// Compare
$cmp = abacus_compare($a1, $a2);
echo "compare(a1, a2) = $cmp (negative means a1 < a2)\n";

// From string
$a_big = abacus_from_string("123456789012345678901234567890");
$big_str = abacus_to_string($a_big);
echo "Large number: $big_str\n";

echo "\n";

// ============================================================================
// CLOCK LATTICE OPERATIONS
// ============================================================================
echo "--- CLOCK LATTICE OPERATIONS ---\n";

// Map prime to clock position
$pos = clock_map_prime(17);
echo "clock_map_prime(17):\n";
echo "  Ring: " . $pos['ring'] . "\n";
echo "  Position: " . $pos['position'] . "\n";
echo "  Angle: " . $pos['angle'] . " radians\n";

// Get prime from position
$prime_from_pos = clock_position_to_prime($pos);
echo "clock_position_to_prime(pos) = $prime_from_pos\n";

// Angular distance
$pos1 = clock_map_prime(17);
$pos2 = clock_map_prime(19);
$distance = clock_angular_distance($pos1, $pos2);
echo "Angular distance between 17 and 19: $distance radians\n";

echo "\n";

// ============================================================================
// NTT (NUMBER THEORETIC TRANSFORM) OPERATIONS
// ============================================================================
echo "--- NTT OPERATIONS ---\n";

// Create NTT context
$ntt_ctx = ntt_create(8);
echo "Created NTT context for size 8\n";

// Find primitive root
$root = ntt_find_primitive_root(8, 17);
echo "Primitive root for n=8, p=17: $root\n";

// Forward NTT
$input = [1, 2, 3, 4, 5, 6, 7, 8];
$output = ntt_forward($ntt_ctx, $input);
echo "NTT forward transform:\n";
print_r($output);

// Inverse NTT
$recovered = ntt_inverse($ntt_ctx, $output);
echo "NTT inverse transform (should match input):\n";
print_r($recovered);

echo "\n";

// ============================================================================
// PLATONIC SOLID OPERATIONS
// ============================================================================
echo "--- PLATONIC SOLID OPERATIONS ---\n";

// Create platonic solids
$tetrahedron = platonic_create(0); // Tetrahedron
$cube = platonic_create(1);        // Cube
$octahedron = platonic_create(2);  // Octahedron
$dodecahedron = platonic_create(3); // Dodecahedron
$icosahedron = platonic_create(4);  // Icosahedron

echo "Tetrahedron: V=" . $tetrahedron['vertices'] . ", E=" . $tetrahedron['edges'] . ", F=" . $tetrahedron['faces'] . "\n";
echo "Cube: V=" . $cube['vertices'] . ", E=" . $cube['edges'] . ", F=" . $cube['faces'] . "\n";
echo "Octahedron: V=" . $octahedron['vertices'] . ", E=" . $octahedron['edges'] . ", F=" . $octahedron['faces'] . "\n";
echo "Dodecahedron: V=" . $dodecahedron['vertices'] . ", E=" . $dodecahedron['edges'] . ", F=" . $dodecahedron['faces'] . "\n";
echo "Icosahedron: V=" . $icosahedron['vertices'] . ", E=" . $icosahedron['edges'] . ", F=" . $icosahedron['faces'] . "\n";

// Verify Euler's formula (V - E + F = 2)
$euler_check = $cube['vertices'] - $cube['edges'] + $cube['faces'];
echo "Euler's formula for cube: V - E + F = $euler_check (should be 2)\n";

echo "\n";

// ============================================================================
// RAINBOW TABLE OPERATIONS
// ============================================================================
echo "--- RAINBOW TABLE OPERATIONS ---\n";

// Create and populate rainbow table
$rainbow = rainbow_create(1000);
rainbow_populate_count($rainbow, 100);
echo "Created rainbow table with 100 primes\n";

// Lookup by index
$prime_50 = rainbow_lookup_by_index($rainbow, 50);
echo "50th prime from rainbow table: $prime_50\n";

// Lookup position
$pos = rainbow_lookup_position($rainbow, 97);
echo "Position of prime 97 in rainbow table:\n";
echo "  Ring: " . $pos['ring'] . "\n";
echo "  Position: " . $pos['position'] . "\n";

// Get table size
$size = rainbow_size($rainbow);
echo "Rainbow table size: $size primes\n";

echo "\n";

// ============================================================================
// VALIDATION FUNCTIONS
// ============================================================================
echo "--- VALIDATION FUNCTIONS ---\n";

// Check if NaN
$is_nan = math_is_nan(sqrt(-1));
echo "math_is_nan(sqrt(-1)) = " . ($is_nan ? "true" : "false") . "\n";

// Check if infinite
$is_inf = math_is_inf(1.0 / 0.0);
echo "math_is_inf(1/0) = " . ($is_inf ? "true" : "false") . "\n";

// Check if finite
$is_finite = math_is_finite(42.0);
echo "math_is_finite(42.0) = " . ($is_finite ? "true" : "false") . "\n";

// Approximate equality
$approx_eq = math_approx_equal(0.1 + 0.2, 0.3, 1e-10);
echo "math_approx_equal(0.1+0.2, 0.3) = " . ($approx_eq ? "true" : "false") . "\n";

echo "\n";

// ============================================================================
// SPHERE TRAJECTORY OPERATIONS
// ============================================================================
echo "--- SPHERE TRAJECTORY OPERATIONS ---\n";

// Get sphere index for prime
$sphere_idx = sphere_get_index(17);
echo "Sphere index for prime 17: $sphere_idx\n";

// Get local position on sphere
$local_pos = sphere_get_local_position(17);
echo "Local position on sphere: $local_pos\n";

// Calculate trajectory
$trajectory = sphere_calculate_trajectory($sphere_idx);
echo "Trajectory for sphere $sphere_idx:\n";
print_r($trajectory);

// Distance between primes on spheres
$dist = sphere_prime_distance(17, 19);
echo "Distance between primes 17 and 19 on spheres: $dist\n";

echo "\n";

echo "=== ALL FUNCTION EXAMPLES COMPLETE ===\n";
?>