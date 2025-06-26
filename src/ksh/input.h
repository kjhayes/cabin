#ifndef __KSH_INPUT_H__
#define __KSH_INPUT_H__

#include <stdio.h>

struct input_state {
    int in_fd;
};

int
input_state_init(
        struct input_state *state);

int
input_state_deinit(
        struct input_state *state);

// 0 -> nothing to read (non-blocking)
// 1 -> read a char
// <0 -> error
int
input_state_try_read_char(
        struct input_state *state,
        char *out);

#endif
