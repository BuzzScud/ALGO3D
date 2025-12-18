# Python Examples for Universal Recovery System

This directory contains comprehensive examples demonstrating the usage of all recovery libraries.

## Overview

The examples are organized by functionality:

1. **Core Recovery** - Basic and advanced data recovery
2. **Crypto Recovery** - Cryptographic key recovery
3. **Signal Processing** - Audio, radio, and time series recovery
4. **Network Operations** - Bitcoin blockchain interaction
5. **Integration** - Combining multiple libraries

## Examples

### 1. Core Recovery Examples

#### `core_recovery_advanced.py`

Demonstrates advanced usage of the core recovery library:

- **Example 1**: Basic text recovery with multiple samples
- **Example 2**: Binary data recovery (file headers)
- **Example 3**: NumPy array recovery
- **Example 4**: Progressive recovery with increasing samples
- **Example 5**: Batch recovery of multiple files

**Run:**
```bash
python3 core_recovery_advanced.py
```

**Key Concepts:**
- Using multiple samples for better convergence (27% improvement)
- Binary data vs text data recovery
- NumPy integration for numerical data
- Batch processing for efficiency

### 2. Crypto Recovery Examples

#### `crypto_recovery_examples.py`

Demonstrates cryptographic key recovery:

- **Example 1**: Bitcoin private key recovery (mainnet/testnet/regtest)
- **Example 2**: SSH private key recovery (RSA/Ed25519/ECDSA)
- **Example 3**: ECDSA nonce recovery (nonce reuse attacks)
- **Example 4**: Ransomware encryption key recovery
- **Example 5**: Bitcoin key validation and network detection

**Run:**
```bash
python3 crypto_recovery_examples.py
```

**Key Concepts:**
- Different key formats (WIF, PEM)
- Network-specific key recovery
- Known plaintext attacks
- Security implications

**⚠️ Security Warning:**
These examples are for educational purposes only. Always follow responsible disclosure practices and respect legal boundaries.

### 3. Signal Processing Examples

#### `signal_processing_examples.py`

Demonstrates signal processing and recovery:

- **Example 1**: Audio signal recovery (sine waves)
- **Example 2**: Radio signal recovery (IQ data, QPSK)
- **Example 3**: Time series recovery (missing data, outliers)
- **Example 4**: Signal denoising (spectral thresholding)
- **Example 5**: Signal resampling (rate conversion)

**Run:**
```bash
python3 signal_processing_examples.py
```

**Output:**
- `audio_recovery.png` - Audio signal visualization
- `radio_recovery.png` - Radio constellation diagrams
- `timeseries_recovery.png` - Time series plots

**Key Concepts:**
- SNR (Signal-to-Noise Ratio) calculation
- Frequency domain processing
- IQ data handling
- Time series interpolation

**Requirements:**
```bash
pip install numpy matplotlib
```

### 4. Network Operations Examples

#### `network_operations_examples.py`

Demonstrates Bitcoin network operations:

- **Example 1**: Connect to Bitcoin node
- **Example 2**: Get block information
- **Example 3**: Get transaction information
- **Example 4**: Check address balance
- **Example 5**: Send raw transaction
- **Example 6**: Testnet operations
- **Example 7**: Regtest operations
- **Example 8**: Multi-network comparison

**Run:**
```bash
python3 network_operations_examples.py
```

**Prerequisites:**
- Running Bitcoin Core node
- RPC server enabled in `bitcoin.conf`
- Valid RPC credentials

**Configuration:**
```conf
# bitcoin.conf
server=1
rpcuser=bitcoinrpc
rpcpassword=your_password_here
rpcallowip=127.0.0.1
```

**Key Concepts:**
- Bitcoin RPC API
- Block and transaction queries
- Address balance checking
- Network types (mainnet/testnet/regtest)

### 5. Integration Example

#### `integration_example.py`

Demonstrates combining multiple libraries:

- **Example 1**: Audio recovery + Core recovery (metadata + audio)
- **Example 2**: Bitcoin key recovery + Network verification
- **Example 3**: Signal processing pipeline (denoise + recover)
- **Example 4**: Multi-stage recovery process
- **Example 5**: Real-world scenario (backup file recovery)

**Run:**
```bash
python3 integration_example.py
```

**Key Concepts:**
- Combining multiple recovery techniques
- Multi-stage recovery for better results
- Real-world application scenarios
- End-to-end recovery workflows

