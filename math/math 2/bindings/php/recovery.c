/*
 * PHP Recovery Extension - Universal Recovery System
 * Provides PHP bindings for all recovery libraries
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_recovery.h"

#include <recovery/recovery_core.h>
#include <recovery/recovery_crypto.h>
#include <recovery/recovery_network.h>
#include <recovery/recovery_signal.h>

/* Resource types */
static int le_recovery_context;
static int le_network_client;

/* Module initialization */
PHP_MINIT_FUNCTION(recovery)
{
    le_recovery_context = zend_register_list_destructors_ex(
        NULL, NULL, "recovery_context", module_number);
    le_network_client = zend_register_list_destructors_ex(
        NULL, NULL, "network_client", module_number);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(recovery)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(recovery)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "recovery support", "enabled");
    php_info_print_table_row(2, "Version", PHP_RECOVERY_VERSION);
    php_info_print_table_row(2, "Core Library", "librecovery_core");
    php_info_print_table_row(2, "Crypto Library", "librecovery_crypto");
    php_info_print_table_row(2, "Network Library", "librecovery_network");
    php_info_print_table_row(2, "Signal Library", "librecovery_signal");
    php_info_print_table_end();
}

/* Function entries */
const zend_function_entry recovery_functions[] = {
    PHP_FE(recovery_create_context,     NULL)
    PHP_FE(recovery_destroy_context,    NULL)
    PHP_FE(recovery_add_sample,         NULL)
    PHP_FE(recovery_recover,            NULL)
    PHP_FE(recovery_get_iterations,     NULL)
    PHP_FE(recovery_get_quality,        NULL)
    PHP_FE(recovery_crypto_recover_bitcoin_key,     NULL)
    PHP_FE(recovery_crypto_recover_ssh_key,         NULL)
    PHP_FE(recovery_crypto_recover_ecdsa_nonce,     NULL)
    PHP_FE(recovery_crypto_recover_ransomware_key,  NULL)
    PHP_FE(recovery_network_create_client,      NULL)
    PHP_FE(recovery_network_destroy_client,     NULL)
    PHP_FE(recovery_network_get_block,          NULL)
    PHP_FE(recovery_network_get_transaction,    NULL)
    PHP_FE(recovery_network_send_transaction,   NULL)
    PHP_FE(recovery_network_get_balance,        NULL)
    PHP_FE(recovery_signal_recover_audio,       NULL)
    PHP_FE(recovery_signal_recover_radio,       NULL)
    PHP_FE(recovery_signal_recover_time_series, NULL)
    PHP_FE(recovery_signal_denoise,             NULL)
    PHP_FE(recovery_signal_resample,            NULL)
    PHP_FE(recovery_signal_get_snr,             NULL)
    PHP_FE_END
};

/* Module entry */
zend_module_entry recovery_module_entry = {
    STANDARD_MODULE_HEADER,
    "recovery",
    recovery_functions,
    PHP_MINIT(recovery),
    PHP_MSHUTDOWN(recovery),
    NULL, NULL,
    PHP_MINFO(recovery),
    PHP_RECOVERY_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_RECOVERY
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(recovery)
#endif

/* Core Recovery Functions */
PHP_FUNCTION(recovery_create_context)
{
    zend_long max_iterations = 1000;
    double convergence_threshold = 1e-6;
    
    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(max_iterations)
        Z_PARAM_DOUBLE(convergence_threshold)
    ZEND_PARSE_PARAMETERS_END();
    
    recovery_context_t *ctx = recovery_create_context((int)max_iterations, convergence_threshold);
    if (!ctx) {
        php_error_docref(NULL, E_WARNING, "Failed to create recovery context");
        RETURN_FALSE;
    }
    
    RETURN_RES(zend_register_resource(ctx, le_recovery_context));
}

PHP_FUNCTION(recovery_destroy_context)
{
    zval *zcontext;
    recovery_context_t *ctx;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcontext)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((ctx = (recovery_context_t *)zend_fetch_resource(Z_RES_P(zcontext),
            "recovery_context", le_recovery_context)) == NULL) {
        RETURN_FALSE;
    }
    
    recovery_destroy_context(ctx);
    zend_list_close(Z_RES_P(zcontext));
    RETURN_TRUE;
}

