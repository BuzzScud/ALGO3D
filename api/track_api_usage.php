<?php
/**
 * API Usage Tracking Function
 * Tracks real API calls with response times, success/failure, etc.
 */

require_once '../includes/config.php';
require_once '../includes/database.php';

/**
 * Track an API call
 * @param string $apiId - API identifier (finnhub, yahoo, etc.)
 * @param string $apiName - Display name of the API
 * @param bool $success - Whether the call was successful
 * @param int $responseTime - Response time in milliseconds
 * @param string $error - Error message if failed
 */
function trackApiUsage($apiId, $apiName, $success = true, $responseTime = 0, $error = null) {
    try {
        $database = new Database();
        $conn = $database->getConnection();
        
        if (!$conn) {
            return false;
        }
        
        // Ensure table exists
        $conn->exec("
            CREATE TABLE IF NOT EXISTS api_usage_stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                api_id TEXT NOT NULL,
                api_name TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                calls_count INTEGER DEFAULT 1,
                success_count INTEGER DEFAULT 1,
                error_count INTEGER DEFAULT 0,
                avg_response_time INTEGER DEFAULT 0,
                error_message TEXT
            )
        ");
        
        // Insert or update stats for current minute
        $currentMinute = date('Y-m-d H:i:00');
        
        // Check if record exists for this minute
        $stmt = $conn->prepare("
            SELECT id, calls_count, success_count, error_count, avg_response_time 
            FROM api_usage_stats 
            WHERE api_id = ? AND timestamp >= datetime('now', '-1 minute')
            ORDER BY timestamp DESC LIMIT 1
        ");
        $stmt->execute([$apiId]);
        $existing = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($existing) {
            // Update existing record
            $oldCallsCount = intval($existing['calls_count'] ?? 0);
            $oldSuccessCount = intval($existing['success_count'] ?? 0);
            $oldErrorCount = intval($existing['error_count'] ?? 0);
            
            $newCallsCount = $oldCallsCount + 1;
            $newSuccessCount = $success ? $oldSuccessCount + 1 : $oldSuccessCount;
            $newErrorCount = $success ? $oldErrorCount : $oldErrorCount + 1;
            
            // Calculate weighted average response time
            $oldAvg = floatval($existing['avg_response_time'] ?? 0);
            $newAvg = $oldCallsCount > 0 ? (($oldAvg * $oldCallsCount) + $responseTime) / $newCallsCount : $responseTime;
            
            $updateStmt = $conn->prepare("
                UPDATE api_usage_stats 
                SET calls_count = ?, 
                    success_count = ?, 
                    error_count = ?,
                    avg_response_time = ?,
                    error_message = ?
                WHERE id = ?
            ");
            $updateStmt->execute([
                $newCallsCount,
                $newSuccessCount,
                $newErrorCount,
                round($newAvg),
                $error,
                $existing['id']
            ]);
        } else {
            // Insert new record
            $insertStmt = $conn->prepare("
                INSERT INTO api_usage_stats 
                (api_id, api_name, calls_count, success_count, error_count, avg_response_time, error_message)
                VALUES (?, ?, 1, ?, ?, ?, ?)
            ");
            $insertStmt->execute([
                $apiId,
                $apiName,
                1, // calls_count
                $success ? 1 : 0, // success_count
                $success ? 0 : 1, // error_count
                $responseTime,
                $error
            ]);
        }
        
        return true;
    } catch (Exception $e) {
        error_log("Failed to track API usage: " . $e->getMessage());
        return false;
    }
}

/**
 * Get real-time metrics for an API
 * @param string $apiId - API identifier
 * @param string $timeframe - hour, day, week, month
 * @return array
 */
function getRealtimeMetrics($apiId, $timeframe = 'hour') {
    try {
        $database = new Database();
        $conn = $database->getConnection();
        
        if (!$conn) {
            return null;
        }
        
        $timeRanges = [
            'hour' => '1 hour',
            'day' => '24 hours',
            'week' => '7 days',
            'month' => '30 days'
        ];
        
        $range = $timeRanges[$timeframe] ?? '1 hour';
        
        // Ensure table exists before querying
        $conn->exec("
            CREATE TABLE IF NOT EXISTS api_usage_stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                api_id TEXT NOT NULL,
                api_name TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                calls_count INTEGER DEFAULT 1,
                success_count INTEGER DEFAULT 1,
                error_count INTEGER DEFAULT 0,
                avg_response_time INTEGER DEFAULT 0,
                error_message TEXT
            )
        ");
        
        $stmt = $conn->prepare("
            SELECT 
                COALESCE(SUM(calls_count), 0) as total_calls,
                COALESCE(SUM(success_count), 0) as total_success,
                COALESCE(SUM(error_count), 0) as total_errors,
                COALESCE(AVG(avg_response_time), 0) as avg_response_time,
                MAX(timestamp) as last_call
            FROM api_usage_stats
            WHERE api_id = ? AND timestamp >= datetime('now', '-' || ?)
        ");
        
        $stmt->execute([$apiId, $range]);
        $result = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($result && $result['total_calls'] !== null) {
            $totalCalls = intval($result['total_calls'] ?? 0);
            $totalSuccess = intval($result['total_success'] ?? 0);
            $totalErrors = intval($result['total_errors'] ?? 0);
            $avgResponseTime = floatval($result['avg_response_time'] ?? 0);
            
            // Ensure no division by zero
            $successRate = $totalCalls > 0 ? ($totalSuccess / $totalCalls) * 100 : 0;
            
            return [
                'total_calls' => max(0, $totalCalls),
                'success_count' => max(0, $totalSuccess),
                'error_count' => max(0, $totalErrors),
                'success_rate' => max(0, min(100, round($successRate, 2))),
                'avg_response_time' => max(0, round($avgResponseTime)),
                'last_call' => $result['last_call'] ?? null
            ];
        }
        
        // Return zero values if no data found
        return [
            'total_calls' => 0,
            'success_count' => 0,
            'error_count' => 0,
            'success_rate' => 0,
            'avg_response_time' => 0,
            'last_call' => null
        ];
    } catch (Exception $e) {
        error_log("Failed to get realtime metrics: " . $e->getMessage());
        return null;
    }
}

?>

