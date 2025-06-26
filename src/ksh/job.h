#ifndef __KSH_JOB_H__
#define __KSH_JOB_H__

#include <kanawha/process.h>

#include "list.h"

struct job
{
    pid_t pid;

    enum {
        JOB_STATE_UNLAUNCHED,
        JOB_STATE_RUNNING,
        JOB_STATE_EXITED,
    } state;

    int exitcode;

    char **argv;
    int argc;

    int in_fd;
    int out_fd;
    int err_fd;

    struct list_node group_node;
};

struct job_group
{
    struct list job_list;
    int in_fd;
    int out_fd;
    int err_fd;
};

struct job_state
{
    struct job_group *fg;
};

int
job_state_init(
        struct job_state *state);

int
job_state_deinit(
        struct job_state *state);

// <0 -> error
#define JOB_TICK_IGNORE   0
#define JOB_TICK_STEAL    1
int
job_state_tick(
        struct job_state *state,
        char c,
        int have_char
        );

int
job_state_run(
        struct job_state *state,
        char *line);

#endif
