<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$rateLimitFile = '../cache/rate_limit.json';
$currentMinute = floor(time() / 60);
$currentTime = time();

// Load custom APIs from database
$db = new Database();
$conn = $db->getConnection();
$customApis = [];
if ($conn) {
    try {
        $stmt = $conn->query("SELECT * FROM custom_apis WHERE is_active = 1");
        $customApisData = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        foreach ($customApisData as $customApi) {
            $apiId = $customApi['api_id'];
            $customApis[$apiId] = [
                'id' => $apiId,
                'name' => $customApi['name'],
                'description' => $customApi['description'] ?? '',
                'baseUrl' => $customApi['base_url'],
                'rateLimit' => (int)$customApi['rate_limit'],
                'rateLimitPeriod' => (int)$customApi['rate_limit_period'],
                'used' => 0,
                'remaining' => (int)$customApi['rate_limit'],
                'available' => true,
                'status' => 'active',
                'requiresKey' => (bool)$customApi['requires_key'],
                'hasKey' => !empty($customApi['api_key']),
                'lastUsed' => null,
                'totalCalls' => 0,
                'successRate' => 100,
                'avgResponseTime' => 0,
                'errors' => 0,
                'isCustom' => true,
                'quoteUrlTemplate' => $customApi['quote_url_template'],
                'apiKey' => $customApi['api_key'] ?? '',
                'responseFormat' => $customApi['response_format'] ?? 'json',
                'quotePath' => $customApi['quote_path'] ?? '',
                'priceField' => $customApi['price_field'] ?? 'c',
                'changeField' => $customApi['change_field'] ?? 'd',
                'changePercentField' => $customApi['change_percent_field'] ?? 'dp',
                'volumeField' => $customApi['volume_field'] ?? 'v',
                'highField' => $customApi['high_field'] ?? 'h',
                'lowField' => $customApi['low_field'] ?? 'l',
                'openField' => $customApi['open_field'] ?? 'o',
                'previousCloseField' => $customApi['previous_close_field'] ?? 'pc'
            ];
        }
    } catch (Exception $e) {
        error_log("Error loading custom APIs: " . $e->getMessage());
    }
}

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
        'errors' => 0,
        'isCustom' => false
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
        'errors' => 0,
        'isCustom' => false
    ]
];

// Merge custom APIs
$apis = array_merge($apis, $customApis);

// Load rate limit data
if (file_exists($rateLimitFile)) {
    $data = json_decode(file_get_contents($rateLimitFile), true);
    
    // Process all APIs (built-in and custom)
    foreach ($apis as $apiId => &$api) {
        if (isset($data[$apiId])) {
            $apiData = $data[$apiId];
            
            // Current minute usage
            if (isset($apiData['minute']) && $apiData['minute'] === $currentMinute) {
                $api['used'] = $apiData['count'] ?? 0;
                $api['remaining'] = $api['rateLimit'] - $api['used'];
                $api['available'] = $api['remaining'] > 0;
            }
            
            // Historical data
            if (isset($apiData['totalCalls'])) {
                $api['totalCalls'] = $apiData['totalCalls'];
            }
            if (isset($apiData['lastUsed'])) {
                $api['lastUsed'] = $apiData['lastUsed'];
            }
            if (isset($apiData['successRate'])) {
                $api['successRate'] = $apiData['successRate'];
            } else {
                // Calculate success rate if not set
                if (isset($apiData['totalCalls']) && $apiData['totalCalls'] > 0) {
                    $successCalls = $apiData['successCalls'] ?? 0;
                    $api['successRate'] = ($successCalls / $apiData['totalCalls']) * 100;
                }
            }
            if (isset($apiData['avgResponseTime'])) {
                $api['avgResponseTime'] = $apiData['avgResponseTime'];
            }
            if (isset($apiData['errorCalls'])) {
                $api['errors'] = $apiData['errorCalls'];
            } else {
                $api['errors'] = 0;
            }
        }
    }
    unset($api); // Break reference
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
        } else if (isset($apiConfig['isCustom']) && $apiConfig['isCustom']) {
            // Test custom API
            if ($apiConfig['requiresKey'] && !$apiConfig['hasKey']) {
                $health['status'] = 'no_key';
                $health['error'] = 'API key not configured';
                return $health;
            }
            
            // Build test URL from template
            $testUrl = str_replace('{symbol}', 'AAPL', $apiConfig['quoteUrlTemplate']);
            if ($apiConfig['requiresKey'] && !empty($apiConfig['apiKey'])) {
                $testUrl = str_replace('{api_key}', $apiConfig['apiKey'], $testUrl);
            }
            
            $ch = curl_init();
            curl_setopt($ch, CURLOPT_URL, $testUrl);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_TIMEOUT, 5);
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
            if (isset($apiConfig['userAgent'])) {
                curl_setopt($ch, CURLOPT_USERAGENT, $apiConfig['userAgent']);
            }
            $response = curl_exec($ch);
            $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            curl_close($ch);
            
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

