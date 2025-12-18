#!/bin/bash

echo "🔧 Fixing Apache PHP Extensions for Crystalline Math"
echo "=================================================="

# Find Apache's php.ini
APACHE_INI=$(php -r "echo php_ini_loaded_file();" | sed 's/cli/apache2/')

if [ ! -f "$APACHE_INI" ]; then
    echo "❌ Apache php.ini not found at: $APACHE_INI"
    echo "Searching for Apache php.ini..."
    APACHE_INI=$(find /etc/php -name php.ini | grep apache2 | head -1)
    if [ -z "$APACHE_INI" ]; then
        echo "❌ Could not find Apache php.ini"
        echo "Please manually add these lines to your Apache php.ini:"
        echo "  extension=crystalline_math.so"
        echo "  extension=algorithms.so"
        exit 1
    fi
fi

echo "✅ Found Apache php.ini: $APACHE_INI"

# Check if extensions are already added
if grep -q "crystalline_math.so" "$APACHE_INI"; then
    echo "✅ crystalline_math.so already in php.ini"
else
    echo "➕ Adding crystalline_math.so to php.ini"
    echo "extension=crystalline_math.so" | sudo tee -a "$APACHE_INI"
fi

if grep -q "algorithms.so" "$APACHE_INI"; then
    echo "✅ algorithms.so already in php.ini"
else
    echo "➕ Adding algorithms.so to php.ini"
    echo "extension=algorithms.so" | sudo tee -a "$APACHE_INI"
fi

# Restart Apache
echo ""
echo "🔄 Restarting Apache..."
if systemctl is-active --quiet apache2; then
    sudo systemctl restart apache2
    echo "✅ Apache restarted"
elif systemctl is-active --quiet httpd; then
    sudo systemctl restart httpd
    echo "✅ Apache (httpd) restarted"
else
    echo "⚠️  Could not detect Apache service"
    echo "Please restart Apache manually:"
    echo "  sudo systemctl restart apache2"
    echo "  OR"
    echo "  sudo systemctl restart httpd"
fi

# Copy check script to web root
echo ""
echo "📋 Copying check script to web root..."
sudo cp check_extensions.php /var/www/html/math/
sudo chmod 644 /var/www/html/math/check_extensions.php

echo ""
echo "✅ Setup complete!"
echo ""
echo "🧪 Test by visiting:"
echo "   http://voynich.online/math/check_extensions.php"
echo ""
echo "Expected output:"
echo "   crystalline_math loaded: YES"
echo "   algorithms loaded: YES"
echo "   prime_nth exists: YES"