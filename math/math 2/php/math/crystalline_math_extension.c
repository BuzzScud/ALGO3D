/*
  Crystalline Math PHP Extension
  
  Provides PHP bindings for the Crystalline Mathematics Library
  Focus: Arithmetic operations, prime numbers, mathematical functions
  
  IMPORTANT: This uses CUSTOM arbitrary precision math library
  NO standard math.h functions are used in the core library
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_crystalline_math.h"

// Include the crystalline math library headers
#include "../../math/include/math/types.h"
#include "../../math/include/math/arithmetic.h"
#include "../../math/include/math/prime.h"
#include "../../math/include/math/transcendental.h"
#include "../../math/include/math/validation.h"
#include "../../math/include/math/platonic_generator.h"
#include "../../math/include/math/polytope.h"
#include "../../math/include/math/rainbow.h"
#include "../../math/include/math/clock.h"

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(crystalline_math)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(crystalline_math)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(crystalline_math)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "crystalline_math support", "enabled");
    php_info_print_table_row(2, "Version", "1.0.0");
    php_info_print_table_end();
}
/* }}} */

/* {{{ proto double math_add(double a, double b) */
PHP_FUNCTION(math_add)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_add(a, b));
}
/* }}} */

/* {{{ proto double math_sub(double a, double b) */
PHP_FUNCTION(math_sub)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_sub(a, b));
}
/* }}} */

/* {{{ proto double math_mul(double a, double b) */
PHP_FUNCTION(math_mul)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_mul(a, b));
}
/* }}} */

/* {{{ proto double math_div(double a, double b) */
PHP_FUNCTION(math_div)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_div(a, b));
}
/* }}} */

/* {{{ proto double math_mod(double a, double b) */
PHP_FUNCTION(math_mod)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_mod(a, b));
}
/* }}} */

/* {{{ proto double math_abs(double x) */
PHP_FUNCTION(math_abs)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_abs(x));
}
/* }}} */

/* {{{ proto double math_min(double a, double b) */
PHP_FUNCTION(math_min)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_min(a, b));
}
/* }}} */

/* {{{ proto double math_max(double a, double b) */
PHP_FUNCTION(math_max)
{
    double a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_max(a, b));
}
/* }}} */

/* {{{ proto double math_clamp(double x, double min, double max) */
PHP_FUNCTION(math_clamp)
{
    double x, min_val, max_val;
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(x)
        Z_PARAM_DOUBLE(min_val)
        Z_PARAM_DOUBLE(max_val)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_clamp(x, min_val, max_val));
}
/* }}} */

/* {{{ proto int math_sign(double x) */
PHP_FUNCTION(math_sign)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(math_sign(x));
}
/* }}} */

/* {{{ proto double math_floor(double x) */
PHP_FUNCTION(math_floor)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_floor(x));
}
/* }}} */

/* {{{ proto double math_ceil(double x) */
PHP_FUNCTION(math_ceil)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_ceil(x));
}
/* }}} */

/* {{{ proto double math_round(double x) */
PHP_FUNCTION(math_round)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_round(x));
}
/* }}} */

/* {{{ proto double math_trunc(double x) */
PHP_FUNCTION(math_trunc)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_trunc(x));
}
/* }}} */

/* {{{ proto double math_sqrt(double x) */
PHP_FUNCTION(math_sqrt)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_sqrt(x));
}
/* }}} */

/* {{{ proto double math_pow(double base, double exp) */
PHP_FUNCTION(math_pow)
{
    double base, exp;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(base)
        Z_PARAM_DOUBLE(exp)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_pow(base, exp));
}
/* }}} */

/* {{{ proto double math_sin(double x) */
PHP_FUNCTION(math_sin)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_sin(x));
}
/* }}} */

/* {{{ proto double math_cos(double x) */
PHP_FUNCTION(math_cos)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_cos(x));
}
/* }}} */

/* {{{ proto double math_tan(double x) */
PHP_FUNCTION(math_tan)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_tan(x));
}
/* }}} */

/* {{{ proto double math_asin(double x) */
PHP_FUNCTION(math_asin)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_asin(x));
}
/* }}} */

