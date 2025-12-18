/**
 * Base-60 (Sexagesimal) Number Converter
 * 
 * Demonstrates Babylonian base-60 arithmetic as described in Chapter 2.
 * Converts between decimal and base-60 representation.
 */

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_DIGITS 10

/**
 * Structure to represent a base-60 number
 */
typedef struct {
    int digits[MAX_DIGITS];  // Each digit is 0-59
    int num_digits;
    int precision;           // Number of fractional digits
} Base60Number;

/**
 * Convert decimal to base-60
 */
Base60Number decimal_to_base60(double decimal, int precision) {
    Base60Number result = {0};
    result.precision = precision;
    
    // Handle integer part
    long long integer_part = (long long)fabs(decimal);
    int digit_count = 0;
    
    // Extract integer digits (right to left)
    if (integer_part == 0) {
        result.digits[precision] = 0;
        digit_count = 1;
    } else {
        while (integer_part > 0 && digit_count < MAX_DIGITS - precision) {
            result.digits[precision + digit_count] = integer_part % 60;
            integer_part /= 60;
            digit_count++;
        }
    }
    
    // Handle fractional part
    double fractional_part = fabs(decimal) - floor(fabs(decimal));
    for (int i = precision - 1; i >= 0; i--) {
        fractional_part *= 60;
        result.digits[i] = (int)fractional_part;
        fractional_part -= floor(fractional_part);
    }
    
    result.num_digits = digit_count + precision;
    return result;
}

/**
 * Convert base-60 to decimal
 */
double base60_to_decimal(Base60Number b60) {
    double result = 0.0;
    
    // Integer part
    for (int i = b60.precision; i < b60.num_digits; i++) {
        int power = i - b60.precision;
        result += b60.digits[i] * pow(60, power);
    }
    
    // Fractional part
    for (int i = b60.precision - 1; i >= 0; i--) {
        int power = i - b60.precision;
        result += b60.digits[i] * pow(60, power);
    }
    
    return result;
}

/**
 * Print base-60 number in Babylonian notation
 */
void print_base60(Base60Number b60) {
    printf("Base-60: ");
    
    // Print integer part (left to right)
    int started = 0;
    for (int i = b60.num_digits - 1; i >= b60.precision; i--) {
        if (b60.digits[i] != 0 || started || i == b60.precision) {
            printf("%d", b60.digits[i]);
            started = 1;
            if (i > b60.precision) printf(",");
        }
    }
    
    // Print fractional part
    if (b60.precision > 0) {
        printf(";");
        for (int i = b60.precision - 1; i >= 0; i--) {
            printf("%d", b60.digits[i]);
            if (i > 0) printf(",");
        }
    }
    printf("\n");
}

/**
 * Add two base-60 numbers
 */
Base60Number add_base60(Base60Number a, Base60Number b) {
    Base60Number result = {0};
    result.precision = (a.precision > b.precision) ? a.precision : b.precision;
    result.num_digits = (a.num_digits > b.num_digits) ? a.num_digits : b.num_digits;
    
    int carry = 0;
    for (int i = 0; i < result.num_digits; i++) {
        int sum = a.digits[i] + b.digits[i] + carry;
        result.digits[i] = sum % 60;
        carry = sum / 60;
    }
    
    if (carry > 0 && result.num_digits < MAX_DIGITS) {
        result.digits[result.num_digits++] = carry;
    }
    
    return result;
}

/**
 * Multiply two base-60 numbers
 */
