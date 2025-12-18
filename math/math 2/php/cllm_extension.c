/**
 * CLLM PHP Extension - Complete API Wrapper
 * 
 * Provides PHP bindings for the Crystalline Lattice Language Model
 * 
 * Features:
 * - Model creation and management
 * - Training with vocabulary
 * - Inference and text generation
 * - Checkpoint save/load
 * - Vocabulary management
 * - Math library functions
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_cllm.h"

// CLLM includes
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_vocabulary.h"
#include "../include/cllm_format.h"
#include "../math/include/math/prime.h"
#include "../math/include/math/transcendental.h"
#include "../math/include/math/arithmetic.h"

/* Resource destructors */
static void cllm_model_dtor(zend_resource *rsrc TSRMLS_DC) {
    CLLMModel *model = (CLLMModel *)rsrc->ptr;
    if (model) {
        cllm_free_model(model);
    }
}

static void cllm_training_dtor(zend_resource *rsrc TSRMLS_DC) {
    CLLMTraining *training = (CLLMTraining *)rsrc->ptr;
    if (training) {
        cllm_training_free(training);
    }
}

static void cllm_vocabulary_dtor(zend_resource *rsrc TSRMLS_DC) {
    CLLMVocabulary *vocab = (CLLMVocabulary *)rsrc->ptr;
    if (vocab) {
        cllm_vocab_destroy(vocab);
    }
}

/* Resource types */
static int le_cllm_model;
static int le_cllm_training;
static int le_cllm_vocabulary;

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(cllm)
{
    le_cllm_model = zend_register_list_destructors_ex(
        cllm_model_dtor, NULL, "CLLM Model", module_number);
    le_cllm_training = zend_register_list_destructors_ex(
        cllm_training_dtor, NULL, "CLLM Training", module_number);
    le_cllm_vocabulary = zend_register_list_destructors_ex(
        cllm_vocabulary_dtor, NULL, "CLLM Vocabulary", module_number);
    
    // Register constants
    REGISTER_LONG_CONSTANT("CLLM_PLATONIC_TETRAHEDRON", PLATONIC_TETRAHEDRON, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CLLM_PLATONIC_CUBE", PLATONIC_CUBE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CLLM_PLATONIC_OCTAHEDRON", PLATONIC_OCTAHEDRON, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CLLM_PLATONIC_DODECAHEDRON", PLATONIC_DODECAHEDRON, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("CLLM_PLATONIC_ICOSAHEDRON", PLATONIC_ICOSAHEDRON, CONST_CS | CONST_PERSISTENT);
    
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(cllm)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(cllm)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "CLLM Support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CLLM_VERSION);
    php_info_print_table_row(2, "Features", "Model Creation, Training, Inference, Vocabulary");
    php_info_print_table_end();
}
/* }}} */

// ============================================================================
// MODEL FUNCTIONS
// ============================================================================

/* {{{ proto resource cllm_create_model(int platonic_solid, int vocab_size, array options)
   Create a new CLLM model */
PHP_FUNCTION(cllm_create_model)
{
    zend_long platonic_solid, vocab_size;
    zval *options = NULL;
    
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_LONG(platonic_solid)
        Z_PARAM_LONG(vocab_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();
    
    // Create config
    CLLMConfig config = cllm_default_config((PlatonicSolidType)platonic_solid, (uint32_t)vocab_size);
    
    // Apply options if provided
    if (options) {
        zval *val;
        
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "embedding_dim", sizeof("embedding_dim")-1)) != NULL) {
            config.embedding_dim = (uint32_t)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "hidden_dim", sizeof("hidden_dim")-1)) != NULL) {
            config.hidden_dim = (uint32_t)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "num_layers", sizeof("num_layers")-1)) != NULL) {
            config.num_layers = (uint32_t)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "num_heads", sizeof("num_heads")-1)) != NULL) {
            config.num_heads = (uint32_t)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "max_seq_len", sizeof("max_seq_len")-1)) != NULL) {
            config.max_seq_len = (uint32_t)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "learning_rate", sizeof("learning_rate")-1)) != NULL) {
            config.learning_rate = zval_get_double(val);
        }
    }
    
    // Create model
    CLLMModel *model = cllm_create_model(&config);
    if (!model) {
        php_error_docref(NULL, E_WARNING, "Failed to create CLLM model");
        RETURN_FALSE;
    }
    
    RETURN_RES(zend_register_resource(model, le_cllm_model));
}
/* }}} */

