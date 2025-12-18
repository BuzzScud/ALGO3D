# Crystalline Math PHP Module

Complete PHP bindings for the Crystalline Math Library with REST API endpoints.

## Installation

```bash
cd php/math
phpize
./configure
make
sudo make install
```

Add to php.ini:
```ini
extension=crystalline_math.so
```

## Available Functions

### Arithmetic Operations
- `math_add($a, $b)` - Addition
- `math_sub($a, $b)` - Subtraction
- `math_mul($a, $b)` - Multiplication
- `math_div($a, $b)` - Division
- `math_mod($a, $b)` - Modulo
- `math_abs($x)` - Absolute value
- `math_min($a, $b)` - Minimum
- `math_max($a, $b)` - Maximum
- `math_clamp($x, $min, $max)` - Clamp value
- `math_sign($x)` - Sign (-1, 0, 1)
- `math_floor($x)` - Floor
- `math_ceil($x)` - Ceiling
- `math_round($x)` - Round
- `math_trunc($x)` - Truncate

### Transcendental Functions
- `math_sqrt($x)` - Square root
- `math_cbrt($x)` - Cube root
- `math_pow($x, $y)` - Power
- `math_exp($x)` - Exponential
- `math_log($x)` - Natural logarithm
- `math_log10($x)` - Base-10 logarithm
- `math_log2($x)` - Base-2 logarithm
- `math_sin($x)` - Sine
- `math_cos($x)` - Cosine
- `math_tan($x)` - Tangent
- `math_asin($x)` - Arcsine
- `math_acos($x)` - Arccosine
- `math_atan($x)` - Arctangent
- `math_atan2($y, $x)` - Two-argument arctangent
- `math_sinh($x)` - Hyperbolic sine
- `math_cosh($x)` - Hyperbolic cosine
- `math_tanh($x)` - Hyperbolic tangent

### Prime Number Operations
- `prime_nth($n)` - Get nth prime
- `prime_next($n)` - Next prime after n
- `prime_prev($n)` - Previous prime before n
- `prime_is_prime($n)` - Check if prime
- `prime_count_below($n)` - Count primes below n
- `prime_factor($n)` - Prime factorization

### Complex Numbers
- `complex_create($real, $imag)` - Create complex number
- `complex_add($a, $b)` - Add complex numbers
- `complex_mul($a, $b)` - Multiply complex numbers
- `complex_magnitude($z)` - Get magnitude
- `complex_phase($z)` - Get phase

### Arbitrary Precision (Abacus)
- `abacus_create()` - Create new abacus
- `abacus_from_string($str)` - Create from string
- `abacus_add($a, $b)` - Add two abacus numbers
- `abacus_mul($a, $b)` - Multiply two abacus numbers
- `abacus_to_string($abacus)` - Convert to string

## REST API Endpoints

All endpoints accept JSON POST requests and return JSON responses.

### Base URL
```
http://localhost:8080/api/math/
```

### Endpoints

#### Arithmetic
- `POST /api/math/add` - Addition
- `POST /api/math/subtract` - Subtraction
- `POST /api/math/multiply` - Multiplication
- `POST /api/math/divide` - Division

#### Transcendental
- `POST /api/math/sqrt` - Square root
- `POST /api/math/sin` - Sine
- `POST /api/math/cos` - Cosine
- `POST /api/math/exp` - Exponential
- `POST /api/math/log` - Logarithm

#### Prime Numbers
- `POST /api/math/prime/nth` - Get nth prime
- `POST /api/math/prime/next` - Next prime
- `POST /api/math/prime/is_prime` - Check primality
- `POST /api/math/prime/factor` - Factorization

#### Complex Numbers
- `POST /api/math/complex/add` - Add complex
- `POST /api/math/complex/multiply` - Multiply complex
- `POST /api/math/complex/magnitude` - Get magnitude

## Examples

See `examples/` directory for complete examples of every function.