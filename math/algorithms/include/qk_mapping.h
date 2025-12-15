/**
 * @file qk_mapping.h
 * @brief Q→k mapping for general inference (Query to Key)
 * 
 * Maps Query vectors (Q) to Key values (k) using tensor distance
 * calculations and crystalline geometry. This is a general algorithm
 * applicable to AI inference, signal processing, search, and more.
 * 
 * The mapping uses:
 * - Tensor distance: det(Q1 - Q2) with golden ratio geometry
 * - Entropy-guided search space reduction
 * - Harmonic alignment for validation
 */

#ifndef QK_MAPPING_H
#define QK_MAPPING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Q→k mapping context
 */
typedef struct {
    double* Q_vector;           /**< Query embedding vector */
    size_t Q_dim;               /**< Dimension of Q vector */
    uint64_t* k_candidates;     /**< Candidate key values */
    size_t num_candidates;      /**< Number of candidates */
    double* distances;          /**< Tensor distances for each candidate */
    uint64_t p_mod;             /**< Modulus for tensor calculations */
} QtoK_Mapping;

/**
 * @brief Create Q→k mapper
 * 
 * @param Q_dim Dimension of query vectors
 * @param num_candidates Number of candidate keys
 * @return Mapper context, or NULL on failure
 */
QtoK_Mapping* qk_mapper_create(size_t Q_dim, size_t num_candidates);

/**
 * @brief Destroy Q→k mapper
 * 
 * @param mapper Mapper to destroy
 */
void qk_mapper_destroy(QtoK_Mapping* mapper);

/**
 * @brief Find best k for given Q query
 * 
 * Searches through candidates to find k with minimum tensor distance.
 * Uses tensor_distance() from tensor_ops.h
 * 
 * @param mapper Mapper context
 * @param Q_query Query vector
 * @return Best matching key value
 */
uint64_t qk_find_best(const QtoK_Mapping* mapper,
                      const double* Q_query);

/**
 * @brief Find top N best k candidates
 * 
 * Returns N candidates with smallest tensor distances.
 * 
 * @param mapper Mapper context
 * @param Q_query Query vector
 * @param top_k Output array for top k values (pre-allocated, size N)
 * @param N Number of top candidates to return
 * @return Number of candidates found
 */
size_t qk_find_top_n(const QtoK_Mapping* mapper,
                     const double* Q_query,
                     uint64_t* top_k, size_t N);

/**
 * @brief Set candidate keys
 * 
 * @param mapper Mapper context
 * @param candidates Array of candidate keys
 * @param num_candidates Number of candidates
 * @return true on success
 */
bool qk_set_candidates(QtoK_Mapping* mapper,
                       const uint64_t* candidates,
                       size_t num_candidates);

#ifdef __cplusplus
}
#endif

#endif /* QK_MAPPING_H */
