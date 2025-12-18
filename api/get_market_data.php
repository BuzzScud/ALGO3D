<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$source = $_GET['source'] ?? DEFAULT_API_SOURCE;
$source = in_array($source, ['finnhub', 'yahoo']) ? $source : DEFAULT_API_SOURCE;

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['success' => false, 'message' => 'Database connection failed']);
    exit;
}

try {
    // Get all symbols from database
    $stmt = $conn->query("SELECT symbol FROM stock_symbols ORDER BY symbol");
    $symbols = $stmt->fetchAll(PDO::FETCH_COLUMN);
    
    if (empty($symbols)) {
        echo json_encode([
            'success' => true,
            'data' => [],
            'source' => $source,
            'message' => 'No symbols found. Add symbols to track market data.'
        ]);
        exit;
    }
    
    $marketData = [];
    
    // Fetch data for each symbol
    foreach ($symbols as $symbol) {
        $symbol = strtoupper(trim($symbol));
        if (empty($symbol)) continue;
        
        $data = null;
        
        if ($source === 'finnhub') {
            $data = fetchFinnhubQuote($symbol);
            if ($data && isset($data['c']) && $data['c'] > 0) {
                $price = $data['c'];
                $prevClose = $data['pc'] ?? $price;
                $change = $data['d'] ?? ($price - $prevClose);
                $changePercent = $data['dp'] ?? (($prevClose > 0) ? (($price - $prevClose) / $prevClose * 100) : 0);
                
                $marketData[] = [
                    'symbol' => $symbol,
                    'price' => $price,
                    'change' => $change,
                    'changePercent' => $changePercent,
                    'open' => $data['o'] ?? $price,
                    'high' => $data['h'] ?? $price,
                    'low' => $data['l'] ?? $price,
                    'previousClose' => $prevClose,
                    'volume' => $data['v'] ?? 0,
                    'source' => 'finnhub'
                ];
                continue;
            }
        }
        
        // Fallback to Yahoo Finance
        $yahooData = fetchYahooQuote($symbol);
        if ($yahooData && isset($yahooData['current']) && $yahooData['current'] > 0) {
            $price = $yahooData['current'];
            $prevClose = $yahooData['previousClose'] ?? $price;
            $change = $yahooData['change'] ?? ($price - $prevClose);
            $changePercent = $yahooData['changePercent'] ?? (($prevClose > 0) ? (($price - $prevClose) / $prevClose * 100) : 0);
            
            $marketData[] = [
                'symbol' => $symbol,
                'price' => $price,
                'change' => $change,
                'changePercent' => $changePercent,
                'open' => $yahooData['open'] ?? $price,
                'high' => $yahooData['high'] ?? $price,
                'low' => $yahooData['low'] ?? $price,
                'previousClose' => $prevClose,
                'volume' => 0,
                'source' => 'yahoo'
            ];
        } else {
            // Add symbol with error flag
            $marketData[] = [
                'symbol' => $symbol,
                'price' => 0,
                'change' => 0,
                'changePercent' => 0,
                'error' => true,
                'source' => $source
            ];
        }
    }
    
    echo json_encode([
        'success' => true,
        'data' => $marketData,
        'source' => $source,
        'count' => count($marketData)
    ]);
    
} catch (Exception $e) {
    error_log("Get market data error: " . $e->getMessage());
    echo json_encode([
        'success' => false,
        'message' => 'Error fetching market data: ' . $e->getMessage()
    ]);
}

function fetchFinnhubQuote($symbol) {
    require_once __DIR__ . '/track_api_usage.php';
    
    $apiKey = FINNHUB_API_KEY;
    if ($apiKey === 'demo' || empty($apiKey)) {
        return null;
    }
    
    // Check rate limit
    if (!checkRateLimit('finnhub', FINNHUB_RATE_LIMIT)) {
        error_log("Finnhub rate limit exceeded");
        return null;
    }
    
    $url = "https://finnhub.io/api/v1/quote?symbol={$symbol}&token={$apiKey}";
    
    $startTime = microtime(true);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $responseTime = (microtime(true) - $startTime) * 1000; // Convert to milliseconds
    
    $success = ($httpCode === 200 && !$error);
    
    if (!$success) {
        trackAPICall('finnhub', false, $responseTime, "HTTP $httpCode: $error");
        return null;
    }
    
    $data = json_decode($response, true);
    
    // Check if we got valid data (Finnhub returns all zeros for invalid symbols)
    if (!$data || (isset($data['c']) && $data['c'] == 0)) {
        trackAPICall('finnhub', false, $responseTime, 'Invalid data returned');
        return null;
    }
    
    // Track successful call
    trackAPICall('finnhub', true, $responseTime);
    
    return $data;
}

function fetchYahooQuote($symbol) {
    require_once __DIR__ . '/track_api_usage.php';
    
    // Check rate limit
    if (!checkRateLimit('yahoo', 100)) {
        error_log("Yahoo Finance rate limit exceeded");
        return null;
    }
    
    $url = "https://query1.finance.yahoo.com/v8/finance/chart/{$symbol}?interval=1d&range=1d";
    
    $startTime = microtime(true);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36');
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);
    $responseTime = (microtime(true) - $startTime) * 1000; // Convert to milliseconds
    
    $success = ($httpCode === 200 && !empty($response) && !$error);
    
    if (!$success) {
        trackAPICall('yahoo', false, $responseTime, "HTTP $httpCode: " . ($error ?: 'Empty response'));
        return null;
    }
    
    $data = json_decode($response, true);
    
    if (!isset($data['chart']['result'][0]['meta'])) {
        trackAPICall('yahoo', false, $responseTime, 'Invalid data structure');
        return null;
    }
    
    $meta = $data['chart']['result'][0]['meta'];
    $current = $meta['regularMarketPrice'] ?? 0;
    $prevClose = $meta['previousClose'] ?? $current;
    
    // If previousClose is 0 or missing, try to use chartPrice or regularMarketPreviousClose
    if ($prevClose <= 0) {
        $prevClose = $meta['chartPreviousClose'] ?? $meta['regularMarketPreviousClose'] ?? $current;
    }
    
    // If still 0, use current price (no change)
    if ($prevClose <= 0) {
        $prevClose = $current;
    }
    
    $change = $current - $prevClose;
    $changePercent = ($prevClose > 0) ? (($change / $prevClose) * 100) : 0;
    
    // Track successful call
    trackAPICall('yahoo', true, $responseTime);
    
    return [
        'success' => true,
        'symbol' => $symbol,
        'current' => $current,
        'high' => $meta['regularMarketDayHigh'] ?? $current,
        'low' => $meta['regularMarketDayLow'] ?? $current,
        'open' => $meta['regularMarketOpen'] ?? $current,
        'previousClose' => $prevClose,
        'change' => $change,
        'changePercent' => $changePercent,
        'timestamp' => time()
    ];
}
?>
