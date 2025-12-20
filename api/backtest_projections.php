<?php
/**
 * Backtest Projections API
 * 
 * Tests price projections against historical data from QQQ and SPY
 * for specific time periods (9 AM - 12 PM EST)
 */

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

if (file_exists(__DIR__ . '/../includes/config.php')) {
    require_once __DIR__ . '/../includes/config.php';
}

require_once __DIR__ . '/price_projection.php';

/**
 * Get historical data for a symbol and time range
 */
function getHistoricalData($symbol, $startDate, $endDate) {
    global $cacheDir;
    
    // Use charts API to get historical data
    $url = "http://localhost:8000/api/charts.php?action=chart&symbol={$symbol}&timeframe=1H";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 30);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);
    
    if ($httpCode !== 200 || empty($response)) {
        return null;
    }
    
    $data = json_decode($response, true);
    
    if (!$data || !isset($data['candles']) || !is_array($data['candles'])) {
        return null;
    }
    
    // Filter candles by time range (9 AM - 12 PM EST)
    $filteredCandles = [];
    foreach ($data['candles'] as $candle) {
        $timestamp = $candle['time'] ?? $candle['timestamp'] ?? 0;
        if ($timestamp < $startDate || $timestamp > $endDate) {
            continue;
        }
        
        // Check if time is between 9 AM and 12 PM EST
        $date = new DateTime('@' . ($timestamp / 1000));
        $date->setTimezone(new DateTimeZone('America/New_York'));
        $hour = (int)$date->format('H');
        
        if ($hour >= 9 && $hour < 12) {
            $filteredCandles[] = $candle;
        }
    }
    
    return $filteredCandles;
}

/**
 * Calculate projection accuracy
 */
function calculateAccuracy($projections, $actualPrices) {
    if (empty($projections) || empty($actualPrices)) {
        return null;
    }
    
    $errors = [];
    $minLength = min(count($projections), count($actualPrices));
    
    for ($i = 0; $i < $minLength; $i++) {
        $projected = $projections[$i];
        $actual = $actualPrices[$i];
        
        if (is_numeric($projected) && is_numeric($actual) && $actual > 0) {
            $error = abs($projected - $actual) / $actual * 100; // Percentage error
            $errors[] = $error;
        }
    }
    
    if (empty($errors)) {
        return null;
    }
    
    return [
        'mean_error' => array_sum($errors) / count($errors),
        'max_error' => max($errors),
        'min_error' => min($errors),
        'rmse' => sqrt(array_sum(array_map(function($e) { return $e * $e; }, $errors)) / count($errors))
    ];
}

// Main handler
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'error' => 'Method not allowed']);
    exit;
}

try {
    $input = json_decode(file_get_contents('php://input'), true);
    
    if (!$input) {
        throw new Exception('Invalid JSON input');
    }
    
    $symbol = strtoupper($input['symbol'] ?? 'SPY');
    $daysBack = (int)($input['days_back'] ?? 7);
    
    // Calculate date range (last week, 9 AM - 12 PM EST)
    $endDate = time() * 1000; // Current time in milliseconds
    $startDate = $endDate - ($daysBack * 24 * 60 * 60 * 1000);
    
    // Get historical data
    $historicalCandles = getHistoricalData($symbol, $startDate, $endDate);
    
    if (!$historicalCandles || count($historicalCandles) < 10) {
        throw new Exception("Insufficient historical data for {$symbol}");
    }
    
    // Extract prices
    $historicalPrices = array_map(function($candle) {
        return (float)($candle['close'] ?? $candle['price'] ?? 0);
    }, $historicalCandles);
    
    // Split into training and testing sets
    $splitPoint = (int)(count($historicalPrices) * 0.7); // 70% training, 30% testing
    $trainingPrices = array_slice($historicalPrices, 0, $splitPoint);
    $testingPrices = array_slice($historicalPrices, $splitPoint);
    
    // Generate projections using PHP fallback
    $config = [
        'historical_prices' => $trainingPrices,
        'depth_prime' => 31,
        'base' => end($trainingPrices),
        'steps' => count($testingPrices),
        'projection_count' => 12,
        'omega_hz' => 432.0,
        'decimals' => 8
    ];
    
    $projectionResult = computeProjectionFallback($config);
    
    if (!$projectionResult['success']) {
        throw new Exception('Failed to generate projections');
    }
    
    // Calculate accuracy for each projection line
    $accuracyResults = [];
    foreach ($projectionResult['projection_lines'] as $idx => $projectionLine) {
        $accuracy = calculateAccuracy($projectionLine, $testingPrices);
        if ($accuracy) {
            $accuracyResults[] = [
                'line_index' => $idx,
                'mean_error' => $accuracy['mean_error'],
                'max_error' => $accuracy['max_error'],
                'min_error' => $accuracy['min_error'],
                'rmse' => $accuracy['rmse']
            ];
        }
    }
    
    // Calculate overall accuracy (average of all lines)
    $overallAccuracy = null;
    if (!empty($accuracyResults)) {
        $overallAccuracy = [
            'mean_error' => array_sum(array_column($accuracyResults, 'mean_error')) / count($accuracyResults),
            'max_error' => max(array_column($accuracyResults, 'max_error')),
            'min_error' => min(array_column($accuracyResults, 'min_error')),
            'rmse' => array_sum(array_column($accuracyResults, 'rmse')) / count($accuracyResults)
        ];
    }
    
    echo json_encode([
        'success' => true,
        'symbol' => $symbol,
        'training_samples' => count($trainingPrices),
        'testing_samples' => count($testingPrices),
        'projection_lines' => count($projectionResult['projection_lines']),
        'accuracy_per_line' => $accuracyResults,
        'overall_accuracy' => $overallAccuracy,
        'method' => 'php_fallback'
    ]);
    
} catch (Exception $e) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ]);
}

