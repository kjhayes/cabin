
#include "command.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

static char **
split_pipes(
        char *line,
        int *len)
{
    int subcmd_count = 0;
    int subcmd_len = 2;
    char **subcmds = malloc(subcmd_len * sizeof(char *));
    if(subcmds == NULL) {
        return NULL;
    }

    char *subcmd = strtok(line, "|");
    while(subcmd != NULL) {
        subcmd_count++;
        if(subcmd_count > subcmd_len) {
            subcmd_len *= 2;
            subcmds = realloc(subcmds, sizeof(char*) * subcmd_len);
            if(subcmds == NULL) {
                return NULL;
            }
        }
        subcmds[subcmd_count - 1] = subcmd;

        subcmd = strtok(NULL, "|");
    }

    if(subcmd_len != subcmd_count) {
        subcmd_len = subcmd_count;
        subcmds = realloc(subcmds, sizeof(char *) * subcmd_len);
        if(subcmds == NULL) {
            return NULL;
        }
    }

    *len = subcmd_len;
    return subcmds;
}

static int
parse_subcmd(
        char *line,
        struct command *cmd)
{
    int res;

    int token_count = 0;
    int tokens_len = 4;
    char **tokens = malloc(tokens_len * sizeof(char*));
    if(tokens == NULL) {
        return -ENOMEM;
    }

    {
        char *tok = strtok(line, " \t");
        while(tok != NULL)
        {
            token_count++;
            if(token_count > tokens_len) {
                tokens_len *= 2;
                tokens = realloc(tokens, sizeof(char*) * tokens_len);
                if(tokens == NULL) {
                    return -ENOMEM;
                }
            }
            tokens[token_count-1] = tok;

            tok = strtok(NULL, " \t");
        }

        if(tokens_len != token_count+1) {
            tokens_len = token_count+1;
            tokens = realloc(tokens, sizeof(char *) * tokens_len);
            if(tokens == NULL) {
                return -ENOMEM;
            }
        }
        tokens[tokens_len-1] = NULL;
    }

    if(token_count < 1) {
        // No command given
        return 0;
    }

    char *command = tokens[0];
    char **argv = tokens;
    int argc = token_count;

    cmd->file_input_path = NULL;
    cmd->file_output_path = NULL;

    for(int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if(arg[0] == '>') {
            // file output
            int args_used;
            char *path;
            if(arg[1] == '\0' && (i+1 < argc)) {
                args_used = 2;
                path = argv[i+1];
            } else {
                args_used = 1;
                path = arg+1;
            }

            if(cmd->file_output_path != NULL) {
                // Ignore it
                continue;
            }

            cmd->file_output_path = path;

            int args_after = argc - (i + args_used) + 1;
            memmove(argv + i, argv + i + args_used, args_after);
            argc -= args_used;
        }
        else if(arg[0] == '<') {
            // file input
            int args_used;
            char *path;
            if(arg[1] == '\0' && (i+1 < argc)) {
                args_used = 2;
                path = argv[i+1];
            } else {
                args_used = 1;
                path = arg+1;
            }

            if(cmd->file_input_path != NULL) {
                // Ignore it
                continue;
            }

            cmd->file_input_path = path;

            int args_after = argc - (i + args_used) + 1;
            memmove(argv + i, argv + i + args_used, args_after);
            argc -= args_used;
        }
    }

    cmd->pid = -1;
    cmd->argc = argc;
    cmd->argv = argv;
    cmd->child = NULL;
}

struct command *
parse_command(
        char *line)
{
    int res;

    int subcmd_count;
    char **subcmds = split_pipes(line, &subcmd_count);
    if(subcmds == NULL) {
        return NULL;
    }

    struct command *root = NULL;
    {
    struct command *cur = NULL;
    for(int i = 0; i < subcmd_count; i++) {
        struct command *cmd = malloc(sizeof(struct command));
        if(cmd == NULL) {
            while(root) {
                struct command *old = root;
                root = root->child;
                free(old->argv);
                free(old);
            }
            free(subcmds);
            return NULL;
        }

        memset(cmd, 0, sizeof(struct command));
        res = parse_subcmd(subcmds[i], cmd);
        if(res) {
            free(cmd);
            while(root) {
                struct command *old = root;
                root = root->child;
                free(old->argv);
                free(old);
            }
            free(subcmds);
            return NULL;
        }

        if(cur != NULL) {
            cur->child = cmd;
        } else {
            root = cmd;
        }
        cur = cmd;
    }
    }

    free(subcmds);

    return root;
}

int
free_command(struct command *command)
{
    struct command *cur = command;
    while(cur) {
        struct command *old = cur;
        cur = cur->child;
        free(old->argv);
        free(old);
    }
    return 0;
}

