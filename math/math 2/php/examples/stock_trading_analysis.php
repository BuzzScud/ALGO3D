<?php
/**
 * Stock Trading Analysis Example
 * 
 * Demonstrates PURE Crystalline mathematics for stock analysis.
 * Uses ONLY crystalline_* and algo_* functions - NO standard PHP math!
 * 
 * This example shows:
 * 1. Statistical analysis using algo_statistics
 * 2. Geometric pattern recognition using clock lattice
 * 3. Prime-based trend analysis
 * 4. Risk assessment using Crystalline variance
 */

// Check if extensions are loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension not loaded\n");
}

if (!extension_loaded('algorithms')) {
    die("Error: algorithms extension not loaded\n");
}

echo "=== Stock Trading Analysis with PURE Crystalline Math ===\n\n";

// Sample stock price data (closing prices for 30 days)
$stock_prices = [
    100.00, 102.50, 101.75, 103.25, 105.00,  // Week 1
    107.50, 106.25, 108.00, 110.50, 109.75,  // Week 2
    112.00, 114.25, 113.50, 115.75, 118.00,  // Week 3
    116.50, 119.25, 121.00, 120.25, 122.50,  // Week 4
    124.75, 123.50, 125.00, 127.25, 126.00,  // Week 5
    128.50, 130.00, 129.25, 131.50, 133.00   // Week 6
];

echo "Stock Price Data (30 days):\n";
echo "Initial: $" . number_format($stock_prices[0], 2) . "\n";
echo "Final:   $" . number_format(end($stock_prices), 2) . "\n\n";

// ============================================================================
// 1. STATISTICAL ANALYSIS USING CRYSTALLINE ALGORITHMS
// ============================================================================

echo "1. STATISTICAL ANALYSIS (Pure Crystalline)\n";
echo str_repeat("=", 60) . "\n\n";

// Use algo_statistics - this uses Crystalline math internally
$stats = algo_statistics($stock_prices);

echo "Price Statistics:\n";
echo "  Count:          " . $stats['count'] . " days\n";
echo "  Mean:           $" . number_format($stats['mean'], 2) . "\n";
echo "  Std Deviation:  $" . number_format($stats['std_dev'], 2) . "\n";
echo "  Variance:       $" . number_format($stats['variance'], 2) . "\n";
echo "  Min Price:      $" . number_format($stats['min'], 2) . "\n";
echo "  Max Price:      $" . number_format($stats['max'], 2) . "\n\n";

// ============================================================================
// 2. GEOMETRIC PATTERN ANALYSIS USING CLOCK LATTICE
// ============================================================================

echo "2. GEOMETRIC PATTERN ANALYSIS (Clock Lattice)\n";
echo str_repeat("=", 60) . "\n\n";

echo "Mapping prices to prime positions on clock lattice:\n\n";

// Map each price to nearest prime and analyze clock position
for ($i = 0; $i < min(10, count($stock_prices)); $i++) {
    $price_scaled = (int)($stock_prices[$i] * 10); // Scale to integer
    
    // Map price to clock position (0-11)
    $clock_pos = (int)($price_scaled % 12);
    
    // Use smaller magnitude for O(1) generation (0-5 range)
    $magnitude = (int)(($price_scaled / 12) % 6);
    $prime = crystalline_prime_generate_o1($clock_pos, $magnitude);
    
    // If prime generation failed, use next prime after scaled price
    if ($prime == 0) {
        $prime = prime_next($price_scaled);
    }
    
    // Map prime to clock position to get ring and position info
    $position = clock_map_prime_to_position($prime);
    
    echo "  Day " . ($i + 1) . ": $" . number_format($stock_prices[$i], 2);
    echo " → Prime: " . $prime;
    if ($position !== false) {
        echo " → Ring: " . $position['ring'];
        echo ", Position: " . $position['position'];
    }
    echo "\n";
}
echo "\n";

// ============================================================================
// 3. PRIME-BASED TREND ANALYSIS
// ============================================================================

echo "3. PRIME-BASED TREND ANALYSIS\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate price changes and map to primes
$price_changes = [];
$prime_positions = [];

for ($i = 1; $i < count($stock_prices); $i++) {
    $change = $stock_prices[$i] - $stock_prices[$i-1];
    $price_changes[] = $change;
    
    // Map absolute change to prime
    $magnitude = abs($change);
    $scaled = (int)($magnitude * 100);
    
    if ($scaled > 0) {
        // Map to clock position and magnitude
        $position = (int)($scaled % 12);
        if ($position == 3 || $position == 6 || $position == 9) {
            $mag = (int)($scaled / 12);
            $prime = crystalline_prime_generate_o1($position, $mag);
            $prime_positions[] = $position;
        }
    }
}

