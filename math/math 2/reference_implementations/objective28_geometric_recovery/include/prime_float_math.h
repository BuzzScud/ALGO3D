/*
 * Prime Float Math - Header
 * 
 * Float-precision mathematical functions WITHOUT external dependencies.
 * Provides replacements for standard math.h functions.
 * 
 * CRITICAL: Maintains mathematical independence!
 */

#ifndef PRIME_FLOAT_MATH_H
#define PRIME_FLOAT_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mathematical Constants
 */
#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

/*
 * Basic Mathematical Functions
 */

// Square root
float math_sqrt(float x);

// Exponential (e^x)
float math_exp(float x);

// Natural logarithm
float math_log(float x);

// Power (x^y)
float math_pow(float x, float y);

/*
 * Trigonometric Functions
 */

// Sine
float math_sin(float x);

// Cosine
float math_cos(float x);

// Tangent
float math_tan(float x);

/*
 * Hyperbolic Functions
 */

// Hyperbolic tangent
float math_tanh(float x);

/*
 * Utility Functions
 */

// Absolute value
float math_abs(float x);
float math_fmod(float x, float y);
double math_abs(double x);

// Double precision math functions (for compatibility)
double math_exp(double x);
double math_sqrt(double x);
double math_log(double x);
double math_cos(double x);
double math_sin(double x);
double math_tan(double x);
double math_tanh(double x);
double math_pow(double x, double y);
double math_atan(double x);
double prime_atan2(double y, double x);

// Float precision math functions
float math_exp(float x);
float math_sqrt(float x);
float math_log(float x);
float math_cos(float x);
float math_sin(float x);
float math_tan(float x);
float math_tanh(float x);
float math_pow(float x, float y);
float math_atan(float x);
float prime_atan2f(float y, float x);

// Additional math functions
double math_floor(double x);
double math_ceil(double x);
double math_round(double x);
double math_fmod(double x, double y);
double math_acos(double x);
double prime_log2(double x);

// Maximum of two values
float math_max(float x, float y);
double math_max(double x, double y);

// Minimum of two values
float math_min(float x, float y);
double math_min(double x, double y);

// Check if NaN
int math_is_nan(float x);
int math_is_nan(double x);

// Check if infinite
int math_is_inf(float x);
int math_is_inf(double x);

#ifdef __cplusplus
}
#endif

#endif /* PRIME_FLOAT_MATH_H */