/* {{{ proto double math_acos(double x) */
PHP_FUNCTION(math_acos)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_acos(x));
}
/* }}} */

/* {{{ proto double math_atan(double x) */
PHP_FUNCTION(math_atan)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_atan(x));
}
/* }}} */

/* {{{ proto double math_sinh(double x) */
PHP_FUNCTION(math_sinh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_sinh(x));
}
/* }}} */

/* {{{ proto double math_cosh(double x) */
PHP_FUNCTION(math_cosh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_cosh(x));
}
/* }}} */

/* {{{ proto double math_tanh(double x) */
PHP_FUNCTION(math_tanh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_tanh(x));
}
/* }}} */

/* {{{ proto double math_cbrt(double x) */
PHP_FUNCTION(math_cbrt)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_cbrt(x));
}
/* }}} */

/* {{{ proto double math_exp(double x) */
PHP_FUNCTION(math_exp)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_exp(x));
}
/* }}} */

/* {{{ proto double math_log(double x) */
PHP_FUNCTION(math_log)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_log(x));
}
/* }}} */

/* {{{ proto double math_log10(double x) */
PHP_FUNCTION(math_log10)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_log10(x));
}
/* }}} */

/* {{{ proto double math_log2(double x) */
PHP_FUNCTION(math_log2)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_log2(x));
}
/* }}} */

/* {{{ proto bool math_is_nan(double x) */
PHP_FUNCTION(math_is_nan)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_is_nan(x));
}
/* }}} */

/* {{{ proto bool math_is_inf(double x) */
PHP_FUNCTION(math_is_inf)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_is_inf(x));
}
/* }}} */

/* {{{ proto bool math_is_finite(double x) */
PHP_FUNCTION(math_is_finite)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_is_finite(x));
}
/* }}} */

/* {{{ proto bool math_approx_equal(double a, double b, double epsilon) */
PHP_FUNCTION(math_approx_equal)
{
    double a, b, epsilon;
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
        Z_PARAM_DOUBLE(epsilon)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(math_approx_equal(a, b, epsilon));
}
/* }}} */

/* {{{ proto bool is_prime(int n) */
PHP_FUNCTION(is_prime)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(prime_is_prime((uint64_t)n));
}

/* {{{ proto bool prime_is_prime(int n) */
PHP_FUNCTION(prime_is_prime)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(prime_is_prime((uint64_t)n));
}
/* }}} */

/* {{{ proto int crystalline_prime_generate_o1(int position, int magnitude) */
PHP_FUNCTION(crystalline_prime_generate_o1)
{
    zend_long position, magnitude;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(position)
        Z_PARAM_LONG(magnitude)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_generate_o1((uint32_t)position, (uint64_t)magnitude));
}
/* }}} */

/* {{{ proto int prime_nth(int n) */
PHP_FUNCTION(prime_nth)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_nth((uint64_t)n));
}
/* }}} */

/* {{{ proto int prime_next(int n) */
PHP_FUNCTION(prime_next)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_next((uint64_t)n));
}
/* }}} */

/* {{{ proto int prime_prev(int n) */
PHP_FUNCTION(prime_prev)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_prev((uint64_t)n));
}
/* }}} */

/* {{{ proto int prime_count_below(int n) */
PHP_FUNCTION(prime_count_below)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_count_below((uint64_t)n));
}
/* }}} */

/* {{{ proto int prime_count_range(int a, int b) */
PHP_FUNCTION(prime_count_range)
{
    zend_long a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(a)
        Z_PARAM_LONG(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_count_range((uint64_t)a, (uint64_t)b));
}
/* }}} */

/* {{{ proto int prime_gap_next(int prime) */
PHP_FUNCTION(prime_gap_next)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_gap_next((uint64_t)prime));
}
/* }}} */

/* {{{ proto int prime_gap_prev(int prime) */
PHP_FUNCTION(prime_gap_prev)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_LONG(prime_gap_prev((uint64_t)prime));
}
/* }}} */

/* {{{ proto bool prime_are_coprime(int a, int b) */
PHP_FUNCTION(prime_are_coprime)
{
    zend_long a, b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(a)
        Z_PARAM_LONG(b)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(prime_are_coprime((uint64_t)a, (uint64_t)b));
}
/* }}} */

