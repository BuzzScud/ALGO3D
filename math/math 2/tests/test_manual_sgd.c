#include <stdio.h>

int main() {
    float params[3] = {1.0f, 2.0f, 3.0f};
    float gradients[3] = {0.1f, 0.2f, 0.3f};
    float lr = 0.1f;
    
    printf("Before: params[0] = %f\n", params[0]);
    
    // Manual SGD step
    for (int i = 0; i < 3; i++) {
        params[i] -= lr * gradients[i];
    }
    
    printf("After: params[0] = %f\n", params[0]);
    printf("Expected: 0.99\n");
    
    return 0;
}
