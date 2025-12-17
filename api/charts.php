<?php
// Suppress deprecation warnings that would break JSON output
error_reporting(E_ALL & ~E_DEPRECATED);

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE');
header('Access-Control-Allow-Headers: Content-Type');

require_once '../includes/config.php';
require_once '../includes/database.php';
require_once 'track_api_usage.php';

$database = new Database();
$conn = $database->getConnection();

$method = $_SERVER['REQUEST_METHOD'];
$action = $_GET['action'] ?? '';

// Cache directory for chart data
$cacheDir = __DIR__ . '/../data/chart_cache';
if (!is_dir($cacheDir)) {
    mkdir($cacheDir, 0755, true);
}

/**
 * Convert Unix timestamp to EST timezone
 * Returns timestamp adjusted to EST/EDT (handles DST automatically)
 * Note: This converts the timestamp representation to EST, but keeps it as milliseconds
 */
function convertToEST($timestampMs) {
    if (!$timestampMs) return $timestampMs;
    
    // Convert milliseconds to seconds for DateTime
    $timestampSeconds = $timestampMs / 1000;
    
    // Create DateTime object from UTC timestamp
    $utcDate = new DateTime('@' . $timestampSeconds, new DateTimeZone('UTC'));
    
    // Convert to EST/EDT timezone
    $estDate = clone $utcDate;
    $estDate->setTimezone(new DateTimeZone('America/New_York'));
    
    // Get the offset in seconds
    $offset = $estDate->getOffset();
    
    // Adjust the timestamp by the offset (convert to milliseconds)
    $estTimestampMs = $timestampMs + ($offset * 1000);
    
    return $estTimestampMs;
}

switch ($method) {
    case 'GET':
        switch ($action) {
            case 'quote':
                getQuote();
                break;
            case 'chart':
                getChartData();
                break;
            case 'studies':
                getStudies();
                break;
            case 'study':
                getStudy();
                break;
            case 'watchlist':
                getWatchlist();
                break;
            default:
                echo json_encode(['success' => false, 'message' => 'Invalid action']);
        }
        break;
    case 'POST':
        switch ($action) {
            case 'study':
                saveStudy();
                break;
            case 'watchlist':
                addToWatchlist();
                break;
            default:
                echo json_encode(['success' => false, 'message' => 'Invalid action']);
        }
        break;
    case 'DELETE':
        switch ($action) {
            case 'study':
                deleteStudy();
                break;
            case 'watchlist':
                removeFromWatchlist();
                break;
            default:
                echo json_encode(['success' => false, 'message' => 'Invalid action']);
        }
        break;
    default:
        echo json_encode(['success' => false, 'message' => 'Method not allowed']);
}

function getQuote() {
    global $cacheDir;
    $symbol = strtoupper($_GET['symbol'] ?? '');
    
    if (empty($symbol)) {
        echo json_encode(['success' => false, 'message' => 'Symbol is required']);
        return;
    }

    // Check cache (1 minute cache for quotes - more frequent updates)
    $cacheFile = $cacheDir . '/quote_' . $symbol . '.json';
    if (file_exists($cacheFile) && (time() - filemtime($cacheFile)) < 60) {
        echo file_get_contents($cacheFile);
        return;
    }

    // Try Finnhub first
    $data = fetchFinnhubQuote($symbol);
    
    if ($data && isset($data['c']) && $data['c'] > 0) {
        $result = [
            'success' => true,
            'symbol' => $symbol,
            'current' => $data['c'],
            'high' => $data['h'],
            'low' => $data['l'],
            'open' => $data['o'],
            'previousClose' => $data['pc'],
            'change' => $data['d'] ?? ($data['c'] - $data['pc']),
            'changePercent' => $data['dp'] ?? (($data['c'] - $data['pc']) / $data['pc'] * 100),
            'timestamp' => $data['t'] ?? time(),
            'source' => 'finnhub'
        ];
        file_put_contents($cacheFile, json_encode($result));
        echo json_encode($result);
        return;
    }
    
    // Try Yahoo Finance as backup
    $yahooData = fetchYahooQuote($symbol);
    if ($yahooData && $yahooData['success']) {
        $yahooData['source'] = 'yahoo';
        file_put_contents($cacheFile, json_encode($yahooData));
        echo json_encode($yahooData);
        return;
    }
    
    // Return error - no demo data
    echo json_encode([
        'success' => false, 
        'message' => 'Unable to fetch real-time quote data for ' . $symbol,
        'symbol' => $symbol
    ]);
}

