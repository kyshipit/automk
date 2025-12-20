#!/bin/bash

# AutoMLOS IPC Library Test Script
# Builds and tests the IPC library functionality

set -e  # Exit on any error

echo "=== AutoMLOS IPC Library Test Script ==="
echo "Building and testing IPC functionality..."

# Change to project directory
cd "$(dirname "$0")/.."

# Clean previous build
echo "Cleaning previous build..."
make clean

# Build libraries
echo "Building libraries..."
make libraries

# Build tests
echo "Building tests..."
cd tests
make all

# Run IPC tests
echo "Running IPC tests..."
make test-ipc

# Run integration tests
echo "Running integration tests..."
make test-integration

echo "=== All tests completed successfully ==="
echo "IPC library is ready for use!"