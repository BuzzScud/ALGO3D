/*
  Crystalline Math PHP Extension Header
*/

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

// Function declarations
PHP_MINIT_FUNCTION(crystalline_math);
PHP_MSHUTDOWN_FUNCTION(crystalline_math);
PHP_MINFO_FUNCTION(crystalline_math);

PHP_FUNCTION(crystalline_gcd);
PHP_FUNCTION(crystalline_is_prime);
PHP_FUNCTION(crystalline_next_prime);
PHP_FUNCTION(crystalline_prime_factors);
PHP_FUNCTION(crystalline_sieve);
PHP_FUNCTION(crystalline_sqrt);
PHP_FUNCTION(crystalline_pow);
PHP_FUNCTION(crystalline_sin);
PHP_FUNCTION(crystalline_cos);
PHP_FUNCTION(crystalline_log);
PHP_FUNCTION(crystalline_exp);

#endif  /* PHP_CRYSTALLINE_MATH_H */