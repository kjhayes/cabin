
#include "kanawha/sys-wrappers.h"
#include "kanawha/uapi/process.h"
#include "kanawha/uapi/environ.h"
#include "command.h"
#include "thread.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

static int
open_executable(
        const char *exec_path,
        fd_t *fd)
{
    int res;

    size_t pathlen = strlen(exec_path);

#define ENV_PATHLEN 0x100
    char path_buf[ENV_PATHLEN + pathlen + 1];
    res = kanawha_sys_environ(
            "PATH",
            path_buf,
            ENV_PATHLEN,
            ENV_GET);
    size_t env_pathlen = 0;
    if(res == 0) {
        path_buf[ENV_PATHLEN] = '\0';
        env_pathlen = strlen(path_buf);
    }
#undef ENV_PATHLEN 

    strcpy(path_buf + env_pathlen, exec_path);

    res = kanawha_sys_open(
                path_buf,
                FILE_PERM_EXEC|FILE_PERM_READ,
                0,
                fd);
    if(res) {
        return res;
    }

    return 0;
}

// Setup the ARGV environment variable with the provided argc and argv,
// ignoring any NULL entries in "argv"
static int
setup_argv_env(
    struct simple_cmd *cmd)
{
    int res;

    // Determine the value of "argc" and how much data we will need to store ARGV
    // (Include the simple_cmd itself in ARGV)
    int argc = 1;
    size_t argv_data_len = strlen(cmd->command) + 1;

    for(struct cmd_arg *iter = cmd->args;
            iter != NULL;
            iter = iter->next)
    {
        if(iter->value == NULL) {
            // Need to evaluate the argument
            continue;
        }
        argv_data_len += (1 + strlen(iter->value));
        argc++;
    }


    char *argv_data = malloc(argv_data_len);
    if(argv_data == NULL) {
        return -ENOMEM;
    }

    char *argv_data_iter = argv_data;

    // Layout the "ARGV" environment variable
    {
        size_t cmd_len = strlen(cmd->command);
        memcpy(argv_data_iter, cmd->command, cmd_len);
        argv_data_iter[cmd_len] = ' ';
        argv_data_iter += (cmd_len + 1);
    }
    for(struct cmd_arg *iter = cmd->args;
            iter != NULL;
            iter = iter->next)
    {
        if(iter->value == NULL) {
            continue;
        }
        size_t arglen = strlen(iter->value);
        memcpy(argv_data_iter, iter->value, arglen);
        argv_data_iter[arglen] = ' ';
        argv_data_iter += (1 + arglen);
    }

    // Set ARGV
    res = kanawha_sys_environ("ARGV", argv_data, argv_data_len, ENV_SET);
    if(res) {
        free(argv_data);
        return res;
    }

    free(argv_data);
    return 0;
}

static int
exec_simple_cmd(struct simple_cmd *cmd) 
{
    int res;

    fd_t exec_file;

    /*
    printf("exec_simple_cmd(%s, stdin=%p, stdout=%p, stderr=%p)\n",
            cmd->command,
            (uintptr_t)cmd->stdin,
            (uintptr_t)cmd->stdout,
            (uintptr_t)cmd->stderr);
            */

    res = open_executable(
            cmd->command,
            &exec_file);
    if(res) {
        fprintf(stderr, "Could not find command \"%s\"!\n", cmd->command);
        goto err;
    }

    res = setup_argv_env(cmd);
    if(res) {
        fprintf(stderr, "Could not setup ARGV for command \"%s\"!\n", cmd->command);
        goto err;
    }

    if(cmd->stdin != 0) {
        kanawha_sys_close(0);
        res = kanawha_sys_fmove(0, cmd->stdin, FMOVE_DUP);
        if(res) {
            goto err;
        }
    }
    if(cmd->stdout != 1) {
        kanawha_sys_close(1);
        res = kanawha_sys_fmove(1, cmd->stdout, FMOVE_DUP);
        if(res) {
            goto err;
        }
    }
    if(cmd->stderr != 2) {
        kanawha_sys_close(2);
        res = kanawha_sys_fmove(2, cmd->stderr, FMOVE_DUP);
        if(res) {
            goto err;
        }
    }

    destroy_simple_cmd(cmd);
    cmd = NULL;

    res = kanawha_sys_exec(
            exec_file,
            0);
    if(res) {
        goto err;
    }

    // We should never reach here
    return -EINVAL;

err:
    if(cmd) {
        destroy_simple_cmd(cmd);
    }
    return res;
}

