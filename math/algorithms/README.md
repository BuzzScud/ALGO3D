# libalgorithms.so - Mathematical Algorithms Library

## Overview

The **algorithms library** provides pure mathematical implementations of optimization algorithms, loss functions, numerical analysis, and statistical methods. This library is designed to be **domain-agnostic** and can be used for:

- Neural network training
- Statistical analysis
- Numerical optimization
- Scientific computing
- Financial modeling
- Physics simulations
- Any optimization problem

## Architecture

```
libcrystalline.so (Pure Mathematics)
    ↓ depends on
libalgorithms.so (Mathematical Algorithms)
    ↓ depends on
libcllm.so (Language Model Specific)
```

### Design Principles

1. **Mathematical Purity**: Algorithms are implemented with mathematical rigor
2. **Domain Agnostic**: Not tied to any specific application domain
3. **Reusability**: Can be used in any project requiring optimization
4. **Numerical Stability**: Careful handling of edge cases and numerical issues
5. **Performance**: Optimized implementations with O2 compilation

## Components

### 1. Loss Functions (`loss_functions.h`)

Mathematical loss/error functions for optimization:

- **Cross-Entropy Loss**: For classification problems
- **Mean Squared Error (MSE)**: For regression problems
- **Mean Absolute Error (MAE)**: For robust regression
- **Huber Loss**: Smooth combination of MSE and MAE
- **KL Divergence**: For probability distribution comparison
- **Binary Cross-Entropy**: For binary classification

**Features:**
- Label smoothing support
- Multiple reduction strategies (mean, sum, none)
- Gradient computation with automatic differentiation
- Gradient clipping (by value and by norm)
- NaN/Inf detection and handling

### 2. Optimizers (`optimizers.h`)

Gradient-based optimization algorithms:

- **SGD**: Stochastic Gradient Descent
- **Momentum**: SGD with momentum
- **Nesterov**: Nesterov accelerated gradient
- **AdaGrad**: Adaptive gradient algorithm
- **RMSprop**: Root mean square propagation
- **Adam**: Adaptive moment estimation
- **AdamW**: Adam with decoupled weight decay
- **NAdam**: Nesterov-accelerated Adam

**Features:**
- Learning rate scheduling (constant, step, exponential, cosine, warmup)
- Weight decay (L2 regularization)
- Bias correction for adaptive methods
- AMSGrad variant support
- State management for stateful optimizers

### 3. Numerical Analysis (`numerical.h`)

Numerical stability and mathematical utilities:

- **Softmax & Log-Softmax**: Probability normalization
- **Log-Sum-Exp**: Numerically stable logarithm of sum of exponentials
- **Safe Math Operations**: Avoiding overflow, underflow, division by zero
- **Array Operations**: Min, max, sum, mean, variance, standard deviation
- **Normalization**: Min-max, standardization, L2 normalization
- **Distance Metrics**: L1, L2, cosine similarity, dot product

### 4. Backpropagation (`backprop.h`)

Gradient computation and accumulation:

- **Gradient Buffers**: Thread-safe gradient storage
- **Gradient Accumulation**: Sum, mean, weighted strategies
- **Gradient Operations**: Add, subtract, scale, multiply
- **Gradient Clipping**: By value, by norm, by global norm
- **Gradient Statistics**: L1/L2 norms, mean, variance, numerical checks
- **Hierarchical Aggregation**: Tree reduction for distributed gradients

### 5. Statistics (`statistics.h`)

Statistical analysis functions:

- **Descriptive Statistics**: Mean, variance, std dev, median, mode, percentiles
- **Correlation**: Pearson, Spearman rank correlation
- **Covariance**: Sample and population covariance
- **Distribution Functions**: Histogram, empirical CDF
- **Normalization**: Z-score, min-max, robust scaling
- **Utility Functions**: Min, max, range, sum, sum of squares

## Building

### Prerequisites

- GCC compiler with C11 support
- libcrystalline.so (built first)
- Standard math library (libm)

### Build Commands

```bash
# Build from algorithms directory
cd algorithms
make

# Or build from main directory
cd ..
make libalgorithms.so
```

### Build Output

- **Library**: `libalgorithms.so` (~49KB)
- **Dependencies**: libcrystalline.so, libm.so, libc.so

## Usage Examples

### Example 1: Loss Computation

```c
#include "loss_functions.h"

// Create loss configuration
LossConfig config = loss_config_create(LOSS_CROSS_ENTROPY);
config.reduction = LOSS_REDUCTION_MEAN;
config.label_smoothing = 0.1;

// Compute loss
LossResult result = loss_cross_entropy(
    predictions, targets,
    batch_size, num_classes,
    &config
);

printf("Loss: %f\n", result.loss_value);

// Compute gradients
GradientInfo grad_info = loss_cross_entropy_gradient(
    predictions, targets,
    batch_size, num_classes,
    &config
);

// Clean up
loss_result_free(&result);
gradient_info_free(&grad_info);
```

