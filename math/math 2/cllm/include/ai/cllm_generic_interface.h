/**
 * @file cllm_generic_interface.h
 * @brief CLLM Implementation of Generic Model Interface
 * 
 * This file provides the bridge between CLLM and the generic model
 * interface defined in the algorithms library. It allows CLLM to work
 * with the threading system without creating circular dependencies.
 * 
 * Architecture:
 *   Math → Algorithms (generic interface) → CLLM (implements interface)
 *   
 * No circular dependencies!
 */

#ifndef CLLM_GENERIC_INTERFACE_H
#define CLLM_GENERIC_INTERFACE_H

#include "cllm.h"
#include "generic_model.h"  // From algorithms library

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a generic model interface for a CLLM model
 * 
 * This wraps a CLLMModel in the generic interface so it can be used
 * by worker threads without creating a circular dependency.
 * 
 * The generic interface holds a pointer to the CLLMModel but does NOT
 * own it. The CLLMModel must remain valid for the lifetime of the
 * generic interface.
 * 
 * @param model CLLM model to wrap
 * @return Generic model interface, or NULL on error
 */
GenericModel* cllm_create_generic_interface(CLLMModel* model);

/**
 * Free a generic interface created for CLLM
 * 
 * This frees the GenericModel structure but does NOT free the
 * underlying CLLMModel. The CLLMModel is managed separately.
 * 
 * @param generic Generic interface to free
 */
void cllm_free_generic_interface(GenericModel* generic);

#ifdef __cplusplus
}
#endif

#endif // CLLM_GENERIC_INTERFACE_H