/* {{{ proto bool cllm_free_model(resource model)
   Free a CLLM model */
PHP_FUNCTION(cllm_free_model)
{
    zval *zmodel;
    CLLMModel *model;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zmodel)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((model = (CLLMModel *)zend_fetch_resource(Z_RES_P(zmodel), "CLLM Model", le_cllm_model)) == NULL) {
        RETURN_FALSE;
    }
    
    zend_list_close(Z_RES_P(zmodel));
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool cllm_save_model(resource model, string filename)
   Save a CLLM model to file */
PHP_FUNCTION(cllm_save_model)
{
    zval *zmodel;
    char *filename;
    size_t filename_len;
    CLLMModel *model;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zmodel)
        Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((model = (CLLMModel *)zend_fetch_resource(Z_RES_P(zmodel), "CLLM Model", le_cllm_model)) == NULL) {
        RETURN_FALSE;
    }
    
    int result = cllm_write_model(model, filename);
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ proto resource cllm_load_model(string filename)
   Load a CLLM model from file */
PHP_FUNCTION(cllm_load_model)
{
    char *filename;
    size_t filename_len;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END();
    
    CLLMModel *model = cllm_read_model(filename);
    if (!model) {
        php_error_docref(NULL, E_WARNING, "Failed to load CLLM model from %s", filename);
        RETURN_FALSE;
    }
    
    RETURN_RES(zend_register_resource(model, le_cllm_model));
}
/* }}} */

/* {{{ proto array cllm_model_info(resource model)
   Get information about a CLLM model */
PHP_FUNCTION(cllm_model_info)
{
    zval *zmodel;
    CLLMModel *model;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zmodel)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((model = (CLLMModel *)zend_fetch_resource(Z_RES_P(zmodel), "CLLM Model", le_cllm_model)) == NULL) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_long(return_value, "vocab_size", model->vocab_size);
    add_assoc_long(return_value, "embedding_dim", model->embedding_dim);
    add_assoc_long(return_value, "hidden_dim", model->hidden_dim);
    add_assoc_long(return_value, "num_layers", model->num_layers);
    add_assoc_long(return_value, "num_heads", model->num_heads);
    add_assoc_long(return_value, "max_seq_len", model->max_seq_len);
    add_assoc_long(return_value, "platonic_solid", model->platonic_solid);
    add_assoc_double(return_value, "learning_rate", model->optimizer.learning_rate);
}
/* }}} */

// ============================================================================
// VOCABULARY FUNCTIONS
// ============================================================================

/* {{{ proto resource cllm_vocab_create(int capacity)
   Create a new vocabulary */
PHP_FUNCTION(cllm_vocab_create)
{
    zend_long capacity;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(capacity)
    ZEND_PARSE_PARAMETERS_END();
    
    CLLMVocabulary *vocab = cllm_vocab_create_with_special_tokens(
        (uint32_t)capacity, "[PAD]", "[UNK]", "[BOS]", "[EOS]");
    
    if (!vocab) {
        php_error_docref(NULL, E_WARNING, "Failed to create vocabulary");
        RETURN_FALSE;
    }
    
    RETURN_RES(zend_register_resource(vocab, le_cllm_vocabulary));
}
/* }}} */

/* {{{ proto int cllm_vocab_build_from_file(resource vocab, string filename)
   Build vocabulary from a text file */
PHP_FUNCTION(cllm_vocab_build_from_file)
{
    zval *zvocab;
    char *filename;
    size_t filename_len;
    CLLMVocabulary *vocab;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvocab)
        Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((vocab = (CLLMVocabulary *)zend_fetch_resource(Z_RES_P(zvocab), "CLLM Vocabulary", le_cllm_vocabulary)) == NULL) {
        RETURN_FALSE;
    }
    
    uint32_t tokens_added = cllm_vocab_build_from_file(vocab, filename);
    RETURN_LONG(tokens_added);
}
/* }}} */

/* {{{ proto array cllm_vocab_tokenize(resource vocab, string text)
   Tokenize text into token IDs */
