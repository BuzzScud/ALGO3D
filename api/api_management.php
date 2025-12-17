<?php
/**
 * API Management Endpoint
 * Handles CRUD operations for API configurations and statistics
 */

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, DELETE');
header('Access-Control-Allow-Headers: Content-Type');

require_once '../includes/config.php';
require_once '../includes/database.php';
require_once 'track_api_usage.php';

$database = new Database();
$conn = $database->getConnection();

if (!$conn) {
    echo json_encode(['success' => false, 'message' => 'Database connection failed']);
    exit;
}

// Create API configurations table if it doesn't exist
try {
    $conn->exec("
        CREATE TABLE IF NOT EXISTS api_configurations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            provider TEXT NOT NULL,
            type TEXT NOT NULL,
            base_url TEXT NOT NULL,
            api_key TEXT NOT NULL,
            rate_limit INTEGER DEFAULT 60,
            priority INTEGER DEFAULT 2,
            enabled INTEGER DEFAULT 1,
            notes TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ");
    
    // Create API usage statistics table
    $conn->exec("
        CREATE TABLE IF NOT EXISTS api_usage_stats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            api_id TEXT NOT NULL,
            api_name TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            calls_count INTEGER DEFAULT 1,
            success_count INTEGER DEFAULT 1,
            error_count INTEGER DEFAULT 0,
            avg_response_time INTEGER DEFAULT 0
        )
    ");
    
    // Create indexes for better performance
    $conn->exec("CREATE INDEX IF NOT EXISTS idx_api_stats_api_id ON api_usage_stats(api_id)");
    $conn->exec("CREATE INDEX IF NOT EXISTS idx_api_stats_timestamp ON api_usage_stats(timestamp)");
} catch (PDOException $e) {
    // Tables might already exist
}

$method = $_SERVER['REQUEST_METHOD'];
$action = $_GET['action'] ?? '';

try {
    switch ($method) {
        case 'GET':
            handleGet($conn, $action);
            break;
        case 'POST':
            handlePost($conn);
            break;
        case 'DELETE':
            handleDelete($conn);
            break;
        default:
            echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    }
} catch (Exception $e) {
    echo json_encode(['success' => false, 'message' => $e->getMessage()]);
}

function handleGet($conn, $action) {
    switch ($action) {
        case 'get_configs':
            getConfigurations($conn);
            break;
        case 'get_stats':
            getStatistics($conn);
            break;
        default:
            echo json_encode(['success' => false, 'message' => 'Invalid action']);
    }
}

function handlePost($conn) {
    $input = json_decode(file_get_contents('php://input'), true);
    $action = $input['action'] ?? '';
    
    switch ($action) {
        case 'create':
            createConfiguration($conn, $input['config']);
            break;
        case 'update':
            updateConfiguration($conn, $input['config']);
            break;
        case 'delete':
            deleteConfiguration($conn, $input['id']);
            break;
        case 'toggle':
            toggleConfiguration($conn, $input['id'], $input['enabled']);
            break;
        case 'test':
            testApiConnection($conn, $input['id']);
            break;
        default:
            echo json_encode(['success' => false, 'message' => 'Invalid action']);
    }
}

function handleDelete($conn) {
    $input = json_decode(file_get_contents('php://input'), true);
    deleteConfiguration($conn, $input['id']);
}

/**
 * Get all API configurations
 */
function getConfigurations($conn) {
    try {
        $stmt = $conn->prepare("SELECT * FROM api_configurations ORDER BY priority ASC, name ASC");
        $stmt->execute();
        $configs = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Convert enabled to boolean
        foreach ($configs as &$config) {
            $config['enabled'] = (bool)$config['enabled'];
        }
        
        // Include default APIs (Finnhub and Yahoo)
        $defaultConfigs = [
            [
                'id' => 'finnhub',
                'name' => 'Finnhub',
                'provider' => 'Finnhub.io',
                'type' => 'market_data',
                'base_url' => FINNHUB_BASE_URL,
                'api_key' => FINNHUB_API_KEY !== 'demo' ? maskApiKey(FINNHUB_API_KEY) : 'Not configured',
                'rate_limit' => FINNHUB_RATE_LIMIT,
                'priority' => 1,
                'enabled' => true,
                'notes' => 'Primary market data API',
                'is_default' => true
            ],
            [
                'id' => 'yahoo',
                'name' => 'Yahoo Finance',
                'provider' => 'Yahoo Inc.',
                'type' => 'market_data',
                'base_url' => YAHOO_FINANCE_BASE_URL,
                'api_key' => 'Not required',
                'rate_limit' => 100,
                'priority' => 2,
                'enabled' => true,
                'notes' => 'Backup market data API (no key required)',
                'is_default' => true
            ]
        ];
        
        echo json_encode([
            'success' => true,
            'configs' => array_merge($defaultConfigs, $configs)
        ]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to load configurations: ' . $e->getMessage()]);
    }
}

/**
 * Create new API configuration
 */
function createConfiguration($conn, $config) {
    try {
        $stmt = $conn->prepare("
            INSERT INTO api_configurations 
            (name, provider, type, base_url, api_key, rate_limit, priority, enabled, notes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ");
        
        $stmt->execute([
            $config['name'],
            $config['provider'],
            $config['type'],
            $config['base_url'],
            $config['api_key'],
            $config['rate_limit'],
            $config['priority'],
            $config['enabled'] ? 1 : 0,
            $config['notes'] ?? ''
        ]);
        
        echo json_encode([
            'success' => true,
            'message' => 'API configuration created successfully',
            'id' => $conn->lastInsertId()
        ]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to create configuration: ' . $e->getMessage()]);
    }
}

/**
 * Update API configuration
 */
function updateConfiguration($conn, $config) {
    try {
        $stmt = $conn->prepare("
            UPDATE api_configurations 
            SET name = ?, provider = ?, type = ?, base_url = ?, api_key = ?, 
                rate_limit = ?, priority = ?, enabled = ?, notes = ?, updated_at = CURRENT_TIMESTAMP
            WHERE id = ?
        ");
        
        $stmt->execute([
            $config['name'],
            $config['provider'],
            $config['type'],
            $config['base_url'],
            $config['api_key'],
            $config['rate_limit'],
            $config['priority'],
            $config['enabled'] ? 1 : 0,
            $config['notes'] ?? '',
            $config['id']
        ]);
        
        echo json_encode([
            'success' => true,
            'message' => 'API configuration updated successfully'
        ]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to update configuration: ' . $e->getMessage()]);
    }
}

/**
 * Delete API configuration
 */
function deleteConfiguration($conn, $id) {
    try {
        // Don't allow deleting default APIs
        if ($id === 'finnhub' || $id === 'yahoo') {
            echo json_encode(['success' => false, 'message' => 'Cannot delete default API']);
            return;
        }
        
        $stmt = $conn->prepare("DELETE FROM api_configurations WHERE id = ?");
        $stmt->execute([$id]);
        
        echo json_encode([
            'success' => true,
            'message' => 'API configuration deleted successfully'
        ]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to delete configuration: ' . $e->getMessage()]);
    }
}

/**
 * Toggle API enabled/disabled
 */
function toggleConfiguration($conn, $id, $enabled) {
    try {
        // Don't allow disabling default APIs
        if ($id === 'finnhub' || $id === 'yahoo') {
            echo json_encode(['success' => false, 'message' => 'Cannot disable default API']);
            return;
        }
        
        $stmt = $conn->prepare("UPDATE api_configurations SET enabled = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
        $stmt->execute([$enabled ? 1 : 0, $id]);
        
        echo json_encode([
            'success' => true,
            'message' => 'API configuration updated successfully'
        ]);
    } catch (PDOException $e) {
        echo json_encode(['success' => false, 'message' => 'Failed to toggle configuration: ' . $e->getMessage()]);
    }
}

/**
 * Test API connection
 */
function testApiConnection($conn, $id) {
    $startTime = microtime(true);
    
    try {
        if ($id === 'finnhub') {
            // Test Finnhub API
            $apiKey = FINNHUB_API_KEY;
            if ($apiKey === 'demo') {
                echo json_encode(['success' => false, 'message' => 'Finnhub API key not configured']);
                return;
            }
            
            $url = FINNHUB_BASE_URL . "/quote?symbol=AAPL&token=" . $apiKey;
            $ch = curl_init($url);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_TIMEOUT, 10);
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
            $response = curl_exec($ch);
            $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            curl_close($ch);
            
            $endTime = microtime(true);
            $responseTime = round(($endTime - $startTime) * 1000);
            
            if ($httpCode === 200) {
                echo json_encode([
                    'success' => true,
                    'message' => 'API connection successful',
                    'response_time' => $responseTime
                ]);
            } else {
                echo json_encode([
                    'success' => false,
                    'message' => 'API test failed (HTTP ' . $httpCode . ')',
                    'response_time' => $responseTime
                ]);
            }
        } else if ($id === 'yahoo') {
            // Test Yahoo Finance API
            $url = YAHOO_FINANCE_BASE_URL . "/AAPL?interval=1d&range=1d";
            $ch = curl_init($url);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_TIMEOUT, 10);
            curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0');
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
            $response = curl_exec($ch);
            $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            curl_close($ch);
            
            $endTime = microtime(true);
            $responseTime = round(($endTime - $startTime) * 1000);
            
            if ($httpCode === 200) {
                echo json_encode([
                    'success' => true,
                    'message' => 'API connection successful',
                    'response_time' => $responseTime
                ]);
            } else {
                echo json_encode([
                    'success' => false,
                    'message' => 'API test failed (HTTP ' . $httpCode . ')',
                    'response_time' => $responseTime
                ]);
            }
        } else {
            // Test custom API
            $stmt = $conn->prepare("SELECT * FROM api_configurations WHERE id = ?");
            $stmt->execute([$id]);
            $config = $stmt->fetch(PDO::FETCH_ASSOC);
            
            if (!$config) {
                echo json_encode(['success' => false, 'message' => 'API configuration not found']);
                return;
            }
            
            // Simple connection test (modify based on API requirements)
            $testUrl = $config['base_url'];
            $ch = curl_init($testUrl);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_TIMEOUT, 10);
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
            curl_setopt($ch, CURLOPT_HTTPHEADER, [
                'Authorization: Bearer ' . $config['api_key']
            ]);
            $response = curl_exec($ch);
            $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            curl_close($ch);
            
            $endTime = microtime(true);
            $responseTime = round(($endTime - $startTime) * 1000);
            
            if ($httpCode >= 200 && $httpCode < 400) {
                echo json_encode([
                    'success' => true,
                    'message' => 'API connection successful',
                    'response_time' => $responseTime
                ]);
            } else {
                echo json_encode([
                    'success' => false,
                    'message' => 'API test failed (HTTP ' . $httpCode . ')',
                    'response_time' => $responseTime
                ]);
            }
        }
    } catch (Exception $e) {
        $endTime = microtime(true);
        $responseTime = round(($endTime - $startTime) * 1000);
        echo json_encode([
            'success' => false,
            'message' => 'API test error: ' . $e->getMessage(),
            'response_time' => $responseTime
        ]);
    }
}

/**
 * Get API usage statistics
 */
function getStatistics($conn) {
    $timeframe = $_GET['timeframe'] ?? 'day';
    
    // Calculate time range
    $timeRanges = [
        'hour' => '1 hour',
        'day' => '24 hours',
        'week' => '7 days',
        'month' => '30 days'
    ];
    
    $range = $timeRanges[$timeframe] ?? '24 hours';
    
    try {
        $stmt = $conn->prepare("
            SELECT 
                api_id,
                api_name,
                SUM(calls_count) as total_calls,
                SUM(success_count) as total_success,
                SUM(error_count) as total_errors,
                AVG(avg_response_time) as avg_response_time
            FROM api_usage_stats
            WHERE timestamp >= datetime('now', '-' || ?)
            GROUP BY api_id, api_name
        ");
        
        $stmt->execute([$range]);
        $stats = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Calculate success rate and ensure data integrity
        foreach ($stats as &$stat) {
            $total = intval($stat['total_calls'] ?? 0);
            $success = intval($stat['total_success'] ?? 0);
            $errors = intval($stat['total_errors'] ?? 0);
            $avgResponseTime = floatval($stat['avg_response_time'] ?? 0);
            
            // Ensure no division by zero and valid percentages
            $stat['success_rate'] = $total > 0 ? max(0, min(100, round(($success / $total) * 100, 2))) : 0;
            $stat['errors'] = max(0, $errors);
            $stat['total_calls'] = max(0, $total);
            $stat['total_success'] = max(0, $success);
            $stat['avg_response_time'] = max(0, round($avgResponseTime));
            
            // Ensure name exists
            if (empty($stat['api_name'])) {
                $stat['api_name'] = $stat['api_id'] ?? 'Unknown API';
            }
        }
        
        // Get real-time metrics for default APIs
        $defaultStats = [];
        
        // Get real-time metrics for Finnhub
        try {
            $finnhubMetrics = getRealtimeMetrics('finnhub', $timeframe);
            if ($finnhubMetrics && is_array($finnhubMetrics)) {
                $defaultStats['finnhub'] = [
                    'name' => 'Finnhub',
                    'total_calls' => intval($finnhubMetrics['total_calls'] ?? 0),
                    'success_count' => intval($finnhubMetrics['success_count'] ?? 0),
                    'success_rate' => floatval($finnhubMetrics['success_rate'] ?? 0),
                    'avg_response_time' => intval($finnhubMetrics['avg_response_time'] ?? 0),
                    'errors' => intval($finnhubMetrics['error_count'] ?? 0)
                ];
            }
        } catch (Exception $e) {
            error_log("Error getting Finnhub metrics: " . $e->getMessage());
        }
        
        // Get real-time metrics for Yahoo
        try {
            $yahooMetrics = getRealtimeMetrics('yahoo', $timeframe);
            if ($yahooMetrics && is_array($yahooMetrics)) {
                $defaultStats['yahoo'] = [
                    'name' => 'Yahoo Finance',
                    'total_calls' => intval($yahooMetrics['total_calls'] ?? 0),
                    'success_count' => intval($yahooMetrics['success_count'] ?? 0),
                    'success_rate' => floatval($yahooMetrics['success_rate'] ?? 0),
                    'avg_response_time' => intval($yahooMetrics['avg_response_time'] ?? 0),
                    'errors' => intval($yahooMetrics['error_count'] ?? 0)
                ];
            }
        } catch (Exception $e) {
            error_log("Error getting Yahoo metrics: " . $e->getMessage());
        }
        
        // Merge with custom API stats
        $allStats = array_merge($defaultStats, $stats);
        
        // Ensure all stats have required fields with validation
        foreach ($allStats as $key => &$stat) {
            // Validate and set total_calls
            $stat['total_calls'] = max(0, intval($stat['total_calls'] ?? 0));
            
            // Validate success_count (check both possible field names)
            $successCount = intval($stat['success_count'] ?? $stat['total_success'] ?? 0);
            $stat['success_count'] = max(0, min($stat['total_calls'], $successCount));
            
            // Validate success_rate (must be 0-100)
            $successRate = floatval($stat['success_rate'] ?? 0);
            $stat['success_rate'] = max(0, min(100, round($successRate, 2)));
            
            // Validate avg_response_time (must be non-negative)
            $stat['avg_response_time'] = max(0, intval($stat['avg_response_time'] ?? 0));
            
            // Validate errors (check both possible field names)
            $errors = intval($stat['errors'] ?? $stat['total_errors'] ?? 0);
            $stat['errors'] = max(0, $errors);
            
            // Ensure name exists
            $stat['name'] = $stat['name'] ?? $stat['api_name'] ?? 'Unknown API';
            
            // Set api_id from key if not present
            if (empty($stat['api_id'])) {
                $stat['api_id'] = $key;
            }
        }
        
        echo json_encode([
            'success' => true,
            'stats' => $allStats
        ]);
    } catch (PDOException $e) {
        error_log("Database error in getStatistics: " . $e->getMessage());
        echo json_encode([
            'success' => false, 
            'message' => 'Failed to load statistics: ' . $e->getMessage(),
            'stats' => []
        ]);
    } catch (Exception $e) {
        error_log("Error in getStatistics: " . $e->getMessage());
        echo json_encode([
            'success' => false, 
            'message' => 'Failed to load statistics: ' . $e->getMessage(),
            'stats' => []
        ]);
    }
}

/**
 * Mask API key for display
 */
function maskApiKey($key) {
    if (!$key || strlen($key) < 8) return '••••••••';
    return substr($key, 0, 4) . '••••' . substr($key, -4);
}

?>

