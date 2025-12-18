# Legacy Geometric Recovery Files

This directory contains the **OLD crypto-specific** geometric recovery implementation that has been deprecated in favor of the new general-purpose modular system.

## Why These Files Are Here

These files were part of the original Bitcoin ECDLP-specific implementation that used:
- OpenSSL library (BIGNUM, EC_POINT, EC_GROUP types)
- Crypto-specific algorithms
- Bitcoin secp256k1 curve specifics

The new system is **general-purpose** and works with ANY system, not just cryptography.

## Directory Structure

### `geometric_recovery/` - OLD Implementation Files
- `geometric_recovery.c` - OLD main implementation (crypto-specific)
- `*_reference.c` - Reference implementations with OpenSSL
- `oscillation_vector.c` - OLD oscillation tracking (crypto-specific)
- `quadrant_polarity.c` - OLD quadrant system (crypto-specific)
- `shared_geometry.c` - OLD shared geometry (crypto-specific)
- `geometric_utils.c` - OLD k↔position mapping (uses BIGNUM)
- `anchor_tracking.c` - OLD anchor tracking
- `geometric_anchors.c` - OLD anchor system
- `ecdsa_sample_loader.c` - ECDSA-specific loader
- `integrated_recovery.c` - OLD integrated system

### `include/` - OLD Headers
- `geometric_recovery.h` - OLD main header (uses OpenSSL types)

### `tests/` - OLD Tests
- `test_geometric_recovery.c` - Tests for OLD system
- `test_secp128_*.c` - Bitcoin secp128 curve tests
- `test_*_validation.c` - OLD validation tests

## New System Location

The **NEW general-purpose** system is located at:
- Headers: `algorithms/include/geometric_recovery/`
- Implementation: `algorithms/src/geometric_recovery/`
- Main orchestrator: `geometric_recovery_orchestrator.h/c`

## Key Differences

### OLD System (This Directory)
```c
// Uses OpenSSL types
BIGNUM* search_torus_orbit(
    EC_GROUP* ec_group,
    EC_POINT* target_Q,
    const BIGNUM** anchor_k_values,
    ...
);
```

### NEW System (Active Code)
```c
// Uses standard types
int geometric_recovery_orchestrator_execute(
    GeometricRecoveryOrchestrator* orch,
    uint64_t target,
    uint64_t* result_min,
    uint64_t* result_max,
    double* reduction_factor
);
```

## Migration Status

✅ **COMPLETED** - All active code now uses the NEW general-purpose system
- No OpenSSL dependencies in active code
- All crypto-specific files archived here
- Build system updated to exclude legacy files
- Tests updated or archived

## If You Need These Files

These files are preserved for:
1. **Historical reference** - Understanding the evolution of the system
2. **Crypto-specific research** - If you need Bitcoin ECDLP specifics
3. **Comparison** - Comparing OLD vs NEW approaches

To use these files, you would need to:
1. Install OpenSSL development libraries
2. Update includes to point to this directory
3. Link against OpenSSL (-lssl -lcrypto)

## Recommended Approach

**Use the NEW system** in `algorithms/include/geometric_recovery/` instead.

The NEW system is:
- ✅ General-purpose (works with ANY system)
- ✅ Modular (clean component architecture)
- ✅ No external crypto dependencies
- ✅ Better documented
- ✅ Actively maintained

## Questions?

See the main README and documentation in the active `geometric_recovery/` directory.