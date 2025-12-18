# Chapter 6: Crystalline Abacus - Prototype

## Overview

This prototype demonstrates the CrystallineAbacus structure as described in Chapter 6 of the thesis. It shows the key concepts of sparse representation, arbitrary precision, and base-60 arithmetic.

## Files

- `simple_abacus.c` - Simplified CrystallineAbacus implementation
- `Makefile` - Build configuration

## Key Concepts Demonstrated

### 1. Sparse Representation
Only non-zero beads are stored, saving memory:
- 3600 = 1×60² requires only 1 bead (not 3)
- Memory savings: 66% for this example

### 2. Bead Structure
Each bead has:
- **value**: 0-59 (for base-60)
- **weight_exponent**: Power of 60 (e.g., 2 for 60², -1 for 60⁻¹)

### 3. Arbitrary Precision
Can represent numbers with any desired precision:
- π with precision 2: error ~2×10⁻⁴
- π with precision 4: error ~6×10⁻⁸
- π with precision 8: error ~3×10⁻¹⁵

### 4. Arithmetic Operations
- Addition: Combines beads from both operands
- Multiplication: Multiplies values and adds exponents

## Building

```bash
make
```

## Running

### Demonstration Mode
```bash
./simple_abacus
```

Shows:
1. Sparse representation advantage
2. Arbitrary precision with π
3. Arithmetic operations (addition, multiplication)

### Convert Specific Values
```bash
./simple_abacus <value> [precision]
```

Examples:
```bash
./simple_abacus 3.141592653589793 6    # Pi with 6 fractional digits
./simple_abacus 3600 0                  # Large integer (sparse)
./simple_abacus 1.5 2                   # Simple fraction
```

## Example Output

```
Converting 3.141592653589793 to CrystallineAbacus (base=60, precision=6)

CrystallineAbacus (base=60, precision=6, beads=6):
  Bead 0: value= 3, weight=60^0 (contributes 3.0000000000)
  Bead 1: value= 8, weight=60^-1 (contributes 0.1333333333)
  Bead 2: value=29, weight=60^-2 (contributes 0.0080555556)
  Bead 3: value=44, weight=60^-3 (contributes 0.0002037037)
  Bead 4: value=47, weight=60^-5 (contributes 0.0000000604)
  Bead 5: value=25, weight=60^-6 (contributes 0.0000000005)
  Total value: 3.141592653570816
```

## Understanding the Structure

### Sparse Representation
Traditional representation of 3600 in base-60:
```
Position:  2    1    0
Value:     1    0    0
```

Sparse representation (CrystallineAbacus):
```
Bead 0: value=1, weight_exponent=2
```

Only stores the non-zero bead!

### Weight Exponents
- Positive exponents: Integer part (60¹, 60², 60³, ...)
- Zero exponent: Ones place (60⁰ = 1)
- Negative exponents: Fractional part (60⁻¹, 60⁻², 60⁻³, ...)

### Example: 1.5 in base-60
```
1.5 = 1 + 30/60
    = 1×60⁰ + 30×60⁻¹

Beads:
  Bead 0: value=1, weight_exponent=0
  Bead 1: value=30, weight_exponent=-1
```

## Connection to Thesis

This prototype demonstrates the foundational concepts from Chapter 6:

- **Sparse Representation**: Only store non-zero values
- **Base-60 Arithmetic**: Babylonian number system
- **Arbitrary Precision**: Configurable accuracy
- **Bead Structure**: Weight-based representation
- **Memory Efficiency**: Sparse storage saves space

The full CrystallineAbacus implementation in the main codebase (`math/src/bigint/abacus.c`) includes:
- Modular arithmetic
- GCD operations
- Fractional support
- Optimized algorithms

## Testing

```bash
make test
```

Runs the abacus with various test cases.

## Cleanup

```bash
make clean
```

---

[← Back to Prototypes](../)