function getChartData() {
    global $cacheDir;
    $symbol = strtoupper($_GET['symbol'] ?? '');
    $timeframe = $_GET['timeframe'] ?? '1D';
    
    if (empty($symbol)) {
        echo json_encode(['success' => false, 'message' => 'Symbol is required']);
        return;
    }

    // Determine resolution and date range based on timeframe
    $params = getTimeframeParams($timeframe);
    
    // Check for cache bypass parameter for real-time data
    $forceRefresh = isset($_GET['refresh']) && $_GET['refresh'] === 'true';
    
    // Check cache (only if not forcing refresh)
    $cacheKey = $symbol . '_' . $timeframe;
    $cacheFile = $cacheDir . '/chart_' . $cacheKey . '.json';
    // Reduced cache time for real-time data: 30 seconds for intraday, 2 minutes for others
    $cacheTime = in_array($timeframe, ['15MIN', '1H', '4H', '1D']) ? 30 : 120;
    
    if (!$forceRefresh && file_exists($cacheFile) && (time() - filemtime($cacheFile)) < $cacheTime) {
        $cachedData = file_get_contents($cacheFile);
        $decoded = json_decode($cachedData, true);
        // Only use cache if it's real data (not demo)
        if ($decoded && !isset($decoded['demo']) && isset($decoded['success']) && $decoded['success'] === true) {
            echo $cachedData;
            return;
        }
    }

    // Try Finnhub first
    $data = fetchFinnhubCandles($symbol, $params['resolution'], $params['from'], $params['to']);
    
    if ($data && isset($data['c']) && is_array($data['c']) && count($data['c']) > 0) {
        $chartData = formatCandleData($data, $symbol, $timeframe);
        $chartData['source'] = 'finnhub';
        file_put_contents($cacheFile, json_encode($chartData));
        echo json_encode($chartData);
        return;
    }
    
    // Try Yahoo Finance as backup
    $yahooData = fetchYahooChartData($symbol, $timeframe);
    if ($yahooData && isset($yahooData['candles']) && count($yahooData['candles']) > 0) {
        $yahooData['source'] = 'yahoo';
        file_put_contents($cacheFile, json_encode($yahooData));
        echo json_encode($yahooData);
        return;
    }
    
    // Return error - NO demo data
    echo json_encode([
        'success' => false, 
        'message' => 'Unable to fetch real-time market data. Please check your API configuration or try again later.',
        'symbol' => $symbol,
        'timeframe' => $timeframe
    ]);
}

function getTimeframeParams($timeframe) {
    $now = time();
    $resolution = '60'; // Default to 1 hour
    $from = $now - 86400; // Default to 1 day
    
    // Only allow 15MIN, 1H, 4H, and 1D timeframes
    switch ($timeframe) {
        case '15MIN':
            $resolution = '15'; // 15-minute bars
            $from = $now - (5 * 86400); // Last 5 days for 15 min bars (to get enough data)
            break;
        case '1H':
            $resolution = '60'; // 1-hour bars
            $from = $now - (30 * 86400); // Last 30 days for 1 hour bars
            break;
        case '4H':
            $resolution = '60'; // Use 1-hour bars (Finnhub doesn't support 4H directly, will show hourly bars)
            $from = $now - (60 * 86400); // Last 60 days for 4 hour view (showing hourly bars)
            break;
        case '1D':
            $resolution = 'D'; // Daily bars
            $from = $now - (10 * 86400); // Last 10 days to ensure at least 5 trading days (accounting for weekends/holidays)
            break;
        default:
            // Default to 1D if invalid timeframe
            $resolution = 'D'; // Daily bars
            $from = $now - (10 * 86400); // Last 10 days to ensure at least 5 trading days (accounting for weekends/holidays)
            break;
    }
    
    return [
        'resolution' => $resolution,
        'from' => $from,
        'to' => $now
    ];
}