Base60Number multiply_base60(Base60Number a, Base60Number b) {
    Base60Number result = {0};
    result.precision = a.precision + b.precision;
    
    // School multiplication algorithm
    for (int i = 0; i < a.num_digits; i++) {
        int carry = 0;
        for (int j = 0; j < b.num_digits; j++) {
            int pos = i + j;
            if (pos >= MAX_DIGITS) break;
            
            int product = a.digits[i] * b.digits[j] + result.digits[pos] + carry;
            result.digits[pos] = product % 60;
            carry = product / 60;
        }
        
        // Handle remaining carry
        int pos = i + b.num_digits;
        while (carry > 0 && pos < MAX_DIGITS) {
            int sum = result.digits[pos] + carry;
            result.digits[pos] = sum % 60;
            carry = sum / 60;
            pos++;
        }
    }
    
    // Find actual number of digits
    result.num_digits = MAX_DIGITS;
    while (result.num_digits > result.precision && result.digits[result.num_digits - 1] == 0) {
        result.num_digits--;
    }
    if (result.num_digits == result.precision) {
        result.num_digits = result.precision + 1;
    }
    
    return result;
}

/**
 * Demonstrate Babylonian arithmetic
 */
void demonstrate_babylonian_arithmetic() {
    printf("=== Babylonian Base-60 Arithmetic Demo ===\n\n");
    
    // Example 1: Simple conversion
    printf("Example 1: Converting 3661 to base-60\n");
    printf("Decimal: 3661\n");
    Base60Number b1 = decimal_to_base60(3661, 0);
    print_base60(b1);
    printf("Verification: %.0f\n", base60_to_decimal(b1));
    printf("Explanation: 3661 = 1×60² + 1×60¹ + 1×60⁰ = 3600 + 60 + 1\n\n");
    
    // Example 2: Fractional number
    printf("Example 2: Converting 1.5 (1;30 in base-60)\n");
    printf("Decimal: 1.5\n");
    Base60Number b2 = decimal_to_base60(1.5, 2);
    print_base60(b2);
    printf("Verification: %.6f\n", base60_to_decimal(b2));
    printf("Explanation: 1.5 = 1 + 30/60 = 1;30 in base-60\n\n");
    
    // Example 3: Pi approximation
    printf("Example 3: Pi (π ≈ 3.141592653589793)\n");
    printf("Decimal: 3.141592653589793\n");
    Base60Number b3 = decimal_to_base60(M_PI, 4);
    print_base60(b3);
    printf("Verification: %.15f\n", base60_to_decimal(b3));
    printf("Explanation: π ≈ 3;8,29,44,0 in base-60\n\n");
    
    // Example 4: Addition
    printf("Example 4: Addition in base-60\n");
    Base60Number a = decimal_to_base60(1.5, 2);
    Base60Number b = decimal_to_base60(2.25, 2);
    printf("1.5 + 2.25 = 3.75\n");
    printf("a = "); print_base60(a);
    printf("b = "); print_base60(b);
    Base60Number sum = add_base60(a, b);
    printf("sum = "); print_base60(sum);
    printf("Verification: %.6f\n\n", base60_to_decimal(sum));
    
    // Example 5: Multiplication
    printf("Example 5: Multiplication in base-60\n");
    Base60Number c = decimal_to_base60(2.0, 2);
    Base60Number d = decimal_to_base60(3.0, 2);
    printf("2.0 × 3.0 = 6.0\n");
    printf("c = "); print_base60(c);
    printf("d = "); print_base60(d);
    Base60Number product = multiply_base60(c, d);
    printf("product = "); print_base60(product);
    printf("Verification: %.6f\n\n", base60_to_decimal(product));
    
    // Example 6: The number 60 itself
    printf("Example 6: The number 60 (1,0 in base-60)\n");
    printf("Decimal: 60\n");
    Base60Number b60 = decimal_to_base60(60, 0);
    print_base60(b60);
    printf("Verification: %.0f\n", base60_to_decimal(b60));
    printf("Explanation: 60 = 1×60¹ + 0×60⁰ = 1,0 in base-60\n\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Convert command-line argument
        double value = atof(argv[1]);
        int precision = (argc > 2) ? atoi(argv[2]) : 4;
        
        printf("Converting %.15f to base-60 (precision=%d)\n", value, precision);
        Base60Number result = decimal_to_base60(value, precision);
        print_base60(result);
        printf("Back to decimal: %.15f\n", base60_to_decimal(result));
    } else {
        // Run demonstration
        demonstrate_babylonian_arithmetic();
    }
    
    return 0;
}