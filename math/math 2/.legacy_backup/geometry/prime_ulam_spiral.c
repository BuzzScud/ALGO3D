/**
 * @file prime_ulam_spiral.c
 * @brief Implementation of Ulam spiral generation and analysis
 */

#include "prime_ulam_spiral.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "prime_basic.h"
#include <string.h>

// Helper: Check if number is prime (using trial division)
static bool is_prime_simple(uint64_t num) {
    if (num <= 1) return false;
    if (num == 2 || num == 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    
    for (uint64_t i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) {
            return false;
        }
    }
    
    return true;
}

void prime_ulam_spiral_generate(uint64_t* grid, uint32_t size) {
    if (!grid || size == 0) {
        return;
    }
    
    // Ensure size is odd
    if (size % 2 == 0) {
        size--;
    }
    
    // Initialize grid to zero
    memset(grid, 0, size * size * sizeof(uint64_t));
    
    // Start at center
    uint32_t x = size / 2;
    uint32_t y = size / 2;
    grid[y * size + x] = 1;
    
    // Direction vectors: right(0), up(1), left(2), down(3)
    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, -1, 0, 1};
    int dir = 0;
    
    uint32_t step = 1;
    uint64_t num = 2;
    
    // Build spiral
    while (num <= (uint64_t)(size * size)) {
        // Move in current direction for 'step' steps (twice per step increment)
        for (int rep = 0; rep < 2; rep++) {
            for (uint32_t s = 0; s < step; s++) {
                x = (uint32_t)((int)x + dx[dir]);
                y = (uint32_t)((int)y + dy[dir]);
                
                if (x < size && y < size) {
                    grid[y * size + x] = num;
                    num++;
                }
                
                if (num > (uint64_t)(size * size)) {
                    return;
                }
            }
            dir = (dir + 1) % 4;
        }
        step++;
    }
}

bool prime_ulam_is_prime_at(const uint64_t* grid, uint32_t size,
                             uint32_t x, uint32_t y) {
    if (!grid || x >= size || y >= size) {
        return false;
    }
    
    uint64_t num = grid[y * size + x];
    return is_prime_simple(num);
}

uint64_t prime_ulam_get_value(const uint64_t* grid, uint32_t size,
                               uint32_t x, uint32_t y) {
    if (!grid || x >= size || y >= size) {
        return 0;
    }
    
    return grid[y * size + x];
}

double prime_ulam_density(const uint64_t* grid, uint32_t size,
                          uint32_t x, uint32_t y, uint32_t radius) {
    if (!grid || x >= size || y >= size || radius == 0) {
        return 0.0;
    }
    
    uint32_t prime_count = 0;
    uint32_t total_count = 0;
    
    // Scan circular region
    for (uint32_t dy = 0; dy <= radius * 2; dy++) {
        for (uint32_t dx = 0; dx <= radius * 2; dx++) {
            int px = (int)x - (int)radius + (int)dx;
            int py = (int)y - (int)radius + (int)dy;
            
            if (px < 0 || py < 0 || px >= (int)size || py >= (int)size) {
                continue;
            }
            
            // Check if within circular radius
            int dist_x = px - (int)x;
            int dist_y = py - (int)y;
            double dist = math_sqrt((double)(dist_x * dist_x + dist_y * dist_y));
            
            if (dist <= (double)radius) {
                total_count++;
                if (prime_ulam_is_prime_at(grid, size, (uint32_t)px, (uint32_t)py)) {
                    prime_count++;
                }
            }
        }
    }
    
    if (total_count == 0) {
        return 0.0;
    }
    
    return math_div((double)prime_count, (double)total_count);
}

uint32_t prime_ulam_find_diagonals(const uint64_t* grid, uint32_t size,
                                    double* diagonal_scores) {
    if (!grid || !diagonal_scores || size == 0) {
        return 0;
    }
    
    uint32_t significant_count = 0;
    double threshold = 0.3; // 30% prime density considered significant
    
    // Check main diagonals (top-left to bottom-right)
    for (uint32_t offset = 0; offset < size; offset++) {
        uint32_t prime_count = 0;
        uint32_t total_count = 0;
        
        // Diagonal starting from (offset, 0)
        for (uint32_t i = 0; i + offset < size && i < size; i++) {
            total_count++;
            if (prime_ulam_is_prime_at(grid, size, offset + i, i)) {
                prime_count++;
            }
        }
        
        double density = (total_count > 0) ? 
                         math_div((double)prime_count, (double)total_count) : 0.0;
        diagonal_scores[offset] = density;
        
        if (density > threshold) {
            significant_count++;
        }
    }
    
    // Check anti-diagonals (top-right to bottom-left)
    for (uint32_t offset = 0; offset < size; offset++) {
        uint32_t prime_count = 0;
        uint32_t total_count = 0;
        
        // Anti-diagonal starting from (size-1-offset, 0)
        for (uint32_t i = 0; i < size && (int)(size - 1 - offset) - (int)i >= 0; i++) {
            total_count++;
            if (prime_ulam_is_prime_at(grid, size, size - 1 - offset - i, i)) {
                prime_count++;
            }
        }
        
        double density = (total_count > 0) ?
                         math_div((double)prime_count, (double)total_count) : 0.0;
        diagonal_scores[size + offset] = density;
        
        if (density > threshold) {
            significant_count++;
        }
    }
    
    return significant_count;
}