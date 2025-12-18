#!/bin/bash
# Create PHP Extension Structure for Crystalline Math

set -e

PHP_EXT_DIR="php"
PHP_EXT_NAME="crystalline_math"

echo "Creating PHP extension files..."

# Create config.m4
cat > "$PHP_EXT_DIR/config.m4" << 'EOF'
PHP_ARG_WITH(crystalline-math, for Crystalline Math support,
[  --with-crystalline-math[=DIR]   Include Crystalline Math support])

if test "$PHP_CRYSTALLINE_MATH" != "no"; then
  SEARCH_PATH="/usr/local ../math"
  SEARCH_FOR="/include/math/abacus.h"
  
  if test -r $PHP_CRYSTALLINE_MATH/$SEARCH_FOR; then
    CRYSTALLINE_MATH_DIR=$PHP_CRYSTALLINE_MATH
  else
    AC_MSG_CHECKING([for Crystalline Math files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        CRYSTALLINE_MATH_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$CRYSTALLINE_MATH_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please install Crystalline Math library])
  fi

  PHP_ADD_INCLUDE($CRYSTALLINE_MATH_DIR/include)
  PHP_ADD_LIBRARY_WITH_PATH(crystallinemath, $CRYSTALLINE_MATH_DIR/lib, CRYSTALLINE_MATH_SHARED_LIBADD)
  PHP_SUBST(CRYSTALLINE_MATH_SHARED_LIBADD)
  
  PHP_NEW_EXTENSION(crystalline_math, crystalline_math.c, $ext_shared)
fi
EOF

# Create minimal PHP extension source
cat > "$PHP_EXT_DIR/$PHP_EXT_NAME.c" << 'EOF'
/* Crystalline Math PHP Extension - Minimal Version */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#define PHP_CRYSTALLINE_MATH_VERSION "1.0.0"
#define PHP_CRYSTALLINE_MATH_EXTNAME "crystalline_math"

/* PHP function declarations */
PHP_FUNCTION(crystalline_version);

/* Function entry table */
const zend_function_entry crystalline_math_functions[] = {
    PHP_FE(crystalline_version, NULL)
    PHP_FE_END
};

/* Module entry */
zend_module_entry crystalline_math_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_CRYSTALLINE_MATH_EXTNAME,
    crystalline_math_functions,
    NULL, /* MINIT */
    NULL, /* MSHUTDOWN */
    NULL, /* RINIT */
    NULL, /* RSHUTDOWN */
    NULL, /* MINFO */
    PHP_CRYSTALLINE_MATH_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CRYSTALLINE_MATH
ZEND_GET_MODULE(crystalline_math)
#endif

/* Return extension version */
PHP_FUNCTION(crystalline_version)
{
    RETURN_STRING(PHP_CRYSTALLINE_MATH_VERSION);
}
EOF

# Create PHP example
mkdir -p examples/php
cat > "examples/php/test_extension.php" << 'EOF'
<?php
/**
 * Crystalline Math Extension Test
 */

if (!extension_loaded('crystalline_math')) {
    die("ERROR: Crystalline Math extension not loaded.\n" .
        "Install with: sudo make install-php-ubuntu (or install-php-centos)\n");
}

echo "Crystalline Math Extension Test\n";
echo "================================\n\n";

echo "Extension loaded: YES\n";
echo "Version: " . crystalline_version() . "\n";

echo "\n✓ Extension is working!\n";
?>
EOF

chmod +x examples/php/test_extension.php

echo "✓ PHP extension files created successfully"