function fetchFinnhubQuote($symbol) {
    $startTime = microtime(true);
    $apiKey = FINNHUB_API_KEY;
    $url = "https://finnhub.io/api/v1/quote?symbol={$symbol}&token={$apiKey}";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $endTime = microtime(true);
    $responseTime = round(($endTime - $startTime) * 1000);
    curl_close($ch);
    
    $success = ($httpCode === 200 && !$error);
    
    // Track API usage (non-blocking)
    try {
        trackApiUsage('finnhub', 'Finnhub', $success, $responseTime, $success ? null : "HTTP $httpCode: $error");
    } catch (Exception $e) {
        error_log("Failed to track API usage: " . $e->getMessage());
    }
    
    if (!$success) {
        error_log("Finnhub quote error: HTTP $httpCode, Error: $error");
        return null;
    }
    
    $data = json_decode($response, true);
    
    // Check if we got valid data (Finnhub returns all zeros for invalid symbols)
    if (!$data || (isset($data['c']) && $data['c'] == 0)) {
        try {
            trackApiUsage('finnhub', 'Finnhub', false, $responseTime, 'Invalid or empty data');
        } catch (Exception $e) {
            error_log("Failed to track API usage: " . $e->getMessage());
        }
        return null;
    }
    
    return $data;
}

function fetchFinnhubCandles($symbol, $resolution, $from, $to) {
    $startTime = microtime(true);
    $apiKey = FINNHUB_API_KEY;
    $url = "https://finnhub.io/api/v1/stock/candle?symbol={$symbol}&resolution={$resolution}&from={$from}&to={$to}&token={$apiKey}";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 15);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $endTime = microtime(true);
    $responseTime = round(($endTime - $startTime) * 1000);
    curl_close($ch);
    
    $success = ($httpCode === 200 && !$error);
    
    // Track API usage (non-blocking)
    try {
        trackApiUsage('finnhub', 'Finnhub', $success, $responseTime, $success ? null : "HTTP $httpCode: $error");
    } catch (Exception $e) {
        error_log("Failed to track API usage: " . $e->getMessage());
    }
    
    if (!$success) {
        error_log("Finnhub candles error: HTTP $httpCode, Error: $error, URL: $url");
        return null;
    }
    
    $data = json_decode($response, true);
    
    // Check for valid response (Finnhub returns 's' => 'no_data' when no data available)
    if (!$data || (isset($data['s']) && $data['s'] === 'no_data')) {
        try {
            trackApiUsage('finnhub', 'Finnhub', false, $responseTime, 'No data available');
        } catch (Exception $e) {
            error_log("Failed to track API usage: " . $e->getMessage());
        }
        error_log("Finnhub candles: No data available for $symbol");
        return null;
    }
    
    return $data;
}

function fetchYahooQuote($symbol) {
    $startTime = microtime(true);
    $url = "https://query1.finance.yahoo.com/v8/finance/chart/{$symbol}?interval=1d&range=1d";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36');
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $endTime = microtime(true);
    $responseTime = round(($endTime - $startTime) * 1000);
    curl_close($ch);
    
    $data = json_decode($response, true);
    $success = ($httpCode === 200 && isset($data['chart']['result'][0]['meta']));
    
    // Track API usage (non-blocking)
    try {
        trackApiUsage('yahoo', 'Yahoo Finance', $success, $responseTime, $success ? null : "HTTP $httpCode: $error");
    } catch (Exception $e) {
        error_log("Failed to track API usage: " . $e->getMessage());
    }
    
    if ($success) {
        $meta = $data['chart']['result'][0]['meta'];
        return [
            'success' => true,
            'symbol' => $symbol,
            'current' => $meta['regularMarketPrice'] ?? 0,
            'high' => $meta['regularMarketDayHigh'] ?? 0,
            'low' => $meta['regularMarketDayLow'] ?? 0,
            'open' => $meta['regularMarketOpen'] ?? 0,
            'previousClose' => $meta['previousClose'] ?? 0,
            'change' => ($meta['regularMarketPrice'] ?? 0) - ($meta['previousClose'] ?? 0),
            'changePercent' => (($meta['regularMarketPrice'] ?? 0) - ($meta['previousClose'] ?? 0)) / ($meta['previousClose'] ?? 1) * 100,
            'timestamp' => time()
        ];
    }
    
    return null;
}

