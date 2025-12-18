#ifndef LOSS_FUNCTIONS_H
#define LOSS_FUNCTIONS_H

#include "math/abacus.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Cross-entropy loss with Crystalline Abacus (arbitrary precision)
 * 
 * MIGRATED: Now uses NEW math library (Crystalline Abacus)
 * RENAMED: Removed "_bigfixed" suffix - implementation detail should not be in name
 */
void cross_entropy_loss(
    CrystallineAbacus** predictions,
    uint32_t* targets,
    CrystallineAbacus* loss,
    int batch_size,
    int num_classes,
    int precision
);

/**
 * Softmax with Crystalline Abacus (arbitrary precision)
 * 
 * MIGRATED: Now uses NEW math library (Crystalline Abacus)
 * RENAMED: Removed "_bigfixed" suffix - implementation detail should not be in name
 */
void softmax(
    CrystallineAbacus** input,
    int size,
    int precision
);

#ifdef __cplusplus
}
#endif

#endif // LOSS_FUNCTIONS_H