/* {{{ proto int prime_totient(int n) */
PHP_FUNCTION(prime_totient)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (n < 0) {
        php_error_docref(NULL, E_WARNING, "Input must be non-negative");
        RETURN_FALSE;
    }
    
    RETURN_LONG(prime_totient((uint64_t)n));
}
/* }}} */

/* {{{ proto int prime_index(int prime) */
PHP_FUNCTION(prime_index)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (prime < 2) {
        php_error_docref(NULL, E_WARNING, "Input must be >= 2");
        RETURN_FALSE;
    }
    
    uint64_t index = prime_index((uint64_t)prime);
    if (index == 0) {
        php_error_docref(NULL, E_WARNING, "Input is not a prime number");
        RETURN_FALSE;
    }
    
    RETURN_LONG(index);
}
/* }}} */

/* {{{ proto bool prime_is_prime_o1(int position, int magnitude) */
PHP_FUNCTION(prime_is_prime_o1)
{
    zend_long position, magnitude;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(position)
        Z_PARAM_LONG(magnitude)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_BOOL(prime_is_prime_o1((uint32_t)position, (uint64_t)magnitude));
}
/* }}} */

/* {{{ proto double math_atan2(double y, double x) */
PHP_FUNCTION(math_atan2)
{
    double y, x;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(y)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_atan2(y, x));
}
/* }}} */

/* {{{ proto double math_expm1(double x) */
PHP_FUNCTION(math_expm1)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_expm1(x));
}
/* }}} */

/* {{{ proto double math_log1p(double x) */
PHP_FUNCTION(math_log1p)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_log1p(x));
}
/* }}} */

/* {{{ proto double math_asinh(double x) */
PHP_FUNCTION(math_asinh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_asinh(x));
}
/* }}} */

/* {{{ proto double math_acosh(double x) */
PHP_FUNCTION(math_acosh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_acosh(x));
}
/* }}} */

/* {{{ proto double math_atanh(double x) */
PHP_FUNCTION(math_atanh)
{
    double x;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(x)
    ZEND_PARSE_PARAMETERS_END();
    RETURN_DOUBLE(math_atanh(x));
}
/* }}} */

/* Helper to convert PlatonicSolid to PHP array */
static void platonic_to_php_array(PlatonicSolid* solid, zval *return_value) {
    if (!solid) {
        RETURN_EMPTY_ARRAY();
    }
    
    array_init(return_value);
    
    // Basic properties
    add_assoc_long(return_value, "dimension", solid->dimension);
    add_assoc_long(return_value, "num_vertices", solid->num_vertices);
    add_assoc_long(return_value, "num_edges", solid->num_edges);
    add_assoc_long(return_value, "num_faces", solid->num_faces);
    add_assoc_double(return_value, "edge_length", solid->edge_length);
    add_assoc_double(return_value, "circumradius", solid->circumradius);
    add_assoc_double(return_value, "inradius", solid->inradius);
    add_assoc_double(return_value, "volume", solid->volume);
    add_assoc_string(return_value, "symmetry_group", solid->symmetry_group);
    
    // Vertices array
    zval vertices;
    array_init(&vertices);
    for (uint64_t i = 0; i < solid->num_vertices; i++) {
        zval vertex;
        array_init(&vertex);
        for (uint32_t d = 0; d < solid->dimension; d++) {
            add_next_index_double(&vertex, solid->vertex_coords[i * solid->dimension + d]);
        }
        add_next_index_zval(&vertices, &vertex);
    }
    add_assoc_zval(return_value, "vertices", &vertices);
    
    // Edges array
    zval edges;
    array_init(&edges);
    for (uint64_t i = 0; i < solid->num_edges; i++) {
        zval edge;
        array_init(&edge);
        add_next_index_long(&edge, solid->edge_indices[i][0]);
        add_next_index_long(&edge, solid->edge_indices[i][1]);
        add_next_index_zval(&edges, &edge);
    }
    add_assoc_zval(return_value, "edges", &edges);
    
    // Faces array
    zval faces;
    array_init(&faces);
    for (uint64_t i = 0; i < solid->num_faces; i++) {
        zval face;
        array_init(&face);
        for (uint32_t j = 0; j < solid->face_sizes[i]; j++) {
            add_next_index_long(&face, solid->face_indices[i][j]);
        }
        add_next_index_zval(&faces, &face);
    }
    add_assoc_zval(return_value, "faces", &faces);
    
    platonic_free(solid);
}

