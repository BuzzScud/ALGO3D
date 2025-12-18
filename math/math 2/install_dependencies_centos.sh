#!/bin/bash
#
# Recovery System Dependencies Installer - CentOS/RHEL
#
# Installs all required dependencies for the Recovery System
#

set -e

echo "=========================================="
echo "Recovery System Dependencies Installer"
echo "CentOS/RHEL"
echo "=========================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

echo "Installing EPEL repository..."
yum install -y epel-release

echo ""
echo "Updating package lists..."
yum update -y

echo ""
echo "Installing build tools..."
yum groupinstall -y "Development Tools"
yum install -y \
    pkgconfig \
    cmake \
    git \
    autoconf \
    automake \
    libtool

echo ""
echo "Installing JSON library (jansson)..."
yum install -y jansson-devel

echo ""
echo "Installing SDR libraries..."
yum install -y \
    hackrf-devel \
    rtl-sdr-devel \
    libusb-devel

echo ""
echo "Installing cryptography libraries..."
yum install -y \
    openssl-devel \
    gmp-devel

echo ""
echo "Installing additional utilities..."
yum install -y \
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
echo "  - Build tools: gcc, g++, make, cmake, pkgconfig"
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