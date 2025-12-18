#!/bin/bash
# ============================================================================
# Crystalline CLLM - Ubuntu/Debian Dependency Installation Script
# ============================================================================
# This script installs all required dependencies for building and running
# the Crystalline CLLM system on Ubuntu/Debian-based distributions.
# ============================================================================

set -e  # Exit on error

echo "============================================================================"
echo "Crystalline CLLM - Dependency Installation (Ubuntu/Debian)"
echo "============================================================================"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
    echo "⚠️  Please do not run this script as root"
    echo "   The script will use sudo when needed"
    exit 1
fi

# Detect Ubuntu/Debian version
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
    echo "Detected: $OS $VER"
else
    echo "❌ Cannot detect OS version"
    exit 1
fi

echo ""
echo "This script will install the following dependencies:"
echo "  - Build tools (gcc, make, git)"
echo "  - Core libraries (libcurl, libxml2, libzip)"
echo "  - SDL2 libraries (libsdl2, libsdl2-ttf)"
echo "  - Document processing tools (poppler-utils, tesseract)"
echo ""
read -p "Continue? (y/n) " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Installation cancelled"
    exit 0
fi

echo ""
echo "Updating package lists..."
sudo apt-get update

echo ""
echo "============================================================================"
echo "1. Installing Build Tools"
echo "============================================================================"
sudo apt-get install -y \
    build-essential \
    gcc \
    g++ \
    make \
    cmake \
    git \
    pkg-config

echo ""
echo "============================================================================"
echo "2. Installing Core Libraries"
echo "============================================================================"
sudo apt-get install -y \
    libcurl4-openssl-dev \
    libxml2-dev \
    libzip-dev \
    zlib1g-dev

echo ""
echo "============================================================================"
echo "3. Installing SDL2 Libraries (for GUI)"
echo "============================================================================"
sudo apt-get install -y \
    libsdl2-dev \
    libsdl2-ttf-dev

echo ""
echo "============================================================================"
echo "4. Installing Document Processing Tools"
echo "============================================================================"
sudo apt-get install -y \
    poppler-utils \
    wkhtmltopdf \
    antiword \
    unrtf \
    catdoc \
    tesseract-ocr \
    tesseract-ocr-eng

echo ""
echo "============================================================================"
echo "5. Installing Text Processing Utilities"
echo "============================================================================"
sudo apt-get install -y \
    grep \
    gawk \
    sed \
    jq \
    csvkit \
    xmlstarlet

echo ""
echo "============================================================================"
echo "6. Installing Additional Utilities"
echo "============================================================================"
sudo apt-get install -y \
    wget \
    curl \
    zip \
    unzip \
    tmux \
    vim \
    tree \
    rsync

echo ""
echo "============================================================================"
echo "7. Installing Node.js (for web tools)"
echo "============================================================================"
# Check if Node.js is already installed
if command -v node &> /dev/null; then
    NODE_VERSION=$(node --version)
    echo "Node.js is already installed: $NODE_VERSION"
else
    echo "Installing Node.js 20.x..."
    curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
    sudo apt-get install -y nodejs
fi

echo ""
echo "============================================================================"
echo "Dependency Installation Complete!"
echo "============================================================================"
echo ""
echo "Installed versions:"
echo "  GCC:     $(gcc --version | head -n1)"
echo "  Make:    $(make --version | head -n1)"
echo "  libcurl: $(pkg-config --modversion libcurl)"
echo "  libxml2: $(pkg-config --modversion libxml-2.0)"
echo "  libzip:  $(pkg-config --modversion libzip)"
echo "  SDL2:    $(pkg-config --modversion sdl2)"
echo "  Node.js: $(node --version)"
echo ""
echo "✅ All dependencies installed successfully!"
echo ""
echo "Next steps:"
echo "  1. Build the libraries: make"
echo "  2. Build the application: make app"
echo "  3. Run the application: cd app && LD_LIBRARY_PATH=.. ./hyper_prime_spiral"
echo ""
echo "For more information, see README.md and DEPENDENCIES.md"
echo "============================================================================"