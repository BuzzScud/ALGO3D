<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';

$rateLimitFile = '../cache/rate_limit.json';
$currentMinute = floor(time() / 60);
$currentTime = time();

// Initialize API statuses
$apis = [
    'finnhub' => [
        'id' => 'finnhub',
        'name' => 'Finnhub',
        'description' => 'Primary market data provider for real-time quotes and historical data',
        'baseUrl' => FINNHUB_BASE_URL,
        'rateLimit' => FINNHUB_RATE_LIMIT,
        'rateLimitPeriod' => 60, // seconds
        'used' => 0,
        'remaining' => FINNHUB_RATE_LIMIT,
        'available' => true,
        'status' => 'active',
        'requiresKey' => true,
        'hasKey' => (FINNHUB_API_KEY !== 'demo' && !empty(FINNHUB_API_KEY)),
        'lastUsed' => null,
        'totalCalls' => 0,
        'successRate' => 100,
        'avgResponseTime' => 0,
        'errors' => 0
    ],
    'yahoo' => [
        'id' => 'yahoo',
        'name' => 'Yahoo Finance',
        'description' => 'Backup data provider, no API key required',
        'baseUrl' => YAHOO_FINANCE_BASE_URL,
        'rateLimit' => 100,
        'rateLimitPeriod' => 60,
        'used' => 0,
        'remaining' => 100,
        'available' => true,
        'status' => 'active',
        'requiresKey' => false,
        'hasKey' => true,
        'lastUsed' => null,
        'totalCalls' => 0,
        'successRate' => 100,
        'avgResponseTime' => 0,
        'errors' => 0
    ]
];

// Load rate limit data
if (file_exists($rateLimitFile)) {
    $data = json_decode(file_get_contents($rateLimitFile), true);
    
    foreach (['finnhub', 'yahoo'] as $apiId) {
        if (isset($data[$apiId])) {
            $apiData = $data[$apiId];
            
            // Current minute usage
            if (isset($apiData['minute']) && $apiData['minute'] === $currentMinute) {
                $apis[$apiId]['used'] = $apiData['count'] ?? 0;
                $apis[$apiId]['remaining'] = $apis[$apiId]['rateLimit'] - $apis[$apiId]['used'];
                $apis[$apiId]['available'] = $apis[$apiId]['remaining'] > 0;
            }
            
            // Historical data
            if (isset($apiData['totalCalls'])) {
                $apis[$apiId]['totalCalls'] = $apiData['totalCalls'];
            }
            if (isset($apiData['lastUsed'])) {
                $apis[$apiId]['lastUsed'] = $apiData['lastUsed'];
            }
            if (isset($apiData['successRate'])) {
                $apis[$apiId]['successRate'] = $apiData['successRate'];
            } else {
                // Calculate success rate if not set
                if (isset($apiData['totalCalls']) && $apiData['totalCalls'] > 0) {
                    $successCalls = $apiData['successCalls'] ?? 0;
                    $apis[$apiId]['successRate'] = ($successCalls / $apiData['totalCalls']) * 100;
                }
            }
            if (isset($apiData['avgResponseTime'])) {
                $apis[$apiId]['avgResponseTime'] = $apiData['avgResponseTime'];
            }
            if (isset($apiData['errorCalls'])) {
                $apis[$apiId]['errors'] = $apiData['errorCalls'];
            } else {
                $apis[$apiId]['errors'] = 0;
            }
        }
    }
}

