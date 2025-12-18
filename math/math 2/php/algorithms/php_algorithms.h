/*
  Algorithms PHP Extension Header
*/

#ifndef PHP_ALGORITHMS_H
#define PHP_ALGORITHMS_H

extern zend_module_entry algorithms_module_entry;
#define phpext_algorithms_ptr &algorithms_module_entry

#define PHP_ALGORITHMS_VERSION "1.0.0"

#ifdef PHP_WIN32
#   define PHP_ALGORITHMS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_ALGORITHMS_API __attribute__ ((visibility("default")))
#else
#   define PHP_ALGORITHMS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

// Function declarations
PHP_MINIT_FUNCTION(algorithms);
PHP_MSHUTDOWN_FUNCTION(algorithms);
PHP_MINFO_FUNCTION(algorithms);

PHP_FUNCTION(algo_statistics);
PHP_FUNCTION(algo_mean);
PHP_FUNCTION(algo_variance);
PHP_FUNCTION(algo_std_dev);
PHP_FUNCTION(algo_dot_product);
PHP_FUNCTION(algo_vector_norm);
PHP_FUNCTION(algo_normalize_vector);
PHP_FUNCTION(algo_cosine_similarity);
PHP_FUNCTION(algo_softmax);
PHP_FUNCTION(algo_cross_entropy);
PHP_FUNCTION(algo_gradient_descent);

#endif  /* PHP_ALGORITHMS_H */