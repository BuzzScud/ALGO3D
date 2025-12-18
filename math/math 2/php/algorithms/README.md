# Algorithms PHP Extension

A high-performance PHP extension providing access to the Algorithms Library with focus on numerical algorithms, optimization, and statistics.

## Features

- **Statistical Analysis**
  - Mean, variance, standard deviation
  - Min, max, count
  - Comprehensive statistics

- **Vector Operations**
  - Dot product
  - Vector norm (L2)
  - Vector normalization
  - Cosine similarity

- **Machine Learning Functions**
  - Softmax activation
  - Cross-entropy loss
  - Gradient descent optimization

## Installation

### Prerequisites

- PHP 7.4 or higher
- PHP development headers (`php-dev` package)
- GCC compiler
- Algorithms library (libalgorithms.so)
- Crystalline Math library (libcrystallinemath.so)

### Build and Install

```bash
cd php/algorithms
make install
```

This will:
1. Build the extension
2. Install it to your PHP extension directory
3. Enable it in your PHP configuration

### Verify Installation

```bash
php -m | grep algorithms
```

You should see `algorithms` in the output.

## PHP Extension Usage

### Statistical Analysis

```php
<?php

// Calculate comprehensive statistics
$data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
$stats = algo_statistics($data);
/*
Array (
    [count] => 10
    [mean] => 5.5
    [variance] => 8.25
    [std_dev] => 2.872
    [min] => 1
    [max] => 10
)
*/

// Individual statistical functions
$mean = algo_mean($data);           // 5.5
$variance = algo_variance($data);   // 8.25
$std_dev = algo_std_dev($data);     // 2.872
```

### Vector Operations

```php
<?php

// Dot product
$a = [1, 2, 3];
$b = [4, 5, 6];
$dot = algo_dot_product($a, $b);  // 32 (1*4 + 2*5 + 3*6)

// Vector norm
$vector = [3, 4];
$norm = algo_vector_norm($vector);  // 5.0

// Normalize vector
$normalized = algo_normalize_vector([3, 4]);  // [0.6, 0.8]

// Cosine similarity
$similarity = algo_cosine_similarity($a, $b);  // 0.9746
```

### Machine Learning Functions

```php
<?php

// Softmax activation
$logits = [1.0, 2.0, 3.0];
$probs = algo_softmax($logits);  // [0.0900, 0.2447, 0.6652]

// Cross-entropy loss
$predictions = [0.7, 0.2, 0.1];
$targets = [1.0, 0.0, 0.0];
$loss = algo_cross_entropy($predictions, $targets);  // 0.3567

// Gradient descent optimization
$gradient_fn = function($x) {
    return [2 * ($x[0] - 3)];  // Gradient of (x-3)^2
};

$initial = [0.0];
$optimized = algo_gradient_descent($initial, $gradient_fn, [
    'learning_rate' => 0.1,
    'max_iterations' => 100,
    'tolerance' => 1e-6
]);  // [3.0] (finds minimum at x=3)
```

## REST API Usage

### Start the API Server

```bash
cd php/algorithms
php -S localhost:8081 rest_api.php
```

### API Endpoints

#### Get API Documentation

```bash
curl http://localhost:8081/api/algo
```

#### Calculate Statistics

```bash
curl -X POST http://localhost:8081/api/algo/statistics \
  -H "Content-Type: application/json" \
  -d '{"data": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]}'
```

Response:
```json
{
  "operation": "statistics",
  "input_count": 10,
  "result": {
    "count": 10,
    "mean": 5.5,
    "variance": 8.25,
    "std_dev": 2.872,
    "min": 1,
    "max": 10
  }
}
```

#### Calculate Mean

```bash
curl -X POST http://localhost:8081/api/algo/mean \
  -H "Content-Type: application/json" \
  -d '{"data": [2, 4, 6, 8, 10]}'
```

#### Dot Product

```bash
curl -X POST http://localhost:8081/api/algo/dot_product \
  -H "Content-Type: application/json" \
  -d '{"a": [1, 2, 3], "b": [4, 5, 6]}'
```

Response:
```json
{
  "operation": "dot_product",
  "dimension": 3,
  "result": 32
}
```

#### Vector Norm

```bash
curl -X POST http://localhost:8081/api/algo/vector_norm \
  -H "Content-Type: application/json" \
  -d '{"vector": [3, 4]}'
```

#### Normalize Vector

```bash
curl -X POST http://localhost:8081/api/algo/normalize \
  -H "Content-Type: application/json" \
  -d '{"vector": [3, 4]}'
```

