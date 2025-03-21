
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

static int
cd_handler(int argc, const char **argv)
{
    int res;
    if(argc != 2) {
        return -1;
    }

    const char *path = argv[1];
    res = chdir(path);
    if(res) {
        perror("chdir");
        return -1;
    }

    return 0;
}

static int
exit_handler(int argc, const char **argv)
{
    int return_code = 0;
    if(argc > 2) {
        fprintf(stderr, "Usage: exit {EXITCODE}\n");
        return -1;
    }

    if(argc > 1) {
        return_code = atoi(argv[1]);
    }

    exit(return_code);
}

static int
chroot_handler(int argc, const char **argv) {
    int res;
    if(argc != 2) {
        fprintf(stderr, "Usage: chroot [PATH]\n");
        return -1;
    }

    const char *path = argv[1];
    res = chroot(path);
    if(res) {
        perror("chroot");
        return -1;
    }

    return 0;
}

struct builtin_command {
    const char *name;
    int(*handler)(int argc, const char **argv);
};

static struct builtin_command builtin_commands[] = {
    {
        .name = "cd",
        .handler = cd_handler,
    },
    {
        .name = "exit",
        .handler = exit_handler,
    },
    {
        .name = "chroot",
        .handler = chroot_handler,
    },
    { .name = NULL, .handler = NULL, }
};

static int
get_line(
        char *buffer,
        size_t buflen)
{
    size_t head = 0;
    while(1) {
        char c = getchar();
        if(isgraph(c) || c == ' ') {
            putchar(c);
        }
        else if(c == '\t') {
            c = ' '; // Treat tabs as spaces on the command line
        }
        else if(c == '\n') {
            putchar(c);
            break;
        }
        else if(c == '\b') {
            if(head > 0) {
                head--;
                buffer[head] = ' ';
                putchar('\b'); // Move Back
                putchar(' '); // Put a Space
                putchar('\b'); // Move Back Again
            }
            continue;
        } else {
            continue;
        }

        if(head >= (buflen-1)) {
            return -ERANGE;
        }

        buffer[head] = c;
        head++;
    }

    buffer[head] = '\0';
    return 0;
}

static int
launch_command(const char *command, int argc, const char **argv)
{
    // Check for any builtin commands
    struct builtin_command *builtin_iter = &builtin_commands[0];
    while(builtin_iter->handler != NULL) {
        if(strcmp(builtin_iter->name, command) == 0) {
            // This is the command :)
            return builtin_iter->handler(argc, (const char **)argv);
        }
        builtin_iter++;
    }

    // This is an external command
    int pid = fork();
    if(pid == 0) {
        // Child thread
        execvp(command, (char* const*)argv);
        // We should not get here
        fprintf(stderr, "could not find command: %s\n", command);
        exit(EXIT_FAILURE);
    } else {
        // Parent Thread
        waitpid(pid, NULL, 0);
    }

    return 0;
}

int
main(int argc, const char **argv)
{

#define LINEBUFLEN 0x1000
static char line_buffer[LINEBUFLEN];

    const char *sh_text = "ksh";

    while(1) {
        printf("%s> ", sh_text);

        get_line(line_buffer, LINEBUFLEN);

        run_command(line_buffer);
    }
    return 0;
}

