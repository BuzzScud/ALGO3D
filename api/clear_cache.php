<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$cacheDir = '../cache';
$count = 0;

if (is_dir($cacheDir)) {
    $files = glob($cacheDir . '/*.json');
    foreach ($files as $file) {
        if (is_file($file)) {
            unlink($file);
            $count++;
        }
    }
}

echo json_encode([
    'success' => true,
    'message' => "Cleared $count cached files"
]);
?>











