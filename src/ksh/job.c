
#include "job.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static inline int
launch_job(
        struct job *job)
{
    int res;

    //printf("Forking...\n");
    int pid = vfork();
    if(pid == 0) {
        // Child
        //printf("Forked! (Child)\n");

        if(job->in_fd != 0) {
            res = dup2(job->in_fd, 0);
            if(res < 0) {
                perror("dup2");
                exit(res);
            }
            if(job->in_fd > 2) {
                close(job->in_fd);
            }
            job->in_fd = 0;
        }
        if(job->out_fd != 1) {
            res = dup2(job->out_fd, 1);
            if(res < 0) {
                perror("dup2");
                exit(res);
            }
            if(job->out_fd > 2) {
                close(job->out_fd);
            }
            job->out_fd = 1;
        }
        if(job->err_fd != 2) {
            res = dup2(job->err_fd, 2);
            if(res < 0) {
                perror("dup2");
                exit(res);
            }
            if(job->err_fd > 2) {
                close(job->err_fd);
            }
            job->err_fd = 2;
        }

        res = execvp(job->argv[0], job->argv);
        if(res) {
            printf("Failed to execute command: \"%s\"\n", job->argv[0]);
        }
        // Should not reach here
        exit(res);
    } else {
        // Parent
        //printf("Forked! (Parent)\n");
        job->pid = pid;
        job->state = JOB_STATE_RUNNING;
        return 0;
    }
}

static struct job *
create_job(char *command)
{
    struct job *job = malloc(sizeof(struct job));
    if(job == NULL) {
        return NULL;
    }

    char **argv = NULL;
    int argc = 0;

    char *tok = strtok(command," \t\r\n");
    while(tok) {
        argc++;
        argv = realloc(argv, argc * sizeof(char*));
        if(argv == NULL) {
            free(job);
            return NULL;
        }
        argv[argc-1] = tok;
        tok = strtok(NULL," \t");
    }

    argv = realloc(argv, (argc+1) * sizeof(char*));
    if(argv == NULL) {
        free(job);
        return NULL;
    }
    argv[argc] = NULL;

    job->argc = argc;
    job->argv = argv;

    if(argc == 0) {
        free(argv);
        free(job);
        return NULL;
    }

    job->pid = -1;
    job->in_fd = -1;
    job->out_fd = -1;
    job->err_fd = -1;

    return job;
}

static int
destroy_job(struct job *job)
{
    // TODO (this killing isn't actually correct yet)
    if(job->state == JOB_STATE_RUNNING) {
        kill(job->pid, SIGKILL);
    }

    free(job->argv);
    if(job->in_fd > 0) {
        close(job->in_fd);
    }
    if(job->out_fd > 0) {
        close(job->out_fd);
    }
    if(job->err_fd > 0) {
        close(job->err_fd);
    }
    free(job);
}

static struct job_group *
create_job_group(void)
{
    struct job_group *grp = malloc(sizeof(struct job_group));
    if(grp == NULL) {
        return NULL;
    }

    list_init(&grp->job_list);

    return grp;
}

static int
destroy_job_group(
        struct job_group *grp)
{
    while(!list_empty(&grp->job_list)) {
        struct list_node *node = grp->job_list.node.next;
        struct job *job = container_of(node, struct job, group_node);
        
        list_remove(&grp->job_list, &job->group_node);

        destroy_job(job);
    }

    free(grp);

    return 0;
}

static inline int
job_group_add(
        struct job_group *grp,
        struct job *job)
{
    list_insert(&grp->job_list, &job->group_node);
    return 0;
}

int
job_state_init(
        struct job_state *state)
{
    state->fg = NULL;
    return 0;
}

int
job_state_deinit(
        struct job_state *state)
{
    return 0;
}

