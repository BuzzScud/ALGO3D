/*
 * Prime Float Math - Float-precision wrappers for transcendental functions
 * 
 * This file provides float-precision mathematical functions WITHOUT using
 * the standard math.h library. All implementations use Taylor series and
 * iterative methods.
 * 
 * CRITICAL: This maintains mathematical independence - NO external math dependencies!
 */

#include <stdint.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/*
 * Square Root: sqrt(x)
 * Uses Newton-Raphson method: x_{n+1} = (x_n + a/x_n) / 2
 */
float math_sqrt(float x) {
    if (x < 0.0f) return 0.0f;  // Return 0 for negative (or could return NaN)
    if (x == 0.0f) return 0.0f;
    if (x == 1.0f) return 1.0f;
    
    // Newton-Raphson iteration
    float guess = x / 2.0f;  // Initial guess
    
    for (int i = 0; i < 10; i++) {  // 10 iterations gives good precision
        float new_guess = (guess + x / guess) * 0.5f;
        if (new_guess == guess) break;  // Converged
        guess = new_guess;
    }
    
    return guess;
}

/*
 * Exponential: exp(x) = e^x
 * Uses Taylor series: e^x = 1 + x + x²/2! + x³/3! + ...
 */
float math_exp(float x) {
    if (x == 0.0f) return 1.0f;
    
    // Clamp to prevent overflow/underflow
    if (x > 88.0f) return 3.4e38f;  // Max float
    if (x < -88.0f) return 0.0f;
    
    // For large x, use exp(x) = exp(x/2)²
    if (x > 10.0f || x < -10.0f) {
        float half = math_exp(x * 0.5f);
        return half * half;
    }
    
    // Taylor series
    float result = 1.0f;
    float term = 1.0f;
    
    for (int n = 1; n < 20; n++) {
        term *= x / (float)n;
        result += term;
        if (term < 1e-7f && term > -1e-7f) break;  // Converged
    }
    
    return result;
}

/*
 * Natural Logarithm: log(x) = ln(x)
 * Uses series expansion for ln(1+x): ln(1+x) = x - x²/2 + x³/3 - x⁴/4 + ...
 */
float math_log(float x) {
    if (x <= 0.0f) return -1e10f;  // Return large negative for invalid input
    if (x == 1.0f) return 0.0f;
    
    // Reduce to range [0.5, 1.5] using ln(a*b) = ln(a) + ln(b)
    int exp_adjust = 0;
    while (x > 1.5f) {
        x *= 0.5f;
        exp_adjust++;
    }
    while (x < 0.5f) {
        x *= 2.0f;
        exp_adjust--;
    }
    
    // Now x is in [0.5, 1.5], compute ln(x) using ln(1+y) where y = x-1
    float y = x - 1.0f;
    float result = 0.0f;
    float term = y;
    
    for (int n = 1; n < 20; n++) {
        result += term / (float)n;
        term *= -y;
        if (term < 1e-7f && term > -1e-7f) break;
    }
    
    // Add back the adjustment: ln(x * 2^n) = ln(x) + n*ln(2)
    result += (float)exp_adjust * 0.693147180559945f;  // ln(2)
    
    return result;
}

/*
 * Power: pow(x, y) = x^y
 * Uses exp and log: x^y = exp(y * ln(x))
 */
float math_pow(float x, float y) {
    if (x == 0.0f) return 0.0f;
    if (y == 0.0f) return 1.0f;
    if (x == 1.0f) return 1.0f;
    if (y == 1.0f) return x;
    
    // Handle negative base with integer exponent
    if (x < 0.0f) {
        // For integer exponents, we can handle negative base
        int int_y = (int)y;
        if ((float)int_y == y) {
            // Integer exponent
            float result = 1.0f;
            float base = (int_y < 0) ? (1.0f / x) : x;
            int exp = (int_y < 0) ? -int_y : int_y;
            
            for (int i = 0; i < exp; i++) {
                result *= base;
            }
            
            return result;
        }
        return 0.0f;  // Can't handle non-integer exponent with negative base
    }
    
    // x^y = exp(y * ln(x))
    return math_exp(y * math_log(x));
}

/*
 * Sine: sin(x)
 * Uses Taylor series: sin(x) = x - x³/3! + x⁵/5! - x⁷/7! + ...
 */
float math_sin(float x) {
    // Reduce to range [-π, π]
    const float MATH_PI = 3.14159265358979323846f;
    const float MATH_TWO_PI = 2.0f * MATH_PI;
    
    while (x > MATH_PI) x -= MATH_TWO_PI;
    while (x < -MATH_PI) x += MATH_TWO_PI;
    
    // Taylor series
    float result = x;
    float term = x;
    float x_squared = x * x;
    
    for (int n = 1; n < 10; n++) {
        term *= -x_squared / ((float)(2*n) * (float)(2*n + 1));
        result += term;
        if (term < 1e-7f && term > -1e-7f) break;
    }
    
    return result;
}

/*
 * Cosine: cos(x)
 * Uses Taylor series: cos(x) = 1 - x²/2! + x⁴/4! - x⁶/6! + ...
 */
