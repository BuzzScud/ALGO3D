// color_utils.c - Color utility functions
#include "app_common.h"
#include "prime_math.h"

SDL_Color get_vibrational_color(double value, double prime) {
    double freq = vibrational_transducer(value, prime);
    SDL_Color color;
    
    if (freq >= SYN_GOLD_FREQ) {
        color.r = 255; color.g = 215; color.b = 0;  // Gold
    } else if (freq >= SYN_GREEN_FREQ) {
        color.r = 0; color.g = 255; color.b = 0;    // Green
    } else if (freq >= SYN_BLUE_FREQ) {
        color.r = 0; color.g = 100; color.b = 255;  // Blue
    } else {
        double t = freq / SYN_BLUE_FREQ;
        color.r = (uint8_t)(255 * (1.0 - t));
        color.g = (uint8_t)(255 * t * math_sin(t * MATH_PHI));
        color.b = (uint8_t)(255 * t);
    }
    color.a = 255;
    return color;
}