/* {{{ proto array platonic_tetrahedron() */
PHP_FUNCTION(platonic_tetrahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_tetrahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_cube() */
PHP_FUNCTION(platonic_cube)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_cube();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_octahedron() */
PHP_FUNCTION(platonic_octahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_octahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_dodecahedron() */
PHP_FUNCTION(platonic_dodecahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_dodecahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_icosahedron() */
PHP_FUNCTION(platonic_icosahedron)
{
    ZEND_PARSE_PARAMETERS_NONE();
    PlatonicSolid* solid = platonic_generate_icosahedron();
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_simplex(int dimension) */
PHP_FUNCTION(platonic_simplex)
{
    zend_long dimension;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(dimension)
    ZEND_PARSE_PARAMETERS_END();
    
    PlatonicSolid* solid = platonic_generate_simplex((uint32_t)dimension);
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_hypercube(int dimension) */
PHP_FUNCTION(platonic_hypercube)
{
    zend_long dimension;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(dimension)
    ZEND_PARSE_PARAMETERS_END();
    
    PlatonicSolid* solid = platonic_generate_hypercube((uint32_t)dimension);
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* {{{ proto array platonic_cross_polytope(int dimension) */
PHP_FUNCTION(platonic_cross_polytope)
{
    zend_long dimension;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(dimension)
    ZEND_PARSE_PARAMETERS_END();
    
    PlatonicSolid* solid = platonic_generate_cross_polytope((uint32_t)dimension);
    platonic_to_php_array(solid, return_value);
}
/* }}} */

/* ============================================================================
 * RAINBOW TABLE FUNCTIONS
 * ============================================================================ */

/* Global rainbow table */
static RainbowTable g_php_rainbow_table = {0};
static bool g_php_rainbow_initialized = false;

/* {{{ proto bool rainbow_init(int capacity) */
PHP_FUNCTION(rainbow_init)
{
    zend_long capacity;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(capacity)
    ZEND_PARSE_PARAMETERS_END();
    
    if (g_php_rainbow_initialized) {
        rainbow_cleanup(&g_php_rainbow_table);
    }
    
    MathError err = rainbow_init(&g_php_rainbow_table, (size_t)capacity);
    g_php_rainbow_initialized = (err == MATH_SUCCESS);
    RETURN_BOOL(err == MATH_SUCCESS);
}
/* }}} */

/* {{{ proto void rainbow_cleanup() */
PHP_FUNCTION(rainbow_cleanup)
{
    if (g_php_rainbow_initialized) {
        rainbow_cleanup(&g_php_rainbow_table);
        g_php_rainbow_initialized = false;
    }
}
/* }}} */

/* {{{ proto bool rainbow_populate_count(int n) */
PHP_FUNCTION(rainbow_populate_count)
{
    zend_long n;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_FALSE;
    }
    
    MathError err = rainbow_populate_count(&g_php_rainbow_table, (uint64_t)n);
    RETURN_BOOL(err == MATH_SUCCESS);
}
/* }}} */

/* {{{ proto bool rainbow_populate_to_prime(int max_prime) */
PHP_FUNCTION(rainbow_populate_to_prime)
{
    zend_long max_prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(max_prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_FALSE;
    }
    
    MathError err = rainbow_populate_to_prime(&g_php_rainbow_table, (uint64_t)max_prime);
    RETURN_BOOL(err == MATH_SUCCESS);
}
/* }}} */

/* {{{ proto int rainbow_lookup_by_index(int index) */
PHP_FUNCTION(rainbow_lookup_by_index)
{
    zend_long index;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_FALSE;
    }
    
    uint64_t prime;
    MathError err = rainbow_lookup_by_index(&g_php_rainbow_table, (uint64_t)index, &prime);
    if (err != MATH_SUCCESS) {
        RETURN_FALSE;
    }
    RETURN_LONG(prime);
}
/* }}} */

/* {{{ proto int rainbow_lookup_index(int prime) */
PHP_FUNCTION(rainbow_lookup_index)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_FALSE;
    }
    
    uint64_t index;
    MathError err = rainbow_lookup_index(&g_php_rainbow_table, (uint64_t)prime, &index);
    if (err != MATH_SUCCESS) {
        RETURN_FALSE;
    }
    RETURN_LONG(index);
}
/* }}} */

/* {{{ proto int rainbow_next_prime(int prime) */
PHP_FUNCTION(rainbow_next_prime)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_FALSE;
    }
    
    uint64_t next;
    MathError err = rainbow_next_prime(&g_php_rainbow_table, (uint64_t)prime, &next);
    if (err != MATH_SUCCESS) {
        RETURN_FALSE;
    }
    RETURN_LONG(next);
}
/* }}} */

/* {{{ proto int rainbow_prev_prime(int prime) */
PHP_FUNCTION(rainbow_prev_prime)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_FALSE;
    }
    
    uint64_t prev;
    MathError err = rainbow_prev_prime(&g_php_rainbow_table, (uint64_t)prime, &prev);
    if (err != MATH_SUCCESS) {
        RETURN_FALSE;
    }
    RETURN_LONG(prev);
}
/* }}} */

/* {{{ proto bool rainbow_contains(int prime) */
PHP_FUNCTION(rainbow_contains)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_FALSE;
    }
    
    RETURN_BOOL(rainbow_contains(&g_php_rainbow_table, (uint64_t)prime));
}
/* }}} */

