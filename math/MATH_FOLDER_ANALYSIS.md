# Math Folder Analysis - Quick Reference

## Overview

The `math/` folder contains sophisticated mathematical models and algorithms that can significantly enhance the price projection system. This document provides a quick reference to the available mathematical foundations.

---

## Folder Structure

```
math/
├── src/
│   ├── mathematical_formulas.c      # 36 core formulas
│   ├── platonic_model/              # High-dimensional geometric models
│   ├── geometric_recovery/          # ECDLP recovery algorithms
│   ├── blind_recovery/               # Universal recovery system
│   ├── statistics.c                  # Statistical analysis functions
│   ├── hierarchical_primes.c        # Prime number system
│   ├── oscillation_decomposition.c  # FFT-based analysis
│   └── [other modules]
└── THESIS.md                         # Comprehensive documentation
```

---

## Mathematical Formulas Catalog

### Category 1: Entropy & Information Theory

| Formula | Purpose | Use Case |
|---------|---------|----------|
| `formula_lbs` | Logarithmic binary entropy | Information content analysis |
| `formula_matrix_entropy` | Matrix-based entropy | Multi-dimensional entropy |
| `formula_hps` | Harmonic probability score | Probability weighting |
| `formula_e_approx` | Exponential approximation | Fast exponential calculations |
| `formula_les` | Logarithmic entropy score | Entropy-based scoring |
| `formula_tlm` | Temporal logarithmic model | Time-series entropy |

### Category 2: Wave Functions

| Formula | Purpose | Use Case |
|---------|---------|----------|
| `formula_wave_z` | Multi-frequency wave synthesis | Price oscillation modeling |
| `formula_psi_mn` | 2D wave function (m,n modes) | Multi-dimensional waves |
| `formula_psm` | Periodic sine modulation | Periodic patterns |
| `formula_eleventh_hg` | 11th harmonic generation | Harmonic analysis |
| `formula_hd` | Harmonic division | Frequency analysis |
| `formula_dps` | Dual periodic synthesis | Dual-frequency patterns |

### Category 3: Tetration & Geometry

| Formula | Purpose | Use Case |
|---------|---------|----------|
| `formula_bga` | Base golden ratio amplification | Golden ratio scaling |
| `formula_tv` | Tetration value | Exponential towers |
| `formula_tvg` | Tetration with golden ratio | φ-based tetration |
| `formula_tv_pi` | Tetration with π scaling | π-based scaling |
| `formula_rif` | Recursive iterative function | Recursive calculations |
| `formula_ivg` | Iterative vector generation | Vector generation |
| `formula_tld` | Tetration logarithmic derivative | Rate of change |

### Category 4: Balance & Quantum

| Formula | Purpose | Use Case |
|---------|---------|----------|
| `formula_balance_bn1` | Binary balance normalization | Equilibrium finding |
| `formula_avd` | Average deviation | Deviation analysis |
| `formula_ndc` | Normalized difference coefficient | Normalized differences |
| `formula_qss` | Quantum state superposition | State combination |
| `formula_pre` | Prime ratio expansion | Prime-based expansion |
| `formula_gnr` | Golden ratio normalization | φ normalization |

### Category 5: Harmonic & Resonance

| Formula | Purpose | Use Case |
|---------|---------|----------|
| `formula_stm` | Symmetric tone modulation | Symmetric patterns |
| `formula_uhh` | Universal harmonic height | Harmonic scaling |
| `formula_pgh` | Prime gap harmonic | Prime-based harmonics |
| `formula_fhs` | Fractional harmonic series | Series analysis |
| `formula_harm_score` | Harmonic scoring | Harmonic evaluation |

---

## Key Mathematical Systems

### 1. Platonic Model System

**Purpose:** High-dimensional geometric structures for price modeling

**Key Features:**
- 13-30+ dimensions
- 2048-16384+ vertices
- Dynamic scaling
- Prime-based projection

**Components:**
- `platonic_model_core.c`: Core structure
- `platonic_model_oscillations.c`: FFT-based oscillation detection
- `tetration_real.c`: Real tetration computation
- `platonic_model_recovery.c`: State recovery
- `platonic_model_scaling.c`: Dynamic scaling

**Use Case:** Geometric price positioning in high-dimensional space

### 2. Geometric Recovery System

**Purpose:** ECDLP recovery using geometric methods

**Key Features:**
- 50 Platonic solid anchors (13D)
- 20-torus structure
- G triangulation
- Oscillation decomposition
- Plateau detection

**Performance:**
- 1.6-5.7x improvement
- 95-100% true k capture
- Scales with bit length

**Use Case:** Precise price recovery and prediction

### 3. Oscillation Decomposition

**Purpose:** FFT-based multi-frequency analysis

**Key Features:**
- Cooley-Tukey FFT
- Power spectrum analysis
- Component extraction
- Residual analysis

**Use Case:** Identify dominant frequencies in price movements

### 4. Statistical Functions

**Purpose:** Comprehensive statistical analysis

**Available Functions:**
- Descriptive: mean, variance, std dev, median, mode, percentiles
- Correlation: Pearson, Spearman
- Distributions: histogram, normal distribution
- Testing: hypothesis testing foundations

