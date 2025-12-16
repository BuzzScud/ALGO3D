<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

$stats = [
    'symbols' => 0,
    'todos' => 0,
    'notes' => 0,
    'api_status' => 'OK'
];

if ($conn) {
    try {
        // Count symbols
        $stmt = $conn->query("SELECT COUNT(*) FROM stock_symbols");
        $stats['symbols'] = $stmt->fetchColumn() ?: 4; // Default 4 symbols
        
        // Count active todos
        $stmt = $conn->query("SELECT COUNT(*) FROM todos WHERE completed = 0");
        $stats['todos'] = $stmt->fetchColumn() ?: 0;
        
        // Count notes
        $stmt = $conn->query("SELECT COUNT(*) FROM notes");
        $stats['notes'] = $stmt->fetchColumn() ?: 0;
        
    } catch (PDOException $e) {
        // Use defaults
    }
}

// Check API status
$rateLimitFile = '../cache/rate_limit.json';
if (file_exists($rateLimitFile)) {
    $data = json_decode(file_get_contents($rateLimitFile), true);
    $currentMinute = floor(time() / 60);
    
    $finnhubUsed = 0;
    if (isset($data['finnhub']) && $data['finnhub']['minute'] === $currentMinute) {
        $finnhubUsed = $data['finnhub']['count'];
    }
    
    if ($finnhubUsed >= 55) {
        $stats['api_status'] = 'Limited';
    }
}

echo json_encode($stats);
?>










