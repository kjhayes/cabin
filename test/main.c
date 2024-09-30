
#include <stdio.h>

int
main(int argc, const char **argv)
{
    if(argc <= 1) {
        fprintf(stderr, "Usage: test [OUTPUT-FILE] [ARGS...]\n");
        return -1;
    }

    const char *path = argv[1];
    FILE *target = fopen(path, "w");
    if(target == NULL) {
        fprintf(stderr, "Could not open or create file \"%s\"!\n", path);
        return -1;
    }

    for(int i = 2; i < argc; i++) {
        fprintf(target, "%s\n", argv[i]);
    }

    return 0;
}

