<?php
/**
 * Test Backtest Script
 * Tests price projections with QQQ and SPY data
 */

require_once __DIR__ . '/api/backtest_projections.php';

// Test with QQQ
echo "Testing QQQ projections...\n";
$qqqData = [
    'symbol' => 'QQQ',
    'days_back' => 7
];

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, 'http://localhost:8000/api/backtest_projections.php');
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($qqqData));
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
$response = curl_exec($ch);
curl_close($ch);

$result = json_decode($response, true);
if ($result && $result['success']) {
    echo "QQQ Backtest Results:\n";
    echo "  Training samples: {$result['training_samples']}\n";
    echo "  Testing samples: {$result['testing_samples']}\n";
    echo "  Projection lines: {$result['projection_lines']}\n";
    if ($result['overall_accuracy']) {
        echo "  Overall Mean Error: " . number_format($result['overall_accuracy']['mean_error'], 2) . "%\n";
        echo "  Overall RMSE: " . number_format($result['overall_accuracy']['rmse'], 2) . "%\n";
    }
} else {
    echo "QQQ Backtest failed: " . ($result['error'] ?? 'Unknown error') . "\n";
}

echo "\n";

// Test with SPY
echo "Testing SPY projections...\n";
$spyData = [
    'symbol' => 'SPY',
    'days_back' => 7
];

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, 'http://localhost:8000/api/backtest_projections.php');
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($spyData));
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
$response = curl_exec($ch);
curl_close($ch);

$result = json_decode($response, true);
if ($result && $result['success']) {
    echo "SPY Backtest Results:\n";
    echo "  Training samples: {$result['training_samples']}\n";
    echo "  Testing samples: {$result['testing_samples']}\n";
    echo "  Projection lines: {$result['projection_lines']}\n";
    if ($result['overall_accuracy']) {
        echo "  Overall Mean Error: " . number_format($result['overall_accuracy']['mean_error'], 2) . "%\n";
        echo "  Overall RMSE: " . number_format($result['overall_accuracy']['rmse'], 2) . "%\n";
    }
} else {
    echo "SPY Backtest failed: " . ($result['error'] ?? 'Unknown error') . "\n";
}

echo "\nDone!\n";

