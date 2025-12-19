<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$input = json_decode(file_get_contents('php://input'), true);
$confirm = $input['confirm'] ?? false;

if (!$confirm) {
    echo json_encode(['success' => false, 'message' => 'Confirmation required']);
    exit;
}

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['success' => false, 'message' => 'Database connection failed']);
    exit;
}

try {
    // Delete all data from tables
    $conn->exec("DELETE FROM todos");
    $conn->exec("DELETE FROM notes");
    $conn->exec("DELETE FROM stock_symbols");
    $conn->exec("DELETE FROM settings");
    
    // Clear cache
    $cacheDir = '../cache';
    if (is_dir($cacheDir)) {
        $files = glob($cacheDir . '/*.json');
        foreach ($files as $file) {
            if (is_file($file)) {
                unlink($file);
            }
        }
    }
    
    echo json_encode(['success' => true, 'message' => 'All data has been reset']);
} catch (PDOException $e) {
    error_log("Reset data error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Error resetting data']);
}
?>














