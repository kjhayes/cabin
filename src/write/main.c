
#include <stdio.h>

int
main(int argc, const char **argv)
{
    if(argc <= 1) {
        fprintf(stderr, "Usage: write [OUTPUT-FILE]\n");
        return -1;
    }

    const char *path = argv[1];
    FILE *target = fopen(path, "w");
    if(target == NULL) {
        fprintf(stderr, "Could not open or create file \"%s\"!\n", path);
        return -1;
    }

    while(1) {
        int c = getchar();
        if(c == EOF) {
            break;
        }
        if(c == '$') {
            break;
        }
        fputc(c, target);
    }

    return 0;
}