PHP_FUNCTION(cllm_vocab_tokenize)
{
    zval *zvocab;
    char *text;
    size_t text_len;
    CLLMVocabulary *vocab;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvocab)
        Z_PARAM_STRING(text, text_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((vocab = (CLLMVocabulary *)zend_fetch_resource(Z_RES_P(zvocab), "CLLM Vocabulary", le_cllm_vocabulary)) == NULL) {
        RETURN_FALSE;
    }
    
    uint32_t num_tokens;
    uint32_t *tokens = cllm_vocab_tokenize(vocab, text, &num_tokens);
    
    if (!tokens) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    for (uint32_t i = 0; i < num_tokens; i++) {
        add_next_index_long(return_value, tokens[i]);
    }
    
    free(tokens);
}
/* }}} */

/* {{{ proto string cllm_vocab_detokenize(resource vocab, array token_ids)
   Convert token IDs back to text */
PHP_FUNCTION(cllm_vocab_detokenize)
{
    zval *zvocab, *ztoken_ids, *entry;
    CLLMVocabulary *vocab;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvocab)
        Z_PARAM_ARRAY(ztoken_ids)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((vocab = (CLLMVocabulary *)zend_fetch_resource(Z_RES_P(zvocab), "CLLM Vocabulary", le_cllm_vocabulary)) == NULL) {
        RETURN_FALSE;
    }
    
    uint32_t num_tokens = zend_hash_num_elements(Z_ARRVAL_P(ztoken_ids));
    uint32_t *tokens = (uint32_t *)emalloc(num_tokens * sizeof(uint32_t));
    
    uint32_t i = 0;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(ztoken_ids), entry) {
        tokens[i++] = (uint32_t)zval_get_long(entry);
    } ZEND_HASH_FOREACH_END();
    
    char *text = cllm_vocab_detokenize(vocab, tokens, num_tokens);
    efree(tokens);
    
    if (!text) {
        RETURN_FALSE;
    }
    
    RETVAL_STRING(text);
    free(text);
}
/* }}} */

/* {{{ proto bool cllm_vocab_save(resource vocab, string filename)
   Save vocabulary to file */
PHP_FUNCTION(cllm_vocab_save)
{
    zval *zvocab;
    char *filename;
    size_t filename_len;
    CLLMVocabulary *vocab;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvocab)
        Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((vocab = (CLLMVocabulary *)zend_fetch_resource(Z_RES_P(zvocab), "CLLM Vocabulary", le_cllm_vocabulary)) == NULL) {
        RETURN_FALSE;
    }
    
    bool result = cllm_vocab_save(vocab, filename);
    RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto resource cllm_vocab_load(string filename)
   Load vocabulary from file */
PHP_FUNCTION(cllm_vocab_load)
{
    char *filename;
    size_t filename_len;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END();
    
    CLLMVocabulary *vocab = cllm_vocab_load(filename);
    if (!vocab) {
        php_error_docref(NULL, E_WARNING, "Failed to load vocabulary from %s", filename);
        RETURN_FALSE;
    }
    
    RETURN_RES(zend_register_resource(vocab, le_cllm_vocabulary));
}
/* }}} */

/* {{{ proto array cllm_vocab_stats(resource vocab)
   Get vocabulary statistics */
PHP_FUNCTION(cllm_vocab_stats)
{
    zval *zvocab;
    CLLMVocabulary *vocab;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zvocab)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((vocab = (CLLMVocabulary *)zend_fetch_resource(Z_RES_P(zvocab), "CLLM Vocabulary", le_cllm_vocabulary)) == NULL) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_long(return_value, "size", vocab->size);
    add_assoc_long(return_value, "capacity", vocab->capacity);
    add_assoc_long(return_value, "total_tokens", vocab->total_tokens);
    add_assoc_long(return_value, "pad_token_id", vocab->pad_token_id);
    add_assoc_long(return_value, "unk_token_id", vocab->unk_token_id);
    add_assoc_long(return_value, "bos_token_id", vocab->bos_token_id);
    add_assoc_long(return_value, "eos_token_id", vocab->eos_token_id);
}
/* }}} */

// ============================================================================
// TRAINING FUNCTIONS
// ============================================================================

/* {{{ proto resource cllm_training_init(resource model, array config)
   Initialize training for a model */
