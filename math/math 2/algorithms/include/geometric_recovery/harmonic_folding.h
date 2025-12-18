/**
 * @file harmonic_folding.h
 * @brief Harmonic Folding for Data Compression and Recovery
 * 
 * Public API for harmonic folding system.
 */

#ifndef HARMONIC_FOLDING_H
#define HARMONIC_FOLDING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct FoldedData FoldedData;

/**
 * @brief Fold data using harmonic folding
 * 
 * @param data Input data
 * @param size Data size
 * @param num_harmonics Number of harmonic components (1-12)
 * @param fold_depth Folding depth (1-5)
 * @return Folded data structure, or NULL on error
 */
FoldedData* harmonic_fold_data(
    const double* data,
    uint32_t size,
    uint32_t num_harmonics,
    uint32_t fold_depth
);

/**
 * @brief Unfold data using harmonic folding
 * 
 * @param folded Folded data structure
 * @param unfolded_data Output buffer (must be allocated)
 * @param target_size Target size (should match original size)
 * @return 0 on success, -1 on error
 */
int harmonic_unfold_data(
    const FoldedData* folded,
    double* unfolded_data,
    uint32_t target_size
);

/**
 * @brief Free folded data
 */
void harmonic_free_folded_data(FoldedData* folded);

/**
 * @brief Get compression ratio
 */
double harmonic_get_compression_ratio(const FoldedData* folded);

/**
 * @brief Compute reconstruction error
 * 
 * Returns RMS error between original and reconstructed data
 */
double harmonic_compute_error(
    const double* original,
    const double* reconstructed,
    uint32_t size
);

/**
 * @brief Print harmonic fold statistics
 */
void harmonic_print_stats(const FoldedData* folded);

#ifdef __cplusplus
}
#endif

#endif /* HARMONIC_FOLDING_H */