float math_cos(float x) {
    // Reduce to range [-π, π]
    const float MATH_PI = 3.14159265358979323846f;
    const float MATH_TWO_PI = 2.0f * MATH_PI;
    
    while (x > MATH_PI) x -= MATH_TWO_PI;
    while (x < -MATH_PI) x += MATH_TWO_PI;
    
    // Taylor series
    float result = 1.0f;
    float term = 1.0f;
    float x_squared = x * x;
    
    for (int n = 1; n < 10; n++) {
        term *= -x_squared / ((float)(2*n - 1) * (float)(2*n));
        result += term;
        if (term < 1e-7f && term > -1e-7f) break;
    }
    
    return result;
}

/*
 * Tangent: tan(x) = sin(x) / cos(x)
 */
float math_tan(float x) {
    float cos_x = math_cos(x);
    if (cos_x == 0.0f) return 1e10f;  // Return large value for undefined
    return math_sin(x) / cos_x;
}

/*
 * Hyperbolic Tangent: tanh(x) = (e^x - e^-x) / (e^x + e^-x)
 */
float math_tanh(float x) {
    if (x > 10.0f) return 1.0f;
    if (x < -10.0f) return -1.0f;
    
    float exp_x = math_exp(x);
    float exp_neg_x = math_exp(-x);
    
    return (exp_x - exp_neg_x) / (exp_x + exp_neg_x);
}

/*
 * Absolute Value: fabs(x)
 */
float math_abs(float x) {
    return (x < 0.0f) ? -x : x;
}

// Double precision math functions
#define HUGE_VAL 1.7976931348623157e+308

static inline double prime_fabs_custom(double x) { return (x < 0.0) ? -x : x; }

double math_exp(double x) {
    // Simple exponential approximation
    if (x > 700.0) return HUGE_VAL;
    if (x < -700.0) return 0.0;
    
    double result = 1.0;
    double term = 1.0;
    int i;
    
    for (i = 1; i < 20; i++) {
        term *= x / i;
        result += term;
        if (term < 1e-15) break;
    }
    
    return result;
}

double math_sqrt(double x) {
    if (x < 0.0) return 0.0;
    if (x == 0.0) return 0.0;
    
    double guess = x / 2.0;
    double prev;
    int i;
    
    for (i = 0; i < 10; i++) {
        prev = guess;
        guess = (guess + x / guess) / 2.0;
        if (prime_fabs_custom(guess - prev) < 1e-10) break;
    }
    
    return guess;
}

double math_log(double x) {
    if (x <= 0.0) return -HUGE_VAL;
    if (x == 1.0) return 0.0;
    
    // Natural logarithm approximation
    double result = 0.0;
    double term = (x - 1.0) / (x + 1.0);
    double term_sq = term * term;
    int i;
    
    for (i = 1; i < 20; i += 2) {
        result += term / i;
        term *= term_sq;
    }
    
    return 2.0 * result;
}

double math_cos(double x) {
    // Reduce angle to [-π, π]
    while (x > 3.141592653589793) x -= 2 * 3.141592653589793;
    while (x < -3.141592653589793) x += 2 * 3.141592653589793;
    
    double result = 1.0;
    double term = 1.0;
    double x_sq = x * x;
    int i;
    
    for (i = 1; i < 20; i += 2) {
        term *= -x_sq / (i * (i + 1));
        result += term;
    }
    
    return result;
}

double math_sin(double x) {
    // Reduce angle to [-π, π]
    while (x > 3.141592653589793) x -= 2 * 3.141592653589793;
    while (x < -3.141592653589793) x += 2 * 3.141592653589793;
    
    double result = x;
    double term = x;
    double x_sq = x * x;
    int i;
    
    for (i = 1; i < 20; i += 1) {
        term *= -x_sq / ((2 * i) * (2 * i + 1));
        result += term;
    }
    
    return result;
}

double math_tan(double x) {
    double cos_val = math_cos(x);
    if (prime_fabs_custom(cos_val) < 1e-10) return 0.0;
    return math_sin(x) / cos_val;
}

double math_tanh(double x) {
    if (x > 20.0) return 1.0;
    if (x < -20.0) return -1.0;
    
    double exp_x = math_exp(x);
    double exp_neg_x = math_exp(-x);
    return (exp_x - exp_neg_x) / (exp_x + exp_neg_x);
}

// Cache for powers of 3 (common in L_lattice formula)
static double pow3_cache[100] = {0};
static int pow3_cache_initialized = 0;

static void init_pow3_cache(void) {
    if (pow3_cache_initialized) return;
    
    // Pre-compute 3^x for x in [0.0, 10.0] with 0.1 precision
    for (int i = 0; i < 100; i++) {
        double exp = (double)i / 10.0;
        pow3_cache[i] = math_exp(exp * math_log(3.0));
    }
    
    pow3_cache_initialized = 1;
}

