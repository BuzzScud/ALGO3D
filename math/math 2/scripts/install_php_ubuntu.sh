#!/bin/bash
# Install Crystalline Math PHP Extension on Ubuntu/Debian

set -e

echo "=== Crystalline Math PHP Extension Installer (Ubuntu/Debian) ==="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

# Detect PHP version
PHP_VERSION=$(php -r 'echo PHP_MAJOR_VERSION.".".PHP_MINOR_VERSION;' 2>/dev/null || echo "")
if [ -z "$PHP_VERSION" ]; then
    echo "Error: PHP not found. Please install PHP first:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install php php-dev"
    exit 1
fi

echo "Detected PHP version: $PHP_VERSION"
echo ""

# Install dependencies
echo "Step 1: Installing dependencies..."
apt-get update
apt-get install -y php-dev build-essential autoconf automake libtool

# Detect script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Build the NEW math library if not already built
echo ""
echo "Step 2: Building Crystalline Math library..."
cd "$PROJECT_ROOT/math"
if [ ! -f "lib/libcrystallinemath.so" ]; then
    make clean
    make
    echo "✓ Math library built successfully"
else
    echo "✓ Math library already built"
fi
cd "$PROJECT_ROOT/php"

# Build PHP extension
echo ""
echo "Step 3: Building PHP extension..."
phpize
./configure --enable-crystalline-math
make clean
make

# Install extension
echo ""
echo "Step 4: Installing PHP extension..."
make install

# Get extension directory
EXT_DIR=$(php-config --extension-dir)
echo "Extension installed to: $EXT_DIR"

# Create php.ini configuration
INI_DIR="/etc/php/$PHP_VERSION/mods-available"
if [ -d "$INI_DIR" ]; then
    echo ""
    echo "Step 5: Configuring PHP..."
    echo "extension=crystalline_math.so" > "$INI_DIR/crystalline_math.ini"
    
    # Enable for CLI
    if [ -d "/etc/php/$PHP_VERSION/cli/conf.d" ]; then
        ln -sf "$INI_DIR/crystalline_math.ini" "/etc/php/$PHP_VERSION/cli/conf.d/20-crystalline_math.ini"
        echo "✓ Enabled for PHP CLI"
    fi
    
    # Enable for Apache
    if [ -d "/etc/php/$PHP_VERSION/apache2/conf.d" ]; then
        ln -sf "$INI_DIR/crystalline_math.ini" "/etc/php/$PHP_VERSION/apache2/conf.d/20-crystalline_math.ini"
        echo "✓ Enabled for Apache"
        
        # Restart Apache
        if systemctl is-active --quiet apache2; then
            systemctl restart apache2
            echo "✓ Apache restarted"
        fi
    fi
    
    # Enable for FPM
    if [ -d "/etc/php/$PHP_VERSION/fpm/conf.d" ]; then
        ln -sf "$INI_DIR/crystalline_math.ini" "/etc/php/$PHP_VERSION/fpm/conf.d/20-crystalline_math.ini"
        echo "✓ Enabled for PHP-FPM"
        
        # Restart PHP-FPM
        if systemctl is-active --quiet php$PHP_VERSION-fpm; then
            systemctl restart php$PHP_VERSION-fpm
            echo "✓ PHP-FPM restarted"
        fi
    fi
else
    echo ""
    echo "Step 5: Manual configuration required"
    echo "Add the following line to your php.ini:"
    echo "  extension=crystalline_math.so"
fi

# Verify installation
echo ""
echo "Step 6: Verifying installation..."
if php -m | grep -q crystalline_math; then
    echo "✓ Extension loaded successfully!"
    echo ""
    php -r 'echo "Version: " . crystalline_version() . "\n";'
else
    echo "✗ Extension not loaded. Please check your PHP configuration."
    exit 1
fi

echo ""
echo "=== Installation Complete ==="
echo ""
echo "Test the extension with:"
echo "  php ../examples/php/prime_generation.php"
echo ""