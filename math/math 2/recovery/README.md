# Recovery System - Production Release

Unified recovery system with comprehensive format support and production-ready tools.

## Features
- **Signal Recovery**: WAV, CSV, I/Q, HackRF native support
- **Crypto Recovery**: Bitcoin, ECDSA, transaction analysis
- **Geometric Recovery**: ECDLP, k recovery, torus analysis
- **Network Recovery**: Graph structures, topology
- **Universal Recovery**: Blind recovery algorithms (Phase 1-6)

## Quick Start
```bash
# Install dependencies
cd scripts
sudo ./install_dependencies_ubuntu.sh  # or install_dependencies_centos.sh

# Build all libraries
cd ..
make

# Run tools
./tools/signal-recovery --help
./tools/bitcoin-recovery --help
```

## Directory Structure
- `lib/` - All recovery libraries
- `include/` - Public headers
- `src/` - Source implementations
- `tools/` - CLI tools
- `tests/` - Test suites
- `docs/` - Documentation
- `scripts/` - Build and install scripts

## Documentation
See `docs/` directory for detailed documentation.
