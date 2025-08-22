
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <kanawha/sys-wrappers.h>

int main(int argc, const char **argv)
{
    volatile unsigned long x = 1;
    volatile unsigned long y = 0;
    volatile unsigned long z = x / y;

    return 0;
}

