
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#include "input.h"
#include "command.h"
#include "job.h"

int
main(int argc, const char **argv)
{
    int res;

    struct input_state input_state;
    struct command_state command_state;
    struct job_state job_state;

    res = input_state_init(&input_state);
    if(res) {return res;}
    res = command_state_init(&command_state);
    if(res) {return res;}
    res = job_state_init(&job_state);
    if(res) {return res;}

    while(1)
    {
        char c;

        // Get a character as input
        res = input_state_try_read_char(&input_state, &c);
        if(res < 0) {
            fprintf(stderr, "Failed to read character!\n");
            break;
        }

        int have_char = res == 1;

        // Let the job control try and intercept the 
        // character.
        res = job_state_tick(&job_state, have_char, c);
        if(res < 0) {
            fprintf(stderr, "Job state signalled an error!\n");
            break;
        }

        // The job state does not want to allow any other subsystems to
        // see this character
        if(res == JOB_TICK_STEAL) {
            continue;
        }

        // Give the character to the command buffer
        //printf("command_state_tick\n");
        res = command_state_tick(&command_state, have_char, c);
        if(res < 0) {
            fprintf(stderr, "Command state signalled an error!\n");
            break;
        }

        if(have_char && c == '\n') {
            //printf("command_state_complete_line\n");
            char *line = command_state_complete_line(&command_state);
            if(line == NULL) {
                fprintf(stderr, "Failed to get line from buffer!\n");
                continue;
            }
            printf("job_state_run\n");
            res = job_state_run(&job_state, line);
            if(res) {
                fprintf(stderr, "Failed to run command!\n");
                continue;
            }
        }
    }

    res = input_state_deinit(&input_state);
    if(res) {return res;}
    res = command_state_deinit(&command_state);
    if(res) {return res;}
    res = job_state_deinit(&job_state);
    if(res) {return res;}

    return 0;
}