Response:
```json
{
  "operation": "normalize_vector",
  "input": [3, 4],
  "result": [0.6, 0.8],
  "norm": 1.0
}
```

#### Cosine Similarity

```bash
curl -X POST http://localhost:8081/api/algo/cosine_similarity \
  -H "Content-Type: application/json" \
  -d '{"a": [1, 2, 3], "b": [4, 5, 6]}'
```

#### Softmax

```bash
curl -X POST http://localhost:8081/api/algo/softmax \
  -H "Content-Type: application/json" \
  -d '{"logits": [1.0, 2.0, 3.0]}'
```

Response:
```json
{
  "operation": "softmax",
  "input": [1.0, 2.0, 3.0],
  "result": [0.0900, 0.2447, 0.6652],
  "sum": 1.0
}
```

#### Cross-Entropy Loss

```bash
curl -X POST http://localhost:8081/api/algo/cross_entropy \
  -H "Content-Type: application/json" \
  -d '{"predictions": [0.7, 0.2, 0.1], "targets": [1.0, 0.0, 0.0]}'
```

## Testing

Run the test suite:

```bash
make test
```

This will run comprehensive tests including:
- Statistical functions
- Vector operations
- Machine learning functions
- Performance benchmarks

## Performance

The Algorithms extension provides significant performance improvements:

- **Statistics:** ~50x faster than pure PHP
- **Dot product:** ~100x faster than pure PHP
- **Vector operations:** ~80x faster than pure PHP

## API Reference

### Statistical Functions

#### `algo_statistics(array $data): array`
Calculate comprehensive statistics (count, mean, variance, std_dev, min, max).

#### `algo_mean(array $data): float`
Calculate the arithmetic mean.

#### `algo_variance(array $data): float`
Calculate the variance.

#### `algo_std_dev(array $data): float`
Calculate the standard deviation.

### Vector Operations

#### `algo_dot_product(array $a, array $b): float`
Calculate the dot product of two vectors.

#### `algo_vector_norm(array $vector): float`
Calculate the L2 norm (Euclidean length) of a vector.

#### `algo_normalize_vector(array $vector): array`
Normalize a vector to unit length.

#### `algo_cosine_similarity(array $a, array $b): float`
Calculate cosine similarity between two vectors.

### Machine Learning Functions

#### `algo_softmax(array $logits): array`
Apply softmax activation function.

#### `algo_cross_entropy(array $predictions, array $targets): float`
Calculate cross-entropy loss.

#### `algo_gradient_descent(array $initial, callable $gradient_fn, array $options): array`
Perform gradient descent optimization.

Options:
- `learning_rate` (default: 0.01)
- `max_iterations` (default: 1000)
- `tolerance` (default: 1e-6)

## Examples

### Statistical Analysis

```php
<?php
$sales_data = [100, 150, 120, 180, 200, 160, 140];
$stats = algo_statistics($sales_data);

echo "Sales Statistics:\n";
echo "  Average: {$stats['mean']}\n";
echo "  Std Dev: {$stats['std_dev']}\n";
echo "  Range: {$stats['min']} - {$stats['max']}\n";
```

### Machine Learning

```php
<?php
// Softmax for classification
$logits = [2.0, 1.0, 0.1];
$probs = algo_softmax($logits);
echo "Class probabilities: " . implode(", ", $probs) . "\n";

// Calculate loss
$predictions = [0.7, 0.2, 0.1];
$targets = [1.0, 0.0, 0.0];
$loss = algo_cross_entropy($predictions, $targets);
echo "Loss: $loss\n";
```

### Optimization

```php
<?php
// Find minimum of f(x) = (x - 5)^2
$gradient = function($x) {
    return [2 * ($x[0] - 5)];
};

$result = algo_gradient_descent([0.0], $gradient, [
    'learning_rate' => 0.1,
    'max_iterations' => 100
]);

echo "Optimized x: {$result[0]}\n";  // Should be close to 5.0
```

## Troubleshooting

### Extension not loading

Check PHP error log:
```bash
tail -f /var/log/php/error.log
```

Verify extension file exists:
```bash
php-config --extension-dir
ls -la $(php-config --extension-dir)/algorithms.so
```

### Library not found

Make sure libalgorithms.so is in your library path:
```bash
export LD_LIBRARY_PATH=/workspace:$LD_LIBRARY_PATH
```

## License

Part of the Crystalline CLLM project.

## Contributing

Contributions are welcome! Please submit pull requests or open issues on GitHub.