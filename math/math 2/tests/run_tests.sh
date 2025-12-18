#!/bin/bash
export LD_LIBRARY_PATH=../math/lib:$LD_LIBRARY_PATH

echo "Running 88D Architecture Test..."
./test_88d_architecture
echo ""

echo "Running Transformer Layer Test..."
./test_transformer_layer
echo ""

echo "Running Transformer Components Test..."
./test_transformer_components