function fetchYahooChartData($symbol, $timeframe) {
    $startTime = microtime(true);
    // Map timeframe to Yahoo Finance parameters
    $yahooParams = getYahooTimeframeParams($timeframe);
    
    $url = "https://query1.finance.yahoo.com/v8/finance/chart/{$symbol}?interval={$yahooParams['interval']}&range={$yahooParams['range']}";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 15);
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36');
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $endTime = microtime(true);
    $responseTime = round(($endTime - $startTime) * 1000);
    curl_close($ch);
    
    $success = ($httpCode === 200 && !empty($response));
    
    // Track API usage (non-blocking)
    try {
        trackApiUsage('yahoo', 'Yahoo Finance', $success, $responseTime, $success ? null : "HTTP $httpCode: $error");
    } catch (Exception $e) {
        error_log("Failed to track API usage: " . $e->getMessage());
    }
    
    if (!$success) {
        return null;
    }
    
    $data = json_decode($response, true);
    
    if (!isset($data['chart']['result'][0])) {
        return null;
    }
    
    $result = $data['chart']['result'][0];
    $meta = $result['meta'] ?? [];
    $timestamps = $result['timestamp'] ?? [];
    $quote = $result['indicators']['quote'][0] ?? [];
    
    if (empty($timestamps) || empty($quote)) {
        return null;
    }
    
    $candles = [];
    $count = count($timestamps);
    
    for ($i = 0; $i < $count; $i++) {
        // Skip null values
        if (!isset($quote['open'][$i]) || $quote['open'][$i] === null) {
            continue;
        }
        
        $originalTime = $timestamps[$i] * 1000; // Convert to milliseconds
        $estTime = convertToEST($originalTime);
        
        $candles[] = [
            'time' => $estTime, // Convert to EST
            'open' => round($quote['open'][$i], 2),
            'high' => round($quote['high'][$i], 2),
            'low' => round($quote['low'][$i], 2),
            'close' => round($quote['close'][$i], 2),
            'volume' => $quote['volume'][$i] ?? 0
        ];
    }
    
    if (empty($candles)) {
        return null;
    }
    
    // Calculate statistics
    $closes = array_column($candles, 'close');
    $volumes = array_column($candles, 'volume');
    $highs = array_column($candles, 'high');
    $lows = array_column($candles, 'low');
    
    // Calculate indicators
    $sma20 = calculateSMA($closes, 20);
    $sma50 = calculateSMA($closes, 50);
    $ema12 = calculateEMA($closes, 12);
    $ema26 = calculateEMA($closes, 26);
    $rsi = calculateRSI($closes, 14);
    $macd = calculateMACD($closes);
    $bollinger = calculateBollingerBands($closes, 20);
    $fibonacci = calculateFibonacciLevels($highs, $lows, $closes);
    
    return [
        'success' => true,
        'symbol' => $symbol,
        'timeframe' => $timeframe,
        'candles' => $candles,
        'statistics' => [
            'open' => $candles[0]['open'],
            'high' => max($highs),
            'low' => min($lows),
            'close' => end($closes),
            'volume' => end($volumes),
            'avgVolume' => count($volumes) > 0 ? array_sum($volumes) / count($volumes) : 0,
            'high52w' => max($highs),
            'low52w' => min($lows)
        ],
        'indicators' => [
            'sma20' => $sma20,
            'sma50' => $sma50,
            'ema12' => $ema12,
            'ema26' => $ema26,
            'rsi' => $rsi,
            'macd' => $macd,
            'bollinger' => $bollinger,
            'fibonacci' => $fibonacci
        ]
    ];
}

function getYahooTimeframeParams($timeframe) {
    // Only allow 15MIN, 1H, 4H, and 1D timeframes
    switch ($timeframe) {
        case '15MIN':
            return ['interval' => '1m', 'range' => '1d'];
        case '1H':
            return ['interval' => '5m', 'range' => '5d'];
        case '4H':
            return ['interval' => '15m', 'range' => '1mo'];
        case '1D':
            return ['interval' => '1d', 'range' => '10d']; // Daily bars, 10 days range to ensure at least 5 trading days
        default:
            // Default to 1D if invalid timeframe
            return ['interval' => '1d', 'range' => '10d']; // Daily bars, 10 days range to ensure at least 5 trading days
    }
}

