/* Auto-generated correction table for deterministic prime generation */

#ifndef CLOCK_CORRECTION_TABLE_H
#define CLOCK_CORRECTION_TABLE_H

#include <stdint.h>

/* Ring 0 (mod 12) correction factors */
static const struct {
    uint64_t base;      /* First prime at this position */
    uint64_t delta;     /* Average spacing */
    double density;     /* Prime density */
} ring0_corrections[12] = {
    {2, 29, 0.033875},  /* Position 0 */
    {3, 0, 1.000000},  /* Position 1 */
    {5, 0, 1.000000},  /* Position 2 */
    {17, 27, 0.036860},  /* Position 3 */
    {0, 0, 0.0},  /* Position 4 (no primes) */
    {0, 0, 0.0},  /* Position 5 (no primes) */
    {7, 27, 0.037023},  /* Position 6 */
    {0, 0, 0.0},  /* Position 7 (no primes) */
    {0, 0, 0.0},  /* Position 8 (no primes) */
    {11, 27, 0.037189},  /* Position 9 */
    {0, 0, 0.0},  /* Position 10 (no primes) */
    {0, 0, 0.0},  /* Position 11 (no primes) */
};

#endif /* CLOCK_CORRECTION_TABLE_H */
