<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

// Base directory for thesis files (relative to this script)
$baseDir = dirname(__DIR__);

// Get the action from query parameter
$action = $_GET['action'] ?? 'structure';

/**
 * Recursively scan directory and build file structure with relative paths
 */
function scanDirectory($dir, $relativePath = '') {
    $result = [];
    
    if (!is_dir($dir)) {
        return $result;
    }
    
    $items = scandir($dir);
    
    foreach ($items as $item) {
        if ($item === '.' || $item === '..') {
            continue;
        }
        
        $fullPath = $dir . '/' . $item;
        $itemRelativePath = $relativePath ? $relativePath . '/' . $item : $item;
        
        if (is_dir($fullPath)) {
            // Skip certain directories
            if (in_array($item, ['api', '.git', 'node_modules'])) {
                continue;
            }
            
            $result[] = [
                'name' => $item,
                'type' => 'directory',
                'path' => $itemRelativePath,
                'children' => scanDirectory($fullPath, $itemRelativePath)
            ];
        } elseif (pathinfo($item, PATHINFO_EXTENSION) === 'md') {
            $result[] = [
                'name' => $item,
                'type' => 'file',
                'path' => $itemRelativePath,
                'size' => filesize($fullPath)
            ];
        }
    }
    
    // Sort: directories first, then files, both alphabetically
    usort($result, function($a, $b) {
        if ($a['type'] !== $b['type']) {
            return $a['type'] === 'directory' ? -1 : 1;
        }
        return strcasecmp($a['name'], $b['name']);
    });
    
    return $result;
}

/**
 * Search for files containing query
 */
function searchFiles($query) {
    $results = [];
    $baseDir = $GLOBALS['baseDir'];
    
    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($baseDir, RecursiveDirectoryIterator::SKIP_DOTS),
        RecursiveIteratorIterator::SELF_FIRST
    );
    
    foreach ($iterator as $file) {
        if ($file->isFile() && $file->getExtension() === 'md') {
            $content = file_get_contents($file->getPathname());
            
            if (stripos($content, $query) !== false) {
                $relativePath = str_replace($baseDir . '/', '', $file->getPathname());
                
                // Find context around match
                $lines = explode("\n", $content);
                $matchingLines = [];
                
                foreach ($lines as $lineNum => $line) {
                    if (stripos($line, $query) !== false) {
                        $start = max(0, $lineNum - 2);
                        $end = min(count($lines) - 1, $lineNum + 2);
                        
                        $context = [];
                        for ($i = $start; $i <= $end; $i++) {
                            $context[] = [
                                'line' => $i + 1,
                                'text' => $lines[$i],
                                'match' => $i === $lineNum
                            ];
                        }
                        
                        $matchingLines[] = $context;
                        
                        // Limit to 3 matches per file
                        if (count($matchingLines) >= 3) {
                            break;
                        }
                    }
                }
                
                $results[] = [
                    'path' => $relativePath,
                    'name' => $file->getFilename(),
                    'matches' => $matchingLines,
                    'size' => $file->getSize()
                ];
            }
        }
    }
    
    return $results;
}

/**
 * Get statistics
 */
function getStatistics() {
    $baseDir = $GLOBALS['baseDir'];
    $stats = [
        'total_files' => 0,
        'total_lines' => 0,
        'total_size' => 0,
        'chapters' => 0,
        'parts' => 0
    ];
    
    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($baseDir, RecursiveDirectoryIterator::SKIP_DOTS),
        RecursiveIteratorIterator::SELF_FIRST
    );
    
    foreach ($iterator as $file) {
        if ($file->isFile() && $file->getExtension() === 'md') {
            $stats['total_files']++;
            $stats['total_size'] += $file->getSize();
            
            $content = file_get_contents($file->getPathname());
            $stats['total_lines'] += substr_count($content, "\n");
            
            $relativePath = str_replace($baseDir . '/', '', $file->getPathname());
            
            if (strpos($relativePath, 'part_') === 0) {
                $stats['parts']++;
            }
            
            if (strpos($relativePath, 'chapter_') !== false) {
                $stats['chapters']++;
            }
        }
    }
    
    return $stats;
}

// Handle different actions
try {
    switch ($action) {
        case 'structure':
            $structure = scanDirectory($baseDir);
            echo json_encode([
                'success' => true,
                'structure' => $structure
            ], JSON_PRETTY_PRINT);
            break;
            
        case 'search':
            $query = $_GET['query'] ?? '';
            if (empty($query)) {
                throw new Exception('Search query is required');
            }
            
            $results = searchFiles($query);
            echo json_encode([
                'success' => true,
                'query' => $query,
                'results' => $results,
                'count' => count($results)
            ], JSON_PRETTY_PRINT);
            break;
            
        case 'stats':
            $stats = getStatistics();
            echo json_encode([
                'success' => true,
                'statistics' => $stats
            ], JSON_PRETTY_PRINT);
            break;
            
        default:
            throw new Exception('Invalid action');
    }
} catch (Exception $e) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ], JSON_PRETTY_PRINT);
}