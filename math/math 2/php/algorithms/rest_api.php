<?php
/**
 * Algorithms REST API
 * 
 * Provides HTTP endpoints for the Algorithms library
 */

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle OPTIONS request for CORS
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

// Check if extension is loaded
if (!extension_loaded('algorithms')) {
    http_response_code(500);
    echo json_encode([
        'error' => 'Algorithms extension not loaded',
        'message' => 'Please install and enable the algorithms PHP extension'
    ]);
    exit();
}

// Get request path
$request_uri = $_SERVER['REQUEST_URI'];
$script_name = dirname($_SERVER['SCRIPT_NAME']);
$path = str_replace($script_name, '', $request_uri);
$path = parse_url($path, PHP_URL_PATH);
$path = trim($path, '/');

// Get request method
$method = $_SERVER['REQUEST_METHOD'];

// Get request body
$input = file_get_contents('php://input');
$data = json_decode($input, true) ?? [];

/**
 * Send JSON response
 */
function send_response($data, $code = 200) {
    http_response_code($code);
    echo json_encode($data, JSON_PRETTY_PRINT);
    exit();
}

/**
 * Send error response
 */
function send_error($message, $code = 400) {
    send_response(['error' => $message], $code);
}

/**
 * Validate required parameters
 */
function require_params($data, $required) {
    foreach ($required as $param) {
        if (!isset($data[$param])) {
            send_error("Missing required parameter: $param");
        }
    }
}

// Route handling
switch ($path) {
    case '':
    case 'api/algo':
        // API documentation
        send_response([
            'name' => 'Algorithms API',
            'version' => '1.0.0',
            'endpoints' => [
                'GET  /api/algo' => 'API documentation',
                'POST /api/algo/statistics' => 'Calculate statistics',
                'POST /api/algo/mean' => 'Calculate mean',
                'POST /api/algo/variance' => 'Calculate variance',
                'POST /api/algo/std_dev' => 'Calculate standard deviation',
                'POST /api/algo/dot_product' => 'Calculate dot product',
                'POST /api/algo/vector_norm' => 'Calculate vector norm',
                'POST /api/algo/normalize' => 'Normalize vector',
                'POST /api/algo/cosine_similarity' => 'Calculate cosine similarity',
                'POST /api/algo/softmax' => 'Apply softmax function',
                'POST /api/algo/cross_entropy' => 'Calculate cross-entropy loss',
            ]
        ]);
        break;
    
    case 'api/algo/statistics':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['data']);
        
        if (!is_array($data['data'])) {
            send_error('Data must be an array');
        }
        
        $result = algo_statistics($data['data']);
        send_response([
            'operation' => 'statistics',
            'input_count' => count($data['data']),
            'result' => $result
        ]);
        break;
    
    case 'api/algo/mean':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['data']);
        
        if (!is_array($data['data'])) {
            send_error('Data must be an array');
        }
        
        $result = algo_mean($data['data']);
        send_response([
            'operation' => 'mean',
            'input_count' => count($data['data']),
            'result' => $result
        ]);
        break;
    
    case 'api/algo/variance':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['data']);
        
        if (!is_array($data['data'])) {
            send_error('Data must be an array');
        }
        
        $result = algo_variance($data['data']);
        send_response([
            'operation' => 'variance',
            'input_count' => count($data['data']),
            'result' => $result
        ]);
        break;
    
    case 'api/algo/std_dev':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['data']);
        
        if (!is_array($data['data'])) {
            send_error('Data must be an array');
        }
        
        $result = algo_std_dev($data['data']);
        send_response([
            'operation' => 'std_dev',
            'input_count' => count($data['data']),
            'result' => $result
        ]);
        break;
    
    case 'api/algo/dot_product':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['a', 'b']);
        
        if (!is_array($data['a']) || !is_array($data['b'])) {
            send_error('Both a and b must be arrays');
        }
        
        if (count($data['a']) !== count($data['b'])) {
            send_error('Arrays must have the same length');
        }
        
        $result = algo_dot_product($data['a'], $data['b']);
        send_response([
            'operation' => 'dot_product',
            'dimension' => count($data['a']),
            'result' => $result
        ]);
        break;
    
    case 'api/algo/vector_norm':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['vector']);
        
        if (!is_array($data['vector'])) {
            send_error('Vector must be an array');
        }
        
        $result = algo_vector_norm($data['vector']);
        send_response([
            'operation' => 'vector_norm',
            'dimension' => count($data['vector']),
            'result' => $result
        ]);
        break;
    
    case 'api/algo/normalize':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['vector']);
        
        if (!is_array($data['vector'])) {
            send_error('Vector must be an array');
        }
        
        $result = algo_normalize_vector($data['vector']);
        send_response([
            'operation' => 'normalize_vector',
            'input' => $data['vector'],
            'result' => $result,
            'norm' => algo_vector_norm($result)
        ]);
        break;
    
    case 'api/algo/cosine_similarity':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['a', 'b']);
        
        if (!is_array($data['a']) || !is_array($data['b'])) {
            send_error('Both a and b must be arrays');
        }
        
        if (count($data['a']) !== count($data['b'])) {
            send_error('Arrays must have the same length');
        }
        
        $result = algo_cosine_similarity($data['a'], $data['b']);
        send_response([
            'operation' => 'cosine_similarity',
            'dimension' => count($data['a']),
            'result' => $result
        ]);
        break;
    
    case 'api/algo/softmax':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['logits']);
        
        if (!is_array($data['logits'])) {
            send_error('Logits must be an array');
        }
        
        $result = algo_softmax($data['logits']);
        send_response([
            'operation' => 'softmax',
            'input' => $data['logits'],
            'result' => $result,
            'sum' => array_sum($result)
        ]);
        break;
    
    case 'api/algo/cross_entropy':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['predictions', 'targets']);
        
        if (!is_array($data['predictions']) || !is_array($data['targets'])) {
            send_error('Both predictions and targets must be arrays');
        }
        
        if (count($data['predictions']) !== count($data['targets'])) {
            send_error('Arrays must have the same length');
        }
        
        $result = algo_cross_entropy($data['predictions'], $data['targets']);
        send_response([
            'operation' => 'cross_entropy',
            'dimension' => count($data['predictions']),
            'result' => $result
        ]);
        break;
    
    default:
        send_error('Endpoint not found', 404);
}