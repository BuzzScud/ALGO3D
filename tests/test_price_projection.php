<?php
/**
 * Test script for Price Projection API
 * 
 * This script tests the PHP backend API directly
 * Usage: php test_price_projection.php
 */

// Include the functions directly (not the full API)
require_once __DIR__ . '/api/price_projection.php';

// Extract functions from the API file
// We'll call them directly instead of simulating HTTP

// Test data
$test_config = [
    'historical_prices' => [100.0, 101.0, 102.0, 103.0, 104.0, 105.0],
    'depth_prime' => 127,
    'base' => 105.0,
    'steps' => 20,
    'projection_count' => 3,
    'omega_hz' => 432.0,
    'decimals' => 8
];

echo "=== Price Projection API Test ===\n\n";
echo "Test Configuration:\n";
echo "  - Historical prices: " . count($test_config['historical_prices']) . " points\n";
echo "  - Depth prime: " . $test_config['depth_prime'] . "\n";
echo "  - Base price: " . $test_config['base'] . "\n";
echo "  - Steps: " . $test_config['steps'] . "\n";
echo "  - Projection count: " . $test_config['projection_count'] . "\n";
echo "  - Omega Hz: " . $test_config['omega_hz'] . "\n\n";

// Simulate POST request
$_SERVER['REQUEST_METHOD'] = 'POST';
$_SERVER['CONTENT_TYPE'] = 'application/json';

// Set input
$input_json = json_encode($test_config);
file_put_contents('php://memory', $input_json);

// Capture output
ob_start();

// Call the API (simulate)
try {
    $input = json_decode($input_json, true);
    
    if (!$input) {
        throw new Exception('Invalid JSON input');
    }
    
    // Validate required fields
    $required = ['depth_prime', 'base', 'steps', 'projection_count'];
    foreach ($required as $field) {
        if (!isset($input[$field])) {
            throw new Exception("Missing required field: $field");
        }
    }
    
    // Prepare config
    $config = [
        'historical_prices' => $input['historical_prices'] ?? [],
        'depth_prime' => (int)$input['depth_prime'],
        'base' => (float)$input['base'],
        'steps' => (int)$input['steps'],
        'projection_count' => (int)$input['projection_count'],
        'omega_hz' => isset($input['omega_hz']) ? (float)$input['omega_hz'] : 432.0,
        'decimals' => isset($input['decimals']) ? (int)$input['decimals'] : 8
    ];
    
    // Try different methods
    $result = null;
    $method_used = 'none';
    
    // Try FFI
    if (extension_loaded('ffi')) {
        $library_path = __DIR__ . '/math/math 2/cllm/lib/libcllm.so';
        if (file_exists($library_path)) {
            echo "Attempting FFI method...\n";
            $result = callCLLMviaFFI($config);
            if ($result && $result['success']) {
                $method_used = 'ffi';
            }
        }
    }
    
    // Try exec if FFI failed
    if (!$result || !$result['success']) {
        $executable_path = __DIR__ . '/math/math 2/cllm/bin/cllm_price_projection';
        if (file_exists($executable_path) && is_executable($executable_path)) {
            echo "Attempting exec method...\n";
            $result = callCLLMviaExec($config);
            if ($result && $result['success']) {
                $method_used = 'exec';
            }
        }
    }
    
    // Use fallback if both failed
    if (!$result || !$result['success']) {
        echo "Using fallback method (PHP math)...\n";
        $result = computeProjectionFallback($config);
        $method_used = 'fallback';
    }
    
    $output = ob_get_clean();
    
    // Display results
    echo "\n=== Test Results ===\n";
    echo "Method used: " . $method_used . "\n";
    
    if ($result && $result['success']) {
        echo "✓ SUCCESS\n\n";
        echo "Projection Lines: " . $result['num_lines'] . "\n";
        echo "Steps per line: " . $result['steps_per_line'] . "\n\n";
        
        if (isset($result['projection_lines'])) {
            foreach ($result['projection_lines'] as $idx => $line) {
                echo "Line " . ($idx + 1) . " (first 5 points): ";
                echo implode(', ', array_slice($line, 0, 5)) . "...\n";
            }
        }
    } else {
        echo "✗ FAILED\n";
        echo "Error: " . ($result['error'] ?? 'Unknown error') . "\n";
    }
    
} catch (Exception $e) {
    ob_end_clean();
    echo "✗ EXCEPTION: " . $e->getMessage() . "\n";
    exit(1);
}

echo "\n=== Test Complete ===\n";

