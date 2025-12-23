<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/database.php';

session_start();

$input = json_decode(file_get_contents('php://input'), true);
$source = $input['source'] ?? '';

$validSources = ['finnhub', 'yahoo'];

// Check if it's a custom API
if (!in_array($source, $validSources)) {
    $db = new Database();
    $conn = $db->getConnection();
    if ($conn) {
        try {
            $stmt = $conn->prepare("SELECT api_id FROM custom_apis WHERE api_id = ? AND is_active = 1");
            $stmt->execute([$source]);
            if ($stmt->fetch()) {
                $validSources[] = $source;
            }
        } catch (Exception $e) {
            // Error checking custom API, continue with validation
        }
    }
}

if (in_array($source, $validSources)) {
    $_SESSION['api_source'] = $source;
    echo json_encode(['success' => true, 'source' => $source]);
} else {
    echo json_encode(['success' => false, 'message' => 'Invalid API source']);
}
?>

