// Check API health by making test calls
function testApiHealth($apiId, $apiConfig) {
    $health = [
        'status' => 'unknown',
        'responseTime' => 0,
        'lastChecked' => time(),
        'error' => null
    ];
    
    try {
        $startTime = microtime(true);
        
        if ($apiId === 'finnhub') {
            if (!$apiConfig['hasKey']) {
                $health['status'] = 'no_key';
                $health['error'] = 'API key not configured';
                return $health;
            }
            
            $url = "https://finnhub.io/api/v1/quote?symbol=AAPL&token=" . FINNHUB_API_KEY;
            $ch = curl_init();
            curl_setopt($ch, CURLOPT_URL, $url);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_TIMEOUT, 5);
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
            $response = curl_exec($ch);
            $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            
            if ($httpCode === 200) {
                $health['status'] = 'healthy';
            } else {
                $health['status'] = 'error';
                $health['error'] = "HTTP $httpCode";
            }
        } else if ($apiId === 'yahoo') {
            $url = "https://query1.finance.yahoo.com/v8/finance/chart/AAPL?interval=1d&range=1d";
            $ch = curl_init();
            curl_setopt($ch, CURLOPT_URL, $url);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_TIMEOUT, 5);
            curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0');
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
            $response = curl_exec($ch);
            $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            
            if ($httpCode === 200) {
                $health['status'] = 'healthy';
            } else {
                $health['status'] = 'error';
                $health['error'] = "HTTP $httpCode";
            }
        }
        
        $health['responseTime'] = round((microtime(true) - $startTime) * 1000, 2);
    } catch (Exception $e) {
        $health['status'] = 'error';
        $health['error'] = $e->getMessage();
    }
    
    return $health;
}

// Test API health (only if not too frequent - cache for 30 seconds)
$healthCacheFile = '../cache/api_health.json';
$healthData = [];
if (file_exists($healthCacheFile)) {
    $healthData = json_decode(file_get_contents($healthCacheFile), true);
}

$shouldTestHealth = true;
if (isset($healthData['lastCheck']) && ($currentTime - $healthData['lastCheck']) < 30) {
    $shouldTestHealth = false;
}

if ($shouldTestHealth) {
    foreach ($apis as $apiId => &$api) {
        $health = testApiHealth($apiId, $api);
        $api['health'] = $health;
        $api['status'] = $health['status'] === 'healthy' ? 'active' : ($health['status'] === 'no_key' ? 'no_key' : 'error');
        
        if ($health['status'] === 'healthy' && $health['responseTime'] > 0) {
            // Update average response time
            if ($api['avgResponseTime'] > 0) {
                $api['avgResponseTime'] = ($api['avgResponseTime'] + $health['responseTime']) / 2;
            } else {
                $api['avgResponseTime'] = $health['responseTime'];
            }
        }
    }
    
    $healthData = [
        'lastCheck' => $currentTime,
        'apis' => array_map(function($api) {
            return ['health' => $api['health'], 'status' => $api['status']];
        }, $apis)
    ];
    file_put_contents($healthCacheFile, json_encode($healthData));
} else {
    // Use cached health data
    if (isset($healthData['apis'])) {
        foreach ($apis as $apiId => &$api) {
            if (isset($healthData['apis'][$apiId])) {
                $api['health'] = $healthData['apis'][$apiId]['health'] ?? null;
                $api['status'] = $healthData['apis'][$apiId]['status'] ?? 'unknown';
            }
        }
    }
}

// Get current default source
session_start();
$currentSource = $_SESSION['api_source'] ?? DEFAULT_API_SOURCE;

// Calculate usage percentage
foreach ($apis as &$api) {
    $api['usagePercent'] = ($api['used'] / $api['rateLimit']) * 100;
    $api['isDefault'] = ($api['id'] === $currentSource);
}

// Prepare response
$response = [
    'success' => true,
    'timestamp' => $currentTime,
    'defaultSource' => $currentSource,
    'cacheDuration' => CACHE_DURATION,
    'apis' => array_values($apis),
    'summary' => [
        'totalApis' => count($apis),
        'activeApis' => count(array_filter($apis, fn($a) => $a['status'] === 'active')),
        'totalCalls' => array_sum(array_column($apis, 'totalCalls')),
        'totalRemaining' => array_sum(array_column($apis, 'remaining'))
    ]
];

echo json_encode($response);
?>

