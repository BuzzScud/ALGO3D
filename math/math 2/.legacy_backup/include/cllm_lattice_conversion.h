#ifndef CLLM_LATTICE_CONVERSION_H
#define CLLM_LATTICE_CONVERSION_H

#include "abacus_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif

void cllm_float_to_bigfixed(BigFixed* output, float* input, int n, int precision);
void cllm_bigfixed_to_float(float* output, BigFixed* input, int n);
void cllm_embeddings_to_basis(BigFixed** basis, float* embeddings, int n, int dim, int precision);
void cllm_basis_to_embeddings(float* embeddings, BigFixed** basis, int n, int dim);
BigFixed** cllm_alloc_bigfixed_basis(int n, int dim);
void cllm_free_bigfixed_basis(BigFixed** basis, int n);
void cllm_embedding_to_bigfixed(BigFixed* output, float* embedding, int dim, int precision);
void cllm_bigfixed_to_embedding(float* embedding, BigFixed* vector, int dim);
float cllm_test_conversion_accuracy(float* input, int n, int precision);
void cllm_print_conversion_stats(float* input, int n, int precision);

#ifdef __cplusplus
}
#endif

#endif
