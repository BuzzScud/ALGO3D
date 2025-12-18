/**
 * @file signal_recovery_block_impl.h
 * @brief GNU Radio Signal Recovery Block Implementation
 */

#ifndef INCLUDED_RECOVERY_SIGNAL_RECOVERY_BLOCK_IMPL_H
#define INCLUDED_RECOVERY_SIGNAL_RECOVERY_BLOCK_IMPL_H

#include <recovery/signal_recovery_block.h>
#include <recovery_signal.h>

namespace gr {
namespace recovery {

class signal_recovery_block_impl : public signal_recovery_block
{
private:
    double d_sample_rate;
    double d_center_freq;
    int d_max_iterations;
    double d_convergence_threshold;
    bool d_use_reference;
    
    // Recovery statistics
    int d_last_iterations;
    double d_last_quality;
    bool d_last_converged;
    
    // Reference samples buffer
    std::vector<gr_complex> d_reference_samples;
    bool d_reference_ready;

public:
    signal_recovery_block_impl(double sample_rate,
                               double center_freq,
                               int max_iterations,
                               double convergence_threshold,
                               bool use_reference);
    
    ~signal_recovery_block_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items);

    void set_sample_rate(double rate) { d_sample_rate = rate; }
    void set_center_freq(double freq) { d_center_freq = freq; }
    void set_max_iterations(int iterations) { d_max_iterations = iterations; }
    void set_convergence_threshold(double threshold) { d_convergence_threshold = threshold; }
    
    int get_last_iterations() const { return d_last_iterations; }
    double get_last_quality() const { return d_last_quality; }
    bool get_last_converged() const { return d_last_converged; }
};

} // namespace recovery
} // namespace gr

#endif /* INCLUDED_RECOVERY_SIGNAL_RECOVERY_BLOCK_IMPL_H */