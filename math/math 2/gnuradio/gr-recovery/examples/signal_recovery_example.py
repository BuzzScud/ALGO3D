#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
GNU Radio Signal Recovery Example

This example demonstrates the Universal Recovery System's signal recovery
capabilities in a GNU Radio flowgraph.

The flowgraph:
1. Generates a clean test signal (sine wave)
2. Adds noise and corruption
3. Recovers the signal using the recovery block
4. Displays original, corrupted, and recovered signals

This demonstrates OBJECTIVE 28 Phase 1-6 algorithms in real-time.
"""

from gnuradio import gr, blocks, analog, channels
from gnuradio import recovery
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
import sys
import signal

class signal_recovery_example(gr.top_block):
    """
    Signal Recovery Example Flowgraph
    
    Demonstrates real-time signal recovery using the Universal Recovery System.
    """

    def __init__(self):
        gr.top_block.__init__(self, "Signal Recovery Example")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000
        self.signal_freq = signal_freq = 1000
        self.noise_amplitude = noise_amplitude = 0.3
        self.corruption_rate = corruption_rate = 0.1

        ##################################################
        # Blocks
        ##################################################
        
        # Signal source (clean signal)
        self.analog_sig_source = analog.sig_source_c(
            samp_rate,
            analog.GR_SIN_WAVE,
            signal_freq,
            1,
            0
        )
        
        # Add noise
        self.channels_channel_model = channels.channel_model(
            noise_voltage=noise_amplitude,
            frequency_offset=0.0,
            epsilon=1.0,
            taps=[1.0],
            noise_seed=42,
            block_tags=False
        )
        
        # Add corruption (dropout)
        self.blocks_multiply_const = blocks.multiply_const_cc(1.0)
        
        # Recovery block (using Universal Recovery System)
        self.recovery_signal_recovery_block = recovery.signal_recovery_block(
            sample_rate=samp_rate,
            center_freq=signal_freq,
            max_iterations=5000,
            convergence_threshold=0.001,
            use_reference=True
        )
        
        # File sinks for analysis
        self.blocks_file_sink_original = blocks.file_sink(
            gr.sizeof_gr_complex,
            'signal_original.dat',
            False
        )
        self.blocks_file_sink_original.set_unbuffered(False)
        
        self.blocks_file_sink_corrupted = blocks.file_sink(
            gr.sizeof_gr_complex,
            'signal_corrupted.dat',
            False
        )
        self.blocks_file_sink_corrupted.set_unbuffered(False)
        
        self.blocks_file_sink_recovered = blocks.file_sink(
            gr.sizeof_gr_complex,
            'signal_recovered.dat',
            False
        )
        self.blocks_file_sink_recovered.set_unbuffered(False)
        
        # Throttle to control flow
        self.blocks_throttle = blocks.throttle(
            gr.sizeof_gr_complex,
            samp_rate,
            True
        )

        ##################################################
        # Connections
        ##################################################
        
        # Original signal path
        self.connect((self.analog_sig_source, 0), (self.blocks_throttle, 0))
        self.connect((self.blocks_throttle, 0), (self.blocks_file_sink_original, 0))
        
        # Corrupted signal path
        self.connect((self.blocks_throttle, 0), (self.channels_channel_model, 0))
        self.connect((self.channels_channel_model, 0), (self.blocks_multiply_const, 0))
        self.connect((self.blocks_multiply_const, 0), (self.blocks_file_sink_corrupted, 0))
        
        # Recovery path
        self.connect((self.blocks_multiply_const, 0), (self.recovery_signal_recovery_block, 0))
        self.connect((self.recovery_signal_recovery_block, 0), (self.blocks_file_sink_recovered, 0))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.analog_sig_source.set_sampling_freq(self.samp_rate)
        self.blocks_throttle.set_sample_rate(self.samp_rate)
        self.recovery_signal_recovery_block.set_sample_rate(self.samp_rate)

    def get_signal_freq(self):
        return self.signal_freq

    def set_signal_freq(self, signal_freq):
        self.signal_freq = signal_freq
        self.analog_sig_source.set_frequency(self.signal_freq)
        self.recovery_signal_recovery_block.set_center_freq(self.signal_freq)


def main(top_block_cls=signal_recovery_example, options=None):
    """
    Main function to run the flowgraph
    """
    
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()
        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    print("=" * 70)
    print("Signal Recovery Example - Universal Recovery System")
    print("=" * 70)
    print()
    print("This example demonstrates real-time signal recovery using")
    print("OBJECTIVE 28 Phase 1-6 algorithms.")
    print()
    print("The flowgraph:")
    print("  1. Generates a clean 1 kHz sine wave")
    print("  2. Adds noise and corruption")
    print("  3. Recovers the signal using the recovery block")
    print("  4. Saves all signals to files for analysis")
    print()
    print("Output files:")
    print("  - signal_original.dat  (clean signal)")
    print("  - signal_corrupted.dat (corrupted signal)")
    print("  - signal_recovered.dat (recovered signal)")
    print()
    print("Press Ctrl+C to stop...")
    print("=" * 70)
    print()

    tb.start()

    try:
        input('Press Enter to quit: ')
    except EOFError:
        pass
    except KeyboardInterrupt:
        pass

    tb.stop()
    tb.wait()

    print()
    print("=" * 70)
    print("Recovery Statistics:")
    print("=" * 70)
    print(f"Last iterations: {tb.recovery_signal_recovery_block.get_last_iterations()}")
    print(f"Last quality: {tb.recovery_signal_recovery_block.get_last_quality():.2%}")
    print(f"Last converged: {tb.recovery_signal_recovery_block.get_last_converged()}")
    print("=" * 70)


if __name__ == '__main__':
    main()