<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

// Finnhub API Key
$apiKey = FINNHUB_API_KEY;

// Cache file
$cacheFile = '../cache/news_cache.json';
$cacheDuration = 300; // 5 minutes

// Check cache first
if (file_exists($cacheFile)) {
    $cacheData = json_decode(file_get_contents($cacheFile), true);
    if ($cacheData && isset($cacheData['timestamp']) && (time() - $cacheData['timestamp']) < $cacheDuration) {
        echo json_encode([
            'success' => true,
            'data' => $cacheData['data'],
            'cached' => true
        ]);
        exit;
    }
}

// Get category (general, forex, crypto, merger)
$category = $_GET['category'] ?? 'general';

// Get symbol if provided (for company-specific news)
$symbol = $_GET['symbol'] ?? '';

// Build API URL
$url = "https://finnhub.io/api/v1/news?category={$category}";
if (!empty($symbol)) {
    $url .= "&symbol=" . urlencode(strtoupper($symbol));
}
$url .= "&token=" . $apiKey;

// Initialize cURL
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_TIMEOUT, 10);
curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);

$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
$curlError = curl_error($ch);
// curl_close() is deprecated in PHP 8.0+ - resources are automatically closed

if ($curlError) {
    echo json_encode([
        'success' => false,
        'message' => 'Error fetching news: ' . $curlError
    ]);
    exit;
}

if ($httpCode !== 200) {
    echo json_encode([
        'success' => false,
        'message' => 'API error: HTTP ' . $httpCode
    ]);
    exit;
}

$newsData = json_decode($response, true);

if (json_last_error() !== JSON_ERROR_NONE) {
    echo json_encode([
        'success' => false,
        'message' => 'Invalid JSON response from API'
    ]);
    exit;
}

// Limit to 150 most recent articles (30 per page × 5 pages)
if (is_array($newsData)) {
    $newsData = array_slice($newsData, 0, 150);
    
    // Save to cache
    file_put_contents($cacheFile, json_encode([
        'timestamp' => time(),
        'data' => $newsData
    ]));
    
    echo json_encode([
        'success' => true,
        'data' => $newsData,
        'cached' => false
    ]);
} else {
    echo json_encode([
        'success' => false,
        'message' => 'No news data received'
    ]);
}
?>

