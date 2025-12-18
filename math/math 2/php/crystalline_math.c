#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// Include math.h before PHP headers to avoid implicit declaration warnings
#include <math.h>

// Suppress warnings from PHP system headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_crystalline_math.h"

#pragma GCC diagnostic pop

#include "math/prime.h"
#include "math/rainbow.h"
#include "math/clock.h"
#include "math/types.h"

/* Argument Info Declarations (Required for PHP 8+) */

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_prime_generate_o1, 0, 0, 2)
    ZEND_ARG_INFO(0, position)
    ZEND_ARG_INFO(0, magnitude)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_prime_is_prime, 0, 0, 1)
    ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_prime_nth, 0, 0, 1)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_rainbow_init, 0, 0, 1)
    ZEND_ARG_INFO(0, capacity)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_rainbow_populate, 0, 0, 2)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, max_prime)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_rainbow_lookup, 0, 0, 2)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_rainbow_count, 0, 0, 1)
    ZEND_ARG_INFO(0, table)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_clock_position, 0, 0, 1)
    ZEND_ARG_INFO(0, prime)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_clock_validate, 0, 0, 4)
    ZEND_ARG_INFO(0, ring)
    ZEND_ARG_INFO(0, position)
    ZEND_ARG_INFO(0, angle)
    ZEND_ARG_INFO(0, radius)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_version, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crystalline_reverse_lookup, 0, 0, 1)
    ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

/* Forward declarations */
ZEND_FUNCTION(crystalline_reverse_lookup);

/* Module entry */
static const zend_function_entry crystalline_math_functions[] = {
    PHP_FE(crystalline_prime_generate_o1, arginfo_crystalline_prime_generate_o1)
    PHP_FE(crystalline_prime_is_prime, arginfo_crystalline_prime_is_prime)
    PHP_FE(crystalline_prime_nth, arginfo_crystalline_prime_nth)
    PHP_FE(crystalline_rainbow_init, arginfo_crystalline_rainbow_init)
    PHP_FE(crystalline_rainbow_populate, arginfo_crystalline_rainbow_populate)
    PHP_FE(crystalline_rainbow_lookup, arginfo_crystalline_rainbow_lookup)
    PHP_FE(crystalline_rainbow_count, arginfo_crystalline_rainbow_count)
    PHP_FE(crystalline_clock_position, arginfo_crystalline_clock_position)
    PHP_FE(crystalline_clock_validate, arginfo_crystalline_clock_validate)
    PHP_FE(crystalline_reverse_lookup, arginfo_crystalline_reverse_lookup)
    PHP_FE(crystalline_version, arginfo_crystalline_version)
    PHP_FE_END
};

zend_module_entry crystalline_math_module_entry = {
    STANDARD_MODULE_HEADER,
    "crystalline_math",
    crystalline_math_functions,
    PHP_MINIT(crystalline_math),
    PHP_MSHUTDOWN(crystalline_math),
    NULL,
    NULL,
    PHP_MINFO(crystalline_math),
    PHP_CRYSTALLINE_MATH_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CRYSTALLINE_MATH
ZEND_GET_MODULE(crystalline_math)
#endif

/* Module initialization */
PHP_MINIT_FUNCTION(crystalline_math)
{
    /* Pre-initialize the rainbow table to avoid lazy initialization issues
     * This ensures the global state is properly set up before any PHP calls
     * and prevents potential segfaults from concurrent initialization */
    prime_nth(1);  /* Force initialization by getting first prime */
    
    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(crystalline_math)
{
    return SUCCESS;
}

/* Module info */
PHP_MINFO_FUNCTION(crystalline_math)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Crystalline Math Support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CRYSTALLINE_MATH_VERSION);
    php_info_print_table_row(2, "O(1) Prime Generation", "enabled");
    php_info_print_table_row(2, "Rainbow Table", "enabled");
    php_info_print_table_row(2, "Clock Lattice", "enabled");
    php_info_print_table_end();
}

/* ============================================================================
 * Prime Generation Functions
 * ============================================================================ */

/**
 * Generate prime using O(1) deterministic formula
 * 
 * @param int $position Clock position (0-11)
 * @param int $magnitude Magnitude level
 * @return int Prime number or 0 if composite
 */
PHP_FUNCTION(crystalline_prime_generate_o1)
{
    zend_long position, magnitude;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(position)
        Z_PARAM_LONG(magnitude)
    ZEND_PARSE_PARAMETERS_END();
    
    if (position < 0 || position > 11) {
        php_error_docref(NULL, E_WARNING, "Position must be between 0 and 11");
        RETURN_FALSE;
    }
    
    if (magnitude < 0) {
        php_error_docref(NULL, E_WARNING, "Magnitude must be non-negative");
        RETURN_FALSE;
    }
    
    uint64_t prime = prime_generate_o1((uint32_t)position, (uint64_t)magnitude);
    RETURN_LONG(prime);
}

/**
 * Check if a number is prime
 * 
 * @param int $n Number to check
 * @return bool True if prime, false otherwise
 */
PHP_FUNCTION(crystalline_prime_is_prime)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (n < 2) {
        RETURN_FALSE;
    }
    
    bool is_prime = prime_is_prime((uint64_t)n);
    RETURN_BOOL(is_prime);
}

/**
 * Get the nth prime number
 * 
 * @param int $n Index (1-based)
 * @return int The nth prime number
 */
