<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';
require_once 'track_api_usage.php';

// Get API source from request (default to config)
$apiSource = $_GET['source'] ?? DEFAULT_API_SOURCE;

// Rate limiting tracker
$rateLimitFile = '../cache/rate_limit.json';

function checkRateLimit($apiSource) {
    global $rateLimitFile;
    
    if (!file_exists($rateLimitFile)) {
        return true;
    }
    
    $data = json_decode(file_get_contents($rateLimitFile), true);
    $currentMinute = floor(time() / 60);
    
    if (!isset($data[$apiSource]) || $data[$apiSource]['minute'] !== $currentMinute) {
        return true;
    }
    
    // Finnhub: 60 calls/minute, Yahoo: 100 calls/minute (more lenient)
    $limit = ($apiSource === 'finnhub') ? FINNHUB_RATE_LIMIT : 100;
    
    return $data[$apiSource]['count'] < $limit;
}

function updateRateLimit($apiSource) {
    global $rateLimitFile;
    
    $currentMinute = floor(time() / 60);
    $data = [];
    
    if (file_exists($rateLimitFile)) {
        $data = json_decode(file_get_contents($rateLimitFile), true) ?? [];
    }
    
    if (!isset($data[$apiSource]) || $data[$apiSource]['minute'] !== $currentMinute) {
        $data[$apiSource] = ['minute' => $currentMinute, 'count' => 0];
    }
    
    $data[$apiSource]['count']++;
    file_put_contents($rateLimitFile, json_encode($data));
}

// Initialize database connection
$db = new Database();
$conn = $db->getConnection();

// Default symbols
$symbols = ['SPY', 'QQQ', 'VIX', 'DXY'];

if ($conn) {
    try {
        $stmt = $conn->prepare("SELECT symbol FROM stock_symbols ORDER BY created_at DESC");
        $stmt->execute();
        $dbSymbols = $stmt->fetchAll(PDO::FETCH_COLUMN);
        if (!empty($dbSymbols)) {
            $symbols = $dbSymbols;
        }
    } catch (PDOException $e) {
        // Use default symbols if database query fails
    }
}

$marketData = [];
$apiUsed = $apiSource;
$errors = [];

foreach ($symbols as $symbol) {
    $data = getCachedData($symbol);
    
    if (!$data) {
        // Check rate limit before making API call
        if (!checkRateLimit($apiSource)) {
            // Try backup API if primary is rate limited
            if ($apiSource === 'finnhub' && checkRateLimit('yahoo')) {
                $apiUsed = 'yahoo';
                $data = fetchFromYahoo($symbol);
            } elseif ($apiSource === 'yahoo' && checkRateLimit('finnhub')) {
                $apiUsed = 'finnhub';
                $data = fetchFromFinnhub($symbol);
            } else {
                $errors[] = "Rate limit exceeded for all APIs";
                $data = getFallbackData($symbol);
            }
        } else {
            if ($apiSource === 'finnhub') {
                $data = fetchFromFinnhub($symbol);
                if (!$data || isset($data['error'])) {
                    // Fallback to Yahoo if Finnhub fails
                    $data = fetchFromYahoo($symbol);
                    $apiUsed = 'yahoo';
                }
            } else {
                $data = fetchFromYahoo($symbol);
                if (!$data || isset($data['error'])) {
                    // Fallback to Finnhub if Yahoo fails
                    $data = fetchFromFinnhub($symbol);
                    $apiUsed = 'finnhub';
                }
            }
        }
        
        if ($data && !isset($data['error'])) {
            cacheData($symbol, $data);
        }
    }
    
    if ($data && !isset($data['error'])) {
        $marketData[] = $data;
    } else {
        // Use fallback data if all APIs fail
        $marketData[] = getFallbackData($symbol);
    }
}

echo json_encode([
    'data' => $marketData,
    'source' => $apiUsed,
    'errors' => $errors,
    'timestamp' => date('Y-m-d H:i:s')
]);

// Fetch from Finnhub API
function fetchFromFinnhub($symbol) {
    $startTime = microtime(true);
    $url = FINNHUB_BASE_URL . "/quote?symbol=" . urlencode($symbol) . "&token=" . FINNHUB_API_KEY;
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $endTime = microtime(true);
    $responseTime = round(($endTime - $startTime) * 1000);
    curl_close($ch);
    
    updateRateLimit('finnhub');
    
    $success = ($httpCode === 200 && $response);
    $data = null;
    
    if ($success) {
        $data = json_decode($response, true);
        $success = isset($data['c']) && $data['c'] > 0;
    }
    
    // Track API usage (non-blocking)
    try {
        trackApiUsage('finnhub', 'Finnhub', $success, $responseTime, $success ? null : "HTTP $httpCode: $error");
    } catch (Exception $e) {
        error_log("Failed to track API usage: " . $e->getMessage());
    }
    
    if ($success && $data) {
        return [
            'symbol' => $symbol,
            'price' => floatval($data['c']),
            'change' => floatval($data['d'] ?? 0),
            'changePercent' => floatval($data['dp'] ?? 0),
            'high' => floatval($data['h'] ?? 0),
            'low' => floatval($data['l'] ?? 0),
            'open' => floatval($data['o'] ?? 0),
            'previousClose' => floatval($data['pc'] ?? 0),
            'volume' => 0, // Finnhub quote doesn't include volume
            'timestamp' => date('Y-m-d H:i:s'),
            'source' => 'finnhub'
        ];
    }
    
    return null;
}

