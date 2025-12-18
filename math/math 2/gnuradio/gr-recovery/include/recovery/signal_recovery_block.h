/**
 * @file signal_recovery_block.h
 * @brief GNU Radio Signal Recovery Block
 * 
 * GNU Radio block that uses the Universal Recovery System to recover
 * corrupted signals in real-time.
 */

#ifndef INCLUDED_RECOVERY_SIGNAL_RECOVERY_BLOCK_H
#define INCLUDED_RECOVERY_SIGNAL_RECOVERY_BLOCK_H

#include <gnuradio/sync_block.h>

namespace gr {
namespace recovery {

/**
 * @brief Signal Recovery Block
 * 
 * This block recovers corrupted signals using the Universal Recovery System.
 * It integrates OBJECTIVE 28 Phase 1-6 algorithms for real-time signal recovery.
 * 
 * Input: Corrupted complex IQ samples
 * Output: Recovered complex IQ samples
 * 
 * Parameters:
 * - sample_rate: Sample rate in Hz
 * - center_freq: Center frequency in Hz
 * - max_iterations: Maximum recovery iterations
 * - convergence_threshold: Convergence threshold
 * - use_reference: Use reference samples for better recovery
 */
class signal_recovery_block : virtual public gr::sync_block
{
public:
    typedef boost::shared_ptr<signal_recovery_block> sptr;

    /**
     * @brief Create a signal recovery block
     * 
     * @param sample_rate Sample rate (Hz)
     * @param center_freq Center frequency (Hz)
     * @param max_iterations Maximum iterations
     * @param convergence_threshold Convergence threshold
     * @param use_reference Use reference samples
     * @return Shared pointer to block
     */
    static sptr make(double sample_rate,
                     double center_freq,
                     int max_iterations,
                     double convergence_threshold,
                     bool use_reference);

    /**
     * @brief Set sample rate
     */
    virtual void set_sample_rate(double rate) = 0;

    /**
     * @brief Set center frequency
     */
    virtual void set_center_freq(double freq) = 0;

    /**
     * @brief Set max iterations
     */
    virtual void set_max_iterations(int iterations) = 0;

    /**
     * @brief Set convergence threshold
     */
    virtual void set_convergence_threshold(double threshold) = 0;

    /**
     * @brief Get recovery statistics
     */
    virtual int get_last_iterations() const = 0;
    virtual double get_last_quality() const = 0;
    virtual bool get_last_converged() const = 0;
};

} // namespace recovery
} // namespace gr

#endif /* INCLUDED_RECOVERY_SIGNAL_RECOVERY_BLOCK_H */