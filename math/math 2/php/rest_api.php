<?php
/**
 * CLLM REST API Server
 * 
 * Provides HTTP REST API endpoints for CLLM functionality
 * 
 * Endpoints:
 * - POST /api/model/create      - Create a new model
 * - GET  /api/model/info        - Get model information
 * - POST /api/model/save        - Save model to file
 * - POST /api/model/load        - Load model from file
 * - POST /api/vocab/create      - Create vocabulary
 * - POST /api/vocab/build       - Build vocabulary from text
 * - POST /api/vocab/tokenize    - Tokenize text
 * - POST /api/vocab/detokenize  - Detokenize token IDs
 * - POST /api/vocab/save        - Save vocabulary
 * - POST /api/vocab/load        - Load vocabulary
 * - POST /api/training/init     - Initialize training
 * - POST /api/generate          - Generate text from prompt
 * - GET  /api/math/prime/:n     - Get nth prime
 * - GET  /api/math/isprime/:num - Check if number is prime
 * - POST /api/math/exp          - Calculate e^x
 * - POST /api/math/log          - Calculate ln(x)
 * - POST /api/math/sin          - Calculate sin(x)
 * - POST /api/math/cos          - Calculate cos(x)
 * - GET  /api/version           - Get API version
 */

// Check if extension is loaded
if (!extension_loaded('cllm')) {
    http_response_code(500);
    die(json_encode(['error' => 'CLLM extension not loaded']));
}

// Set headers
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle OPTIONS requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Parse request
$method = $_SERVER['REQUEST_METHOD'];
$path = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
$path = preg_replace('#^/api#', '', $path);

// Get request body for POST requests
$input = null;
if ($method === 'POST') {
    $input = json_decode(file_get_contents('php://input'), true);
}

// Session storage for resources (in production, use Redis or database)
session_start();
if (!isset($_SESSION['models'])) {
    $_SESSION['models'] = [];
}
if (!isset($_SESSION['vocabs'])) {
    $_SESSION['vocabs'] = [];
}
if (!isset($_SESSION['trainings'])) {
    $_SESSION['trainings'] = [];
}

// Helper function to send JSON response
function send_json($data, $code = 200) {
    http_response_code($code);
    echo json_encode($data, JSON_PRETTY_PRINT);
    exit;
}

// Helper function to send error
function send_error($message, $code = 400) {
    send_json(['error' => $message], $code);
}

// ============================================================================
// ROUTING
// ============================================================================

// Model endpoints
if ($path === '/model/create' && $method === 'POST') {
    $platonic_solid = $input['platonic_solid'] ?? CLLM_PLATONIC_CUBE;
    $vocab_size = $input['vocab_size'] ?? 1000;
    $options = $input['options'] ?? [];
    
    $model = cllm_create_model($platonic_solid, $vocab_size, $options);
    if (!$model) {
        send_error('Failed to create model', 500);
    }
    
    $model_id = uniqid('model_');
    $_SESSION['models'][$model_id] = $model;
    
    send_json([
        'success' => true,
        'model_id' => $model_id,
        'info' => cllm_model_info($model)
    ]);
}

if ($path === '/model/info' && $method === 'GET') {
    $model_id = $_GET['model_id'] ?? null;
    if (!$model_id || !isset($_SESSION['models'][$model_id])) {
        send_error('Model not found', 404);
    }
    
    $model = $_SESSION['models'][$model_id];
    send_json([
        'success' => true,
        'info' => cllm_model_info($model)
    ]);
}

if ($path === '/model/save' && $method === 'POST') {
    $model_id = $input['model_id'] ?? null;
    $filename = $input['filename'] ?? null;
    
    if (!$model_id || !isset($_SESSION['models'][$model_id])) {
        send_error('Model not found', 404);
    }
    if (!$filename) {
        send_error('Filename required', 400);
    }
    
    $model = $_SESSION['models'][$model_id];
    $result = cllm_save_model($model, $filename);
    
    send_json([
        'success' => $result,
        'filename' => $filename,
        'size' => $result ? filesize($filename) : 0
    ]);
}

if ($path === '/model/load' && $method === 'POST') {
    $filename = $input['filename'] ?? null;
    
    if (!$filename) {
        send_error('Filename required', 400);
    }
    if (!file_exists($filename)) {
        send_error('File not found', 404);
    }
    
    $model = cllm_load_model($filename);
    if (!$model) {
        send_error('Failed to load model', 500);
    }
    
    $model_id = uniqid('model_');
    $_SESSION['models'][$model_id] = $model;
    
    send_json([
        'success' => true,
        'model_id' => $model_id,
        'info' => cllm_model_info($model)
    ]);
}