int
job_state_run(
        struct job_state *state,
        char *command)
{
    if(strspn(command, " \t\r\n") == strlen(command)) {
        // Empty string
        return 0;
    }

    // Determine how many sub-commands there are by counting the pipes
    size_t subcmd_count = 1;
    {
        char *iter = command;
        while(*iter) {
            if(*iter == '|') {
                subcmd_count++;
            }
            iter++;
        }
    }

    // Actually get each piped command's string
    char *piped_commands[subcmd_count];
    {
    size_t index = 0;
    char *tok = strtok(command,"|");
    while(tok && index < subcmd_count) {
        piped_commands[index] = tok;
        index++;
        tok = strtok(NULL,"|");
    }
    if(index != subcmd_count) {
        return -1;
    }
    }

    struct job *jobs[subcmd_count];
    for(size_t i = 0; i < subcmd_count; i++) {
        jobs[i] = create_job(piped_commands[i]);
        if(jobs[i] == NULL) {
            for(size_t u = 0; u < i; u++) {
                destroy_job(jobs[u]);
            }
            return -1;
        }
    }

    struct job *head_job = jobs[0];
    struct job *tail_job = jobs[subcmd_count-1];

    // Link stdout to stdin with pipes

    //printf("Linking Jobs With Pipes...\n");

    for(size_t i = 0; i < subcmd_count-1; i++) {
        struct job *out = jobs[i];
        struct job *in = jobs[i+1];
        if(out->out_fd >= 0 && in->in_fd >= 0) {
            // No need to create a pipe
            continue;
        }
        int pipe_des[2];
        if(pipe(pipe_des)) {
            perror("pipe");
            for(size_t i = 0; i < subcmd_count; i++) {
                destroy_job(jobs[i]);
            }
            return -1;
        }

        if(out->out_fd < 0) {
            out->out_fd = pipe_des[1];
        }
        if(in->in_fd < 0) {
            in->in_fd = pipe_des[0];
        }
    }

    // Set stderr
    for(size_t i = 0; i < subcmd_count; i++) {
        if(jobs[i]->err_fd < 0) {
            jobs[i]->err_fd = dup(2);
            if(jobs[i]->err_fd < 0) {
                perror("dup");
                for(size_t i = 0; i < subcmd_count; i++) {
                    destroy_job(jobs[i]);
                }
                return -1;
            }
        }
    }

    //printf("Creating Job Group...\n");
    
    struct job_group *grp = create_job_group();
    if(grp == NULL) {
        for(size_t i = 0; i < subcmd_count; i++) {
            destroy_job(jobs[i]);
        }
        return -1;
    }
    for(size_t i = 0; i < subcmd_count; i++) {
        job_group_add(grp, jobs[i]);
    }

    // Link group stdin to head process stdin
    if(head_job->in_fd < 0) {
        int filedes[2];
        if(pipe(filedes)) {
            perror("pipe");
            destroy_job_group(grp);
        }
        grp->in_fd = filedes[1];
        head_job->in_fd = filedes[0];
    }
    // Link group stdout to tail process stdout
    //if(tail_job->out_fd < 0) {
    //    int filedes[2];
    //    if(pipe(filedes)) {
    //        perror("pipe");
    //        destroy_job_group(grp);
    //    }
    //    grp->out_fd = filedes[0];
    //    tail_job->out_fd = filedes[1];
    //}
    // Link group stderr to tail process stderr
    //if(tail_job->err_fd < 0) {
    //    int filedes[2];
    //    if(pipe(filedes)) {
    //        perror("pipe");
    //        destroy_job_group(grp);
    //    }
    //    grp->err_fd = filedes[0];
    //    tail_job->err_fd = filedes[1];
    //}

    // For now, just link both stdout and stderr to the shell output
    if(head_job->out_fd < 0) {
        head_job->out_fd = dup(1);
        if(head_job->out_fd < 0) {
            perror("dup");
            destroy_job_group(grp);
        }
    }
    if(head_job->err_fd < 0) {
        head_job->err_fd = dup(2);
        if(head_job->err_fd < 0) {
            perror("dup");
            destroy_job_group(grp);
        }
    }

    //printf("Launching %d Jobs...\n", subcmd_count);

    for(size_t i = 0; i < subcmd_count; i++) {
        launch_job(jobs[i]);
    }

    state->fg = grp;

    return 0;
}

static int
job_state_check_running(
        struct job_state *state)
{
    int res;

    if(state->fg) {
        int num_running = 0;

        struct list_node *list_node = state->fg->job_list.node.next;
        while(list_node != &state->fg->job_list.node) {
            struct job *job = container_of(list_node, struct job, group_node);
            if(job->state == JOB_STATE_RUNNING) {
                int exitcode;
                int ret = waitpid(
                            job->pid,
                            &exitcode,
                            WNOHANG);
                if(ret == 0) {
                    // No changes
                    num_running++;
                } else if(ret < 0) {
                    // An error occurred!
                    //printf("Error Occurred When Reaping foreground job!\n");
                    return ret;
                } else {
                    // The job is complete,
                    //printf("Reaped foreground job!\n");
                    job->exitcode = exitcode;
                    job->state = JOB_STATE_EXITED;
                }
            }
            list_node = list_node->next;
        }

        if(num_running == 0) {
            // Clear the foreground job,

            // Cleaning up job
            //printf("Destroying foreground job group!\n");

            struct job_group *grp = state->fg;
            state->fg = NULL;
            res = destroy_job_group(grp);
            if(res) {
                fprintf(stderr, "Failed to destroy foreground job group after all jobs hae exited!\n");
                return res;
            }
        }
    }

    return 0;
}

// <0 -> error
// JOB_TICK_IGNORE -> ignored (forward along to next subsystem)
// JOB_TICK_STEAL  -> accepted (don't forward)
int
job_state_tick(
        struct job_state *state,
        char c,
        int have_char)
{
    int res;

    res = job_state_check_running(state);
    if(res < 0) {
        return res;
    }

    if(have_char) {
        switch(c) {
            case 0x3: // CTRL-C
                if(state->fg) {
                    destroy_job_group(state->fg);
                    state->fg = NULL;
                } else {
                    fprintf(stderr, "No job is currently running!\n");
                }
                return JOB_TICK_STEAL;
            default:
                break;
        }
    }

    if(state->fg != NULL) {

        if(have_char) {
            int written = write(state->fg->in_fd, &c, 1);
            if(written < 0) {
                return -1;
            }
        }

        // Do not forward anything if we have a current job
        return JOB_TICK_STEAL;
    } else {
        return JOB_TICK_IGNORE;
    }
}

