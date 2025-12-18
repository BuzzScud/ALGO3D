#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int main() {
    printf("Type sizes:\n");
    printf("  sizeof(size_t) = %zu\n", sizeof(size_t));
    printf("  sizeof(uint32_t) = %zu\n", sizeof(uint32_t));
    
    // Test array casting
    size_t shape_size_t[] = {2, 3};
    uint32_t shape_uint32[] = {2, 3};
    
    printf("\nArray values:\n");
    printf("  shape_size_t[0] = %zu\n", shape_size_t[0]);
    printf("  shape_size_t[1] = %zu\n", shape_size_t[1]);
    printf("  shape_uint32[0] = %u\n", shape_uint32[0]);
    printf("  shape_uint32[1] = %u\n", shape_uint32[1]);
    
    // Cast size_t to uint32_t pointer
    uint32_t* casted = (uint32_t*)shape_size_t;
    printf("\nCasted values (size_t[] -> uint32_t*):\n");
    printf("  casted[0] = %u\n", casted[0]);
    printf("  casted[1] = %u\n", casted[1]);
    
    return 0;
}
