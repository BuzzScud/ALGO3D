# Chapter 13: NTT Algorithms - Prototype

## Overview

This prototype demonstrates the Number Theoretic Transform (NTT) as described in Chapter 13 of the thesis. NTT is like FFT but works in modular arithmetic, providing exact results without floating-point errors.

## Files

- `ntt_demo.c` - Complete NTT implementation
- `Makefile` - Build configuration

## Key Concepts Demonstrated

### 1. Exact Arithmetic
- Uses modular arithmetic (integers mod prime)
- No rounding errors
- Results are exact
- Critical for cryptography

### 2. Fast Polynomial Multiplication
- O(n log n) complexity
- 100x faster than naive O(n²) for n=1000
- Uses Cooley-Tukey algorithm

### 3. Connection to Clock Lattice
- 12-fold symmetry in clock lattice
- 12th roots of unity in NTT
- Both use modular arithmetic
- Geometric structure for efficiency

### 4. Primitive Roots of Unity
- Uses prime modulus p = 12289 = 3 × 2¹² + 1
- Primitive root = 11
- Allows NTT of size up to 4096

## Building

```bash
make
```

## Running

### Demonstration Mode
```bash
./ntt_demo
```

Shows:
1. Polynomial multiplication example
2. Connection to clock lattice
3. Exact arithmetic advantages
4. Performance characteristics

## Example Output

```
Polynomial A: 1 + 2x + 3x^2
Polynomial B: 4 + 5x + 6x^2

Result (using NTT):
4 + 13x + 28x^2 + 27x^3 + 18x^4

Expected: 4 + 13x + 28x^2 + 27x^3 + 18x^4
Got:      4 + 13x + 28x^2 + 27x^3 + 18x^4  ✓ EXACT!
```

## Understanding NTT

### What is NTT?
Number Theoretic Transform is the integer/modular arithmetic version of FFT (Fast Fourier Transform).

### Why NTT?
1. **Exact Results**: No floating-point errors
2. **Fast**: O(n log n) complexity
3. **Cryptographic**: Exact values required
4. **Scalable**: No error accumulation

### How it Works
1. Choose prime modulus p where p = k × 2ⁿ + 1
2. Find primitive root of unity
3. Apply Cooley-Tukey algorithm
4. All operations in modular arithmetic

## NTT vs FFT

### FFT (Floating Point)
- Uses complex numbers
- Approximate results
- Rounding errors accumulate
- Example: 1.0000000001 ≈ 1

### NTT (Modular Arithmetic)
- Uses integers mod prime
- Exact results
- No rounding errors
- Example: 1 = 1 (exactly)

## Connection to Thesis

This prototype demonstrates key concepts from Chapter 13:

- **Exact Arithmetic**: No floating-point errors
- **Clock Lattice Connection**: 12-fold symmetry
- **Fast Computation**: O(n log n) complexity
- **Modular Arithmetic**: Prime modulus
- **Geometric Structure**: Roots of unity

The full implementation in the thesis includes:
- Multi-dimensional NTT
- Optimized algorithms
- Hardware acceleration
- Cryptographic applications

## Performance

### Complexity
- **Naive multiplication**: O(n²)
- **NTT multiplication**: O(n log n)

### Speedup Examples
- n = 1,000: ~100x faster
- n = 10,000: ~1,000x faster
- n = 100,000: ~10,000x faster

## Applications

1. **Polynomial Multiplication**: Fast and exact
2. **Cryptography**: RSA, lattice-based crypto
3. **Signal Processing**: Exact convolution
4. **Large Integer Arithmetic**: Fast multiplication

## Testing

```bash
make test
```

Runs the demo and verifies polynomial multiplication.

## Cleanup

```bash
make clean
```

---

[← Back to Prototypes](../)