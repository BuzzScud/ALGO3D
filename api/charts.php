<?php
// Suppress deprecation warnings that would break JSON output
error_reporting(E_ALL & ~E_DEPRECATED);

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE');
header('Access-Control-Allow-Headers: Content-Type');

require_once '../includes/config.php';
require_once '../includes/database.php';

$database = new Database();
$conn = $database->getConnection();

$method = $_SERVER['REQUEST_METHOD'];
$action = $_GET['action'] ?? '';

// Cache directory for chart data
$cacheDir = __DIR__ . '/../data/chart_cache';
if (!is_dir($cacheDir)) {
    mkdir($cacheDir, 0755, true);
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

    // Check cache (5 minute cache for quotes)
    $cacheFile = $cacheDir . '/quote_' . $symbol . '.json';
    if (file_exists($cacheFile) && (time() - filemtime($cacheFile)) < 300) {
        echo file_get_contents($cacheFile);
        return;
    }

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
            'timestamp' => $data['t'] ?? time()
        ];
        file_put_contents($cacheFile, json_encode($result));
        echo json_encode($result);
    } else {
        // Try Yahoo Finance as backup
        $yahooData = fetchYahooQuote($symbol);
        if ($yahooData) {
            file_put_contents($cacheFile, json_encode($yahooData));
            echo json_encode($yahooData);
        } else {
            echo json_encode(['success' => false, 'message' => 'Unable to fetch quote data']);
        }
    }
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
    
    // Check cache
    $cacheKey = $symbol . '_' . $timeframe;
    $cacheFile = $cacheDir . '/chart_' . $cacheKey . '.json';
    $cacheTime = $timeframe === '1D' ? 60 : 300; // 1 min for intraday, 5 min for others
    
    if (file_exists($cacheFile) && (time() - filemtime($cacheFile)) < $cacheTime) {
        echo file_get_contents($cacheFile);
        return;
    }

    $data = fetchFinnhubCandles($symbol, $params['resolution'], $params['from'], $params['to']);
    
    if ($data && isset($data['c']) && count($data['c']) > 0) {
        $chartData = formatCandleData($data, $symbol, $timeframe);
        file_put_contents($cacheFile, json_encode($chartData));
        echo json_encode($chartData);
    } else {
        // Generate demo data if API fails
        $demoData = generateDemoData($symbol, $timeframe);
        echo json_encode($demoData);
    }
}

function getTimeframeParams($timeframe) {
    $now = time();
    $resolution = '60'; // Default to 1 hour
    $from = $now - 86400; // Default to 1 day
    
    switch ($timeframe) {
        case '1D':
            $resolution = '5';
            $from = strtotime('today 09:30', $now);
            break;
        case '5D':
            $resolution = '15';
            $from = $now - (5 * 86400);
            break;
        case '1M':
            $resolution = '60';
            $from = $now - (30 * 86400);
            break;
        case '3M':
            $resolution = 'D';
            $from = $now - (90 * 86400);
            break;
        case '6M':
            $resolution = 'D';
            $from = $now - (180 * 86400);
            break;
        case '1Y':
            $resolution = 'D';
            $from = $now - (365 * 86400);
            break;
        case '5Y':
            $resolution = 'W';
            $from = $now - (5 * 365 * 86400);
            break;
    }
    
    return [
        'resolution' => $resolution,
        'from' => $from,
        'to' => $now
    ];
}

function fetchFinnhubQuote($symbol) {
    $apiKey = FINNHUB_API_KEY;
    $url = "https://finnhub.io/api/v1/quote?symbol={$symbol}&token={$apiKey}";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    $response = curl_exec($ch);
    
    return json_decode($response, true);
}

function fetchFinnhubCandles($symbol, $resolution, $from, $to) {
    $apiKey = FINNHUB_API_KEY;
    $url = "https://finnhub.io/api/v1/stock/candle?symbol={$symbol}&resolution={$resolution}&from={$from}&to={$to}&token={$apiKey}";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 15);
    $response = curl_exec($ch);
    
    return json_decode($response, true);
}

function fetchYahooQuote($symbol) {
    $url = "https://query1.finance.yahoo.com/v8/finance/chart/{$symbol}?interval=1d&range=1d";
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0');
    $response = curl_exec($ch);
    
    $data = json_decode($response, true);
    
    if (isset($data['chart']['result'][0]['meta'])) {
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

function formatCandleData($data, $symbol, $timeframe) {
    $candles = [];
    $count = count($data['c']);
    
    for ($i = 0; $i < $count; $i++) {
        $candles[] = [
            'time' => $data['t'][$i] * 1000, // Convert to milliseconds
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
            'bollinger' => $bollinger
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
            'bollinger' => calculateBollingerBands($closes, 20)
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

