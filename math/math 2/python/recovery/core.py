"""
Universal Recovery System - Core Module

Python bindings for librecovery_core using ctypes.

This module provides access to the core recovery algorithms
implementing OBJECTIVE 28 Phase 1-6.
"""

import ctypes
import os
from typing import Optional, Tuple
import numpy as np

# Load library
_lib_path = os.environ.get('RECOVERY_LIB_PATH', '/usr/local/lib')
_lib = ctypes.CDLL(os.path.join(_lib_path, 'librecovery_core.so'))

# Error codes
RECOVERY_OK = 0
RECOVERY_ERROR_INVALID_PARAM = -1
RECOVERY_ERROR_OUT_OF_MEMORY = -2
RECOVERY_ERROR_NOT_CONVERGED = -3
RECOVERY_ERROR_NO_SAMPLES = -4
RECOVERY_ERROR_INTERNAL = -5

# Recovery methods
RECOVERY_METHOD_AUTO = 0
RECOVERY_METHOD_CRYPTO = 1
RECOVERY_METHOD_SIGNAL = 2
RECOVERY_METHOD_BINARY = 3
RECOVERY_METHOD_FAST = 4
RECOVERY_METHOD_ACCURATE = 5

# Structures
class RecoveryConfig(ctypes.Structure):
    """Recovery configuration"""
    _fields_ = [
        ('max_iterations', ctypes.c_int),
        ('convergence_threshold', ctypes.c_double),
        ('num_samples', ctypes.c_int),
        ('method', ctypes.c_int),
        ('verbose', ctypes.c_int),
        ('num_threads', ctypes.c_int),
        ('use_gpu', ctypes.c_bool)
    ]

class RecoveryResult(ctypes.Structure):
    """Recovery result"""
    _fields_ = [
        ('data', ctypes.POINTER(ctypes.c_uint8)),
        ('length', ctypes.c_size_t),
        ('iterations', ctypes.c_int),
        ('final_oscillation', ctypes.c_double),
        ('convergence_rate', ctypes.c_double),
        ('quality_score', ctypes.c_double),
        ('converged', ctypes.c_bool),
        ('time_seconds', ctypes.c_double)
    ]

# Function prototypes
_lib.recovery_default_config.restype = RecoveryConfig
_lib.recovery_default_config.argtypes = []

_lib.recovery_init.restype = ctypes.c_void_p
_lib.recovery_init.argtypes = [ctypes.POINTER(RecoveryConfig)]

_lib.recovery_free.restype = None
_lib.recovery_free.argtypes = [ctypes.c_void_p]

_lib.recovery_set_q.restype = ctypes.c_int
_lib.recovery_set_q.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t]

_lib.recovery_add_sample.restype = ctypes.c_int
_lib.recovery_add_sample.argtypes = [
    ctypes.c_void_p,
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.c_size_t,
    ctypes.c_double
]

_lib.recovery_run.restype = ctypes.c_int
_lib.recovery_run.argtypes = [ctypes.c_void_p]

_lib.recovery_get_result.restype = ctypes.POINTER(RecoveryResult)
_lib.recovery_get_result.argtypes = [ctypes.c_void_p]

_lib.recovery_free_result.restype = None
_lib.recovery_free_result.argtypes = [ctypes.POINTER(RecoveryResult)]


class RecoveryContext:
    """
    Recovery context for running recovery algorithms.
    
    Example:
        ctx = RecoveryContext(max_iterations=5000)
        ctx.set_q(corrupted_data)
        ctx.add_sample(sample_data, offset=0, confidence=1.0)
        result = ctx.run()
        print(f"Recovered {len(result.data)} bytes in {result.iterations} iterations")
    """
    
    def __init__(self,
                 max_iterations: int = 10000,
                 convergence_threshold: float = 0.001,
                 method: int = RECOVERY_METHOD_AUTO,
                 verbose: int = 0,
                 num_threads: int = 0,
                 use_gpu: bool = False):
        """
        Initialize recovery context.
        
        Args:
            max_iterations: Maximum iterations (default: 10000)
            convergence_threshold: Convergence threshold (default: 0.001)
            method: Recovery method (default: AUTO)
            verbose: Verbose level (0=quiet, 1=normal, 2=debug)
            num_threads: Number of threads (0=auto)
            use_gpu: Use GPU acceleration if available
        """
        config = RecoveryConfig()
        config.max_iterations = max_iterations
        config.convergence_threshold = convergence_threshold
        config.num_samples = 0
        config.method = method
        config.verbose = verbose
        config.num_threads = num_threads
        config.use_gpu = use_gpu
        
        self._ctx = _lib.recovery_init(ctypes.byref(config))
        if not self._ctx:
            raise RuntimeError("Failed to initialize recovery context")
    
    def __del__(self):
        """Clean up context"""
        if hasattr(self, '_ctx') and self._ctx:
            _lib.recovery_free(self._ctx)
    
    def set_q(self, data: bytes) -> None:
        """
        Set Q data (target/corrupted data).
        
        Args:
            data: Corrupted data as bytes
        """
        data_array = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
        err = _lib.recovery_set_q(self._ctx, data_array, len(data))
        if err != RECOVERY_OK:
            raise RuntimeError(f"Failed to set Q data: error {err}")
    
    def add_sample(self, data: bytes, offset: int = 0, confidence: float = 1.0) -> None:
        """
        Add sample data (known good data as anchor).
        
        Args:
            data: Sample data as bytes
            offset: Offset in result (default: 0)
            confidence: Confidence in sample 0-1 (default: 1.0)
        """
        data_array = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
        err = _lib.recovery_add_sample(self._ctx, data_array, len(data), offset, confidence)
        if err != RECOVERY_OK:
            raise RuntimeError(f"Failed to add sample: error {err}")
    
    def run(self) -> 'RecoveryResultWrapper':
        """
        Run recovery algorithm.
        
        Returns:
            RecoveryResultWrapper with recovered data and statistics
        """
        err = _lib.recovery_run(self._ctx)
        if err != RECOVERY_OK:
            raise RuntimeError(f"Recovery failed: error {err}")
        
        result_ptr = _lib.recovery_get_result(self._ctx)
        if not result_ptr:
            raise RuntimeError("Failed to get recovery result")
        
        return RecoveryResultWrapper(result_ptr)


