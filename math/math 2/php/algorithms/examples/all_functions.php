<?php
/**
 * Complete Examples for All Crystalline Algorithms Functions
 * 
 * This file demonstrates every available function in the Crystalline Algorithms library.
 */

echo "=== CRYSTALLINE ALGORITHMS LIBRARY - COMPLETE FUNCTION EXAMPLES ===\n\n";

// ============================================================================
// NUMERICAL OPERATIONS
// ============================================================================
echo "--- NUMERICAL OPERATIONS ---\n";

// Softmax
$logits = [1.0, 2.0, 3.0, 4.0, 5.0];
$probs = numerical_softmax($logits);
echo "Softmax of [1,2,3,4,5]:\n";
print_r($probs);

// Log-softmax
$log_probs = numerical_log_softmax($logits);
echo "Log-softmax:\n";
print_r($log_probs);

// Safe operations
$safe_log = numerical_safe_log(0.0001, 1e-10);
echo "Safe log(0.0001) = $safe_log\n";

$safe_div = numerical_safe_divide(10.0, 0.0, 0.0);
echo "Safe divide(10/0) with default 0 = $safe_div\n";

// Clipping
$clipped = numerical_clip(150.0, 0.0, 100.0);
echo "Clip 150 to [0,100] = $clipped\n";

echo "\n";

// ============================================================================
// STATISTICS OPERATIONS
// ============================================================================
echo "--- STATISTICS OPERATIONS ---\n";

$data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0];

// Mean
$mean = stats_mean($data);
echo "Mean of data: $mean\n";

// Variance
$variance = stats_variance($data, $mean);
echo "Variance: $variance\n";

// Standard deviation
$std_dev = stats_std_dev($data, $mean);
echo "Standard deviation: $std_dev\n";

// Median
$median = stats_median($data);
echo "Median: $median\n";

// Percentile
$p75 = stats_percentile($data, 75.0);
echo "75th percentile: $p75\n";

// Min/Max
$min = stats_min($data);
$max = stats_max($data);
echo "Min: $min, Max: $max\n";

// Correlation
$data2 = [2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0];
$corr = stats_correlation($data, $data2);
echo "Correlation between data and data2: $corr\n";

echo "\n";

// ============================================================================

// ============================================================================
// NOTE: Optimizer and gradient operations are not exposed to PHP
// These are complex C structures better used directly in C code
// ============================================================================

echo "\n=== All available functions demonstrated successfully! ===\n";
