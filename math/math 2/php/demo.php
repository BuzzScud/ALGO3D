<?php
/**
 * CLLM PHP Extension - Comprehensive Demo
 * 
 * This script demonstrates all features of the CLLM PHP extension:
 * - Model creation and management
 * - Vocabulary building and tokenization
 * - Training initialization
 * - Text generation (inference)
 * - Math library functions
 * - Model save/load
 */

echo "╔══════════════════════════════════════════════════════════════╗\n";
echo "║         CLLM PHP Extension - Comprehensive Demo             ║\n";
echo "║    Crystalline Lattice Language Model for PHP               ║\n";
echo "╚══════════════════════════════════════════════════════════════╝\n\n";

// Check if extension is loaded
if (!extension_loaded('cllm')) {
    die("ERROR: CLLM extension is not loaded!\n");
}

echo "✓ CLLM Extension Version: " . cllm_version() . "\n\n";

// ============================================================================
// SECTION 1: MODEL CREATION
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 1: MODEL CREATION\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Creating a CLLM model with Cube geometry...\n";

$model = cllm_create_model(
    CLLM_PLATONIC_CUBE,  // Platonic solid type
    1000,                 // Vocabulary size
    [
        'embedding_dim' => 64,
        'hidden_dim' => 128,
        'num_layers' => 2,
        'num_heads' => 8,
        'max_seq_len' => 128,
        'learning_rate' => 0.001
    ]
);

if (!$model) {
    die("ERROR: Failed to create model!\n");
}

echo "✓ Model created successfully!\n\n";

// Get model information
$info = cllm_model_info($model);
echo "Model Information:\n";
echo "  Vocabulary Size:  {$info['vocab_size']}\n";
echo "  Embedding Dim:    {$info['embedding_dim']}\n";
echo "  Hidden Dim:       {$info['hidden_dim']}\n";
echo "  Layers:           {$info['num_layers']}\n";
echo "  Attention Heads:  {$info['num_heads']}\n";
echo "  Max Sequence:     {$info['max_seq_len']}\n";
echo "  Platonic Solid:   {$info['platonic_solid']}\n";
echo "  Learning Rate:    {$info['learning_rate']}\n\n";

// ============================================================================
// SECTION 2: VOCABULARY MANAGEMENT
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 2: VOCABULARY MANAGEMENT\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Creating vocabulary...\n";
$vocab = cllm_vocab_create(10000);

if (!$vocab) {
    die("ERROR: Failed to create vocabulary!\n");
}

echo "✓ Vocabulary created!\n\n";

// Create sample training data
$training_text = <<<EOT
The sky is blue because of Rayleigh scattering.
Blue light has a shorter wavelength than red light.
When sunlight enters Earth's atmosphere it collides with air molecules.
Prime numbers are fundamental in mathematics.
A prime number is only divisible by one and itself.
The Riemann Hypothesis concerns the distribution of primes.
Language models learn patterns from text data.
Neural networks consist of interconnected layers.
Attention mechanisms help models focus on relevant information.
Crystalline structures are found throughout nature.
Snowflakes form beautiful crystalline patterns.
The lattice structure determines crystal properties.
EOT;

// Save training data to file
file_put_contents('/tmp/training_data.txt', $training_text);

echo "Building vocabulary from training data...\n";
$tokens_added = cllm_vocab_build_from_file($vocab, '/tmp/training_data.txt');
echo "✓ Added $tokens_added tokens to vocabulary\n\n";

// Get vocabulary statistics
$stats = cllm_vocab_stats($vocab);
echo "Vocabulary Statistics:\n";
echo "  Size:         {$stats['size']}\n";
echo "  Capacity:     {$stats['capacity']}\n";
echo "  Total Tokens: {$stats['total_tokens']}\n";
echo "  PAD Token ID: {$stats['pad_token_id']}\n";
echo "  UNK Token ID: {$stats['unk_token_id']}\n";
echo "  BOS Token ID: {$stats['bos_token_id']}\n";
echo "  EOS Token ID: {$stats['eos_token_id']}\n\n";

// ============================================================================
// SECTION 3: TOKENIZATION
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 3: TOKENIZATION & DETOKENIZATION\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

$test_sentences = [
    "The sky is blue",
    "Prime numbers are important",
    "Language models learn patterns"
];

foreach ($test_sentences as $sentence) {
    echo "Original: &quot;$sentence&quot;\n";
    
    // Tokenize
    $token_ids = cllm_vocab_tokenize($vocab, $sentence);
    echo "Tokens:   [" . implode(", ", $token_ids) . "]\n";
    
    // Detokenize
    $reconstructed = cllm_vocab_detokenize($vocab, $token_ids);
    echo "Decoded:  &quot;$reconstructed&quot;\n";
    echo "Match:    " . ($sentence === $reconstructed ? "✓ Perfect" : "✗ Mismatch") . "\n\n";
}

// ============================================================================
// SECTION 4: VOCABULARY SAVE/LOAD
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 4: VOCABULARY PERSISTENCE\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Saving vocabulary to file...\n";
$vocab_file = '/tmp/test_vocab.vocab';
if (cllm_vocab_save($vocab, $vocab_file)) {
    echo "✓ Vocabulary saved to $vocab_file\n";
    echo "  File size: " . filesize($vocab_file) . " bytes\n\n";
} else {
    echo "✗ Failed to save vocabulary\n\n";
}

