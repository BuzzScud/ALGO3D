#!/bin/bash
#
# Recovery System Dependencies Installer - Ubuntu/Debian
#
# Installs all required dependencies for the Recovery System
#

set -e

echo "=========================================="
echo "Recovery System Dependencies Installer"
echo "Ubuntu/Debian"
echo "=========================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

echo "Updating package lists..."
apt-get update

echo ""
echo "Installing build tools..."
apt-get install -y \
    build-essential \
    pkg-config \
    cmake \
    git \
    autoconf \
    automake \
    libtool

echo ""
echo "Installing JSON library (jansson)..."
apt-get install -y libjansson-dev

echo ""
echo "Installing SDR libraries..."
apt-get install -y \
    libhackrf-dev \
    librtlsdr-dev \
    libusb-1.0-0-dev

echo ""
echo "Installing cryptography libraries..."
apt-get install -y \
    libssl-dev \
    libgmp-dev

echo ""
echo "Installing additional utilities..."
apt-get install -y \
    curl \
    wget \
    vim \
    htop

echo ""
echo "=========================================="
echo "Dependency Installation Complete!"
echo "=========================================="
echo ""
echo "Installed packages:"
echo "  - Build tools: gcc, g++, make, cmake, pkg-config"
echo "  - JSON: jansson"
echo "  - SDR: hackrf, rtl-sdr, libusb"
echo "  - Crypto: OpenSSL, GMP"
echo ""
echo "Verifying installations..."
echo ""

# Verify installations
echo -n "pkg-config: "
pkg-config --version || echo "NOT FOUND"

echo -n "jansson: "
pkg-config --modversion jansson || echo "NOT FOUND"

echo -n "libusb: "
pkg-config --modversion libusb-1.0 || echo "NOT FOUND"

echo -n "OpenSSL: "
openssl version || echo "NOT FOUND"

echo ""
echo "Ready to build Recovery System!"