<?php
/**
 * Crystalline Algorithms REST API
 * 
 * Complete REST API for all Crystalline Algorithms functions
 */

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle OPTIONS request
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

// Get request path
$path = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
$path = str_replace('/api/algorithms/', '', $path);
$path = trim($path, '/');

// Get request body
$input = json_decode(file_get_contents('php://input'), true);

// Response helper
function respond($data, $status = 200) {
    http_response_code($status);
    echo json_encode($data, JSON_PRETTY_PRINT);
    exit();
}

// Error helper
function error($message, $status = 400) {
    respond(['error' => $message], $status);
}

// ============================================================================
// NUMERICAL ENDPOINTS
// ============================================================================

if ($path === 'numerical/softmax') {
    if (!isset($input['logits'])) {
        error('Missing parameter: logits (array) required');
    }
    $result = numerical_softmax($input['logits']);
    respond(['result' => $result, 'operation' => 'softmax']);
}

if ($path === 'numerical/log_softmax') {
    if (!isset($input['logits'])) {
        error('Missing parameter: logits (array) required');
    }
    $result = numerical_log_softmax($input['logits']);
    respond(['result' => $result, 'operation' => 'log_softmax']);
}

if ($path === 'numerical/safe_log') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $epsilon = $input['epsilon'] ?? 1e-10;
    $result = numerical_safe_log($input['x'], $epsilon);
    respond(['result' => $result, 'operation' => 'safe_log']);
}

if ($path === 'numerical/safe_divide') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    $default = $input['default'] ?? 0.0;
    $result = numerical_safe_divide($input['a'], $input['b'], $default);
    respond(['result' => $result, 'operation' => 'safe_divide']);
}

if ($path === 'numerical/clip') {
    if (!isset($input['x']) || !isset($input['min']) || !isset($input['max'])) {
        error('Missing parameters: x, min, max required');
    }
    $result = numerical_clip($input['x'], $input['min'], $input['max']);
    respond(['result' => $result, 'operation' => 'clip']);
}

// ============================================================================
// STATISTICS ENDPOINTS
// ============================================================================

if ($path === 'stats/mean') {
    if (!isset($input['values'])) {
        error('Missing parameter: values (array) required');
    }
    $result = stats_mean($input['values']);
    respond(['result' => $result, 'operation' => 'mean']);
}

if ($path === 'stats/variance') {
    if (!isset($input['values'])) {
        error('Missing parameter: values (array) required');
    }
    $mean = $input['mean'] ?? stats_mean($input['values']);
    $result = stats_variance($input['values'], $mean);
    respond(['result' => $result, 'operation' => 'variance', 'mean' => $mean]);
}

if ($path === 'stats/std_dev') {
    if (!isset($input['values'])) {
        error('Missing parameter: values (array) required');
    }
    $mean = $input['mean'] ?? stats_mean($input['values']);
    $result = stats_std_dev($input['values'], $mean);
    respond(['result' => $result, 'operation' => 'standard_deviation', 'mean' => $mean]);
}

if ($path === 'stats/median') {
    if (!isset($input['values'])) {
        error('Missing parameter: values (array) required');
    }
    $result = stats_median($input['values']);
    respond(['result' => $result, 'operation' => 'median']);
}

if ($path === 'stats/percentile') {
    if (!isset($input['values']) || !isset($input['p'])) {
        error('Missing parameters: values (array), p (percentile) required');
    }
    $result = stats_percentile($input['values'], $input['p']);
    respond(['result' => $result, 'operation' => 'percentile', 'p' => $input['p']]);
}

if ($path === 'stats/correlation') {
    if (!isset($input['x']) || !isset($input['y'])) {
        error('Missing parameters: x, y (arrays) required');
    }
    $result = stats_correlation($input['x'], $input['y']);
    respond(['result' => $result, 'operation' => 'correlation']);
}

if ($path === 'stats/min') {
    if (!isset($input['values'])) {
        error('Missing parameter: values (array) required');
    }
    $result = stats_min($input['values']);
    respond(['result' => $result, 'operation' => 'minimum']);
}

if ($path === 'stats/max') {
    if (!isset($input['values'])) {
        error('Missing parameter: values (array) required');
    }
    $result = stats_max($input['values']);
    respond(['result' => $result, 'operation' => 'maximum']);
}

// ============================================================================
// OPTIMIZER ENDPOINTS
// ============================================================================

if ($path === 'optimizer/create') {
    if (!isset($input['type'])) {
        error('Missing parameter: type (sgd, momentum, adam, rmsprop) required');
    }
    $config = optimizer_config_create($input['type']);
    if (isset($input['learning_rate'])) {
        $config['learning_rate'] = $input['learning_rate'];
    }
    respond(['result' => $config, 'operation' => 'create_optimizer']);
}

if ($path === 'optimizer/lr_schedule') {
    if (!isset($input['type'])) {
        error('Missing parameter: type (constant, step, exponential, cosine, warmup) required');
    }
    $config = lr_schedule_config_create($input['type']);
    if (isset($input['initial_lr'])) {
        $config['initial_lr'] = $input['initial_lr'];
    }
    respond(['result' => $config, 'operation' => 'create_lr_schedule']);
}