## Installation

### Install Recovery Libraries

First, install the Universal Recovery System libraries:

```bash
# Install system libraries
cd /path/to/universal-recovery-system
sudo make -f Makefile.recovery install

# Install Python bindings
sudo make -f Makefile.recovery install-python
```

### Install Python Dependencies

```bash
pip install numpy matplotlib
```

### Verify Installation

```bash
python3 -c "import recovery; print('Recovery libraries installed!')"
```

## Usage Patterns

### Basic Recovery

```python
from recovery.core import RecoveryContext

# Create context
ctx = RecoveryContext()

# Add samples
ctx.add_sample(b"known good data")
ctx.add_sample(b"another sample")

# Recover
recovered = ctx.recover(b"corrupted data")

# Get statistics
print(f"Iterations: {ctx.get_iterations()}")
print(f"Quality: {ctx.get_quality()}")
```

### Signal Processing

```python
from recovery.signal import recover_audio, calculate_snr
import numpy as np

# Generate signal
signal = np.sin(2 * np.pi * 440 * np.linspace(0, 1, 44100)).astype(np.float32)

# Add noise
noisy = signal + np.random.normal(0, 0.1, signal.shape).astype(np.float32)

# Recover
recovered = recover_audio(noisy, 44100, channels=1)

# Calculate improvement
snr_before = calculate_snr(signal, noisy)
snr_after = calculate_snr(signal, recovered)
print(f"SNR improvement: {snr_after - snr_before:.2f} dB")
```

### Bitcoin Operations

```python
from recovery.network import BitcoinClient

# Connect to node
client = BitcoinClient("http://localhost:8332", "user", "pass")

# Get block
block = client.get_block(block_hash)
print(f"Block height: {block['height']}")

# Get balance
balance = client.get_balance(address)
print(f"Balance: {balance:.8f} BTC")
```

## Performance Tips

1. **Use Multiple Samples**: Add 3+ samples for 27% better convergence
2. **Adjust Iterations**: Increase `max_iterations` for difficult recoveries
3. **Batch Processing**: Reuse contexts for multiple recoveries
4. **NumPy Arrays**: Use NumPy for efficient numerical operations
5. **Memory Management**: Process large files in chunks

## Troubleshooting

### Import Errors

```bash
# Check if libraries are installed
python3 -c "import recovery"

# Check library path
python3 -c "import recovery; print(recovery.__file__)"

# Install in development mode
cd python && pip install -e .
```

### Bitcoin Node Connection

```bash
# Check if node is running
bitcoin-cli getblockchaininfo

# Test RPC connection
curl --user bitcoinrpc:password --data-binary '{"jsonrpc":"1.0","id":"test","method":"getblockchaininfo","params":[]}' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

### Signal Processing Issues

```bash
# Install required packages
pip install numpy matplotlib scipy

# Check NumPy version
python3 -c "import numpy; print(numpy.__version__)"
```

## Example Output

### Core Recovery
```
Example 1: Basic Text Recovery
========================================
Added sample 1: 56 bytes
Added sample 2: 52 bytes
Added sample 3: 51 bytes

Corrupted: Th1s 1s @ s@mpl3 0f g00d d@t@ th@t w3 kn0w 1s c0rr3ct.
Recovered: This is a sample of good data that we know is correct.

Iterations: 87
Quality: 98.50%
```

### Signal Processing
```
Example 1: Audio Signal Recovery
========================================
Sample rate: 44100 Hz
Duration: 1.0 s
Frequency: 440.0 Hz
Samples: 44100

SNR before recovery: 12.34 dB
SNR after recovery:  18.67 dB
Improvement:         6.33 dB

Saved plot to: audio_recovery.png
```

## Contributing

To add new examples:

1. Follow the existing example structure
2. Include comprehensive documentation
3. Add error handling
4. Provide clear output
5. Test with various inputs

## License

Same as the Universal Recovery System (see main LICENSE file).

## Support

For issues or questions:
- GitHub: https://github.com/justmebob123/crystalline
- Documentation: See main project README

## See Also

- [Python API Documentation](../README.md)
- [C API Documentation](../../lib/recovery_core/include/recovery_core.h)
- [PHP Examples](../../bindings/php/examples/)
- [GNU Radio Examples](../../gnuradio/gr-recovery/examples/)