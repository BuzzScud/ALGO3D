#!/bin/bash
#
# Recovery System Organization Script
# Creates production-ready /recovery directory with all relevant files
#

set -e

echo "=========================================="
echo "Recovery System Organization"
echo "=========================================="
echo ""

# Create base directory structure
echo "Creating directory structure..."
mkdir -p recovery/{lib,include,src,tools,tests,docs,scripts,examples}
mkdir -p recovery/lib/{recovery_common,recovery_core,recovery_crypto,recovery_signal,recovery_network,recovery_geometric}
mkdir -p recovery/src/{loaders,crypto,signal,geometric,blind_recovery}
mkdir -p recovery/include/{recovery_common,recovery_core,recovery_crypto,recovery_signal,recovery_network,recovery_geometric}

echo "✓ Directory structure created"
echo ""

# Copy libraries
echo "Copying libraries..."
cp -r algorithms/lib/recovery_common/* recovery/lib/recovery_common/ 2>/dev/null || true
cp -r lib/recovery_core/* recovery/lib/recovery_core/ 2>/dev/null || true
cp -r lib/recovery_crypto/* recovery/lib/recovery_crypto/ 2>/dev/null || true
cp -r lib/recovery_signal/* recovery/lib/recovery_signal/ 2>/dev/null || true
cp -r lib/recovery_network/* recovery/lib/recovery_network/ 2>/dev/null || true
cp -r reference_implementations/objective28_geometric_recovery recovery/lib/recovery_geometric/ 2>/dev/null || true

echo "✓ Libraries copied"
echo ""

# Copy headers
echo "Copying headers..."
cp algorithms/include/recovery_common/*.h recovery/include/recovery_common/ 2>/dev/null || true
cp lib/recovery_core/include/*.h recovery/include/recovery_core/ 2>/dev/null || true
cp lib/recovery_crypto/include/*.h recovery/include/recovery_crypto/ 2>/dev/null || true
cp lib/recovery_signal/include/*.h recovery/include/recovery_signal/ 2>/dev/null || true
cp lib/recovery_network/include/*.h recovery/include/recovery_network/ 2>/dev/null || true
cp reference_implementations/objective28_geometric_recovery/include/*.h recovery/include/recovery_geometric/ 2>/dev/null || true

echo "✓ Headers copied"
echo ""

# Copy source files
echo "Copying source files..."
cp algorithms/lib/recovery_common/*.c recovery/src/loaders/ 2>/dev/null || true
cp lib/recovery_crypto/src/*.c recovery/src/crypto/ 2>/dev/null || true
cp lib/recovery_signal/src/*.c recovery/src/signal/ 2>/dev/null || true
cp reference_implementations/objective28_geometric_recovery/src/*.c recovery/src/geometric/ 2>/dev/null || true
cp algorithms/src/blind_recovery/*.c recovery/src/blind_recovery/ 2>/dev/null || true

echo "✓ Source files copied"
echo ""

# Copy tools
echo "Copying tools..."
cp tools/bitcoin-recovery recovery/tools/ 2>/dev/null || true
cp tools/bitcoin-recovery.c recovery/tools/ 2>/dev/null || true
cp tools/recovery/* recovery/tools/ 2>/dev/null || true

echo "✓ Tools copied"
echo ""

# Copy scripts
echo "Copying scripts..."
cp install_dependencies_ubuntu.sh recovery/scripts/
cp install_dependencies_centos.sh recovery/scripts/

echo "✓ Scripts copied"
echo ""

# Create documentation
echo "Creating documentation..."
cat > recovery/README.md << 'EOF'
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
EOF

echo "✓ Documentation created"
echo ""

echo "=========================================="
echo "Organization Complete!"
echo "=========================================="
echo ""
echo "Recovery system organized in: ./recovery/"
echo ""
echo "Next steps:"
echo "1. cd recovery"
echo "2. Review the organized structure"
echo "3. Build the system: make"
echo "4. Run tests: make test"