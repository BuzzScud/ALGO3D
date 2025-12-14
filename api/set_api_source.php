<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

session_start();

$input = json_decode(file_get_contents('php://input'), true);
$source = $input['source'] ?? '';

$validSources = ['finnhub', 'yahoo'];

if (in_array($source, $validSources)) {
    $_SESSION['api_source'] = $source;
    echo json_encode(['success' => true, 'source' => $source]);
} else {
    echo json_encode(['success' => false, 'message' => 'Invalid API source']);
}
?>