PHP_FUNCTION(recovery_add_sample)
{
    zval *zcontext;
    char *data;
    size_t data_len;
    recovery_context_t *ctx;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zcontext)
        Z_PARAM_STRING(data, data_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((ctx = (recovery_context_t *)zend_fetch_resource(Z_RES_P(zcontext),
            "recovery_context", le_recovery_context)) == NULL) {
        RETURN_FALSE;
    }
    
    int result = recovery_add_sample(ctx, (const uint8_t *)data, data_len);
    RETURN_BOOL(result == 0);
}

PHP_FUNCTION(recovery_recover)
{
    zval *zcontext;
    char *data;
    size_t data_len;
    recovery_context_t *ctx;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zcontext)
        Z_PARAM_STRING(data, data_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((ctx = (recovery_context_t *)zend_fetch_resource(Z_RES_P(zcontext),
            "recovery_context", le_recovery_context)) == NULL) {
        RETURN_FALSE;
    }
    
    uint8_t *recovered = (uint8_t *)emalloc(data_len);
    if (!recovered) {
        RETURN_FALSE;
    }
    
    int result = recovery_recover(ctx, (const uint8_t *)data, recovered, data_len);
    if (result != 0) {
        efree(recovered);
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)recovered, data_len);
    efree(recovered);
}

PHP_FUNCTION(recovery_get_iterations)
{
    zval *zcontext;
    recovery_context_t *ctx;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcontext)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((ctx = (recovery_context_t *)zend_fetch_resource(Z_RES_P(zcontext),
            "recovery_context", le_recovery_context)) == NULL) {
        RETURN_FALSE;
    }
    
    RETURN_LONG(recovery_get_iterations(ctx));
}

PHP_FUNCTION(recovery_get_quality)
{
    zval *zcontext;
    recovery_context_t *ctx;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcontext)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((ctx = (recovery_context_t *)zend_fetch_resource(Z_RES_P(zcontext),
            "recovery_context", le_recovery_context)) == NULL) {
        RETURN_FALSE;
    }
    
    RETURN_DOUBLE(recovery_get_quality(ctx));
}

/* Crypto Recovery Functions */
PHP_FUNCTION(recovery_crypto_recover_bitcoin_key)
{
    char *partial_key;
    size_t key_len;
    zend_long network = 0;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(partial_key, key_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(network)
    ZEND_PARSE_PARAMETERS_END();
    
    char recovered_key[256];
    int result = recovery_crypto_recover_bitcoin_key(
        (const uint8_t *)partial_key, key_len,
        (recovery_network_type_t)network,
        recovered_key, sizeof(recovered_key)
    );
    
    if (result != 0) {
        RETURN_FALSE;
    }
    
    RETURN_STRING(recovered_key);
}

PHP_FUNCTION(recovery_crypto_recover_ssh_key)
{
    char *partial_key;
    size_t key_len;
    zend_long key_type = 0;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(partial_key, key_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(key_type)
    ZEND_PARSE_PARAMETERS_END();
    
    char recovered_key[4096];
    int result = recovery_crypto_recover_ssh_key(
        (const uint8_t *)partial_key, key_len,
        (recovery_ssh_key_type_t)key_type,
        recovered_key, sizeof(recovered_key)
    );
    
    if (result != 0) {
        RETURN_FALSE;
    }
    
    RETURN_STRING(recovered_key);
}

PHP_FUNCTION(recovery_crypto_recover_ecdsa_nonce)
{
    char *signature, *message, *public_key;
    size_t sig_len, msg_len, key_len;
    
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STRING(signature, sig_len)
        Z_PARAM_STRING(message, msg_len)
        Z_PARAM_STRING(public_key, key_len)
    ZEND_PARSE_PARAMETERS_END();
    
    uint8_t nonce[32];
    int result = recovery_crypto_recover_ecdsa_nonce(
        (const uint8_t *)signature, sig_len,
        (const uint8_t *)message, msg_len,
        (const uint8_t *)public_key, key_len,
        nonce, sizeof(nonce)
    );
    
    if (result != 0) {
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)nonce, sizeof(nonce));
}

PHP_FUNCTION(recovery_crypto_recover_ransomware_key)
{
    char *encrypted, *plaintext;
    size_t enc_len, plain_len;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(encrypted, enc_len)
        Z_PARAM_STRING(plaintext, plain_len)
    ZEND_PARSE_PARAMETERS_END();
    
    uint8_t key[32];
    int result = recovery_crypto_recover_ransomware_key(
        (const uint8_t *)encrypted, enc_len,
        (const uint8_t *)plaintext, plain_len,
        key, sizeof(key)
    );
    
    if (result != 0) {
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)key, sizeof(key));
}

