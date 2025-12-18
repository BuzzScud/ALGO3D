<?php
/**
 * Crystalline Math REST API
 * 
 * Comprehensive REST API for all Crystalline Math functions
 * Supports 112+ mathematical operations via GET requests
 * 
 * Usage: GET /math/?operation=function_name&param1=value1&param2=value2
 */

// Enable error reporting for debugging
error_reporting(E_ALL);
ini_set('display_errors', 0); // Don't display errors in output
ini_set('log_errors', 1);

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, OPTIONS');
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
        'success' => false,
        'error' => 'Crystalline Math extension not loaded',
        'message' => 'Please install and enable the crystalline_math PHP extension'
    ], JSON_PRETTY_PRINT);
    exit();
}

// Check if algorithms extension is loaded
$has_algorithms = extension_loaded('algorithms');

/**
 * Send JSON response
 */
function send_response($data, $code = 200) {
    http_response_code($code);
    echo json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    exit();
}

/**
 * Send error response
 */
function send_error($message, $code = 400) {
    send_response([
        'success' => false,
        'error' => $message
    ], $code);
}

/**
 * Get parameter with default
 */
function get_param($name, $default = null) {
    return $_GET[$name] ?? $default;
}

/**
 * Get required parameter
 */
function require_param($name) {
    if (!isset($_GET[$name])) {
        send_error("Missing required parameter: $name");
    }
    return $_GET[$name];
}

// Get operation
$operation = get_param('operation', 'help');

// Initialize rainbow table and clock if needed
static $rainbow_initialized = false;
static $clock_initialized = false;

function ensure_rainbow_initialized() {
    global $rainbow_initialized;
    if (!$rainbow_initialized) {
        rainbow_init(10000);
        rainbow_populate_count(1000);
        $rainbow_initialized = true;
    }
}

function ensure_clock_initialized() {
    global $clock_initialized;
    if (!$clock_initialized) {
        clock_init();
        $clock_initialized = true;
    }
}

