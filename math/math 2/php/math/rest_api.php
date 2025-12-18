<?php
/**
 * Crystalline Math REST API
 * 
 * Provides HTTP endpoints for the Crystalline Math library
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
if (!extension_loaded('crystalline_math')) {
    http_response_code(500);
    echo json_encode([
        'error' => 'Crystalline Math extension not loaded',
        'message' => 'Please install and enable the crystalline_math PHP extension'
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

// Get query parameters
$params = $_GET;

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
    case 'api/math':
        // API documentation
        send_response([
            'name' => 'Crystalline Math API',
            'version' => '1.0.0',
            'endpoints' => [
                'GET  /api/math' => 'API documentation',
                'POST /api/math/gcd' => 'Calculate GCD of two numbers',
                'POST /api/math/prime/check' => 'Check if number is prime',
                'POST /api/math/prime/next' => 'Get next prime number',
                'POST /api/math/prime/factors' => 'Get prime factorization',
                'POST /api/math/prime/sieve' => 'Generate primes up to limit',
                'POST /api/math/sqrt' => 'Calculate square root',
                'POST /api/math/pow' => 'Calculate power',
                'POST /api/math/sin' => 'Calculate sine',
                'POST /api/math/cos' => 'Calculate cosine',
                'POST /api/math/log' => 'Calculate natural logarithm',
                'POST /api/math/exp' => 'Calculate exponential',
            ]
        ]);
        break;
    
    case 'api/math/gcd':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['a', 'b']);
        
        $a = intval($data['a']);
        $b = intval($data['b']);
        
        $result = crystalline_gcd($a, $b);
        send_response([
            'operation' => 'gcd',
            'inputs' => ['a' => $a, 'b' => $b],
            'result' => $result
        ]);
        break;
    
    case 'api/math/prime/check':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['n']);
        
        $n = intval($data['n']);
        $result = crystalline_is_prime($n);
        
        send_response([
            'operation' => 'is_prime',
            'input' => $n,
            'result' => $result,
            'message' => $result ? "$n is prime" : "$n is not prime"
        ]);
        break;
    
    case 'api/math/prime/next':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['n']);
        
        $n = intval($data['n']);
        $result = crystalline_next_prime($n);
        
        send_response([
            'operation' => 'next_prime',
            'input' => $n,
            'result' => $result
        ]);
        break;
    
    case 'api/math/prime/factors':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['n']);
        
        $n = intval($data['n']);
        $result = crystalline_prime_factors($n);
        
        send_response([
            'operation' => 'prime_factors',
            'input' => $n,
            'result' => $result,
            'count' => count($result)
        ]);
        break;
    
    case 'api/math/prime/sieve':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['limit']);
        
        $limit = intval($data['limit']);
        if ($limit > 1000000) {
            send_error('Limit too large (max: 1,000,000)');
        }
        
        $result = crystalline_sieve($limit);
        
        send_response([
            'operation' => 'sieve',
            'limit' => $limit,
            'primes' => $result,
            'count' => count($result)
        ]);
        break;
    
    case 'api/math/sqrt':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['x']);
        
        $x = floatval($data['x']);
        $result = crystalline_sqrt($x);
        
        send_response([
            'operation' => 'sqrt',
            'input' => $x,
            'result' => $result
        ]);
        break;
    
    case 'api/math/pow':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['base', 'exponent']);
        
        $base = floatval($data['base']);
        $exp = floatval($data['exponent']);
        $result = crystalline_pow($base, $exp);
        
        send_response([
            'operation' => 'pow',
            'inputs' => ['base' => $base, 'exponent' => $exp],
            'result' => $result
        ]);
        break;
    
    case 'api/math/sin':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['x']);
        
        $x = floatval($data['x']);
        $result = crystalline_sin($x);
        
        send_response([
            'operation' => 'sin',
            'input' => $x,
            'result' => $result
        ]);
        break;
    
    case 'api/math/cos':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['x']);
        
        $x = floatval($data['x']);
        $result = crystalline_cos($x);
        
        send_response([
            'operation' => 'cos',
            'input' => $x,
            'result' => $result
        ]);
        break;
    
    case 'api/math/log':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['x']);
        
        $x = floatval($data['x']);
        $result = crystalline_log($x);
        
        send_response([
            'operation' => 'log',
            'input' => $x,
            'result' => $result
        ]);
        break;
    
    case 'api/math/exp':
        if ($method !== 'POST') {
            send_error('Method not allowed', 405);
        }
        require_params($data, ['x']);
        
        $x = floatval($data['x']);
        $result = crystalline_exp($x);
        
        send_response([
            'operation' => 'exp',
            'input' => $x,
            'result' => $result
        ]);
        break;
    
    default:
        send_error('Endpoint not found', 404);
}