/* Network Recovery Functions */
PHP_FUNCTION(recovery_network_create_client)
{
    char *url, *user, *pass;
    size_t url_len, user_len, pass_len;
    
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STRING(url, url_len)
        Z_PARAM_STRING(user, user_len)
        Z_PARAM_STRING(pass, pass_len)
    ZEND_PARSE_PARAMETERS_END();
    
    recovery_network_client_t *client = recovery_network_create_client(url, user, pass);
    if (!client) {
        RETURN_FALSE;
    }
    
    RETURN_RES(zend_register_resource(client, le_network_client));
}

PHP_FUNCTION(recovery_network_destroy_client)
{
    zval *zclient;
    recovery_network_client_t *client;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zclient)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((client = (recovery_network_client_t *)zend_fetch_resource(Z_RES_P(zclient),
            "network_client", le_network_client)) == NULL) {
        RETURN_FALSE;
    }
    
    recovery_network_destroy_client(client);
    zend_list_close(Z_RES_P(zclient));
    RETURN_TRUE;
}

PHP_FUNCTION(recovery_network_get_block)
{
    zval *zclient;
    char *block_hash;
    size_t hash_len;
    recovery_network_client_t *client;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zclient)
        Z_PARAM_STRING(block_hash, hash_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((client = (recovery_network_client_t *)zend_fetch_resource(Z_RES_P(zclient),
            "network_client", le_network_client)) == NULL) {
        RETURN_FALSE;
    }
    
    recovery_block_t block;
    int result = recovery_network_get_block(client, block_hash, &block);
    if (result != 0) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_string(return_value, "hash", block.hash);
    add_assoc_long(return_value, "height", block.height);
    add_assoc_long(return_value, "timestamp", block.timestamp);
}

PHP_FUNCTION(recovery_network_get_transaction)
{
    zval *zclient;
    char *tx_hash;
    size_t hash_len;
    recovery_network_client_t *client;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zclient)
        Z_PARAM_STRING(tx_hash, hash_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((client = (recovery_network_client_t *)zend_fetch_resource(Z_RES_P(zclient),
            "network_client", le_network_client)) == NULL) {
        RETURN_FALSE;
    }
    
    recovery_transaction_t tx;
    int result = recovery_network_get_transaction(client, tx_hash, &tx);
    if (result != 0) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_string(return_value, "txid", tx.txid);
    add_assoc_long(return_value, "confirmations", tx.confirmations);
}

PHP_FUNCTION(recovery_network_send_transaction)
{
    zval *zclient;
    char *raw_tx;
    size_t tx_len;
    recovery_network_client_t *client;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zclient)
        Z_PARAM_STRING(raw_tx, tx_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((client = (recovery_network_client_t *)zend_fetch_resource(Z_RES_P(zclient),
            "network_client", le_network_client)) == NULL) {
        RETURN_FALSE;
    }
    
    char txid[65];
    int result = recovery_network_send_transaction(client, raw_tx, txid, sizeof(txid));
    if (result != 0) {
        RETURN_FALSE;
    }
    
    RETURN_STRING(txid);
}

PHP_FUNCTION(recovery_network_get_balance)
{
    zval *zclient;
    char *address;
    size_t addr_len;
    recovery_network_client_t *client;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zclient)
        Z_PARAM_STRING(address, addr_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if ((client = (recovery_network_client_t *)zend_fetch_resource(Z_RES_P(zclient),
            "network_client", le_network_client)) == NULL) {
        RETURN_FALSE;
    }
    
    double balance;
    int result = recovery_network_get_balance(client, address, &balance);
    if (result != 0) {
        RETURN_FALSE;
    }
    
    RETURN_DOUBLE(balance);
}