/* {{{ proto int rainbow_size() */
PHP_FUNCTION(rainbow_size)
{
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_LONG(0);
    }
    
    RETURN_LONG(g_php_rainbow_table.size);
}
/* }}} */

/* {{{ proto int rainbow_max_prime() */
PHP_FUNCTION(rainbow_max_prime)
{
    if (!g_php_rainbow_initialized) {
        php_error_docref(NULL, E_WARNING, "Rainbow table not initialized. Call rainbow_init() first.");
        RETURN_LONG(0);
    }
    
    RETURN_LONG(rainbow_max_prime(&g_php_rainbow_table));
}
/* }}} */

/* ============================================================================
 * CLOCK LATTICE FUNCTIONS
 * ============================================================================ */

/* Global clock context */
static ClockContext g_php_clock_ctx = {0};
static bool g_php_clock_initialized = false;

/* {{{ proto bool clock_init() */
PHP_FUNCTION(clock_init)
{
    if (g_php_clock_initialized) {
        clock_cleanup(&g_php_clock_ctx);
    }
    
    MathError err = clock_init(&g_php_clock_ctx);
    g_php_clock_initialized = (err == MATH_SUCCESS);
    RETURN_BOOL(err == MATH_SUCCESS);
}
/* }}} */

/* {{{ proto void clock_cleanup() */
PHP_FUNCTION(clock_cleanup)
{
    if (g_php_clock_initialized) {
        clock_cleanup(&g_php_clock_ctx);
        g_php_clock_initialized = false;
    }
}
/* }}} */

/* {{{ proto array clock_map_prime_to_position(int prime) */
PHP_FUNCTION(clock_map_prime_to_position)
{
    zend_long prime;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    ClockPosition pos;
    MathError err = clock_map_prime_to_position((uint64_t)prime, &pos);
    if (err != MATH_SUCCESS) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_long(return_value, "ring", pos.ring);
    add_assoc_long(return_value, "position", pos.position);
    add_assoc_double(return_value, "angle", pos.angle);
    add_assoc_double(return_value, "radius", pos.radius);
}
/* }}} */

