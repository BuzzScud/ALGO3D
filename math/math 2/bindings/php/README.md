# PHP Recovery Extension

PHP bindings for the Universal Recovery System libraries.

## Overview

The PHP Recovery extension provides a complete PHP interface to all four recovery libraries:
- **Core Recovery** - Universal data recovery algorithms
- **Crypto Recovery** - Cryptographic key recovery (Bitcoin, SSH, ECDSA, ransomware)
- **Network Recovery** - Bitcoin network operations (RPC, transactions, blocks)
- **Signal Recovery** - Audio, radio, and time series signal processing

## Installation

### Prerequisites

1. PHP development files (php-dev or php-devel)
2. Universal Recovery System libraries installed
3. phpize tool

### Build and Install

```bash
cd bindings/php
phpize
./configure --with-recovery=/usr/local
make
sudo make install
```

### Enable the Extension

Add to your php.ini:
```ini
extension=recovery.so
```

Or load it at runtime:
```php
<?php
dl('recovery.so');
?>
```

### Verify Installation

```bash
php -m | grep recovery
```

Or in PHP:
```php
<?php
if (extension_loaded('recovery')) {
    echo "Recovery extension loaded!\n";
    phpinfo(INFO_MODULES);
}
?>
```

## API Reference

### Core Recovery Functions

#### recovery_create_context()
```php
resource recovery_create_context([int $max_iterations = 1000, float $convergence_threshold = 1e-6])
```
Create a new recovery context.

**Parameters:**
- `$max_iterations` - Maximum number of iterations (default: 1000)
- `$convergence_threshold` - Convergence threshold (default: 1e-6)

**Returns:** Recovery context resource or FALSE on failure

**Example:**
```php
$ctx = recovery_create_context(1000, 1e-6);
```

#### recovery_destroy_context()
```php
bool recovery_destroy_context(resource $context)
```
Destroy a recovery context and free resources.

**Parameters:**
- `$context` - Recovery context resource

**Returns:** TRUE on success, FALSE on failure

#### recovery_add_sample()
```php
bool recovery_add_sample(resource $context, string $data)
```
Add a sample (known good data) to the recovery context.

**Parameters:**
- `$context` - Recovery context resource
- `$data` - Sample data (binary string)

**Returns:** TRUE on success, FALSE on failure

**Example:**
```php
recovery_add_sample($ctx, "This is good data");
recovery_add_sample($ctx, "Another good sample");
```

#### recovery_recover()
```php
string recovery_recover(resource $context, string $corrupted_data)
```
Recover corrupted data using the samples in the context.

**Parameters:**
- `$context` - Recovery context resource
- `$corrupted_data` - Corrupted data to recover

**Returns:** Recovered data or FALSE on failure

**Example:**
```php
$recovered = recovery_recover($ctx, $corrupted_data);
```

#### recovery_get_iterations()
```php
int recovery_get_iterations(resource $context)
```
Get the number of iterations used in the last recovery.

**Returns:** Number of iterations

#### recovery_get_quality()
```php
float recovery_get_quality(resource $context)
```
Get the quality score (0.0 to 1.0) of the last recovery.

**Returns:** Quality score

### Crypto Recovery Functions

#### recovery_crypto_recover_bitcoin_key()
```php
string recovery_crypto_recover_bitcoin_key(string $partial_key[, int $network_type = 0])
```
Recover a Bitcoin private key from partial data.

**Parameters:**
- `$partial_key` - Partial key data
- `$network_type` - Network type (0=mainnet, 1=testnet, 2=regtest)

**Returns:** Recovered key (WIF format) or FALSE on failure

#### recovery_crypto_recover_ssh_key()
```php
string recovery_crypto_recover_ssh_key(string $partial_key[, int $key_type = 0])
```
Recover an SSH private key from partial data.

**Parameters:**
- `$partial_key` - Partial key data
- `$key_type` - Key type (0=RSA, 1=Ed25519, 2=ECDSA)

**Returns:** Recovered key (PEM format) or FALSE on failure

#### recovery_crypto_recover_ecdsa_nonce()
```php
string recovery_crypto_recover_ecdsa_nonce(string $signature, string $message, string $public_key)
```
Recover ECDSA nonce from signature (for nonce reuse attacks).

**Parameters:**
- `$signature` - ECDSA signature
- `$message` - Signed message
- `$public_key` - Public key

**Returns:** Recovered nonce (32 bytes) or FALSE on failure

#### recovery_crypto_recover_ransomware_key()
```php
string recovery_crypto_recover_ransomware_key(string $encrypted_file, string $known_plaintext)
```
Recover ransomware encryption key using known plaintext.

**Parameters:**
- `$encrypted_file` - Encrypted file data
- `$known_plaintext` - Known plaintext from the file

**Returns:** Recovered key (32 bytes) or FALSE on failure

### Network Recovery Functions

#### recovery_network_create_client()
```php
resource recovery_network_create_client(string $rpc_url, string $rpc_user, string $rpc_password)
```
Create a Bitcoin network client.

**Parameters:**
- `$rpc_url` - Bitcoin RPC URL (e.g., "http://localhost:8332")
- `$rpc_user` - RPC username
- `$rpc_password` - RPC password

**Returns:** Network client resource or FALSE on failure

#### recovery_network_destroy_client()
```php
bool recovery_network_destroy_client(resource $client)
```
Destroy a network client.

#### recovery_network_get_block()
```php
array recovery_network_get_block(resource $client, string $block_hash)
```
Get block information.

**Returns:** Array with keys: hash, height, timestamp, tx_count

#### recovery_network_get_transaction()
```php
array recovery_network_get_transaction(resource $client, string $tx_hash)
```
Get transaction information.

