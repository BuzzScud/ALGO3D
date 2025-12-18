#!/bin/bash

echo "=== COMPREHENSIVE GEOMETRIC RECOVERY TESTS ==="
echo "Starting at: $(date)"
echo ""

cd /workspace/algorithms

# Build test if needed
echo "Building test_geometric_recovery..."
gcc -Wall -Wextra -O2 -fPIC -std=c11 -D_GNU_SOURCE -I./include -I./include/blind_recovery -I../include -o tests/test_geometric_recovery tests/test_geometric_recovery.c -L. -lalgorithms -L.. -lcrystalline -lssl -lcrypto -lm

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo "Build successful"
echo ""

# Run geometric recovery test
echo "=== TEST 1: Geometric Recovery System ==="
echo "Running with 10,000 anchors, 52 dimensions..."
LD_LIBRARY_PATH=.:.. timeout 300 ./tests/test_geometric_recovery > geometric_recovery_results.txt 2>&1
TEST1_EXIT=$?

if [ $TEST1_EXIT -eq 124 ]; then
    echo "TIMEOUT after 300 seconds"
elif [ $TEST1_EXIT -eq 0 ]; then
    echo "COMPLETED"
    cat geometric_recovery_results.txt
else
    echo "ERROR: Exit code $TEST1_EXIT"
    cat geometric_recovery_results.txt
fi

echo ""
echo "=== TEST 2: Bitcoin Recovery Benchmark ==="
echo "Running Bitcoin k recovery test..."

# Check if bitcoin test exists
if [ -f "../tools/universal-recovery" ]; then
    cd ..
    echo "Generating 1000 Bitcoin samples..."
    # Generate test data
    python3 -c "
import os
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization

# Generate 1000 samples
os.makedirs('test_samples', exist_ok=True)
for i in range(1000):
    private_key = ec.generate_private_key(ec.SECP256K1())
    private_bytes = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption()
    )
    with open(f'test_samples/bitcoin_{i}.pem', 'wb') as f:
        f.write(private_bytes)
    if (i + 1) % 100 == 0:
        print(f'Generated {i+1}/1000 samples')
" 2>&1

    if [ $? -eq 0 ]; then
        echo "Running universal-recovery with benchmarks..."
        timeout 300 ./tools/universal-recovery --input test_samples --output recovered --benchmark > bitcoin_recovery_results.txt 2>&1
        TEST2_EXIT=$?
        
        if [ $TEST2_EXIT -eq 124 ]; then
            echo "TIMEOUT after 300 seconds"
        elif [ $TEST2_EXIT -eq 0 ]; then
            echo "COMPLETED"
            cat bitcoin_recovery_results.txt
        else
            echo "ERROR: Exit code $TEST2_EXIT"
            cat bitcoin_recovery_results.txt
        fi
    else
        echo "ERROR: Failed to generate test samples"
    fi
else
    echo "SKIPPED: universal-recovery tool not found"
fi

echo ""
echo "=== TEST 3: SSH Recovery Benchmark ==="
echo "Running SSH key recovery test..."

if [ -f "../tools/universal-recovery" ]; then
    echo "Generating 1000 SSH samples..."
    python3 -c "
import os
from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.hazmat.primitives import serialization

os.makedirs('test_samples_ssh', exist_ok=True)
for i in range(1000):
    private_key = ed25519.Ed25519PrivateKey.generate()
    private_bytes = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.OpenSSH,
        encryption_algorithm=serialization.NoEncryption()
    )
    with open(f'test_samples_ssh/ssh_{i}.pem', 'wb') as f:
        f.write(private_bytes)
    if (i + 1) % 100 == 0:
        print(f'Generated {i+1}/1000 samples')
" 2>&1

    if [ $? -eq 0 ]; then
        echo "Running universal-recovery with benchmarks..."
        timeout 300 ./tools/universal-recovery --input test_samples_ssh --output recovered_ssh --benchmark > ssh_recovery_results.txt 2>&1
        TEST3_EXIT=$?
        
        if [ $TEST3_EXIT -eq 124 ]; then
            echo "TIMEOUT after 300 seconds"
        elif [ $TEST3_EXIT -eq 0 ]; then
            echo "COMPLETED"
            cat ssh_recovery_results.txt
        else
            echo "ERROR: Exit code $TEST3_EXIT"
            cat ssh_recovery_results.txt
        fi
    else
        echo "ERROR: Failed to generate SSH test samples"
    fi
else
    echo "SKIPPED: universal-recovery tool not found"
fi

echo ""
echo "=== SUMMARY ==="
echo "Completed at: $(date)"
echo ""
echo "Results saved to:"
echo "  - geometric_recovery_results.txt"
echo "  - bitcoin_recovery_results.txt"
echo "  - ssh_recovery_results.txt"