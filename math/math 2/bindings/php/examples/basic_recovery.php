<?php
/**
 * Basic Recovery Example
 * 
 * Demonstrates basic data recovery using the PHP Recovery extension
 */

// Create a recovery context
$context = recovery_create_context(1000, 1e-6);
if (!$context) {
    die("Failed to create recovery context\n");
}

// Add sample data (known good data)
$sample1 = "This is a sample of good data that we know is correct.";
$sample2 = "Another sample of good data for better convergence.";
$sample3 = "Third sample helps achieve 27% better convergence!";

recovery_add_sample($context, $sample1);
recovery_add_sample($context, $sample2);
recovery_add_sample($context, $sample3);

// Corrupted data to recover
$corrupted = "Th1s 1s @ s@mpl3 0f g00d d@t@ th@t w3 kn0w 1s c0rr3ct.";

echo "Original (corrupted): $corrupted\n";

// Recover the data
$recovered = recovery_recover($context, $corrupted);
if ($recovered === false) {
    die("Recovery failed\n");
}

echo "Recovered: $recovered\n";

// Get recovery statistics
$iterations = recovery_get_iterations($context);
$quality = recovery_get_quality($context);

echo "\nRecovery Statistics:\n";
echo "  Iterations: $iterations\n";
echo "  Quality: " . number_format($quality * 100, 2) . "%\n";

// Clean up
recovery_destroy_context($context);

echo "\nRecovery complete!\n";
?>