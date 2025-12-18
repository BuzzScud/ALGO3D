# Mathematical Framework Documentation

This directory contains the complete mathematical specification for the Crystalline Lattice arbitrary precision math library.

## Contents

### Formula Images (Authoritative Source)
1. `01_lattice_formula.jpeg` - Core lattice formula L(n,d,k,λ)
2. `02_lattice_components.jpeg` - Component breakdown
3. `03_variable_definitions.jpeg` - Variable definitions table
4. `04_unabridged_formula.jpeg` - Complete unabridged formula
5. `05_pythagorean_triples.jpeg` - Pythagorean triple system
6. `06_framework_overview.jpeg` - Framework overview
7. `07_complete_symbol_table.jpeg` - Complete symbol table (part 1)
8. `08_symbol_table_continued.jpeg` - Complete symbol table (part 2)

### Documentation
- `COMPLETE_MATHEMATICAL_SPECIFICATION.md` - Full mathematical specification derived from images

## Key Mathematical Components

### Master Formula
```
ℒ(n, d, k, λ, ω, ψ) = 3^(O(n,k,λ)) × ∏ᵢ₌₁ᵈ cos(θ(n,k,λ,ω,ψ)·φᵢ) × Γ(k) × ν(λ) × (ω) × Ψ(ψ) × Γ(n,d)
```

### Core Symbols
- **n**: Prime/element index (1, 2, 3, ...)
- **d**: Dimension (0, 1, 2, ..., 12)
- **k**: Spiral index / Chant step
- **λ**: Phonetic / Cultural layer
- **ω**: Cymatic / Vibrational layer (432 Hz, 528 Hz, ...)
- **ψ**: Plimpton 322 triple layer
- **θ**: Angular position
- **φ**: Golden ratio (1+√5)/2
- **φᵢ**: Dimensional frequencies [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, ...]

### Critical Constants
- **144000**: Vector culmination = 3 × 12³ × (250/9)
- **3/144000**: Einstein's Λ correction
- **143999, 144001**: Twin primes (lattice twins)
- **355/113**: π dust (Zu Chongzhi approximation)
- **432**: Cymatic base (432 Hz triad chant)
- **7.83**: Schumann resonance (Earth)

## Implementation Status

See `COMPLETE_MATHEMATICAL_SPECIFICATION.md` for detailed implementation status of each component.

## Usage

These images and specifications are the authoritative source for all mathematical implementations in the Crystalline library. Any code changes must maintain consistency with these formulas.