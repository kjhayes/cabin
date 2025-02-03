
#include <stdio.h>
#include <stdint.h>

int main(int argc, const char **argv)
{
    printf("Touching invalid pointer\n");
    volatile uint64_t *touch = (void*)0xCAFEBABECAFEBABEULL;
    volatile uint64_t value = *touch;
    printf("Survived?????\n");
    return 0;
}
