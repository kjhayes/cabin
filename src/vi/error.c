#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void errExit(char *err)
{
    printf("%s\n", err);
    exit(EXIT_FAILURE);
}

