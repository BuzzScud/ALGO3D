<?php
/**
 * Setup Verification Script
 * Run this file once to verify your installation
 */

require_once 'includes/config.php';
require_once 'includes/database.php';

$errors = [];
$warnings = [];
$success = [];

// Check PHP version
if (version_compare(PHP_VERSION, '7.4.0', '<')) {
    $errors[] = "PHP version 7.4 or higher is required. Current version: " . PHP_VERSION;
} else {
    $success[] = "PHP version: " . PHP_VERSION . " ✓";
}

// Check required extensions
$required_extensions = ['pdo', 'pdo_mysql', 'curl', 'json'];
foreach ($required_extensions as $ext) {
    if (!extension_loaded($ext)) {
        $errors[] = "Required PHP extension missing: $ext";
    } else {
        $success[] = "PHP extension '$ext' is loaded ✓";
    }
}

// Check database connection
try {
    $db = new Database();
    $conn = $db->getConnection();
    if ($conn) {
        $success[] = "Database connection successful ✓";
        
        // Check if tables exist
        $tables = ['stock_symbols', 'todos'];
        foreach ($tables as $table) {
            $stmt = $conn->query("SHOW TABLES LIKE '$table'");
            if ($stmt->rowCount() > 0) {
                $success[] = "Table '$table' exists ✓";
            } else {
                $warnings[] = "Table '$table' does not exist (will be created automatically)";
            }
        }
    }
} catch (Exception $e) {
    $errors[] = "Database connection failed: " . $e->getMessage();
}

// Check cache directory
$cacheDir = __DIR__ . '/cache';
if (!is_dir($cacheDir)) {
    if (mkdir($cacheDir, 0777, true)) {
        $success[] = "Cache directory created ✓";
    } else {
        $errors[] = "Failed to create cache directory";
    }
} else {
    if (is_writable($cacheDir)) {
        $success[] = "Cache directory is writable ✓";
    } else {
        $warnings[] = "Cache directory is not writable. Run: chmod 777 cache";
    }
}

// Check API key
if (ALPHA_VANTAGE_API_KEY === 'demo') {
    $warnings[] = "Alpha Vantage API key is set to 'demo'. Update it in includes/config.php for real data.";
} else {
    $success[] = "Alpha Vantage API key is configured ✓";
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ALGO3D - Setup Verification</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: #0f172a;
            color: #f1f5f9;
        }
        h1 { color: #2563eb; }
        .success { color: #10b981; padding: 5px 0; }
        .warning { color: #f59e0b; padding: 5px 0; }
        .error { color: #ef4444; padding: 5px 0; }
        .section { margin: 20px 0; padding: 15px; background: #1e293b; border-radius: 8px; }
    </style>
</head>
<body>
    <h1>ALGO3D Setup Verification</h1>
    
    <?php if (!empty($success)): ?>
    <div class="section">
        <h2>Success</h2>
        <?php foreach ($success as $msg): ?>
            <div class="success"><?php echo htmlspecialchars($msg); ?></div>
        <?php endforeach; ?>
    </div>
    <?php endif; ?>
    
    <?php if (!empty($warnings)): ?>
    <div class="section">
        <h2>Warnings</h2>
        <?php foreach ($warnings as $msg): ?>
            <div class="warning"><?php echo htmlspecialchars($msg); ?></div>
        <?php endforeach; ?>
    </div>
    <?php endif; ?>
    
    <?php if (!empty($errors)): ?>
    <div class="section">
        <h2>Errors</h2>
        <?php foreach ($errors as $msg): ?>
            <div class="error"><?php echo htmlspecialchars($msg); ?></div>
        <?php endforeach; ?>
    </div>
    <?php endif; ?>
    
    <?php if (empty($errors)): ?>
    <div class="section">
        <h2>Setup Complete!</h2>
        <p>Your installation looks good. You can now <a href="index.php" style="color: #2563eb;">access the dashboard</a>.</p>
    </div>
    <?php else: ?>
    <div class="section">
        <h2>Please Fix Errors</h2>
        <p>Please resolve the errors above before using the application.</p>
    </div>
    <?php endif; ?>
</body>
</html>

















