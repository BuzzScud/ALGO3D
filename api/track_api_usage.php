<?php
/**
 * API Usage Tracking Utility
 * Tracks API calls, response times, errors, and maintains statistics
 */

function trackAPICall($apiId, $success = true, $responseTime = null, $error = null) {
    $rateLimitFile = __DIR__ . '/../cache/rate_limit.json';
    $currentMinute = floor(time() / 60);
    $currentTime = time();
    
    // Load existing data
    $data = [];
    if (file_exists($rateLimitFile)) {
        $data = json_decode(file_get_contents($rateLimitFile), true) ?: [];
    }
    
    // Initialize API data if not exists
    if (!isset($data[$apiId])) {
        $data[$apiId] = [
            'minute' => $currentMinute,
            'count' => 0,
            'totalCalls' => 0,
            'successCalls' => 0,
            'errorCalls' => 0,
            'totalResponseTime' => 0,
            'responseTimes' => [],
            'lastUsed' => null,
            'errors' => []
        ];
    }
    
    // Reset count if new minute
    if ($data[$apiId]['minute'] !== $currentMinute) {
        $data[$apiId]['minute'] = $currentMinute;
        $data[$apiId]['count'] = 0;
    }
    
    // Increment counters
    $data[$apiId]['count']++;
    $data[$apiId]['totalCalls']++;
    
    if ($success) {
        $data[$apiId]['successCalls']++;
    } else {
        $data[$apiId]['errorCalls']++;
        if ($error) {
            $data[$apiId]['errors'][] = [
                'time' => $currentTime,
                'message' => $error
            ];
            // Keep only last 10 errors
            if (count($data[$apiId]['errors']) > 10) {
                $data[$apiId]['errors'] = array_slice($data[$apiId]['errors'], -10);
            }
        }
    }
    
    // Track response time
    if ($responseTime !== null && $responseTime > 0) {
        $data[$apiId]['totalResponseTime'] += $responseTime;
        $data[$apiId]['responseTimes'][] = $responseTime;
        // Keep only last 100 response times for average calculation
        if (count($data[$apiId]['responseTimes']) > 100) {
            $data[$apiId]['responseTimes'] = array_slice($data[$apiId]['responseTimes'], -100);
        }
    }
    
    $data[$apiId]['lastUsed'] = $currentTime;
    
    // Calculate success rate
    if ($data[$apiId]['totalCalls'] > 0) {
        $data[$apiId]['successRate'] = ($data[$apiId]['successCalls'] / $data[$apiId]['totalCalls']) * 100;
    } else {
        $data[$apiId]['successRate'] = 100;
    }
    
    // Calculate average response time
    if (count($data[$apiId]['responseTimes']) > 0) {
        $data[$apiId]['avgResponseTime'] = array_sum($data[$apiId]['responseTimes']) / count($data[$apiId]['responseTimes']);
    } else {
        $data[$apiId]['avgResponseTime'] = 0;
    }
    
    // Save data
    file_put_contents($rateLimitFile, json_encode($data, JSON_PRETTY_PRINT));
    
    return $data[$apiId];
}

function checkRateLimit($apiId, $limit) {
    $rateLimitFile = __DIR__ . '/../cache/rate_limit.json';
    $currentMinute = floor(time() / 60);
    
    if (file_exists($rateLimitFile)) {
        $data = json_decode(file_get_contents($rateLimitFile), true);
        if (isset($data[$apiId]) && $data[$apiId]['minute'] === $currentMinute) {
            return $data[$apiId]['count'] < $limit;
        }
    }
    
    return true;
}
?>

