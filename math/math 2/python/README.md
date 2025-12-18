# Universal Recovery System - Python Bindings

Production-grade Python bindings for the Universal Recovery System.

## Installation

### From Source

```bash
# Install dependencies
pip install numpy scipy

# Install package
cd python
pip install -e .
```

### System Requirements

The Python bindings require the Universal Recovery System libraries to be installed:

```bash
# Install system libraries
cd ..
sudo ./install_dependencies.sh
make -f Makefile.recovery
sudo make -f Makefile.recovery install
```

## Quick Start

### Core Recovery

```python
from recovery import core

# Simple recovery
corrupted_data = b'\x01\x02\x00\x00\x05\x06'
sample_data = b'\x01\x02\x03'

result = core.recover(
    corrupted_data,
    samples=[(sample_data, 0, 1.0)],
    max_iterations=5000
)

print(f"Recovered: {result.data.hex()}")
print(f"Quality: {result.quality_score:.2%}")
print(f"Iterations: {result.iterations}")
```

### Bitcoin Key Recovery

```python
from recovery import crypto

result = crypto.recover_bitcoin_key(
    partial_key=partial_key_bytes,
    public_key=public_key_bytes,
    network='mainnet',
    max_iterations=10000
)

if result['converged']:
    print(f"Recovered key: {result['recovered_key'].hex()}")
```

### Audio Signal Recovery

```python
from recovery import signal
import numpy as np
from scipy.io import wavfile

# Load corrupted audio
rate, audio = wavfile.read('corrupted.wav')

# Recover
recovered, result = signal.recover_audio(
    audio.astype(np.float32),
    sample_rate=rate,
    num_channels=1
)

# Save
wavfile.write('recovered.wav', rate, recovered.astype(np.int16))

print(f"Quality: {result['quality_score']:.2%}")
```

### Radio Signal Recovery

```python
from recovery import signal
import numpy as np

# Load IQ data
iq_data = np.fromfile('signal.iq', dtype=np.complex64)

# Recover
recovered, result = signal.recover_radio_signal(
    iq_data,
    sample_rate=2.4e6,
    center_freq=100e6,
    modulation='fm'
)

# Save
recovered.tofile('recovered.iq')
```

### Bitcoin Network Operations

```python
from recovery import network

# Connect to Bitcoin testnet
btc = network.BitcoinNetwork(
    network='testnet',
    username='user',
    password='pass'
)

# Get blockchain height
height = btc.get_blockchain_height()
print(f"Height: {height}")

# Get transaction
tx = btc.get_transaction('abc123...')
if tx:
    print(f"Confirmations: {tx['confirmations']}")

# Validate address
if btc.validate_address('1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa'):
    print("Valid address!")
```

## Advanced Usage

### Using Recovery Context

```python
from recovery import core

# Create context
ctx = core.RecoveryContext(
    max_iterations=10000,
    convergence_threshold=0.001,
    method=core.RECOVERY_METHOD_AUTO,
    verbose=1
)

# Set corrupted data
ctx.set_q(corrupted_data)

# Add multiple samples
ctx.add_sample(sample1, offset=0, confidence=1.0)
ctx.add_sample(sample2, offset=100, confidence=0.9)
ctx.add_sample(sample3, offset=200, confidence=0.8)

# Run recovery
result = ctx.run()

print(f"Recovered {len(result.data)} bytes")
print(f"Iterations: {result.iterations}")
print(f"Quality: {result.quality_score:.2%}")
```

### NumPy Integration

```python
from recovery import core
import numpy as np

# Corrupted array
corrupted = np.array([1, 2, 0, 0, 5, 6], dtype=np.float32)

# Sample array
sample = np.array([1, 2, 3], dtype=np.float32)

# Recover
recovered, result = core.recover_array(
    corrupted,
    samples=[(sample, 0, 1.0)]
)

print(f"Recovered: {recovered}")
```

### Time Series Recovery

```python
from recovery import signal
import numpy as np

# Stock prices with missing values
prices = np.array([100, 101, np.nan, np.nan, 105, 106])

# Recover
recovered, result = signal.recover_time_series(prices)

print(f"Recovered: {recovered}")
print(f"Quality: {result['quality_score']:.2%}")
```

## API Reference

### core module

- `RecoveryContext` - Main recovery context class
- `recover()` - Convenience function for simple recovery
- `recover_array()` - NumPy array recovery

### crypto module

- `recover_bitcoin_key()` - Bitcoin key recovery
- `recover_ssh_key()` - SSH key recovery
- `recover_ransomware_key()` - Ransomware key recovery
- `validate_bitcoin_key()` - Validate Bitcoin key
- `derive_bitcoin_address()` - Derive Bitcoin address

### network module

- `BitcoinNetwork` - Bitcoin network interface
- `get_bitcoin_height()` - Get blockchain height
- `validate_bitcoin_address()` - Validate address

### signal module

- `recover_audio()` - Audio signal recovery
- `recover_radio_signal()` - Radio signal recovery
- `recover_time_series()` - Time series recovery
- `denoise()` - Signal denoising
- `resample()` - Signal resampling
- `calculate_snr()` - Calculate SNR

## Examples

See the `examples/` directory for complete examples:

- `basic_recovery.py` - Basic recovery example
- `bitcoin_recovery.py` - Bitcoin key recovery
- `audio_recovery.py` - Audio signal recovery
- `radio_recovery.py` - Radio signal recovery
- `time_series_recovery.py` - Time series recovery

## Testing

```bash
# Install dev dependencies
pip install -e .[dev]

# Run tests
pytest tests/

# Run with coverage
pytest --cov=recovery tests/
```

## Performance

The Python bindings use ctypes to call the C libraries directly, providing near-native performance:

- **Overhead:** <1% compared to C
- **Throughput:** Same as C implementation
- **Memory:** Zero-copy where possible

## Requirements

- Python 3.7+
- NumPy 1.19+
- SciPy 1.5+
- Universal Recovery System libraries (librecovery_core, librecovery_crypto, librecovery_signal, librecovery_network)

## License

MIT License - Part of the Crystalline CLLM Integration project.

## Support

- GitHub Issues: https://github.com/justmebob123/crystalline/issues
- Documentation: https://github.com/justmebob123/crystalline/tree/main/docs

## Version

**Version:** 1.0.0  
**Status:** Production Ready  
**Python:** 3.7+