// Route handling
try {
    switch ($operation) {
        // ====================================================================
        // DEBUG / TEST
        // ====================================================================
        case 'debug':
        case 'test':
            send_response([
                'success' => true,
                'php_version' => phpversion(),
                'extensions' => [
                    'crystalline_math' => extension_loaded('crystalline_math'),
                    'algorithms' => extension_loaded('algorithms'),
                ],
                'available_functions' => extension_loaded('crystalline_math') ? get_extension_funcs('crystalline_math') : [],
                'test_is_prime' => function_exists('is_prime') ? is_prime(17) : 'function not found',
                'test_math_add' => function_exists('math_add') ? math_add(5, 3) : 'function not found',
            ]);
            break;

        // ====================================================================
        // HELP / DOCUMENTATION
        // ====================================================================
        case 'help':
        case 'info':
            send_response([
                'success' => true,
                'name' => 'Crystalline Math REST API',
                'version' => '2.0.0',
                'total_functions' => 112,
                'usage' => 'GET /?operation=function_name&param1=value1&param2=value2',
                'categories' => [
                    'arithmetic' => 'Basic arithmetic operations',
                    'transcendental' => 'Trigonometric and logarithmic functions',
                    'prime' => 'Prime number operations',
                    'rainbow' => 'Rainbow table operations',
                    'clock' => 'Clock lattice operations',
                    'platonic' => 'Platonic solid generation',
                    'statistics' => 'Statistical operations (requires algorithms extension)',
                    'numerical' => 'Numerical operations (requires algorithms extension)',
                ],
                'examples' => [
                    'Add two numbers' => '/?operation=add&a=5&b=3',
                    'Check if prime' => '/?operation=is_prime&n=17',
                    'Generate prime' => '/?operation=prime_generate&position=3&magnitude=1',
                    'Rainbow lookup' => '/?operation=rainbow_lookup_by_index&index=10',
                    'Clock position' => '/?operation=clock_map_prime&prime=17',
                    'Calculate sine' => '/?operation=sin&x=1.5708',
                    'Square root' => '/?operation=sqrt&x=16',
                ]
            ]);
            break;

        // ====================================================================
        // LIST ALL FUNCTIONS
        // ====================================================================
        case 'list':
        case 'functions':
            $math_functions = get_extension_funcs('crystalline_math');
            $algo_functions = $has_algorithms ? get_extension_funcs('algorithms') : [];
            
            send_response([
                'success' => true,
                'math_functions' => $math_functions,
                'math_count' => count($math_functions),
                'algorithm_functions' => $algo_functions,
                'algorithm_count' => count($algo_functions),
                'total' => count($math_functions) + count($algo_functions)
            ]);
            break;

        // ====================================================================
        // ARITHMETIC OPERATIONS
        // ====================================================================
        case 'add':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            send_response(['success' => true, 'result' => math_add($a, $b)]);
            break;

        case 'sub':
        case 'subtract':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            send_response(['success' => true, 'result' => math_sub($a, $b)]);
            break;

        case 'mul':
        case 'multiply':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            send_response(['success' => true, 'result' => math_mul($a, $b)]);
            break;

        case 'div':
        case 'divide':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            if ($b == 0) send_error('Division by zero');
            send_response(['success' => true, 'result' => math_div($a, $b)]);
            break;

        case 'mod':
        case 'modulo':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            send_response(['success' => true, 'result' => math_mod($a, $b)]);
            break;

        case 'abs':
        case 'absolute':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_abs($x)]);
            break;

        case 'min':
        case 'minimum':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            send_response(['success' => true, 'result' => math_min($a, $b)]);
            break;

        case 'max':
        case 'maximum':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            send_response(['success' => true, 'result' => math_max($a, $b)]);
            break;

        case 'clamp':
            $x = (float)require_param('x');
            $min = (float)require_param('min');
            $max = (float)require_param('max');
            send_response(['success' => true, 'result' => math_clamp($x, $min, $max)]);
            break;

        case 'sign':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_sign($x)]);
            break;

        case 'floor':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_floor($x)]);
            break;

        case 'ceil':
        case 'ceiling':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_ceil($x)]);
            break;

        case 'round':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_round($x)]);
            break;

        case 'trunc':
        case 'truncate':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_trunc($x)]);
            break;

        // ====================================================================
        // POWER AND ROOT OPERATIONS
        // ====================================================================
        case 'sqrt':
        case 'square_root':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_sqrt($x)]);
            break;

        case 'cbrt':
        case 'cube_root':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_cbrt($x)]);
            break;

        case 'pow':
        case 'power':
            $base = (float)require_param('base');
            $exp = (float)require_param('exp');
            send_response(['success' => true, 'result' => math_pow($base, $exp)]);
            break;

        case 'exp':
        case 'exponential':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_exp($x)]);
            break;

        case 'expm1':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_expm1($x)]);
            break;

        // ====================================================================
        // LOGARITHMIC OPERATIONS
        // ====================================================================
        case 'log':
        case 'ln':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_log($x)]);
            break;

        case 'log10':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_log10($x)]);
            break;

        case 'log2':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_log2($x)]);
            break;

        case 'log1p':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_log1p($x)]);
            break;

        // ====================================================================
        // TRIGONOMETRIC OPERATIONS
        // ====================================================================
        case 'sin':
        case 'sine':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_sin($x)]);
            break;

        case 'cos':
        case 'cosine':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_cos($x)]);
            break;

        case 'tan':
        case 'tangent':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_tan($x)]);
            break;

        case 'asin':
        case 'arcsin':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_asin($x)]);
            break;

        case 'acos':
        case 'arccos':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_acos($x)]);
            break;

        case 'atan':
        case 'arctan':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_atan($x)]);
            break;

        case 'atan2':
            $y = (float)require_param('y');
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_atan2($y, $x)]);
            break;

        // ====================================================================
        // HYPERBOLIC OPERATIONS
        // ====================================================================
        case 'sinh':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_sinh($x)]);
            break;

        case 'cosh':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_cosh($x)]);
            break;

        case 'tanh':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_tanh($x)]);
            break;

        case 'asinh':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_asinh($x)]);
            break;

        case 'acosh':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_acosh($x)]);
            break;

        case 'atanh':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_atanh($x)]);
            break;

        // ====================================================================
        // VALIDATION OPERATIONS
        // ====================================================================
        case 'is_nan':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_is_nan($x)]);
            break;

        case 'is_inf':
        case 'is_infinite':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_is_inf($x)]);
            break;

        case 'is_finite':
            $x = (float)require_param('x');
            send_response(['success' => true, 'result' => math_is_finite($x)]);
            break;

        case 'approx_equal':
            $a = (float)require_param('a');
            $b = (float)require_param('b');
            $epsilon = (float)get_param('epsilon', 1e-9);
            send_response(['success' => true, 'result' => math_approx_equal($a, $b, $epsilon)]);
            break;

        // ====================================================================
        // PRIME NUMBER OPERATIONS
        // ====================================================================
        case 'is_prime':
        case 'prime_check':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => is_prime($n), 'number' => $n]);
            break;

        case 'prime_generate':
        case 'crystalline_prime_generate_o1':
            $position = (int)require_param('position');
            $magnitude = (int)require_param('magnitude');
            $prime = crystalline_prime_generate_o1($position, $magnitude);
            send_response(['success' => true, 'result' => $prime, 'position' => $position, 'magnitude' => $magnitude]);
            break;

        case 'prime_nth':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => prime_nth($n), 'index' => $n]);
            break;

        case 'prime_next':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => prime_next($n), 'after' => $n]);
            break;

        case 'prime_prev':
        case 'prime_previous':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => prime_prev($n), 'before' => $n]);
            break;

        case 'prime_count_below':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => prime_count_below($n), 'limit' => $n]);
            break;

        case 'prime_count_range':
            $start = (int)require_param('start');
            $end = (int)require_param('end');
            send_response(['success' => true, 'result' => prime_count_range($start, $end), 'range' => [$start, $end]]);
            break;

        case 'prime_gap_next':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => prime_gap_next($n), 'after' => $n]);
            break;

        case 'prime_gap_prev':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => prime_gap_prev($n), 'before' => $n]);
            break;

        case 'prime_are_coprime':
        case 'coprime':
            $a = (int)require_param('a');
            $b = (int)require_param('b');
            send_response(['success' => true, 'result' => prime_are_coprime($a, $b), 'numbers' => [$a, $b]]);
            break;

        case 'prime_is_prime_o1':
            $position = (int)require_param('position');
            $magnitude = (int)require_param('magnitude');
            send_response(['success' => true, 'result' => prime_is_prime_o1($position, $magnitude)]);
            break;

        case 'prime_totient':
        case 'totient':
        case 'euler_phi':
            $n = (int)require_param('n');
            send_response(['success' => true, 'result' => prime_totient($n), 'number' => $n]);
            break;

        case 'prime_index':
            $prime = (int)require_param('prime');
            $index = prime_index($prime);
            if ($index === false) {
                send_error("$prime is not a prime number");
            }
            send_response(['success' => true, 'result' => $index, 'prime' => $prime]);
            break;

        // ====================================================================
        // RAINBOW TABLE OPERATIONS
        // ====================================================================
        case 'rainbow_init':
            $capacity = (int)get_param('capacity', 10000);
            rainbow_init($capacity);
            send_response(['success' => true, 'message' => 'Rainbow table initialized', 'capacity' => $capacity]);
            break;

        case 'rainbow_populate':
        case 'rainbow_populate_count':
            ensure_rainbow_initialized();
            $count = (int)get_param('count', 1000);
            rainbow_populate_count($count);
            send_response(['success' => true, 'message' => 'Rainbow table populated', 'count' => $count, 'size' => rainbow_size()]);
            break;

        case 'rainbow_size':
            ensure_rainbow_initialized();
            send_response(['success' => true, 'result' => rainbow_size()]);
            break;

        case 'rainbow_max_prime':
            ensure_rainbow_initialized();
            send_response(['success' => true, 'result' => rainbow_max_prime()]);
            break;

        case 'rainbow_lookup_by_index':
        case 'rainbow_lookup':
            ensure_rainbow_initialized();
            $index = (int)require_param('index');
            $prime = rainbow_lookup_by_index($index);
            if ($prime === false) {
                send_error("Index $index not found in rainbow table");
            }
            send_response(['success' => true, 'result' => $prime, 'index' => $index]);
            break;

        case 'rainbow_lookup_index':
            ensure_rainbow_initialized();
            $prime = (int)require_param('prime');
            $index = rainbow_lookup_index($prime);
            if ($index === false) {
                send_error("Prime $prime not found in rainbow table");
            }
            send_response(['success' => true, 'result' => $index, 'prime' => $prime]);
            break;

        case 'rainbow_next_prime':
            ensure_rainbow_initialized();
            $prime = (int)require_param('prime');
            $next = rainbow_next_prime($prime);
            if ($next === false) {
                send_error("No next prime found in rainbow table");
            }
            send_response(['success' => true, 'result' => $next, 'after' => $prime]);
            break;

        case 'rainbow_prev_prime':
            ensure_rainbow_initialized();
            $prime = (int)require_param('prime');
            $prev = rainbow_prev_prime($prime);
            if ($prev === false) {
                send_error("No previous prime found in rainbow table");
            }
            send_response(['success' => true, 'result' => $prev, 'before' => $prime]);
            break;

        case 'rainbow_contains':
            ensure_rainbow_initialized();
            $prime = (int)require_param('prime');
            send_response(['success' => true, 'result' => rainbow_contains($prime), 'prime' => $prime]);
            break;

        // ====================================================================
        // CLOCK LATTICE OPERATIONS
        // ====================================================================
        case 'clock_init':
            clock_init();
            send_response(['success' => true, 'message' => 'Clock lattice initialized']);
            break;

        case 'clock_map_prime':
        case 'clock_map_prime_to_position':
            ensure_clock_initialized();
            $prime = (int)require_param('prime');
            $pos = clock_map_prime_to_position($prime);
            if ($pos === false) {
                send_error("Failed to map prime $prime to clock position");
            }
            send_response(['success' => true, 'result' => $pos, 'prime' => $prime]);
            break;

        case 'clock_position_to_prime':
            ensure_clock_initialized();
            $ring = (int)require_param('ring');
            $position = (int)require_param('position');
            $pos_array = ['ring' => $ring, 'position' => $position];
            $prime = clock_position_to_prime($pos_array);
            send_response(['success' => true, 'result' => $prime, 'position' => $pos_array]);
            break;

        case 'clock_is_valid_position':
            ensure_clock_initialized();
            $ring = (int)require_param('ring');
            $position = (int)require_param('position');
            $pos_array = ['ring' => $ring, 'position' => $position];
            send_response(['success' => true, 'result' => clock_is_valid_position($pos_array), 'position' => $pos_array]);
            break;

        case 'clock_reverse_lookup':
            ensure_clock_initialized();
            $number = (int)require_param('number');
            $lookup = clock_reverse_lookup($number);
            if ($lookup === false) {
                send_error("Failed to reverse lookup number $number");
            }
            send_response(['success' => true, 'result' => $lookup, 'number' => $number]);
            break;

        // ====================================================================
        // PLATONIC SOLID OPERATIONS
        // ====================================================================
        case 'platonic_tetrahedron':
        case 'tetrahedron':
            $solid = platonic_tetrahedron();
            send_response(['success' => true, 'result' => $solid]);
            break;

        case 'platonic_cube':
        case 'cube':
            $solid = platonic_cube();
            send_response(['success' => true, 'result' => $solid]);
            break;

        case 'platonic_octahedron':
        case 'octahedron':
            $solid = platonic_octahedron();
            send_response(['success' => true, 'result' => $solid]);
            break;

        case 'platonic_dodecahedron':
        case 'dodecahedron':
            $solid = platonic_dodecahedron();
            send_response(['success' => true, 'result' => $solid]);
            break;

        case 'platonic_icosahedron':
        case 'icosahedron':
            $solid = platonic_icosahedron();
            send_response(['success' => true, 'result' => $solid]);
            break;

        case 'platonic_simplex':
        case 'simplex':
            $dimension = (int)require_param('dimension');
            $solid = platonic_simplex($dimension);
            send_response(['success' => true, 'result' => $solid, 'dimension' => $dimension]);
            break;

        case 'platonic_hypercube':
        case 'hypercube':
            $dimension = (int)require_param('dimension');
            $solid = platonic_hypercube($dimension);
            send_response(['success' => true, 'result' => $solid, 'dimension' => $dimension]);
            break;

        case 'platonic_cross_polytope':
        case 'cross_polytope':
            $dimension = (int)require_param('dimension');
            $solid = platonic_cross_polytope($dimension);
            send_response(['success' => true, 'result' => $solid, 'dimension' => $dimension]);
            break;

        // ====================================================================
        // STATISTICS OPERATIONS (requires algorithms extension)
        // ====================================================================
        case 'stats_mean':
        case 'mean':
        case 'average':
            if (!$has_algorithms) send_error('Algorithms extension not loaded', 501);
            $data = json_decode(require_param('data'), true);
            if (!is_array($data)) send_error('Data must be a JSON array');
            send_response(['success' => true, 'result' => stats_mean($data), 'count' => count($data)]);
            break;

        case 'stats_variance':
        case 'variance':
            if (!$has_algorithms) send_error('Algorithms extension not loaded', 501);
            $data = json_decode(require_param('data'), true);
            if (!is_array($data)) send_error('Data must be a JSON array');
            send_response(['success' => true, 'result' => stats_variance($data), 'count' => count($data)]);
            break;

        case 'stats_std_dev':
        case 'std_dev':
        case 'stddev':
            if (!$has_algorithms) send_error('Algorithms extension not loaded', 501);
            $data = json_decode(require_param('data'), true);
            if (!is_array($data)) send_error('Data must be a JSON array');
            send_response(['success' => true, 'result' => stats_std_dev($data), 'count' => count($data)]);
            break;

        case 'stats_median':
        case 'median':
            if (!$has_algorithms) send_error('Algorithms extension not loaded', 501);
            $data = json_decode(require_param('data'), true);
            if (!is_array($data)) send_error('Data must be a JSON array');
            send_response(['success' => true, 'result' => stats_median($data), 'count' => count($data)]);
            break;

        case 'stats_min':
            if (!$has_algorithms) send_error('Algorithms extension not loaded', 501);
            $data = json_decode(require_param('data'), true);
            if (!is_array($data)) send_error('Data must be a JSON array');
            send_response(['success' => true, 'result' => stats_min($data), 'count' => count($data)]);
            break;

        case 'stats_max':
            if (!$has_algorithms) send_error('Algorithms extension not loaded', 501);
            $data = json_decode(require_param('data'), true);
            if (!is_array($data)) send_error('Data must be a JSON array');
            send_response(['success' => true, 'result' => stats_max($data), 'count' => count($data)]);
            break;

        // ====================================================================
        // BATCH OPERATIONS
        // ====================================================================
        case 'batch':
            $operations = json_decode(require_param('operations'), true);
            if (!is_array($operations)) send_error('Operations must be a JSON array');
            
            $results = [];
            foreach ($operations as $op) {
                if (!isset($op['operation'])) {
                    $results[] = ['success' => false, 'error' => 'Missing operation'];
                    continue;
                }
                
                // Temporarily set $_GET to the operation parameters
                $old_get = $_GET;
                $_GET = array_merge(['operation' => $op['operation']], $op['params'] ?? []);
                
                // Capture output
                ob_start();
                try {
                    // Recursive call would go here, but we'll handle inline
                    $results[] = ['success' => true, 'note' => 'Batch operations not yet implemented'];
                } catch (Exception $e) {
                    $results[] = ['success' => false, 'error' => $e->getMessage()];
                }
                ob_end_clean();
                
                $_GET = $old_get;
            }
            
            send_response(['success' => true, 'results' => $results]);
            break;

        // ====================================================================
        // DEFAULT - UNKNOWN OPERATION
        // ====================================================================
        default:
            send_error("Unknown operation: $operation. Use operation=help for documentation.", 404);
    }

} catch (Exception $e) {
    send_error($e->getMessage(), 500);
} catch (Error $e) {
    send_error($e->getMessage(), 500);
}