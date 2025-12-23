<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$rateLimitFile = '../cache/rate_limit.json';
$currentMinute = floor(time() / 60);

$status = [
    'finnhub' => [
        'name' => 'Finnhub',
        'limit' => FINNHUB_RATE_LIMIT,
        'used' => 0,
        'remaining' => FINNHUB_RATE_LIMIT,
        'available' => true
    ],
    'yahoo' => [
        'name' => 'Yahoo Finance',
        'limit' => 100,
        'used' => 0,
        'remaining' => 100,
        'available' => true
    ]
];

// Load custom APIs
$db = new Database();
$conn = $db->getConnection();
if ($conn) {
    try {
        $stmt = $conn->query("SELECT api_id, name, rate_limit FROM custom_apis WHERE is_active = 1");
        $customApis = $stmt->fetchAll(PDO::FETCH_ASSOC);
        foreach ($customApis as $customApi) {
            $apiId = $customApi['api_id'];
            $status[$apiId] = [
                'name' => $customApi['name'],
                'limit' => (int)$customApi['rate_limit'],
                'used' => 0,
                'remaining' => (int)$customApi['rate_limit'],
                'available' => true
            ];
        }
    } catch (Exception $e) {
        error_log("Error loading custom APIs in get_api_status: " . $e->getMessage());
    }
}

if (file_exists($rateLimitFile)) {
    $data = json_decode(file_get_contents($rateLimitFile), true);
    
    // Process all APIs (built-in and custom)
    foreach ($status as $apiId => &$apiStatus) {
        if (isset($data[$apiId]) && $data[$apiId]['minute'] === $currentMinute) {
            $apiStatus['used'] = $data[$apiId]['count'];
            $apiStatus['remaining'] = $apiStatus['limit'] - $data[$apiId]['count'];
            $apiStatus['available'] = $apiStatus['remaining'] > 0;
        }
    }
    unset($apiStatus); // Break reference
}

$status['default'] = DEFAULT_API_SOURCE;
$status['cache_duration'] = CACHE_DURATION;

echo json_encode($status);
?>

















