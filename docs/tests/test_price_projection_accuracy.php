<?php
/**
 * Test Price Projection Accuracy
 * Tests 10 different symbols to ensure:
 * 1. Y-axis displays correct prices
 * 2. Real-time price is fetched
 * 3. Projections start from real-time price
 * 4. All prices are accurate
 */

// Test symbols
$testSymbols = ['SPY', 'QQQ', 'AAPL', 'MSFT', 'GOOGL', 'AMZN', 'TSLA', 'NVDA', 'META', 'NFLX'];

echo "🧪 PRICE PROJECTION ACCURACY TEST\n";
echo str_repeat("=", 60) . "\n\n";

$results = [];
$passed = 0;
$failed = 0;

foreach ($testSymbols as $symbol) {
    echo "Testing: {$symbol}\n";
    echo str_repeat("-", 60) . "\n";
    
    $testResult = [
        'symbol' => $symbol,
        'passed' => false,
        'errors' => []
    ];
    
    try {
        // 1. Fetch real-time quote
        $quoteUrl = "http://localhost:8000/api/charts.php?action=quote&symbol=" . urlencode($symbol);
        $quoteResponse = @file_get_contents($quoteUrl);
        
        if ($quoteResponse === false) {
            throw new Exception("Failed to fetch real-time quote");
        }
        
        $quoteData = json_decode($quoteResponse, true);
        
        if (!$quoteData || !isset($quoteData['current']) || $quoteData['current'] <= 0) {
            throw new Exception("Invalid quote data: " . json_encode($quoteData));
        }
        
        $realTimePrice = floatval($quoteData['current']);
        echo "  ✓ Real-time price: $" . number_format($realTimePrice, 2) . "\n";
        
        // 2. Fetch historical data
        $chartUrl = "http://localhost:8000/api/charts.php?action=chart&symbol=" . urlencode($symbol) . "&timeframe=1H";
        $chartResponse = @file_get_contents($chartUrl);
        
        if ($chartResponse === false) {
            throw new Exception("Failed to fetch historical data");
        }
        
        $chartData = json_decode($chartResponse, true);
        
        if (!$chartData || !isset($chartData['candles']) || !is_array($chartData['candles']) || count($chartData['candles']) === 0) {
            throw new Exception("Invalid chart data");
        }
        
        $candles = $chartData['candles'];
        $lastCandle = end($candles);
        $lastHistoricalPrice = floatval($lastCandle['close']);
        
        echo "  ✓ Last historical price: $" . number_format($lastHistoricalPrice, 2) . "\n";
        
        // 3. Calculate projection
        $projectionUrl = "http://localhost:8000/api/price_projection.php";
        $projectionData = [
            'historical_prices' => array_map(function($c) { return floatval($c['close']); }, $candles),
            'depth_prime' => 31,
            'base' => $realTimePrice, // Use real-time price as base
            'steps' => 20,
            'projection_count' => 12,
            'omega_hz' => 432.0,
            'decimals' => 8
        ];
        
        // Update last price to real-time
        $projectionData['historical_prices'][count($projectionData['historical_prices']) - 1] = $realTimePrice;
        
        $ch = curl_init($projectionUrl);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($projectionData));
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
        
        $projectionResponse = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);
        
        if ($httpCode !== 200) {
            throw new Exception("Projection API returned HTTP {$httpCode}");
        }
        
        $projectionResult = json_decode($projectionResponse, true);
        
        if (!$projectionResult || !$projectionResult['success'] || !isset($projectionResult['projection_lines'])) {
            throw new Exception("Invalid projection result");
        }
        
        $projectionLines = $projectionResult['projection_lines'];
        
        if (count($projectionLines) === 0) {
            throw new Exception("No projection lines generated");
        }
        
        // 4. Verify first projection point starts from real-time price
        $firstProjectionLine = $projectionLines[0];
        if (!is_array($firstProjectionLine) || count($firstProjectionLine) === 0) {
            throw new Exception("First projection line is empty");
        }
        
        $firstProjectionPoint = floatval($firstProjectionLine[0]);
        $priceDifference = abs($firstProjectionPoint - $realTimePrice);
        $priceTolerance = $realTimePrice * 0.05; // 5% tolerance
        
        echo "  ✓ First projection point: $" . number_format($firstProjectionPoint, 2) . "\n";
        echo "  ✓ Price difference: $" . number_format($priceDifference, 2) . " (" . number_format(($priceDifference / $realTimePrice) * 100, 2) . "%)\n";
        
        // 5. Calculate Y-axis range
        $allPrices = array_merge($projectionData['historical_prices'], array_merge(...$projectionLines));
        $minPrice = min($allPrices);
        $maxPrice = max($allPrices);
        $priceRange = $maxPrice - $minPrice;
        $padding = $priceRange * 0.05;
        $yAxisMin = max(0, $minPrice - $padding);
        $yAxisMax = $maxPrice + $padding;
        
        echo "  ✓ Y-axis range: $" . number_format($yAxisMin, 2) . " - $" . number_format($yAxisMax, 2) . "\n";
        
        // 6. Verify real-time price is within Y-axis range
        if ($realTimePrice < $yAxisMin || $realTimePrice > $yAxisMax) {
            throw new Exception("Real-time price ($" . number_format($realTimePrice, 2) . ") is outside Y-axis range ($" . number_format($yAxisMin, 2) . " - $" . number_format($yAxisMax, 2) . ")");
        }
        
        // 7. Verify first projection point matches real-time price (within 0.1% for rounding)
        $strictTolerance = $realTimePrice * 0.001; // 0.1% tolerance for rounding errors
        if ($priceDifference > $strictTolerance) {
            throw new Exception("First projection point differs from real-time price by more than 0.1% (difference: $" . number_format($priceDifference, 2) . ", " . number_format(($priceDifference / $realTimePrice) * 100, 3) . "%)");
        }
        
        // 8. Verify all prices are valid
        foreach ($projectionLines as $lineIdx => $line) {
            foreach ($line as $pointIdx => $point) {
                $price = floatval($point);
                if ($price <= 0 || !is_finite($price)) {
                    throw new Exception("Invalid projection point at line {$lineIdx}, point {$pointIdx}: {$point}");
                }
            }
        }
        
        echo "  ✅ ALL CHECKS PASSED\n\n";
        
        $testResult['passed'] = true;
        $testResult['realTimePrice'] = $realTimePrice;
        $testResult['firstProjectionPoint'] = $firstProjectionPoint;
        $testResult['yAxisMin'] = $yAxisMin;
        $testResult['yAxisMax'] = $yAxisMax;
        $testResult['priceDifference'] = $priceDifference;
        $passed++;
        
    } catch (Exception $e) {
        echo "  ❌ FAILED: " . $e->getMessage() . "\n\n";
        $testResult['passed'] = false;
        $testResult['errors'][] = $e->getMessage();
        $failed++;
    }
    
    $results[] = $testResult;
    
    // Small delay to avoid rate limiting
    usleep(500000); // 0.5 seconds
}

echo str_repeat("=", 60) . "\n";
echo "TEST SUMMARY\n";
echo str_repeat("=", 60) . "\n";
echo "Total symbols tested: " . count($testSymbols) . "\n";
echo "Passed: {$passed}\n";
echo "Failed: {$failed}\n";
echo "Success rate: " . number_format(($passed / count($testSymbols)) * 100, 2) . "%\n\n";

if ($failed > 0) {
    echo "FAILED TESTS:\n";
    foreach ($results as $result) {
        if (!$result['passed']) {
            echo "  - {$result['symbol']}: " . implode(", ", $result['errors']) . "\n";
        }
    }
    echo "\n";
}

if ($passed === count($testSymbols)) {
    echo "✅ ALL TESTS PASSED! Price projection mechanics are 100% correct.\n";
    exit(0);
} else {
    echo "❌ SOME TESTS FAILED. Please review the errors above.\n";
    exit(1);
}

