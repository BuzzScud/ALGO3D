<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['success' => false, 'message' => 'Database connection failed']);
    exit;
}

// Create settings table if it doesn't exist
try {
    $conn->exec("
        CREATE TABLE IF NOT EXISTS settings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            setting_key TEXT NOT NULL UNIQUE,
            setting_value TEXT,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ");
} catch (PDOException $e) {
    // Table might already exist
}

$method = $_SERVER['REQUEST_METHOD'];

if ($method !== 'POST') {
    echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    exit;
}

$input = json_decode(file_get_contents('php://input'), true);

if (!isset($input['api_id']) || !isset($input['api_key'])) {
    echo json_encode(['success' => false, 'message' => 'Missing required fields']);
    exit;
}

$apiId = $input['api_id'];
$apiKey = trim($input['api_key']);

// Validate API ID
if ($apiId !== 'finnhub') {
    echo json_encode(['success' => false, 'message' => 'Invalid API ID']);
    exit;
}

// Validate API key format (basic validation)
if (empty($apiKey) || strlen($apiKey) < 10) {
    echo json_encode(['success' => false, 'message' => 'Invalid API key format']);
    exit;
}

try {
    // Save to database
    $stmt = $conn->prepare("
        INSERT INTO settings (setting_key, setting_value) VALUES (?, ?)
        ON CONFLICT(setting_key) DO UPDATE SET setting_value = ?, updated_at = CURRENT_TIMESTAMP
    ");
    $stmt->execute(['finnhub_key', $apiKey, $apiKey]);
    
    // Update config.php file
    $configFile = __DIR__ . '/../includes/config.php';
    
    if (!file_exists($configFile) || !is_writable($configFile)) {
        // Config file not writable, but database was updated
        echo json_encode([
            'success' => true, 
            'message' => 'API key saved to database. Config file is not writable - please update manually.',
            'api_id' => $apiId,
            'warning' => true
        ]);
        exit;
    }
    
    $configContent = file_get_contents($configFile);
    
    // Escape the API key for use in PHP string
    $escapedApiKey = addslashes($apiKey);
    
    // Replace the FINNHUB_API_KEY definition
    // Pattern matches: define('FINNHUB_API_KEY', ...) or define("FINNHUB_API_KEY", ...)
    // Handles various whitespace and quote styles
    $pattern = "/define\s*\(\s*['\"]FINNHUB_API_KEY['\"]\s*,\s*[^)]+\)\s*;/";
    $replacement = "define('FINNHUB_API_KEY', getenv('FINNHUB_API_KEY') ?: '$escapedApiKey');";
    
    $newConfigContent = preg_replace($pattern, $replacement, $configContent);
    
    if ($newConfigContent !== null && $newConfigContent !== $configContent) {
        // Backup original config
        $backupFile = $configFile . '.backup.' . time();
        if (file_put_contents($backupFile, $configContent) !== false) {
            // Write new config
            if (file_put_contents($configFile, $newConfigContent) !== false) {
                echo json_encode([
                    'success' => true, 
                    'message' => 'API key updated successfully',
                    'api_id' => $apiId
                ]);
            } else {
                // Config update failed but database was updated
                echo json_encode([
                    'success' => true, 
                    'message' => 'API key saved to database. Config file update failed - please update manually.',
                    'api_id' => $apiId,
                    'warning' => true
                ]);
            }
        } else {
            // Backup failed, but try to update anyway
            if (file_put_contents($configFile, $newConfigContent) !== false) {
                echo json_encode([
                    'success' => true, 
                    'message' => 'API key updated successfully (backup failed)',
                    'api_id' => $apiId
                ]);
            } else {
                echo json_encode([
                    'success' => true, 
                    'message' => 'API key saved to database. Config file update failed - please update manually.',
                    'api_id' => $apiId,
                    'warning' => true
                ]);
            }
        }
    } else {
        // Pattern didn't match, but database was updated
        echo json_encode([
            'success' => true, 
            'message' => 'API key saved to database. Config file format may have changed - please update manually.',
            'api_id' => $apiId,
            'warning' => true
        ]);
    }
    
} catch (PDOException $e) {
    error_log("API key update error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    error_log("API key update error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Error updating API key: ' . $e->getMessage()]);
}
?>

