#!/bin/bash
# ============================================================================
# Crystalline CLLM - CentOS/RHEL Dependency Installation Script
# ============================================================================
# This script installs all required dependencies for building and running
# the Crystalline CLLM system on CentOS/RHEL-based distributions.
# ============================================================================

set -e  # Exit on error

echo "============================================================================"
echo "Crystalline CLLM - Dependency Installation (CentOS/RHEL)"
echo "============================================================================"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
    echo "⚠️  Please do not run this script as root"
    echo "   The script will use sudo when needed"
    exit 1
fi

# Detect CentOS/RHEL version
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
echo "  - Development tools (gcc, make, git)"
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
echo "============================================================================"
echo "1. Enabling EPEL Repository"
echo "============================================================================"
# EPEL is required for many packages
if ! rpm -q epel-release &> /dev/null; then
    echo "Installing EPEL repository..."
    sudo yum install -y epel-release
else
    echo "EPEL repository already installed"
fi

echo ""
echo "============================================================================"
echo "2. Installing Development Tools"
echo "============================================================================"
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    gcc \
    gcc-c++ \
    make \
    cmake \
    git \
    pkgconfig

echo ""
echo "============================================================================"
echo "3. Installing Core Libraries"
echo "============================================================================"
sudo yum install -y \
    libcurl-devel \
    libxml2-devel \
    libzip-devel \
    zlib-devel

echo ""
echo "============================================================================"
echo "4. Installing SDL2 Libraries (for GUI)"
echo "============================================================================"
sudo yum install -y \
    SDL2-devel \
    SDL2_ttf-devel

echo ""
echo "============================================================================"
echo "5. Installing Document Processing Tools"
echo "============================================================================"
sudo yum install -y \
    poppler-utils \
    wkhtmltopdf \
    antiword \
    catdoc \
    tesseract \
    tesseract-langpack-eng

# unrtf may not be available in all repos
if yum list unrtf &> /dev/null; then
    sudo yum install -y unrtf
else
    echo "⚠️  unrtf not available in repositories (optional)"
fi

echo ""
echo "============================================================================"
echo "6. Installing Text Processing Utilities"
echo "============================================================================"
sudo yum install -y \
    grep \
    gawk \
    sed \
    jq \
    xmlstarlet

# csvkit may require pip
if ! command -v csvcut &> /dev/null; then
    echo "Installing csvkit via pip..."
    sudo yum install -y python3-pip
    sudo pip3 install csvkit
fi

echo ""
echo "============================================================================"
echo "7. Installing Additional Utilities"
echo "============================================================================"
sudo yum install -y \
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
echo "8. Installing Node.js (for web tools)"
echo "============================================================================"
# Check if Node.js is already installed
if command -v node &> /dev/null; then
    NODE_VERSION=$(node --version)
    echo "Node.js is already installed: $NODE_VERSION"
else
    echo "Installing Node.js 20.x..."
    curl -fsSL https://rpm.nodesource.com/setup_20.x | sudo bash -
    sudo yum install -y nodejs
fi

echo ""
echo "============================================================================"
echo "Dependency Installation Complete!"
echo "============================================================================"
echo ""
echo "Installed versions:"
echo "  GCC:     $(gcc --version | head -n1)"
echo "  Make:    $(make --version | head -n1)"
echo "  libcurl: $(pkg-config --modversion libcurl 2>/dev/null || echo 'N/A')"
echo "  libxml2: $(pkg-config --modversion libxml-2.0 2>/dev/null || echo 'N/A')"
echo "  libzip:  $(pkg-config --modversion libzip 2>/dev/null || echo 'N/A')"
echo "  SDL2:    $(pkg-config --modversion sdl2 2>/dev/null || echo 'N/A')"
echo "  Node.js: $(node --version 2>/dev/null || echo 'N/A')"
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