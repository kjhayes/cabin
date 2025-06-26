
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    char *dirname;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dirname>\n", argv[0]);
        return 1;
    }

    dirname = argv[1];

    dir = opendir(dirname);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    if (errno != 0) {
        perror("readdir");
        closedir(dir);
        return 1;
    }

    if (closedir(dir) == -1) {
        perror("closedir");
        return 1;
    }

    return 0;
}