echo "Loading vocabulary from file...\n";
$loaded_vocab = cllm_vocab_load($vocab_file);
if ($loaded_vocab) {
    echo "✓ Vocabulary loaded successfully!\n";
    $loaded_stats = cllm_vocab_stats($loaded_vocab);
    echo "  Loaded size: {$loaded_stats['size']} tokens\n\n";
} else {
    echo "✗ Failed to load vocabulary\n\n";
}

// ============================================================================
// SECTION 5: TRAINING INITIALIZATION
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 5: TRAINING INITIALIZATION\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Initializing training...\n";
$training = cllm_training_init($model, [
    'learning_rate' => 0.001,
    'batch_size' => 4,
    'num_epochs' => 10,
    'sequence_length' => 64
]);

if ($training) {
    echo "✓ Training initialized successfully!\n";
    echo "  Configuration:\n";
    echo "    Learning Rate:    0.001\n";
    echo "    Batch Size:       4\n";
    echo "    Epochs:           10\n";
    echo "    Sequence Length:  64\n\n";
} else {
    echo "✗ Failed to initialize training\n\n";
}

// ============================================================================
// SECTION 6: TEXT GENERATION (INFERENCE)
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 6: TEXT GENERATION\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

$prompts = [
    "The sky is",
    "Prime numbers",
    "Language models"
];

foreach ($prompts as $prompt) {
    echo "Prompt: &quot;$prompt&quot;\n";
    
    $generated = cllm_generate($model, $prompt, [
        'max_tokens' => 20,
        'temperature' => 0.8
    ]);
    
    echo "Generated: $generated\n\n";
}

// ============================================================================
// SECTION 7: MODEL SAVE/LOAD
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 7: MODEL PERSISTENCE\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Saving model to file...\n";
$model_file = '/tmp/test_model.cllm';
if (cllm_save_model($model, $model_file)) {
    echo "✓ Model saved to $model_file\n";
    echo "  File size: " . filesize($model_file) . " bytes\n\n";
} else {
    echo "✗ Failed to save model\n\n";
}

echo "Loading model from file...\n";
$loaded_model = cllm_load_model($model_file);
if ($loaded_model) {
    echo "✓ Model loaded successfully!\n";
    $loaded_info = cllm_model_info($loaded_model);
    echo "  Loaded vocab size: {$loaded_info['vocab_size']}\n";
    echo "  Loaded layers:     {$loaded_info['num_layers']}\n\n";
} else {
    echo "✗ Failed to load model\n\n";
}

// ============================================================================
// SECTION 8: MATH LIBRARY FUNCTIONS
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 8: MATH LIBRARY FUNCTIONS\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Prime Number Functions:\n";
for ($i = 1; $i <= 10; $i++) {
    $prime = cllm_prime_nth($i);
    $is_prime = cllm_prime_is_prime($prime) ? "✓" : "✗";
    echo "  {$i}th prime: $prime $is_prime\n";
}
echo "\n";

echo "Transcendental Functions:\n";
$test_values = [0, 1, 2, M_PI/2, M_PI];
foreach ($test_values as $x) {
    $exp_val = cllm_math_exp($x);
    $log_val = $x > 0 ? cllm_math_log($x) : "undefined";
    $sin_val = cllm_math_sin($x);
    $cos_val = cllm_math_cos($x);
    
    echo sprintf("  x = %.4f:\n", $x);
    echo sprintf("    exp(x) = %.6f\n", $exp_val);
    echo sprintf("    log(x) = %s\n", is_numeric($log_val) ? sprintf("%.6f", $log_val) : $log_val);
    echo sprintf("    sin(x) = %.6f\n", $sin_val);
    echo sprintf("    cos(x) = %.6f\n", $cos_val);
    echo "\n";
}

// ============================================================================
// SECTION 9: PLATONIC SOLID TYPES
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " SECTION 9: PLATONIC SOLID TYPES\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

$solids = [
    'CLLM_PLATONIC_TETRAHEDRON' => CLLM_PLATONIC_TETRAHEDRON,
    'CLLM_PLATONIC_CUBE' => CLLM_PLATONIC_CUBE,
    'CLLM_PLATONIC_OCTAHEDRON' => CLLM_PLATONIC_OCTAHEDRON,
    'CLLM_PLATONIC_DODECAHEDRON' => CLLM_PLATONIC_DODECAHEDRON,
    'CLLM_PLATONIC_ICOSAHEDRON' => CLLM_PLATONIC_ICOSAHEDRON
];

echo "Available Platonic Solids:\n";
foreach ($solids as $name => $value) {
    echo "  $name = $value\n";
}
echo "\n";

// ============================================================================
// CLEANUP
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " CLEANUP\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Freeing resources...\n";
// Resources are automatically freed when they go out of scope
echo "✓ Resources freed\n\n";

// ============================================================================
// SUMMARY
// ============================================================================

echo "═══════════════════════════════════════════════════════════════\n";
echo " DEMO COMPLETE\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Summary of demonstrated features:\n";
echo "  ✓ Model creation with custom configuration\n";
echo "  ✓ Vocabulary building from text files\n";
echo "  ✓ Tokenization and detokenization\n";
echo "  ✓ Vocabulary save/load\n";
echo "  ✓ Training initialization\n";
echo "  ✓ Text generation (inference)\n";
echo "  ✓ Model save/load\n";
echo "  ✓ Math library functions (primes, exp, log, sin, cos)\n";
echo "  ✓ Platonic solid types\n\n";

echo "All features demonstrated successfully!\n";
?>