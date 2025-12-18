/**
 * @file cllm_threading.h
 * @brief CLLM 88D Threading System
 * 
 * 88D ARCHITECTURE - THREADING IS MANDATORY:
 * - Threading is ALWAYS enabled - no optional threading
 * - 96 threads: 8 layers × 12 threads per layer (88 workers + 8 control)
 * - Threading is initialized automatically in cllm_create_model()
 * - No separate initialization or cleanup required
 * - All operations are thread-centric by default
 */

#ifndef AI_CLLM_THREADING_H
#define AI_CLLM_THREADING_H

#include "cllm.h"
#include "../../algorithms/include/hierarchical_threading.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CORE 88D THREADING API
// ============================================================================

/**
 * @brief Get the 88D thread pool from a model
 * 
 * In 88D architecture, every model has a thread pool.
 * This function provides access to the mandatory threading system.
 * 
 * @param model CLLM model (must be non-NULL)
 * @return Thread pool (always non-NULL in 88D)
 */
HierarchicalThreadPool* cllm_get_thread_pool(CLLMModel* model);

/**
 * @brief Get thread by layer and position
 * 
 * @param model CLLM model
 * @param layer Layer number (0-7)
 * @param position Clock position (0-11, where 0 is control)
 * @return Thread pointer
 */
HierarchicalThread* cllm_get_thread(CLLMModel* model, uint8_t layer, uint8_t position);

/**
 * @brief Get thread assigned to a specific token
 * 
 * Tokens have permanent thread assignments in 88D architecture.
 * 
 * @param model CLLM model
 * @param token_id Token ID
 * @return Thread assigned to this token
 */
HierarchicalThread* cllm_get_token_thread(CLLMModel* model, uint32_t token_id);

// ============================================================================
// GEOMETRY MAPPING
// ============================================================================

/**
 * @brief Map geometric structures to threads
 * 
 * Maps Platonic solids, clock positions, and other geometric structures
 * to the 96-thread 88D architecture.
 * 
 * @param model CLLM model
 * @return true on success
 */
bool cllm_map_geometry_to_threads(CLLMModel* model);

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

/**
 * @brief Start all threads in the 88D pool
 * 
 * @param model CLLM model
 * @return 0 on success, -1 on error
 */
int cllm_start_threads(CLLMModel* model);

/**
 * @brief Stop all threads in the 88D pool
 * 
 * @param model CLLM model
 * @return 0 on success, -1 on error
 */
int cllm_stop_threads(CLLMModel* model);

/**
 * @brief Wait for all threads to complete
 * 
 * @param model CLLM model
 * @return 0 on success, -1 on error
 */
int cllm_wait_for_threads(CLLMModel* model);

// ============================================================================
// THREAD-CENTRIC OPERATIONS
// ============================================================================

/**
 * @brief Start a specific thread
 * 
 * @param thread Thread to start
 * @return 0 on success, -1 on error
 */
int cllm_start_thread(HierarchicalThread* thread);

/**
 * @brief Stop a specific thread
 * 
 * @param thread Thread to stop
 * @return 0 on success, -1 on error
 */
int cllm_stop_thread(HierarchicalThread* thread);

/**
 * @brief Get thread state
 * 
 * @param thread Thread to query
 * @return Thread state
 */
StateType cllm_get_thread_state(HierarchicalThread* thread);

// ============================================================================
// VALIDATION
// ============================================================================

/**
 * @brief Validate 88D threading configuration
 * 
 * @param model CLLM model
 * @return true if configuration is valid
 */
bool cllm_validate_threading(CLLMModel* model);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Print threading information
 * 
 * @param model CLLM model
 */
void cllm_print_threading_info(CLLMModel* model);

#ifdef __cplusplus
}
#endif

#endif /* AI_CLLM_THREADING_H */