**Returns:** Array with keys: txid, confirmations, size

#### recovery_network_send_transaction()
```php
string recovery_network_send_transaction(resource $client, string $raw_tx)
```
Send a raw transaction to the network.

**Returns:** Transaction ID or FALSE on failure

#### recovery_network_get_balance()
```php
float recovery_network_get_balance(resource $client, string $address)
```
Get the balance of a Bitcoin address.

**Returns:** Balance in BTC or FALSE on failure

### Signal Recovery Functions

#### recovery_signal_recover_audio()
```php
string recovery_signal_recover_audio(string $corrupted_audio, int $sample_rate, int $channels)
```
Recover corrupted audio signal.

**Parameters:**
- `$corrupted_audio` - Corrupted audio data (float samples)
- `$sample_rate` - Sample rate in Hz (e.g., 44100)
- `$channels` - Number of channels (1=mono, 2=stereo)

**Returns:** Recovered audio data or FALSE on failure

#### recovery_signal_recover_radio()
```php
string recovery_signal_recover_radio(string $corrupted_iq, int $sample_rate, float $center_freq)
```
Recover corrupted radio signal (IQ data).

**Parameters:**
- `$corrupted_iq` - Corrupted IQ data (interleaved I/Q float samples)
- `$sample_rate` - Sample rate in Hz
- `$center_freq` - Center frequency in Hz

**Returns:** Recovered IQ data or FALSE on failure

#### recovery_signal_recover_time_series()
```php
string recovery_signal_recover_time_series(string $corrupted_data, float $sample_interval)
```
Recover corrupted time series data.

**Parameters:**
- `$corrupted_data` - Corrupted time series (double samples)
- `$sample_interval` - Time between samples in seconds

**Returns:** Recovered time series or FALSE on failure

#### recovery_signal_denoise()
```php
string recovery_signal_denoise(string $noisy_signal, float $threshold)
```
Denoise a signal using spectral thresholding.

**Parameters:**
- `$noisy_signal` - Noisy signal data (float samples)
- `$threshold` - Noise threshold (0.0 to 1.0)

**Returns:** Denoised signal or FALSE on failure

#### recovery_signal_resample()
```php
string recovery_signal_resample(string $signal, int $old_rate, int $new_rate)
```
Resample a signal to a new sample rate.

**Parameters:**
- `$signal` - Signal data (float samples)
- `$old_rate` - Original sample rate in Hz
- `$new_rate` - New sample rate in Hz

**Returns:** Resampled signal or FALSE on failure

#### recovery_signal_get_snr()
```php
float recovery_signal_get_snr(string $signal, string $noise)
```
Calculate signal-to-noise ratio.

**Parameters:**
- `$signal` - Clean signal data (float samples)
- `$noise` - Noisy signal data (float samples)

**Returns:** SNR in dB

## Examples

### Basic Recovery

```php
<?php
// Create context
$ctx = recovery_create_context();

// Add samples
recovery_add_sample($ctx, "This is good data");
recovery_add_sample($ctx, "Another sample");

// Recover corrupted data
$corrupted = "Th1s 1s g00d d@t@";
$recovered = recovery_recover($ctx, $corrupted);

echo "Recovered: $recovered\n";
echo "Iterations: " . recovery_get_iterations($ctx) . "\n";
echo "Quality: " . (recovery_get_quality($ctx) * 100) . "%\n";

recovery_destroy_context($ctx);
?>
```

### Bitcoin Key Recovery

```php
<?php
$partial_key = hex2bin("5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsre...");
$recovered = recovery_crypto_recover_bitcoin_key($partial_key, 0); // mainnet
echo "Recovered key: $recovered\n";
?>
```

### Audio Signal Processing

```php
<?php
// Generate sine wave
$sample_rate = 44100;
$samples = [];
for ($i = 0; $i < $sample_rate; $i++) {
    $samples[] = sin(2 * M_PI * 440 * $i / $sample_rate);
}
$audio = pack('f*', ...$samples);

// Add noise
$noisy = recovery_signal_denoise($audio, 0.1);

// Resample to 48kHz
$resampled = recovery_signal_resample($audio, 44100, 48000);
?>
```

### Bitcoin Network Operations

```php
<?php
$client = recovery_network_create_client(
    "http://localhost:8332",
    "bitcoinrpc",
    "password"
);

$block = recovery_network_get_block($client, $block_hash);
$balance = recovery_network_get_balance($client, $address);

recovery_network_destroy_client($client);
?>
```

## Performance Tips

1. **Reuse contexts**: Create one context and reuse it for multiple recoveries
2. **Add multiple samples**: More samples = better convergence (27% improvement with 3 samples)
3. **Adjust parameters**: Tune max_iterations and convergence_threshold for your use case
4. **Binary data**: Use pack()/unpack() for efficient binary data handling

## Error Handling

All functions return FALSE on failure and may trigger PHP warnings. Use error handling:

```php
<?php
$ctx = recovery_create_context();
if ($ctx === false) {
    die("Failed to create context\n");
}

$recovered = recovery_recover($ctx, $data);
if ($recovered === false) {
    error_log("Recovery failed");
    // Handle error
}
?>
```

## Thread Safety

The extension is thread-safe when using PHP in multi-threaded environments (e.g., with Apache's worker MPM).

## License

Same as the Universal Recovery System (see main project LICENSE).

## Support

For issues, questions, or contributions, see the main project repository.

## See Also

- [Python Bindings](../../python/README.md)
- [GNU Radio Module](../../gnuradio/gr-recovery/README.md)
- [C API Documentation](../../lib/recovery_core/include/recovery_core.h)