function formatCandleData($data, $symbol, $timeframe) {
    $candles = [];
    $count = count($data['c']);
    
    for ($i = 0; $i < $count; $i++) {
        $originalTime = $data['t'][$i] * 1000; // Convert to milliseconds
        $estTime = convertToEST($originalTime);
        
        $candles[] = [
            'time' => $estTime, // Convert to EST
            'open' => $data['o'][$i],
            'high' => $data['h'][$i],
            'low' => $data['l'][$i],
            'close' => $data['c'][$i],
            'volume' => $data['v'][$i]
        ];
    }
    
    // Calculate statistics
    $closes = $data['c'];
    $volumes = $data['v'];
    $highs = $data['h'];
    $lows = $data['l'];
    
    // Calculate moving averages
    $sma20 = calculateSMA($closes, 20);
    $sma50 = calculateSMA($closes, 50);
    $ema12 = calculateEMA($closes, 12);
    $ema26 = calculateEMA($closes, 26);
    
    // Calculate RSI
    $rsi = calculateRSI($closes, 14);
    
    // Calculate MACD
    $macd = calculateMACD($closes);
    
    // Calculate Bollinger Bands
    $bollinger = calculateBollingerBands($closes, 20);
    
    // Calculate Fibonacci levels
    $fibonacci = calculateFibonacciLevels($highs, $lows, $closes);
    
    // 52-week high/low (if we have enough data)
    $high52w = max($highs);
    $low52w = min($lows);
    
    return [
        'success' => true,
        'symbol' => $symbol,
        'timeframe' => $timeframe,
        'candles' => $candles,
        'statistics' => [
            'open' => $data['o'][0],
            'high' => max($highs),
            'low' => min($lows),
            'close' => end($closes),
            'volume' => end($volumes),
            'avgVolume' => array_sum($volumes) / count($volumes),
            'high52w' => $high52w,
            'low52w' => $low52w
        ],
        'indicators' => [
            'sma20' => $sma20,
            'sma50' => $sma50,
            'ema12' => $ema12,
            'ema26' => $ema26,
            'rsi' => $rsi,
            'macd' => $macd,
            'bollinger' => $bollinger,
            'fibonacci' => $fibonacci
        ]
    ];
}

function calculateSMA($data, $period) {
    $count = count($data);
    if ($count < $period) return null;
    
    $smaValues = [];
    for ($i = $period - 1; $i < $count; $i++) {
        $sum = 0;
        for ($j = 0; $j < $period; $j++) {
            $sum += $data[$i - $j];
        }
        $smaValues[] = $sum / $period;
    }
    
    return [
        'values' => $smaValues,
        'current' => end($smaValues)
    ];
}

function calculateEMA($data, $period) {
    $count = count($data);
    if ($count < $period) return null;
    
    $multiplier = 2 / ($period + 1);
    $emaValues = [];
    
    // Start with SMA
    $sum = 0;
    for ($i = 0; $i < $period; $i++) {
        $sum += $data[$i];
    }
    $emaValues[] = $sum / $period;
    
    // Calculate EMA
    for ($i = $period; $i < $count; $i++) {
        $ema = ($data[$i] - end($emaValues)) * $multiplier + end($emaValues);
        $emaValues[] = $ema;
    }
    
    return [
        'values' => $emaValues,
        'current' => end($emaValues)
    ];
}

function calculateRSI($data, $period = 14) {
    $count = count($data);
    if ($count < $period + 1) return ['value' => 50, 'signal' => 'Neutral'];
    
    $gains = [];
    $losses = [];
    
    for ($i = 1; $i < $count; $i++) {
        $change = $data[$i] - $data[$i - 1];
        $gains[] = $change > 0 ? $change : 0;
        $losses[] = $change < 0 ? abs($change) : 0;
    }
    
    $avgGain = array_sum(array_slice($gains, -$period)) / $period;
    $avgLoss = array_sum(array_slice($losses, -$period)) / $period;
    
    if ($avgLoss == 0) {
        $rsi = 100;
    } else {
        $rs = $avgGain / $avgLoss;
        $rsi = 100 - (100 / (1 + $rs));
    }
    
    $signal = 'Neutral';
    if ($rsi > 70) $signal = 'Overbought';
    else if ($rsi < 30) $signal = 'Oversold';
    
    return [
        'value' => round($rsi, 2),
        'signal' => $signal
    ];
}

function calculateMACD($data) {
    $ema12 = calculateEMA($data, 12);
    $ema26 = calculateEMA($data, 26);
    
    if (!$ema12 || !$ema26) {
        return ['value' => 0, 'signal' => 'Neutral'];
    }
    
    $macdLine = $ema12['current'] - $ema26['current'];
    
    $signal = 'Neutral';
    if ($macdLine > 0) $signal = 'Bullish';
    else if ($macdLine < 0) $signal = 'Bearish';
    
    return [
        'value' => round($macdLine, 4),
        'signal' => $signal
    ];
}

