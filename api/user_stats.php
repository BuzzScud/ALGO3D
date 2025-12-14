<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

$stats = [
    'total_symbols' => 0,
    'total_todos' => 0,
    'total_notes' => 0,
    'completed_todos' => 0,
    'account_age' => 0,
    'api_calls_today' => 0
];

if ($conn) {
    try {
        // Total symbols
        $stmt = $conn->query("SELECT COUNT(*) FROM stock_symbols");
        $stats['total_symbols'] = intval($stmt->fetchColumn() ?: 0);
        
        // Total todos
        $stmt = $conn->query("SELECT COUNT(*) FROM todos");
        $stats['total_todos'] = intval($stmt->fetchColumn() ?: 0);
        
        // Completed todos
        $stmt = $conn->query("SELECT COUNT(*) FROM todos WHERE completed = 1");
        $stats['completed_todos'] = intval($stmt->fetchColumn() ?: 0);
        
        // Total notes
        $stmt = $conn->query("SELECT COUNT(*) FROM notes");
        $stats['total_notes'] = intval($stmt->fetchColumn() ?: 0);
        
        // Account age (from user profile creation)
        $stmt = $conn->query("SELECT created_at FROM user_profile ORDER BY id DESC LIMIT 1");
        $createdAt = $stmt->fetchColumn();
        if ($createdAt) {
            $created = new DateTime($createdAt);
            $now = new DateTime();
            $stats['account_age'] = $now->diff($created)->days;
        }
        
        // API calls today (from rate limit file)
        $rateLimitFile = '../cache/rate_limit.json';
        if (file_exists($rateLimitFile)) {
            $data = json_decode(file_get_contents($rateLimitFile), true);
            $currentMinute = floor(time() / 60);
            $todayCalls = 0;
            
            foreach (['finnhub', 'yahoo'] as $api) {
                if (isset($data[$api])) {
                    // Count calls from today (approximate)
                    $todayCalls += isset($data[$api]['count']) ? $data[$api]['count'] : 0;
                }
            }
            $stats['api_calls_today'] = $todayCalls;
        }
        
    } catch (PDOException $e) {
        // Use defaults
    }
}

echo json_encode($stats);
?>







