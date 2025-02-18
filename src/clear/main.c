
#include <stdio.h>

int main(int argc, const char **argv) {
    printf("%c[2J", (char)0x1b);
    return 0;
}