/* Signal Recovery Functions */
PHP_FUNCTION(recovery_signal_recover_audio)
{
    char *audio;
    size_t audio_len;
    zend_long sample_rate, channels;
    
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STRING(audio, audio_len)
        Z_PARAM_LONG(sample_rate)
        Z_PARAM_LONG(channels)
    ZEND_PARSE_PARAMETERS_END();
    
    size_t sample_count = audio_len / sizeof(float);
    float *recovered = (float *)emalloc(audio_len);
    if (!recovered) {
        RETURN_FALSE;
    }
    
    int result = recovery_signal_recover_audio(
        (const float *)audio, sample_count,
        (int)sample_rate, (int)channels, recovered
    );
    
    if (result != 0) {
        efree(recovered);
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)recovered, audio_len);
    efree(recovered);
}

PHP_FUNCTION(recovery_signal_recover_radio)
{
    char *iq_data;
    size_t iq_len;
    zend_long sample_rate;
    double center_freq;
    
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STRING(iq_data, iq_len)
        Z_PARAM_LONG(sample_rate)
        Z_PARAM_DOUBLE(center_freq)
    ZEND_PARSE_PARAMETERS_END();
    
    size_t sample_count = iq_len / (2 * sizeof(float));
    float *recovered = (float *)emalloc(iq_len);
    if (!recovered) {
        RETURN_FALSE;
    }
    
    int result = recovery_signal_recover_radio(
        (const float *)iq_data, sample_count,
        (int)sample_rate, center_freq, recovered
    );
    
    if (result != 0) {
        efree(recovered);
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)recovered, iq_len);
    efree(recovered);
}

PHP_FUNCTION(recovery_signal_recover_time_series)
{
    char *data;
    size_t data_len;
    double sample_interval;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(data, data_len)
        Z_PARAM_DOUBLE(sample_interval)
    ZEND_PARSE_PARAMETERS_END();
    
    size_t sample_count = data_len / sizeof(double);
    double *recovered = (double *)emalloc(data_len);
    if (!recovered) {
        RETURN_FALSE;
    }
    
    int result = recovery_signal_recover_time_series(
        (const double *)data, sample_count,
        sample_interval, recovered
    );
    
    if (result != 0) {
        efree(recovered);
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)recovered, data_len);
    efree(recovered);
}

PHP_FUNCTION(recovery_signal_denoise)
{
    char *signal;
    size_t signal_len;
    double threshold;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(signal, signal_len)
        Z_PARAM_DOUBLE(threshold)
    ZEND_PARSE_PARAMETERS_END();
    
    size_t sample_count = signal_len / sizeof(float);
    float *denoised = (float *)emalloc(signal_len);
    if (!denoised) {
        RETURN_FALSE;
    }
    
    int result = recovery_signal_denoise(
        (const float *)signal, sample_count,
        threshold, denoised
    );
    
    if (result != 0) {
        efree(denoised);
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)denoised, signal_len);
    efree(denoised);
}

PHP_FUNCTION(recovery_signal_resample)
{
    char *signal;
    size_t signal_len;
    zend_long old_rate, new_rate;
    
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STRING(signal, signal_len)
        Z_PARAM_LONG(old_rate)
        Z_PARAM_LONG(new_rate)
    ZEND_PARSE_PARAMETERS_END();
    
    size_t old_count = signal_len / sizeof(float);
    size_t new_count = (old_count * new_rate) / old_rate;
    size_t new_len = new_count * sizeof(float);
    
    float *resampled = (float *)emalloc(new_len);
    if (!resampled) {
        RETURN_FALSE;
    }
    
    int result = recovery_signal_resample(
        (const float *)signal, old_count,
        (int)old_rate, (int)new_rate,
        resampled, new_count
    );
    
    if (result != 0) {
        efree(resampled);
        RETURN_FALSE;
    }
    
    RETVAL_STRINGL((char *)resampled, new_len);
    efree(resampled);
}

PHP_FUNCTION(recovery_signal_get_snr)
{
    char *signal, *noise;
    size_t signal_len, noise_len;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(signal, signal_len)
        Z_PARAM_STRING(noise, noise_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if (signal_len != noise_len) {
        php_error_docref(NULL, E_WARNING, "Signal and noise must have same length");
        RETURN_FALSE;
    }
    
    size_t sample_count = signal_len / sizeof(float);
    double snr = recovery_signal_get_snr(
        (const float *)signal,
        (const float *)noise,
        sample_count
    );
    
    RETURN_DOUBLE(snr);
}