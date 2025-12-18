/*
 * PHP Recovery Extension
 * Provides PHP bindings for Universal Recovery System libraries
 */

#ifndef PHP_RECOVERY_H
#define PHP_RECOVERY_H

extern zend_module_entry recovery_module_entry;
#define phpext_recovery_ptr &recovery_module_entry

#define PHP_RECOVERY_VERSION "1.0.0"

#ifdef PHP_WIN32
#   define PHP_RECOVERY_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_RECOVERY_API __attribute__ ((visibility("default")))
#else
#   define PHP_RECOVERY_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/*
 * Core Recovery Functions
 */
PHP_FUNCTION(recovery_create_context);
PHP_FUNCTION(recovery_destroy_context);
PHP_FUNCTION(recovery_add_sample);
PHP_FUNCTION(recovery_recover);
PHP_FUNCTION(recovery_get_iterations);
PHP_FUNCTION(recovery_get_quality);

/*
 * Crypto Recovery Functions
 */
PHP_FUNCTION(recovery_crypto_recover_bitcoin_key);
PHP_FUNCTION(recovery_crypto_recover_ssh_key);
PHP_FUNCTION(recovery_crypto_recover_ecdsa_nonce);
PHP_FUNCTION(recovery_crypto_recover_ransomware_key);

/*
 * Network Recovery Functions
 */
PHP_FUNCTION(recovery_network_create_client);
PHP_FUNCTION(recovery_network_destroy_client);
PHP_FUNCTION(recovery_network_get_block);
PHP_FUNCTION(recovery_network_get_transaction);
PHP_FUNCTION(recovery_network_send_transaction);
PHP_FUNCTION(recovery_network_get_balance);

/*
 * Signal Recovery Functions
 */
PHP_FUNCTION(recovery_signal_recover_audio);
PHP_FUNCTION(recovery_signal_recover_radio);
PHP_FUNCTION(recovery_signal_recover_time_series);
PHP_FUNCTION(recovery_signal_denoise);
PHP_FUNCTION(recovery_signal_resample);
PHP_FUNCTION(recovery_signal_get_snr);

#endif  /* PHP_RECOVERY_H */