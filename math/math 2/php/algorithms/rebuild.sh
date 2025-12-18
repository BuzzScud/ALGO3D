#!/bin/bash
# Script to properly rebuild PHP extension
# This regenerates all build files with correct paths for your system

set -e

echo "=== Cleaning PHP Algorithms Extension ==="
make clean 2>/dev/null || true
phpize --clean

echo ""
echo "=== Regenerating build configuration ==="
phpize

echo ""
echo "=== Configuring extension ==="
./configure

echo ""
echo "=== Building extension ==="
make

echo ""
echo "=== Build complete! ==="
echo "Extension built: modules/crystalline_algorithms.so"