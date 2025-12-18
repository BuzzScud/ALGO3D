# Crystalline Math REST API

A comprehensive REST API providing access to 112+ mathematical functions from the Crystalline Mathematics Library.

## 🚀 Quick Start

### Installation

1. Copy the `api` directory to your web server's document root under `math/`:
   ```bash
   cp -r php/math/api /var/www/html/math
   ```

2. Ensure PHP extensions are loaded:
   ```bash
   php -m | grep crystalline_math
   php -m | grep algorithms
   ```

3. Set proper permissions:
   ```bash
   chmod 755 /var/www/html/math
   chmod 644 /var/www/html/math/index.php
   chmod 644 /var/www/html/math/calculator.html
   ```

4. Access the API:
   - API Endpoint: `http://your-domain.com/math/`
   - Calculator: `http://your-domain.com/math/calculator.html`

## 📖 API Documentation

### Base URL
```
http://your-domain.com/math/
```

### Request Format
All requests use GET method:
```
GET /?operation=function_name&param1=value1&param2=value2
```

### Response Format
All responses are JSON:
```json
{
  "success": true,
  "result": 42
}
```

Error responses:
```json
{
  "success": false,
  "error": "Error message"
}
```

## 🔢 Available Operations

### Arithmetic Operations
- `add` - Addition: `?operation=add&a=5&b=3`
- `sub` - Subtraction: `?operation=sub&a=10&b=3`
- `mul` - Multiplication: `?operation=mul&a=4&b=5`
- `div` - Division: `?operation=div&a=20&b=4`
- `mod` - Modulo: `?operation=mod&a=17&b=5`
- `abs` - Absolute value: `?operation=abs&x=-5`
- `min` - Minimum: `?operation=min&a=3&b=7`
- `max` - Maximum: `?operation=max&a=3&b=7`
- `clamp` - Clamp value: `?operation=clamp&x=15&min=0&max=10`
- `sign` - Sign of number: `?operation=sign&x=-5`
- `floor` - Floor: `?operation=floor&x=3.7`
- `ceil` - Ceiling: `?operation=ceil&x=3.2`
- `round` - Round: `?operation=round&x=3.5`
- `trunc` - Truncate: `?operation=trunc&x=3.9`

### Power & Root Operations
- `sqrt` - Square root: `?operation=sqrt&x=16`
- `cbrt` - Cube root: `?operation=cbrt&x=27`
- `pow` - Power: `?operation=pow&base=2&exp=10`
- `exp` - Exponential: `?operation=exp&x=2`
- `expm1` - exp(x)-1: `?operation=expm1&x=0.1`

### Logarithmic Operations
- `log` - Natural log: `?operation=log&x=10`
- `log10` - Base-10 log: `?operation=log10&x=100`
- `log2` - Base-2 log: `?operation=log2&x=8`
- `log1p` - log(1+x): `?operation=log1p&x=0.1`

### Trigonometric Operations
- `sin` - Sine: `?operation=sin&x=1.5708`
- `cos` - Cosine: `?operation=cos&x=0`
- `tan` - Tangent: `?operation=tan&x=0.785`
- `asin` - Arcsine: `?operation=asin&x=0.5`
- `acos` - Arccosine: `?operation=acos&x=0.5`
- `atan` - Arctangent: `?operation=atan&x=1`
- `atan2` - Two-argument arctangent: `?operation=atan2&y=1&x=1`

### Hyperbolic Operations
- `sinh` - Hyperbolic sine: `?operation=sinh&x=1`
- `cosh` - Hyperbolic cosine: `?operation=cosh&x=1`
- `tanh` - Hyperbolic tangent: `?operation=tanh&x=1`
- `asinh` - Inverse hyperbolic sine: `?operation=asinh&x=1`
- `acosh` - Inverse hyperbolic cosine: `?operation=acosh&x=2`
- `atanh` - Inverse hyperbolic tangent: `?operation=atanh&x=0.5`

### Validation Operations
- `is_nan` - Check if NaN: `?operation=is_nan&x=0`
- `is_inf` - Check if infinite: `?operation=is_inf&x=1e308`
- `is_finite` - Check if finite: `?operation=is_finite&x=100`
- `approx_equal` - Approximate equality: `?operation=approx_equal&a=0.1&b=0.10000001`

### Prime Number Operations
- `is_prime` - Check primality: `?operation=is_prime&n=17`
- `prime_generate` - Generate prime (O(1)): `?operation=prime_generate&position=3&magnitude=1`
- `prime_nth` - Get nth prime: `?operation=prime_nth&n=10`
- `prime_next` - Next prime: `?operation=prime_next&n=17`
- `prime_prev` - Previous prime: `?operation=prime_prev&n=17`
- `prime_count_below` - Count primes below n: `?operation=prime_count_below&n=100`
- `prime_count_range` - Count primes in range: `?operation=prime_count_range&start=10&end=100`
- `prime_gap_next` - Next prime gap: `?operation=prime_gap_next&n=17`
- `prime_gap_prev` - Previous prime gap: `?operation=prime_gap_prev&n=17`
- `prime_are_coprime` - Check if coprime: `?operation=coprime&a=15&b=28`
- `prime_totient` - Euler's totient: `?operation=totient&n=12`
- `prime_index` - Get prime index: `?operation=prime_index&prime=17`

