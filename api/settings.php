<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['error' => 'Database connection failed']);
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

try {
    switch ($method) {
        case 'GET':
            $stmt = $conn->prepare("SELECT setting_key, setting_value FROM settings");
            $stmt->execute();
            $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
            
            $settings = [];
            foreach ($rows as $row) {
                $settings[$row['setting_key']] = $row['setting_value'];
            }
            
            // Return defaults if not set
            $defaults = [
                'finnhub_key' => FINNHUB_API_KEY,
                'default_api' => DEFAULT_API_SOURCE,
                'refresh_interval' => '60',
                'theme' => 'dark',
                'show_ticker' => 'true',
                'compact_cards' => 'false',
                'notifications' => 'false',
                'sound_alerts' => 'false'
            ];
            
            foreach ($defaults as $key => $value) {
                if (!isset($settings[$key])) {
                    $settings[$key] = $value;
                }
            }
            
            echo json_encode($settings);
            break;
            
        case 'POST':
            $input = json_decode(file_get_contents('php://input'), true);
            
            if (!is_array($input)) {
                echo json_encode(['success' => false, 'message' => 'Invalid input']);
                exit;
            }
            
            foreach ($input as $key => $value) {
                $stmt = $conn->prepare("
                    INSERT INTO settings (setting_key, setting_value) VALUES (?, ?)
                    ON CONFLICT(setting_key) DO UPDATE SET setting_value = ?, updated_at = CURRENT_TIMESTAMP
                ");
                $stmt->execute([$key, $value, $value]);
            }
            
            echo json_encode(['success' => true, 'message' => 'Settings saved successfully']);
            break;
            
        default:
            echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    }
} catch (PDOException $e) {
    error_log("Settings API error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error']);
}
?>






