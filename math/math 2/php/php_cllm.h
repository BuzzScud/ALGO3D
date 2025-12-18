/**
 * CLLM PHP Extension Header
 */

#ifndef PHP_CLLM_H
#define PHP_CLLM_H

extern zend_module_entry cllm_module_entry;
#define phpext_cllm_ptr &cllm_module_entry

#define PHP_CLLM_VERSION "1.0.0"

#ifdef PHP_WIN32
#   define PHP_CLLM_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_CLLM_API __attribute__ ((visibility("default")))
#else
#   define PHP_CLLM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(cllm);
PHP_MSHUTDOWN_FUNCTION(cllm);
PHP_MINFO_FUNCTION(cllm);

// Model functions
PHP_FUNCTION(cllm_create_model);
PHP_FUNCTION(cllm_free_model);
PHP_FUNCTION(cllm_save_model);
PHP_FUNCTION(cllm_load_model);
PHP_FUNCTION(cllm_model_info);

// Vocabulary functions
PHP_FUNCTION(cllm_vocab_create);
PHP_FUNCTION(cllm_vocab_build_from_file);
PHP_FUNCTION(cllm_vocab_tokenize);
PHP_FUNCTION(cllm_vocab_detokenize);
PHP_FUNCTION(cllm_vocab_save);
PHP_FUNCTION(cllm_vocab_load);
PHP_FUNCTION(cllm_vocab_stats);

// Training functions
PHP_FUNCTION(cllm_training_init);

// Inference functions
PHP_FUNCTION(cllm_generate);

// Math functions
PHP_FUNCTION(cllm_prime_nth);
PHP_FUNCTION(cllm_prime_is_prime);
PHP_FUNCTION(cllm_math_exp);
PHP_FUNCTION(cllm_math_log);
PHP_FUNCTION(cllm_math_sin);
PHP_FUNCTION(cllm_math_cos);

// Utility functions
PHP_FUNCTION(cllm_version);

#endif  /* PHP_CLLM_H */