// Fetch from Yahoo Finance API
function fetchFromYahoo($symbol) {
    $startTime = microtime(true);
    // Handle special symbols
    $yahooSymbol = $symbol;
    if ($symbol === 'VIX') {
        $yahooSymbol = '^VIX';
    } elseif ($symbol === 'DXY') {
        $yahooSymbol = 'DX-Y.NYB';
    }
    
    $url = YAHOO_FINANCE_BASE_URL . "/" . urlencode($yahooSymbol) . "?interval=1d&range=1d";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36');
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $endTime = microtime(true);
    $responseTime = round(($endTime - $startTime) * 1000);
    curl_close($ch);
    
    updateRateLimit('yahoo');
    
    $success = ($httpCode === 200 && $response);
    $data = null;
    
    if ($success) {
        $data = json_decode($response, true);
        $success = isset($data['chart']['result'][0]);
    }
    
    // Track API usage (non-blocking)
    try {
        trackApiUsage('yahoo', 'Yahoo Finance', $success, $responseTime, $success ? null : "HTTP $httpCode: $error");
    } catch (Exception $e) {
        error_log("Failed to track API usage: " . $e->getMessage());
    }
    
    if ($success && $data && isset($data['chart']['result'][0])) {
            $result = $data['chart']['result'][0];
            $meta = $result['meta'] ?? [];
            $quote = $result['indicators']['quote'][0] ?? [];
            
            $currentPrice = floatval($meta['regularMarketPrice'] ?? 0);
            $previousClose = floatval($meta['previousClose'] ?? $meta['chartPreviousClose'] ?? 0);
            $change = $currentPrice - $previousClose;
            $changePercent = $previousClose > 0 ? ($change / $previousClose) * 100 : 0;
            
            // Get the latest values from quote arrays
            $high = 0;
            $low = 0;
            $open = 0;
            $volume = 0;
            
            // Get day high (max of high array)
            if (!empty($quote['high']) && is_array($quote['high'])) {
                $highArray = array_filter($quote['high'], function($v) { return $v !== null && $v > 0; });
                $high = !empty($highArray) ? floatval(max($highArray)) : 0;
            }
            
            // Get day low (min of low array)
            if (!empty($quote['low']) && is_array($quote['low'])) {
                $lowArray = array_filter($quote['low'], function($v) { return $v !== null && $v > 0; });
                $low = !empty($lowArray) ? floatval(min($lowArray)) : 0;
            }
            
            // Get open (first non-null value)
            if (!empty($quote['open']) && is_array($quote['open'])) {
                $openArray = array_filter($quote['open'], function($v) { return $v !== null && $v > 0; });
                $open = !empty($openArray) ? floatval(reset($openArray)) : 0;
            }
            
            // Get volume (sum of all volumes for the day)
            if (!empty($quote['volume']) && is_array($quote['volume'])) {
                $volumeArray = array_filter($quote['volume'], function($v) { return $v !== null && $v > 0; });
                $volume = !empty($volumeArray) ? intval(array_sum($volumeArray)) : 0;
            }
            
            // Use current price as fallback if values are 0 or invalid
            $high = ($high > 0) ? $high : $currentPrice;
            $low = ($low > 0) ? $low : $currentPrice;
            $open = ($open > 0) ? $open : $currentPrice;
            
            return [
                'symbol' => $symbol,
                'price' => $currentPrice,
                'change' => round($change, 2),
                'changePercent' => round($changePercent, 2),
                'high' => $high,
                'low' => $low,
                'open' => $open,
                'previousClose' => $previousClose,
                'volume' => $volume,
                'timestamp' => date('Y-m-d H:i:s'),
                'source' => 'yahoo'
            ];
        }
    }
    
    return null;
}

// Get cached data
function getCachedData($symbol) {
    $cacheDir = '../cache';
    if (!is_dir($cacheDir)) {
        mkdir($cacheDir, 0777, true);
    }
    
    $cacheFile = $cacheDir . '/' . strtolower($symbol) . '.json';
    
    if (file_exists($cacheFile) && (time() - filemtime($cacheFile)) < CACHE_DURATION) {
        $cached = json_decode(file_get_contents($cacheFile), true);
        if ($cached) {
            return $cached;
        }
    }
    
    return null;
}

// Cache data
function cacheData($symbol, $data) {
    $cacheDir = '../cache';
    if (!is_dir($cacheDir)) {
        mkdir($cacheDir, 0777, true);
    }
    
    $cacheFile = $cacheDir . '/' . strtolower($symbol) . '.json';
    file_put_contents($cacheFile, json_encode($data));
}

// Fallback data when all APIs fail
function getFallbackData($symbol) {
    return [
        'symbol' => $symbol,
        'price' => 0,
        'change' => 0,
        'changePercent' => 0,
        'high' => 0,
        'low' => 0,
        'open' => 0,
        'previousClose' => 0,
        'volume' => 0,
        'timestamp' => date('Y-m-d H:i:s'),
        'source' => 'offline',
        'error' => 'Data unavailable - API limit reached or connection failed'
    ];
}
?>