/* {{{ proto int clock_position_to_prime(array position) */
PHP_FUNCTION(clock_position_to_prime)
{
    zval *position_arr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(position_arr)
    ZEND_PARSE_PARAMETERS_END();
    
    zval *ring_val = zend_hash_str_find(Z_ARRVAL_P(position_arr), "ring", sizeof("ring")-1);
    zval *pos_val = zend_hash_str_find(Z_ARRVAL_P(position_arr), "position", sizeof("position")-1);
    
    if (!ring_val || !pos_val) {
        php_error_docref(NULL, E_WARNING, "Position array must contain 'ring' and 'position' keys");
        RETURN_FALSE;
    }
    
    ClockPosition pos;
    pos.ring = (uint32_t)Z_LVAL_P(ring_val);
    pos.position = (uint32_t)Z_LVAL_P(pos_val);
    pos.angle = 0.0;
    pos.radius = 0.0;
    
    uint64_t prime = clock_position_to_prime(&pos);
    RETURN_LONG(prime);
}
/* }}} */

/* {{{ proto bool clock_is_valid_position(array position) */
PHP_FUNCTION(clock_is_valid_position)
{
    zval *position_arr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(position_arr)
    ZEND_PARSE_PARAMETERS_END();
    
    zval *ring_val = zend_hash_str_find(Z_ARRVAL_P(position_arr), "ring", sizeof("ring")-1);
    zval *pos_val = zend_hash_str_find(Z_ARRVAL_P(position_arr), "position", sizeof("position")-1);
    
    if (!ring_val || !pos_val) {
        RETURN_FALSE;
    }
    
    ClockPosition pos;
    pos.ring = (uint32_t)Z_LVAL_P(ring_val);
    pos.position = (uint32_t)Z_LVAL_P(pos_val);
    pos.angle = 0.0;
    pos.radius = 0.0;
    
    RETURN_BOOL(clock_is_valid_position(&pos));
}
/* }}} */

/* {{{ proto array clock_reverse_lookup(int number) */
PHP_FUNCTION(clock_reverse_lookup)
{
    zend_long number;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(number)
    ZEND_PARSE_PARAMETERS_END();
    
    uint32_t ring, position;
    uint64_t magnitude;
    MathError err = clock_reverse_lookup((uint64_t)number, &ring, &position, &magnitude);
    if (err != MATH_SUCCESS) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_long(return_value, "ring", ring);
    add_assoc_long(return_value, "position", position);
    add_assoc_long(return_value, "magnitude", magnitude);
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_binary, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_unary, 0)
    ZEND_ARG_INFO(0, x)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_math_clamp, 0)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, min)
    ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_prime, 0)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_prime_generate, 0)
    ZEND_ARG_INFO(0, position)
    ZEND_ARG_INFO(0, magnitude)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ crystalline_math_functions[] */
