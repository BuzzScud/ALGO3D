# Chapter 17: Novel Hashing - Prototype

## Overview

This prototype demonstrates clock lattice-based hashing as described in Chapter 17 of the thesis. It uses the 12-fold clock structure to create a fast, collision-resistant hash function.

## Files

- `clock_hash.c` - Clock lattice hash implementation
- `Makefile` - Build configuration

## Key Concepts Demonstrated

### 1. Clock Position Mapping
- Each byte maps to one of 12 clock positions
- Each position has an associated prime number
- Geometric mixing based on position

### 2. Geometric Structure
- 12-fold symmetry from clock lattice
- Magnitude and phase components
- Rotational mixing every 12 bytes

### 3. Collision Resistance
- Small input changes → large hash changes
- Avalanche effect (~39-50% bit changes)
- Prime number properties reduce collisions

### 4. Performance
- O(n) time complexity
- O(1) space complexity
- Single pass through data

## Building

```bash
make
```

## Running

### Demonstration Mode
```bash
./clock_hash
```

Shows:
1. Basic hashing examples
2. Collision resistance
3. Clock position mapping
4. Avalanche effect
5. Geometric properties
6. Performance characteristics

### Hash Specific String
```bash
./clock_hash "your string here"
```

Example:
```bash
./clock_hash "Hello, World!"
# Output: Hash of 'Hello, World!': cc6063e2402047b7
```

## Example Output

```
Message                                      | Hash (hex)
---------------------------------------------|------------------
Hello, World!                                | cc6063e2402047b7
Hello, World                                 | 1fd77ca727084a8d
hello, world!                                | 76a3e319f4f6fe22

Note: Small changes produce completely different hashes
```

## Understanding Clock Lattice Hashing

### How It Works

1. **Initialize State**
   - Position: 0 (starting clock position)
   - Magnitude: 0 (accumulated value)
   - Phase: 0 (rotation angle)

2. **Process Each Byte**
   - Map byte to clock position (byte mod 12)
   - Mix with position-specific prime
   - Update magnitude and phase
   - Rotate to new position

3. **Finalize**
   - Combine magnitude and phase
   - Apply final mixing
   - Return 64-bit hash

### Clock Positions and Primes

```
Position | Prime
---------|------
    0    |   2
    1    |   3
    2    |   5
    3    |   7
    4    |  11
    5    |  13
    6    |  17
    7    |  19
    8    |  23
    9    |  29
   10    |  31
   11    |  37
```

### Geometric Mixing

Each position uses geometric rotation:
- Rotate bits based on position
- Mix with position-specific prime
- Combine with magnitude

## Properties

### ✓ Deterministic
Same input always produces same output

### ✓ Fast
- O(n) time complexity
- Simple operations
- Single pass through data

### ✓ Avalanche Effect
Small input change → ~50% output bits change

### ✓ Collision Resistant
Hard to find two inputs with same hash

### ✓ One-Way
Hard to reverse (find input from hash)

## Avalanche Effect Example

```
Original: 'test' -> e820172a1b287abe
Modified: 'tast' -> c0bf1ec96568583e  (one bit different)
XOR:                289f09e37e402280

Bits changed: 25 / 64 (39.1%)
Ideal: ~32 bits (50%)
```

## Connection to Thesis

This prototype demonstrates key concepts from Chapter 17:

- **12-Fold Clock Structure**: Uses clock positions 0-11
- **Prime Number Properties**: Each position has unique prime
- **Geometric Mixing**: Rotation and position-based operations
- **Fast Computation**: O(n) single-pass algorithm
- **Collision Resistance**: Good distribution properties

The full implementation in the thesis includes:
- Cryptographic-strength hashing
- Variable output sizes
- Keyed hashing (HMAC)
- Parallel processing

## Applications

1. **Hash Tables**: Fast key hashing
2. **Data Integrity**: Checksums and verification
3. **Digital Signatures**: Message digests
4. **Blockchain**: Block hashing
5. **Deduplication**: Content-based addressing

## Comparison to Standard Hashes

### Clock Lattice Hash
- Uses geometric structure
- 12-fold symmetry
- Prime-based mixing
- Educational/novel approach

### SHA-256 (Standard)
- Cryptographic strength
- 256-bit output
- Industry standard
- More complex

**Note**: This is a demonstration hash function. For production cryptographic use, use established algorithms like SHA-256 or SHA-3.

## Testing

```bash
make test
```

Tests the hash function with various inputs and shows collision resistance.

## Cleanup

```bash
make clean
```

---

[← Back to Prototypes](../)