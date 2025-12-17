<?php
/**
 * Fibonacci Calculator API
 * Fetches YTD market data and calculates Fibonacci levels
 */

// Suppress deprecation warnings that would break JSON output
error_reporting(E_ALL & ~E_DEPRECATED);

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Content-Type');

// Fibonacci ratios
$POSITIVE_LEVELS = [
    ['ratio' => 0, 'label' => '0.0'],
    ['ratio' => 0.5, 'label' => '0.5'],
    ['ratio' => 1, 'label' => '1.0'],
    ['ratio' => 1.382, 'label' => '1.382'],
    ['ratio' => 1.618, 'label' => '1.618'],
    ['ratio' => 2, 'label' => '2.0'],
    ['ratio' => 2.382, 'label' => '2.382'],
    ['ratio' => 2.618, 'label' => '2.618'],
    ['ratio' => 3, 'label' => '3.0'],
    ['ratio' => 3.382, 'label' => '3.382'],
    ['ratio' => 3.618, 'label' => '3.618'],
    ['ratio' => 4.24, 'label' => '4.24'],
    ['ratio' => 5.08, 'label' => '5.08'],
    ['ratio' => 6.86, 'label' => '6.86'],
    ['ratio' => 11.01, 'label' => '11.01']
];

$NEGATIVE_LEVELS = [
    ['ratio' => -11.01, 'label' => '-11.01'],
    ['ratio' => -6.86, 'label' => '-6.86'],
    ['ratio' => -5.08, 'label' => '-5.08'],
    ['ratio' => -4.24, 'label' => '-4.24'],
    ['ratio' => -3.618, 'label' => '-3.618'],
    ['ratio' => -3.382, 'label' => '-3.382'],
    ['ratio' => -3, 'label' => '-3.0'],
    ['ratio' => -2.618, 'label' => '-2.618'],
    ['ratio' => -2.382, 'label' => '-2.382'],
    ['ratio' => -2, 'label' => '-2.0'],
    ['ratio' => -1.618, 'label' => '-1.618'],
    ['ratio' => -1.382, 'label' => '-1.382'],
    ['ratio' => -1, 'label' => '-1.0'],
    ['ratio' => -0.5, 'label' => '-0.5']
];

$action = $_GET['action'] ?? 'calculate';
$symbol = strtoupper($_GET['symbol'] ?? 'AAPL');
$precision = intval($_GET['precision'] ?? 2);

if ($precision < 2) $precision = 2;
if ($precision > 5) $precision = 5;

switch ($action) {
    case 'calculate':
        calculateFibonacci($symbol, $precision);
        break;
    case 'quote':
        fetchYahooData($symbol);
        break;
    default:
        echo json_encode(['success' => false, 'message' => 'Invalid action']);
}

function fetchYahooData($symbol) {
    // Fetch YTD data from Yahoo Finance
    $currentYear = date('Y');
    $startOfYear = strtotime("$currentYear-01-01");
    $now = time();
    
    // Use 1d interval for YTD data
    $url = "https://query1.finance.yahoo.com/v8/finance/chart/{$symbol}?period1={$startOfYear}&period2={$now}&interval=1d";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 15);
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36');
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    // Note: curl_close() is deprecated in PHP 8.5+, resource is auto-released
    
    if ($httpCode !== 200 || empty($response)) {
        return null;
    }
    
    return json_decode($response, true);
}