PHP_FUNCTION(cllm_training_init)
{
    zval *zmodel, *zconfig;
    CLLMModel *model;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zmodel)
        Z_PARAM_ARRAY(zconfig)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((model = (CLLMModel *)zend_fetch_resource(Z_RES_P(zmodel), "CLLM Model", le_cllm_model)) == NULL) {
        RETURN_FALSE;
    }
    
    CLLMTrainingConfig config = {0};
    zval *val;
    
    if ((val = zend_hash_str_find(Z_ARRVAL_P(zconfig), "learning_rate", sizeof("learning_rate")-1)) != NULL) {
        config.learning_rate = zval_get_double(val);
    } else {
        config.learning_rate = 0.001;
    }
    
    if ((val = zend_hash_str_find(Z_ARRVAL_P(zconfig), "batch_size", sizeof("batch_size")-1)) != NULL) {
        config.batch_size = (int)zval_get_long(val);
    } else {
        config.batch_size = 32;
    }
    
    if ((val = zend_hash_str_find(Z_ARRVAL_P(zconfig), "num_epochs", sizeof("num_epochs")-1)) != NULL) {
        config.num_epochs = (int)zval_get_long(val);
    } else {
        config.num_epochs = 10;
    }
    
    if ((val = zend_hash_str_find(Z_ARRVAL_P(zconfig), "sequence_length", sizeof("sequence_length")-1)) != NULL) {
        config.sequence_length = (int)zval_get_long(val);
    } else {
        config.sequence_length = 128;
    }
    
    strcpy(config.optimizer, "adam");
    strcpy(config.lr_scheduler, "none");
    
    CLLMTraining *training = cllm_training_init(model, &config);
    if (!training) {
        php_error_docref(NULL, E_WARNING, "Failed to initialize training");
        RETURN_FALSE;
    }
    
    RETURN_RES(zend_register_resource(training, le_cllm_training));
}
/* }}} */

// ============================================================================
// INFERENCE FUNCTIONS
// ============================================================================

/* {{{ proto string cllm_generate(resource model, string prompt, array options)
   Generate text from a prompt */
PHP_FUNCTION(cllm_generate)
{
    zval *zmodel, *zoptions = NULL;
    char *prompt;
    size_t prompt_len;
    CLLMModel *model;
    
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_RESOURCE(zmodel)
        Z_PARAM_STRING(prompt, prompt_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(zoptions)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((model = (CLLMModel *)zend_fetch_resource(Z_RES_P(zmodel), "CLLM Model", le_cllm_model)) == NULL) {
        RETURN_FALSE;
    }
    
    // Parse options
    int max_tokens = 50;
    double temperature = 0.8;
    
    if (zoptions) {
        zval *val;
        if ((val = zend_hash_str_find(Z_ARRVAL_P(zoptions), "max_tokens", sizeof("max_tokens")-1)) != NULL) {
            max_tokens = (int)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(zoptions), "temperature", sizeof("temperature")-1)) != NULL) {
            temperature = zval_get_double(val);
        }
    }
    
    // For now, return a placeholder
    // TODO: Implement actual inference
    char result[1024];
    snprintf(result, sizeof(result), "%s [generated text with %d tokens at temp %.2f]", 
             prompt, max_tokens, temperature);
    
    RETURN_STRING(result);
}
/* }}} */

// ============================================================================
// MATH LIBRARY FUNCTIONS
// ============================================================================

/* {{{ proto int cllm_prime_nth(int n)
   Get the nth prime number */
PHP_FUNCTION(cllm_prime_nth)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    uint64_t prime = prime_nth((uint64_t)n);
    RETURN_LONG(prime);
}
/* }}} */

/* {{{ proto bool cllm_prime_is_prime(int number)
   Check if a number is prime */
PHP_FUNCTION(cllm_prime_is_prime)
{
    zend_long number;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(number)
    ZEND_PARSE_PARAMETERS_END();
    
    bool is_prime = prime_is_prime((uint64_t)number);
    RETURN_BOOL(is_prime);
}
/* }}} */

/* {{{ proto float cllm_math_exp(float x)
   Calculate e^x */
