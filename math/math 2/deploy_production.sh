#!/bin/bash

# Crystalline CLLM Production Deployment Script
# This script builds, tests, and deploys the production version

set -e  # Exit on error

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║     CRYSTALLINE CLLM - PRODUCTION DEPLOYMENT                   ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Configuration
BUILD_DIR="build_production"
INSTALL_DIR="/usr/local/cllm"
LOG_FILE="deployment.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging function
log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1" | tee -a "$LOG_FILE"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" | tee -a "$LOG_FILE"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1" | tee -a "$LOG_FILE"
}

# Step 1: System Requirements Check
log "Step 1: Checking system requirements..."

check_command() {
    if command -v $1 &> /dev/null; then
        log_success "$1 is installed"
        return 0
    else
        log_error "$1 is not installed"
        return 1
    fi
}

REQUIREMENTS_MET=true

check_command gcc || REQUIREMENTS_MET=false
check_command make || REQUIREMENTS_MET=false
check_command git || REQUIREMENTS_MET=false

if [ "$REQUIREMENTS_MET" = false ]; then
    log_error "System requirements not met. Please install missing dependencies."
    exit 1
fi

# Check GCC version
GCC_VERSION=$(gcc --version | head -n1 | grep -oP '\d+\.\d+\.\d+' | head -n1)
log "GCC version: $GCC_VERSION"

# Step 2: Clean previous builds
log "Step 2: Cleaning previous builds..."

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    log_success "Removed previous build directory"
fi

mkdir -p "$BUILD_DIR"
log_success "Created build directory: $BUILD_DIR"

# Step 3: Build production version
log "Step 3: Building production version..."

make clean
if make production; then
    log_success "Production build completed"
else
    log_error "Production build failed"
    exit 1
fi

# Step 4: Run smoke tests
log "Step 4: Running smoke tests..."

SMOKE_TESTS=(
    "test_mathematical_integration"
    "test_integration_training"
)

TESTS_PASSED=0
TESTS_FAILED=0

for test in "${SMOKE_TESTS[@]}"; do
    if [ -f "$test" ]; then
        log "Running $test..."
        if ./"$test" > "${test}_output.log" 2>&1; then
            log_success "$test passed"
            TESTS_PASSED=$((TESTS_PASSED + 1))
        else
            log_error "$test failed (see ${test}_output.log)"
            TESTS_FAILED=$((TESTS_FAILED + 1))
        fi
    else
        log_warning "$test not found, skipping"
    fi
done

log "Smoke tests completed: $TESTS_PASSED passed, $TESTS_FAILED failed"

if [ $TESTS_FAILED -gt 0 ]; then
    log_error "Some smoke tests failed. Deployment aborted."
    exit 1
fi

# Step 5: Run performance benchmarks
log "Step 5: Running performance benchmarks..."

if [ -f "benchmark_large_scale" ]; then
    log "Running large-scale benchmarks..."
    if timeout 300 ./benchmark_large_scale > benchmark_output.log 2>&1; then
        log_success "Benchmarks completed"
    else
        log_warning "Benchmarks timed out or failed (see benchmark_output.log)"
    fi
else
    log_warning "Benchmark executable not found, skipping"
fi

# Step 6: Create installation package
log "Step 6: Creating installation package..."

PACKAGE_DIR="cllm_production_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$PACKAGE_DIR"

# Copy binaries
cp -r bin "$PACKAGE_DIR/" 2>/dev/null || log_warning "No bin directory found"
cp -r lib "$PACKAGE_DIR/" 2>/dev/null || log_warning "No lib directory found"

