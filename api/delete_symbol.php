<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$input = json_decode(file_get_contents('php://input'), true);
$symbol = strtoupper(trim($input['symbol'] ?? ''));

if (empty($symbol)) {
    echo json_encode(['success' => false, 'message' => 'Symbol is required']);
    exit;
}

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['success' => false, 'message' => 'Database connection failed']);
    exit;
}

try {
    $stmt = $conn->prepare("DELETE FROM stock_symbols WHERE symbol = ?");
    $stmt->execute([$symbol]);
    echo json_encode(['success' => true, 'message' => 'Symbol deleted successfully']);
} catch(PDOException $e) {
    error_log("Delete symbol error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Error deleting symbol']);
}
?>