### Rainbow Table Operations
- `rainbow_lookup_by_index` - Lookup by index: `?operation=rainbow_lookup_by_index&index=10`
- `rainbow_lookup_index` - Get index: `?operation=rainbow_lookup_index&prime=29`
- `rainbow_next_prime` - Next prime: `?operation=rainbow_next_prime&prime=29`
- `rainbow_prev_prime` - Previous prime: `?operation=rainbow_prev_prime&prime=29`
- `rainbow_contains` - Check if in table: `?operation=rainbow_contains&prime=31`
- `rainbow_size` - Get table size: `?operation=rainbow_size`
- `rainbow_max_prime` - Get max prime: `?operation=rainbow_max_prime`

### Clock Lattice Operations
- `clock_map_prime` - Map prime to position: `?operation=clock_map_prime&prime=17`
- `clock_position_to_prime` - Position to prime: `?operation=clock_position_to_prime&ring=0&position=3`
- `clock_is_valid_position` - Validate position: `?operation=clock_is_valid_position&ring=0&position=5`
- `clock_reverse_lookup` - Reverse lookup: `?operation=clock_reverse_lookup&number=17`

### Platonic Solid Operations
- `platonic_tetrahedron` - Generate tetrahedron: `?operation=tetrahedron`
- `platonic_cube` - Generate cube: `?operation=cube`
- `platonic_octahedron` - Generate octahedron: `?operation=octahedron`
- `platonic_dodecahedron` - Generate dodecahedron: `?operation=dodecahedron`
- `platonic_icosahedron` - Generate icosahedron: `?operation=icosahedron`
- `platonic_simplex` - Generate n-simplex: `?operation=simplex&dimension=4`
- `platonic_hypercube` - Generate hypercube: `?operation=hypercube&dimension=4`
- `platonic_cross_polytope` - Generate cross-polytope: `?operation=cross_polytope&dimension=4`

### Statistics Operations (requires algorithms extension)
- `stats_mean` - Mean: `?operation=mean&data=[1,2,3,4,5]`
- `stats_variance` - Variance: `?operation=variance&data=[1,2,3,4,5]`
- `stats_std_dev` - Standard deviation: `?operation=std_dev&data=[1,2,3,4,5]`
- `stats_median` - Median: `?operation=median&data=[1,2,3,4,5]`
- `stats_min` - Minimum: `?operation=stats_min&data=[1,2,3,4,5]`
- `stats_max` - Maximum: `?operation=stats_max&data=[1,2,3,4,5]`

### Special Operations
- `help` - API documentation: `?operation=help`
- `list` - List all functions: `?operation=list`

## 📊 Examples

### Basic Arithmetic
```bash
curl "http://your-domain.com/math/?operation=add&a=5&b=3"
# {"success":true,"result":8}

curl "http://your-domain.com/math/?operation=sqrt&x=16"
# {"success":true,"result":4}
```

### Prime Numbers
```bash
curl "http://your-domain.com/math/?operation=is_prime&n=17"
# {"success":true,"result":true,"number":17}

curl "http://your-domain.com/math/?operation=prime_generate&position=3&magnitude=1"
# {"success":true,"result":17,"position":3,"magnitude":1}

curl "http://your-domain.com/math/?operation=totient&n=12"
# {"success":true,"result":4,"number":12}
```

### Rainbow Table
```bash
curl "http://your-domain.com/math/?operation=rainbow_lookup_by_index&index=10"
# {"success":true,"result":29,"index":10}

curl "http://your-domain.com/math/?operation=rainbow_next_prime&prime=29"
# {"success":true,"result":31,"after":29}
```

### Clock Lattice
```bash
curl "http://your-domain.com/math/?operation=clock_map_prime&prime=17"
# {"success":true,"result":{"ring":0,"position":3,"angle":1.5708,"radius":1},"prime":17}
```

### Trigonometry
```bash
curl "http://your-domain.com/math/?operation=sin&x=1.5708"
# {"success":true,"result":1}

curl "http://your-domain.com/math/?operation=cos&x=0"
# {"success":true,"result":1}
```

## 🎨 Web Calculator

A beautiful HTML5 calculator is included at `calculator.html`. Features:
- Basic arithmetic operations
- Scientific functions (trig, log, exp)
- Prime number operations
- Rainbow table lookups
- Clock lattice mapping
- Statistics calculations
- History tracking
- Responsive design

Access it at: `http://your-domain.com/math/calculator.html`

## 🔒 Security Notes

1. **Input Validation**: All inputs are validated and sanitized
2. **CORS**: Enabled for cross-origin requests
3. **Rate Limiting**: Consider implementing rate limiting for production
4. **HTTPS**: Always use HTTPS in production

## 🐛 Error Handling

The API returns appropriate HTTP status codes:
- `200` - Success
- `400` - Bad request (missing parameters, invalid input)
- `404` - Unknown operation
- `500` - Server error
- `501` - Not implemented (missing extension)

## 📝 Notes

- Rainbow table is automatically initialized with 1000 primes on first use
- Clock lattice is automatically initialized on first use
- All operations use the Crystalline Math library (no standard PHP math functions)
- Results are arbitrary precision where applicable

## 🚀 Performance

- **Arithmetic**: O(1)
- **Prime generation**: O(1) using clock lattice
- **Rainbow table lookup**: O(1)
- **Primality testing**: O(√n)
- **Trigonometric**: O(1)

## 📚 Additional Resources

- [Crystalline Math Library Documentation](../../README.md)
- [PHP Extension Documentation](../README.md)
- [Example Scripts](../../examples/)

## 🤝 Support

For issues or questions, please refer to the main project repository.

---

**Powered by Crystalline Mathematics Library**