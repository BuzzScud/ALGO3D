"""
Universal Recovery System - Python Bindings

Production-grade Python bindings for the Universal Recovery System.

Modules:
- core: Core recovery algorithms (OBJECTIVE 28 Phase 1-6)
- crypto: Cryptographic recovery (Bitcoin, SSH, ransomware)
- network: Network protocol operations (Bitcoin all networks)
- signal: Signal processing recovery (audio, radio, time series)

Example usage:

    from recovery import core, crypto, signal
    
    # Core recovery
    ctx = core.RecoveryContext(max_iterations=5000)
    ctx.set_q(corrupted_data)
    ctx.add_sample(sample_data, offset=0, confidence=1.0)
    result = ctx.run()
    
    # Bitcoin key recovery
    result = crypto.recover_bitcoin_key(
        partial_key=partial_key_data,
        public_key=public_key_data,
        network='mainnet'
    )
    
    # Signal recovery
    result = signal.recover_audio(
        corrupted_audio,
        sample_rate=44100,
        num_channels=2
    )
"""

__version__ = '1.0.0'
__author__ = 'NinjaTech AI'
__license__ = 'MIT'

from . import core
from . import crypto
from . import network
from . import signal

__all__ = ['core', 'crypto', 'network', 'signal']