if ($path === 'optimizer/get_lr') {
    if (!isset($input['config']) || !isset($input['step'])) {
        error('Missing parameters: config, step required');
    }
    $result = lr_schedule_get_lr($input['config'], $input['step']);
    respond(['result' => $result, 'operation' => 'get_learning_rate', 'step' => $input['step']]);
}

// ============================================================================
// GRADIENT ENDPOINTS
// ============================================================================

if ($path === 'gradient/stats') {
    if (!isset($input['gradients'])) {
        error('Missing parameter: gradients (array) required');
    }
    $result = gradient_compute_stats($input['gradients']);
    respond(['result' => $result, 'operation' => 'gradient_statistics']);
}

if ($path === 'gradient/clip_by_value') {
    if (!isset($input['gradients']) || !isset($input['min']) || !isset($input['max'])) {
        error('Missing parameters: gradients, min, max required');
    }
    $gradients = $input['gradients'];
    $count = gradient_clip_by_value($gradients, $input['min'], $input['max']);
    respond([
        'result' => $gradients,
        'clipped_count' => $count,
        'operation' => 'clip_gradients_by_value'
    ]);
}

if ($path === 'gradient/clip_by_norm') {
    if (!isset($input['gradients']) || !isset($input['max_norm'])) {
        error('Missing parameters: gradients, max_norm required');
    }
    $gradients = $input['gradients'];
    $success = gradient_clip_by_norm($gradients, $input['max_norm']);
    respond([
        'result' => $gradients,
        'success' => $success,
        'operation' => 'clip_gradients_by_norm'
    ]);
}

// ============================================================================
// THREADING ENDPOINTS
// ============================================================================

if ($path === 'threading/detect_cores') {
    $result = detect_num_cpu_cores();
    respond(['result' => $result, 'operation' => 'detect_cpu_cores']);
}

if ($path === 'threading/optimal_threads') {
    if (!isset($input['work_groups'])) {
        error('Missing parameter: work_groups required');
    }
    $result = get_optimal_thread_count($input['work_groups']);
    respond(['result' => $result, 'operation' => 'optimal_thread_count']);
}

// ============================================================================
// SPHERE PACKING ENDPOINTS
// ============================================================================

if ($path === 'sphere/are_kissing') {
    if (!isset($input['sphere1']) || !isset($input['sphere2'])) {
        error('Missing parameters: sphere1, sphere2 required (each with x,y,z,radius)');
    }
    $tolerance = $input['tolerance'] ?? 0.01;
    $result = spheres_are_kissing($input['sphere1'], $input['sphere2'], $tolerance);
    respond(['result' => $result, 'operation' => 'check_kissing_spheres']);
}

if ($path === 'sphere/distance') {
    if (!isset($input['sphere1']) || !isset($input['sphere2'])) {
        error('Missing parameters: sphere1, sphere2 required (each with x,y,z,radius)');
    }
    $result = sphere_distance($input['sphere1'], $input['sphere2']);
    respond(['result' => $result, 'operation' => 'sphere_distance']);
}

if ($path === 'sphere/gap') {
    if (!isset($input['sphere1']) || !isset($input['sphere2'])) {
        error('Missing parameters: sphere1, sphere2 required (each with x,y,z,radius)');
    }
    $result = sphere_gap($input['sphere1'], $input['sphere2']);
    respond(['result' => $result, 'operation' => 'sphere_gap']);
}

if ($path === 'sphere/volume') {
    if (!isset($input['sphere'])) {
        error('Missing parameter: sphere required (with x,y,z,radius)');
    }
    $result = sphere_volume($input['sphere']);
    respond(['result' => $result, 'operation' => 'sphere_volume']);
}

if ($path === 'sphere/surface_area') {
    if (!isset($input['sphere'])) {
        error('Missing parameter: sphere required (with x,y,z,radius)');
    }
    $result = sphere_surface_area($input['sphere']);
    respond(['result' => $result, 'operation' => 'sphere_surface_area']);
}

// ============================================================================
// NTT ATTENTION ENDPOINTS
// ============================================================================

if ($path === 'ntt/should_use') {
    if (!isset($input['seq_len'])) {
        error('Missing parameter: seq_len required');
    }
    $config = $input['config'] ?? ntt_attention_config_init();
    $result = should_use_ntt_attention($input['seq_len'], $config);
    respond(['result' => $result, 'operation' => 'should_use_ntt_attention']);
}

if ($path === 'ntt/estimate_speedup') {
    if (!isset($input['seq_len'])) {
        error('Missing parameter: seq_len required');
    }
    $result = ntt_attention_estimate_speedup($input['seq_len']);
    respond(['result' => $result, 'operation' => 'estimate_ntt_speedup']);
}

// ============================================================================
// VALIDATION ENDPOINTS
// ============================================================================