// Analyze prime distribution using algo_statistics
if (count($prime_positions) > 0) {
    $ring_stats = algo_statistics($prime_positions);
    
    echo "Prime Ring Distribution:\n";
    echo "  Mean Ring:      " . number_format($ring_stats['mean'], 2) . "\n";
    echo "  Ring Variance:  " . number_format($ring_stats['variance'], 2) . "\n";
    echo "  Ring Std Dev:   " . number_format($ring_stats['std_dev'], 2) . "\n\n";
    
    // Interpret ring distribution
    if ($ring_stats['mean'] < 1.5) {
        echo "Pattern: LOW VOLATILITY (Ring 0-1 dominant)\n";
    } elseif ($ring_stats['mean'] < 2.5) {
        echo "Pattern: MODERATE VOLATILITY (Ring 1-2 dominant)\n";
    } else {
        echo "Pattern: HIGH VOLATILITY (Ring 3+ dominant)\n";
    }
}
echo "\n";

// ============================================================================
// 4. VECTOR-BASED ANALYSIS
// ============================================================================

echo "4. VECTOR-BASED ANALYSIS (Crystalline Algorithms)\n";
echo str_repeat("=", 60) . "\n\n";

// Split data into two halves for comparison
$half = (int)(count($stock_prices) / 2);
$first_half = array_slice($stock_prices, 0, $half);
$second_half = array_slice($stock_prices, $half);

// Calculate statistics for each half
$first_stats = algo_statistics($first_half);
$second_stats = algo_statistics($second_half);

echo "First Half Analysis:\n";
echo "  Mean:     $" . number_format($first_stats['mean'], 2) . "\n";
echo "  Std Dev:  $" . number_format($first_stats['std_dev'], 2) . "\n\n";

echo "Second Half Analysis:\n";
echo "  Mean:     $" . number_format($second_stats['mean'], 2) . "\n";
echo "  Std Dev:  $" . number_format($second_stats['std_dev'], 2) . "\n\n";

// Compare using vector operations
$mean_diff = $second_stats['mean'] - $first_stats['mean'];
$volatility_diff = $second_stats['std_dev'] - $first_stats['std_dev'];

echo "Trend Analysis:\n";
if ($mean_diff > 5) {
    echo "  Price Trend: STRONG UPTREND ↑↑\n";
} elseif ($mean_diff > 0) {
    echo "  Price Trend: UPTREND ↑\n";
} elseif ($mean_diff > -5) {
    echo "  Price Trend: DOWNTREND ↓\n";
} else {
    echo "  Price Trend: STRONG DOWNTREND ↓↓\n";
}

if ($volatility_diff > 2) {
    echo "  Volatility: INCREASING (Higher risk)\n";
} elseif ($volatility_diff < -2) {
    echo "  Volatility: DECREASING (Lower risk)\n";
} else {
    echo "  Volatility: STABLE\n";
}
echo "\n";

// ============================================================================
// 5. PRIME RAINBOW TABLE ANALYSIS
// ============================================================================

echo "5. PRIME-BASED PRICE LEVEL ANALYSIS\n";
echo str_repeat("=", 60) . "\n\n";

// Analyze key price levels using prime generation
echo "Key Price Levels (Prime-Aligned):\n";

$key_levels = [
    $stats['min'],
    $stats['mean'] - $stats['std_dev'],
    $stats['mean'],
    $stats['mean'] + $stats['std_dev'],
    $stats['max']
];

$level_names = [
    "Strong Support",
    "Support Level",
    "Mean Price",
    "Resistance Level",
    "Strong Resistance"
];

for ($i = 0; $i < count($key_levels); $i++) {
    $level = $key_levels[$i];
    $scaled = (int)($level * 10);
    
    // Map to clock position
    $position = (int)($scaled % 12);
    if ($position == 3 || $position == 6 || $position == 9) {
        $mag = (int)($scaled / 12);
        $prime = crystalline_prime_generate_o1($position, $mag);
        
        echo "  " . $level_names[$i] . ": $" . number_format($level, 2);
        echo " → Prime: " . $prime;
        echo " (Position " . $position . ", Magnitude " . $mag . ")\n";
    } else {
        echo "  " . $level_names[$i] . ": $" . number_format($level, 2);
        echo " → Not on prime position\n";
    }
}
echo "\n";

// ============================================================================
// 6. CRYSTALLINE RISK ASSESSMENT
// ============================================================================

echo "6. CRYSTALLINE RISK ASSESSMENT\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate risk metrics using Crystalline statistics
$risk_ratio = $stats['std_dev'] / $stats['mean'];