// Vocabulary endpoints
if ($path === '/vocab/create' && $method === 'POST') {
    $capacity = $input['capacity'] ?? 10000;
    
    $vocab = cllm_vocab_create($capacity);
    if (!$vocab) {
        send_error('Failed to create vocabulary', 500);
    }
    
    $vocab_id = uniqid('vocab_');
    $_SESSION['vocabs'][$vocab_id] = $vocab;
    
    send_json([
        'success' => true,
        'vocab_id' => $vocab_id,
        'stats' => cllm_vocab_stats($vocab)
    ]);
}

if ($path === '/vocab/build' && $method === 'POST') {
    $vocab_id = $input['vocab_id'] ?? null;
    $text = $input['text'] ?? null;
    $filename = $input['filename'] ?? null;
    
    if (!$vocab_id || !isset($_SESSION['vocabs'][$vocab_id])) {
        send_error('Vocabulary not found', 404);
    }
    
    $vocab = $_SESSION['vocabs'][$vocab_id];
    
    if ($filename) {
        // Build from file
        if (!file_exists($filename)) {
            send_error('File not found', 404);
        }
        $tokens_added = cllm_vocab_build_from_file($vocab, $filename);
    } elseif ($text) {
        // Build from text (save to temp file first)
        $temp_file = tempnam(sys_get_temp_dir(), 'cllm_vocab_');
        file_put_contents($temp_file, $text);
        $tokens_added = cllm_vocab_build_from_file($vocab, $temp_file);
        unlink($temp_file);
    } else {
        send_error('Either text or filename required', 400);
    }
    
    send_json([
        'success' => true,
        'tokens_added' => $tokens_added,
        'stats' => cllm_vocab_stats($vocab)
    ]);
}

if ($path === '/vocab/tokenize' && $method === 'POST') {
    $vocab_id = $input['vocab_id'] ?? null;
    $text = $input['text'] ?? null;
    
    if (!$vocab_id || !isset($_SESSION['vocabs'][$vocab_id])) {
        send_error('Vocabulary not found', 404);
    }
    if (!$text) {
        send_error('Text required', 400);
    }
    
    $vocab = $_SESSION['vocabs'][$vocab_id];
    $tokens = cllm_vocab_tokenize($vocab, $text);
    
    send_json([
        'success' => true,
        'text' => $text,
        'tokens' => $tokens,
        'num_tokens' => count($tokens)
    ]);
}

if ($path === '/vocab/detokenize' && $method === 'POST') {
    $vocab_id = $input['vocab_id'] ?? null;
    $tokens = $input['tokens'] ?? null;
    
    if (!$vocab_id || !isset($_SESSION['vocabs'][$vocab_id])) {
        send_error('Vocabulary not found', 404);
    }
    if (!$tokens || !is_array($tokens)) {
        send_error('Tokens array required', 400);
    }
    
    $vocab = $_SESSION['vocabs'][$vocab_id];
    $text = cllm_vocab_detokenize($vocab, $tokens);
    
    send_json([
        'success' => true,
        'tokens' => $tokens,
        'text' => $text
    ]);
}

if ($path === '/vocab/save' && $method === 'POST') {
    $vocab_id = $input['vocab_id'] ?? null;
    $filename = $input['filename'] ?? null;
    
    if (!$vocab_id || !isset($_SESSION['vocabs'][$vocab_id])) {
        send_error('Vocabulary not found', 404);
    }
    if (!$filename) {
        send_error('Filename required', 400);
    }
    
    $vocab = $_SESSION['vocabs'][$vocab_id];
    $result = cllm_vocab_save($vocab, $filename);
    
    send_json([
        'success' => $result,
        'filename' => $filename,
        'size' => $result ? filesize($filename) : 0
    ]);
}

if ($path === '/vocab/load' && $method === 'POST') {
    $filename = $input['filename'] ?? null;
    
    if (!$filename) {
        send_error('Filename required', 400);
    }
    if (!file_exists($filename)) {
        send_error('File not found', 404);
    }
    
    $vocab = cllm_vocab_load($filename);
    if (!$vocab) {
        send_error('Failed to load vocabulary', 500);
    }
    
    $vocab_id = uniqid('vocab_');
    $_SESSION['vocabs'][$vocab_id] = $vocab;
    
    send_json([
        'success' => true,
        'vocab_id' => $vocab_id,
        'stats' => cllm_vocab_stats($vocab)
    ]);
}

// Training endpoints
if ($path === '/training/init' && $method === 'POST') {
    $model_id = $input['model_id'] ?? null;
    $config = $input['config'] ?? [];
    
    if (!$model_id || !isset($_SESSION['models'][$model_id])) {
        send_error('Model not found', 404);
    }
    
    $model = $_SESSION['models'][$model_id];
    $training = cllm_training_init($model, $config);
    
    if (!$training) {
        send_error('Failed to initialize training', 500);
    }
    
    $training_id = uniqid('training_');
    $_SESSION['trainings'][$training_id] = $training;
    
    send_json([
        'success' => true,
        'training_id' => $training_id,
        'config' => $config
    ]);
}