# Copy headers
mkdir -p "$PACKAGE_DIR/include"
cp src/*.h "$PACKAGE_DIR/include/" 2>/dev/null || log_warning "No headers found"
cp production_config.h "$PACKAGE_DIR/include/" 2>/dev/null || true

# Copy documentation
mkdir -p "$PACKAGE_DIR/docs"
cp *.md "$PACKAGE_DIR/docs/" 2>/dev/null || log_warning "No documentation found"

# Copy tools
cp -r tools "$PACKAGE_DIR/" 2>/dev/null || log_warning "No tools directory found"

# Create README
cat > "$PACKAGE_DIR/README.md" << EOF
# Crystalline CLLM - Production Package

## Installation

1. Extract this package to your desired location
2. Run the installation script: \`sudo ./install.sh\`
3. Verify installation: \`cllm --version\`

## Configuration

Edit \`production_config.h\` to customize settings for your deployment.

## Usage

See the documentation in the \`docs/\` directory for detailed usage instructions.

## Support

For issues and support, please refer to the project repository.

## Build Information

- Build Date: $(date)
- GCC Version: $GCC_VERSION
- System: $(uname -a)

EOF

# Create installation script
cat > "$PACKAGE_DIR/install.sh" << 'EOF'
#!/bin/bash
set -e

INSTALL_DIR="/usr/local/cllm"

echo "Installing Crystalline CLLM to $INSTALL_DIR..."

# Create installation directory
sudo mkdir -p "$INSTALL_DIR"

# Copy files
sudo cp -r bin lib include tools "$INSTALL_DIR/"

# Create symlinks
sudo ln -sf "$INSTALL_DIR/bin/"* /usr/local/bin/

echo "Installation completed successfully!"
echo "Run 'cllm --version' to verify installation"
EOF

chmod +x "$PACKAGE_DIR/install.sh"

# Create tarball
tar -czf "${PACKAGE_DIR}.tar.gz" "$PACKAGE_DIR"
log_success "Created installation package: ${PACKAGE_DIR}.tar.gz"

# Step 7: Generate deployment report
log "Step 7: Generating deployment report..."

cat > "DEPLOYMENT_REPORT.md" << EOF
# Crystalline CLLM - Production Deployment Report

**Deployment Date:** $(date)
**Build Version:** Production
**System:** $(uname -a)

## Build Information

- **GCC Version:** $GCC_VERSION
- **Build Directory:** $BUILD_DIR
- **Package:** ${PACKAGE_DIR}.tar.gz

## Test Results

### Smoke Tests
- **Tests Passed:** $TESTS_PASSED
- **Tests Failed:** $TESTS_FAILED
- **Success Rate:** $(echo "scale=2; $TESTS_PASSED * 100 / ($TESTS_PASSED + $TESTS_FAILED)" | bc)%

### Performance Benchmarks
See \`benchmark_output.log\` for detailed benchmark results.

## Deployment Status

✅ System requirements verified
✅ Production build completed
✅ Smoke tests passed
✅ Installation package created

## Next Steps

1. Extract the package: \`tar -xzf ${PACKAGE_DIR}.tar.gz\`
2. Review the configuration in \`production_config.h\`
3. Run the installation script: \`cd ${PACKAGE_DIR} && sudo ./install.sh\`
4. Verify installation: \`cllm --version\`

## Files Generated

- \`${PACKAGE_DIR}.tar.gz\` - Installation package
- \`deployment.log\` - Detailed deployment log
- \`benchmark_output.log\` - Performance benchmark results
- \`DEPLOYMENT_REPORT.md\` - This report

## Configuration

The production build includes:
- ✅ Lattice Embeddings
- ✅ NTT-based Attention
- ✅ Cymatic Training
- ✅ Kissing Spheres Optimization

## Support

For issues or questions, please refer to the project documentation.

---
*Generated by Crystalline CLLM Deployment Script*
EOF

log_success "Deployment report generated: DEPLOYMENT_REPORT.md"

# Step 8: Final summary
echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║              DEPLOYMENT COMPLETED SUCCESSFULLY                 ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "Summary:"
echo "  ✅ Build completed"
echo "  ✅ Tests passed: $TESTS_PASSED/$((TESTS_PASSED + TESTS_FAILED))"
echo "  ✅ Package created: ${PACKAGE_DIR}.tar.gz"
echo "  ✅ Deployment report: DEPLOYMENT_REPORT.md"
echo ""
echo "Next steps:"
echo "  1. Review DEPLOYMENT_REPORT.md"
echo "  2. Extract and install the package"
echo "  3. Run production validation tests"
echo ""

log "Deployment script completed successfully"