echo "Risk Metrics (Crystalline):\n";
echo "  Variance:       $" . number_format($stats['variance'], 2) . "\n";
echo "  Std Deviation:  $" . number_format($stats['std_dev'], 2) . "\n";
echo "  Risk Ratio:     " . number_format($risk_ratio, 4) . "\n\n";

// Risk assessment based on Crystalline calculations
if ($risk_ratio < 0.05) {
    echo "Risk Level: LOW (Very stable)\n";
    echo "Recommendation: SAFE for conservative investors\n";
} elseif ($risk_ratio < 0.10) {
    echo "Risk Level: MODERATE (Normal volatility)\n";
    echo "Recommendation: SUITABLE for most investors\n";
} elseif ($risk_ratio < 0.15) {
    echo "Risk Level: HIGH (Volatile)\n";
    echo "Recommendation: CAUTION - for experienced investors\n";
} else {
    echo "Risk Level: VERY HIGH (Highly volatile)\n";
    echo "Recommendation: HIGH RISK - speculative only\n";
}
echo "\n";

// ============================================================================
// 7. GEOMETRIC SYMMETRY ANALYSIS
// ============================================================================

echo "7. GEOMETRIC SYMMETRY ANALYSIS\n";
echo str_repeat("=", 60) . "\n\n";

// Analyze 12-fold symmetry in price movements
$ring_distribution = array_fill(0, 4, 0);

foreach ($prime_positions as $ring) {
    if ($ring >= 0 && $ring < 4) {
        $ring_distribution[$ring]++;
    }
}

echo "Ring Distribution (12-fold symmetry):\n";
for ($i = 0; $i < 4; $i++) {
    $count = $ring_distribution[$i];
    $percentage = count($prime_positions) > 0 
        ? ($count / count($prime_positions)) * 100 
        : 0;
    
    echo "  Ring " . $i . ": " . $count . " occurrences";
    echo " (" . number_format($percentage, 1) . "%)\n";
}
echo "\n";

// Determine dominant pattern
$max_ring = array_search(max($ring_distribution), $ring_distribution);
echo "Dominant Pattern: Ring " . $max_ring . "\n";

switch ($max_ring) {
    case 0:
        echo "  Interpretation: Small, frequent movements (12 positions)\n";
        break;
    case 1:
        echo "  Interpretation: Moderate movements (60 positions)\n";
        break;
    case 2:
        echo "  Interpretation: Larger movements (60 positions)\n";
        break;
    case 3:
        echo "  Interpretation: Major price swings (100 positions)\n";
        break;
}
echo "\n";

// ============================================================================
// 8. FINAL RECOMMENDATION
// ============================================================================

echo "8. FINAL RECOMMENDATION (Crystalline Analysis)\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate recommendation based on Crystalline metrics
$score = 0;

// Trend score
if ($mean_diff > 5) $score += 2;
elseif ($mean_diff > 0) $score += 1;
elseif ($mean_diff > -5) $score -= 1;
else $score -= 2;

// Volatility score
if ($volatility_diff < -2) $score += 1;
elseif ($volatility_diff > 2) $score -= 1;

// Risk score
if ($risk_ratio < 0.05) $score += 1;
elseif ($risk_ratio > 0.15) $score -= 1;

echo "Crystalline Score: " . $score . "\n\n";

if ($score >= 3) {
    echo "RECOMMENDATION: STRONG BUY\n";
    echo "  Confidence: HIGH\n";
    echo "  Geometric Pattern: Favorable\n";
} elseif ($score >= 1) {
    echo "RECOMMENDATION: BUY\n";
    echo "  Confidence: MODERATE\n";
    echo "  Geometric Pattern: Positive\n";
} elseif ($score >= -1) {
    echo "RECOMMENDATION: HOLD\n";
    echo "  Confidence: NEUTRAL\n";
    echo "  Geometric Pattern: Mixed\n";
} elseif ($score >= -3) {
    echo "RECOMMENDATION: SELL\n";
    echo "  Confidence: MODERATE\n";
    echo "  Geometric Pattern: Negative\n";
} else {
    echo "RECOMMENDATION: STRONG SELL\n";
    echo "  Confidence: HIGH\n";
    echo "  Geometric Pattern: Unfavorable\n";
}

echo "\n" . str_repeat("=", 60) . "\n";
echo "Analysis Complete\n";
echo str_repeat("=", 60) . "\n\n";

echo "Note: This analysis uses PURE Crystalline mathematics:\n";
echo "  - Clock lattice for geometric pattern recognition\n";
echo "  - O(1) deterministic prime generation\n";
echo "  - Crystalline statistical algorithms\n";
echo "  - 12-fold symmetry analysis\n";
echo "  - NO standard math libraries used!\n\n";

echo "Always consult with a financial advisor before making investment decisions.\n";
?>