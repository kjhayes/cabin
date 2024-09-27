
#include "kanawha/sys-wrappers.h"
#include "kanawha/uapi/spawn.h"
#include "kanawha/uapi/environ.h"
#include "kanawha/uapi/errno.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *exec_path = "";
char *argv_data = "";

static int
create_thread(
        int(*thread_f)(void),
        pid_t *pid)
{
    extern void _thread_start(void);

    int res;

    res = kanawha_sys_spawn(
            _thread_start,
            (void*)thread_f,
            SPAWN_MMAP_SHARED|SPAWN_ENV_CLONE|SPAWN_FILES_NONE,
            pid);

    if(res) {
        puts("kanawha_sys_spawn Failed!\n");
        kanawha_sys_exit(-res);
    }

    return res;
}

static int
do_exec(void)
{
    int res;

    fd_t exec_file;

    res = kanawha_sys_open(
                exec_path,
                FILE_PERM_EXEC|FILE_PERM_READ,
                0,
                &exec_file);
    if(res) {
        return res;
    }

    res = kanawha_sys_environ("ARGV", argv_data, strlen(argv_data), ENV_SET);
    if(res) {
        return res;
    }

    res = kanawha_sys_exec(
            exec_file,
            0);
    if(res) {
        return res;
    }

    // We should never reach here
    return 1;
}

static void
do_command(void)
{
    if(strcmp(argv_data,"") == 0) {
        if(strcmp(exec_path,"exit") == 0) {
            kanawha_sys_exit(0);
        }
        if(strcmp(exec_path,"echo") == 0) {
            puts(argv_data);
        }
    } 

    pid_t pid;
    int res = create_thread(do_exec, &pid);
    if(res) {
        puts("Failed to launch process!\n");
    }

    int exitcode;
    while(kanawha_sys_reap(pid, 0, &exitcode)) {}
}

int
main(int argc, const char **argv)
{
    int running = 1;

    const size_t buffer_len = 0x2000;
    char input_buffer[buffer_len];

    while(running)
    {
        puts("> ");

        int prev_was_whitespace = 1;
        size_t input_end = 0;
        
        do {
            char c = getchar();

            if(c == '\n' || c == '\r') {
                puts("\n");
                break;
            }

            if(!isprint(c)) {
                continue;
            }

            char put_buf[2];
            put_buf[0] = c;
            put_buf[1] = '\0';
            puts(put_buf);

            switch(c) {
                case ' ':
                case '\t':
                    if(prev_was_whitespace) {
                        continue;
                    } else {
                        c = ' ';
                        prev_was_whitespace = 1;
                        break;
                    }
                default:
                    prev_was_whitespace = 0;
                    break;
            }
            if(input_end < buffer_len-1) {
                input_buffer[input_end] = c;
                input_end++;
            }
        } while(1);

        input_buffer[input_end] = '\0';

        exec_path = &input_buffer[0];
        argv_data = &input_buffer[0];

        while(1) {
            char val = *argv_data;
            if(val == '\0') {
                argv_data = "";
                break;
            }
            else if(val == ' ') {
                *argv_data = '\0';
                argv_data++;
                break;
            }
            argv_data++;
        }

        do_command();
    }
}