const zend_function_entry crystalline_math_functions[] = {
    PHP_FE(math_add, arginfo_math_binary)
    PHP_FE(math_sub, arginfo_math_binary)
    PHP_FE(math_mul, arginfo_math_binary)
    PHP_FE(math_div, arginfo_math_binary)
    PHP_FE(math_mod, arginfo_math_binary)
    PHP_FE(math_abs, arginfo_math_unary)
    PHP_FE(math_min, arginfo_math_binary)
    PHP_FE(math_max, arginfo_math_binary)
    PHP_FE(math_clamp, arginfo_math_clamp)
    PHP_FE(math_sign, arginfo_math_unary)
    PHP_FE(math_floor, arginfo_math_unary)
    PHP_FE(math_ceil, arginfo_math_unary)
    PHP_FE(math_round, arginfo_math_unary)
    PHP_FE(math_trunc, arginfo_math_unary)
    PHP_FE(math_sqrt, arginfo_math_unary)
    PHP_FE(math_cbrt, arginfo_math_unary)
    PHP_FE(math_pow, arginfo_math_binary)
    PHP_FE(math_exp, arginfo_math_unary)
    PHP_FE(math_log, arginfo_math_unary)
    PHP_FE(math_log10, arginfo_math_unary)
    PHP_FE(math_log2, arginfo_math_unary)
    PHP_FE(math_sin, arginfo_math_unary)
    PHP_FE(math_cos, arginfo_math_unary)
    PHP_FE(math_tan, arginfo_math_unary)
    PHP_FE(math_asin, arginfo_math_unary)
    PHP_FE(math_acos, arginfo_math_unary)
    PHP_FE(math_atan, arginfo_math_unary)
    PHP_FE(math_sinh, arginfo_math_unary)
    PHP_FE(math_cosh, arginfo_math_unary)
    PHP_FE(math_tanh, arginfo_math_unary)
    PHP_FE(math_is_nan, arginfo_math_unary)
    PHP_FE(math_is_inf, arginfo_math_unary)
    PHP_FE(math_is_finite, arginfo_math_unary)
    PHP_FE(math_approx_equal, arginfo_math_clamp)
    PHP_FE(is_prime, arginfo_is_prime)
    PHP_FE(prime_is_prime, arginfo_is_prime)
    PHP_FE(crystalline_prime_generate_o1, arginfo_prime_generate)
    PHP_FE(prime_nth, arginfo_is_prime)
    PHP_FE(prime_next, arginfo_is_prime)
    PHP_FE(prime_prev, arginfo_is_prime)
    PHP_FE(prime_count_below, arginfo_is_prime)
    PHP_FE(prime_count_range, arginfo_math_binary)
    PHP_FE(prime_gap_next, arginfo_is_prime)
    PHP_FE(prime_gap_prev, arginfo_is_prime)
    PHP_FE(prime_are_coprime, arginfo_math_binary)
    PHP_FE(prime_is_prime_o1, arginfo_prime_generate)
    PHP_FE(prime_totient, arginfo_math_unary)
    PHP_FE(prime_index, arginfo_math_unary)
    PHP_FE(math_atan2, arginfo_math_binary)
    PHP_FE(math_expm1, arginfo_math_unary)
    PHP_FE(math_log1p, arginfo_math_unary)
    PHP_FE(math_asinh, arginfo_math_unary)
    PHP_FE(math_acosh, arginfo_math_unary)
    PHP_FE(math_atanh, arginfo_math_unary)
    PHP_FE(platonic_tetrahedron, arginfo_void)
    PHP_FE(platonic_cube, arginfo_void)
    PHP_FE(platonic_octahedron, arginfo_void)
    PHP_FE(platonic_dodecahedron, arginfo_void)
    PHP_FE(platonic_icosahedron, arginfo_void)
    PHP_FE(platonic_simplex, arginfo_is_prime)
    PHP_FE(platonic_hypercube, arginfo_is_prime)
    PHP_FE(platonic_cross_polytope, arginfo_is_prime)
    
    /* Rainbow Table Functions */
    PHP_FE(rainbow_init, arginfo_math_unary)
    PHP_FE(rainbow_cleanup, arginfo_void)
    PHP_FE(rainbow_populate_count, arginfo_math_unary)
    PHP_FE(rainbow_populate_to_prime, arginfo_math_unary)
    PHP_FE(rainbow_lookup_by_index, arginfo_math_unary)
    PHP_FE(rainbow_lookup_index, arginfo_math_unary)
    PHP_FE(rainbow_next_prime, arginfo_math_unary)
    PHP_FE(rainbow_prev_prime, arginfo_math_unary)
    PHP_FE(rainbow_contains, arginfo_math_unary)
    PHP_FE(rainbow_size, arginfo_void)
    PHP_FE(rainbow_max_prime, arginfo_void)
    
    /* Clock Lattice Functions */
    PHP_FE(clock_init, arginfo_void)
    PHP_FE(clock_cleanup, arginfo_void)
    PHP_FE(clock_map_prime_to_position, arginfo_math_unary)
    PHP_FE(clock_position_to_prime, arginfo_math_unary)
    PHP_FE(clock_is_valid_position, arginfo_math_unary)
    PHP_FE(clock_reverse_lookup, arginfo_math_unary)
    
    PHP_FE_END
};
/* }}} */

/* {{{ crystalline_math_module_entry */
zend_module_entry crystalline_math_module_entry = {
    STANDARD_MODULE_HEADER,
    "crystalline_math",
    crystalline_math_functions,
    PHP_MINIT(crystalline_math),
    PHP_MSHUTDOWN(crystalline_math),
    NULL,
    NULL,
    PHP_MINFO(crystalline_math),
    "1.0.0",
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CRYSTALLINE_MATH
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(crystalline_math)
#endif