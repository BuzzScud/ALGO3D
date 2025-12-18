# Chapter 2: Babylonian Mathematics - Prototype

## Overview

This prototype demonstrates the Babylonian base-60 (sexagesimal) number system as described in Chapter 2 of the thesis.

## Files

- `base60_converter.c` - Complete base-60 arithmetic implementation
- `Makefile` - Build configuration

## Features

### Conversion
- Decimal to base-60 conversion
- Base-60 to decimal conversion
- Support for fractional numbers
- Configurable precision

### Arithmetic Operations
- Addition in base-60
- Multiplication in base-60
- Proper carry handling

### Examples Included
1. Converting 3661 to base-60 (1,1,1)
2. Fractional numbers (1.5 = 1;30)
3. Pi approximation (π ≈ 3;8,29,44,0)
4. Addition (1.5 + 2.25 = 3.75)
5. Multiplication (2.0 × 3.0 = 6.0)
6. The number 60 itself (1,0)

## Building

```bash
make
```

## Running

### Demonstration Mode
```bash
./base60_converter
```

This runs all examples and shows how base-60 arithmetic works.

### Convert Specific Values
```bash
./base60_converter <decimal_value> [precision]
```

Examples:
```bash
./base60_converter 3661 0          # Integer conversion
./base60_converter 3.141592 4      # Pi with 4 fractional digits
./base60_converter 60 0            # The number 60
./base60_converter 1.5 2           # 1.5 = 1;30 in base-60
```

## Understanding Base-60 Notation

### Integer Part
Numbers are written with commas separating digits:
- `1,1,1` = 1×60² + 1×60¹ + 1×60⁰ = 3600 + 60 + 1 = 3661

### Fractional Part
Semicolon separates integer and fractional parts:
- `1;30` = 1 + 30/60 = 1.5
- `3;8,29,44` ≈ π (pi)

### Why Base-60?

1. **Highly Divisible**: 60 has many factors (1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60)
2. **Practical**: Easy to divide into halves, thirds, quarters, fifths, sixths
3. **Historical**: Used by Babylonians for astronomy and mathematics
4. **Modern Legacy**: Still used for time (60 seconds, 60 minutes) and angles (360° = 6×60)

## Connection to Thesis

This prototype demonstrates the foundational concepts from Chapter 2:

- **Base-60 Arithmetic**: The Babylonian number system
- **Precision**: How fractional values are represented
- **Divisibility**: Why 60 is mathematically elegant
- **Historical Context**: Ancient mathematical wisdom

The base-60 system is fundamental to the CrystallineAbacus architecture described later in the thesis.

## Testing

```bash
make test
```

This runs the converter with various test cases to verify correctness.

## Cleanup

```bash
make clean
```

---

[← Back to Prototypes](../)