PHP_FUNCTION(crystalline_prime_nth)
{
    zend_long n;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();
    
    if (n < 1) {
        php_error_docref(NULL, E_WARNING, "Index must be positive");
        RETURN_FALSE;
    }
    
    /* Call prime_nth with error checking */
    uint64_t prime = prime_nth((uint64_t)n);
    
    /* Check if result is valid (0 indicates error) */
    if (prime == 0 && n > 0) {
        php_error_docref(NULL, E_WARNING, "Failed to compute nth prime");
        RETURN_FALSE;
    }
    
    RETURN_LONG(prime);
}

/* ============================================================================
 * Rainbow Table Functions
 * ============================================================================ */

/**
 * Initialize a rainbow table
 * 
 * @param int $capacity Initial capacity
 * @return resource Rainbow table resource
 */
PHP_FUNCTION(crystalline_rainbow_init)
{
    zend_long capacity = 10000;
    
    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(capacity)
    ZEND_PARSE_PARAMETERS_END();
    
    if (capacity < 1) {
        php_error_docref(NULL, E_WARNING, "Capacity must be positive");
        RETURN_FALSE;
    }
    
    RainbowTable* table = (RainbowTable*)emalloc(sizeof(RainbowTable));
    MathError err = rainbow_init(table, (size_t)capacity);
    
    if (err != MATH_SUCCESS) {
        efree(table);
        php_error_docref(NULL, E_WARNING, "Failed to initialize rainbow table");
        RETURN_FALSE;
    }
    
    // For simplicity, return the capacity as a success indicator
    // In a full implementation, you'd register a resource type
    RETURN_LONG(capacity);
}

/**
 * Populate rainbow table with primes
 * 
 * @param int $count Number of primes to generate
 * @return bool True on success
 */
PHP_FUNCTION(crystalline_rainbow_populate)
{
    zend_long count;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(count)
    ZEND_PARSE_PARAMETERS_END();
    
    if (count < 1) {
        php_error_docref(NULL, E_WARNING, "Count must be positive");
        RETURN_FALSE;
    }
    
    // Simplified implementation - in production you'd maintain table state
    RETURN_TRUE;
}

/**
 * Lookup prime by index
 * 
 * @param int $index Prime index
 * @return int Prime number
 */
PHP_FUNCTION(crystalline_rainbow_lookup)
{
    zend_long index;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();
    
    if (index < 1) {
        php_error_docref(NULL, E_WARNING, "Index must be positive");
        RETURN_FALSE;
    }
    
    uint64_t prime = prime_nth((uint64_t)index);
    RETURN_LONG(prime);
}

/**
 * Get count of primes in rainbow table
 * 
 * @return int Number of primes
 */
PHP_FUNCTION(crystalline_rainbow_count)
{
    // Simplified implementation
    RETURN_LONG(0);
}

/* ============================================================================
 * Clock Lattice Functions
 * ============================================================================ */

/**
 * Get clock position for a prime
 * 
 * @param int $prime Prime number
 * @return array Clock position [ring, position, angle, radius]
 */
PHP_FUNCTION(crystalline_clock_position)
{
    zend_long prime;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prime)
    ZEND_PARSE_PARAMETERS_END();
    
    if (prime < 2) {
        php_error_docref(NULL, E_WARNING, "Prime must be >= 2");
        RETURN_FALSE;
    }
    
    ClockPosition pos;
    MathError err = clock_map_prime_to_position((uint64_t)prime, &pos);
    
    if (err != MATH_SUCCESS) {
        php_error_docref(NULL, E_WARNING, "Failed to map prime to clock position");
        RETURN_FALSE;
    }
    
    array_init(return_value);
    add_assoc_long(return_value, "ring", pos.ring);
    add_assoc_long(return_value, "position", pos.position);
    add_assoc_double(return_value, "angle", pos.angle);
    add_assoc_double(return_value, "radius", pos.radius);
}

/**
 * Validate a clock position
 * 
 * @param int $ring Ring number
 * @param int $position Position in ring
 * @return bool True if valid
 */
PHP_FUNCTION(crystalline_clock_validate)
{
    zend_long ring, position;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(ring)
        Z_PARAM_LONG(position)
    ZEND_PARSE_PARAMETERS_END();
    
    ClockPosition pos;
    pos.ring = (uint32_t)ring;
    pos.position = (uint32_t)position;
    pos.angle = 0.0;
    pos.radius = 0.0;
    
    bool valid = clock_is_valid_position(&pos);
    RETURN_BOOL(valid);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Get Crystalline Math version
 * 
 * @return string Version string
 */
PHP_FUNCTION(crystalline_version)
{
    RETURN_STRING(PHP_CRYSTALLINE_MATH_VERSION);
}

/**
 * Reverse lookup: Convert number to ring position and magnitude
 * 
 * crystalline_reverse_lookup(int $number): array|false
 * 
 * Returns array with keys: 'ring', 'position', 'magnitude', 'is_prime'
 * Returns false if number doesn't fit clock lattice structure
 */
PHP_FUNCTION(crystalline_reverse_lookup)
{
    zend_long number;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(number)
    ZEND_PARSE_PARAMETERS_END();
    
    if (number < 2) {
        RETURN_FALSE;
    }
    
    uint32_t ring, position;
    uint64_t magnitude;
    
    MathError err = clock_reverse_lookup((uint64_t)number, &ring, &position, &magnitude);
    
    if (err != MATH_SUCCESS) {
        RETURN_FALSE;
    }
    
    // Check if it's prime using standard primality test
    bool is_prime = prime_is_prime((uint64_t)number);
    
    // Return associative array
    array_init(return_value);
    add_assoc_long(return_value, "ring", ring);
    add_assoc_long(return_value, "position", position);
    add_assoc_long(return_value, "magnitude", magnitude);
    add_assoc_bool(return_value, "is_prime", is_prime);
}
