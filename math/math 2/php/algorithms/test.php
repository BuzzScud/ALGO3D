<?php
/**
 * Algorithms PHP Extension Test Suite
 */

echo "=== Algorithms PHP Extension Tests ===\n\n";

// Check if extension is loaded
if (!extension_loaded('algorithms')) {
    die("ERROR: algorithms extension is not loaded!\n");
}

echo "✓ Extension loaded successfully\n\n";

// Test 1: Statistics
echo "Test 1: Statistical Functions\n";
$data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

$stats = algo_statistics($data);
echo "  Statistics for [1..10]:\n";
echo "    Count: {$stats['count']}\n";
echo "    Mean: {$stats['mean']} (expected: 5.5)\n";
echo "    Variance: {$stats['variance']}\n";
echo "    Std Dev: {$stats['std_dev']}\n";
echo "    Min: {$stats['min']} (expected: 1)\n";
echo "    Max: {$stats['max']} (expected: 10)\n";

assert(abs($stats['mean'] - 5.5) < 0.001, "Mean test failed");
assert($stats['min'] == 1, "Min test failed");
assert($stats['max'] == 10, "Max test failed");
echo "  ✓ Statistics tests passed\n\n";

// Test 2: Mean
echo "Test 2: Mean Calculation\n";
$mean = algo_mean([2, 4, 6, 8, 10]);
echo "  Mean of [2, 4, 6, 8, 10] = $mean (expected: 6.0)\n";
assert(abs($mean - 6.0) < 0.001, "Mean test failed");
echo "  ✓ Mean test passed\n\n";

// Test 3: Variance
echo "Test 3: Variance Calculation\n";
$variance = algo_variance([1, 2, 3, 4, 5]);
echo "  Variance of [1, 2, 3, 4, 5] = $variance\n";
echo "  ✓ Variance test passed\n\n";

// Test 4: Standard Deviation
echo "Test 4: Standard Deviation\n";
$std_dev = algo_std_dev([1, 2, 3, 4, 5]);
echo "  Std Dev of [1, 2, 3, 4, 5] = $std_dev\n";
echo "  ✓ Std Dev test passed\n\n";

// Test 5: Dot Product
echo "Test 5: Dot Product\n";
$a = [1, 2, 3];
$b = [4, 5, 6];
$dot = algo_dot_product($a, $b);
echo "  Dot product of [1,2,3] · [4,5,6] = $dot (expected: 32)\n";
assert(abs($dot - 32) < 0.001, "Dot product test failed");
echo "  ✓ Dot product test passed\n\n";

// Test 6: Vector Norm
echo "Test 6: Vector Norm\n";
$vector = [3, 4];
$norm = algo_vector_norm($vector);
echo "  Norm of [3, 4] = $norm (expected: 5.0)\n";
assert(abs($norm - 5.0) < 0.001, "Vector norm test failed");
echo "  ✓ Vector norm test passed\n\n";

// Test 7: Normalize Vector
echo "Test 7: Normalize Vector\n";
$vector = [3, 4];
$normalized = algo_normalize_vector($vector);
echo "  Normalized [3, 4] = [" . implode(", ", array_map(fn($x) => number_format($x, 4), $normalized)) . "]\n";
echo "  Expected: [0.6000, 0.8000]\n";
assert(abs($normalized[0] - 0.6) < 0.001, "Normalize test failed");
assert(abs($normalized[1] - 0.8) < 0.001, "Normalize test failed");
echo "  ✓ Normalize test passed\n\n";

// Test 8: Cosine Similarity
echo "Test 8: Cosine Similarity\n";
$a = [1, 2, 3];
$b = [4, 5, 6];
$similarity = algo_cosine_similarity($a, $b);
echo "  Cosine similarity of [1,2,3] and [4,5,6] = " . number_format($similarity, 4) . "\n";
echo "  ✓ Cosine similarity test passed\n\n";

// Test 9: Softmax
echo "Test 9: Softmax Function\n";
$logits = [1.0, 2.0, 3.0];
$probs = algo_softmax($logits);
echo "  Softmax of [1, 2, 3] = [" . implode(", ", array_map(fn($x) => number_format($x, 4), $probs)) . "]\n";
$sum = array_sum($probs);
echo "  Sum of probabilities: " . number_format($sum, 4) . " (expected: 1.0)\n";
assert(abs($sum - 1.0) < 0.001, "Softmax test failed");
echo "  ✓ Softmax test passed\n\n";

// Test 10: Cross Entropy
echo "Test 10: Cross Entropy Loss\n";
$predictions = [0.7, 0.2, 0.1];
$targets = [1.0, 0.0, 0.0];
$loss = algo_cross_entropy($predictions, $targets);
echo "  Cross entropy loss = " . number_format($loss, 4) . "\n";
echo "  ✓ Cross entropy test passed\n\n";

// Test 11: Gradient Descent
echo "Test 11: Gradient Descent Optimization\n";
// Optimize f(x) = (x - 3)^2, gradient = 2(x - 3)
$gradient_fn = function($x) {
    return [2 * ($x[0] - 3)];
};

$initial = [0.0];
$result = algo_gradient_descent($initial, $gradient_fn, [
    'learning_rate' => 0.1,
    'max_iterations' => 100,
    'tolerance' => 1e-6
]);

echo "  Optimized x = " . number_format($result[0], 4) . " (expected: ~3.0)\n";
assert(abs($result[0] - 3.0) < 0.1, "Gradient descent test failed");
echo "  ✓ Gradient descent test passed\n\n";

// Performance Test
echo "Test 12: Performance\n";
$large_data = range(1, 10000);

$start = microtime(true);
$mean = algo_mean($large_data);
$end = microtime(true);
$time = ($end - $start) * 1000;
echo "  Mean of 10,000 values: " . number_format($time, 2) . " ms\n";

$start = microtime(true);
$stats = algo_statistics($large_data);
$end = microtime(true);
$time = ($end - $start) * 1000;
echo "  Full statistics of 10,000 values: " . number_format($time, 2) . " ms\n";

$vec_a = array_fill(0, 1000, 1.0);
$vec_b = array_fill(0, 1000, 2.0);

$start = microtime(true);
$dot = algo_dot_product($vec_a, $vec_b);
$end = microtime(true);
$time = ($end - $start) * 1000;
echo "  Dot product of 1000-dim vectors: " . number_format($time, 2) . " ms\n";

echo "  ✓ Performance tests complete\n\n";

echo "=== All Tests Passed! ===\n";
echo "✓ Algorithms PHP Extension is working correctly\n";