function calculateBollingerBands($data, $period = 20) {
    $count = count($data);
    if ($count < $period) return null;
    
    $slice = array_slice($data, -$period);
    $sma = array_sum($slice) / $period;
    
    $variance = 0;
    foreach ($slice as $value) {
        $variance += pow($value - $sma, 2);
    }
    $stdDev = sqrt($variance / $period);
    
    return [
        'upper' => round($sma + (2 * $stdDev), 2),
        'middle' => round($sma, 2),
        'lower' => round($sma - (2 * $stdDev), 2)
    ];
}

function calculateFibonacciLevels($highs, $lows, $closes) {
    if (empty($highs) || empty($lows)) return null;
    
    $high = max($highs);
    $low = min($lows);
    $diff = $high - $low;
    $currentPrice = end($closes);
    
    // Fibonacci retracement levels (from high to low)
    $levels = [
        '0' => $high,                           // 0% - High
        '236' => $high - ($diff * 0.236),       // 23.6%
        '382' => $high - ($diff * 0.382),       // 38.2%
        '50' => $high - ($diff * 0.5),          // 50%
        '618' => $high - ($diff * 0.618),       // 61.8% (Golden Ratio)
        '786' => $high - ($diff * 0.786),       // 78.6%
        '100' => $low                            // 100% - Low
    ];
    
    // Fibonacci extension levels for price targets
    $extensions = [
        '1618' => $high + ($diff * 0.618),      // 161.8% extension
        '2618' => $high + ($diff * 1.618),      // 261.8% extension
        '4236' => $high + ($diff * 3.236)       // 423.6% extension
    ];
    
    // Fibonacci spiral price targets based on current price
    $spiralTargets = [
        'target1' => round($currentPrice * 1.618, 2),   // Golden ratio up
        'target2' => round($currentPrice * 2.618, 2),   // 2.618x up
        'support1' => round($currentPrice * 0.618, 2),  // Golden ratio down
        'support2' => round($currentPrice * 0.382, 2)   // 0.382x down
    ];
    
    // Determine current position relative to Fibonacci levels
    $position = 'neutral';
    if ($currentPrice > $levels['382']) {
        $position = 'above_382';
    } else if ($currentPrice > $levels['618']) {
        $position = 'between_382_618';
    } else {
        $position = 'below_618';
    }
    
    return [
        'levels' => array_map(function($v) { return round($v, 2); }, $levels),
        'extensions' => array_map(function($v) { return round($v, 2); }, $extensions),
        'spiralTargets' => $spiralTargets,
        'high' => round($high, 2),
        'low' => round($low, 2),
        'range' => round($diff, 2),
        'currentPosition' => $position
    ];
}

function generateDemoData($symbol, $timeframe) {
    $candles = [];
    $basePrice = 100 + (ord($symbol[0]) % 400);
    $now = time() * 1000;
    
    $intervals = [
        '1D' => ['count' => 78, 'interval' => 5 * 60 * 1000],
        '5D' => ['count' => 100, 'interval' => 15 * 60 * 1000],
        '1M' => ['count' => 30, 'interval' => 24 * 60 * 60 * 1000],
        '3M' => ['count' => 90, 'interval' => 24 * 60 * 60 * 1000],
        '6M' => ['count' => 180, 'interval' => 24 * 60 * 60 * 1000],
        '1Y' => ['count' => 252, 'interval' => 24 * 60 * 60 * 1000],
        '5Y' => ['count' => 260, 'interval' => 7 * 24 * 60 * 60 * 1000]
    ];
    
    $config = $intervals[$timeframe] ?? $intervals['1D'];
    $price = $basePrice;
    
    for ($i = $config['count']; $i >= 0; $i--) {
        $volatility = $basePrice * 0.02;
        $change = (mt_rand(-100, 100) / 100) * $volatility;
        $open = $price;
        $close = $price + $change;
        $high = max($open, $close) + abs($change) * 0.5;
        $low = min($open, $close) - abs($change) * 0.5;
        $volume = mt_rand(1000000, 10000000);
        
        $candles[] = [
            'time' => $now - ($i * $config['interval']),
            'open' => round($open, 2),
            'high' => round($high, 2),
            'low' => round($low, 2),
            'close' => round($close, 2),
            'volume' => $volume
        ];
        
        $price = $close;
    }
    
    $closes = array_column($candles, 'close');
    $volumes = array_column($candles, 'volume');
    $highs = array_column($candles, 'high');
    $lows = array_column($candles, 'low');
    
    return [
        'success' => true,
        'symbol' => $symbol,
        'timeframe' => $timeframe,
        'demo' => true,
        'candles' => $candles,
        'statistics' => [
            'open' => $candles[0]['open'],
            'high' => max($highs),
            'low' => min($lows),
            'close' => end($closes),
            'volume' => end($volumes),
            'avgVolume' => array_sum($volumes) / count($volumes),
            'high52w' => max($highs) * 1.1,
            'low52w' => min($lows) * 0.9
        ],
        'indicators' => [
            'sma20' => calculateSMA($closes, 20),
            'sma50' => calculateSMA($closes, 50),
            'ema12' => calculateEMA($closes, 12),
            'ema26' => calculateEMA($closes, 26),
            'rsi' => calculateRSI($closes, 14),
            'macd' => calculateMACD($closes),
            'bollinger' => calculateBollingerBands($closes, 20),
            'fibonacci' => calculateFibonacciLevels($highs, $lows, $closes)
        ]
    ];
}

