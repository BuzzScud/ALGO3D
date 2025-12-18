#!/bin/bash
# Universal Recovery System - Dependency Installation Script
# Supports: CentOS, RHEL, Ubuntu, Debian

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Detect OS
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        VER=$VERSION_ID
    elif [ -f /etc/redhat-release ]; then
        OS="rhel"
    else
        echo -e "${RED}Cannot detect OS${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Detected OS: $OS $VER${NC}"
}

# Install dependencies for CentOS/RHEL
install_centos_rhel() {
    echo -e "${YELLOW}Installing dependencies for CentOS/RHEL...${NC}"
    
    # Enable EPEL repository
    sudo yum install -y epel-release
    
    # Development tools
    sudo yum groupinstall -y "Development Tools"
    
    # Core dependencies
    sudo yum install -y \
        gcc \
        gcc-c++ \
        make \
        cmake \
        git \
        wget \
        curl \
        openssl-devel \
        libcurl-devel \
        json-c-devel \
        python3 \
        python3-devel \
        python3-pip \
        php \
        php-devel \
        gnuradio \
        gnuradio-devel
    
    # Try to install libsecp256k1 (may need to build from source)
    if ! sudo yum install -y libsecp256k1-devel 2>/dev/null; then
        echo -e "${YELLOW}libsecp256k1 not available in repos, will build from source${NC}"
        build_secp256k1
    fi
    
    echo -e "${GREEN}CentOS/RHEL dependencies installed${NC}"
}

# Install dependencies for Ubuntu/Debian
install_ubuntu_debian() {
    echo -e "${YELLOW}Installing dependencies for Ubuntu/Debian...${NC}"
    
    # Update package list
    sudo apt-get update
    
    # Core dependencies
    sudo apt-get install -y \
        build-essential \
        gcc \
        g++ \
        make \
        cmake \
        git \
        wget \
        curl \
        libssl-dev \
        libcurl4-openssl-dev \
        libjson-c-dev \
        python3 \
        python3-dev \
        python3-pip \
        php \
        php-dev \
        gnuradio \
        gnuradio-dev \
        libsecp256k1-dev \
        autoconf \
        automake \
        libtool \
        pkg-config
    
    echo -e "${GREEN}Ubuntu/Debian dependencies installed${NC}"
}

# Build libsecp256k1 from source
build_secp256k1() {
    echo -e "${YELLOW}Building libsecp256k1 from source...${NC}"
    
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    git clone https://github.com/bitcoin-core/secp256k1.git
    cd secp256k1
    
    ./autogen.sh
    ./configure --enable-module-recovery --enable-module-ecdh
    make -j$(nproc)
    sudo make install
    sudo ldconfig
    
    cd /
    rm -rf "$TEMP_DIR"
    
    echo -e "${GREEN}libsecp256k1 built and installed${NC}"
}

# Install Python dependencies
install_python_deps() {
    echo -e "${YELLOW}Installing Python dependencies...${NC}"
    
    sudo pip3 install --upgrade pip
    sudo pip3 install \
        numpy \
        scipy \
        matplotlib \
        requests \
        websocket-client \
        pycryptodome
    
    echo -e "${GREEN}Python dependencies installed${NC}"
}

# Verify installation
verify_installation() {
    echo -e "${YELLOW}Verifying installation...${NC}"
    
    # Check for required tools
    REQUIRED_TOOLS="gcc g++ make cmake git openssl"
    for tool in $REQUIRED_TOOLS; do
        if command -v $tool &> /dev/null; then
            echo -e "${GREEN}✓ $tool${NC}"
        else
            echo -e "${RED}✗ $tool not found${NC}"
        fi
    done
    
    # Check for required libraries
    echo -e "\n${YELLOW}Checking libraries...${NC}"
    
    # OpenSSL
    if pkg-config --exists openssl; then
        echo -e "${GREEN}✓ OpenSSL $(pkg-config --modversion openssl)${NC}"
    else
        echo -e "${RED}✗ OpenSSL not found${NC}"
    fi
    
    # libcurl
    if pkg-config --exists libcurl; then
        echo -e "${GREEN}✓ libcurl $(pkg-config --modversion libcurl)${NC}"
    else
        echo -e "${RED}✗ libcurl not found${NC}"
    fi
    
    # Python
    if command -v python3 &> /dev/null; then
        echo -e "${GREEN}✓ Python $(python3 --version)${NC}"
    else
        echo -e "${RED}✗ Python not found${NC}"
    fi
    
    echo -e "\n${GREEN}Verification complete${NC}"
}

# Main installation
main() {
    echo -e "${GREEN}=== Universal Recovery System - Dependency Installation ===${NC}\n"
    
    detect_os
    
    case "$OS" in
        centos|rhel|fedora)
            install_centos_rhel
            ;;
        ubuntu|debian)
            install_ubuntu_debian
            ;;
        *)
            echo -e "${RED}Unsupported OS: $OS${NC}"
            exit 1
            ;;
    esac
    
    install_python_deps
    verify_installation
    
    echo -e "\n${GREEN}=== Installation Complete ===${NC}"
    echo -e "${YELLOW}You can now build the Universal Recovery System with:${NC}"
    echo -e "  make clean && make -j\$(nproc)"
    echo -e "  sudo make install"
}

# Run main
main