function calculateFibonacci($symbol, $precision) {
    global $POSITIVE_LEVELS, $NEGATIVE_LEVELS;
    
    $data = fetchYahooData($symbol);
    
    if (!$data || !isset($data['chart']['result'][0])) {
        echo json_encode([
            'success' => false,
            'message' => 'Unable to fetch market data for ' . $symbol
        ]);
        return;
    }
    
    $result = $data['chart']['result'][0];
    $meta = $result['meta'] ?? [];
    $timestamps = $result['timestamp'] ?? [];
    $quote = $result['indicators']['quote'][0] ?? [];
    
    if (empty($timestamps) || empty($quote)) {
        echo json_encode([
            'success' => false,
            'message' => 'No data available for ' . $symbol
        ]);
        return;
    }
    
    // Find first trading day of the year
    $currentYear = date('Y');
    $firstCandle = null;
    $firstCandleIndex = -1;
    
    for ($i = 0; $i < count($timestamps); $i++) {
        $date = new DateTime("@{$timestamps[$i]}");
        $date->setTimezone(new DateTimeZone('America/New_York'));
        
        if ($date->format('Y') == $currentYear) {
            if ($quote['open'][$i] !== null && $quote['high'][$i] !== null && 
                $quote['low'][$i] !== null && $quote['close'][$i] !== null) {
                $firstCandle = [
                    'timestamp' => $timestamps[$i],
                    'date' => $date->format('Y-m-d'),
                    'open' => $quote['open'][$i],
                    'high' => $quote['high'][$i],
                    'low' => $quote['low'][$i],
                    'close' => $quote['close'][$i]
                ];
                $firstCandleIndex = $i;
                break;
            }
        }
    }
    
    if (!$firstCandle) {
        echo json_encode([
            'success' => false,
            'message' => 'No data available for current year'
        ]);
        return;
    }
    
    // Determine if candle is bullish or bearish
    $isBullish = $firstCandle['close'] > $firstCandle['open'];
    $candleType = $isBullish ? 'BULLISH' : 'BEARISH';
    
    // For BULLISH candle: 0 = Low, 1 = High
    // For BEARISH candle: 0 = High, 1 = Low
    if ($isBullish) {
        $fibHigh = $firstCandle['high'];
        $fibLow = $firstCandle['low'];
    } else {
        $fibHigh = $firstCandle['low'];
        $fibLow = $firstCandle['high'];
    }
    
    $range = $fibHigh - $fibLow;
    
    // Get current price
    $closes = array_filter($quote['close'], function($c) { return $c !== null; });
    $currentPrice = end($closes);
    
    // Get period high/low
    $highs = array_filter($quote['high'], function($h) { return $h !== null; });
    $lows = array_filter($quote['low'], function($l) { return $l !== null; });
    $periodHigh = max($highs);
    $periodLow = min($lows);
    
    // Calculate Fibonacci levels
    $positiveLevels = [];
    foreach ($POSITIVE_LEVELS as $level) {
        $price = $fibLow + ($range * $level['ratio']);
        $positiveLevels[] = [
            'label' => $level['label'],
            'ratio' => $level['ratio'],
            'price' => round($price, $precision)
        ];
    }
    
    $negativeLevels = [];
    foreach ($NEGATIVE_LEVELS as $level) {
        $price = $fibLow + ($range * $level['ratio']);
        $negativeLevels[] = [
            'label' => $level['label'],
            'ratio' => $level['ratio'],
            'price' => round($price, $precision)
        ];
    }
    
    // Prepare candle data for chart
    $candles = [];
    for ($i = 0; $i < count($timestamps); $i++) {
        if ($quote['open'][$i] !== null && $quote['high'][$i] !== null && 
            $quote['low'][$i] !== null && $quote['close'][$i] !== null) {
            $candles[] = [
                'time' => $timestamps[$i],
                'open' => round($quote['open'][$i], 2),
                'high' => round($quote['high'][$i], 2),
                'low' => round($quote['low'][$i], 2),
                'close' => round($quote['close'][$i], 2),
                'volume' => $quote['volume'][$i] ?? 0
            ];
        }
    }
    
    echo json_encode([
        'success' => true,
        'symbol' => $symbol,
        'currency' => $meta['currency'] ?? 'USD',
        'currentPrice' => round($currentPrice, $precision),
        'periodHigh' => round($periodHigh, $precision),
        'periodLow' => round($periodLow, $precision),
        'range' => round($periodHigh - $periodLow, $precision),
        'rangePercent' => round((($periodHigh - $periodLow) / $periodLow) * 100, 2),
        'firstCandle' => [
            'date' => $firstCandle['date'],
            'type' => $candleType,
            'open' => round($firstCandle['open'], $precision),
            'high' => round($firstCandle['high'], $precision),
            'low' => round($firstCandle['low'], $precision),
            'close' => round($firstCandle['close'], $precision)
        ],
        'fibonacci' => [
            'high' => round($fibHigh, $precision),
            'low' => round($fibLow, $precision),
            'range' => round($range, $precision),
            'anchorNote' => $isBullish ? '0=Low, 1=High' : '0=High, 1=Low'
        ],
        'positiveLevels' => $positiveLevels,
        'negativeLevels' => $negativeLevels,
        'candles' => $candles,
        'precision' => $precision
    ]);
}
?>