if ($path === 'validation/check_residual') {
    if (!isset($input['value']) || !isset($input['threshold'])) {
        error('Missing parameters: value, threshold required');
    }
    $result = validation_check_residual($input['value'], $input['threshold']);
    respond(['result' => $result, 'operation' => 'check_residual']);
}

if ($path === 'validation/check_entropy') {
    if (!isset($input['value']) || !isset($input['threshold'])) {
        error('Missing parameters: value, threshold required');
    }
    $result = validation_check_entropy($input['value'], $input['threshold']);
    respond(['result' => $result, 'operation' => 'check_entropy']);
}

if ($path === 'validation/check_range') {
    if (!isset($input['value']) || !isset($input['min']) || !isset($input['max'])) {
        error('Missing parameters: value, min, max required');
    }
    $result = validation_check_range($input['value'], $input['min'], $input['max']);
    respond(['result' => $result, 'operation' => 'check_range']);
}

if ($path === 'validation/calculate_harm') {
    if (!isset($input['P_misuse']) || !isset($input['D_damage']) || !isset($input['R_resilience'])) {
        error('Missing parameters: P_misuse, D_damage, R_resilience required');
    }
    $result = validation_calculate_harm($input['P_misuse'], $input['D_damage'], $input['R_resilience']);
    respond(['result' => $result, 'operation' => 'calculate_harm']);
}

if ($path === 'validation/safety_probability') {
    if (!isset($input['proof_level']) || !isset($input['alpha'])) {
        error('Missing parameters: proof_level, alpha required');
    }
    $result = validation_safety_probability($input['proof_level'], $input['alpha']);
    respond(['result' => $result, 'operation' => 'safety_probability']);
}

// ============================================================================
// VISUALIZATION ENDPOINTS
// ============================================================================

if ($path === 'viz/project_2d') {
    if (!isset($input['points']) || !isset($input['dimensions'])) {
        error('Missing parameters: points (array of arrays), dimensions required');
    }
    $viz_data = viz_create(count($input['points']), $input['dimensions']);
    foreach ($input['points'] as $i => $point) {
        viz_set_point_position($viz_data, $i, $point);
    }
    viz_project_2d($viz_data);
    $result = viz_get_2d_rendering_data($viz_data);
    respond(['result' => $result, 'operation' => 'project_to_2d']);
}

if ($path === 'viz/project_3d') {
    if (!isset($input['points']) || !isset($input['dimensions'])) {
        error('Missing parameters: points (array of arrays), dimensions required');
    }
    $viz_data = viz_create(count($input['points']), $input['dimensions']);
    foreach ($input['points'] as $i => $point) {
        viz_set_point_position($viz_data, $i, $point);
    }
    viz_project_3d($viz_data);
    $result = viz_get_3d_rendering_data($viz_data);
    respond(['result' => $result, 'operation' => 'project_to_3d']);
}

if ($path === 'viz/statistics') {
    if (!isset($input['points'])) {
        error('Missing parameter: points (array of arrays) required');
    }
    $dimensions = count($input['points'][0]);
    $viz_data = viz_create(count($input['points']), $dimensions);
    foreach ($input['points'] as $i => $point) {
        viz_set_point_position($viz_data, $i, $point);
    }
    $result = viz_calculate_statistics($viz_data);
    respond(['result' => $result, 'operation' => 'visualization_statistics']);
}

// ============================================================================
// API DOCUMENTATION ENDPOINT
// ============================================================================

if ($path === '' || $path === 'help') {
    $endpoints = [
        'numerical' => [
            'numerical/softmax', 'numerical/log_softmax', 'numerical/safe_log',
            'numerical/safe_divide', 'numerical/clip'
        ],
        'statistics' => [
            'stats/mean', 'stats/variance', 'stats/std_dev', 'stats/median',
            'stats/percentile', 'stats/correlation', 'stats/min', 'stats/max'
        ],
        'optimizer' => [
            'optimizer/create', 'optimizer/lr_schedule', 'optimizer/get_lr'
        ],
        'gradient' => [
            'gradient/stats', 'gradient/clip_by_value', 'gradient/clip_by_norm'
        ],
        'threading' => [
            'threading/detect_cores', 'threading/optimal_threads'
        ],
        'sphere' => [
            'sphere/are_kissing', 'sphere/distance', 'sphere/gap',
            'sphere/volume', 'sphere/surface_area'
        ],
        'ntt' => [
            'ntt/should_use', 'ntt/estimate_speedup'
        ],
        'validation' => [
            'validation/check_residual', 'validation/check_entropy',
            'validation/check_range', 'validation/calculate_harm',
            'validation/safety_probability'
        ],
        'visualization' => [
            'viz/project_2d', 'viz/project_3d', 'viz/statistics'
        ]
    ];
    
    respond([
        'name' => 'Crystalline Algorithms REST API',
        'version' => '1.0.0',
        'endpoints' => $endpoints,
        'usage' => 'POST to /api/algorithms/{endpoint} with JSON body'
    ]);
}

// 404 - Endpoint not found
error('Endpoint not found: ' . $path, 404);
?>