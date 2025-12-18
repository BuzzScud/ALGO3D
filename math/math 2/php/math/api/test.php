<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);

header('Content-Type: application/json');

$result = [
    'php_version' => phpversion(),
    'extensions_loaded' => [
        'crystalline_math' => extension_loaded('crystalline_math'),
        'algorithms' => extension_loaded('algorithms'),
    ],
    'test_operations' => []
];

// Test is_prime
if (function_exists('is_prime')) {
    try {
        $result['test_operations']['is_prime'] = [
            'exists' => true,
            'test' => is_prime(17),
            'error' => null
        ];
    } catch (Exception $e) {
        $result['test_operations']['is_prime'] = [
            'exists' => true,
            'test' => null,
            'error' => $e->getMessage()
        ];
    }
} else {
    $result['test_operations']['is_prime'] = [
        'exists' => false,
        'error' => 'Function does not exist'
    ];
}

// Test math_add
if (function_exists('math_add')) {
    try {
        $result['test_operations']['math_add'] = [
            'exists' => true,
            'test' => math_add(5, 3),
            'error' => null
        ];
    } catch (Exception $e) {
        $result['test_operations']['math_add'] = [
            'exists' => true,
            'test' => null,
            'error' => $e->getMessage()
        ];
    }
} else {
    $result['test_operations']['math_add'] = [
        'exists' => false,
        'error' => 'Function does not exist'
    ];
}

// List all available functions
if (extension_loaded('crystalline_math')) {
    $result['available_functions'] = get_extension_funcs('crystalline_math');
}

echo json_encode($result, JSON_PRETTY_PRINT);