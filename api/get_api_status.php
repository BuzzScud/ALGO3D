<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';

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

if (file_exists($rateLimitFile)) {
    $data = json_decode(file_get_contents($rateLimitFile), true);
    
    foreach (['finnhub', 'yahoo'] as $api) {
        if (isset($data[$api]) && $data[$api]['minute'] === $currentMinute) {
            $status[$api]['used'] = $data[$api]['count'];
            $status[$api]['remaining'] = $status[$api]['limit'] - $data[$api]['count'];
            $status[$api]['available'] = $status[$api]['remaining'] > 0;
        }
    }
}

$status['default'] = DEFAULT_API_SOURCE;
$status['cache_duration'] = CACHE_DURATION;

echo json_encode($status);
?>