// Generation endpoint
if ($path === '/generate' && $method === 'POST') {
    $model_id = $input['model_id'] ?? null;
    $prompt = $input['prompt'] ?? null;
    $options = $input['options'] ?? [];
    
    if (!$model_id || !isset($_SESSION['models'][$model_id])) {
        send_error('Model not found', 404);
    }
    if (!$prompt) {
        send_error('Prompt required', 400);
    }
    
    $model = $_SESSION['models'][$model_id];
    $generated = cllm_generate($model, $prompt, $options);
    
    send_json([
        'success' => true,
        'prompt' => $prompt,
        'generated' => $generated,
        'options' => $options
    ]);
}

// Math endpoints
if (preg_match('#^/math/prime/(\d+)$#', $path, $matches) && $method === 'GET') {
    $n = (int)$matches[1];
    $prime = cllm_prime_nth($n);
    
    send_json([
        'success' => true,
        'n' => $n,
        'prime' => $prime,
        'is_prime' => cllm_prime_is_prime($prime)
    ]);
}

if (preg_match('#^/math/isprime/(\d+)$#', $path, $matches) && $method === 'GET') {
    $num = (int)$matches[1];
    $is_prime = cllm_prime_is_prime($num);
    
    send_json([
        'success' => true,
        'number' => $num,
        'is_prime' => $is_prime
    ]);
}

if ($path === '/math/exp' && $method === 'POST') {
    $x = $input['x'] ?? null;
    if ($x === null) {
        send_error('Parameter x required', 400);
    }
    
    $result = cllm_math_exp((float)$x);
    send_json([
        'success' => true,
        'x' => $x,
        'result' => $result
    ]);
}

if ($path === '/math/log' && $method === 'POST') {
    $x = $input['x'] ?? null;
    if ($x === null) {
        send_error('Parameter x required', 400);
    }
    if ($x <= 0) {
        send_error('Parameter x must be positive', 400);
    }
    
    $result = cllm_math_log((float)$x);
    send_json([
        'success' => true,
        'x' => $x,
        'result' => $result
    ]);
}

if ($path === '/math/sin' && $method === 'POST') {
    $x = $input['x'] ?? null;
    if ($x === null) {
        send_error('Parameter x required', 400);
    }
    
    $result = cllm_math_sin((float)$x);
    send_json([
        'success' => true,
        'x' => $x,
        'result' => $result
    ]);
}

if ($path === '/math/cos' && $method === 'POST') {
    $x = $input['x'] ?? null;
    if ($x === null) {
        send_error('Parameter x required', 400);
    }
    
    $result = cllm_math_cos((float)$x);
    send_json([
        'success' => true,
        'x' => $x,
        'result' => $result
    ]);
}

// Version endpoint
if ($path === '/version' && $method === 'GET') {
    send_json([
        'success' => true,
        'version' => cllm_version(),
        'php_version' => PHP_VERSION,
        'api_version' => '1.0.0'
    ]);
}

// Health check endpoint
if ($path === '/health' && $method === 'GET') {
    send_json([
        'success' => true,
        'status' => 'healthy',
        'extension_loaded' => extension_loaded('cllm'),
        'timestamp' => time()
    ]);
}

// List all endpoints
if ($path === '/' || $path === '/endpoints' && $method === 'GET') {
    send_json([
        'success' => true,
        'api' => 'CLLM REST API',
        'version' => '1.0.0',
        'endpoints' => [
            'model' => [
                'POST /api/model/create' => 'Create a new model',
                'GET  /api/model/info?model_id=xxx' => 'Get model information',
                'POST /api/model/save' => 'Save model to file',
                'POST /api/model/load' => 'Load model from file'
            ],
            'vocabulary' => [
                'POST /api/vocab/create' => 'Create vocabulary',
                'POST /api/vocab/build' => 'Build vocabulary from text',
                'POST /api/vocab/tokenize' => 'Tokenize text',
                'POST /api/vocab/detokenize' => 'Detokenize token IDs',
                'POST /api/vocab/save' => 'Save vocabulary',
                'POST /api/vocab/load' => 'Load vocabulary'
            ],
            'training' => [
                'POST /api/training/init' => 'Initialize training'
            ],
            'inference' => [
                'POST /api/generate' => 'Generate text from prompt'
            ],
            'math' => [
                'GET  /api/math/prime/:n' => 'Get nth prime',
                'GET  /api/math/isprime/:num' => 'Check if number is prime',
                'POST /api/math/exp' => 'Calculate e^x',
                'POST /api/math/log' => 'Calculate ln(x)',
                'POST /api/math/sin' => 'Calculate sin(x)',
                'POST /api/math/cos' => 'Calculate cos(x)'
            ],
            'utility' => [
                'GET  /api/version' => 'Get API version',
                'GET  /api/health' => 'Health check'
            ]
        ]
    ]);
}

// 404 - Endpoint not found
send_error('Endpoint not found: ' . $path, 404);
?>