PHP_FUNCTION(cllm_math_exp)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_exp(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float cllm_math_log(float x)
   Calculate natural logarithm */
PHP_FUNCTION(cllm_math_log)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_log(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float cllm_math_sin(float x)
   Calculate sine */
PHP_FUNCTION(cllm_math_sin)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_sin(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto float cllm_math_cos(float x)
   Calculate cosine */
PHP_FUNCTION(cllm_math_cos)
{
    double x;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    
    double result = math_cos(x);
    RETURN_DOUBLE(result);
}
/* }}} */

/* {{{ proto string cllm_version()
   Get CLLM version */
PHP_FUNCTION(cllm_version)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_STRING(PHP_CLLM_VERSION);
}
/* }}} */

// ============================================================================
// FUNCTION ENTRIES
// ============================================================================

/* Argument info declarations */
ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_create_model, 0, 0, 2)
    ZEND_ARG_INFO(0, platonic_solid)
    ZEND_ARG_INFO(0, vocab_size)
    ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_resource, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_save_model, 0, 0, 2)
    ZEND_ARG_INFO(0, model)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_load_model, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_vocab_create, 0, 0, 1)
    ZEND_ARG_INFO(0, capacity)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_vocab_build, 0, 0, 2)
    ZEND_ARG_INFO(0, vocab)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_vocab_tokenize, 0, 0, 2)
    ZEND_ARG_INFO(0, vocab)
    ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_vocab_detokenize, 0, 0, 2)
    ZEND_ARG_INFO(0, vocab)
    ZEND_ARG_ARRAY_INFO(0, token_ids, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_training_init, 0, 0, 2)
    ZEND_ARG_INFO(0, model)
    ZEND_ARG_ARRAY_INFO(0, config, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_generate, 0, 0, 2)
    ZEND_ARG_INFO(0, model)
    ZEND_ARG_INFO(0, prompt)
    ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_prime_nth, 0, 0, 1)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_math_func, 0, 0, 1)
    ZEND_ARG_INFO(0, x)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cllm_version, 0, 0, 0)
ZEND_END_ARG_INFO()

/* Function entries */
static const zend_function_entry cllm_functions[] = {
    // Model functions
    PHP_FE(cllm_create_model, arginfo_cllm_create_model)
    PHP_FE(cllm_free_model, arginfo_cllm_resource)
    PHP_FE(cllm_save_model, arginfo_cllm_save_model)
    PHP_FE(cllm_load_model, arginfo_cllm_load_model)
    PHP_FE(cllm_model_info, arginfo_cllm_resource)
    
    // Vocabulary functions
    PHP_FE(cllm_vocab_create, arginfo_cllm_vocab_create)
    PHP_FE(cllm_vocab_build_from_file, arginfo_cllm_vocab_build)
    PHP_FE(cllm_vocab_tokenize, arginfo_cllm_vocab_tokenize)
    PHP_FE(cllm_vocab_detokenize, arginfo_cllm_vocab_detokenize)
    PHP_FE(cllm_vocab_save, arginfo_cllm_save_model)
    PHP_FE(cllm_vocab_load, arginfo_cllm_load_model)
    PHP_FE(cllm_vocab_stats, arginfo_cllm_resource)
    
    // Training functions
    PHP_FE(cllm_training_init, arginfo_cllm_training_init)
    
    // Inference functions
    PHP_FE(cllm_generate, arginfo_cllm_generate)
    
    // Math functions
    PHP_FE(cllm_prime_nth, arginfo_cllm_prime_nth)
    PHP_FE(cllm_prime_is_prime, arginfo_cllm_prime_nth)
    PHP_FE(cllm_math_exp, arginfo_cllm_math_func)
    PHP_FE(cllm_math_log, arginfo_cllm_math_func)
    PHP_FE(cllm_math_sin, arginfo_cllm_math_func)
    PHP_FE(cllm_math_cos, arginfo_cllm_math_func)
    
    // Utility functions
    PHP_FE(cllm_version, arginfo_cllm_version)
    
    PHP_FE_END
};

/* Module entry */
zend_module_entry cllm_module_entry = {
    STANDARD_MODULE_HEADER,
    "cllm",
    cllm_functions,
    PHP_MINIT(cllm),
    PHP_MSHUTDOWN(cllm),
    NULL,
    NULL,
    PHP_MINFO(cllm),
    PHP_CLLM_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CLLM
ZEND_GET_MODULE(cllm)
#endif