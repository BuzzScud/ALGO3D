#include <stdio.h>
#include <stdint.h>
#include "math/clock.h"
#include "math/types.h"

int main() {
    ClockPosition pos;
    MathError err = clock_map_prime_to_position(17, &pos);
    
    printf("Error: %d\n", err);
    printf("Ring: %u\n", pos.ring);
    printf("Position: %u\n", pos.position);
    printf("Angle: %f\n", pos.angle);
    printf("Radius: %f\n", pos.radius);
    
    return 0;
}