function getStudies() {
    global $conn;
    
    try {
        $stmt = $conn->query("SELECT * FROM chart_studies ORDER BY updated_at DESC");
        $studies = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        echo json_encode([
            'success' => true,
            'studies' => $studies
        ]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Database error']);
    }
}

function getStudy() {
    global $conn;
    $id = $_GET['id'] ?? 0;
    
    try {
        $stmt = $conn->prepare("SELECT * FROM chart_studies WHERE id = ?");
        $stmt->execute([$id]);
        $study = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($study) {
            $study['indicators'] = json_decode($study['indicators'], true);
            $study['price_data'] = json_decode($study['price_data'], true);
            echo json_encode(['success' => true, 'study' => $study]);
        } else {
            echo json_encode(['success' => false, 'message' => 'Study not found']);
        }
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Database error']);
    }
}

function saveStudy() {
    global $conn;
    $data = json_decode(file_get_contents('php://input'), true);
    
    if (!$data || empty($data['name']) || empty($data['symbol'])) {
        echo json_encode(['success' => false, 'message' => 'Name and symbol are required']);
        return;
    }
    
    try {
        $stmt = $conn->prepare("
            INSERT INTO chart_studies (name, symbol, timeframe, chart_type, indicators, notes, price_data)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ");
        
        $stmt->execute([
            $data['name'],
            $data['symbol'],
            $data['timeframe'] ?? '1D',
            $data['chartType'] ?? 'candlestick',
            json_encode($data['indicators'] ?? []),
            $data['notes'] ?? '',
            json_encode($data['priceData'] ?? [])
        ]);
        
        echo json_encode([
            'success' => true,
            'message' => 'Study saved successfully',
            'id' => $conn->lastInsertId()
        ]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to save study']);
    }
}

function deleteStudy() {
    global $conn;
    $id = $_GET['id'] ?? 0;
    
    try {
        $stmt = $conn->prepare("DELETE FROM chart_studies WHERE id = ?");
        $stmt->execute([$id]);
        
        echo json_encode(['success' => true, 'message' => 'Study deleted']);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to delete study']);
    }
}

function getWatchlist() {
    global $conn;
    
    try {
        $stmt = $conn->query("SELECT * FROM watchlist ORDER BY added_at DESC");
        $items = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        echo json_encode(['success' => true, 'watchlist' => $items]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Database error']);
    }
}

function addToWatchlist() {
    global $conn;
    $data = json_decode(file_get_contents('php://input'), true);
    $symbol = strtoupper($data['symbol'] ?? '');
    
    if (empty($symbol)) {
        echo json_encode(['success' => false, 'message' => 'Symbol is required']);
        return;
    }
    
    try {
        $stmt = $conn->prepare("INSERT OR IGNORE INTO watchlist (symbol) VALUES (?)");
        $stmt->execute([$symbol]);
        
        echo json_encode(['success' => true, 'message' => 'Added to watchlist']);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to add to watchlist']);
    }
}

function removeFromWatchlist() {
    global $conn;
    $symbol = strtoupper($_GET['symbol'] ?? '');
    
    try {
        $stmt = $conn->prepare("DELETE FROM watchlist WHERE symbol = ?");
        $stmt->execute([$symbol]);
        
        echo json_encode(['success' => true, 'message' => 'Removed from watchlist']);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to remove']);
    }
}
?>

