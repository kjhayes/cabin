
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "input.h"

int
input_state_init(
        struct input_state *state)
{
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    state->in_fd = 0;
    return 0;
}

int
input_state_deinit(
        struct input_state *state)
{
    return 0;
}

int
input_state_try_read_char(
        struct input_state *state,
        char *out)
{
    int res = read(state->in_fd, out, 1); 
    if(res < 0) {
        switch(errno) {
            case -EAGAIN:
            case -EWOULDBLOCK:
                return 0;
            default:
                return -1;
        }
    } else if(res == 0) {
        return 0;
    } else if(res == 1) {
        return 1;
    } else {
        return -1;
    }
}