static int
exec_simple_cmd_thread_wrapper(void *arg)
{
    return exec_simple_cmd((struct simple_cmd*)arg);
}


// Consumes cmd even on failure
int
fork_simple_cmd(
        struct simple_cmd *cmd,
        pid_t *pid)
{
    int res = create_thread(
            exec_simple_cmd_thread_wrapper,
            (void*)cmd,
            pid);
    if(res) {
        // The thread failure to run, so
        // we need to free the simple_cmd
        destroy_simple_cmd(cmd);
    }
    return res;
}

struct simple_cmd *
parse_simple_cmd(const char *raw)
{
    size_t line_len = strlen(raw);
    char line_copy[line_len+1];
    memmove(line_copy, raw, line_len+1);

    {
        char string_delimiter = '\0';
        char *iter = line_copy;
        while(*iter)
        {
            if(*iter == string_delimiter) {
                string_delimiter = '\0';
            } else if (*iter == '\'') {
                string_delimiter = '\'';
            } else if (*iter == '"') {
                string_delimiter = '"';
            } else if(string_delimiter == '\0' 
                    && isspace(*iter)) {
                *iter = '\0';
            } 
            iter++;
        }
    };

    struct simple_cmd *cmd = malloc(sizeof(struct simple_cmd));
    if(cmd == NULL) {
        goto err;
    }
    memset(cmd, 0, sizeof(struct simple_cmd));

    cmd->stdin = 0;
    cmd->stdout = 1;
    cmd->stderr = 2;

    {
        cmd->args = NULL;
        struct cmd_arg *cur_prev = NULL;
        struct cmd_arg **arg_slot = &cmd->args;
        size_t argc = 0;
        for(size_t i = 0; i < line_len; i++) {
            char *pot_arg = line_copy + i;
            size_t arglen = strlen(pot_arg);
            if(arglen > 0) {
                char *arg_copy = malloc(arglen+1);
                if(arg_copy == NULL) {
                    goto err;
                }
                memcpy(arg_copy, pot_arg, arglen+1);
                if(argc == 0) {
                    cmd->command = arg_copy;
                } else {
                    struct cmd_arg *arg =
                        malloc(sizeof(struct cmd_arg));
                    if(arg == NULL) {
                        goto err;
                    }
                    memset(arg, 0, sizeof(struct cmd_arg));
                    arg->value = arg_copy;
                    arg->next = NULL;
                    arg->prev = cur_prev;

                    *arg_slot = arg;
                    arg_slot = &arg->next;
                    cur_prev = arg;
                }
                argc++;
            }
            i += arglen;
        }
    }

    return cmd;

err:
    if(cmd != NULL) {
        destroy_simple_cmd(cmd);
    }

    return NULL;
}

int
destroy_simple_cmd(struct simple_cmd *cmd)
{
    struct cmd_arg *prev_iter = NULL;
    for(struct cmd_arg *iter = cmd->args;
            iter != NULL;
            iter = iter->next)
    {
        if(prev_iter) {
            free(prev_iter);
        }

        if(iter->value) {
            free(iter->value);
        }

        prev_iter = iter;
    }
    if(prev_iter) {
        free(prev_iter);
    }

    if(cmd->command) {
        free(cmd->command);
    }

    free(cmd);

    return 0;
}

/*
 * Compound Commands
 */

void
dump_cmd(struct cmd *cmd) {
    switch(cmd->type) {
        case CMD_SIMPLE:
            printf("SIMPLE(%s)(in=%p,out=%p,err=%p)", cmd->primary->command,
                    (uintptr_t)cmd->primary->stdin,
                    (uintptr_t)cmd->primary->stdout,
                    (uintptr_t)cmd->primary->stderr);
            break;
        case CMD_SECONDARY_INPUT:
            printf("(");
            dump_cmd(cmd->secondary);
            printf(" | SIMPLE(%s)(in=%p,out=%p,err=%p))", cmd->primary->command,
                    (uintptr_t)cmd->primary->stdin,
                    (uintptr_t)cmd->primary->stdout,
                    (uintptr_t)cmd->primary->stderr);
            break;
        case CMD_SECONDARY_AND:
            printf("(");
            dump_cmd(cmd->secondary);
            printf(" && SIMPLE(%s)(in=%p,out=%p,err=%p))", cmd->primary->command,
                    (uintptr_t)cmd->primary->stdin,
                    (uintptr_t)cmd->primary->stdout,
                    (uintptr_t)cmd->primary->stderr);
            break;
        case CMD_SECONDARY_OR:
            printf("(");
            dump_cmd(cmd->secondary);
            printf(" || SIMPLE(%s)(in=%p,out=%p,err=%p))", cmd->primary->command,
                    (uintptr_t)cmd->primary->stdin,
                    (uintptr_t)cmd->primary->stdout,
                    (uintptr_t)cmd->primary->stderr);
            break;
        default:
            printf("ERROR");
            break;
    }
}

