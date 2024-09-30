
#include "kanawha/sys-wrappers.h"
#include "kanawha/uapi/spawn.h"
#include "kanawha/uapi/process.h"
#include "kanawha/uapi/environ.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

static struct thread_entry_info {
    int(*func)(void *arg);
    void *arg;
} shared_thread_entry_info;

static int
thread_entry(void)
{
    struct thread_entry_info *info = &shared_thread_entry_info;
    return (*info->func)(info->arg);
}

static int
create_thread(
        int(*thread_f)(void *arg),
        void *arg,
        pid_t *pid)
{
    extern void _thread_start(void);

    // TODO: Lock This
    shared_thread_entry_info.func = thread_f;
    shared_thread_entry_info.arg = arg;

    int res;
    res = kanawha_sys_spawn(
            _thread_start,
            (void*)thread_entry,
            SPAWN_MMAP_SHARED|SPAWN_ENV_CLONE|SPAWN_FILES_CLONE,
            pid);

    if(res) {
        puts("kanawha_sys_spawn Failed!\n");
        kanawha_sys_exit(-res);
    }

    return res;
}


struct do_exec_args {
    const char *exec_path;
    int argc;
    const char **argv;
};

static int
do_exec(void *data)
{
    int res;

    struct do_exec_args *args = data;
    const char *exec_path = args->exec_path;
    int argc = args->argc;
    const char **argv = args->argv;

    fd_t exec_file;

    res = kanawha_sys_open(
                exec_path,
                FILE_PERM_EXEC|FILE_PERM_READ,
                0,
                &exec_file);
    if(res) {
        fprintf(stderr, "Could not find command \"%s\"!\n", exec_path);
        return res;
    }

    // Setup the ARGV environment variable
    size_t argv_data_len = 0;
    for(int i = 0; i < argc; i++) {
        argv_data_len += (1 + strlen(argv[i]));
    }
    char *argv_data = malloc(argv_data_len);
    if(argv_data == NULL) {
        return -ENOMEM;
    }
    char *argv_data_iter = argv_data;
    for(int i = 0; i < argc; i++) {
        size_t arglen = strlen(argv[i]);
        memmove(argv_data_iter, argv[i], arglen);
        argv_data_iter[arglen] = ' ';
        argv_data_iter += (1 + arglen);
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

static int
cmd_exit(
        const char *raw,
        int argc,
        const char **argv)
{
    printf("[Exiting...]\n");
    kanawha_sys_exit(-1);
}

static int
cmd_setstdin(
        const char *raw,
        int argc,
        const char **argv)
{
    if(argc != 2) {
        return -EINVAL;
    }
    
    const char *path = argv[1];

    fd_t file;

    int res;
    res = kanawha_sys_open(
            path,
            FILE_PERM_READ,
            0,
            &file);
    if(res) {
        return res;
    }

    if(file != 1) {
        res = kanawha_sys_fswap(file, 1);
        if(res) {
            kanawha_sys_close(file);
            return res;
        }
    }
    // TODO Check if the old stdin was open, and close it if needed

    return 0;
}

static int
cmd_setstdout(
        const char *raw,
        int argc,
        const char **argv)
{
    if(argc != 2) {
        return -EINVAL;
    }
    const char *path = argv[1];

    fd_t file;

    int res;
    res = kanawha_sys_open(
            path,
            FILE_PERM_WRITE,
            0,
            &file);
    if(res) {
        return res;
    }

    if(file != 2) {
        res = kanawha_sys_fswap(file, 2);
        if(res) {
            kanawha_sys_close(file);
            return res;
        }
    }
    // TODO Check if the old stdout was open, and close it if needed

    return 0;
}

static int
cmd_setstderr(
        const char *raw,
        int argc,
        const char **argv)
{
    if(argc != 2) {
        return -EINVAL;
    }
    const char *path = argv[1];

    fd_t file;

    int res;
    res = kanawha_sys_open(
            path,
            FILE_PERM_WRITE,
            0,
            &file);
    if(res) {
        return res;
    }

    if(file != 3) {
        res = kanawha_sys_fswap(file, 3);
        if(res) {
            kanawha_sys_close(file);
            return res;
        }
    }

    // TODO Check if the old stderr was open, and close it if needed

    return 0;
}

static int
cmd_exec(
        const char *raw,
        int argc,
        const char **argv)
{
    if(argc < 2) {
        return -EINVAL;
    }

    struct do_exec_args args = {
        .argv = &argv[1],
        .argc = argc-1,
        .exec_path = argv[1],
    };
    do_exec(&args);

    // We should never return
    return -1;
}

static struct cmd_handler {
    int(*handler)(const char *raw, int argc, const char **argv);
    const char *command;
} cmd_handlers[] = {
    {
        .handler = cmd_exit,
        .command = "exit",
    },
    {
        .handler = cmd_exec,
        .command = "exec",
    },
    {
        .handler = cmd_setstdin,
        .command = "setstdin",
    },
    {
        .handler = cmd_setstdout,
        .command = "setstdout",
    },
    {
        .handler = cmd_setstderr,
        .command = "setstderr",
    },

    // MUST GO LAST
    {
        .handler = NULL,
        .command = NULL,
    },
};

int 
do_command(const char *raw_line)
{
    size_t line_len = strlen(raw_line);
    char line_copy[line_len+1];
    memmove(line_copy, raw_line, line_len+1);

    {
        char *iter = line_copy;
        while(*iter) {
            if(isspace(*iter)) {
                *iter = '\0';
            }
            iter++;
        }
    };

    size_t argc = 0;
    {
        for(size_t i = 0; i < line_len; i++) {
            char *pot_arg = line_copy + i;
            size_t arglen = strlen(pot_arg);
            if(arglen > 0) {
                argc++;
            }
            i += arglen;
        }
    }

    if(argc == 0) {
        // No command provided
        return 0;
    }

    const char *argv[argc];
    memset(argv, 0, sizeof(argv));
    {
        int i = 0;
        char *iter = line_copy;
        while(i < argc) {
            size_t arglen = strlen(iter);
            if(arglen > 0) {
                argv[i] = iter;
                i++;
            }
            iter += (1+arglen);
        }
    }

    // argc and argv are set correctly

    const char *command_name = argv[0];

    {
        size_t i = 0;

        while(1) {
            struct cmd_handler *cmd = &cmd_handlers[i];
            if(cmd->command == NULL) {
                break;
            }

            if(strcmp(command_name, cmd->command) == 0) {
                // This is the command given
                return (*cmd->handler)(raw_line, argc, argv);
            }
            i++;
        }
    }

    pid_t pid;
    struct do_exec_args do_exec_args = {
        .exec_path = command_name,
        .argc = argc,
        .argv = argv,
    };

    int res = create_thread(do_exec, (void*)&do_exec_args, &pid);
    if(res) {
        puts("Failed to launch process!\n");
    }

    int exitcode;
    while(kanawha_sys_reap(pid, 0, &exitcode)) {}

    return 0;
}

