
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define BUFSIZE 512

int main(int argc, const char **argv)
{
    if(argc < 1) {
        return 0;
    }

    const char *path = argv[0];

    FILE *file = fopen(path, "r");
    if(file == NULL) {
        puts("Failed to open file!\n");
        return -1;
    }

    uint8_t *buffer = malloc(BUFSIZE);
    if(buffer == NULL) {
        puts("Failed to allocate buffer!\n");
        return -1;
    }

    free(buffer);
    return 0;
}

