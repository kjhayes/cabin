
#include "kanawha/sys-wrappers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

extern int
do_command(const char *command);

int
main(int argc, const char **argv)
{
    int running = 1;

#define BUFLEN 0x4000
    char *command_buffer = malloc(BUFLEN);
    if(command_buffer == NULL) {
        fputs("Could not allocate command buffer!\n", stderr);
        return -1;
    }

    int interactive;

    char *script_buffer = NULL;
    size_t script_size = 0;
    size_t script_index = 0;

    if(argc > 1) {
        interactive = 0;

        const char *script = argv[1];

        FILE *script_file = fopen(script, "r");
        if(script_file == NULL) {
            printf("Could not open script \"%s\"!\n", script);
            return -1;
        }

        fseek(script_file, 0, SEEK_END);
        script_size = ftell(script_file);
        fseek(script_file, 0, SEEK_SET);

        script_buffer = malloc(script_size);
        if(script_buffer == NULL) {
            return -1;
        }

        size_t to_read = script_size;
        char *buf_iter = script_buffer;
        while(to_read) {
            ssize_t read = fread(buf_iter, 1, to_read, script_file);
            if(read <= 0) {
                fprintf(stderr, "Failed to read script \"%s\"!\n", argv[1]);
                return -1;
            }
            buf_iter += read;
            to_read -= read;
        }

        fclose(script_file);

    } else {
        interactive = 1;
    }

    while(running)
    {
        if(interactive) {
            puts("sh> ");
        }

        int prev_was_whitespace = 1;
        size_t input_end = 0;
        
        do {
            int i;
            if(interactive) {
                i = getchar();
            } else {
                if(script_index >= script_size) {
                    running = 0;
                    break;
                }
                i = script_buffer[script_index];
                script_index++;
            }

            if(i == EOF) {
                running = 0;
                break;
            }
            char c = i;

            if(c == '\n' || c == '\r') {
                if(interactive) {
                    puts("\n");
                }
                break;
            }

            // Backspace-Like Characters
            switch(c) {
              case '\b':
              case 127:
                if(input_end > 0) {
                    input_end--;
                    if(interactive) {
                        putchar('\b');
                        putchar(' ');
                        putchar('\b');
                    }
                }
                continue;
            }

            if(!isprint(c)) {
                continue;
            }

            if(interactive) {
                putchar(c);
            }

            if(input_end < BUFLEN-1) {
                command_buffer[input_end] = c;
                input_end++;
            }
        } while(1);

        command_buffer[input_end] = '\0';

        int res = do_command(command_buffer);
        if(res) {
            free(command_buffer);
            if(script_buffer) {
                free(script_buffer);
            }
            return res;
        }
    }

    if(script_buffer) {
        free(script_buffer);
    }
    free(command_buffer);
    return 0;
}