**Use Case:** Validate projections, calculate confidence intervals

### 5. Hierarchical Prime System

**Purpose:** Prime number generation with 12-fold symmetry

**Key Features:**
- O(1) deterministic formula for clock positions
- Symmetry group filtering (mod 12)
- Hierarchical partitioning
- Prime cache optimization

**Use Case:** Prime-based calculations in projections

---

## Current JavaScript Implementation

### Location: `assets/js/projections.js`

### Current Model: Crystalline Projection

**Formula Components:**
1. **Theta (θ):** `k·π·φ + n·2π/12 + log(ν)/log(3) + ω/432 + (p²-q²)`
2. **Growth (g):** `g_prev · 3^(θ/100) · (1 + τ/1000)`
3. **Lattice Sum:** `Σ[cos(angle) · polQuad · polMob · ψ · (1 + 0.5·tanh(g/1e5))]`
4. **Price Delta:** `latticeSum · log(depthPrime)/log(2) · 0.5 · max(1, τ)`

**Parameters:**
- `depthPrime`: Prime number (default: 31)
- `omegaHz`: Frequency (default: 432)
- `triad`: Three prime numbers (default: [2, 5, 7])
- `lambdaSchedule`: Schedule array (default: ['dub', 'kubt', "k'anch", ...])
- `N`: Number of steps (default: 120)

---

## Integration Opportunities

### High Priority

1. **Oscillation Analysis**
   - Port FFT to JavaScript
   - Use `formula_wave_z`, `formula_psi_mn`
   - Integrate with projection model

2. **Statistical Validation**
   - Use `statistics.c` functions
   - Calculate accuracy metrics
   - Confidence intervals

3. **Harmonic Analysis**
   - Use `formula_harm_score`, `formula_fhs`
   - Enhance theta calculation
   - Improve growth step

### Medium Priority

1. **Multi-Model Ensemble**
   - Implement multiple formulas
   - Weight by accuracy
   - Combine predictions

2. **Geometric Recovery**
   - Simplified Platonic anchors
   - Torus tracking
   - High-dimensional projection

3. **Tetration Integration**
   - Simplified tetration
   - Use in growth calculations
   - Golden ratio amplification

### Low Priority

1. **Advanced Formulas**
   - Entropy-based models
   - Quantum state superposition
   - Crystalline dimension

---

## Formula Mapping to Projections

### For Price Oscillations
- `formula_wave_z`: Multi-frequency price waves
- `formula_psi_mn`: 2D price patterns
- `formula_fhs`: Fractional harmonic series

### For Growth/Decay
- `formula_tv`: Tetration-based growth
- `formula_tvg`: Golden ratio growth
- `formula_pre`: Prime ratio expansion

### For Balance/Equilibrium
- `formula_balance_bn1`: Price equilibrium
- `formula_qss`: State superposition
- `formula_gnr`: Golden ratio normalization

### For Validation
- `statistics.c`: All validation functions
- Correlation analysis
- Distribution fitting

---

## Performance Characteristics

### C Library (math/)
- **Speed:** Optimized C code
- **Precision:** High (double precision)
- **Scalability:** Handles high dimensions
- **Limitation:** Not directly accessible from JavaScript

### JavaScript (projections.js)
- **Speed:** Browser-dependent
- **Precision:** JavaScript number precision
- **Scalability:** Limited by browser
- **Advantage:** Direct integration with UI

### Integration Strategy
- Port key algorithms to JavaScript
- Use Web Workers for heavy computation
- Cache intermediate results
- Progressive rendering

---

## Recommended Reading Order

1. **Start Here:**
   - `PROJECTION_IMPROVEMENT_PLAN.md` (this document's companion)
   - `assets/js/projections.js` (current implementation)

2. **Mathematical Foundations:**
   - `mathematical_formulas.c` (36 formulas)
   - `statistics.c` (statistical functions)
   - `oscillation_decomposition.c` (FFT analysis)

3. **Advanced Systems:**
   - `platonic_model/` (geometric models)
   - `geometric_recovery/` (recovery algorithms)
   - `hierarchical_primes.c` (prime system)

4. **Deep Dive:**
   - `THESIS.md` (comprehensive documentation)

---

## Quick Reference: Key Constants

### From projections.js
- `PHI_D = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 31]`
- `PRIME_STOPS = [11, 13, 17, 29, 31, 47, 59, 61, 97, 101]`
- `SECTORS = 12`
- `LAMBDA_DEFAULT = ['dub', 'kubt', "k'anch", 'dub', 'kubt', "k'anch"]`
- `PRIMES_500`: First 500 prime numbers

### Mathematical Constants
- Golden Ratio (φ): `(1 + √5) / 2 ≈ 1.618`
- π: `3.14159...`
- Base frequency: `432 Hz`

---

## Next Steps

1. **Review:** `PROJECTION_IMPROVEMENT_PLAN.md` for detailed roadmap
2. **Identify:** Which formulas/models to port first
3. **Implement:** Start with statistical validation
4. **Test:** Validate improvements with backtesting
5. **Iterate:** Gradually add more sophisticated models

---

**Document Version:** 1.0  
**Last Updated:** 2024-12-XX  
**Status:** Quick Reference Guide






