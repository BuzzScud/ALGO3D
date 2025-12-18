/**
 * @file signal_recovery_block_impl.cc
 * @brief GNU Radio Signal Recovery Block Implementation
 */

#include "signal_recovery_block_impl.h"
#include <gnuradio/io_signature.h>
#include <complex>

namespace gr {
namespace recovery {

signal_recovery_block::sptr
signal_recovery_block::make(double sample_rate,
                            double center_freq,
                            int max_iterations,
                            double convergence_threshold,
                            bool use_reference)
{
    return gnuradio::get_initial_sptr(
        new signal_recovery_block_impl(sample_rate,
                                       center_freq,
                                       max_iterations,
                                       convergence_threshold,
                                       use_reference));
}

signal_recovery_block_impl::signal_recovery_block_impl(double sample_rate,
                                                       double center_freq,
                                                       int max_iterations,
                                                       double convergence_threshold,
                                                       bool use_reference)
    : gr::sync_block("signal_recovery_block",
                     gr::io_signature::make(1, 1, sizeof(gr_complex)),
                     gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_sample_rate(sample_rate),
      d_center_freq(center_freq),
      d_max_iterations(max_iterations),
      d_convergence_threshold(convergence_threshold),
      d_use_reference(use_reference),
      d_last_iterations(0),
      d_last_quality(0.0),
      d_last_converged(false),
      d_reference_ready(false)
{
    // Reserve space for reference samples
    if (d_use_reference) {
        d_reference_samples.reserve(1024);
    }
}

signal_recovery_block_impl::~signal_recovery_block_impl()
{
}

int signal_recovery_block_impl::work(int noutput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
{
    const gr_complex *in = (const gr_complex *) input_items[0];
    gr_complex *out = (gr_complex *) output_items[0];

    // If using reference samples and not ready, collect them
    if (d_use_reference && !d_reference_ready) {
        for (int i = 0; i < noutput_items && d_reference_samples.size() < 1024; i++) {
            d_reference_samples.push_back(in[i]);
        }
        
        if (d_reference_samples.size() >= 1024) {
            d_reference_ready = true;
        }
        
        // Pass through while collecting reference
        memcpy(out, in, noutput_items * sizeof(gr_complex));
        return noutput_items;
    }

    // Perform recovery on input samples
    signal_recovery_result_t* result = recover_radio_signal(
        (const double complex*)in,
        noutput_items,
        (uint32_t)d_sample_rate,
        d_center_freq,
        MODULATION_UNKNOWN
    );

    if (result && result->converged && result->recovered_signal) {
        // Copy recovered samples to output
        memcpy(out, result->recovered_signal->samples, 
               noutput_items * sizeof(gr_complex));
        
        // Update statistics
        d_last_iterations = result->iterations;
        d_last_quality = result->quality_score;
        d_last_converged = result->converged;
        
        signal_free_result(result);
    } else {
        // Recovery failed, pass through
        memcpy(out, in, noutput_items * sizeof(gr_complex));
        
        if (result) {
            d_last_iterations = result->iterations;
            d_last_quality = result->quality_score;
            d_last_converged = false;
            signal_free_result(result);
        }
    }

    return noutput_items;
}

} /* namespace recovery */
} /* namespace gr */