#!/bin/bash
#
# Recovery Directory Cleanup Script
# Removes all non-essential files, keeping only what's needed for building
#

set -e

echo "=========================================="
echo "Recovery Directory Cleanup"
echo "=========================================="
echo ""

cd recovery

echo "Analyzing current size..."
du -sh . 2>/dev/null || true
echo ""

# Keep track of what we're removing
echo "Files to remove:"
echo "  - All test files (75 files)"
echo "  - All sample files (300 files)"
echo "  - All test outputs"
echo "  - All documentation (except README)"
echo "  - Build artifacts"
echo ""

read -p "Continue with cleanup? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cleanup cancelled."
    exit 0
fi

echo ""
echo "Cleaning up..."

# Remove tests from objective28
echo "  Removing tests from objective28..."
rm -rf lib/recovery_geometric/objective28_geometric_recovery/tests/

# Remove samples
echo "  Removing sample files..."
rm -rf lib/recovery_geometric/objective28_geometric_recovery/samples/

# Remove test outputs
echo "  Removing test outputs..."
rm -rf lib/recovery_geometric/objective28_geometric_recovery/test_outputs/
rm -rf lib/recovery_geometric/objective28_geometric_recovery/test_results/
rm -rf lib/recovery_geometric/objective28_geometric_recovery/test_data/

# Remove documentation (keep only README)
echo "  Removing documentation..."
rm -rf lib/recovery_geometric/objective28_geometric_recovery/docs/
rm -rf lib/recovery_geometric/objective28_geometric_recovery/documentation/
rm -f lib/recovery_geometric/objective28_geometric_recovery/*.md
rm -f lib/recovery_geometric/objective28_geometric_recovery/*.platonic

# Remove build artifacts
echo "  Removing build artifacts..."
rm -rf lib/recovery_geometric/objective28_geometric_recovery/build/*.o
rm -f lib/recovery_geometric/objective28_geometric_recovery/test_*.bin

# Remove test scripts
echo "  Removing test scripts..."
rm -f lib/recovery_geometric/objective28_geometric_recovery/run_deployment_tests.sh

# Remove extra Makefiles
echo "  Removing extra Makefiles..."
rm -f lib/recovery_geometric/objective28_geometric_recovery/Makefile.phase1
rm -f lib/recovery_geometric/objective28_geometric_recovery/Makefile.phase6

# Clean up empty directories
echo "  Cleaning up empty directories..."
find . -type d -empty -delete 2>/dev/null || true

echo ""
echo "Cleanup complete!"
echo ""
echo "New size:"
du -sh . 2>/dev/null || true
echo ""

echo "Remaining structure:"
tree -L 3 -d . 2>/dev/null || find . -type d | head -30