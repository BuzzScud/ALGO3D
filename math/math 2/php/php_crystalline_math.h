#ifndef PHP_CRYSTALLINE_MATH_H
#define PHP_CRYSTALLINE_MATH_H

extern zend_module_entry crystalline_math_module_entry;
#define phpext_crystalline_math_ptr &crystalline_math_module_entry

#define PHP_CRYSTALLINE_MATH_VERSION "1.0.0"

#ifdef PHP_WIN32
#   define PHP_CRYSTALLINE_MATH_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_CRYSTALLINE_MATH_API __attribute__ ((visibility("default")))
#else
#   define PHP_CRYSTALLINE_MATH_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* Module functions */
PHP_MINIT_FUNCTION(crystalline_math);
PHP_MSHUTDOWN_FUNCTION(crystalline_math);
PHP_MINFO_FUNCTION(crystalline_math);

/* Prime generation functions */
PHP_FUNCTION(crystalline_prime_generate_o1);
PHP_FUNCTION(crystalline_prime_is_prime);
PHP_FUNCTION(crystalline_prime_nth);

/* Rainbow table functions */
PHP_FUNCTION(crystalline_rainbow_init);
PHP_FUNCTION(crystalline_rainbow_populate);
PHP_FUNCTION(crystalline_rainbow_lookup);
PHP_FUNCTION(crystalline_rainbow_count);

/* Clock lattice functions */
PHP_FUNCTION(crystalline_clock_position);
PHP_FUNCTION(crystalline_clock_validate);

/* Utility functions */
PHP_FUNCTION(crystalline_version);

#endif /* PHP_CRYSTALLINE_MATH_H */