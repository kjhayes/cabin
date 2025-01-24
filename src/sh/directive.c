
#include "directive.h"
#include "command.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <kanawha/sys-wrappers.h>
#include <kanawha/uapi/environ.h>

static int
do_exit(struct simple_cmd *cmd)
{
    printf("[Exiting...]\n");
    kanawha_sys_exit(-1);
}

static int
do_setstdin(struct simple_cmd *cmd)
{
    const char *path = cmd->args->value;

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

    if(file != 0) {
        res = kanawha_sys_fswap(file, 0);
        if(res) {
            kanawha_sys_close(file);
            return res;
        }
    }
    // TODO Check if the old stdin was open, and close it if needed

    return 0;
}

static int
do_setstdout(struct simple_cmd *cmd)
{
    const char *path = cmd->args->value;

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

    if(file != 1) {
        res = kanawha_sys_fswap(file, 1);
        if(res) {
            kanawha_sys_close(file);
            return res;
        }
    }
    // TODO Check if the old stdout was open, and close it if needed

    return 0;
}

static int
do_setstderr(struct simple_cmd *cmd)
{
    const char *path = cmd->args->value;

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

    // TODO Check if the old stderr was open, and close it if needed

    return 0;
}

static int
do_chroot(struct simple_cmd *cmd)
{
    const char *path = cmd->args->value;

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

    res = kanawha_sys_chroot(file);
    if(res) {
        return res;
    }

    kanawha_sys_close(file);

    return 0;
}

static int
do_cd(struct simple_cmd *cmd)
{
    const char *path = cmd->args->value;

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

    res = kanawha_sys_chwdir(file);
    if(res) {
        return res;
    }

    kanawha_sys_close(file);

    return 0;
}

static int
do_exec(struct simple_cmd *simple)
{
    struct cmd_arg *first_arg = simple->args;
    if(first_arg == NULL || first_arg->value == NULL) {
        fprintf(stderr, "Usage: exec [COMMAND]\n");
        return -1;
    }

    if(simple->command) {
        free(simple->command);
    }

    simple->command = first_arg->value;
    simple->args = first_arg->next;
    simple->args->prev = NULL;
    free(first_arg);

    struct cmd *cmd = parse_cmd(simple);

    exec_cmd(cmd);

    // We should never return
    return -1;
}

static int
do_getenv(struct simple_cmd *cmd)
{
    int res;

    const char *key = cmd->args->value;

#define BUFLEN 0x1000
    char *buffer = malloc(BUFLEN);
    res = kanawha_sys_environ(
            key,
            buffer,
            BUFLEN,
            ENV_GET);
    if(res) {
        return res;
    }

    buffer[BUFLEN-1] = '\0';

    printf(buffer);
    printf("\n");

#undef BUFLEN
    return 0;
}

static int
do_setenv(struct simple_cmd *cmd)
{
    int res;

    const char *key = cmd->args->value;
    const char *value = cmd->args->next->value;

    res = kanawha_sys_environ(
            key,
            (char*)value,
            strlen(value),
            ENV_SET);
    if(res) {
        return res;
    }

    return 0;
}

static struct directive_handler {
    int(*handler)(struct simple_cmd *cmd);
    const char *directive;

} directive_handlers[] = {
    {
        .handler = do_exit,
        .directive = "exit",
    },
    {
        .handler = do_exec,
        .directive = "exec",
    },
    {
        .handler = do_setstdin,
        .directive = "setstdin",
    },
    {
        .handler = do_setstdout,
        .directive = "setstdout",
    },
    {
        .handler = do_setstderr,
        .directive = "setstderr",
    },
    {
        .handler = do_chroot,
        .directive = "chroot",
    },
    {
        .handler = do_cd,
        .directive = "cd",
    },
    {
        .handler = do_getenv,
        .directive = "getenv",
    },
    {
        .handler = do_setenv,
        .directive = "setenv",
    },

    // MUST GO LAST
    {
        .handler = NULL,
        .directive = NULL,
    },
};

int
is_directive(struct simple_cmd *cmd)
{
    if(cmd->command == NULL) {
        return 0;
    }

    size_t i = 0;
    while(1) {
        struct directive_handler *dir = &directive_handlers[i];
        if(dir->directive == NULL) {
            break;
        }

        if(strcmp(dir->directive, cmd->command) == 0) {
            // This is the command given
            return 1;
        }
        i++;
    }

    return 0;
}

int
run_directive(
        struct simple_cmd *cmd)
{
    if(cmd->command == NULL) {
        destroy_simple_cmd(cmd);
        return -EINVAL;
    }

    size_t i = 0;
    while(1) {
        struct directive_handler *dir = &directive_handlers[i];
        if(dir->directive == NULL) {
            break;
        }

        if(strcmp(dir->directive, cmd->command) == 0) {
            (*dir->handler)(cmd);
            destroy_simple_cmd(cmd);
            return 0;
        }
        i++;
    }

    destroy_simple_cmd(cmd);
    return -EINVAL;
}