double math_pow(double x, double y) {
    // Fast path: y == 0
    if (y == 0.0) return 1.0;
    
    // Fast path: y == 1
    if (y == 1.0) return x;
    
    // Fast path: x == 0
    if (x == 0.0) {
        if (y <= 0.0) return 0.0;
        return 0.0;
    }
    
    // Fast path: x < 0 and y is not integer
    if (x < 0.0 && (int)y != y) return 0.0;
    
    // Fast path: Small positive integer exponents (common case)
    if (y == (int)y && y > 0.0 && y < 10.0) {
        int int_exp = (int)y;
        double result = 1.0;
        for (int i = 0; i < int_exp; i++) {
            result *= x;
        }
        return result;
    }
    
    // Fast path: Powers of 3 (very common in L_lattice)
    if (x == 3.0) {
        // Initialize cache on first use
        if (!pow3_cache_initialized) {
            init_pow3_cache();
        }
        
        // Check if exponent is in cache range [0.0, 10.0]
        if (y >= 0.0 && y <= 10.0) {
            int cache_idx = (int)(y * 10.0);
            if (cache_idx >= 0 && cache_idx < 100) {
                // Interpolate between cached values for better precision
                double frac = (y * 10.0) - cache_idx;
                if (frac < 0.01 || cache_idx == 99) {
                    return pow3_cache[cache_idx];
                } else {
                    // Linear interpolation
                    double v1 = pow3_cache[cache_idx];
                    double v2 = pow3_cache[cache_idx + 1];
                    return v1 + frac * (v2 - v1);
                }
            }
        }
    }
    
    // Fast path: Powers of 2 (common)
    if (x == 2.0 && y == (int)y && y > 0.0 && y < 64.0) {
        return (double)(1ULL << (int)y);
    }
    
    // Standard path: Use exp(y * log(x))
    return math_exp(y * math_log(x));
}

double math_atan(double x) {
    if (x > 1.0) return 1.570796326794897 - math_atan(1.0 / x);
    if (x < -1.0) return -1.570796326794897 - math_atan(1.0 / x);
    
    double result = 0.0;
    double term = x;
    double x_sq = x * x;
    int i;
    
    for (i = 1; i < 20; i += 2) {
        result += term / i;
        term *= -x_sq;
    }
    
    return result;
}

double prime_atan2(double y, double x) {
    if (x > 0.0) return math_atan(y / x);
    if (x < 0.0) {
        if (y >= 0.0) return math_atan(y / x) + 3.141592653589793;
        else return math_atan(y / x) - 3.141592653589793;
    }
    if (y > 0.0) return 1.570796326794897;
    if (y < 0.0) return -1.570796326794897;
    return 0.0;
}

// Float precision versions already defined above

/*
 * Maximum: fmax(x, y)
 */
float math_max(float x, float y) {
    return (x > y) ? x : y;
}

/*
 * Minimum: fmin(x, y)
 */
float math_min(float x, float y) {
    return (x < y) ? x : y;
}

/*
 * Check if NaN
 */
int math_is_nan(float x) {
    // NaN is the only value that doesn't equal itself
    return (x != x);
}

/*
 * Check if infinite
 */
int math_is_inf(float x) {
    // Check if absolute value is greater than max float
    return (x > 1e38f || x < -1e38f);
}

/* Additional double precision functions */

double math_max(double x, double y) {
    return (x > y) ? x : y;
}

double math_min(double x, double y) {
    return (x < y) ? x : y;
}

double math_floor(double x) {
    if (x >= 0.0) {
        return (double)(int64_t)x;
    } else {
        double int_part = (double)(int64_t)x;
        return (x == int_part) ? int_part : int_part - 1.0;
    }
}

double math_ceil(double x) {
    if (x <= 0.0) {
        return (double)(int64_t)x;
    } else {
        double int_part = (double)(int64_t)x;
        return (x == int_part) ? int_part : int_part + 1.0;
    }
}

double math_round(double x) {
    if (x >= 0.0) {
        return (double)(int64_t)(x + 0.5);
    } else {
        return (double)(int64_t)(x - 0.5);
    }
}

double math_fmod(double x, double y) {
    if (y == 0.0) return 0.0;
    return x - (int64_t)(x / y) * y;
}

// Float version of prime_fmod
float math_fmod(float x, float y) {
    if (y == 0.0f) return 0.0f;
    return x - (int32_t)(x / y) * y;
}

double math_acos(double x) {
    // Simple approximation - for production use proper series or lookup
    if (x < -1.0) x = -1.0;
    if (x > 1.0) x = 1.0;
    // acos(x) = atan2(sqrt(1-x*x), 1)
    return prime_atan2(math_sqrt(1.0 - x * x), 1.0);
}

double prime_log2(double x) {
    if (x <= 0.0) return -1e308;  // Return large negative for invalid input
    return math_log(x) / 0.6931471805599453;  // divide by ln(2)
}

// Float version of prime_atan2
float prime_atan2f(float y, float x) {
    return (float)prime_atan2((double)y, (double)x);
}

// Double precision versions
double math_abs(double x) {
    return (x < 0.0) ? -x : x;
}

int math_is_nan(double x) {
    return (x != x);
}

int math_is_inf(double x) {
    return (x == x + 1.0) || (x == x - 1.0);
}