### Example 2: Optimization

```c
#include "optimizers.h"

// Create optimizer
OptimizerConfig config = optimizer_config_create(OPTIMIZER_ADAM);
config.learning_rate = 0.001;
config.beta1 = 0.9;
config.beta2 = 0.999;

OptimizerState* optimizer = optimizer_state_create(&config, num_parameters);

// Training loop
for (int epoch = 0; epoch < num_epochs; epoch++) {
    // ... compute gradients ...
    
    // Update parameters
    optimizer_step(optimizer, parameters, gradients, num_parameters);
}

// Clean up
optimizer_state_free(optimizer);
```

### Example 3: Numerical Stability

```c
#include "numerical.h"

// Compute softmax with numerical stability
double input[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
double output[5];

numerical_softmax(input, output, 5);

// Check for numerical issues
bool has_nan, has_inf;
numerical_check_array(output, 5, &has_nan, &has_inf);

if (has_nan || has_inf) {
    printf("Numerical issues detected!\n");
}
```

### Example 4: Gradient Accumulation

```c
#include "backprop.h"

// Create gradient buffer
GradientBuffer* buffer = gradient_buffer_create(
    num_parameters,
    GRADIENT_ACCUMULATION_MEAN
);

// Accumulate gradients from multiple sources
for (int i = 0; i < num_batches; i++) {
    // ... compute batch gradients ...
    gradient_buffer_accumulate(buffer, batch_gradients, num_parameters);
}

// Finalize (apply mean)
gradient_buffer_finalize(buffer);

// Get accumulated gradients
double* final_gradients = malloc(num_parameters * sizeof(double));
gradient_buffer_get_gradients(buffer, final_gradients, num_parameters);

// Clean up
gradient_buffer_free(buffer);
free(final_gradients);
```

### Example 5: Statistical Analysis

```c
#include "statistics.h"

double data[100];
// ... fill data ...

// Compute statistics
double mean = stats_mean(data, 100);
double variance = stats_variance(data, 100, true);
double std_dev = stats_std_dev(data, 100, true);
double median = stats_median(data, 100);

printf("Mean: %f, Std Dev: %f, Median: %f\n", mean, std_dev, median);

// Normalize data
stats_z_score_normalize(data, 100);
```

## API Documentation

### Loss Functions

- `LossConfig loss_config_create(LossFunctionType type)`
- `LossResult loss_cross_entropy(...)`
- `LossResult loss_mse(...)`
- `LossResult loss_mae(...)`
- `LossResult loss_huber(...)`
- `GradientInfo loss_*_gradient(...)`

### Optimizers

- `OptimizerConfig optimizer_config_create(OptimizerType type)`
- `OptimizerState* optimizer_state_create(...)`
- `bool optimizer_step(...)`
- `double lr_schedule_get_lr(...)`

### Numerical

- `void numerical_softmax(...)`
- `void numerical_log_softmax(...)`
- `double numerical_log_sum_exp(...)`
- `double numerical_safe_log/exp/divide/sqrt(...)`

### Backpropagation

- `GradientBuffer* gradient_buffer_create(...)`
- `bool gradient_buffer_accumulate(...)`
- `bool gradient_clip_by_norm(...)`
- `GradientStats gradient_compute_stats(...)`

### Statistics

- `double stats_mean/variance/std_dev(...)`
- `double stats_correlation(...)`
- `void stats_z_score_normalize(...)`

## Testing

Tests for the algorithms library will be added in the main test suite:

```bash
cd ../tests
make test_algorithms
./test_algorithms
```

## Performance Considerations

1. **Compilation**: Built with `-O2` optimization
2. **Memory**: Efficient memory usage with minimal allocations
3. **Numerical Stability**: Careful handling of edge cases
4. **Cache Efficiency**: Data structures designed for cache locality

## Dependencies

- **libcrystalline.so**: For arbitrary precision arithmetic (future enhancement)
- **libm.so**: Standard math library (exp, log, sqrt, etc.)
- **libc.so**: Standard C library

## Future Enhancements

1. **Arbitrary Precision**: Use libcrystalline for high-precision computations
2. **SIMD Optimization**: Vectorized operations for performance
3. **GPU Support**: CUDA/OpenCL implementations
4. **More Algorithms**: Additional optimizers and loss functions
5. **Automatic Differentiation**: Full autodiff framework

## License

Part of the Crystalline Lattice project.

## Contributing

When adding new algorithms:
1. Ensure mathematical correctness
2. Add numerical stability checks
3. Provide comprehensive documentation
4. Include usage examples
5. Add unit tests

## Contact

For questions or contributions, see the main Crystalline project README.