int
exec_cmd(struct cmd *cmd)
{
    int res;
    struct simple_cmd *simple;
    pid_t primary, secondary;
    int primary_exit, secondary_exit;

    switch(cmd->type) {
        case CMD_SIMPLE:
            simple = cmd->primary;
            cmd->primary = NULL;
            destroy_cmd(cmd);
            return exec_simple_cmd(simple);
            break;
        case CMD_SECONDARY_INPUT:
            res = fork_cmd(cmd->secondary, &secondary);
            cmd->secondary = NULL;
            if(res) {
                destroy_cmd(cmd);
                return res;
            }
            res = fork_simple_cmd(cmd->primary, &primary);
            cmd->primary = NULL;
            if(res) {
                destroy_cmd(cmd);
                return res;
            }
            while(kanawha_sys_reap(primary, 0, &primary_exit)) {}
            while(kanawha_sys_reap(secondary, 0, &secondary_exit)) {}
            kanawha_sys_exit(primary_exit);
            break;
        default:
            destroy_cmd(cmd);
            return -EINVAL;
    }
}

static int
exec_cmd_thread_wrapper(void *arg)
{
    return exec_cmd((struct cmd*)arg);
}


// Consumes cmd even on failure
int
fork_cmd(
        struct cmd *cmd,
        pid_t *pid)
{
    int res = create_thread(
            exec_cmd_thread_wrapper,
            (void*)cmd,
            pid);
    if(res) {
        // The thread failure to run, so
        // we need to free the simple_cmd
        destroy_cmd(cmd);
    }
    return res;
}

struct cmd *
parse_cmd(struct simple_cmd *simple)
{
    struct cmd *cmd = malloc(sizeof(struct cmd));
    if(cmd == NULL) {
        destroy_simple_cmd(simple);
        return NULL;
    }
    memset(cmd, 0, sizeof(struct cmd));

    // Get the last element of the argument list
    struct cmd_arg *last_arg = simple->args;
    while(last_arg && last_arg->next != NULL) {
        last_arg = last_arg->next;
    }

    struct cmd_arg *iter = last_arg;
    while(iter) {
        if(strcmp(iter->value, "|") == 0) {

            fd_t pipe_fd;
            int res = kanawha_sys_pipe(0, &pipe_fd);
            if(res) {
                destroy_simple_cmd(simple);
                free(cmd);
                return NULL;
            }

            if(iter->prev) {
                iter->prev->next = NULL;
            } else {
                simple->args = NULL;
            }
            iter->prev = NULL;

            fd_t primary_stdout = simple->stdout;
            fd_t primary_stderr = simple->stderr;

            simple->stdout = pipe_fd;
            simple->stderr = pipe_fd;
            cmd->secondary = parse_cmd(simple);

            struct cmd_arg *primary_args = iter->next;
            free(iter->value);
            free(iter);

            if(primary_args == NULL) {
                fprintf(stderr, "Missing command after \"|\"!\n");
                destroy_cmd(cmd->secondary);
                free(cmd);
                return NULL;
            }

            struct simple_cmd *primary = malloc(sizeof(struct simple_cmd));
            if(primary == NULL) {
                destroy_cmd(cmd->secondary);
                free(cmd);
                return NULL;
            }
            memset(primary, 0, sizeof(struct simple_cmd));

            primary->stdin = pipe_fd;
            primary->stdout = primary_stdout;
            primary->stderr = primary_stderr;

            primary->command = primary_args->value;
            primary->args = primary_args->next;
            free(primary_args);

            cmd->type = CMD_SECONDARY_INPUT;
            cmd->primary = primary;
            return cmd;
        }
        iter = iter->prev;
    }


    cmd->type = CMD_SIMPLE;
    cmd->primary = simple;
    return cmd;
}

int
destroy_cmd(struct cmd *cmd)
{
    if(cmd->primary) {
        destroy_simple_cmd(cmd->primary);
    }
    if(cmd->secondary) {
        destroy_cmd(cmd->secondary);
    }

    free(cmd);

    return 0;
}