class RecoveryResultWrapper:
    """
    Wrapper for recovery result.
    
    Attributes:
        data: Recovered data as bytes
        iterations: Number of iterations taken
        final_oscillation: Final oscillation value
        convergence_rate: Convergence rate
        quality_score: Quality score (0-1)
        converged: Whether recovery converged
        time_seconds: Time taken in seconds
    """
    
    def __init__(self, result_ptr):
        """Initialize from C result pointer"""
        self._ptr = result_ptr
        self._result = result_ptr.contents
        
        # Copy data
        self.data = bytes(self._result.data[:self._result.length])
        self.iterations = self._result.iterations
        self.final_oscillation = self._result.final_oscillation
        self.convergence_rate = self._result.convergence_rate
        self.quality_score = self._result.quality_score
        self.converged = self._result.converged
        self.time_seconds = self._result.time_seconds
    
    def __del__(self):
        """Clean up result"""
        if hasattr(self, '_ptr') and self._ptr:
            _lib.recovery_free_result(self._ptr)
    
    def __repr__(self):
        return (f"RecoveryResult(length={len(self.data)}, "
                f"iterations={self.iterations}, "
                f"quality={self.quality_score:.2%}, "
                f"converged={self.converged})")


def recover(corrupted_data: bytes,
           samples: list = None,
           max_iterations: int = 10000,
           convergence_threshold: float = 0.001,
           method: int = RECOVERY_METHOD_AUTO,
           verbose: int = 0) -> RecoveryResultWrapper:
    """
    Convenience function for simple recovery.
    
    Args:
        corrupted_data: Corrupted data as bytes
        samples: List of (data, offset, confidence) tuples
        max_iterations: Maximum iterations
        convergence_threshold: Convergence threshold
        method: Recovery method
        verbose: Verbose level
    
    Returns:
        RecoveryResultWrapper with recovered data
    
    Example:
        result = recover(
            corrupted_data,
            samples=[(sample1, 0, 1.0), (sample2, 100, 0.9)],
            max_iterations=5000
        )
        print(f"Recovered: {result.data}")
    """
    ctx = RecoveryContext(
        max_iterations=max_iterations,
        convergence_threshold=convergence_threshold,
        method=method,
        verbose=verbose
    )
    
    ctx.set_q(corrupted_data)
    
    if samples:
        for sample_data, offset, confidence in samples:
            ctx.add_sample(sample_data, offset, confidence)
    
    return ctx.run()


# Numpy integration
def recover_array(corrupted: np.ndarray,
                 samples: list = None,
                 **kwargs) -> Tuple[np.ndarray, RecoveryResultWrapper]:
    """
    Recover numpy array.
    
    Args:
        corrupted: Corrupted numpy array
        samples: List of (array, offset, confidence) tuples
        **kwargs: Additional arguments for recover()
    
    Returns:
        Tuple of (recovered_array, result)
    
    Example:
        corrupted = np.array([1, 2, 0, 0, 5], dtype=np.float32)
        recovered, result = recover_array(corrupted)
    """
    # Convert to bytes
    corrupted_bytes = corrupted.tobytes()
    
    # Convert samples
    if samples:
        samples_bytes = [
            (arr.tobytes(), offset, confidence)
            for arr, offset, confidence in samples
        ]
    else:
        samples_bytes = None
    
    # Recover
    result = recover(corrupted_bytes, samples_bytes, **kwargs)
    
    # Convert back to array
    recovered = np.frombuffer(result.data, dtype=corrupted.dtype)
    recovered = recovered.reshape(corrupted.shape)
    
    return recovered, result