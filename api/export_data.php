<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

$exportData = [
    'exported_at' => date('Y-m-d H:i:s'),
    'todos' => [],
    'notes' => [],
    'symbols' => [],
    'settings' => []
];

if ($conn) {
    try {
        // Export todos
        $stmt = $conn->query("SELECT * FROM todos ORDER BY created_at DESC");
        $exportData['todos'] = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Export notes
        $stmt = $conn->query("SELECT * FROM notes ORDER BY updated_at DESC");
        $exportData['notes'] = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Export symbols
        $stmt = $conn->query("SELECT * FROM stock_symbols ORDER BY created_at DESC");
        $exportData['symbols'] = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Export settings
        $stmt = $conn->query("SELECT setting_key, setting_value FROM settings");
        $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
        foreach ($rows as $row) {
            $exportData['settings'][$row['setting_key']] = $row['setting_value'];
        }
        
    } catch (PDOException $e) {
        // Continue with whatever we have
    }
}

echo json_encode($exportData, JSON_PRETTY_PRINT);
?>














